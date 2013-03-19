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

.IDENTifer   SCIA_CodeCalib
.AUTHOR      R.M. van Hees
.KEYWORDS    calibration option encoding/decoding
.LANGUAGE    ANSI C
.PURPOSE     SCIA level 1b calibration parameter coding
.COMMENTS    Contains SCIA_SET_CALIB, SCIA_GET_CALIB, SCIA_SHOW_CALIB
.ENVIRONment None
.VERSION      2.2   08-Jul-2007	final fix to correctly process options?, RvH
              2.1   29-Mar-2006	fixed nasty bug in NADC_SCIA_CalibMask, RvH
              2.0   31-Jan-2005	complete rewrite, and combined functionality
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
#include <nadc_scia.h>

/*+++++ Static Variables +++++*/
static 
const struct calib_opt {
     const char key[2];
     const short nr_sub;
     const unsigned int val;
     const char *desc;
} calib_opts[] = {
     { "0", 3 , (DO_CORR_VIS_MEM|DO_CORR_IR_NLIN), 
       "application of memory/non-linearity correction" },
     { "m", 0, DO_CORR_VIS_MEM, "Reticon memory correction (chan 1-5)" },
     { "l", 0, DO_CORR_IR_NLIN, "Epitaxx non-linearity correction (chan 6-8)"},
     { "+", 0, DO_SRON_MEM_NLIN, 
       "memory/non-linearity effect calculated on-the-fly\n"
       "\trequires files: \"MEMcorr.h5\" and/or \"NLcorr.h5\"" },

     { "1", 7, (DO_CORR_AO|DO_CORR_DARK|DO_CORR_VDARK),
       "application of dark signal correction" },
     { "a", 0, DO_CORR_AO, "analog offset correction (default)" },
     { "c", 0, DO_CORR_DARK, "(constant) dark current correction (default)" },
     { "v", 0, DO_CORR_VDARK, "(variable) dark current correction (default)" },
     { "s", 0, DO_CORR_VSTRAY, "solar straylight correction" },
     { "L", 0, DO_CORR_LDARK, 
       "use Limb darks to do dark correction (limb states only)" },
     { "D", 0, DO_CORR_ADARK, 
       "dark correction based on \"Dark_Average\" ADS or State Dark (SDMF)" },
     { "+", 0, DO_SRON_DARK,
       "dark correction values extracted from SRON Monitoring database" },

     { "2", 2, DO_CORR_PPG, "application of pixel-to-pixel gain correction" },
     { "+", 0, DO_SRON_PPG,
       "use PPG correction extracted from SRON Monitoring database" },
     { "f", 0, DO_FIXED_PPG,
       "use fixed PPG correction values, channel 8 only" },

     { "3", 0, DO_CORR_ETALON, "application of etalon correction" },

     { "4", 1, DO_CORR_STRAY, 
       "application of (spectral) straylight correction" },
     { "+", 0, DO_SRON_STRAY,
       "apply new straylight keydata (channel 2, only)" },

     { "5", 1, DO_CALIB_WAVE, "application of wavelength calibration" },
     { "+", 0, DO_SRON_WAVE, 
       "fix wavelength calibration (chan 6-8) for SCIA/4.x and earlier" },

     { "6", 0, DO_CORR_POL, "application of polarisation correction" },

     { "7", 1, DO_CORR_RAD, "application of radiance correction" },
     { "k", 0, DO_KEYDATA_RAD,
       "radiance correction values calculated from keydata \"key_radsens.h5\""},

     { "8", 1, DO_DIVIDE_SUN, "calculate reflectances" },
     { "+", 0, DO_SRON_SUN,
       "Solar spectrum extracted from SRON Monitoring database" },

     { "9", 1, DO_MASK_BDPM, "application of bad/dead pixel mask" },
     { "+", 0, DO_SRON_BDPM,
       "bad/dead pixel mask extracted from SRON Monitoring database" },

     { "E", 1, DO_CALC_ERROR,
       "estimate the total relative accuracy on the measured signal" },
     { "+", 0, DO_SRON_NOISE, 
       "estimate measurement noise of channel 8 readouts"},

/* KB: Apply m-factors in applicator */
     { "M", 0, DO_MFACTOR_RAD,
       "apply m-factors from directory \"m-factor_07.01\" with auxiliary files"
       "\n\t -- use environment variable SCIA_MFACTOR_DIR to change" },

     { "T", 0, DO_SRON_TRANS, "apply transmission correction on channel 8" },

     { "p", 0, DO_PATCH_L1C, "multiply radiances in L1C product" 
       "\n\t -- use environment variable SCIA_CORR_L1C" },
};

