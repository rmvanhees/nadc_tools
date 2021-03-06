/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2003 - 2019 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_WR_H5_VERSION
.AUTHOR      R.M. van Hees
.KEYWORDS    
.LANGUAGE    ANSI C
.PURPOSE     add version of the software to a HDF5 file

.INPUT/OUTPUT
  call as   SCIA_WR_H5_VERSION();

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION     2.0   24-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
             1.0   25-Mar-2003 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <string.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_COMMON
#include <nadc_scia.h>
#include "VERSION"
#include "../VERSION"

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_WR_H5_VERSION(void)
{
     char   cbuff[SHORT_STRING_LENGTH];
     hid_t  fid = nadc_get_param_hid("hdf_file_id");
/*
 * write software versions to HDF5 file
 */
     (void) snprintf(cbuff, SHORT_STRING_LENGTH, "version %-d.%-d.%-d", 
		     nadc_vers_major, nadc_vers_minor, nadc_vers_release);
     (void) H5LTset_attribute_string(fid, "/", "nadc_tools", cbuff);
     
     (void) snprintf(cbuff, SHORT_STRING_LENGTH, "version %-d.%-d.%-d",
		     version_major, version_minor, version_release);
     (void) H5LTset_attribute_string(fid, "/", "libnadc_scia", cbuff);
}
