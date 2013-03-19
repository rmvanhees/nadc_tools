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

.IDENTifer   calibSronDark
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy - Level 1b - Calibration
.LANGUAGE    ANSI C
.PURPOSE     perform dark current correction on Sciamachy L1b science data
.INPUT/OUTPUT
  call as   SCIA_SRON_CAL_DARK( fileParam, state, mds_1c );
     input:  
             struct file_rec *fileParam : file/calibration parameters
	     struct state1_scia *state  : structure with States of the product
 in/output:  
             struct mds1c_scia *mds_1c  : level 1c MDS records
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     2.1   18-Mar-2011 back-ported SDMF 2.4 & 3.0, RvH
             2.0   21-Apr-2010 upgrade to SDMF 3.1, RvH
             1.0   19-Apr-2010 seperate SDMF implementation, RvH
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
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_sdmf.h>
#include <nadc_scia_cal.h>

#include "getCorrIntg.inc"
#define _DarkCalib_
#include "calibCalcError.inc"

/*+++++ Macros +++++*/
        /* NONE */

/*+++++ Static Variables +++++*/
        /* NONE */

/*+++++ Global Variables +++++*/
        /* NONE */

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
.IDENTifer   readConstDarkSDMF
.PURPOSE     Read darkcurrent correction values from level 1b product
.INPUT/OUTPUT
  call as    readConstDarkSDMF( fileParam, &DarkData );
     input:
	   struct file_rec fileParam :  file/calibration parameters
    output:
	   struct DarkRec *DarkData  :  record holding Dark Calibration data

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void readConstDarkSDMF( const struct file_rec *fileParam,
			struct DarkRec *DarkData )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp, DarkData@*/
{
     const char prognm[] = "readDarkDataADS";

     bool  found;
#ifdef DEBUG
     register unsigned short ii;
     
     static bool init = TRUE;
#endif
     const size_t nr_byte = SCIENCE_PIXELS * sizeof(float);
/*
 * read constant dark parameters from SDMF database
 */
     switch ( fileParam->sdmf_version ) {
     case 24:
	  found = SDMF_get_FittedDark_24( 0, fileParam->absOrbit, 
					  DarkData->AnalogOffs, 
					  DarkData->DarkCurrent, 
					  DarkData->AnalogOffsError,
					  DarkData->DarkCurrentError, NULL );
	  break;
     case 30:
	  found = SDMF_get_FittedDark_30( 0, fileParam->absOrbit, 
					  DarkData->AnalogOffs, 
					  DarkData->DarkCurrent, 
					  DarkData->AnalogOffsError,
					  DarkData->DarkCurrentError, NULL );
	  break;
     default:
	  found = SDMF_get_FittedDark( 0, fileParam->absOrbit, 
				       DarkData->AnalogOffs, 
				       DarkData->DarkCurrent, 
				       DarkData->AnalogOffsError,
				       DarkData->DarkCurrentError,
				       DarkData->MeanNoise, NULL, NULL );
	  break;
     }
     if ( IS_ERR_STAT_FATAL ) 
          NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "SDMF_get_FittedDark" );
     if ( ! found ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_NONE, "no SDMF Dark data" );
/*
 * invert channel 2 dark parameters
 */
     __Inverse_Chan2( DarkData->AnalogOffs );
     __Inverse_Chan2( DarkData->AnalogOffsError );
     __Inverse_Chan2( DarkData->DarkCurrent );
     __Inverse_Chan2( DarkData->DarkCurrentError );
/*
 * obtain meanNoise for SDMF version < 3.1
 */
     if ( fileParam->sdmf_version == 31 ) {
	  __Inverse_Chan2( DarkData->MeanNoise );
     } else {
	  struct clcp_scia  clcp;

	  (void) SCIA_LV1_RD_CLCP( fileParam->fp, fileParam->num_dsd, 
				   fileParam->dsd, &clcp );
	  (void) memcpy( DarkData->MeanNoise, clcp.mean_noise, nr_byte );
     }
/*
 * reset values, if requested
 */
     if ( (fileParam->calibFlag & DO_CORR_AO) == UINT_ZERO ) {
	  (void) memset( DarkData->AnalogOffs, 0, nr_byte );
	  (void) memset( DarkData->AnalogOffsError, 0, nr_byte );
     }

     if ( (fileParam->calibFlag & DO_CORR_DARK) == UINT_ZERO ) {
	  (void) memset( DarkData->DarkCurrent, 0, nr_byte );
	  (void) memset( DarkData->DarkCurrentError, 0, nr_byte );
     }
#ifdef DEBUG
     if ( init ) {
	  for ( ii = 7*1024; ii < 8*1024; ii++ ) {
	       (void) fprintf( stderr, "%5hu %14.6f %14.6f\n", ii, 
			       DarkData->AnalogOffs[ii], 
			       DarkData->DarkCurrent[ii] );
	  }
	  init = FALSE;
     }
#endif
}

