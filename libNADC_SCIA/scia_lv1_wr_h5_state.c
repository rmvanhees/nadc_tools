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

.IDENTifer   SCIA_LV1_WR_H5_STATE
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 STATE data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_STATE(nr_state, state);

     input:  
	     unsigned int nr_state      : number of Summary of Quality Flags
	     struct state1_scia *state  : States of the Product

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.1   12-Apr-2005 removed bug found after linking with hdf5-1.6.4
              3.0   20-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.1   08-Jun-2001	store Cluster configuration as compound, RvH 
              1.0   24-Nov-1999 created by R. M. van Hees
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

#define NFIELDS    23

static const size_t state_size = sizeof(struct state1_scia);
static const size_t state_offs[NFIELDS] = {
     HOFFSET(struct state1_scia, mjd),
     HOFFSET(struct state1_scia, Clcon),
     HOFFSET(struct state1_scia, flag_mds),
     HOFFSET(struct state1_scia, flag_reason),
     HOFFSET(struct state1_scia, type_mds),
     HOFFSET(struct state1_scia, category),
     HOFFSET(struct state1_scia, state_id),
     HOFFSET(struct state1_scia, dur_scan),
     HOFFSET(struct state1_scia, longest_intg_time),
     HOFFSET(struct state1_scia, num_clus),
     HOFFSET(struct state1_scia, num_aux),
     HOFFSET(struct state1_scia, num_pmd),
     HOFFSET(struct state1_scia, num_intg),
     HOFFSET(struct state1_scia, intg_times),
     HOFFSET(struct state1_scia, num_polar),
     HOFFSET(struct state1_scia, total_polar),
     HOFFSET(struct state1_scia, num_dsr),
     HOFFSET(struct state1_scia, indx),
     HOFFSET(struct state1_scia, length_dsr),
     HOFFSET(struct state1_scia, offset),
     HOFFSET(struct state1_scia, offs_pmd),
     HOFFSET(struct state1_scia, offs_polV),
     HOFFSET(struct state1_scia, orbit_phase)
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_STATE(unsigned int nr_state,
			  const struct state1_scia *state)
{
     hid_t   fid, ads_id;
     hid_t   type_id;
     hbool_t compress;
     hsize_t adim;
     herr_t  stat;

     const char *state_names[NFIELDS] = {
          "dsr_time", "clus_config", "attach_flag", "reason_code", "mds_type", 
	  "meas_cat", "state_id", "dur_scan_phase", "longest_intg_time", 
	  "num_clus", "num_rep_geo", "num_pmd", "num_diff_intg_times", 
	  "intg_times", "num_pol_per_intg", "num_pol", "num_dsr", "indxSTATE", 
	  "len_dsr", "offs_MDS", "offs_PMD", "offs_POLV", "orb_phase"
     };
#if !defined(__mips) && !defined (__hpux)
     hid_t state_type[NFIELDS] = {
          -1, -1, H5T_NATIVE_UCHAR, H5T_NATIVE_UCHAR,
	  H5T_NATIVE_UCHAR, H5T_NATIVE_USHORT, H5T_NATIVE_USHORT,
	  H5T_NATIVE_USHORT, H5T_NATIVE_USHORT, H5T_NATIVE_USHORT,
	  H5T_NATIVE_USHORT, H5T_NATIVE_USHORT, H5T_NATIVE_USHORT,
	  -1, -1, H5T_NATIVE_USHORT, H5T_NATIVE_USHORT, H5T_NATIVE_UINT,
	  H5T_NATIVE_UINT, H5T_NATIVE_UINT, H5T_NATIVE_UINT, H5T_NATIVE_UINT, 
	  H5T_NATIVE_FLOAT
     };
#else
     hid_t state_type[NFIELDS];

     state_type[0] = -1;
     state_type[1] = -1;
     state_type[2] = H5T_NATIVE_UCHAR;
     state_type[3] = H5T_NATIVE_UCHAR;
     state_type[4] = H5T_NATIVE_UCHAR;
     state_type[5] = H5T_NATIVE_USHORT;
     state_type[6] = H5T_NATIVE_USHORT;
     state_type[7] = H5T_NATIVE_USHORT;
     state_type[8] = H5T_NATIVE_USHORT;
     state_type[9] = H5T_NATIVE_USHORT;
     state_type[10] = H5T_NATIVE_USHORT;
     state_type[11] = H5T_NATIVE_USHORT;
     state_type[12] = H5T_NATIVE_USHORT;
     state_type[13] = -1;
     state_type[14] = -1;
     state_type[15] = H5T_NATIVE_USHORT;
     state_type[16] = H5T_NATIVE_USHORT;
     state_type[17] = H5T_NATIVE_UINT;
     state_type[18] = H5T_NATIVE_UINT;
     state_type[19] = H5T_NATIVE_UINT;
     state_type[20] = H5T_NATIVE_UINT;
     state_type[21] = H5T_NATIVE_UINT;
     state_type[22] = H5T_NATIVE_FLOAT;
#endif
/*
 * check number of STATE records
 */
     if (nr_state == 0) return;
/*
 * set HDF5 boolean variable for compression
 */
     if (nadc_get_param_uint8("flag_deflate") == PARAM_SET)
          compress = TRUE;
     else
          compress = FALSE;
/*
 * open group /ADS/STATE
 */
     fid = nadc_get_param_hid("hdf_file_id");
     ads_id = NADC_OPEN_HDF5_Group(fid, "/ADS");
     if (ads_id < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, "/ADS");
/*
 * define user-defined data types of the Table-fields
 */
     state_type[0] = H5Topen(fid, "mjd", H5P_DEFAULT);
     adim = MAX_CLUSTER;
     type_id = H5Topen(fid, "Clcon", H5P_DEFAULT);
     state_type[1] = H5Tarray_create(type_id, 1, &adim);
     state_type[13] = H5Tarray_create(H5T_NATIVE_USHORT, 1, &adim);
     state_type[14] = H5Tarray_create(H5T_NATIVE_USHORT, 1, &adim);
/*
 * create table
 */
     stat = H5TBmake_table("state", ads_id, "STATES", NFIELDS, 
			    nr_state, state_size, state_names, state_offs, 
			    state_type, nr_state, NULL, compress, state);
     if (stat < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "state");
/*
 * close interface
 */
     (void) H5Tclose(state_type[0]);
     (void) H5Tclose(state_type[1]);
     (void) H5Tclose(type_id);
     (void) H5Tclose(state_type[13]);
     (void) H5Tclose(state_type[14]);
     (void) H5Gclose(ads_id);
}
