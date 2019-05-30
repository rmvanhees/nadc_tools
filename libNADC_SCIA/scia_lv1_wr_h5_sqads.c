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

.IDENTifer   SCIA_LV1_WR_H5_SQADS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 SQADS data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_SQADS(nr_sqads, sqads);

     input:  
	     unsigned int nr_sqads      : number of Summary of Quality Flags
	     struct sqads1_scia *sqads  : Summary of Quality Flags per State

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   20-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.1   03-Sep-2002	moved DSD-data to subgroup ADS, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.0   17-Nov-1999	created by R. M. van Hees 
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

#define NFIELDS    10

static const size_t sqads_size = sizeof(struct sqads1_scia);
static const size_t sqads_offs[NFIELDS] = {
     HOFFSET(struct sqads1_scia, mjd),
     HOFFSET(struct sqads1_scia, flag_mds),
     HOFFSET(struct sqads1_scia, flag_glint),
     HOFFSET(struct sqads1_scia, flag_rainbow),
     HOFFSET(struct sqads1_scia, flag_saa_region),
     HOFFSET(struct sqads1_scia, missing_readouts),
     HOFFSET(struct sqads1_scia, hotpixel),
     HOFFSET(struct sqads1_scia, mean_wv_diff),
     HOFFSET(struct sqads1_scia, sdev_wv_diff),
     HOFFSET(struct sqads1_scia, mean_diff_leak)
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_SQADS(unsigned int nr_sqads,
			  const struct sqads1_scia *sqads)
{
     hid_t   ads_id;
     hsize_t adim;

     const hid_t fid = nadc_get_param_hid("hdf_file_id");
     const hbool_t compress =
	  (nadc_get_param_uint8("flag_deflate") == PARAM_SET) ? TRUE : FALSE;
     const char *sqads_names[NFIELDS] = {
          "dsr_time", "attach_flag", "sun_glint_flag", "rainbow_flag",
	  "saa_region_flag", "num_miss_readouts", "num_hotpixels_perchannel",
	  "mean_wavlen_diff", "std_dev_wavlen_diff", "mean_diff_leak"
     };
#if !defined(__mips) && !defined (__hpux)
     hid_t sqads_type[NFIELDS] = {
          -1, H5T_NATIVE_UCHAR, H5T_NATIVE_UCHAR, H5T_NATIVE_UCHAR, 
	  H5T_NATIVE_UCHAR, H5T_NATIVE_USHORT, -1, -1, -1, -1
     };
#else
     hid_t sqads_type[NFIELDS];

     sqads_type[0] = -1;
     sqads_type[1] = H5T_NATIVE_UCHAR;
     sqads_type[2] = H5T_NATIVE_UCHAR;
     sqads_type[3] = H5T_NATIVE_UCHAR;
     sqads_type[4] = H5T_NATIVE_UCHAR;
     sqads_type[5] = H5T_NATIVE_USHORT;
     sqads_type[6] = -1;
     sqads_type[7] = -1;
     sqads_type[8] = -1;
     sqads_type[9] = -1;
#endif
/*
 * check number of SQADS records
 */
     if (nr_sqads == 0) return;
/*
 * create group /ADS
 */
     ads_id = NADC_OPEN_HDF5_Group(fid, "/ADS");
     if (ads_id < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, "/ADS");
/*
 * define user-defined data types of the Table-fields
 */
     sqads_type[0] = H5Topen(fid, "mjd", H5P_DEFAULT);
     adim = ALL_CHANNELS;
     sqads_type[6] = H5Tarray_create(H5T_NATIVE_USHORT, 1, &adim);
     adim = SCIENCE_CHANNELS;
     sqads_type[7] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
     adim = SCIENCE_CHANNELS;
     sqads_type[8] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
     adim = ALL_CHANNELS;
     sqads_type[9] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
/*
 * create table
 */
     (void) H5TBmake_table("sqads", ads_id, "SUMMARY_QUALITY", NFIELDS, 
			    nr_sqads, sqads_size, sqads_names, sqads_offs, 
			    sqads_type, nr_sqads, NULL, compress, sqads);
/*
 * close interface
 */
     (void) H5Tclose(sqads_type[0]);
     (void) H5Tclose(sqads_type[6]);
     (void) H5Tclose(sqads_type[7]);
     (void) H5Tclose(sqads_type[8]);
     (void) H5Tclose(sqads_type[9]);
     (void) H5Gclose(ads_id);
}
