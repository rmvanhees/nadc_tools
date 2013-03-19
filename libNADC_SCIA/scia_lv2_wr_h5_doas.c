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

.IDENTifer   SCIA_LV2_WR_H5_DOAS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIA level 2 DOAS Fitting Application Data Set(s)
.INPUT/OUTPUT
  call as    SCIA_LV2_WR_H5_DOAS( doas_name, param, nr_doas, doas );
     input:  
             char doas_name[]           : name of DOAS data set
             struct param_record param  : struct holding user-defined settings
	     unsigned int nr_doas       : number of Cloud/Aerosol data sets
	     struct doas_scia *doas     : DOAS Fitting Application Data Set(s)

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   26-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.0   18-Sep-2001	created by R. M. van Hees 
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

#define NFIELDS    19

static const size_t doas_size = sizeof( struct doas_scia );
static const size_t doas_offs[NFIELDS] = {
     HOFFSET( struct doas_scia, mjd ),
     HOFFSET( struct doas_scia, quality ),
     HOFFSET( struct doas_scia, vcdflag ),
     HOFFSET( struct doas_scia, escflag ),
     HOFFSET( struct doas_scia, amfflag ),
     HOFFSET( struct doas_scia, intg_time ),
     HOFFSET( struct doas_scia, numfitp ),
     HOFFSET( struct doas_scia, numiter ),
     HOFFSET( struct doas_scia, dsrlen ),
     HOFFSET( struct doas_scia, vcd ),
     HOFFSET( struct doas_scia, errvcd ),
     HOFFSET( struct doas_scia, esc ),
     HOFFSET( struct doas_scia, erresc ),
     HOFFSET( struct doas_scia, rms ),
     HOFFSET( struct doas_scia, amfgnd ),
     HOFFSET( struct doas_scia, amfcld ),
     HOFFSET( struct doas_scia, reflgnd ),
     HOFFSET( struct doas_scia, reflcld ),
     HOFFSET( struct doas_scia, refl )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV2_WR_H5_DOAS( const char doas_name[], struct param_record param, 
			  unsigned int nr_doas, const struct doas_scia *doas )
{
     const char prognm[] = "SCIA_LV2_WR_H5_DOAS";

     register unsigned int nr;

     hid_t   grp_id;
     hid_t   doas_id;
     hbool_t compress;
     hsize_t adim;
     hvl_t   *vdata;
     hid_t   doas_type[NFIELDS];

     const char *doas_names[NFIELDS] = {
          "dsr_time", "quality_flag", "flag_vcd_flags", "flag_slant_col_flags",
	  "flag_amf_flags", "integr_time", "num_fit_para", "iter_num_fit_win",
	  "drs_length", "vcd", "vcd_err", "slant_col_den", "err_slant_col",
	  "rms_chi_2_gof", "amf_gr", "amf_cl", "refl_ground", "refl_cloud_top",
	  "measured_refl"
     };
/*
 * check number of DOAS records
 */
     if ( nr_doas == 0 ) return;
/*
 * set HDF5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
          compress = TRUE;
     else
          compress = FALSE;
/*
 * create group /MDS/<doas_name>
 */
     grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/MDS" );
     if ( grp_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/MDS" );
     doas_id = H5Gcreate( grp_id, doas_name,
			  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     if ( doas_id < 0 ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, doas_name );
/*
 * define user-defined data types of the Table-fields
 */
     doas_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     doas_type[1] = H5Tcopy( H5T_NATIVE_CHAR );
     doas_type[2] = H5Tcopy( H5T_NATIVE_USHORT );
     doas_type[3] = H5Tcopy( H5T_NATIVE_USHORT );
     doas_type[4] = H5Tcopy( H5T_NATIVE_USHORT );
     doas_type[5] = H5Tcopy( H5T_NATIVE_USHORT );
     doas_type[6] = H5Tcopy( H5T_NATIVE_USHORT );
     doas_type[7] = H5Tcopy( H5T_NATIVE_USHORT );
     doas_type[8] = H5Tcopy( H5T_NATIVE_UINT );
     doas_type[9] = H5Tcopy( H5T_NATIVE_FLOAT );
     doas_type[10] = H5Tcopy( H5T_NATIVE_FLOAT );
     doas_type[11] = H5Tcopy( H5T_NATIVE_FLOAT );
     doas_type[12] = H5Tcopy( H5T_NATIVE_FLOAT );
     adim = 3;
     doas_type[13] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     doas_type[14] = H5Tcopy( H5T_NATIVE_FLOAT );
     doas_type[15] = H5Tcopy( H5T_NATIVE_FLOAT );
     doas_type[16] = H5Tcopy( H5T_NATIVE_FLOAT );
     doas_type[17] = H5Tcopy( H5T_NATIVE_FLOAT );
     doas_type[18] = H5Tcopy( H5T_NATIVE_FLOAT );
/*
 * create table
 */
     (void) H5TBmake_table( doas_name, doas_id, "doas", NFIELDS, 
			    nr_doas, doas_size, doas_names, doas_offs, 
			    doas_type, nr_doas, NULL, compress, doas );
/*
 * close interface
 */
     for ( nr = 0; nr < NFIELDS; nr++ ) (void) H5Tclose( doas_type[nr] );
/*
 * +++++ create/write variable part of the <doas_name> group
 */
     adim = (hsize_t) nr_doas;
/*
 * cross correlation parameters
 */
     vdata = (hvl_t *) malloc( nr_doas * sizeof(hvl_t) );
     if ( vdata == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata" );
     nr = 0;
     do {
	  vdata[nr].len = 
	       (size_t) (doas[nr].numfitp * (doas[nr].numfitp-1)) / 2;
	  vdata[nr].p = malloc( vdata[nr].len * sizeof(float) );
	  if ( vdata[nr].p == NULL ) {
	       free( vdata );
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata.p" );
	  }
	  (void) memcpy( vdata[nr].p , doas[nr].corrpar,
			 vdata[nr].len * sizeof(float) );

     } while ( ++nr < nr_doas );
     NADC_WR_HDF5_Vlen_Dataset( compress, doas_id, "corrpar",
				H5T_NATIVE_FLOAT, 1, &adim, vdata );
/*
 * close interface
 */
     (void) H5Gclose( doas_id );
     (void) H5Gclose( grp_id );
}
