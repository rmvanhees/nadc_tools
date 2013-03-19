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

.IDENTifer   SCIA_LV1_WR_H5_SRS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 SRS data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_SRS( param, nr_srs, srs );
     input:
             struct param_record param : struct holding user-defined settings
	     unsigned int nr_srs       : number of Sun reference spectra
	     struct srs_scia *srs    : Sun Reference Spectrum 

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   20-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.0   18-Nov-1999	Created by R. M. van Hees 
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

#define NFIELDS    13

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_SRS( struct param_record param, unsigned int nr_srs,
			 const struct srs_scia *srs )
{
     const char prognm[] = "SCIA_LV1_WR_H5_SRS";

     hid_t   gads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   srs_type[NFIELDS];

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;

     const char *srs_names[NFIELDS] = { 
	  "sun_spec_id", "avg_asm", "avg_esm", "avg_elev_sun", 
	  "dopp_shift", "wvlen_sun", "mean_sun", "precision_sun", 
	  "accuracy_sun", "etalon", "pmd_mean", "pmd_out_nd_out", 
	  "pmd_out_nd_in"
     };
     const size_t srs_size = sizeof( struct srs_scia );
     const size_t srs_offs[NFIELDS] = {
	  HOFFSET( struct srs_scia, sun_spec_id ),
	  HOFFSET( struct srs_scia, avg_asm ),
	  HOFFSET( struct srs_scia, avg_esm ),
	  HOFFSET( struct srs_scia, avg_elev_sun ),
	  HOFFSET( struct srs_scia, dopp_shift ),
	  HOFFSET( struct srs_scia, wvlen_sun ),
	  HOFFSET( struct srs_scia, mean_sun ),
	  HOFFSET( struct srs_scia, precision_sun ),
	  HOFFSET( struct srs_scia, accuracy_sun ),
	  HOFFSET( struct srs_scia, etalon ),
	  HOFFSET( struct srs_scia, pmd_mean ),
	  HOFFSET( struct srs_scia, pmd_out_nd_out ),
	  HOFFSET( struct srs_scia, pmd_out_nd_in )
     };
/*
 * check number of RSPO records
 */
     if ( nr_srs == 0 ) return;
/*
 * open/create group /GADS
 */
     gads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/GADS" );
     if ( gads_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/GADS" );
/*
 * write SRS data sets
 */
     adim = 3;
     srs_type[0] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( srs_type[0], (size_t) 3 );
     srs_type[1] = H5Tcopy( H5T_NATIVE_FLOAT );
     srs_type[2] = H5Tcopy( H5T_NATIVE_FLOAT );
     srs_type[3] = H5Tcopy( H5T_NATIVE_FLOAT );
     srs_type[4] = H5Tcopy( H5T_NATIVE_FLOAT );
     adim = SCIENCE_PIXELS;
     srs_type[5] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     srs_type[6] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     srs_type[7] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     srs_type[8] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     srs_type[9] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     adim = PMD_NUMBER;
     srs_type[10] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     srs_type[11] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     srs_type[12] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );

     stat = H5TBmake_table( "srs", gads_id, "SUN_REFERENCE",
                            NFIELDS, nr_srs, srs_size, srs_names,
                            srs_offs, srs_type, 1,
                            NULL, compress, srs );
     if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, "srs" );
/*
 * close interface
 */
 done:
     (void) H5Tclose( srs_type[0] );
     (void) H5Tclose( srs_type[1] );
     (void) H5Tclose( srs_type[2] );
     (void) H5Tclose( srs_type[3] );
     (void) H5Tclose( srs_type[4] );
     (void) H5Tclose( srs_type[5] );
     (void) H5Tclose( srs_type[6] );
     (void) H5Tclose( srs_type[7] );
     (void) H5Tclose( srs_type[8] );
     (void) H5Tclose( srs_type[9] );
     (void) H5Tclose( srs_type[10] );
     (void) H5Tclose( srs_type[11] );
     (void) H5Tclose( srs_type[12] );
     (void) H5Gclose( gads_id );
}
