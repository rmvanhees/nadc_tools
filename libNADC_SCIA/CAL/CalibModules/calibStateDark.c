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

.IDENTifer   calibStateDark
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy - Level 1b - Calibration
.LANGUAGE    ANSI C
.PURPOSE     perform dark correction using DARK_AVERAGE ADS or SDMF state darks
.INPUT/OUTPUT
  call as   SCIA_STATE_CAL_DARK( fileParam, state, mds_1c );

     input:  
             struct file_rec *fileParam : file/calibration parameters
	     struct state1_scia *state  : structure with States of the product
 in/output:  
             struct mds1c_scia *mds_1c  : level 1c MDS records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     3.0   21-Apr-2010 renamed module and upgrade to SDMF 3.1, RvH
             2.1   21-Aug-2009 added error estimate, RvH 
             2.0   20-Aug-2009 added usage of SDMF average darks, RvH 
             1.0   09-Nov-2007 initial release by R. M. van Hees
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

#define NUM_DARKCORR   5
struct darkCorr_rec {
     unsigned short num[SCIENCE_CHANNELS];
     unsigned short saa[SCIENCE_CHANNELS];
     float pet[SCIENCE_CHANNELS];
     float val[SCIENCE_CHANNELS][CHANNEL_SIZE];
     float sdev[SCIENCE_CHANNELS][CHANNEL_SIZE];
};

/*+++++ Macros +++++*/

/*+++++ Static Variables +++++*/
static int Weight_noSAA = 10;
static int Weight_SAA = 1;

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static inline
void __Inverse_Chan2( float *rbuff )
{
     register unsigned short nr;
     register float rtemp;

     rbuff += CHANNEL_SIZE;                   /* move to channel 2 data */
     for ( nr = 0; nr <  CHANNEL_SIZE / 2; nr++ ) {
	  rtemp = rbuff[nr];
	  rbuff[nr] = rbuff[(CHANNEL_SIZE-1)-nr];
	  rbuff[(CHANNEL_SIZE-1)-nr] = rtemp;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   calcShotNoise
.PURPOSE     calculate precision of SCIA data, which is shot noise limited
.INPUT/OUTPUT
  call as    calcShotNoise( electron_bu, darkCorr, mds1c );
     input:
	   float  electron_bu            :  number of photon-electrons per BU
	   struct darkCorr_rec *darkCorr :  record holding Dark Calibration data
 in/output:
	   struct mds1c_scia   *mds_1c   :  level 1c MDS records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
void calcShotNoise( float electron_bu, const struct darkCorr_rec *darkCorr, 
		    struct mds1c_scia *mds_1c )
     /*@modifies errno, mds_1c->pixel_err@*/
{
     register unsigned short nobs = 0;

     register float *e_signal = mds_1c->pixel_err;

     register const float *signal = mds_1c->pixel_val;
     register const float *analog, *noise;

     const unsigned short chanIndx = mds_1c->chan_id - 1;
     const unsigned short pixelIndx = mds_1c->pixel_ids[0] % CHANNEL_SIZE;
/*
 * find dark state with shortest integration time
 */
     register unsigned short np = 0;
     register unsigned short i_pet_mn = 0;
     register float pet_mn = 100.f;

     do {
	  if ( darkCorr[np].pet[chanIndx] < pet_mn ) {
	       i_pet_mn = np;
	       pet_mn = darkCorr[np].pet[chanIndx];
	  }
     } while ( ++np < NUM_DARKCORR );
     analog = &darkCorr[i_pet_mn].val[chanIndx][pixelIndx];
     noise  = &darkCorr[i_pet_mn].sdev[chanIndx][pixelIndx];
/*
 * calculate shot-noise
 */
     do {
	  register unsigned short npix = 0;
	  do {
	       register float N_sign = *signal - mds_1c->coaddf * analog[npix];

	       *e_signal++ = fabsf( N_sign ) / electron_bu
		    + mds_1c->coaddf * noise[npix] * noise[npix];
	  } while( signal++, ++npix < mds_1c->num_pixels );
     } while ( ++nobs < mds_1c->num_obs );
}

/*+++++++++++++++++++++++++
.IDENTifer   calcStateDarkError
.PURPOSE     calculate error on the measured signal due to the substraction
             of the Dark Current correction (average dark)
.INPUT/OUTPUT
  call as    calcStateDarkError( mds_1c );

 in/output:
	   struct mds1c_scia *mds_1c :  level 1c MDS records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
void calcStateDarkError( struct mds1c_scia *mds_1c )
     /*@modifies mds_1c->pixel_err@*/
{
     register unsigned short nobs = 0;

     register float *e_signal = mds_1c->pixel_err;

     do {
	  register unsigned short npix = 0;
	  do {
	       *e_signal *= 5;
	  } while( e_signal++, ++npix < mds_1c->num_pixels );
     } while ( ++nobs < mds_1c->num_obs );
}

/*+++++++++++++++++++++++++
.IDENTifer   readStateDark_ADS
.PURPOSE     Read darkcurrent correction values from DARK_AVERAGE ADS (Lv1b)
.INPUT/OUTPUT
  call as    readStateDark_ADS( fileParam, darkCorr );
     input:
	   struct file_rec fileParam     :  file/calibration parameters
    output:
	   struct darkCorr_rec *darkCorr :  Dark Calibration data

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void readStateDark_ADS( const struct file_rec *fileParam,
			/*@out@*/ struct darkCorr_rec *darkCorr )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, fileParam->fp, darkCorr@*/
{
     register unsigned short nc, nd, ni;
     register unsigned int   nr;

     unsigned int num_dark, num_sqads, num_state;

     struct sqads1_scia *sqads = NULL;
     struct state1_scia *state = NULL;
     struct dark_scia   *dark  = NULL;

     const bool Save_Extern_Alloc = Use_Extern_Alloc;
/*
 * read Dark average ADS records
 */
     Use_Extern_Alloc = FALSE;
     num_dark = SCIA_LV1_RD_DARK( fileParam->fp, fileParam->num_dsd, 
				  fileParam->dsd, &dark );
     if ( IS_ERR_STAT_ABSENT || num_dark == 0 ) {
	  nadc_stat &= ~NADC_STAT_ABSENT;
	  NADC_RETURN_ERROR( NADC_ERR_WARN, "no DARK_AVERAGE found" );
     } else if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "DARK" );
/*
 * read/write Summary of Quality Flags per State records
 */
     num_sqads = SCIA_LV1_RD_SQADS( fileParam->fp, fileParam->num_dsd, 
				    fileParam->dsd, &sqads );
     if ( IS_ERR_STAT_FATAL || num_sqads == 0 )
          NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "SQADS" );
