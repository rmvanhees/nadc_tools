/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2000 - 2019 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_WR_H5_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 SPH data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_SPH(sph);
     input:  
	     struct sph1_scia    *sph  : Specific Product Header data

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   06-Nov-2003 moved to the NSCA hdf5_hl routines, RvH
              2.1   03-Sep-2002	moved DSD-data to subgroup Headers, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.0   13-Aug-1999	created by R. M. van Hees 
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
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

#define NFIELDS    24

static const size_t sph_size = sizeof(struct sph1_scia);
static const size_t sph_offs[NFIELDS] = {
     HOFFSET(struct sph1_scia, spec_cal),
     HOFFSET(struct sph1_scia, saturate),
     HOFFSET(struct sph1_scia, dark_check),
     HOFFSET(struct sph1_scia, dead_pixel),
     HOFFSET(struct sph1_scia, key_data),
     HOFFSET(struct sph1_scia, m_factor),
     HOFFSET(struct sph1_scia, descriptor),
     HOFFSET(struct sph1_scia, init_version),
     HOFFSET(struct sph1_scia, start_time),
     HOFFSET(struct sph1_scia, stop_time),
     HOFFSET(struct sph1_scia, stripline),
     HOFFSET(struct sph1_scia, slice_pos),
     HOFFSET(struct sph1_scia, no_slice),
     HOFFSET(struct sph1_scia, no_nadir),
     HOFFSET(struct sph1_scia, no_limb),
     HOFFSET(struct sph1_scia, no_occult),
     HOFFSET(struct sph1_scia, no_monitor),
     HOFFSET(struct sph1_scia, no_noproc),
     HOFFSET(struct sph1_scia, comp_dark),
     HOFFSET(struct sph1_scia, incomp_dark),
     HOFFSET(struct sph1_scia, start_lat),
     HOFFSET(struct sph1_scia, start_lon),
     HOFFSET(struct sph1_scia, stop_lat),
     HOFFSET(struct sph1_scia, stop_lon)
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_SPH(const struct sph1_scia *sph)
{
     hid_t   fid;
     hid_t   sph_type[NFIELDS];
     hid_t   string_type00, string_type01, string_type02, string_type03, 
	  string_type04, string_type05, string_type06, string_type07, 
	  string_type08, string_type09;
     int     *fill_data = NULL;

     const hsize_t chunk_size = 16;
     const int     compress   = FALSE;

     const char *sph_names[NFIELDS] = {
          "spec_cal_check_sum", "saturated_pixel", "dark_check_sum", 
	  "dead_pixel", "key_data_version", "m_factor_version", 
	  "sph_descriptor", "init_version", "start_time", "stop_time", 
	  "stripline_continuity_indicator", "slice_position", 
	  "no_of_slices", "no_of_nadir_states", "no_of_limb_states", 
	  "no_of_occultation_states", "no_of_monitor_states", 
	  "no_of_noproc_states", "comp_dark_states", "incomp_dark_states", 
	  "start_lat", "start_long", "stop_lat", "stop_long"
     };
/*
 * define user-defined data types of the Table-fields
 */
     string_type00 = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(string_type00, (size_t) 5);
     string_type01 = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(string_type01, (size_t) 5);
     string_type02 = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(string_type02, (size_t) 5);
     string_type03 = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(string_type03, (size_t) 5);
     string_type04 = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(string_type04, (size_t) 6);
     string_type05 = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(string_type05, (size_t) 6);
     string_type06 = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(string_type06, (size_t) 29);
     string_type07 = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(string_type07, (size_t) 38);
     string_type08 = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(string_type08, (size_t) UTC_STRING_LENGTH);
     string_type09 = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(string_type09, (size_t) UTC_STRING_LENGTH);
     sph_type[0]  = string_type00;
     sph_type[1]  = string_type01;
     sph_type[2]  = string_type02;
     sph_type[3]  = string_type03;
     sph_type[4]  = string_type04;
     sph_type[5]  = string_type05;
     sph_type[6]  = string_type06;
     sph_type[7]  = string_type07;
     sph_type[8]  = string_type08;
     sph_type[9]  = string_type09;
     sph_type[10] = H5T_NATIVE_SHORT;
     sph_type[11] = H5T_NATIVE_SHORT;
     sph_type[12] = H5T_NATIVE_USHORT;
     sph_type[13] = H5T_NATIVE_USHORT;
     sph_type[14] = H5T_NATIVE_USHORT;
     sph_type[15] = H5T_NATIVE_USHORT;
     sph_type[16] = H5T_NATIVE_USHORT;
     sph_type[17] = H5T_NATIVE_USHORT;
     sph_type[18] = H5T_NATIVE_USHORT;
     sph_type[19] = H5T_NATIVE_USHORT;
     sph_type[20] = H5T_NATIVE_DOUBLE;
     sph_type[21] = H5T_NATIVE_DOUBLE;
     sph_type[22] = H5T_NATIVE_DOUBLE;
     sph_type[23] = H5T_NATIVE_DOUBLE;
/*
 * create table
 */
     fid = nadc_get_param_hid("hdf_file_id");
     (void) H5TBmake_table("Specific Product Header", fid, 
			    "SPH", (hsize_t) NFIELDS, (hsize_t) 1, sph_size, 
			    sph_names, sph_offs, sph_type, 
			    chunk_size, fill_data, compress, sph);
/*
 * close interface
 */
     (void) H5Tclose(string_type00);
     (void) H5Tclose(string_type01);
     (void) H5Tclose(string_type02);
     (void) H5Tclose(string_type03);
     (void) H5Tclose(string_type04);
     (void) H5Tclose(string_type05);
     (void) H5Tclose(string_type06);
     (void) H5Tclose(string_type07);
     (void) H5Tclose(string_type08);
     (void) H5Tclose(string_type09);
}
