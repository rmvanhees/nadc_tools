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

.IDENTifer   NADC_STRING
.AUTHOR      R.M. van Hees
.KEYWORDS    string functions
.LANGUAGE    ANSI C
.COMMENTS    Compatible with *BSD
             contains: strlcpy, strlcat, nadc_rstrip
.ENVIRONment None
.VERSION     2.0     03-Nov-2013   renamed function to avoid name conflicts, RvH
             1.0     10-Jul-2003   originate from the Linux kernel source
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   nadc_strlcpy
.PURPOSE     copy a NULL-terminated string into a sized buffer
.INPUT/OUTPUT
  call as   nchar = nadc_strlcpy( dest, src, size );

     input:  
             char   *src  :   where to copy the string from
	     size_t size  :   size of destination buffer
    output:  
             char   *dest :   where to copy the string to

.RETURNS     length of the destination string
.COMMENTS    Compatible with *BSD: the result is always a valid
             NULL-terminated string that fits in the buffer (unless,
             of course, the buffer size is zero). It does not pad
             out the result like strncpy() does.
-------------------------*/
size_t nadc_strlcpy( char *dest, /*@unique@*/ const char *src, size_t size )
{
     size_t ret = strlen( src );

     if ( size ) {
	  size_t len = (ret >= size) ? size-1 : ret;
	  (void) memcpy( dest, src, len );
	  dest[len] = '\0';
     }
     return ret;
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_strlcat
.PURPOSE     append a length-limited, NULL-terminated string to another
.INPUT/OUTPUT
  call as   nchar = nadc_strlcat( dest, src, size );

     input:  
             char   *src  :   where to copy the string from
	     size_t size  :   size of destination buffer
    output:  
             char   *dest :   where to copy the string to

.RETURNS     length of the destination string
.COMMENTS    Compatible with *BSD: the result is always a valid
             NULL-terminated string that fits in the buffer (unless,
             of course, the buffer size is zero). It does not pad
             out the result like strncpy() does.
-------------------------*/
size_t nadc_strlcat( char *dest, const char *src, size_t count )
{
     const char prognm[] = "nadc_strlcat";

     size_t dsize = strlen( dest );
     size_t len = strlen( src );
     size_t res = dsize + len;

/* This would be a bug */
     if ( dsize >= count ) {	  
	  NADC_GOTO_ERROR( prognm, 
			   NADC_ERR_FATAL, "strlen( dest ) >= count" );
     }
     dest += dsize;
     count -= dsize;
     if ( len >= count ) len = count-1;
     memcpy( dest, src, len );
     dest[len] = '\0';
 done:
     return res;
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_rstrip
.PURPOSE     return copy of string with trailing whitespace removed
.INPUT/OUTPUT
  call as   nadc_rstrip( dest, src );
     input:  
             char   *src  :   where to copy the string from
    output:  
             char   *dest :   where to copy the string to

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void nadc_rstrip( /*@out@*/ char *dest, const char *src )
{
     size_t len = strlen( src );

     (void) strcpy( dest, src );
     while( --len > 0 && isspace((int) dest[len]) ) dest[len] = '\0';    
}
