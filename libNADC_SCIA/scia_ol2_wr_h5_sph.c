/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2019 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_OL2_WR_H5_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SPH from SCIAMACHY Offline level 2 product
.INPUT/OUTPUT
  call as    SCIA_OL2_WR_H5_SPH(sph);
     input:  
	     struct sph_sci_ol   *sph  : Specific Product Header data

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      2.1   07-Dec-2005 write lat/lon values as doubles, RvH
              2.0   26-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              1.1   03-Sep-2002	moved DSD-data to subgroup Headers, RvH
              1.0   26-Apr-2002	created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

#define NFIELDS   60

static const size_t sph_size = sizeof(struct sph_sci_ol);
static const size_t sph_offs[NFIELDS] = {
     HOFFSET(struct sph_sci_ol, dbserver),
     HOFFSET(struct sph_sci_ol, errorsum),
     HOFFSET(struct sph_sci_ol, descriptor),
     HOFFSET(struct sph_sci_ol, decont),
     HOFFSET(struct sph_sci_ol, nadir_win_uv0),
     HOFFSET(struct sph_sci_ol, nadir_win_uv1),
     HOFFSET(struct sph_sci_ol, nadir_win_uv2),
     HOFFSET(struct sph_sci_ol, nadir_win_uv3),
     HOFFSET(struct sph_sci_ol, nadir_win_uv4),
     HOFFSET(struct sph_sci_ol, nadir_win_uv5),
     HOFFSET(struct sph_sci_ol, nadir_win_uv6),
     HOFFSET(struct sph_sci_ol, nadir_win_uv7),
     HOFFSET(struct sph_sci_ol, nadir_win_uv8),
     HOFFSET(struct sph_sci_ol, nadir_win_uv9),
     HOFFSET(struct sph_sci_ol, nadir_win_ir0),
     HOFFSET(struct sph_sci_ol, nadir_win_ir1),
     HOFFSET(struct sph_sci_ol, nadir_win_ir2),
     HOFFSET(struct sph_sci_ol, nadir_win_ir3),
     HOFFSET(struct sph_sci_ol, nadir_win_ir4),
     HOFFSET(struct sph_sci_ol, nadir_win_ir5),
     HOFFSET(struct sph_sci_ol, limb_win_pth),
     HOFFSET(struct sph_sci_ol, limb_win_uv0),
     HOFFSET(struct sph_sci_ol, limb_win_uv1),
     HOFFSET(struct sph_sci_ol, limb_win_uv2),
     HOFFSET(struct sph_sci_ol, limb_win_uv3),
     HOFFSET(struct sph_sci_ol, limb_win_uv4),
     HOFFSET(struct sph_sci_ol, limb_win_uv5),
     HOFFSET(struct sph_sci_ol, limb_win_uv6),
     HOFFSET(struct sph_sci_ol, limb_win_uv7),
     HOFFSET(struct sph_sci_ol, limb_win_ir0),
     HOFFSET(struct sph_sci_ol, limb_win_ir1),
     HOFFSET(struct sph_sci_ol, limb_win_ir2),
     HOFFSET(struct sph_sci_ol, limb_win_ir3),
     HOFFSET(struct sph_sci_ol, limb_win_ir4),
     HOFFSET(struct sph_sci_ol, occl_win_pth),
     HOFFSET(struct sph_sci_ol, occl_win_uv0),
     HOFFSET(struct sph_sci_ol, occl_win_uv1),
     HOFFSET(struct sph_sci_ol, occl_win_uv2),
     HOFFSET(struct sph_sci_ol, occl_win_uv3),
     HOFFSET(struct sph_sci_ol, occl_win_uv4),
     HOFFSET(struct sph_sci_ol, occl_win_uv5),
     HOFFSET(struct sph_sci_ol, occl_win_uv6),
     HOFFSET(struct sph_sci_ol, occl_win_uv7),
     HOFFSET(struct sph_sci_ol, occl_win_ir0),
     HOFFSET(struct sph_sci_ol, occl_win_ir1),
     HOFFSET(struct sph_sci_ol, occl_win_ir2),
     HOFFSET(struct sph_sci_ol, occl_win_ir3),
     HOFFSET(struct sph_sci_ol, occl_win_ir4),
     HOFFSET(struct sph_sci_ol, start_time),
     HOFFSET(struct sph_sci_ol, stop_time),
     HOFFSET(struct sph_sci_ol, stripline),
     HOFFSET(struct sph_sci_ol, slice_pos),
     HOFFSET(struct sph_sci_ol, no_slice),
     HOFFSET(struct sph_sci_ol, no_nadir_win),
     HOFFSET(struct sph_sci_ol, no_limb_win),
     HOFFSET(struct sph_sci_ol, no_occl_win),
     HOFFSET(struct sph_sci_ol, start_lat),
     HOFFSET(struct sph_sci_ol, start_lon),
     HOFFSET(struct sph_sci_ol, stop_lat),
     HOFFSET(struct sph_sci_ol, stop_lon),
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_OL2_WR_H5_SPH(const struct sph_sci_ol *sph)
{
     register short ni;

     hid_t   sph_type[NFIELDS];

     const hid_t fid = nadc_get_param_uint8("hdf_file_id");
     const hbool_t compress = FALSE;
     const char *sph_names[NFIELDS] = {
	  "db_server_ver", "fitting_error_sum", "sph_descriptor", "decont", 
	  "nadir_win_uv0", "nadir_win_uv1", "nadir_win_uv2", "nadir_win_uv3", 
	  "nadir_win_uv4", "nadir_win_uv5", "nadir_win_uv6", "nadir_win_uv7", 
	  "nadir_win_uv8", "nadir_win_uv9",
	  "nadir_win_ir0", "nadir_win_ir1", "nadir_win_ir2", "nadir_win_ir3", 
	  "nadir_win_ir4", "nadir_win_ir5", 
	  "limb_win_pth", "limb_win_uv0", "limb_win_uv1", "limb_win_uv2", 
	  "limb_win_uv3", "limb_win_uv4", "limb_win_uv5", "limb_win_uv6", 
	  "limb_win_uv7", "limb_win_ir0", "limb_win_ir1", "limb_win_ir2", 
	  "limb_win_ir3", "limb_win_ir4", 
	  "occl_win_pth", "occl_win_uv0", "occl_win_uv1", "occl_win_uv2", 
	  "occl_win_uv3", "occl_win_uv4", "occl_win_uv5", "occl_win_uv6", 
	  "occl_win_uv7", "occl_win_ir0", "occl_win_ir1", "occl_win_ir2", 
	  "occl_win_ir3", "occl_win_ir4", 
	  "start_time", "stop_time", 
	  "stripline_continuity_indicator", "slice_position", 
	  "num_slices", "no_of_nadir_fitting_windows", 
	  "no_of_limb_fitting_windows", "no_of_occl_fitting_windows", 
	  "start_lat", "start_long", "stop_lat", "stop_long"
     };
/*
 * define user-defined data types of the Table-fields
 */
     sph_type[0] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[0], (size_t) 6);
     sph_type[1] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[1], (size_t) 5);
     sph_type[2] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[2], (size_t) 29);
     sph_type[3] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[3], (size_t) 42);
     sph_type[4] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[4], (size_t) 31);
     sph_type[5] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[5], (size_t) 31);
     sph_type[6] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[6], (size_t) 31);
     sph_type[7] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[7], (size_t) 31);
     sph_type[8] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[8], (size_t) 31);
     sph_type[9] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[9], (size_t) 31);
     sph_type[10] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[10], (size_t) 31);
     sph_type[11] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[11], (size_t) 31);
     sph_type[12] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[12], (size_t) 31);
     sph_type[13] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[13], (size_t) 31);
     sph_type[14] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[14], (size_t) 31);
     sph_type[15] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[15], (size_t) 31);
     sph_type[16] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[16], (size_t) 31);
     sph_type[17] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[17], (size_t) 31);
     sph_type[18] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[18], (size_t) 31);
     sph_type[19] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[19], (size_t) 31);
     sph_type[20] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[20], (size_t) 31);
     sph_type[21] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[21], (size_t) 31);
     sph_type[22] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[22], (size_t) 31);
     sph_type[23] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[23], (size_t) 31);
     sph_type[24] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[24], (size_t) 31);
     sph_type[25] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[25], (size_t) 31);
     sph_type[26] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[26], (size_t) 31);
     sph_type[27] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[27], (size_t) 31);
     sph_type[28] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[28], (size_t) 31);
     sph_type[29] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[29], (size_t) 31);
     sph_type[30] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[30], (size_t) 31);
     sph_type[31] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[31], (size_t) 31);
     sph_type[32] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[32], (size_t) 31);
     sph_type[33] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[33], (size_t) 31);
     sph_type[34] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[34], (size_t) 31);
     sph_type[35] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[35], (size_t) 31);
     sph_type[36] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[36], (size_t) 31);
     sph_type[37] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[37], (size_t) 31);
     sph_type[38] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[38], (size_t) 31);
     sph_type[39] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[39], (size_t) 31);
     sph_type[40] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[40], (size_t) 31);
     sph_type[41] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[41], (size_t) 31);
     sph_type[42] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[42], (size_t) 31);
     sph_type[43] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[43], (size_t) 31);
     sph_type[44] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[44], (size_t) 31);
     sph_type[45] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[45], (size_t) 31);
     sph_type[46] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[46], (size_t) 31);
     sph_type[47] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[47], (size_t) 31);
     sph_type[48] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[48], (size_t) UTC_STRING_LENGTH);
     sph_type[49] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[49], (size_t) UTC_STRING_LENGTH);
     sph_type[50] = H5Tcopy(H5T_NATIVE_SHORT);
     sph_type[51] = H5Tcopy(H5T_NATIVE_SHORT);
     sph_type[52] = H5Tcopy(H5T_NATIVE_USHORT);
     sph_type[53] = H5Tcopy(H5T_NATIVE_USHORT);
     sph_type[54] = H5Tcopy(H5T_NATIVE_USHORT);
     sph_type[55] = H5Tcopy(H5T_NATIVE_USHORT);
     sph_type[56] = H5Tcopy(H5T_NATIVE_DOUBLE);
     sph_type[57] = H5Tcopy(H5T_NATIVE_DOUBLE);
     sph_type[58] = H5Tcopy(H5T_NATIVE_DOUBLE);
     sph_type[59] = H5Tcopy(H5T_NATIVE_DOUBLE);
/*
 * create table
 */
     (void) H5TBmake_table("Specific Product Header", fid, 
			    "sph", NFIELDS, 1, sph_size, sph_names, sph_offs, 
			    sph_type, 1, NULL, compress, sph);
/*
 * close interface
 */
     for (ni = 0; ni < NFIELDS; ni++) (void) H5Tclose(sph_type[ni]);
}
