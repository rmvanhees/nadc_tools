/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2008 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   ADAUC_VERSION
.AUTHOR      R.M. van Hees
.KEYWORDS    NADC software
.LANGUAGE    ANSI C
.PURPOSE     Returns the library version numbers through arguments.
.COMMENTS    contains ADAGUCget_Version and ADAGUCshow_Version
.ENVIRONment none
.VERSION     1.0     24-Nov-2008   Created by R. M. van Hees
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
#include <nadc_common.h>
#include "VERSION"
#include "../VERSION"

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   ADAGUCshow_Version
.PURPOSE     display version of SCIA library and release date
.INPUT/OUTPUT
  call as    ADAGUCshow_Version( stream, prognm );
    output:
             FILE *stream  :   file pointer
	     char *prognm  :   name of the calling program

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void ADAGUCshow_Version( FILE *stream, const char prognm[] )
{
     (void) fprintf( stream, "%s (version %-d.%-d.%-d) is linked with:\n", 
		     prognm,
		     nadc_vers_major, nadc_vers_minor, nadc_vers_release );
     (void) fprintf( stream,
		     "\tlibCOMMON version %-d.%-d.%-d\n", 
		     version_major, version_minor, version_release );
     NADC_CopyRight( stream );
}
