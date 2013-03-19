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

.IDENTifer   GOME_CodeCalib
.AUTHOR      R.M. van Hees
.KEYWORDS    calibration option encoding/decoding
.LANGUAGE    ANSI C
.PURPOSE     GOME level 1b calibration parameter coding
.COMMENTS    Contains GOME_SET_CALIB, GOME_GET_CALIB, GOME_SHOW_CALIB
.ENVIRONment None
.VERSION      2.1   08-Mar-2013	renamed and added to GOME modules, RvH
              2.0   31-Jan-2005	complete rewrite and combined functionality
                                nadc_encodecalib.c/nadc_decodecalib.c, RvH
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
#include <nadc_gome.h>

/*+++++ Static Variables +++++*/
static 
const struct calib_opt {
     const char key[2];
     const unsigned short val;
     const char *desc;
} calib_opts[] = {
     { "L", GOME_CAL_LEAK, "correct for fixed-pattern noise and leakage" },
     { "A", GOME_CAL_FPA, "correct band 1a for cross-talk" },
     { "F", GOME_CAL_FIXED, "correct signals for pixel-to-pixel variations" },
     { "S", GOME_CAL_STRAY, "correct signals for stray-light" },
     { "N", GOME_CAL_NORM, "normalise signals to 1 second integration time" },
     { "P", GOME_CAL_POLAR, "Polarisation sensitivity correction" },
     { "I", GOME_CAL_INTENS, "absolute radiance calibration " },
     { "B", GOME_CAL_BSDF, "divide signals from Sun measurements by BSDF" },
     { "a", GOME_CAL_ALBEDO, "convert intensity to albedo" },
     { "j", GOME_CAL_JUMPS, "not implemented" },
     { "d", GOME_CAL_AGING, "not implemented" },
     { "U", GOME_CAL_UNIT, "unit conversion to photons/(s~cm$^{2}$sr~nm)" }
};

static
const int NumOptions = sizeof( calib_opts ) / sizeof( struct calib_opt );

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   GOME_SET_CALIB
.PURPOSE     convert string with calibration options to mask
.INPUT/OUTPUT
   call as:  GOME_SET_CALIB( calib_str, &calib_mask );
     input:
            char calib_str[] :           string with encoded calibration steps
   output:
            unsigned short *calib_mask :   encoded calibration steps
.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void GOME_SET_CALIB( const char calib_str[], unsigned short *calib_mask )
{
     register int nr;

     *calib_mask = CALIB_NONE;
/*
 * first check the easy declarations
 */
     if ( strlen( calib_str ) == 0 || strncmp( calib_str, "none", 4 ) == 0 )
	  return;
/*
 * check every available calibration option
 */
     for ( nr = 0; nr < NumOptions; nr++ ) {
	  if ( strstr( calib_str, calib_opts[nr].key ) != NULL )
	       *calib_mask = ((*calib_mask) | calib_opts[nr].val);
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   GOME_GET_CALIB
.PURPOSE     convert calibration mask to string
.INPUT/OUTPUT
   call as:  GOME_GET_CALIB( calib_val, calib_str );
     input:
            unsigned short calib_val :   encoded calibration steps
    output:  
            char calib_str[] :           string with encoded calibration steps
.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void GOME_GET_CALIB( unsigned short calib_val, char calib_str[] )
{
     register int nr;

     (void) strcpy( calib_str, "" );
     for ( nr = 0; nr < NumOptions; nr++ ) {
	  if ( (calib_val & calib_opts[nr].val) != USHRT_ZERO ) {
	       if ( *calib_str != '\0' ) (void) strcat( calib_str, "," );
	       (void) strcat( calib_str, calib_opts[nr].key );
	  }
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   GOME_SHOW_CALIB
.PURPOSE     convert string with calibration options to mask
.INPUT/OUTPUT
  call as:   GOME_SHOW_CALIB( FILE *stream );
    input:
            FILE stream :   stream to show available calibration options
.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void GOME_SHOW_CALIB( FILE *stream )
{
     register int nr;

     for ( nr = 0; nr < NumOptions; nr++ ) {
	  (void) fprintf( stream, "   %s:\t%s\n", calib_opts[nr].key, 
			  calib_opts[nr].desc );
     }
}
