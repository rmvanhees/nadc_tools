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

.IDENTifer   SCIA_LV1_WR_H5_SCP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 SCP data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_SCP( param, nr_scp, scp );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned int nr_scp       : number of Spectral Calibration Params.
	     struct scp_scia *scp      : Spectral Calibration Parameters

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

#define NFIELDS    4

static const size_t scp_size = sizeof( struct scp_scia );
static const size_t scp_offs[NFIELDS] = {
     HOFFSET( struct scp_scia, orbit_phase ),
     HOFFSET( struct scp_scia, coeffs ),
     HOFFSET( struct scp_scia, num_lines ),
     HOFFSET( struct scp_scia, wv_error_calib )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_SCP( struct param_record param, unsigned int nr_scp,
			 const struct scp_scia *scp )
{
     hid_t   gads_id;
     hsize_t adim;
     hid_t   scp_type[NFIELDS];

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;

     const char *scp_names[NFIELDS] = {
          "orb_phase", "coeff", "num_lines", "cal_err"
     };
/*
 * check number of SCP records
 */
     if ( nr_scp == 0 ) return;
/*
 * open/create group /GADS
 */
     gads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/GADS" );
     if ( gads_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/GADS" );
/*
 * define user-defined data types of the Table-fields
 */
     scp_type[0] = H5Tcopy( H5T_NATIVE_FLOAT );
     adim = NUM_SPEC_COEFFS * SCIENCE_CHANNELS;
     scp_type[1] = H5Tarray_create( H5T_NATIVE_DOUBLE, 1, &adim );
     adim = SCIENCE_CHANNELS;
     scp_type[2] = H5Tarray_create( H5T_NATIVE_USHORT, 1, &adim );
     scp_type[3] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
/*
 * create table
 */
     (void) H5TBmake_table( "scp", gads_id, "SPECTRAL_CALIBRATION",
			    NFIELDS, nr_scp, scp_size, scp_names, scp_offs, 
			    scp_type, nr_scp, NULL, compress, scp );
/*
 * close interface
 */
     (void) H5Tclose( scp_type[0] );
     (void) H5Tclose( scp_type[1] );
     (void) H5Tclose( scp_type[2] );
     (void) H5Tclose( scp_type[3] );
     (void) H5Gclose( gads_id );
}
