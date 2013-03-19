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

.IDENTifer   calibFlagBDPM
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy - Level 1b - Calibration
.LANGUAGE    ANSI C
.PURPOSE     mask Dead/Bad pixels on Sciamachy L1b science data
.INPUT/OUTPUT
  call as   SCIA_ATBD_FLAG_BDPM( fileParam, state, mds_1c );
            SCIA_SRON_FLAG_BDPM( fileParam, state, mds_1c );
            SCIA_LV1C_FLAG_BDPM( absOrbit, num_mds, mds_1c );
     input:  
             struct file_rec *fileParam : file/calibration parameters
	     struct state1_scia *state  : structure with States of the product
 in/output:  
             struct mds1c_scia *mds_1c  : level 1c MDS records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      3.3   15-Mar-2012 export functions getBadPixelMaskSRON_24
                                and getBadPixelMaskSRON_30, RvH
              3.2   17-Mar-2011 back-ported SDMF v2.4, RvH
              3.1   15-Sep-2010 fixed bug causing an overwrite of 
                                the SDMF mask with the L1b mask, RvH
              3.0   30-May-2010 renamed modules, RvH
              2.0   21-Jan-2009 moved to SDMF v3, RvH
              1.2   26-Nov-2006 added SCIA_LV0_CAL_BDPM, RvH
              1.0   06-Jun-2006 initial release by R. M. van Hees
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
#include <glob.h>

#include <hdf5.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_sdmf.h>
#include <nadc_scia_cal.h>

/*+++++ Macros +++++*/
#define FIRST_VALID_SDMF_BDPM 3899

/*+++++ Static Variables +++++*/
        /* NONE */

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static inline
void Apply_flagBDPM( const unsigned char *bdpm, struct mds1c_scia *mds_1c )
{
     register unsigned short nobs  = 0u;
     register float *signal        = mds_1c->pixel_val;

     const unsigned short *pixelID = mds_1c->pixel_ids;

     do {
	  register unsigned short npix = 0u;
	  do {
	       if ( bdpm[pixelID[npix]] != UCHAR_ZERO ) *signal = NAN;
	       ++signal;
	  } while ( ++npix < mds_1c->num_pixels );
     } while ( ++nobs < mds_1c->num_obs );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_ATBD_FLAG_BDPM( const struct file_rec *fileParam,
			  const struct state1_scia *state, 
			  struct mds1c_scia *mds_1c )
{
     const char prognm[] = "SCIA_ATBD_FLAG_BDPM";

     register unsigned short num = 0u;

     static unsigned char bdpm[SCIENCE_PIXELS];
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
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "PPG" );
	  (void) memcpy( bdpm, ppg.bad_pixel, SCIENCE_PIXELS );
     }
/*
 * apply bad/dead pixel mask
 */
     do {
	  Apply_flagBDPM( bdpm, mds_1c );
     } while ( mds_1c++, ++num < state->num_clus );   
}

/*--------------------------------------------------*/
void SCIA_SRON_FLAG_BDPM( const struct file_rec *fileParam,
			  const struct state1_scia *state, 
			  struct mds1c_scia *mds_1c )
{
     const char prognm[] = "SCIA_SRON_FLAG_BDPM";

     register unsigned short num = 0u;

     static unsigned char bdpm[SCIENCE_PIXELS];
/*
 * Reread calibration parameters
 *  - at first call
 *  - when a new file was opened
 */
     if ( fileParam->flagInitFile ) {
	  unsigned short orbit = (fileParam->absOrbit > FIRST_VALID_SDMF_BDPM)
	       ? fileParam->absOrbit : FIRST_VALID_SDMF_BDPM;

	  bool found;

	  if ( fileParam->sdmf_version == 24 )
	       found = SDMF_get_BDPM_24( orbit, bdpm );
	  else
	       found = SDMF_get_BDPM_30( orbit, bdpm );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR(prognm, NADC_ERR_FATAL, "SDMF_get_BDPM");
	  if ( ! found ) {
	       struct ppg_scia ppg;

	       (void) SCIA_LV1_RD_PPG( fileParam->fp, fileParam->num_dsd, 
				       fileParam->dsd, &ppg );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "PPG" );
	       (void) memcpy( bdpm, ppg.bad_pixel, SCIENCE_PIXELS );
	  }
     }
/*
 * apply bad/dead pixel mask
 */
     do {
	  Apply_flagBDPM( bdpm, mds_1c );
     } while ( mds_1c++, ++num < state->num_clus );   
}
     
void SCIA_LV1C_FLAG_BDPM( unsigned short absOrbit, 
			  unsigned short num_mds, struct mds1c_scia *mds_1c )
{
     const char prognm[] = "SCIA_LV1C_FLAG_BDPM";

     register unsigned short num = 0u;     /* counter for number of clusters */

     static unsigned short orbit_old = 0;
     static unsigned char  bdpm[SCIENCE_PIXELS];

     if ( absOrbit != orbit_old ) {
	  bool found;

	  /* set SDMF version */
	  char *sdmf_version = getenv( "USE_SDMF_VERSION" );

	  if ( sdmf_version == NULL ) {
	       found = SDMF_get_BDPM_30( absOrbit, bdpm );
	  } else {
	       if ( strncmp( sdmf_version, "2.4", 3 ) == 0 )
		    found = SDMF_get_BDPM_24( absOrbit, bdpm );
	       else 
		    found = SDMF_get_BDPM_30( absOrbit, bdpm );
	  }
	  if ( ! found || IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "SDMF_get_BDPM" );
	  orbit_old = absOrbit;
     }
/*
 * apply bad/dead pixel mask
 */
     do {
	  Apply_flagBDPM( bdpm, mds_1c );
     } while ( mds_1c++, ++num < num_mds );   
}