/*+++++++++++++++++++++++++
.IDENTifer   addOrbitDarkSDMF
.PURPOSE     Add variable darkcurrent correction values from SDMF
.INPUT/OUTPUT
  call as    addOrbitDarkSDMF( fileParam, orbit_phase, &DarkData );
     input:
	   float orbit_phase         :  orbit phase
	   struct file_rec fileParam :  file/calibration parameters
 in/output:
	   struct DarkRec *DarkData  :  record holding Dark Calibration data

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void addOrbitDarkSDMF( const struct file_rec *fileParam, 
		       float orbit_phase, struct DarkRec *DarkData )
     /*@globals  errno, nadc_stat, nadc_err_stack@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, DarkData@*/
{
     const char prognm[] = "addOrbitDarkSDMF";

     bool   found;
     float  ao[CHANNEL_SIZE], ao_err[CHANNEL_SIZE];
     float  lc[CHANNEL_SIZE], lc_err[CHANNEL_SIZE];

     const size_t offs_ch8   = 7 * CHANNEL_SIZE;
/*
 * read variable portion of the dark-current parameters
 */
     if ( fileParam->sdmf_version == 24 ) {
	  found = SDMF_get_OrbitalDark_24( fileParam->absOrbit, orbit_phase,
					   ao, lc, ao_err, lc_err );
	  (void) memcpy( DarkData->DarkCurrent+offs_ch8,
			 lc, CHANNEL_SIZE * sizeof(float) );
     } else {
	  found = SDMF_get_OrbitalDark_30( fileParam->absOrbit, orbit_phase,
					   ao, lc, ao_err, lc_err );
	  (void) memcpy( DarkData->AnalogOffs+offs_ch8,
			 ao, CHANNEL_SIZE * sizeof(float) );
	  (void) memcpy( DarkData->AnalogOffsError+offs_ch8,
			 ao_err, CHANNEL_SIZE * sizeof(float) );
	  (void) memcpy( DarkData->DarkCurrent+offs_ch8,
			 lc, CHANNEL_SIZE * sizeof(float) );
	  (void) memcpy( DarkData->DarkCurrentError+offs_ch8,
			 lc_err, CHANNEL_SIZE * sizeof(float) );
     }
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "OrbitDARK" );
     if ( ! found )
	  NADC_ERROR( prognm, NADC_ERR_NONE, "no SDMF orbitalDark data" );
}

/*+++++++++++++++++++++++++
.IDENTifer   applyDarkCorr
.PURPOSE     apply Dark Current correction
.INPUT/OUTPUT
  call as    applyDarkCorr( intg, DarkData, mds_1c );

     input:
	   float intg                  :  integration time
	   struct DarkRec *DarkData    :  record holding Dark Calibration data
 in/output:
	   struct mds1c_scia *mds_1c   :  level 1c MDS records

.RETURNS     nothing
.COMMENTS    static function
             The variable DarkCurrent may include constant part, 
             variable part, and straylight
-------------------------*/
static inline 
void applyDarkCorr( float intg, const struct DarkRec *DarkData, 
		    struct mds1c_scia *mds_1c )
     /*@modifies mds_1c->pixel_val@*/
{
     register unsigned short nobs = 0;
     register float *signal = mds_1c->pixel_val;

/* constants */
     register const float *analog_pntr = 
	  DarkData->AnalogOffs + mds_1c->pixel_ids[0];
     register const float *dark_pntr   = 
	  DarkData->DarkCurrent + mds_1c->pixel_ids[0];
/*
 * apply dark correction
 */
     do {
	  register unsigned short npix = 0;
	  do {
	       *signal++ -= ((float) mds_1c->coaddf * analog_pntr[npix] 
			     + intg * dark_pntr[npix]);
	  } while( ++npix < mds_1c->num_pixels );
     } while ( ++nobs < mds_1c->num_obs );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_SRON_CAL_DARK( const struct file_rec *fileParam,
			 const struct state1_scia *state,
			 struct mds1c_scia *mds_1c )
{
     const char prognm[] = "SCIA_SRON_CAL_DARK";

     register unsigned short num = 0u;     /* counter for number of clusters */

     static struct DarkRec DarkData_Save;
     struct DarkRec DarkData;

     const bool do_vardark = 
	((fileParam->calibFlag & (DO_CORR_VDARK|DO_CORR_VSTRAY)) != UINT_ZERO);
/*
 * read calibration data for DarkCurrent correction
 */
     if ( fileParam->flagInitFile ) {
	  readConstDarkSDMF( fileParam, &DarkData_Save );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "DARK" );
     }
     (void) memcpy( &DarkData, &DarkData_Save, sizeof( struct DarkRec ) );
     if ( fileParam->flagInitFile || fileParam->flagInitPhase ) {
          if ( do_vardark ) {
	       addOrbitDarkSDMF( fileParam, state->orbit_phase, &DarkData );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "OrbitDARK" );
	  }
     }
/*
 * do actual dark current correction
 */
     do {
	  const float intg = getCorrIntg( state->Clcon[num] );

	  if ( (fileParam->calibFlag & DO_CALC_ERROR) != UINT_ZERO ) {
	       const float electron_bu = 
		    fileParam->electron_bu[mds_1c->chan_id-1];

	       calcShotNoise( electron_bu, &DarkData, mds_1c );
	  }
	  applyDarkCorr( intg, &DarkData, mds_1c );
	  if ( (fileParam->calibFlag & DO_CALC_ERROR) != UINT_ZERO )
	       calcDarkError( intg, &DarkData, mds_1c );

     } while ( ++mds_1c, ++num < state->num_clus );   
}
