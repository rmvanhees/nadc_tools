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

.IDENTifer   SCIA_CodePatch
.AUTHOR      R.M. van Hees
.KEYWORDS    patch option encoding/decoding
.LANGUAGE    ANSI C
.PURPOSE     Sciamachy level 1b patch parameter coding
.COMMENTS    Contains SCIA_SET_PATCH, SCIA_GET_PATH, SCIA_SHOW_PATH
.ENVIRONment None
.VERSION      2.1   22-Nov-2010	modified patch options, RvH
              2.0   31-Jan-2005	complete rewrite, and combined functionality
                                nadc_encodepatch.c/nadc_decodepatch.c, RvH
              1.1   30-Mar-2004	added option to patch "SPECTRAL BASE", RvH
              1.0   04-Nov-2003	created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#include <nadc_scia.h>

/*+++++ Static Variables +++++*/
static 
const struct patch_opt {
     const char  key[2];
     const short nr_sub;
     const unsigned short val;
     const char  *desc;
} patch_opts[] = {
     { "0", 2, (SCIA_PATCH_MEM|SCIA_PATCH_NLIN), 
       "Patch both memory and non-linearity correction values" },
     { "m", 0, SCIA_PATCH_MEM, "Patch memory correction (chan 1-5)" },
     { "l", 0, SCIA_PATCH_NLIN, "Patch non-linearity correction (chan 6-8)" },
     { "1", 0, SCIA_PATCH_DARK, "Patch darkcurrent calibration keydata" },
     { "2", 0, SCIA_PATCH_PPG, "Patch PPG calibration keydata" },
     { "4", 0, SCIA_PATCH_STRAY, "Patch Straylight correction values (ch2)" },
     { "5", 0, SCIA_PATCH_BASE, "Patch wavelength calibration keydata" },
     { "7", 0, SCIA_PATCH_RAD, "Patch RSP, PSP and SRS keydata" },
     { "9", 0, SCIA_PATCH_BDPM, "Patch Bad/Dead pixel mask" }
};

static
const int NumOptions = sizeof( patch_opts ) / sizeof( struct patch_opt );

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_SET_PATCH
.PURPOSE     extract patchration options from command-line string
.INPUT/OUTPUT
   call as:  SCIA_SET_PATCH( patch_str, &patch_mask );
     input:
            char patch_str[] :           string with encoded patch steps
   output:
            unsigned short *patch_mask :   encoded patch steps

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void SCIA_SET_PATCH( const char patch_str[], unsigned short *patch_mask )
{
     register short ns;
     register int   nr = 0;

     char *cpntr = NULL;

     *patch_mask = SCIA_PATCH_NONE;
/*
 * first check the easy declarations
 */
     if ( strlen( patch_str ) == 0 || strncmp( patch_str, "none", 4 ) == 0 )
	  return;

     if ( strncmp( patch_str, "all", 3 ) == 0 ) {
	  *patch_mask = SCIA_PATCH_ALL;
	  return;
     }
/*
 * loop over all patch options, make sure we jump over all sub-options
 */
     while ( nr < NumOptions ) {
	  if ( (cpntr = strstr( patch_str, patch_opts[nr].key )) != NULL ) {
	       if ( cpntr[1] == '\0' || cpntr[1] == ',' ) { /* no sub-options */
		    *patch_mask = ((*patch_mask) | patch_opts[nr].val);
	       } else {                                  /* check sub-options */
		    while ( cpntr[1] != '\0' && cpntr[1] != ',' ) {
			 ++cpntr;
			 for ( ns = 0; ns <= patch_opts[nr].nr_sub; ns++ ) {
			      if ( *cpntr == *patch_opts[nr+ns].key )
				   *patch_mask = 
					((*patch_mask)| patch_opts[nr+ns].val);
			 }
		    }
		    if ( ((*patch_mask) & patch_opts[nr].val) == USHRT_ZERO )
			 *patch_mask = ((*patch_mask) | patch_opts[nr].val);
	       }
	  }
	  nr += (patch_opts[nr].nr_sub + 1);
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_GET_PATCH
.PURPOSE     return string with used patch options
.INPUT/OUTPUT
   call as:  SCIA_GET_PATCH( patch_val, patch_str );
     input:
            unsigned short patch_val :   encoded patch steps
    output:  
            char patch_str[] :           string with encoded patch steps

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void SCIA_GET_PATCH( unsigned short patch_val, char patch_str[] )
{
     register short ns;
     register int   nr;

     (void) strcpy( patch_str, "" );
     for ( nr = 0; nr < NumOptions; nr++ ) {
	  const short nr_sub = patch_opts[nr].nr_sub;

	  if ( (patch_val & patch_opts[nr].val) != USHRT_ZERO ) {
	       if ( *patch_str != '\0' ) (void) strcat( patch_str, "," );
	       (void) strcat( patch_str, patch_opts[nr].key );
	       for ( ns = 0; ns < nr_sub; ns++ ) {
		    if ( (patch_val & patch_opts[++nr].val) != USHRT_ZERO )
			 (void) strcat( patch_str, patch_opts[nr].key );
	       }
	  } else
	       nr += nr_sub;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_SHOW_PATCH
.PURPOSE     show all implemented patch options
.INPUT/OUTPUT
  call as:   SCIA_SHOW_PATCH( FILE *stream );
    input:
            FILE stream :   stream to show available patch options

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void SCIA_SHOW_PATCH( FILE *stream )
{
     register short ns;
     register int   nr;

     for ( nr = 0; nr < NumOptions; nr++ ) {
	  const short nr_sub = patch_opts[nr].nr_sub;

	  (void) fprintf( stream, "   %s:\t%s\n", patch_opts[nr].key, 
			  patch_opts[nr].desc );
	  for ( ns = 0; ns < nr_sub; ns++ ) {
	       ++nr;
	       (void) fprintf( stream, "     %s:\t%s\n", 
			       patch_opts[nr].key, patch_opts[nr].desc );
	  }
     }
}
