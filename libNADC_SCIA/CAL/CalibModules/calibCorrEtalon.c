/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2006 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   calibAtbdEtalon
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy - Level 1b - Calibration
.LANGUAGE    ANSI C
.PURPOSE     perform Etalon correction on Sciamachy L1b science data
.INPUT/OUTPUT
  call as   SCIA_ATBD_CAL_ETALON( fileParam, state, mds_1c );
     input:  
             struct file_rec *fileParam : file/calibration parameters
	     struct state1_scia *state  : structure with States of the product
 in/output:  
             struct mds1c_scia *mds_1c  : level 1c MDS records
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0   06-Jun-2006 initial release by R. M. van Hees
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
#define _SCIA_LEVEL_1
#include <nadc_scia_cal.h>

/*+++++ Macros +++++*/
        /* NONE */

/*+++++ Static Variables +++++*/
        /* NONE */

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_ATBD_CAL_ETALON( const struct file_rec *fileParam,
			   const struct state1_scia *state, 
			   struct mds1c_scia *mds_1c )
{
     register unsigned short num = 0u;

     static float etalon[SCIENCE_PIXELS];
/*
 * Reread calibration parameters
 *  - at first call
 *  - when a new file was opened
 */
     if ( fileParam->flagInitFile ) {
	  struct ppg_scia ppg;

	  (void) SCIA_LV1_RD_PPG( fileParam->fp, fileParam->num_dsd, 
				  fileParam->dsd, &ppg );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_RD, "PPG" );
	  (void) memcpy( etalon, ppg.etalon_fact, 
			 SCIENCE_PIXELS * sizeof(float) );
     }
/*
 * apply Etalon correction
 */
     do {
	  register unsigned short nobs     = 0u;
	  register unsigned short *pixelID = mds_1c->pixel_ids;
	  register float          *signal  = mds_1c->pixel_val;

	  do {
	       register unsigned short npix = 0u;

	       do {
#ifdef DEBUG
		    if ( pixelID[npix] >= 190 && pixelID[npix] <= 200 )
			 (void) fprintf( stderr, "%5hu %20.8f\n",
					 pixelID[npix],
					 etalon[pixelID[npix]] );
#endif
		    *signal++ /= etalon[pixelID[npix]];
	       } while ( ++npix < mds_1c->num_pixels );
	  } while ( ++nobs < mds_1c->num_obs );
     } while ( mds_1c++, ++num < state->num_clus );   
}