/*
 * read/write States of the Product
 */
     num_state = SCIA_LV1_RD_STATE( fileParam->fp, fileParam->num_dsd, 
				    fileParam->dsd, &state );
     if ( IS_ERR_STAT_FATAL || num_state == 0 )
          NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "STATE" );
     Use_Extern_Alloc = Save_Extern_Alloc;
/*
 * initialise darkCorr array
 */
     for ( nr = 0; nr < NUM_DARKCORR; nr++ ) {
	  (void) memset( darkCorr[nr].num, 0, 
			 SCIENCE_CHANNELS * sizeof( short ));
	  (void) memset( darkCorr[nr].saa, 0, 
			 SCIENCE_CHANNELS * sizeof( short ));
	  (void) memset( darkCorr[nr].pet, 0, 
			 SCIENCE_CHANNELS * sizeof( float ));
	  (void) memset( darkCorr[nr].val[0], 0, 
			 SCIENCE_PIXELS * sizeof( float ));
	  (void) memset( darkCorr[nr].sdev[0], 0, 
			 SCIENCE_PIXELS * sizeof( float ));
     }
/*
 * combine multiple dark ADS (with the same state ID)
 */
     for ( nr = 0; nr < num_dark; nr++ ) {
	  register unsigned char  channel = 0;
	  register unsigned short indx = 0;
	  register unsigned short chanIndx, offs;
	  register int wght;
	  register float pet;

	  do {
	       if ( memcmp( &dark[nr].mjd, &state[indx].mjd, 
			    sizeof( struct mjd_envi ) ) == 0 )
		    break;
	  } while ( ++indx < num_state );
	  if ( indx == num_state ) continue;

	  nc = 0;
	  while ( TRUE ) {
	       while ( nc < state[indx].num_clus 
		       && state[indx].Clcon[nc].channel == channel ) nc++;
	       if ( nc == state[indx].num_clus ) break;

	       channel = state[indx].Clcon[nc].channel;
	       chanIndx = channel - 1;
	       pet = state[indx].Clcon[nc].pet;

	       for ( nd = 0; nd < NUM_DARKCORR; nd++ ) {
		    if ( darkCorr[nd].num[chanIndx] == 0
			 || darkCorr[nd].pet[chanIndx] == pet )
			 break;
	       }
	       if ( nd == NUM_DARKCORR ) continue;

	       if ( sqads[indx].flag_saa_region == 1 ) {
		    wght = Weight_SAA;
		    darkCorr[nd].saa[chanIndx] += 1;
	       } else {
		    wght = Weight_noSAA;
	       }
	       darkCorr[nd].num[chanIndx] += wght;
	       darkCorr[nd].pet[chanIndx] = pet;

	       offs = chanIndx * CHANNEL_SIZE;
	       for ( ni = 0; ni < CHANNEL_SIZE; ni++, offs++ ) {
		    float coaddf = state[indx].Clcon[nc].coaddf;
		    float rbuff  = wght * dark[nr].sdev_dark_spec[offs];

		    darkCorr[nd].val[chanIndx][ni] += 
			 wght * dark[nr].dark_spec[offs] / coaddf;
		    darkCorr[nd].sdev[chanIndx][ni] += 
			 (rbuff * rbuff) / coaddf;
	       }
	  }
     }
     for ( nd = 0; nd < NUM_DARKCORR; nd++ ) {
	  for ( nc = 0; nc < SCIENCE_CHANNELS; nc++ ) {
	       for ( ni = 0; ni < CHANNEL_SIZE; ni++ ) {
		    darkCorr[nd].val[nc][ni] /= darkCorr[nd].num[nc];
		    darkCorr[nd].sdev[nc][ni] /= darkCorr[nd].num[nc];
		    darkCorr[nd].sdev[nc][ni] = 
			 sqrtf( darkCorr[nd].sdev[nc][ni] );
	       }
	  }
	  
     }
 done:
     if ( sqads != NULL ) free( sqads );
     if ( state != NULL ) free( state );
     if ( dark  != NULL ) free( dark );
}

