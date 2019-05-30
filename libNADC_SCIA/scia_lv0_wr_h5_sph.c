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

.IDENTifer   SCIA_LV0_WR_H5_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 0 SPH data
.INPUT/OUTPUT
  call as    SCIA_LV0_WR_H5_SPH(sph);
     input:  
	     struct sph0_scia    *sph  : Specific Product Header data

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      4.1   06-Feb-2013	struct layout according to PDS definition, RvH
              4.0   26-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              3.1   03-Sep-2002	moved DSD-data to subgroup Headers, RvH
              3.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              2.0   01-Nov-2001	moved to new Error handling routines, RvH 
              1.0   27-Mar-2001 created by R. M. van Hees
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
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

#define NFIELDS    20

static const size_t sph_size = sizeof(struct sph0_scia);
static const size_t sph_offs[NFIELDS] = {
     HOFFSET(struct sph0_scia, descriptor),
     HOFFSET(struct sph0_scia, start_lat),
     HOFFSET(struct sph0_scia, start_lon),
     HOFFSET(struct sph0_scia, stop_lat),
     HOFFSET(struct sph0_scia, stop_lon),
     HOFFSET(struct sph0_scia, sat_track),

     HOFFSET(struct sph0_scia, isp_errors),
     HOFFSET(struct sph0_scia, missing_isps),
     HOFFSET(struct sph0_scia, isp_discard),
     HOFFSET(struct sph0_scia, rs_sign),

     HOFFSET(struct sph0_scia, num_error_isps),
     HOFFSET(struct sph0_scia, error_isps_thres),
     HOFFSET(struct sph0_scia, num_miss_isps),
     HOFFSET(struct sph0_scia, miss_isps_thres),
     HOFFSET(struct sph0_scia, num_discard_isps),
     HOFFSET(struct sph0_scia, discard_isps_thres),
     HOFFSET(struct sph0_scia, num_rs_isps),
     HOFFSET(struct sph0_scia, rs_thres),

     HOFFSET(struct sph0_scia, tx_rx_polar),
     HOFFSET(struct sph0_scia, swath)
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV0_WR_H5_SPH(const struct sph0_scia *sph)
{
     register unsigned short ni = 0;

     hid_t   fid, sph_type[NFIELDS];

     const int compress = 0;
     const char *sph_names[NFIELDS] = {
          "sph_descriptor", 
	  "start_lat", "start_long", "stop_lat", "stop_long", "sat_track", 
	  "isp_errors_significant", "missing_isps_significant", 
	  "isp_discard_significant", "rs_significant", 
	  "num_error_isps", "error_isps_thresh", 
	  "num_missing_isps", "missing_isps_thresh", 
	  "num_discarded_isps", "discarded_isps_thresh", 
	  "num_rs_isps", "rs_thresh",
	  "tx_rx_polar", "swath"
     };
/*
 * define user-defined data types of the Table-fields
 */
     sph_type[0] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[0], (size_t) 29);
     sph_type[1] = H5Tcopy(H5T_NATIVE_DOUBLE);
     sph_type[2] = H5Tcopy(H5T_NATIVE_DOUBLE);
     sph_type[3] = H5Tcopy(H5T_NATIVE_DOUBLE);
     sph_type[4] = H5Tcopy(H5T_NATIVE_DOUBLE);
     sph_type[5] = H5Tcopy(H5T_NATIVE_DOUBLE);
     sph_type[6] = H5Tcopy(H5T_NATIVE_USHORT);
     sph_type[7] = H5Tcopy(H5T_NATIVE_USHORT);
     sph_type[8] = H5Tcopy(H5T_NATIVE_USHORT);
     sph_type[9] = H5Tcopy(H5T_NATIVE_USHORT);
     sph_type[10] = H5Tcopy(H5T_NATIVE_INT);
     sph_type[11] = H5Tcopy(H5T_NATIVE_DOUBLE);
     sph_type[12] = H5Tcopy(H5T_NATIVE_INT);
     sph_type[13] = H5Tcopy(H5T_NATIVE_DOUBLE);
     sph_type[14] = H5Tcopy(H5T_NATIVE_INT);
     sph_type[15] = H5Tcopy(H5T_NATIVE_DOUBLE);
     sph_type[16] = H5Tcopy(H5T_NATIVE_INT);
     sph_type[17] = H5Tcopy(H5T_NATIVE_DOUBLE);
     sph_type[18] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[18], (size_t) 6);
     sph_type[19] = H5Tcopy(H5T_C_S1);
     (void) H5Tset_size(sph_type[19], (size_t) 4);
/*
 * create table
 */
     fid = nadc_get_param_hid("hdf_file_id");
     (void) H5TBmake_table("Specific Product Header", fid, 
			    "SPH", NFIELDS, 1, sph_size, sph_names, sph_offs, 
			    sph_type, 1, NULL, compress, sph);
/*
 * close interface
 */
     do {
	  (void) H5Tclose(sph_type[ni]);
     } while (++ni < NFIELDS);
}
