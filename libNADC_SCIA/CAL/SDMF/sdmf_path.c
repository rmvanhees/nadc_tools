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

.IDENTifer   SDMF_PATH
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF 
.LANGUAGE    ANSI C
.PURPOSE     obtain path to SDMF directory for given version
.COMMENTS    None
.ENVIRONment None
.VERSION      1.1   17-Sept-2015 added path to version v3.2, RvH
              1.0   20-May-2012 initial release by R. M. van Hees
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
char *SDMF_PATH(const char version[])
{
     static char sdmf24_path[80] = "/SCIA/sdmf/2.4.1";
     static char sdmf30_path[80] = "/SCIA/sdmf/3.0";
     static char sdmf31_path[80] = "/SCIA/sdmf/3.1";
     static char sdmf32_path[80] = "/SCIA/sdmf/3.2";
     static char no_path[80] = "";

     static bool init = TRUE;

     if (init) {
	  char *env24_str = getenv("SDMF24_PATH");
	  char *env30_str = getenv("SDMF30_PATH");
	  char *env31_str = getenv("SDMF31_PATH");
	  char *env32_str = getenv("SDMF32_PATH");

	  if (env24_str != NULL) {
	       (void) nadc_strlcpy(sdmf24_path, env24_str, 80);
	  }
	  if (env30_str != NULL) {
	       (void) nadc_strlcpy(sdmf30_path, env30_str, 80);
	  }
	  if (env31_str != NULL) {
	       (void) nadc_strlcpy(sdmf31_path, env31_str, 80);
	  }
	  if (env32_str != NULL) {
	       (void) nadc_strlcpy(sdmf32_path, env32_str, 80);
	  }
	  init = FALSE;
     }
     if (strncmp(version, "2.4", 3) == 0)
	  return sdmf24_path;
     else if (strncmp(version, "3.0", 3) == 0)
	  return sdmf30_path;
     else if (strncmp(version, "3.1", 3) == 0)
	  return sdmf31_path;
     else if (strncmp(version, "3.2", 3) == 0)
	  return sdmf32_path;
     else
	  return no_path;
}
    
/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
#ifdef TEST_PROG
bool Use_Extern_Alloc = FALSE;

int main(int argc, char *argv[])
{
     (void) printf("%s\n", SDMF_PATH("1.0"));
     (void) printf("%s\n", SDMF_PATH("2.4"));
     (void) printf("%s\n", SDMF_PATH("3.0"));
     (void) printf("%s\n", SDMF_PATH("3.1"));
     (void) printf("%s\n", SDMF_PATH("3.2"));
     (void) printf("%s\n", SDMF_PATH("3.3"));

     (void) printf("%s\n", SDMF_PATH("1.0"));
     (void) printf("%s\n", SDMF_PATH("2.4"));
     (void) printf("%s\n", SDMF_PATH("3.0"));
     (void) printf("%s\n", SDMF_PATH("3.1"));
     (void) printf("%s\n", SDMF_PATH("3.2"));
     (void) printf("%s\n", SDMF_PATH("3.3"));

     exit(EXIT_SUCCESS);
}
#endif
