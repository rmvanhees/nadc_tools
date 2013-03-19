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

.IDENTifer   calibSronPPG
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy - Level 1b - Calibration
.LANGUAGE    ANSI C
.PURPOSE     perform SDMF Pixel-to-Pixel Gain correction
.INPUT/OUTPUT
  call as   SCIA_SRON_CAL_PPG( fileParam, state, mds_1c );
     input:  
             struct file_rec *fileParam : file/calibration parameters
	     struct state1_scia *state  : structure with States of the product
 in/output:  
             struct mds1c_scia *mds_1c  : level 1c MDS records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   04-Jul-2012 seperated ATBD and SDMF implementation, RvH
              2.2   17-Mar-2011 back-ported SDMF v2.4, RvH
              2.0   21-Jan-2009 moved to SDMF v3, RvH
              1.1   05-Sep-2007 apply L1b PPG when no data is available in SDMF
              1.0   06-Jun-2006 initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <hdf5.h>
#include <hdf5_hl.h>

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
static
void SCIA_SET_FIXED_PPG( float *ppg_fact )
{
     const char prognm[] = "SCIA_GET_FIXED_PPG";

     const char ppg_fl[] = DATA_DIR"/ppg_fixed_ch8.h5";

     const size_t offs = 7 * CHANNEL_SIZE;

     herr_t stat;

     hid_t file_id = H5Fopen( ppg_fl, H5F_ACC_RDONLY, H5P_DEFAULT );
     if ( file_id < 0 ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_FILE, ppg_fl );

     stat = H5LTread_dataset_float( file_id, "ppg", ppg_fact+offs );
     (void) H5Fclose( file_id );
     if ( stat < 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, "ppg" );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_SRON_CAL_PPG( const struct file_rec *fileParam,
			const struct state1_scia *state, 
			struct mds1c_scia *mds_1c )
{
     const char prognm[] = "SCIA_SRON_CAL_PPG";

     register unsigned short num = 0u;

     register double derror;

     static float ppg_fact[SCIENCE_PIXELS];
/*
 * Read calibration parameters
 *  - at first call
 *  - when a new file was opened
 */
     if ( fileParam->flagInitFile ) {
          bool found;

          if ( fileParam->sdmf_version == 24 )
               found = SDMF_get_PPG_24( fileParam->absOrbit, 0, ppg_fact );
          else
               found = SDMF_get_PPG_30( fileParam->absOrbit, 0, ppg_fact );
          if ( IS_ERR_STAT_FATAL )
               NADC_RETURN_ERROR( prognm,NADC_ERR_FATAL,"SDMF_get_PPG" );
	  if ( ! found )
	       NADC_ERROR( prognm, NADC_ERR_NONE, "no SDMF PPG data" );

	  if ( (fileParam->calibFlag & DO_FIXED_PPG) != UINT_ZERO ) {
	       SCIA_SET_FIXED_PPG( ppg_fact );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "PPG_FIXED" );
	  }
     }
/*
 * apply Pixel-to-Pixel Gain correction
 */
     do {
	  register unsigned short nobs      = 0u;
	  register unsigned short *pixelID  = mds_1c->pixel_ids;
	  register float          *signal   = mds_1c->pixel_val;
	  register float          *e_signal = mds_1c->pixel_err;

	  do {
	       register unsigned short npix = 0u;

	       do {
		    if ( (fileParam->calibFlag & DO_CALC_ERROR) != UINT_ZERO ){
			 derror = fileParam->ppgError * (*signal);

			 *e_signal += (float) (derror * derror);
		    }
		    if ( fabsf( ppg_fact[pixelID[npix]] ) < 1e-3 )
			 *signal = 0.f;
		    else
			 *signal /= ppg_fact[pixelID[npix]];

		    signal++; e_signal++; 
	       } while ( ++npix < mds_1c->num_pixels );
	  } while ( ++nobs < mds_1c->num_obs );
     } while ( mds_1c++, ++num < state->num_clus );   
}
