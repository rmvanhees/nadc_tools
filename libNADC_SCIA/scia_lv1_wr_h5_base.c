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

.IDENTifer   SCIA_LV1_WR_H5_BASE
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 SPECTRAL_BASE data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_BASE(base);
     input:  
	     struct base_scia *base    : Spectral Base Parameters

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   20-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.1   31-Aug-2001	corrected the dimension of "dims", RvH 
              1.0   18-Dec-2000 created by R. M. van Hees
------------------------------------------------------------*/
/* * Define _POSIX_SOURCE to indicate
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

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_BASE(const struct base_scia *base)
{
     hid_t   fid, gads_id;
     hsize_t adim;
/*
 * open/create group /GADS
 */
     fid = nadc_get_param_hid("hdf_file_id");
     gads_id = NADC_OPEN_HDF5_Group(fid, "/GADS");
     if (gads_id < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, "/GADS");
/*
 * write datasets
 */
     adim = SCIENCE_PIXELS;
     (void) H5LTmake_dataset_float(gads_id, "SPECTRAL_BASE", 1, &adim, 
				   base->wvlen_det_pix);
/*
 * close interface
 */
     (void) H5Gclose(gads_id);
}
