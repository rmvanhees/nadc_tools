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

.IDENTifer   calibSronNoise
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy - Level 1b - Calibration
.LANGUAGE    ANSI C
.PURPOSE     calculate measurement noise of channels 8
.INPUT/OUTPUT
  call as   SCIA_SRON_CAL_NOISE( fileParam, state, mds_1c );
     input:  
             struct file_rec *fileParam : file/calibration parameters
	     struct state1_scia *state  : structure with States of the product
 in/output:  
             struct mds1c_scia *mds_1c  : level 1c MDS records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0   02-Apr-2012 initial release by R. M. van Hees
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

#include <math.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_sdmf.h>
#include <nadc_scia_cal.h>

/*+++++ Macros +++++*/
        /* NONE */

/*+++++ Static Variables +++++*/
        /* NONE */

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_SRON_CAL_NOISE( const struct file_rec *fileParam,
			  const struct state1_scia *state, 
			  struct mds1c_scia *mds_1c )
{
     const char prognm[] = "SCIA_SRON_CAL_NOISE";

     register unsigned short num = 0u;

     unsigned char stateID_05, stateID_10;
     bool  found_05, found_10;
     float pet;
     float darkSignal[CHANNEL_SIZE];

     /* mean dark noise - chan 8 - 0.5 sec */
     float darkNoise_05[CHANNEL_SIZE];         
     /* mean dark noise - chan 8 - 1.0 sec */
     float darkNoise_10[CHANNEL_SIZE];
/*
 * read mean Dark parameters
 */
     stateID_05 = SDMF_PET2StateID( fileParam->absOrbit, 8, 0.5 );
     stateID_10 = SDMF_PET2StateID( fileParam->absOrbit, 8, 1.0 );

     if ( fileParam->sdmf_version == 24 ) {
	  found_05 = SDMF_get_StateDark_24( stateID_05, 8, fileParam->absOrbit,
					    &pet, darkSignal, darkNoise_05 );

	  found_10 = SDMF_get_StateDark_24( stateID_10, 8, fileParam->absOrbit,
					    &pet, darkSignal, darkNoise_10 );
     } else if ( fileParam->sdmf_version == 30 ) {
	  found_05 = SDMF_get_StateDark_30( stateID_05, 8, fileParam->absOrbit,
					    &pet, darkSignal, darkNoise_05 );

	  found_10 = SDMF_get_StateDark_30( stateID_10, 8, fileParam->absOrbit,
					    &pet, darkSignal, darkNoise_10 );
     } else {
	  found_05 = SDMF_get_StateDark( stateID_05, 8, fileParam->absOrbit,
					 &pet, darkSignal, darkNoise_05 );

	  found_10 = SDMF_get_StateDark( stateID_10, 8, fileParam->absOrbit,
					 &pet, darkSignal, darkNoise_10 );
     }
     if ( ! found_05 || ! found_10 )
	  NADC_RETURN_ERROR(prognm, NADC_ERR_FATAL, "no Dark parameters found");
/*
 * estimate measurement noise
 */
     do {
	  register unsigned short nobs      = 0u;
	  register const float    *signal   = mds_1c->pixel_val;
	  register float          *e_signal = mds_1c->pixel_err;
	  register float          *meanNoise;

	  const unsigned short  ipx = mds_1c->pixel_ids[0] % CHANNEL_SIZE;

	  const double facs = 177;

	  if ( mds_1c->chan_id != 8 ) continue;   /* only channel 8 */
/*
 * calculate noise
 */
	  do {
	       register unsigned short npix = 0u;

	       meanNoise = 
		    (mds_1c->pet > 0.75) ? darkNoise_10+ipx : darkNoise_05+ipx;

	       do {
		    if ( *meanNoise > 0.f ) {
			 register double dbuff = (*meanNoise) * (*meanNoise);

			 if ( *signal > 0.f ) dbuff += *signal / facs;
			 *e_signal = (float) sqrt( dbuff );
		    } else
			 *e_signal = (float) 1e10;

		    signal++; e_signal++; meanNoise++;
	       } while ( ++npix < mds_1c->num_pixels );
	  } while ( ++nobs < mds_1c->num_obs );
     } while ( mds_1c++, ++num < state->num_clus );   
}
