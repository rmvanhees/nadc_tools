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

.IDENTifer   SCIA_LV1_WR_H5_SCPN
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 SCPN data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_SCPN( param, nr_scpn, scpn );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned int nr_scpn      : number of Spectral Calibration Params
	     struct scpn_scia *scpn    : new Spectral Calibration Parameters

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

#define NFIELDS    9

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_SCPN( struct param_record param, unsigned int nr_scpn,
			  const struct scpn_scia *scpn )
{
     const char prognm[] = "SCIA_LV1_WR_H5_SCPN";

     hid_t   ads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   scpn_type[NFIELDS];

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;
     const size_t scpn_size = sizeof( struct scpn_scia );
     const char *scpn_names[NFIELDS] = { 
	  "mjd", "flag_mds", "orbit_phase", "srs_param", "num_lines", 
	  "wv_error_calib", "sol_spec", "line_pos", "coeffs"
     };
     const size_t scpn_offs[NFIELDS] = {
	  HOFFSET( struct scpn_scia, mjd ),
	  HOFFSET( struct scpn_scia, flag_mds ),
	  HOFFSET( struct scpn_scia, orbit_phase ),
	  HOFFSET( struct scpn_scia, srs_param ),
	  HOFFSET( struct scpn_scia, num_lines ),
	  HOFFSET( struct scpn_scia, wv_error_calib ),
	  HOFFSET( struct scpn_scia, sol_spec ),
	  HOFFSET( struct scpn_scia, line_pos ),
	  HOFFSET( struct scpn_scia, coeffs )
     };
/*
 * check number of PMD records
 */
     if ( nr_scpn == 0 ) return;
/*
 * open/create group /ADS
 */
     ads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/ADS" );
     if ( ads_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/ADS" );
/*
 * write SCPN data sets
 */
     adim = SCIENCE_PIXELS;
     scpn_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     scpn_type[1] = H5Tcopy( H5T_NATIVE_UCHAR );
     scpn_type[2] = H5Tcopy( H5T_NATIVE_FLOAT );
     adim = SCIENCE_CHANNELS;
     scpn_type[3] = H5Tarray_create( H5T_NATIVE_UCHAR, 1, &adim );
     scpn_type[4] = H5Tarray_create( H5T_NATIVE_USHORT, 1, &adim );
     scpn_type[5] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     adim = SCIENCE_PIXELS;
     scpn_type[6] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     adim = 3 * SCIENCE_CHANNELS;
     scpn_type[7] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     adim = NUM_SPEC_COEFFS * SCIENCE_CHANNELS;
     scpn_type[8] = H5Tarray_create( H5T_NATIVE_DOUBLE, 1, &adim );

     stat = H5TBmake_table( "scpn", ads_id, "NEW_SPECTRAL_CALIBRATION", 
                            NFIELDS, 1, scpn_size, scpn_names,
                            scpn_offs, scpn_type, 1,
                            NULL, compress, scpn );
     if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, "scpn" );
/*
 * close interface
 */
 done:
     (void) H5Tclose( scpn_type[0] );
     (void) H5Tclose( scpn_type[1] );
     (void) H5Tclose( scpn_type[2] );
     (void) H5Tclose( scpn_type[3] );
     (void) H5Tclose( scpn_type[4] );
     (void) H5Tclose( scpn_type[5] );
     (void) H5Tclose( scpn_type[6] );
     (void) H5Tclose( scpn_type[7] );
     (void) H5Tclose( scpn_type[8] );
     (void) H5Gclose( ads_id );
}
