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

.IDENTifer   SCIA_LV1_WR_H5_PSP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 PSP data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_PSPN(nr_psp, pspn);
             SCIA_LV1_WR_H5_PSPL(nr_psp, psplo);
             SCIA_LV1_WR_H5_PSPO(nr_psp, psplo);
     input:  
	     unsigned int        nr_psp: number of nadir PSP's
	     struct pspn_scia  *pspn : Polarisation Sensitivity Parameters
                                        (nadir)
	     struct psplo_scia *psplo: Polarisation Sensitivity Parameters
                                        (limb/occultation)
.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.1   07-Dec-2005 removed delaration of unused variables, RvH
	      3.0   20-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.1   03-Jan-2001	separate functions for nadir 
                                   & limb/occultation parameter sets, RvH
              1.0   19-Nov-1999 created by R. M. van Hees
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

/*+++++++++++++++++++++++++ Main Program or Functions +++++++++++++++*/
void SCIA_LV1_WR_H5_PSPN(unsigned int nr_psp, const struct pspn_scia *pspn)
{
     hid_t   fid, gads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   pspn_type[] = { H5T_NATIVE_FLOAT, -1, -1 };

     const hbool_t compress = \
	  (nadc_get_param_uint8("flag_deflate") == PARAM_SET) ? TRUE : FALSE;

     const char *pspn_names[] = { "ang_esm", "mu2", "mu3" };
     const size_t pspn_size = sizeof(struct pspn_scia);
     const size_t pspn_offs[] = {
	  HOFFSET(struct pspn_scia, ang_esm),
	  HOFFSET(struct pspn_scia, mu2),
	  HOFFSET(struct pspn_scia, mu3)
     };
/*
 * check number of PSPN records
 */
     if (nr_psp == 0) return;
/*
 * open/create group /GADS
 */
     fid = nadc_get_param_hid("hdf_file_id");
     gads_id = NADC_OPEN_HDF5_Group(fid, "/GADS");
     if (gads_id < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, "/GADS");
/*
 * write PSPN data sets
 */
     adim = SCIENCE_PIXELS;
     pspn_type[1] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, &adim);
     pspn_type[2] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, &adim);

     stat = H5TBmake_table("pspn", gads_id, "POL_SENS_NADIR", 
                            3, nr_psp, pspn_size, pspn_names,
                            pspn_offs, pspn_type, 1,
                            NULL, compress, pspn);
     if (stat < 0) NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "pspn");
/*
 * close interface
 */
done:
     (void) H5Tclose(pspn_type[1]);
     (void) H5Tclose(pspn_type[2]);
     (void) H5Gclose(gads_id);
}

void SCIA_LV1_WR_H5_PSPL(unsigned int nr_psp, const struct psplo_scia *pspl)
{
     hid_t   fid, gads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   pspl_type[] = { H5T_NATIVE_FLOAT, H5T_NATIVE_FLOAT, -1, -1 };

     const hbool_t compress = \
	  (nadc_get_param_uint8("flag_deflate") == PARAM_SET) ? TRUE : FALSE;

     const char *pspl_names[] = { "ang_esm", "ang_asm", "mu2", "mu3" };
     const size_t pspl_size = sizeof(struct psplo_scia);
     const size_t pspl_offs[] = {
	  HOFFSET(struct psplo_scia, ang_esm),
	  HOFFSET(struct psplo_scia, ang_asm),
	  HOFFSET(struct psplo_scia, mu2),
	  HOFFSET(struct psplo_scia, mu3)
     };
/*
 * check number of PSPL records
 */
     if (nr_psp == 0) return;
/*
 * open/create group /GADS
 */
     fid = nadc_get_param_hid("hdf_file_id");
     gads_id = NADC_OPEN_HDF5_Group(fid, "/GADS");
     if (gads_id < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, "/GADS");
/*
 * write PSPL data sets
 */
     adim = SCIENCE_PIXELS;
     pspl_type[2] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, &adim);
     pspl_type[3] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, &adim);

     stat = H5TBmake_table("pspl", gads_id, "POL_SENS_LIMB",
                            4, nr_psp, pspl_size, pspl_names,
                            pspl_offs, pspl_type, 1,
                            NULL, compress, pspl);
     if (stat < 0) NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "pspl");
/*
 * close interface
 */
 done:
     (void) H5Tclose(pspl_type[2]);
     (void) H5Tclose(pspl_type[3]);
     (void) H5Gclose(gads_id);
}

void SCIA_LV1_WR_H5_PSPO(unsigned int nr_psp, const struct psplo_scia *pspo)
{
     hid_t   fid, gads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   pspl_type[] = { H5T_NATIVE_FLOAT, H5T_NATIVE_FLOAT, -1, -1 };

     const hbool_t compress =
	  (nadc_get_param_uint8("flag_deflate") == PARAM_SET) ? TRUE : FALSE;

     const char *pspl_names[] = { "ang_esm", "ang_asm", "mu2", "mu3" };
     const size_t pspl_size = sizeof(struct psplo_scia);
     const size_t pspl_offs[] = {
	  HOFFSET(struct psplo_scia, ang_esm),
	  HOFFSET(struct psplo_scia, ang_asm),
	  HOFFSET(struct psplo_scia, mu2),
	  HOFFSET(struct psplo_scia, mu3)
     };
/*
 * check number of PSPO records
 */
     if (nr_psp == 0) return;
/*
 * open/create group /GADS
 */
     fid = nadc_get_param_hid("hdf_file_id");
     gads_id = NADC_OPEN_HDF5_Group(fid, "/GADS");
     if (gads_id < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, "/GADS");
/*
 * write PSPO data sets
 */
     adim = SCIENCE_PIXELS;
     pspl_type[2] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, &adim);
     pspl_type[3] = H5Tarray_create(H5T_NATIVE_DOUBLE, 1, &adim);

     stat = H5TBmake_table("pspo", gads_id, "POL_SENS_OCC",
                            4, nr_psp, pspl_size, pspl_names,
                            pspl_offs, pspl_type, 1,
                            NULL, compress, pspo);
     if (stat < 0) NADC_GOTO_ERROR(NADC_ERR_HDF_DATA, "pspo");
/*
 * close interface
 */
 done:
     (void) H5Tclose(pspl_type[2]);
     (void) H5Tclose(pspl_type[3]);
     (void) H5Gclose(gads_id);
}
