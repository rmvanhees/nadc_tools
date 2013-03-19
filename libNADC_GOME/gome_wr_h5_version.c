/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2003 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GOME_WR_H5_VERSION
.AUTHOR      R.M. van Hees
.KEYWORDS    
.LANGUAGE    ANSI C
.PURPOSE     add version of the software to a HDF5 file

.INPUT/OUTPUT
  call as   GOME_WR_H5_VERSION( file_id );

     input:  
             hid_id file_id : HDF5 file identifier

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
#define _GOME_COMMON
#include <nadc_gome.h>
#include "VERSION"
#include "../VERSION"

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_WR_H5_VERSION( hid_t file_id )
{
     char    cbuff[SHORT_STRING_LENGTH];
/*
 * write software versions to HDF5 file
 */
     (void) snprintf( cbuff, SHORT_STRING_LENGTH, "version %-d.%-d.%-d", 
		      nadc_vers_major, nadc_vers_minor, nadc_vers_release );
     (void) H5LTset_attribute_string( file_id, "/", "nadc_tools", cbuff );
     
     (void) snprintf( cbuff, SHORT_STRING_LENGTH, "version %-d.%-d.%-d",
                      version_major, version_minor, version_release );
     (void) H5LTset_attribute_string( file_id, "/", "libnadc_gome", cbuff );
}
