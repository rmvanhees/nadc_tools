/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2012 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SDMF_DBPM
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF - Bad Dead Pixel Mask
.LANGUAGE    ANSI C
.PURPOSE     read Dead/Bad pixels mask from SDMF databases
.COMMENTS    contains SDMF_get_BDPM_24 and SDMF_get_BDPM_30
.ENVIRONment None
.VERSION      1.0   20-May-2012 initial release by R. M. van Hees
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
#include <nadc_sdmf.h>

/*+++++ Macros +++++*/
        /* NONE */

/*+++++ Static Variables +++++*/
        /* NONE */

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/


/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
char *SDMF_PATH( const char version[] )
{
     static char sdmf24_path[80] = "/SCIA/SDMF241";
     static char sdmf30_path[80] = "/SCIA/SDMF30";
     static char sdmf31_path[80] = "/SCIA/SDMF31";

     static bool init = TRUE;

     if ( init ) {
	  char *env24_str = getenv( "SDMF24_PATH" );
	  char *env30_str = getenv( "SDMF30_PATH" );
	  char *env31_str = getenv( "SDMF31_PATH" );

	  if ( env24_str != NULL ) {
	       (void) strlcpy( sdmf24_path, env24_str, 80 );
	  }
	  if ( env30_str != NULL ) {
	       (void) strlcpy( sdmf30_path, env30_str, 80 );
	  }
	  if ( env31_str != NULL ) {
	       (void) strlcpy( sdmf31_path, env31_str, 80 );
	  }
	  init = FALSE;
     }
     if ( strncmp( version, "2.4", 3 ) == 0 )
	  return sdmf24_path;
     else if ( strncmp( version, "3.0", 3 ) == 0 )
	  return sdmf30_path;
     else if ( strncmp( version, "3.1", 3 ) == 0 )
	  return sdmf31_path;
     else
	  return "";
}
    
/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
#ifdef TEST_PROG
bool Use_Extern_Alloc = FALSE;

int main( int argc, char *argv[] )
{
     const char prognm[] = "sdmf_path";

     (void) printf( "%s\n", SDMF_PATH("1.0") );
     (void) printf( "%s\n", SDMF_PATH("2.4") );
     (void) printf( "%s\n", SDMF_PATH("3.0") );
     (void) printf( "%s\n", SDMF_PATH("3.1") );

     (void) printf( "%s\n", SDMF_PATH("1.0") );
     (void) printf( "%s\n", SDMF_PATH("2.4") );
     (void) printf( "%s\n", SDMF_PATH("3.0") );
     (void) printf( "%s\n", SDMF_PATH("3.1") );

     exit( EXIT_SUCCESS );
}
#endif
