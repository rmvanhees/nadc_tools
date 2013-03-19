/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2007 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_RECEIVEDATE
.AUTHOR      R.M. van Hees
.KEYWORDS    
.LANGUAGE    ANSI C
.PURPOSE     obtain modification date from operating system
.INPUT/OUTPUT
  call as   NADC_RECEIVEDATE( flname, receivedate );

     input:  
             char *flname      :  name of file
	     char *receivedate :  receive date (UTC)

.RETURNS     nothing
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     10-Apr-2008   returns UTC time instead of localtime, RvH
             1.0     29-Jan-2007   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void NADC_RECEIVEDATE( const char *flname, char *receivedate )
{
     struct stat  flstat;

     (void) stat( flname, &flstat );
     (void) strftime( receivedate, SHORT_STRING_LENGTH, 
		      "%F %T", gmtime( &flstat.st_mtime ) );
}
