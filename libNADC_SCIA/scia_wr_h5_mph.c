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

.IDENTifer   SCIA_WR_H5_MPH
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     write the Main Product Header of a PDS SCIAMACHY file

.INPUT/OUTPUT
  call as    SCIA_WR_H5_MPH( param, mph );
     input:  
             struct param_record param : struct holding user-defined settings
	     struct mph_envi     *mph  : Main Product Header data

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      4.2   06-Feb-2013	struct layout according to PDS definition, RvH
              4.1   07-Dec-2005	removed delaration of unused variables, RvH
              4.0   13-Oct-2003	moved to the NSCA hdf5_hl routines, RvH
              3.1   03-Sep-2002	moved DSD-data to subgroup Headers, RvH
              3.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              2.0   23-Aug-2001	renamed to SCIA_WR_H5_MDS, RvH 
              1.0   12-Aug-1999 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <string.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_COMMON
#include <nadc_scia.h>

#define NFIELDS    34

static const size_t mph_size = sizeof( struct mph_envi );
static const size_t mph_offs[NFIELDS] = {
     HOFFSET( struct mph_envi, product ),
     HOFFSET( struct mph_envi, proc_stage ),
     HOFFSET( struct mph_envi, ref_doc ),
     HOFFSET( struct mph_envi, acquis ),
     HOFFSET( struct mph_envi, proc_center ),
     HOFFSET( struct mph_envi, proc_time ),
     HOFFSET( struct mph_envi, soft_version ),
     HOFFSET( struct mph_envi, sensing_start ),
     HOFFSET( struct mph_envi, sensing_stop ),
     HOFFSET( struct mph_envi, phase ),
     HOFFSET( struct mph_envi, cycle ),
     HOFFSET( struct mph_envi, rel_orbit ),
     HOFFSET( struct mph_envi, abs_orbit ),
     HOFFSET( struct mph_envi, state_vector ),
     HOFFSET( struct mph_envi, delta_ut ),
     HOFFSET( struct mph_envi, x_position),
     HOFFSET( struct mph_envi, y_position ),
     HOFFSET( struct mph_envi, z_position ),
     HOFFSET( struct mph_envi, x_velocity ),
     HOFFSET( struct mph_envi, y_velocity ),
     HOFFSET( struct mph_envi, z_velocity ),
     HOFFSET( struct mph_envi, vector_source ),
     HOFFSET( struct mph_envi, utc_sbt_time ),
     HOFFSET( struct mph_envi, sat_binary_time ),
     HOFFSET( struct mph_envi, clock_step ),
     HOFFSET( struct mph_envi, leap_utc ),
     HOFFSET( struct mph_envi, leap_sign ),
     HOFFSET( struct mph_envi, leap_err ),
     HOFFSET( struct mph_envi, product_err ),
     HOFFSET( struct mph_envi, tot_size ),
     HOFFSET( struct mph_envi, sph_size ),
     HOFFSET( struct mph_envi, num_dsd ),
     HOFFSET( struct mph_envi, dsd_size ),
     HOFFSET( struct mph_envi, num_data_sets )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_WR_H5_MPH( struct param_record param, 
		     const struct mph_envi *mph )
{
/*     const char prognm[] = "SCIA_WR_H5_MPH"; */

     register unsigned short ni = 0;

     hid_t   mph_type[NFIELDS];

     const int compress = 0;
     const char *mph_names[NFIELDS] = {
	  "product_name", "proc_stage", "ref_doc", 
	  "acquisition_station", "proc_center", "proc_time", 
	  "software_version", 
	  "sensing_start", "sensing_stop", 
	  "phase", "cycle", "rel_orbit", "abs_orbit", "state_vector_time", 
	  "delta_ut1", 
	  "x_position", "y_position", "z_position", 
	  "x_velocity", "y_velocity", "z_velocity",
	  "vector_source", "utc_sbt_time", "sat_binary_time", "clock_step", 
	  "leap_utc", "leap_sign", "leap_err", 
	  "product_err", "tot_size", "sph_size", "num_dsd", "dsd_size", 
	  "num_data_sets"
     };
/*
 * define user-defined data types of the Table-fields
 */
     mph_type[0] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[0], (size_t) ENVI_FILENAME_SIZE );
     mph_type[1] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[1], (size_t) 2 );
     mph_type[2] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[2], (size_t) 24 );

     mph_type[3] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[3], (size_t) 21 );
     mph_type[4] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[4], (size_t) 7 );
     mph_type[5] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[5], (size_t) UTC_STRING_LENGTH );
     mph_type[6] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[6], (size_t) 15 );

     mph_type[7] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[7], (size_t) UTC_STRING_LENGTH );
     mph_type[8] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[8], (size_t) UTC_STRING_LENGTH );

     mph_type[9] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[9], (size_t) 2 );
     mph_type[10] = H5Tcopy( H5T_NATIVE_SHORT );
     mph_type[11] = H5Tcopy( H5T_NATIVE_INT );
     mph_type[12] = H5Tcopy( H5T_NATIVE_INT );
     mph_type[13] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[13], (size_t) UTC_STRING_LENGTH );
     mph_type[14] = H5Tcopy( H5T_NATIVE_DOUBLE );
     mph_type[15] = H5Tcopy( H5T_NATIVE_DOUBLE );
     mph_type[16] = H5Tcopy( H5T_NATIVE_DOUBLE );
     mph_type[17] = H5Tcopy( H5T_NATIVE_DOUBLE );
     mph_type[18] = H5Tcopy( H5T_NATIVE_DOUBLE );
     mph_type[19] = H5Tcopy( H5T_NATIVE_DOUBLE );
     mph_type[20] = H5Tcopy( H5T_NATIVE_DOUBLE );

     mph_type[21] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[21], (size_t) 3 );
     mph_type[22] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[22], (size_t) UTC_STRING_LENGTH );
     mph_type[23] = H5Tcopy( H5T_NATIVE_UINT );
     mph_type[24] = H5Tcopy( H5T_NATIVE_UINT );

     mph_type[25] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[25], (size_t) UTC_STRING_LENGTH );
     mph_type[26] = H5Tcopy( H5T_NATIVE_SHORT );
     mph_type[27] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[27], (size_t) 2 );

     mph_type[28] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( mph_type[28], (size_t) 2 );
     mph_type[29] = H5Tcopy( H5T_NATIVE_UINT );
     mph_type[30] = H5Tcopy( H5T_NATIVE_UINT );
     mph_type[31] = H5Tcopy( H5T_NATIVE_UINT );
     mph_type[32] = H5Tcopy( H5T_NATIVE_UINT );
     mph_type[33] = H5Tcopy( H5T_NATIVE_UINT );
/*
 * create table
 */
     (void) H5TBmake_table( "Main Product Header", param.hdf_file_id, "MPH", 
			    NFIELDS, 1, mph_size, mph_names, mph_offs, 
			    mph_type, 1, NULL, compress, mph );
/*
 * close interface
 */
     do {
	  (void) H5Tclose( mph_type[ni] );
     } while ( ++ni < NFIELDS );
}