/*+++++++++++++++++++++++++
.IDENTifer   readStateDark_SDMF
.PURPOSE     Read darkcurrent correction values from SDMF database
.INPUT/OUTPUT
  call as    readStateDark_SDMF( fileParam, darkCorr );
     input:
	   struct file_rec fileParam     :  file/calibration parameters
    output:
	   struct darkCorr_rec *darkCorr :  Dark Calibration data

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void readStateDark_SDMF( const struct file_rec *fileParam,
			   /*@out@*/ struct darkCorr_rec *darkCorr )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, fileParam->fp, darkCorr@*/
{
     const int absOrbit = fileParam->absOrbit;

     register unsigned short nc, nr;

     unsigned short num_dark = 0;

     bool  found;
     float pet[SCIENCE_CHANNELS];
     float darkcorr[SCIENCE_PIXELS], darknoise[SCIENCE_PIXELS];
/*
 * initialise darkCorr array
 */
     for ( nr = 0; nr < NUM_DARKCORR; nr++ ) {
	  (void) memset( darkCorr[nr].num, 0, 
			 SCIENCE_CHANNELS * sizeof( short ));
	  (void) memset( darkCorr[nr].saa, 0, 
			 SCIENCE_CHANNELS * sizeof( short ));
	  (void) memset( darkCorr[nr].pet, 0, 
			 SCIENCE_CHANNELS * sizeof( float ));
	  (void) memset( darkCorr[nr].val[0], 0, 
			 SCIENCE_PIXELS * sizeof( float ));
	  (void) memset( darkCorr[nr].sdev[0], 0, 
			 SCIENCE_PIXELS * sizeof( float ));
     }
/*
 * read Dark average ADS records from SDMF database
 */
     found = SDMF_get_StateDark( 8, 0, absOrbit, pet, darkcorr, darknoise );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, "failed for state_08" ) ;
     if ( ! found )
	  NADC_ERROR( NADC_ERR_NONE, "no SDMF state_08" );
     else {
	  for ( nc = 0; nc < SCIENCE_CHANNELS; nc++ )
	       darkCorr[num_dark].num[nc] = 1;
	  (void) memcpy( darkCorr[num_dark].pet,
			 pet, SCIENCE_CHANNELS * sizeof( float ));
	  __Inverse_Chan2( darkcorr );
	  (void) memcpy( darkCorr[num_dark].val[0],
			 darkcorr, SCIENCE_PIXELS * sizeof( float ));
	  (void) memcpy( darkCorr[num_dark].sdev[0],
			 darknoise, SCIENCE_PIXELS * sizeof( float ));
	  num_dark++;
     }
     found = SDMF_get_StateDark( 26, 0, absOrbit, pet, darkcorr, darknoise );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, "failed for state_26" ) ;
     if ( ! found )
	  NADC_ERROR( NADC_ERR_NONE, "no SDMF state_26" );
     else {
	  for ( nc = 0; nc < SCIENCE_CHANNELS; nc++ )
	       darkCorr[num_dark].num[nc] = 1;
	  (void) memcpy( darkCorr[num_dark].pet,
			 pet, SCIENCE_CHANNELS * sizeof( float ));
	  __Inverse_Chan2( darkcorr );
	  (void) memcpy( darkCorr[num_dark].val[0],
			 darkcorr, SCIENCE_PIXELS * sizeof( float ));
	  (void) memcpy( darkCorr[num_dark].sdev[0],
			 darknoise, SCIENCE_PIXELS * sizeof( float ));
	  num_dark++;
     }
     found = SDMF_get_StateDark( 46, 0, absOrbit, pet, darkcorr, darknoise );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, "failed for state_46" ) ;
     if ( ! found )
	  NADC_ERROR( NADC_ERR_NONE, "no SDMF state_46" );
     else {
	  for ( nc = 0; nc < SCIENCE_CHANNELS; nc++ )
	       darkCorr[num_dark].num[nc] = 1;
	  (void) memcpy( darkCorr[num_dark].pet,
			 pet, SCIENCE_CHANNELS * sizeof( float ));
	  __Inverse_Chan2( darkcorr );
	  (void) memcpy( darkCorr[num_dark].val[0],
			 darkcorr, SCIENCE_PIXELS * sizeof( float ));
	  (void) memcpy( darkCorr[num_dark].sdev[0],
			 darknoise, SCIENCE_PIXELS * sizeof( float ));
	  num_dark++;
     }
     found = SDMF_get_StateDark( 63, 0, absOrbit, pet, darkcorr, darknoise );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, "failed for state_63" ) ;
     if ( ! found )
	  NADC_ERROR( NADC_ERR_NONE, "no SDMF state_63" );
     else {
	  for ( nc = 0; nc < SCIENCE_CHANNELS; nc++ )
	       darkCorr[num_dark].num[nc] = 1;
	  (void) memcpy( darkCorr[num_dark].pet,
			 pet, SCIENCE_CHANNELS * sizeof( float ));
	  __Inverse_Chan2( darkcorr );
	  (void) memcpy( darkCorr[num_dark].val[0],
			 darkcorr, SCIENCE_PIXELS * sizeof( float ));
	  (void) memcpy( darkCorr[num_dark].sdev[0],
			 darknoise, SCIENCE_PIXELS * sizeof( float ));
	  num_dark++;
     }
     found = SDMF_get_StateDark( 67, 0, absOrbit, pet, darkcorr, darknoise );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, "failed for state_67" ) ;
     if ( ! found )
	  NADC_ERROR( NADC_ERR_NONE, "no SDMF state_67" );
     else {
	  for ( nc = 0; nc < SCIENCE_CHANNELS; nc++ )
	       darkCorr[num_dark].num[nc] = 1;
	  (void) memcpy( darkCorr[num_dark].pet,
			 pet, SCIENCE_CHANNELS * sizeof( float ));
	  __Inverse_Chan2( darkcorr );
	  (void) memcpy( darkCorr[num_dark].val[0],
			 darkcorr, SCIENCE_PIXELS * sizeof( float ));
	  (void) memcpy( darkCorr[num_dark].sdev[0],
			 darknoise, SCIENCE_PIXELS * sizeof( float ));
	  num_dark++;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   applyStateDarkData
.PURPOSE     apply darkcurrent correction values from DARK_AVERAGE ADS (Lv1b)
.INPUT/OUTPUT
  call as    applyStateDarkData( darkCorr, mds_1c );
     input:
	   struct file_rec fileParam     :  file/calibration parameters
    output:
	   struct darkCorr_rec *darkCorr :  record holding Dark Calibration

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
static inline
void applyStateDarkData( const struct darkCorr_rec *darkCorr,
		       struct mds1c_scia *mds_1c )
     /*@modifies mds_1c->pixel_val@*/
{
     register unsigned short nd, ni, nj;
     register float diff;

     register unsigned short nobs = 0;
     register float *signal   = mds_1c->pixel_val;

     char  msg[64];
     float rbuff[CHANNEL_SIZE];

/* constants */
     register const float *dark_pntr = NULL;

     const unsigned short chanIndx = mds_1c->chan_id - 1;
     const unsigned short pixIndx = mds_1c->pixel_ids[0] % CHANNEL_SIZE;
/*
 * find corresponding Dark dataset
 */
     for ( nd = 0; nd < NUM_DARKCORR; nd++ ) {
	  if ( mds_1c->pet == darkCorr[nd].pet[chanIndx] ) {
	       dark_pntr   = darkCorr[nd].val[chanIndx] + pixIndx;

               /* apply dark correction */
	       do {
		    register unsigned short npix = 0;
		    do {
			 *signal++ -= (float) mds_1c->coaddf * dark_pntr[npix];
		    } while( ++npix < mds_1c->num_pixels );
	       } while ( ++nobs < mds_1c->num_obs );
	       return;
	  }
     }
/*
 * interpolate to required pixel exposure time
 */
     diff = 999.f;
     for ( ni = 0, nd = 0; nd < NUM_DARKCORR; nd++ ) {
	  if ( mds_1c->pet > darkCorr[nd].pet[chanIndx]
	       && (mds_1c->pet - darkCorr[nd].pet[chanIndx]) < diff ) {
	       ni = nd;
	       diff = mds_1c->pet - darkCorr[nd].pet[chanIndx];
	  }
     }
     diff = 999.f;
     for ( nj = 0, nd = 0; nd < NUM_DARKCORR; nd++ ) {
	  if ( mds_1c->pet < darkCorr[nd].pet[chanIndx]
	       && (darkCorr[nd].pet[chanIndx] - mds_1c->pet) < diff ) {
	       nj = nd;
	       diff = darkCorr[nd].pet[chanIndx] - mds_1c->pet;
	  }
     }
     NADC_INTERPOL( mds_1c->pet, 
		    darkCorr[ni].pet[chanIndx], darkCorr[nj].pet[chanIndx],
		    mds_1c->num_pixels,
		    darkCorr[ni].val[chanIndx] + pixIndx,
		    darkCorr[nj].val[chanIndx] + pixIndx, rbuff );
/*
 * apply dark correction
 */
     do {
	  register unsigned short npix = 0;
	  do {
	       *signal++ -= (float) mds_1c->coaddf * rbuff[npix];
	  } while( ++npix < mds_1c->num_pixels );
     } while ( ++nobs < mds_1c->num_obs );

     (void) snprintf( msg, 64, "state/cluster [%-hhu,%-hhu]"
		     " required interpolation for PET: %.4f",
		     mds_1c->state_id, mds_1c->clus_id, mds_1c->pet );
     NADC_ERROR( NADC_ERR_NONE, msg );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_STATE_CAL_DARK( const struct file_rec *fileParam,
			  const struct state1_scia *state,
			  struct mds1c_scia *mds_1c )
{
     register unsigned short num = 0u;     /* counter for number of clusters */

     struct darkCorr_rec darkCorr[NUM_DARKCORR];
/*
 * read calibration data for DarkCurrent correction
 */
     if ( (fileParam->calibFlag & DO_SRON_DARK) != USHRT_ZERO )
	  readStateDark_SDMF( fileParam, darkCorr );
     else
	  readStateDark_ADS( fileParam, darkCorr );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_RD, "readStateDark" );
/*
 * do actual dark current correction
 */
     do {
	  if ( (fileParam->calibFlag & DO_CALC_ERROR) != UINT_ZERO ) {
	       const float electron_bu = 
		    fileParam->electron_bu[mds_1c->chan_id-1];

	       calcShotNoise( electron_bu, darkCorr, mds_1c );
	  }
	  applyStateDarkData( darkCorr, mds_1c );
	  if ( (fileParam->calibFlag & DO_CALC_ERROR) != UINT_ZERO )
	       calcStateDarkError( mds_1c );
     } while ( ++mds_1c, ++num < state->num_clus );
}
