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

.IDENTifer   SCIA_LV2_WR_H5_STATE
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 2 STATE data

.INPUT/OUTPUT
  call as    SCIA_LV2_WR_H5_STATE( param, nr_state, state );
     input:  
             struct param_record param  : struct holding user-defined settings
	     unsigned int nr_state      : number of Summary of Quality Flags
	     struct state2_scia *state  : States of the Product

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   26-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.2   03-Sep-2002	moved DSD-data to subgroup ADS, RvH
              2.1   03-Sep-2002	moved DSD-data to subgroup ADS, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.0   13-Sep-2001	created by R. M. van Hees 
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

#define NFIELDS    7

static const size_t state_size = sizeof( struct state2_scia );
static const size_t state_offs[NFIELDS] = {
     HOFFSET( struct state2_scia, mjd ),
     HOFFSET( struct state2_scia, flag_mds ),
     HOFFSET( struct state2_scia, state_id ),
     HOFFSET( struct state2_scia, duration ),
     HOFFSET( struct state2_scia, longest_intg_time ),
     HOFFSET( struct state2_scia, shortest_intg_time ),
     HOFFSET( struct state2_scia, num_obs_state )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV2_WR_H5_STATE( struct param_record param, unsigned int nr_state,
			   const struct state2_scia *state )
{
     hid_t   ads_id;
     hid_t   state_type[NFIELDS];

     const hbool_t compress = FALSE;
     const char    *state_names[NFIELDS] = {
          "dsr_time", "attach_flag", "state_id", "duration_scan_state", 
	  "longest_int_time", "shortest_int_time", "num_obs_state"
     };
/*
 * check number of STATE records
 */
     if ( nr_state == 0 ) return;
/*
 * open group /ADS
 */
     ads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/ADS" );
     if ( ads_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/ADS" );
/*
 * create group /ADS
 */
     ads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/ADS" );
     if ( ads_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/ADS" );
/*
 * define user-defined data types of the Table-fields
 */
     state_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     state_type[1] = H5Tcopy( H5T_NATIVE_UCHAR );
     state_type[2] = H5Tcopy( H5T_NATIVE_USHORT );
     state_type[3] = H5Tcopy( H5T_NATIVE_USHORT );
     state_type[4] = H5Tcopy( H5T_NATIVE_USHORT );
     state_type[5] = H5Tcopy( H5T_NATIVE_USHORT );
     state_type[6] = H5Tcopy( H5T_NATIVE_USHORT );
/*
 * create table
 */
     (void) H5TBmake_table( "States of the Product", ads_id, "state", NFIELDS, 
			    nr_state, state_size, state_names, state_offs, 
			    state_type, nr_state, NULL, compress, state );
/*
 * close interface
 */
     (void) H5Tclose( state_type[0] );
     (void) H5Tclose( state_type[1] );
     (void) H5Tclose( state_type[2] );
     (void) H5Tclose( state_type[3] );
     (void) H5Tclose( state_type[4] );
     (void) H5Tclose( state_type[5] );
     (void) H5Tclose( state_type[6] );
     (void) H5Gclose( ads_id );
}
