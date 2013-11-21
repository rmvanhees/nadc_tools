/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   ENVI_RD_PDS_INFO
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY PDS data
.LANGUAGE    ANSI C
.PURPOSE     read one SCIAMACHY header-line, 
             split line in keyword and keyvalue
.INPUT/OUTPUT
  call as   nbyte = ENVI_RD_PDS_INFO( fd, keyword, keyvalue );

     input:  
            FILE *fd        :  open stream pointer
    output:  
            char *keyword   :  string with keyword name
            char *keyvalue  :  string with keyword value

.RETURNS     number of bytes read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.EXTERNALs   NADC_GOTO_ERROR
.VERSION      2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.2   10-Oct-2001	added list of external (NADC) function, RvH 
              1.1   05-Sep-2001 moved define-statements to include-file, RvH
              1.0   22-Aug-2001 Created by R. M. van Hees
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
unsigned int ENVI_RD_PDS_INFO( FILE *fd, /*@out@*/ char *keyword, 
			       /*@out@*/ char *keyvalue )
{
     const char prognm[] = "ENVI_RD_PDS_INFO";

     char         *sep, *newline;
     char         line[PDS_ASCII_HDR_LENGTH];
     unsigned int nbyte = 0u;
     size_t       nchar;
/*
 * initialize
 */
     (void) strcpy( keyword, "" );
     (void) strcpy( keyvalue, "" );
/*
 * read line including '\n'
 */
     do { 
	  if ( fgets( line, PDS_ASCII_HDR_LENGTH-1, fd ) == NULL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, 
				"error reading PDS header" );

	  nbyte = (unsigned int) strlen( line );
     } while ((sep = strchr( line, '=' )) == NULL );
/*
 * check if we found a newline
 */
     if ( (newline = strchr( line, '\n' )) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, 
			   "no newline: corrupted input?!?" );
/*
 * get keyword name
 */
     nchar = min_t( size_t, PDS_KEYWORD_LENGTH, (sep - line + 1));
     (void) nadc_strlcpy( keyword, line, nchar );
/*
 * get keyword value (including double-quotes)
 */
     ++sep; 
     nchar = min_t( size_t, PDS_KEYVAL_LENGTH, (newline - sep + 1));
     (void) nadc_strlcpy( keyvalue, sep, nchar );

 done:
     return nbyte;
}
