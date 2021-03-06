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

.IDENTifer   SCIA_LV1_WR_H5_SFP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 SFP data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_SFP(nr_sfp, sfp);
     input:  
	     unsigned int nr_sfp       : number of SFP parameters
	     struct sfp_scia *sfp      : slit function parameters

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   21-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.0   19-Dec-2000	Created by R. M. van Hees 
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

#define NFIELDS    4

static const size_t sfp_size = sizeof(struct sfp_scia);
static const size_t sfp_offs[NFIELDS] = {
     HOFFSET(struct sfp_scia, type),
     HOFFSET(struct sfp_scia, pixel_position),
     HOFFSET(struct sfp_scia, fwhm),
     HOFFSET(struct sfp_scia, fwhm_gauss)
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_SFP(unsigned int nr_sfp, const struct sfp_scia *sfp)
{
     hid_t  gads_id;

     const hid_t   fid = nadc_get_param_hid("hdf_file_id");
     const hbool_t compress =
	  (nadc_get_param_uint8("flag_deflate") == PARAM_SET) ? TRUE : FALSE;
     const char *sfp_names[NFIELDS] = {
          "type", "pixel_position", "fwhm", "fwhm_gauss"
     };
#if !defined(__mips) && !defined (__hpux)
     const hid_t sfp_type[NFIELDS] = {
	  H5T_NATIVE_CHAR, H5T_NATIVE_SHORT, 
	  H5T_NATIVE_DOUBLE, H5T_NATIVE_DOUBLE
     };
#else
     hid_t sfp_type[NFIELDS];

     sfp_type[0] = H5T_NATIVE_CHAR;
     sfp_type[1] = H5T_NATIVE_SHOR;
     sfp_type[2] = H5T_NATIVE_DOUBLE;
     sfp_type[3] = H5T_NATIVE_DOUBLE;
#endif
/*
 * check number of SFP records
 */
     if (nr_sfp == 0) return;
/*
 * open/create group /GADS
 */
     gads_id = NADC_OPEN_HDF5_Group(fid, "/GADS");
     if (gads_id < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, "/GADS");
/*
 * create table
 */
     (void) H5TBmake_table("sfp", gads_id, "SLIT_FUNCTION", 
			   NFIELDS, nr_sfp, sfp_size, sfp_names, sfp_offs, 
			   sfp_type, nr_sfp, NULL, compress, sfp);
/*
 * close interface
 */
     (void) H5Gclose(gads_id);
}
