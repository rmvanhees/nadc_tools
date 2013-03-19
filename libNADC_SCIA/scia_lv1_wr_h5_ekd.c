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

.IDENTifer   SCIA_LV1_WR_H5_EKD
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 EKD data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_EKD( param, ekd );
     input:  
             struct param_record param : struct holding user-defined settings
	     struct ekd_scia *ekd      : Errors on Key data

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      4.0   04-Oct-2005 there is only one EKD record to write, RvH
              3.0   20-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.0   19-Nov-1999	created by R. M. van Hees 
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

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_EKD( struct param_record param,
			 const struct ekd_scia *ekd )
{
     const char prognm[] = "SCIA_LV1_WR_H5_EKD";

     hid_t   gads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   ekd_type[9];

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;

     const char *ekd_names[] = { "mu2_nadir", "mu3_nadir", "mu2_limb", 
				 "mu3_limb", "radiance_vis", "radiance_nadir", 
				 "radiance_limb", "radiance_sun", "bsdf" };
     const size_t ekd_size = sizeof( struct ekd_scia );
     const size_t ekd_offs[] = {
	  HOFFSET( struct ekd_scia, mu2_nadir ),
	  HOFFSET( struct ekd_scia, mu3_nadir ),
	  HOFFSET( struct ekd_scia, mu2_limb ),
	  HOFFSET( struct ekd_scia, mu3_limb ),
	  HOFFSET( struct ekd_scia, radiance_vis ),
	  HOFFSET( struct ekd_scia, radiance_nadir ),
	  HOFFSET( struct ekd_scia, radiance_limb ),
	  HOFFSET( struct ekd_scia, radiance_sun ),
	  HOFFSET( struct ekd_scia, bsdf ),
     };
/*
 * open/create group /GADS
 */
     gads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/GADS" );
     if ( gads_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/GADS" );
/*
 * write EKD data sets
 */
     adim = SCIENCE_PIXELS;
     ekd_type[0] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     ekd_type[1] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     ekd_type[2] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     ekd_type[3] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     ekd_type[4] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     ekd_type[5] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     ekd_type[6] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     ekd_type[7] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     ekd_type[8] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );

     stat = H5TBmake_table( "ekd", gads_id, "ERRORS_ON_KEY_DATA",
                            9, 1, ekd_size, ekd_names,
                            ekd_offs, ekd_type, 1,
                            NULL, compress, ekd );
     if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, "ekd" );
/*
 * close interface
 */
 done:
     (void) H5Tclose( ekd_type[0] );
     (void) H5Tclose( ekd_type[1] );
     (void) H5Tclose( ekd_type[2] );
     (void) H5Tclose( ekd_type[3] );
     (void) H5Tclose( ekd_type[4] );
     (void) H5Tclose( ekd_type[5] );
     (void) H5Tclose( ekd_type[6] );
     (void) H5Tclose( ekd_type[7] );
     (void) H5Tclose( ekd_type[8] );
     (void) H5Gclose( gads_id );
}
