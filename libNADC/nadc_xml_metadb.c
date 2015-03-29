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

.IDENTifer   NADC_GET_XML_METADB
.AUTHOR      R.M. van Hees
.KEYWORDS    NADC XML SQL
.LANGUAGE    ANSI C
.PURPOSE     obtain host, user, passwd values from NADC configuration file
.INPUT/OUTPUT
  call as   NADC_GET_XML_METADB( fp, host, user, passwd );
     input:  
             FILE *fp      :  filename of the NADC configuration file
    output:  
             char *host    :  hostname with the PostgreSQL server
	     char *port    :  port to connect
	     char *user    :  username of account
	     char *passwd  :  password of account

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     14-Feb-2007   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#define _SCIA_COMMON
#include <nadc_common.h>

/*+++++ Macros +++++*/
#define MAX_LINE_LENGTH 256

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static void NADC_STRIP_ALL( const char *str_in, char *str_out )
{
     size_t ni = 0, nj = 0;

     while ( str_in[ni] != '\0' && str_in[ni] != '\n' ) {
	  if ( str_in[ni] != ' ' && str_in[ni] != '\t' && str_in[ni] != '"' ) {
	       str_out[nj++] = str_in[ni];
	  }
	  ni++;
     }
     str_out[nj] = '\0';
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void NADC_GET_XML_METADB( FILE *fp, char *host, char *port, 
			  char *user, char *passwd )
{
     char   line[MAX_LINE_LENGTH], line_strip[MAX_LINE_LENGTH];
     char   keyword[MAX_LINE_LENGTH];
     char   *sep;

     unsigned int nchar;
/*
 * initialize
 */
     *host = *user = *passwd = '\0'; 
/*
 * read until tag "metatables"
 */
     do {
          if ( fgets( line, MAX_LINE_LENGTH-1, fp ) == NULL )
               NADC_RETURN_ERROR( NADC_ERR_FILE_RD,
				  "error reading nadc.config.xml" );
     } while (strstr( line, "metatables" ) == NULL );
/*
 * read until tag "server"
 */
     do {
          if ( fgets( line, MAX_LINE_LENGTH-1, fp ) == NULL )
               NADC_RETURN_ERROR( NADC_ERR_FILE_RD,
				  "error reading nadc.config.xml" );
     } while (strstr( line, "<server" ) == NULL );
/*
 * read value for "host", "user", "passwd"
 */
     do {
          if ( fgets( line, MAX_LINE_LENGTH-1, fp ) == NULL )
               NADC_RETURN_ERROR( NADC_ERR_FILE_RD,
				  "error reading nadc.config.xml" );
	  NADC_STRIP_ALL( line, line_strip );

	  if ( (sep = strchr( line_strip, '=' )) != NULL ) {
	       nchar = min_t(size_t, MAX_LINE_LENGTH, (sep - line_strip + 1));
	       (void) nadc_strlcpy( keyword, line_strip, nchar );
	       if ( strncmp( keyword, "host", 4 ) == 0 )
		    (void) strcpy( host, ++sep );
	       else if ( strncmp( keyword, "port", 4 ) == 0 )
		    (void) strcpy( port, ++sep );
	       else if ( strncmp( keyword, "user", 4 ) == 0 )
		    (void) strcpy( user, ++sep );
	       else if ( strncmp( keyword, "passwd", 6 ) == 0 )
		    (void) strcpy( passwd, ++sep );
	  }
     } while (strstr( line, "/>" ) == NULL );
}
