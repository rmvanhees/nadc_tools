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

.IDENTifer   SCIA_LV1_WR_H5_DARK
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 DARK data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_DARK(nr_dark, dark);
     input:  
	     unsigned int nr_dark      : number of DARK/Etalon parameters
	     struct dark_scia *dark    : new DARK average parameters

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   22-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.0   03-Jan-2001	Created by R. M. van Hees 
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

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_DARK(unsigned int nr_dark, const struct dark_scia *dark)
{
     hid_t   fid, ads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   dark_type[NFIELDS];

     const hbool_t compress = \
	  (nadc_get_param_uint8("flag_deflate") == PARAM_SET) ? TRUE : FALSE;
     const size_t dark_size = sizeof(struct dark_scia);
     const char *dark_names[NFIELDS] = { 
	  "mjd", "flag_mds", "dark_spec", "sdev_dark_spec", "pmd_off", 
	  "pmd_off_error", "sol_stray", "sol_stray_error", "pmd_stray", 
	  "pmd_stray_error"
     };
     const size_t dark_offs[NFIELDS] = {
	  HOFFSET(struct dark_scia, mjd),
	  HOFFSET(struct dark_scia, flag_mds),
	  HOFFSET(struct dark_scia, dark_spec),
	  HOFFSET(struct dark_scia, sdev_dark_spec),
	  HOFFSET(struct dark_scia, pmd_off),
	  HOFFSET(struct dark_scia, pmd_off_error),
	  HOFFSET(struct dark_scia, sol_stray),
	  HOFFSET(struct dark_scia, sol_stray_error),
	  HOFFSET(struct dark_scia, pmd_stray),
	  HOFFSET(struct dark_scia, pmd_stray_error)
     };
/*
 * check number of PMD records
 */
     if (nr_dark == 0) return;
/*
 * open/create group /ADS
 */
     fid = nadc_get_param_hid("hdf_file_id");
     ads_id = NADC_OPEN_HDF5_Group(fid, "/ADS");
     if (ads_id < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, "/ADS");
/*
 * write DARK data sets
 */
     adim = SCIENCE_PIXELS;
     dark_type[0] = H5Topen(fid, "mjd", H5P_DEFAULT);
     dark_type[1] = H5Tcopy(H5T_NATIVE_UCHAR);
     adim = SCIENCE_PIXELS;
     dark_type[2] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
     dark_type[3] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
     adim = 2 * PMD_NUMBER;
     dark_type[4] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
     dark_type[5] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
     adim = SCIENCE_PIXELS;
     dark_type[6] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
     dark_type[7] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
     adim = PMD_NUMBER;
     dark_type[8] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
     dark_type[9] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);

     stat = H5TBmake_table("dark", ads_id, "DARK_AVERAGE", 
                            NFIELDS, 1, dark_size, dark_names,
                            dark_offs, dark_type, 1,
                            NULL, compress, dark);
     if (stat < 0) NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "dark");
/*
 * close interface
 */
 done:
     (void) H5Tclose(dark_type[0]);
     (void) H5Tclose(dark_type[1]);
     (void) H5Tclose(dark_type[2]);
     (void) H5Tclose(dark_type[3]);
     (void) H5Tclose(dark_type[4]);
     (void) H5Tclose(dark_type[5]);
     (void) H5Tclose(dark_type[6]);
     (void) H5Tclose(dark_type[7]);
     (void) H5Tclose(dark_type[8]);
     (void) H5Tclose(dark_type[9]);
     (void) H5Gclose(ads_id);
}
