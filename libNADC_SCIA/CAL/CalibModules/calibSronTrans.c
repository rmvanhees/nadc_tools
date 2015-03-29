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

.IDENTifer   calibSronTrans
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy - Level 1b - Calibration
.LANGUAGE    ANSI C
.PURPOSE     perform Transmission correction on Sciamachy channel 8 data
.INPUT/OUTPUT
  call as   SCIA_SRON_CAL_TRANS( fileParam, state, mds_1c );
     input:  
             struct file_rec *fileParam : file/calibration parameters
	     struct state1_scia *state  : structure with States of the product
 in/output:  
             struct mds1c_scia *mds_1c  : level 1c MDS records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      1.1   21-May-2012 bug fixes and usage of SDMF routines, RvH
              1.0   27-Mar-2012 initial release by R. M. van Hees
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

#include <hdf5.h>

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
void SCIA_SRON_CAL_TRANS( const struct file_rec *fileParam,
			  const struct state1_scia *state, 
			  struct mds1c_scia *mds_1c )
{
     register unsigned short np;
     register unsigned short num = 0u;

     static float trans_fact[CHANNEL_SIZE];

     char *cbuff = getenv( "SCIA_TRANS_RANGE" );

     size_t dim;
     float  new_val = 1.f;
     float  trans_avg;
     
     unsigned short pixel_range[] = {505, 615};
/*
 * Read calibration parameters
 *  - at first call
 *  - when a new file was opened
 */
     if ( fileParam->flagInitFile ) {
	  bool found;

	  if ( fileParam->sdmf_version == 24 )
	       found = SDMF_get_Transmission_24( FALSE, fileParam->absOrbit, 
						 8, trans_fact );
	  else
	       found = SDMF_get_Transmission_30( FALSE, fileParam->absOrbit, 
						 8, trans_fact );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( NADC_ERR_FATAL, "SDMF_get_Transmission" );
	  if ( ! found )
	       NADC_ERROR( NADC_ERR_NONE, "no SDMF Transmission data" );
     }
/*
 * calculate average transmission
 */
     if ( cbuff != NULL ) {
	  int nrval;
	  NADC_USRINP( UINT16_T, cbuff, 2, pixel_range, &nrval );
     }

     /* replace NaN by 0.0 or 1.0 */
     for ( np = pixel_range[0]; np <= pixel_range[1]; np++ ) {
	  if ( isnan(trans_fact[np]) ) {
	       trans_fact[np] = new_val;
	       new_val = (new_val == 1.f) ? 0.f : 1.f;
	  }
     }
     dim = pixel_range[1] - pixel_range[0] + 1;
     trans_avg = SELECTr( (dim+1)/2, dim, trans_fact+pixel_range[0] );
     /* (void) fprintf( stderr, "%04hu-%04hu: %4zd %12.6g\n",  */
     /* 		     pixel_range[0], pixel_range[1], dim, trans_avg ); */
/*
 * apply Transmission correction
 */
     do {
	  register unsigned short nobs      = 0u;
	  register float          *signal   = mds_1c->pixel_val;

	  do {
	       register unsigned short npix    = 0u;

	       do {
		    *signal /= trans_avg;

		    signal++; 
	       } while ( ++npix < mds_1c->num_pixels );
	  } while ( ++nobs < mds_1c->num_obs );
     } while ( mds_1c++, ++num < state->num_clus );   
}
