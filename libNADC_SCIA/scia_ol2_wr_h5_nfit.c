/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2002 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_OL2_WR_H5_NFIT
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 Offline product - HDF5
.LANGUAGE    ANSI C
.PURPOSE     write SCIAMACHY level 2 Nadir Fitting Window Application datasets
.INPUT/OUTPUT
  call as    SCIA_OL2_WR_H5_NFIT( nfit_name, param, nr_nfit, nfit );
     input:  
             char nfit_name[]          : name of fitted species
             struct param_record param : struct holding user-defined settings
	     unsigned int nr_nfit      : number of Nadir Fitting Windows
	     struct nfit_scia *nfit    : Nadir Fitting Window Data Set(s)

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   27-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              1.1   16-Aug-2002	updated to ENV-ID-DLR-SCI-2200-4, RvH
              1.0   17-May-2002	created by R. M. van Hees 
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

static const size_t nfit_size = sizeof( struct nfit_scia );
static const size_t nfit_offs[NFIELDS] = {
     HOFFSET( struct nfit_scia, mjd ),
     HOFFSET( struct nfit_scia, quality ),
     HOFFSET( struct nfit_scia, intg_time ),
     HOFFSET( struct nfit_scia, numvcd ),
     HOFFSET( struct nfit_scia, vcdflag ),
     HOFFSET( struct nfit_scia, num_fitp ),
     HOFFSET( struct nfit_scia, num_nfitp ),
     HOFFSET( struct nfit_scia, numiter ),
     HOFFSET( struct nfit_scia, fitflag ),
     HOFFSET( struct nfit_scia, amfflag ),
     HOFFSET( struct nfit_scia, dsrlen ),
     HOFFSET( struct nfit_scia, esc ),
     HOFFSET( struct nfit_scia, erresc ),
     HOFFSET( struct nfit_scia, rms ),
     HOFFSET( struct nfit_scia, amfgrd ),
     HOFFSET( struct nfit_scia, erramfgrd ),
     HOFFSET( struct nfit_scia, amfcld ),
     HOFFSET( struct nfit_scia, erramfcld ),
     HOFFSET( struct nfit_scia, temperature )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_OL2_WR_H5_NFIT( const char nfit_name[], struct param_record param, 
			  unsigned int nr_nfit, const struct nfit_scia *nfit )
{
     const char prognm[] = "SCIA_OL2_WR_H5_NFIT";

     register unsigned int nr;

     hid_t   grp_id;
     hid_t   nfit_id;
     hbool_t compress;
     hsize_t adim;
     hvl_t   *vdata;
     hid_t   nfit_type[NFIELDS];

     const char *nfit_names[NFIELDS] = {
          "dsr_time", "quality_flag", "integr_time", "num_vcd", 
	  "flag_vcd_flags", "num_lin_fitp", "num_nlin_fitp", 
	  "iter_num_fit_win", "flag_slant_col_flags", "flag_amf_flags", 
	  "dsr_length", "slant_col_den", "err_slant_col", "rms_chi_2_gof",
	  "amf_gr", "amf_gr_err", "amf_cl", "amf_cl_err", "temp_ref"
     };
/*
 * check number of NFIT records
 */
     if ( nr_nfit == 0 ) return;
/*
 * set HDF5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
          compress = TRUE;
     else
          compress = FALSE;
/*
 * create group /MDS/<nfit_name>
 */
     grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/MDS" );
     if ( grp_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/MDS" );
     nfit_id = H5Gcreate( grp_id, nfit_name,
			  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     if ( nfit_id < 0 ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, nfit_name );
/*
 * define user-defined data types of the Table-fields
 */
     nfit_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     nfit_type[1] = H5Tcopy( H5T_NATIVE_CHAR );
     nfit_type[2] = H5Tcopy( H5T_NATIVE_USHORT );
     nfit_type[3] = H5Tcopy( H5T_NATIVE_USHORT );
     nfit_type[4] = H5Tcopy( H5T_NATIVE_USHORT );
     nfit_type[5] = H5Tcopy( H5T_NATIVE_USHORT );
     nfit_type[6] = H5Tcopy( H5T_NATIVE_USHORT );
     nfit_type[7] = H5Tcopy( H5T_NATIVE_USHORT );
     nfit_type[8] = H5Tcopy( H5T_NATIVE_USHORT );
     nfit_type[9] = H5Tcopy( H5T_NATIVE_USHORT );
     nfit_type[10] = H5Tcopy( H5T_NATIVE_UINT );
     nfit_type[11] = H5Tcopy( H5T_NATIVE_FLOAT );
     nfit_type[12] = H5Tcopy( H5T_NATIVE_FLOAT );
     adim = 3;
     nfit_type[13] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     nfit_type[14] = H5Tcopy( H5T_NATIVE_FLOAT );
     nfit_type[15] = H5Tcopy( H5T_NATIVE_FLOAT );
     nfit_type[16] = H5Tcopy( H5T_NATIVE_FLOAT );
     nfit_type[17] = H5Tcopy( H5T_NATIVE_FLOAT );
     nfit_type[18] = H5Tcopy( H5T_NATIVE_FLOAT );
/*
 * create table
 */
     (void) H5TBmake_table( nfit_name, nfit_id, "nfit", NFIELDS, 
			    nr_nfit, nfit_size, nfit_names, nfit_offs, 
			    nfit_type, nr_nfit, NULL, compress, nfit );
/*
 * close interface
 */
     for ( nr = 0; nr < NFIELDS; nr++ ) (void) H5Tclose( nfit_type[nr] );
/*
 * +++++ create/write attributes in the /MDS/NFIT group
 */
     adim = (hsize_t) nr_nfit;
/*
 * Vertical column densities
 */
     vdata = (hvl_t *) malloc( nr_nfit * sizeof( hvl_t ) );
     if ( vdata == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata" );
     nr = 0;
     do {
          vdata[nr].len = (size_t) nfit[nr].numvcd;
          if ( nfit[nr].numvcd > (unsigned short) 0 ) {
               vdata[nr].p = malloc( vdata[nr].len * sizeof(float) );
               if ( vdata[nr].p == NULL ) {
                    free( vdata );
                    NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata.p" );
               }
               (void) memcpy( vdata[nr].p, nfit[nr].vcd,
                              vdata[nr].len * sizeof(float) );
          }
     } while ( ++nr < nr_nfit );
     NADC_WR_HDF5_Vlen_Dataset( compress, nfit_id, "vcd",
                               H5T_NATIVE_FLOAT, 1, &adim, vdata );

     vdata = (hvl_t *) malloc( nr_nfit * sizeof( hvl_t ) );
     if ( vdata == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata" );
     nr = 0;
     do {
          vdata[nr].len = (size_t) nfit[nr].numvcd;
          if ( nfit[nr].numvcd > (unsigned short) 0 ) {
               vdata[nr].p = malloc( vdata[nr].len * sizeof(float) );
               if ( vdata[nr].p == NULL ) {
                    free( vdata );
                    NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata.p" );
               }
               (void) memcpy( vdata[nr].p, nfit[nr].errvcd,
                              vdata[nr].len * sizeof(float) );
          }
     } while ( ++nr < nr_nfit );
     NADC_WR_HDF5_Vlen_Dataset( compress, nfit_id, "errvcd",
                               H5T_NATIVE_FLOAT, 1, &adim, vdata );
/*
 * linear fitted parameters
 */
     vdata = (hvl_t *) malloc( nr_nfit * sizeof( hvl_t ) );
     if ( vdata == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata" );
     nr = 0;
     do {
          vdata[nr].len = (size_t) nfit[nr].num_fitp;
          if ( nfit[nr].num_fitp > (unsigned short) 0 ) {
               vdata[nr].p = malloc( vdata[nr].len * sizeof(float) );
               if ( vdata[nr].p == NULL ) {
                    free( vdata );
                    NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata.p" );
               }
               (void) memcpy( vdata[nr].p, nfit[nr].linpars,
                              vdata[nr].len * sizeof(float) );
          }
     } while ( ++nr < nr_nfit );
     NADC_WR_HDF5_Vlen_Dataset( compress, nfit_id, "linpars",
                               H5T_NATIVE_FLOAT, 1, &adim, vdata );

     vdata = (hvl_t *) malloc( nr_nfit * sizeof( hvl_t ) );
     if ( vdata == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata" );
     nr = 0;
     do {
          vdata[nr].len = (size_t) nfit[nr].num_fitp;
          if ( nfit[nr].num_fitp > (unsigned short) 0 ) {
               vdata[nr].p = malloc( vdata[nr].len * sizeof(float) );
               if ( vdata[nr].p == NULL ) {
                    free( vdata );
                    NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata.p" );
               }
               (void) memcpy( vdata[nr].p, nfit[nr].errlinpars,
                              vdata[nr].len * sizeof(float) );
          }
     } while ( ++nr < nr_nfit );
     NADC_WR_HDF5_Vlen_Dataset( compress, nfit_id, "errlinpars",
                               H5T_NATIVE_FLOAT, 1, &adim, vdata );
/*
 * non-linear fitted parameters
 */
     vdata = (hvl_t *) malloc( nr_nfit * sizeof( hvl_t ) );
     if ( vdata == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata" );
     nr = 0;
     do {
          vdata[nr].len = (size_t) nfit[nr].num_nfitp;
          if ( nfit[nr].num_nfitp > (unsigned short) 0 ) {
               vdata[nr].p = malloc( vdata[nr].len * sizeof(float) );
               if ( vdata[nr].p == NULL ) {
                    free( vdata );
                    NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata.p" );
               }
               (void) memcpy( vdata[nr].p, nfit[nr].nlinpars,
                              vdata[nr].len * sizeof(float) );
          }
     } while ( ++nr < nr_nfit );
     NADC_WR_HDF5_Vlen_Dataset( compress, nfit_id, "nlinpars",
                               H5T_NATIVE_FLOAT, 1, &adim, vdata );

     vdata = (hvl_t *) malloc( nr_nfit * sizeof( hvl_t ) );
     if ( vdata == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata" );
     nr = 0;
     do {
          vdata[nr].len = (size_t) nfit[nr].num_nfitp;
          if ( nfit[nr].num_nfitp > (unsigned short) 0 ) {
               vdata[nr].p = malloc( vdata[nr].len * sizeof(float) );
               if ( vdata[nr].p == NULL ) {
                    free( vdata );
                    NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata.p" );
               }
               (void) memcpy( vdata[nr].p, nfit[nr].errnlinpars,
                              vdata[nr].len * sizeof(float) );
          }
     } while ( ++nr < nr_nfit );
     NADC_WR_HDF5_Vlen_Dataset( compress, nfit_id, "errnlinpars",
                               H5T_NATIVE_FLOAT, 1, &adim, vdata );
/*
 * close interface
 */
     (void) H5Gclose( nfit_id );
     (void) H5Gclose( grp_id );
}
