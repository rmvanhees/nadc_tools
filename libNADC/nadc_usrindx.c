/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 1999 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_USRINDX
.AUTHOR      R.M. van Hees
.KEYWORDS    Command-line options
.LANGUAGE    ANSI C
.PURPOSE     extent NADC_USRINP: interpret negative indices relative to 
             maximum index, and '*' as the maximum index
.INPUT/OUTPUT
  call as    nr_indx = NADC_USRINDX( str_range, max_indx, indices );
     input:
            char str_range   :  string holding range specification
	    int  max_indx    :  maximum index
    output:
            short *indices   :  array holding indices

.RETURNS     number of indices (short)
.COMMENTS    none
.ENVIRONment none
.VERSION     2.0     31-Oct-2001   moved to new Error handling routines, RvH
             1.1     25-Oct-2001   renamed module, RvH
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
#include <string.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
short NADC_USRINDX( const char str_range[], int max_indx, short *indices )
{
     const char prognm[] = "NADC_USRINDX";

     register int nr;

     char    *cpntr, cbuff[MAX_STRING_LENGTH];
     int     num;
     size_t  nchar;

     if ( (cpntr = strchr( str_range, '*' )) == NULL ) {
	  (void) NADC_USRINP( INT16_T, str_range, max_indx, indices, &num );
     } else {
	  nchar = cpntr - str_range;
	  (void) strlcpy( cbuff, str_range, nchar );
	  (void) sprintf( cbuff, "%s%-d%s", cbuff, max_indx-1, cpntr+1 );
	  (void) NADC_USRINP( INT16_T, cbuff, max_indx, indices, &num );

     }
     for ( nr = 0; nr < num; nr++ ) {
	  if ( indices[nr] < 0 ) indices[nr] += (short) max_indx;
	  if ( indices[nr] < 0 || indices[nr] >= (short) max_indx ) {
	       num = nr;
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, "indices" );
	  }
     }
 done:
     return ((short) num);
}
