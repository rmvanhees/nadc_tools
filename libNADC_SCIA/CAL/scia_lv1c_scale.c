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

.IDENTifer   SCIA_LV1C_SCALE
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 1c product
.LANGUAGE    ANSI C
.PURPOSE     apply correction factor on Level 1 MDS Science data
.INPUT/OUTPUT
  call as    SCIA_LV1C_SCALE( num_mds, mds_1c );
     input:
            unsigned int num_mds      : number of MDS records
 in/output:
            struct mds1c_scia *mds_1c : level 1c MDS records

.RETURNS     nothing
.COMMENTS    nothing
.ENVIRONment None
.VERSION      2.0   30-May-2010	renamed to scia_lv1c_scale, RvH
              1.0   31-May-2006	Initial release, Richard van Hees (SRON)
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia_cal.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/

/*+++++ Global Variables +++++*/

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static
void ReadCorrFactors( float *CorrFact )
{
     const char flname[] = "SciaL1c_CorrFact.dat";

     register unsigned short nr;

     FILE  *fp_corr;
     char  str_buf[SHORT_STRING_LENGTH];
     float corr;

     char *env_str = getenv( "SCIA_CORR_L1C" );

     if ( env_str == NULL ) {
	  if ( (fp_corr = fopen( flname, "r" )) == NULL )
	       NADC_RETURN_ERROR( NADC_ERR_FILE_RD, flname );
     } else {
	  if ( (fp_corr = fopen( env_str, "r" )) == NULL )
	       NADC_RETURN_ERROR( NADC_ERR_FILE_RD, env_str );
     }
/*
 * read correction factors
 */
     nr = 0;
     while ( fgets(str_buf, SHORT_STRING_LENGTH, fp_corr) != NULL ) {
	  if ( *str_buf != '#' && nr < SCIENCE_PIXELS ) {
	       (void) sscanf( str_buf, "%f", &corr );
	       CorrFact[nr++] = corr;
	  }
     }
     (void) fclose( fp_corr );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1C_SCALE( unsigned int num_mds, struct mds1c_scia *mds_1c )
{
     register unsigned short num = 0;

     static float CorrFact[SCIENCE_PIXELS];

     static int do_init = TRUE;
/*
 * read correction factors
 */
     if ( do_init ) {
	  ReadCorrFactors( CorrFact );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( NADC_ERR_FATAL, "ReadCorrFactors" );
	  do_init = FALSE;
     }
/*
 * apply correction factors
 */
     do {
	  register unsigned short nobs = 0u;
	  register float *signal       = mds_1c->pixel_val;
	  do {
	       register unsigned short npix = 0u;
	       do {
                    *signal++ *= CorrFact[npix];
	       } while ( ++npix < mds_1c->num_pixels );
	  } while ( ++nobs < mds_1c->num_obs );
     } while ( mds_1c++, ++num < num_mds );
}
