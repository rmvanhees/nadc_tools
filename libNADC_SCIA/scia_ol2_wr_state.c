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

.IDENTifer   SCIA_OL2_WR_H5_STATE
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 2 STATE data

.INPUT/OUTPUT
  call as    SCIA_OL2_WR_H5_STATE(nr_state, state);
     input:  
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

static const size_t state_size = sizeof(struct state2_scia);
static const size_t state_offs[NFIELDS] = {
     HOFFSET(struct state2_scia, mjd),
     HOFFSET(struct state2_scia, flag_mds),
     HOFFSET(struct state2_scia, state_id),
     HOFFSET(struct state2_scia, duration),
     HOFFSET(struct state2_scia, longest_intg_time),
     HOFFSET(struct state2_scia, shortest_intg_time),
     HOFFSET(struct state2_scia, num_obs_state)
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_OL2_WR_H5_STATE(unsigned int nr_state,
			  const struct state2_scia *state)
{
     hid_t   ads_id;
     hid_t   state_type[NFIELDS];

     const hid_t fid = nadc_get_param_hid("hdf_file_id");
     const hbool_t compress = FALSE;
     const char    *state_names[NFIELDS] = {
          "dsr_time", "attach_flag", "state_id", "duration_scan_state", 
	  "longest_int_time", "shortest_int_time", "num_obs_state"
     };
/*
 * check number of STATE records
 */
     if (nr_state == 0) return;
/*
 * open group /ADS
 */
     ads_id = NADC_OPEN_HDF5_Group(fid, "/ADS");
     if (ads_id < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, "/ADS");
/*
 * create group /ADS
 */
     ads_id = NADC_OPEN_HDF5_Group(fid, "/ADS");
     if (ads_id < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, "/ADS");
/*
 * define user-defined data types of the Table-fields
 */
     state_type[0] = H5Topen(fid, "mjd", H5P_DEFAULT);
     state_type[1] = H5Tcopy(H5T_NATIVE_UCHAR);
     state_type[2] = H5Tcopy(H5T_NATIVE_USHORT);
     state_type[3] = H5Tcopy(H5T_NATIVE_USHORT);
     state_type[4] = H5Tcopy(H5T_NATIVE_USHORT);
     state_type[5] = H5Tcopy(H5T_NATIVE_USHORT);
     state_type[6] = H5Tcopy(H5T_NATIVE_USHORT);
/*
 * create table
 */
     (void) H5TBmake_table("States of the Product", ads_id, "state", NFIELDS, 
			    nr_state, state_size, state_names, state_offs, 
			    state_type, nr_state, NULL, compress, state);
/*
 * close interface
 */
     (void) H5Tclose(state_type[0]);
     (void) H5Tclose(state_type[1]);
     (void) H5Tclose(state_type[2]);
     (void) H5Tclose(state_type[3]);
     (void) H5Tclose(state_type[4]);
     (void) H5Tclose(state_type[5]);
     (void) H5Tclose(state_type[6]);
     (void) H5Gclose(ads_id);
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_OL2_WR_ASCII_STATE
.PURPOSE    dump -- in ASCII Format -- the STATE records
.INPUT/OUTPUT
  call as   SCIA_OL2_WR_ASCII_STATE(num_dsr, state);
     input:
            unsigned int num_dsr      : number of data sets
            struct state2_scia *state : pointer to STATE records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_OL2_WR_ASCII_STATE(unsigned int num_dsr,
			     const struct state2_scia *state)
{
     register unsigned int nd, nr;

     char date_str[UTC_STRING_LENGTH];

     char *cpntr;
     FILE *outfl;

     cpntr = nadc_get_param_string("outfile");
     outfl = CRE_ASCII_File(cpntr, "state");
     if (outfl == NULL || IS_ERR_STAT_FATAL)
          NADC_RETURN_ERROR(NADC_ERR_FILE_CRE, cpntr);
     free(cpntr);
/*
 * write ASCII dump of STATE record
 */
     cpntr = nadc_get_param_string("infile");
     nadc_write_header(outfl, 0, cpntr, "States of the Product");
     free(cpntr);
     for (nd = 0; nd < num_dsr; nd++) {
          nr = 1;
          (void) MJD_2_ASCII(state[nd].mjd.days, state[nd].mjd.secnd,
                              state[nd].mjd.musec, date_str);
          nadc_write_text(outfl, nr, "MJD (Date Time)", date_str);
          nadc_write_double(outfl, nr++, "MJD (Julian Day)", 16, 
                             (double) state[nd].mjd.days + 
                             ((state[nd].mjd.secnd + state[nd].mjd.musec / 1e6)
                              / (24. * 60 * 60)));
          nadc_write_uchar(outfl, nr++, "MDS DSR attached", 
                             state[nd].flag_mds);
          nadc_write_ushort(outfl, nr++, "State ID", 
                              state[nd].state_id);
          nadc_write_ushort(outfl, nr++, "Duration of scan phase", 
                              state[nd].duration);
          nadc_write_ushort(outfl, nr++, "Longest integration time", 
                              state[nd].longest_intg_time);
          nadc_write_ushort(outfl, nr++, "Shortest integration time", 
                              state[nd].shortest_intg_time);
          nadc_write_ushort(outfl, nr++, "Number of Observations", 
                              state[nd].num_obs_state);
     }
     (void) fclose(outfl);
}
