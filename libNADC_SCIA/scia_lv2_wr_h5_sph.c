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

.IDENTifer   SCIA_LV2_WR_H5_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 2 SPH data
.INPUT/OUTPUT
  call as    SCIA_LV2_WR_H5_SPH( param, sph );
     input:  
             struct param_record param : struct holding user-defined settings
	     struct sph2_scia    *sph  : Specific Product Header data

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   06-Nov-2003 moved to the NSCA hdf5_hl routines, RvH
              2.1   03-Sep-2002	moved DSD-data to subgroup Headers, RvH
              2.0   08-Nov-2001 moved to the new Error handling routines, RvH 
              1.0   18-Aug-2001 created by R. M. van Hees 
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
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

#define NFIELDS    19

static const size_t sph_size = sizeof( struct sph2_scia );
static const size_t sph_offs[NFIELDS] = {
     HOFFSET( struct sph2_scia, fit_error ),
     HOFFSET( struct sph2_scia, descriptor ),
     HOFFSET( struct sph2_scia, start_time ),
     HOFFSET( struct sph2_scia, stop_time ),
     HOFFSET( struct sph2_scia, bias_mol ),
     HOFFSET( struct sph2_scia, doas_mol ),
     HOFFSET( struct sph2_scia, stripline ),
     HOFFSET( struct sph2_scia, slice_pos ),
     HOFFSET( struct sph2_scia, no_slice ),
     HOFFSET( struct sph2_scia, no_bias_win ),
     HOFFSET( struct sph2_scia, no_bias_mol ),
     HOFFSET( struct sph2_scia, no_doas_win ),
     HOFFSET( struct sph2_scia, no_doas_mol ),
     HOFFSET( struct sph2_scia, start_lat ),
     HOFFSET( struct sph2_scia, start_lon ),
     HOFFSET( struct sph2_scia, stop_lat ),
     HOFFSET( struct sph2_scia, stop_lon ),
     HOFFSET( struct sph2_scia, bias_win ),
     HOFFSET( struct sph2_scia, doas_win )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV2_WR_H5_SPH( struct param_record param, 
			 const struct sph2_scia *sph )
{
     register short ni;

     hid_t   type_id;
     hsize_t adim;
     hid_t   sph_type[NFIELDS];

     const hbool_t compress = FALSE;
     const char *sph_names[NFIELDS] = {
          "fitting_error_sum", "sph_descriptor", "start_time", "stop_time",
	  "bias_molecules", "doas_molecules", "stripline_continuity_indicator",
	  "slice_position", "no_of_slices", "no_of_bias_fitting_windows", 
	  "no_of_bias_molecules", "no_of_doas_fitting_windows", 
	  "no_of_doas_molecules", "start_lat", "start_long", 
	  "stop_lat", "stop_long", "bias_windows", "doas_windows"
     };
/*
 * define user-defined data types of the Table-fields
 */
     sph_type[0] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( sph_type[0], (size_t) 5 );
     sph_type[1] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( sph_type[1], (size_t) 19 );
     sph_type[2] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( sph_type[2], (size_t) UTC_STRING_LENGTH );
     sph_type[3] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( sph_type[3], (size_t) UTC_STRING_LENGTH );
     type_id = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( type_id, (size_t) 9 );
     adim = MAX_BIAS_SPECIES;
     sph_type[4] = H5Tarray_create( type_id, 1, &adim );
     adim = MAX_DOAS_SPECIES;
     sph_type[5] = H5Tarray_create( type_id, 1, &adim );
     (void) H5Tclose( type_id );
     sph_type[6] = H5Tcopy( H5T_NATIVE_SHORT );
     sph_type[7] = H5Tcopy( H5T_NATIVE_SHORT );
     sph_type[8] = H5Tcopy( H5T_NATIVE_USHORT );
     sph_type[9] = H5Tcopy( H5T_NATIVE_USHORT );
     sph_type[10] = H5Tcopy( H5T_NATIVE_USHORT );
     sph_type[11] = H5Tcopy( H5T_NATIVE_USHORT );
     sph_type[12] = H5Tcopy( H5T_NATIVE_USHORT );
     sph_type[13] = H5Tcopy( H5T_NATIVE_DOUBLE );
     sph_type[14] = H5Tcopy( H5T_NATIVE_DOUBLE );
     sph_type[15] = H5Tcopy( H5T_NATIVE_DOUBLE );
     sph_type[16] = H5Tcopy( H5T_NATIVE_DOUBLE );
     adim = MAX_BIAS_FITTING_WIN;
     type_id = H5Topen( param.hdf_file_id, "bias_win", H5P_DEFAULT );
     sph_type[17] = H5Tarray_create( type_id, 1, &adim );
     (void) H5Tclose( type_id );
     adim = MAX_DOAS_FITTING_WIN;
     type_id = H5Topen( param.hdf_file_id, "doas_win", H5P_DEFAULT );
     sph_type[18] = H5Tarray_create( type_id, 1, &adim );
     (void) H5Tclose( type_id );
/*
 * create table
 */
     (void) H5TBmake_table( "Specific Product Header", param.hdf_file_id, 
			    "sph", NFIELDS, 1, sph_size, sph_names, sph_offs, 
			    sph_type, 1, NULL, compress, sph );
/*
 * close interface
 */
     for ( ni = 0; ni < NFIELDS; ni++ ) (void) H5Tclose( sph_type[ni] );
}
