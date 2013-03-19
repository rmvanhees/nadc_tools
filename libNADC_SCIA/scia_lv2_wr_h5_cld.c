/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2013 SRON (R.M.van.Hees@sron.nl)

   This is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License, version 2, as
   published by the Free Software Foundation.

   The software is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA  02111-1307, USA.

.IDENTifer   SCIA_LV2_WR_H5_CLD
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 2 Cloud/Aerosol Data Set(s)
.INPUT/OUTPUT
  call as    SCIA_LV2_WR_H5_CLD( param, nr_cld, cld );
     input:  
             struct param_record param  : struct holding user-defined settings
	     unsigned int nr_cld        : number of Cloud/Aerosol data sets
	     struct cld_scia *cld       : Cloud/Aerosol Data Set(s)

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   26-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.0   16-Sep-2001	Created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

#define NFIELDS    11

static const size_t cld_size = sizeof( struct cld_scia );
static const size_t cld_offs[NFIELDS] = {
     HOFFSET( struct cld_scia, mjd ),
     HOFFSET( struct cld_scia, quality ),
     HOFFSET( struct cld_scia, quality_cld ),
     HOFFSET( struct cld_scia, outputflag ),
     HOFFSET( struct cld_scia, intg_time ),
     HOFFSET( struct cld_scia, numpmd ),
     HOFFSET( struct cld_scia, dsrlen ),
     HOFFSET( struct cld_scia, cloudfrac ),
     HOFFSET( struct cld_scia, toppress ),
     HOFFSET( struct cld_scia, aai ),
     HOFFSET( struct cld_scia, albedo )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV2_WR_H5_CLD( struct param_record param, unsigned int nr_cld,
			 const struct cld_scia *cld )
{
     const char prognm[] = "SCIA_LV2_WR_H5_CLD";

     register hsize_t  nr;

     hid_t   grp_id;
     hbool_t compress;
     hsize_t adim;
     hvl_t   *vdata;
     hid_t   cld_type[NFIELDS];

     const char *cld_names[NFIELDS] = {
          "dsr_time", "quality_flag", "qual_frac_cloud_cov", 
	  "flag_output_flags", "integr_time", "num_pmd", "dsr_length", 
	  "ave_clo_fract_foot", "cl_top_pres", "aero_abso_ind", "equiv_lam_alb"
     };
/*
 * check number of CLD records
 */
     if ( nr_cld == 0 ) return;
/*
 * set HDF5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
          compress = TRUE;
     else
          compress = FALSE;
/*
 * create group /MDS
 */
     grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/MDS" );
     if ( grp_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/MDS" );
/*
 * define user-defined data types of the Table-fields
 */
     cld_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     cld_type[1] = H5Tcopy( H5T_NATIVE_CHAR );
     cld_type[2] = H5Tcopy( H5T_NATIVE_UCHAR );
     cld_type[3] = H5Tcopy( H5T_NATIVE_USHORT );
     cld_type[4] = H5Tcopy( H5T_NATIVE_USHORT );
     cld_type[5] = H5Tcopy( H5T_NATIVE_USHORT );
     cld_type[6] = H5Tcopy( H5T_NATIVE_UINT );
     cld_type[7] = H5Tcopy( H5T_NATIVE_FLOAT );
     cld_type[8] = H5Tcopy( H5T_NATIVE_FLOAT );
     cld_type[9] = H5Tcopy( H5T_NATIVE_FLOAT );
     cld_type[10] = H5Tcopy( H5T_NATIVE_FLOAT );
/*
 * create table
 */
     (void) H5TBmake_table( "Cloud end Aerosol MDS", grp_id, "cld", NFIELDS, 
			    nr_cld, cld_size, cld_names, cld_offs, 
			    cld_type, nr_cld, NULL, compress, cld );
/*
 * close interface
 */
     for ( nr = 0; nr < NFIELDS; nr++ ) (void) H5Tclose( cld_type[nr] );
/*
 * +++++ create/write variable part of the CLOUDS_AEROSOL record
 */
     adim = (hsize_t) nr_cld;
/*
 * cloud fraction per PMD pixel
 */
     vdata = (hvl_t *) malloc( nr_cld * sizeof(hvl_t) );
     if ( vdata == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata" );
     nr = 0;
     do {
	  vdata[nr].len = (size_t) cld[nr].numpmd;
	  vdata[nr].p = malloc( vdata[nr].len * sizeof(float) );
	  if ( vdata[nr].p == NULL ) {
	       free( vdata );
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata.p" );
	  }
	  (void) memcpy( vdata[nr].p , cld[nr].pmdcloudfrac,
			 vdata[nr].len * sizeof(float) );

     } while ( ++nr < nr_cld );
     NADC_WR_HDF5_Vlen_Dataset( compress, grp_id, "cl_frac",
				H5T_NATIVE_FLOAT, 1, &adim, vdata );
/*
 * close interface
 */
     (void) H5Gclose( grp_id );
}