static 
const int NumOptions = (sizeof( calib_opts ) / sizeof( struct calib_opt ));

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_SET_CALIB
.PURPOSE     extract calibration options from command-line string
.INPUT/OUTPUT
   call as:  SCIA_SET_CALIB( calib_str, &calib_mask );
     input:
            char calib_str[] :         string with encoded calibration steps
   output:
            unsigned int *calib_mask :   encoded calibration steps

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void SCIA_SET_CALIB( const char calib_str[], unsigned int *calib_mask )
{
     register short ns;
     register int   nr = 0;

     char *cpntr = NULL;

     *calib_mask = CALIB_NONE;
/*
 * first check the easy declarations
 */
     if ( strlen( calib_str ) == 0 || strncmp( calib_str, "none", 4 ) == 0 )
	  return;

     if ( strncmp( calib_str, "atbd", 4 ) == 0 
	  || strncmp( calib_str, "all", 3 ) == 0 ) {
	  *calib_mask = SCIA_ATBD_CALIB;
          return;
     }
     if ( strncmp( calib_str, "sron", 4 ) == 0 ) {
	  *calib_mask = SCIA_SRON_CALIB;
          return;
     }
/*
 * loop over all calibration options, make sure we jump over all sub-options
 */
     while ( nr < NumOptions ) {
	  if ( (cpntr = strstr( calib_str, calib_opts[nr].key )) != NULL ) {
	       if ( cpntr[1] == '\0' || cpntr[1] == ',' ) { /* no sub-options */
		    *calib_mask = ((*calib_mask) | calib_opts[nr].val);
	       } else {                                  /* check sub-options */
		    while ( cpntr[1] != '\0' && cpntr[1] != ',' ) {
			 ++cpntr;
			 for ( ns = 0; ns <= calib_opts[nr].nr_sub; ns++ ) {
			      if ( *cpntr == *calib_opts[nr+ns].key )
				   *calib_mask = 
					((*calib_mask) | calib_opts[nr+ns].val);
			 }
		    }
		    if ( ((*calib_mask) & calib_opts[nr].val) == UINT_ZERO )
			 *calib_mask = ((*calib_mask) | calib_opts[nr].val);
	       }
	  }
	  nr += (calib_opts[nr].nr_sub + 1);
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_GET_CALIB
.PURPOSE     return string with used calibration options
.INPUT/OUTPUT
   call as:  SCIA_GET_CALIB( calib_val, calib_str );
     input:
            unsigned int calib_val :   encoded calibration steps
    output:  
            char calib_str[] :         string with encoded calibration steps

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void SCIA_GET_CALIB( unsigned int calib_val, char calib_str[] )
{
     register short ns;
     register int   nr;

     (void) strcpy( calib_str, "" );
     for ( nr = 0; nr < NumOptions; nr++ ) {
	  const short nr_sub = calib_opts[nr].nr_sub;

	  if ( (calib_val & calib_opts[nr].val) != UINT_ZERO ) {
	       if ( *calib_str != '\0' ) (void) strcat( calib_str, "," );
	       (void) strcat( calib_str, calib_opts[nr].key );
	       for ( ns = 0; ns < nr_sub; ns++ ) {
		    ++nr;
		    if ((calib_val & calib_opts[nr].val) != UINT_ZERO)
			 (void) strcat( calib_str, calib_opts[nr].key );
	       }
	  } else
	       nr += nr_sub;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_SHOW_CALIB
.PURPOSE     show all implemented calibration options
.INPUT/OUTPUT
  call as:   SCIA_SHOW_CALIB( FILE *stream );
    input:
            FILE stream :   stream to show available calibration options

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void SCIA_SHOW_CALIB( FILE *stream )
{
     register short ns;
     register int   nr;

     for ( nr = 0; nr < NumOptions; nr++ ) {
	  const short nr_sub = calib_opts[nr].nr_sub;

	  (void) fprintf( stream, "   %s:\t%s\n", calib_opts[nr].key, 
			  calib_opts[nr].desc );
	  for ( ns = 0; ns < nr_sub; ns++ ) {
	       ++nr;
	       (void) fprintf( stream, "     %s:\t%s\n", 
			       calib_opts[nr].key, calib_opts[nr].desc );
	  }
     }
}
