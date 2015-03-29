/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2000 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_WR_H5_SRSN
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 SRSN data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_SRSN( param, nr_srsn, srsn );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned int nr_srsn      : number of Sun reference spectra
	     struct srsn_scia *srsn  : Sun Reference Spectrum 

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   21-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.0   24-Nov-1999	created by R. M. van Hees 
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
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

#define NFIELDS    15

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_SRSN( struct param_record param, unsigned int nr_srsn,
			  const struct srsn_scia *srsn )
{
     hid_t   ads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   srsn_type[NFIELDS];

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;
     const size_t srsn_size = sizeof( struct srsn_scia );
     const char *srsn_names[NFIELDS] = { 
	  "mjd", "flag_mds", "flag_neu", "sun_spec_id", "avg_asm", "avg_esm", 
	  "avg_elev_sun", "dopp_shift", "wvlen_sun", "mean_sun", 
	  "precision_sun", "accuracy_sun", "etalon", "pmd_mean", "pmd_out"
     };
     const size_t srsn_offs[NFIELDS] = {
	  HOFFSET( struct srsn_scia, mjd ),
	  HOFFSET( struct srsn_scia, flag_mds ),
	  HOFFSET( struct srsn_scia, flag_neu ),
	  HOFFSET( struct srsn_scia, sun_spec_id ),
	  HOFFSET( struct srsn_scia, avg_asm ),
	  HOFFSET( struct srsn_scia, avg_esm ),
	  HOFFSET( struct srsn_scia, avg_elev_sun ),
	  HOFFSET( struct srsn_scia, dopp_shift ),
	  HOFFSET( struct srsn_scia, wvlen_sun ),
	  HOFFSET( struct srsn_scia, mean_sun ),
	  HOFFSET( struct srsn_scia, precision_sun ),
	  HOFFSET( struct srsn_scia, accuracy_sun ),
	  HOFFSET( struct srsn_scia, etalon ),
	  HOFFSET( struct srsn_scia, pmd_mean ),
	  HOFFSET( struct srsn_scia, pmd_out )
     };
/*
 * check number of PMD records
 */
     if ( nr_srsn == 0 ) return;
/*
 * open/create group /ADS
 */
     ads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/ADS" );
     if ( ads_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/ADS" );
/*
 * write SRSN data sets
 */
     adim = SCIENCE_PIXELS;
     srsn_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     srsn_type[1] = H5Tcopy( H5T_NATIVE_UCHAR );
     srsn_type[2] = H5Tcopy( H5T_NATIVE_UCHAR );
     srsn_type[3] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( srsn_type[3], (size_t) 3 );
     srsn_type[4] = H5Tcopy( H5T_NATIVE_FLOAT );
     srsn_type[5] = H5Tcopy( H5T_NATIVE_FLOAT );
     srsn_type[6] = H5Tcopy( H5T_NATIVE_FLOAT );
     srsn_type[7] = H5Tcopy( H5T_NATIVE_FLOAT );
     adim = SCIENCE_PIXELS;
     srsn_type[8] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     srsn_type[9] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     srsn_type[10] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     srsn_type[11] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     srsn_type[12] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     adim = PMD_NUMBER;
     srsn_type[13] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     srsn_type[14] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );

     stat = H5TBmake_table( "srsn", ads_id, "NEW_SUN_REFERENCE", 
                            NFIELDS, 1, srsn_size, srsn_names,
                            srsn_offs, srsn_type, 1,
                            NULL, compress, srsn );
     if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "srsn" );
/*
 * close interface
 */
 done:
     (void) H5Tclose( srsn_type[0] );
     (void) H5Tclose( srsn_type[1] );
     (void) H5Tclose( srsn_type[2] );
     (void) H5Tclose( srsn_type[3] );
     (void) H5Tclose( srsn_type[4] );
     (void) H5Tclose( srsn_type[5] );
     (void) H5Tclose( srsn_type[6] );
     (void) H5Tclose( srsn_type[7] );
     (void) H5Tclose( srsn_type[8] );
     (void) H5Tclose( srsn_type[9] );
     (void) H5Tclose( srsn_type[10] );
     (void) H5Tclose( srsn_type[11] );
     (void) H5Tclose( srsn_type[12] );
     (void) H5Tclose( srsn_type[13] );
     (void) H5Tclose( srsn_type[14] );
     (void) H5Gclose( ads_id );
}
