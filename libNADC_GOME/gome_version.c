/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2013 SRON (R.M.van.Hees@sron.nl)

   This is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License, version 2, as
   Free Software Foundation; either version 2.

   The software is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA  02111-1307, USA.

.IDENTifer   GOME_VERSION
.AUTHOR      R.M. van Hees
.KEYWORDS    NL_SCIA_DC software
.LANGUAGE    ANSI C
.PURPOSE     Returns the library version numbers through arguments.
.COMMENTS    contains GOME_SHOW_VERSION
.ENVIRONment none
.VERSION     3.0     14-Mar-2013   Renamed to GOME_SHOW_VERSION, RvH
             2.0     14-Nov-2001   Added GOMEshow_version, RvH
             1.0     22-Aug-2001   Created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#include <nadc_gome.h>
#include "../VERSION"
#include "VERSION"

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   GOME_SHOW_VERSION
.PURPOSE     display version of GOME library
.INPUT/OUTPUT
  call as    GOME_SHOW_VERSION( stream, prognm );
    output:
             FILE *stream  :   file pointer
	     char *prognm  :   name of the calling program

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void GOME_SHOW_VERSION( FILE *stream, const char *prognm  )
{
     (void) fprintf( stream, 
		     "%s (version %-d.%-d release %-d) is linked with:\n", 
		     prognm,
		     nadc_vers_major, nadc_vers_minor, nadc_vers_release );
     (void) fprintf( stream,
		     "\tlibnadc_gome version %-d.%-d release %-d\n", 
		     version_major, version_minor, version_release );
     NADC_CopyRight( stream );
}
