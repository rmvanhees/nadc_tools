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
  call as    nr_indx = NADC_USRINDX(str_range, max_indx, indices);
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
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
short NADC_USRINDX(const char str_range[], int max_indx, short *indices)
{
     register int nr;

     char *cpntr;
     int num;

     if ((cpntr = strchr(str_range, '*')) == NULL) {
	  (void) NADC_USRINP(INT16_T, str_range, max_indx, indices, &num);
     } else {
	  size_t nchar = cpntr - str_range;
	  char cbuff1[MAX_STRING_LENGTH], cbuff2[MAX_STRING_LENGTH];
	  
	  (void) nadc_strlcpy(cbuff1, str_range, nchar);
	  num = snprintf(cbuff2, MAX_STRING_LENGTH,
			 "%s%-d%s", cbuff1, max_indx-1, cpntr+1);
	  if (num > (int) MAX_STRING_LENGTH)
	       NADC_ERROR(NADC_ERR_WARN, "cbuff2 truncated");
	  (void) NADC_USRINP(INT16_T, cbuff2, max_indx, indices, &num);

     }
     for (nr = 0; nr < num; nr++) {
	  if (indices[nr] < 0) indices[nr] += (short) max_indx;
	  if (indices[nr] < 0 || indices[nr] >= (short) max_indx) {
	       num = nr;
	       NADC_GOTO_ERROR(NADC_ERR_PARAM, "indices");
	  }
     }
 done:
     return ((short) num);
}

