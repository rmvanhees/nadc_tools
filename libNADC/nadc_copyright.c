/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 1999 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_CopyRight
.AUTHOR      R.M. van Hees
.KEYWORDS    NADC software
.LANGUAGE    ANSI C
.PURPOSE     display copyright
.INPUT/OUTPUT
  call as   NADC_CopyRight( stream );

     input:  
             FILE *stream :  open stream to write copyright info

.RETURNS     nothing, function exits with EXIT_SUCCESS
.COMMENTS    none
.ENVIRONment none
.VERSION     3.2     26-Nov-2002   modified copyright, added disclaimer, RvH
             3.1     25-Oct-2001   rename modules to NADC_..., RvH
             3.0     22-Aug-2001   only show copyright and exit, RvH
             2.0     13-May-2001   Removed HDF5 dependence, RvH
             1.0     30-Jul-1999   Created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++ Macros +++++*/
#define DISCLAIMER \
"We at SRON have developed nadc_tools as part of our commitment\n\
towards the verification and further improvement of the calibration of\n\
Sciamachy data. This software is developed for in-house usage, and\n\
generously shared with you WITHOUT ANY WARRANTY. We will try to help\n\
you with any problems, but only on a ``best effort'' basis.\n\n\
Note that this software package is also not supported in any way by\n\
ESA or DLR, although the nadc_tools extractors and libraries mimic\n\
some of the functionality of the gdp and EnviView toolbox.\n\n\
The software package is in no way intended to replace the official data\n\
processor and should not be treated as such. Especially it is not meant\n\
to (and cannot) produce official data products. These have to be derived\n\
with ESA approved tools such as EnviView that can be downloaded for free\n\
from ESA\n\n\
It is your own responsibility to verify the results you produce with\n\
this package against official data products. If you find discrepancies,\n\
please inform SRON (not ESA or DLR) at once. Any information you give\n\
will help us to improve our software. We will relay to ESA all\n\
information to improve the official data processor.\n"

#define GPL_COPY \
"Copyright (C) 1999 - 2013 by Space Research Organization Netherlands\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."

#define BUG_ADDRESS  "R.M.van.Hees@sron.nl"

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void NADC_CopyRight( FILE *stream )
{
     (void) fprintf( stream, "\n%s\n", GPL_COPY );
     (void) fprintf( stream, "\nDisclaimer:\n%s\n", DISCLAIMER );
     (void) fprintf( stream, "\tBug report/questions, e-mail: %s\n", 
		     BUG_ADDRESS );
}
