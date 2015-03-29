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

.IDENTifer   SCIA_LV1_WR_H5_LCPN
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 LCPN data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_LCPN( param, nr_lcpn, lcpn );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned int nr_lcpn      : number of new Leakage Current Param.
	     struct lcpn_scia *lcpn    : new Leakage Current Paramameters

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   22-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.0   24-Nov-1999	Created by R. M. van Hees 
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

#define NFIELDS    12

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_LCPN( struct param_record param, unsigned int nr_lcpn,
			  const struct lcpn_scia *lcpn )
{
     hid_t   ads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   lcpn_type[NFIELDS];

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;
     const size_t lcpn_size = sizeof( struct lcpn_scia );
     const char *lcpn_names[NFIELDS] = { 
	  "mjd", "mjd_last", "flag_mds", "orbit_phase", "obm_pmd", "fpn", 
	  "fpn_error", "lc", "lc_error", "mean_noise", "pmd_off", 
	  "pmd_off_error"
     };
     const size_t lcpn_offs[NFIELDS] = {
	  HOFFSET( struct lcpn_scia, mjd ),
	  HOFFSET( struct lcpn_scia, mjd_last ),
	  HOFFSET( struct lcpn_scia, flag_mds ),
	  HOFFSET( struct lcpn_scia, orbit_phase ),
	  HOFFSET( struct lcpn_scia, obm_pmd ),
	  HOFFSET( struct lcpn_scia, fpn ),
	  HOFFSET( struct lcpn_scia, fpn_error ),
	  HOFFSET( struct lcpn_scia, lc ),
	  HOFFSET( struct lcpn_scia, lc_error ),
	  HOFFSET( struct lcpn_scia, mean_noise ),
	  HOFFSET( struct lcpn_scia, pmd_off ),
	  HOFFSET( struct lcpn_scia, pmd_off_error ),
     };
/*
 * check number of PMD records
 */
     if ( nr_lcpn == 0 ) return;
/*
 * open/create group /ADS
 */
     ads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/ADS" );
     if ( ads_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/ADS" );
/*
 * write LCPN data sets
 */
     adim = SCIENCE_PIXELS;
     lcpn_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     lcpn_type[1] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     lcpn_type[2] = H5Tcopy( H5T_NATIVE_UCHAR );
     lcpn_type[3] = H5Tcopy( H5T_NATIVE_FLOAT );
     adim = IR_CHANNELS + PMD_NUMBER;
     lcpn_type[4] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     adim = SCIENCE_PIXELS;
     lcpn_type[5] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     lcpn_type[6] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     lcpn_type[7] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     lcpn_type[8] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     lcpn_type[9] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     adim = 2 * PMD_NUMBER;
     lcpn_type[10] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     lcpn_type[11] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );

     stat = H5TBmake_table( "lcpn", ads_id, "NEW_LEAKAGE", 
                            NFIELDS, 1, lcpn_size, lcpn_names,
                            lcpn_offs, lcpn_type, 1,
                            NULL, compress, lcpn );
     if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "lcpn" );
/*
 * close interface
 */
 done:
     (void) H5Tclose( lcpn_type[0] );
     (void) H5Tclose( lcpn_type[1] );
     (void) H5Tclose( lcpn_type[2] );
     (void) H5Tclose( lcpn_type[3] );
     (void) H5Tclose( lcpn_type[4] );
     (void) H5Tclose( lcpn_type[5] );
     (void) H5Tclose( lcpn_type[6] );
     (void) H5Tclose( lcpn_type[7] );
     (void) H5Tclose( lcpn_type[8] );
     (void) H5Tclose( lcpn_type[9] );
     (void) H5Tclose( lcpn_type[10] );
     (void) H5Tclose( lcpn_type[11] );
     (void) H5Gclose( ads_id );
}
