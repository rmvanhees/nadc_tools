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

.IDENTifer   SCIA_LV1_WR_H5_PPG
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 PPG data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_PPG( param, nr_ppg, ppg );
     input:  
             struct param_record param : struct holding user-defined settings
	     struct ppg_scia *ppg      : PPG/Etalon parameters

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   20-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.1   31-Aug-2001	added comment for group PPG, RvH 
              1.0   19-Nov-1999 created by R. M. van Hees
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
void SCIA_LV1_WR_H5_PPG( struct param_record param, 
			 const struct ppg_scia *ppg )
{
     hid_t   gads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   ppg_type[5];

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;

     const char *ppg_names[] = { "ppg_fact", "etalon_fact", "etalon_resid", 
				 "wls_deg_fact", "bad_pixel" };
     const size_t ppg_size = sizeof( struct ppg_scia );
     const size_t ppg_offs[] = {
	  HOFFSET( struct ppg_scia, ppg_fact ),
	  HOFFSET( struct ppg_scia, etalon_fact ),
	  HOFFSET( struct ppg_scia, etalon_resid ),
	  HOFFSET( struct ppg_scia, wls_deg_fact ),
	  HOFFSET( struct ppg_scia, bad_pixel )
     };
/*
 * open/create group /GADS
 */
     gads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/GADS" );
     if ( gads_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/GADS" );
/*
 * write PPG data sets
 */
     adim = SCIENCE_PIXELS;
     ppg_type[0] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     ppg_type[1] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     ppg_type[2] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     ppg_type[3] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     ppg_type[4] = H5Tarray_create( H5T_NATIVE_UCHAR, 1, &adim );

     stat = H5TBmake_table( "ppg", gads_id, "PPG_ETALON",
                            5, 1, ppg_size, ppg_names,
                            ppg_offs, ppg_type, 1,
                            NULL, compress, ppg );
     if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "ppg" );
/*
 * close interface
 */
 done:
     (void) H5Tclose( ppg_type[0] );
     (void) H5Tclose( ppg_type[1] );
     (void) H5Tclose( ppg_type[2] );
     (void) H5Tclose( ppg_type[3] );
     (void) H5Tclose( ppg_type[4] );
     (void) H5Gclose( gads_id );
}
