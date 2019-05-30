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

.IDENTifer   SCIA_LV1_WR_H5_PPGN
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 PPGN data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_PPGN(nr_ppgn, ppgn);
     input:  
	     unsigned int nr_ppgn      : number of PPGN/Etalon parameters
	     struct ppgn_scia *ppgn    : new PPG/Etalon parameters

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   22-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.0   24-Nov-1999	Created by R. M. van Hees 
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

#define NFIELDS    8

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_PPGN(unsigned int nr_ppgn, const struct ppgn_scia *ppgn)
{
     hid_t   fid, ads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   ppgn_type[NFIELDS];

     const hbool_t compress = \
	  (nadc_get_param_uint8("flag_deflate") == PARAM_SET) ? TRUE : FALSE;
     const size_t ppgn_size = sizeof(struct ppgn_scia);
     const char *ppgn_names[NFIELDS] = { 
	  "mjd", "flag_mds", "gain_fact", "etalon_fact", "etalon_resid", 
	  "avg_wls_spec", "sd_wls_spec", "bad_pixel"
     };
     const size_t ppgn_offs[NFIELDS] = {
	  HOFFSET(struct ppgn_scia, mjd),
	  HOFFSET(struct ppgn_scia, flag_mds),
	  HOFFSET(struct ppgn_scia, gain_fact),
	  HOFFSET(struct ppgn_scia, etalon_fact),
	  HOFFSET(struct ppgn_scia, etalon_resid),
	  HOFFSET(struct ppgn_scia, avg_wls_spec),
	  HOFFSET(struct ppgn_scia, sd_wls_spec),
	  HOFFSET(struct ppgn_scia, bad_pixel)
     };
/*
 * check number of PMD records
 */
     if (nr_ppgn == 0) return;
/*
 * open/create group /ADS
 */
     fid = nadc_get_param_hid("hdf_file_id");
     ads_id = NADC_OPEN_HDF5_Group(fid, "/ADS");
     if (ads_id < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, "/ADS");
/*
 * write PPGN data sets
 */
     adim = SCIENCE_PIXELS;
     ppgn_type[0] = H5Topen(fid, "mjd", H5P_DEFAULT);
     ppgn_type[1] = H5Tcopy(H5T_NATIVE_UCHAR);
     adim = SCIENCE_PIXELS;
     ppgn_type[2] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
     ppgn_type[3] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
     ppgn_type[4] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
     ppgn_type[5] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
     ppgn_type[6] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
     ppgn_type[7] = H5Tarray_create(H5T_NATIVE_UCHAR, 1, &adim);

     stat = H5TBmake_table("ppgn", ads_id, "NEW_PPG_ETALON", 
                            NFIELDS, 1, ppgn_size, ppgn_names,
                            ppgn_offs, ppgn_type, 1,
                            NULL, compress, ppgn);
     if (stat < 0) NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "ppgn");
/*
 * close interface
 */
 done:
     (void) H5Tclose(ppgn_type[0]);
     (void) H5Tclose(ppgn_type[1]);
     (void) H5Tclose(ppgn_type[2]);
     (void) H5Tclose(ppgn_type[3]);
     (void) H5Tclose(ppgn_type[4]);
     (void) H5Tclose(ppgn_type[5]);
     (void) H5Tclose(ppgn_type[6]);
     (void) H5Tclose(ppgn_type[7]);
     (void) H5Gclose(ads_id);
}
