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

.IDENTifer   calibAtbdDark
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy - Level 1b - Calibration
.LANGUAGE    ANSI C
.PURPOSE     perform dark current correction on Sciamachy L1b science data
.INPUT/OUTPUT
  call as   SCIA_ATBD_CAL_DARK( fileParam, state, mds_1c );
     input:  
             struct file_rec *fileParam : file/calibration parameters
	     struct state1_scia *state  : structure with States of the product
 in/output:  
             struct mds1c_scia *mds_1c  : level 1c MDS records
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     4.2   14-May-2013 fixed longstanding issue with GADS orbit-phase
                               interpolation, RvH
             4.1   21-Aug-2009 moved all error functions to include file, RvH
             4.0   22-Jan-2009 combined ATBD and SRON/SDMF implementation
                               moved to SDMF v3, RvH
             3.0   09-Nov-2007 seperate ATBD, SRON/SDMF and ADS method, RvH
             2.0   06-Nov-2007 added readDarkDataADARK, applyDarkDataADARK, RvH
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

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_sdmf.h>
#include <nadc_scia_cal.h>

#include "getCorrIntg.inc"
#define __NEED_DARK_ERROR__
#include "calibCalcError.inc"

/*+++++ Macros +++++*/
#define DoChanVLC(source, nchan) ((source == SCIA_LIMB \
              && strncmp(&sip.do_var_lc_cha[nchan*4],"LIMB",4) == 0) \
              || strncmp(&sip.do_var_lc_cha[nchan*4],"ALL",3) == 0)

#define DoChanVSTRAY(source, nchan) ((source == SCIA_LIMB \
              && strncmp(&sip.do_stray_lc_cha[nchan*4],"LIMB",4) == 0) \
              || strncmp(&sip.do_stray_lc_cha[nchan*4],"ALL",3) == 0)

/*+++++ Static Variables +++++*/
static const size_t nr_byte = SCIENCE_PIXELS * sizeof(float);

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   readDarkDataADS
.PURPOSE     Read darkcurrent correction values from level 1b product
.INPUT/OUTPUT
  call as    readDarkDataADS( fp, num_dsd, dsd, &DarkData );
     input:
           FILE *fp                  :  (open) stream pointer
	   unsigned int num_dsd      :  number of DSD's
	   struct dsd_envi *dsd      :  DSD's records
    output:
	   struct DarkRec *DarkData  :  record holding Dark Calibration data

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void readDarkDataADS( unsigned int calib_flag, FILE *fp, 
		      unsigned int num_dsd, const struct dsd_envi *dsd,
		      struct DarkRec *DarkData )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp, DarkData@*/
{
     const char prognm[] = "readDarkDataADS";

     struct clcp_scia  clcp;
/*
 * read CLCP
 */
     (void) SCIA_LV1_RD_CLCP( fp, num_dsd, dsd, &clcp );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "CLCP" );
/*
 * store data in dark-record
 */
     if ( (calib_flag & DO_CORR_AO) == UINT_ZERO ) { 
	  (void) memset( DarkData->AnalogOffs, 0, nr_byte );
	  (void) memset( DarkData->AnalogOffsError, 0, nr_byte );
     } else {
	  (void) memcpy( DarkData->AnalogOffs, clcp.fpn, nr_byte );
	  (void) memcpy( DarkData->AnalogOffsError, clcp.fpn_error, nr_byte );
     }
     if ( (calib_flag & DO_CORR_DARK) == UINT_ZERO ) {
	  (void) memset( DarkData->DarkCurrent, 0, nr_byte );
	  (void) memset( DarkData->DarkCurrentError, 0, nr_byte );
     } else {
	  (void) memcpy( DarkData->DarkCurrent, clcp.lc, nr_byte );
	  (void) memcpy( DarkData->DarkCurrentError, clcp.lc_error, nr_byte );
     }
     (void) memcpy( DarkData->MeanNoise, clcp.mean_noise, nr_byte );
}

/*+++++++++++++++++++++++++
.IDENTifer   addOrbitDarkADS
.PURPOSE     Add variable darkcurrent correction values from level 1b product
.INPUT/OUTPUT
  call as    addOrbitDarkADS( source, fp, num_dsd, dsd, orbit_phase, &DarkData);
     input:
           int source                :  type of observation
           FILE *fp                  :  (open) stream pointer
	   unsigned int num_dsd      :  number of DSD's
	   struct dsd_envi *dsd      :  DSD's records
	   float orbit_phase         :  orbit phase
 in/output:
	   struct DarkRec *DarkData  :  record holding Dark Calibration data

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void addOrbitDarkADS( int source, unsigned int calib_flag,
		      FILE *fp, unsigned int num_dsd, 
		      const struct dsd_envi *dsd, float orbit_phase,
		      struct DarkRec *DarkData )
     /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp, DarkData@*/
{
     const char prognm[] = "addOrbitDarkADS";

     register unsigned int  nd;

     unsigned int num_dsr;
     unsigned int nd_low = 0;

     struct sip_scia  sip;
     struct vlcp_scia *vlcp;
     struct vlcp_scia *vlcp_orig;

     const bool Save_Extern_Alloc = Use_Extern_Alloc;
/*
 * read Static Instrument Parameters
 */
     (void) SCIA_LV1_RD_SIP( fp, num_dsd, dsd, &sip );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "SIP" );
/*
 * read variable portion of the dark-current parameters
 */
     Use_Extern_Alloc = FALSE;
     num_dsr = SCIA_LV1_RD_VLCP( fp, num_dsd, dsd, &vlcp_orig );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "VLCP" );
     Use_Extern_Alloc = Save_Extern_Alloc;
/*
 * extend vlcp with two records to cover orbit phases between zero and one
 */
     vlcp = (struct vlcp_scia *) 
	  malloc( (num_dsr + 2) * sizeof( struct vlcp_scia ) );
     if ( vlcp == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vlcp" );
     (void) memcpy( vlcp, vlcp_orig+(num_dsr-1), sizeof( struct vlcp_scia ) );
     (void) memcpy( vlcp+1, vlcp_orig, num_dsr * sizeof( struct vlcp_scia ) );
     (void) memcpy( vlcp+(num_dsr+1), vlcp_orig, sizeof( struct vlcp_scia ) );
     free( vlcp_orig );
     num_dsr += 2;

     /* correct the orbit_phases */
     vlcp[num_dsr-1].orbit_phase = 1.f;
     for ( nd = 1; nd < num_dsr-1; nd++ ) {
	  vlcp[nd].orbit_phase = 
	       (vlcp[nd].orbit_phase + vlcp[nd+1].orbit_phase) / 2.f;
     }
     vlcp[0].orbit_phase = vlcp[num_dsr-2].orbit_phase - 1.f;
     vlcp[num_dsr-1].orbit_phase = vlcp[1].orbit_phase + 1.f;
/*
 * find vlcp record with orbit_phase just smaller than requested orbit_phase
 */
     nd = 0;
     do {
	  if ( orbit_phase < vlcp[nd].orbit_phase ) break;
     } while ( ++nd < num_dsr );
     if ( nd == num_dsr ) 
	  nd_low = nd - 2;
     else if ( nd > 0 ) 
	  nd_low = nd - 1;
/*
 * variable fraction of the leakage current
 */
     if ( (calib_flag & DO_CORR_VDARK) != UINT_ZERO ) {
	  register unsigned short nch = 0;
	  register unsigned short npix = 0;

	  float *pntr_dark  = DarkData->DarkCurrent + VIS_SCIENCE_PIXELS;
	  float *pntr_error = DarkData->DarkCurrentError + VIS_SCIENCE_PIXELS;

	  const float frac = (orbit_phase - vlcp[nd_low].orbit_phase)
	       / (vlcp[nd_low+1].orbit_phase - vlcp[nd_low].orbit_phase);

	  do {
	       if ( DoChanVLC(source, nch) ) {
		    register unsigned short nc = 0;

		    do {
			 pntr_dark[npix] += 
			      (1 - frac) * vlcp[nd_low].var_lc[npix] 
			      + frac * vlcp[nd_low+1].var_lc[npix];
			 pntr_error[npix] += vlcp[nd_low].var_lc_error[npix];
		    } while ( ++npix, ++nc < CHANNEL_SIZE );
	       } else 
		    npix += CHANNEL_SIZE;
	  } while ( ++nch < IR_CHANNELS );
     }
/*
 * variable fraction of the Solar straylight
 */
     if ( (calib_flag & DO_CORR_VSTRAY) != UINT_ZERO ) {
	  register unsigned short nch = 0;
	  register unsigned short npix = 0;

	  const float frac = (orbit_phase - vlcp[nd_low].orbit_phase)
	       / (vlcp[nd_low+1].orbit_phase - vlcp[nd_low].orbit_phase);

	  do {
	       if ( DoChanVSTRAY(source, nch) ) {
		    register unsigned short nc = 0;

		    do {
			 DarkData->DarkCurrent[npix] += 
			      (1 - frac) * vlcp[nd_low].solar_stray[npix] 
			      + frac * vlcp[nd_low+1].solar_stray[npix];
			 DarkData->DarkCurrentError[npix] += 
			      vlcp[nd_low].solar_stray_error[npix];
		    } while ( ++npix, ++nc < CHANNEL_SIZE );
	       } else
		    npix += CHANNEL_SIZE;
	  } while ( ++nch < SCIENCE_CHANNELS );
     }
     free( vlcp );
}

/*+++++++++++++++++++++++++
.IDENTifer   applyDarkCorrLimb
.PURPOSE     apply Dark Current correction on Limb measurements
.INPUT/OUTPUT
  call as    applyDarkCorrLimb( mds_1c );

 in/output:
	   struct mds1c_scia *mds_1c :  level 1c MDS records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
void applyDarkCorrLimb( struct mds1c_scia *mds_1c )
     /*@modifies mds_1c->pixel_val@*/
{
     register unsigned short nobs;
     register float *signal = mds_1c->pixel_val;

     float LimbDark[CHANNEL_SIZE];

     const unsigned short NumIntgDarkLimb = 
	  (unsigned short) ( 0.5 + 1.5 / (mds_1c->coaddf * mds_1c->pet) );
/*
 * State 27 is a Mesoric limb state (category 26), 
 *  performing a scan from 150 to 75 km
 */
     size_t DarkOffs = (mds_1c->state_id == 27) ? 
	  0 : (size_t) (mds_1c->num_obs-1) * mds_1c->num_pixels;
/*
 * Each Limb scan lasts 1.5 s and can consist of several readouts for 
 * individual clusters, if the integration time (IT) of the cluster is 
 * smaller than 1.5s. Here we use the *average* of all readouts found 
 * in the last scan. Using the average of the scan will reduce the noise 
 * and is thus preferable.
 */
     (void) memset( LimbDark, 0, mds_1c->num_pixels * sizeof(float) );
     for ( nobs = 0; nobs < NumIntgDarkLimb; nobs++ ) {
	  register unsigned short npix = 0;

	  do {
	       LimbDark[npix] += mds_1c->pixel_val[DarkOffs + npix];
	  } while ( ++npix < mds_1c->num_pixels );
	  
	  if ( mds_1c->state_id == 27 ) 
	       DarkOffs += mds_1c->num_pixels;
	  else
	       DarkOffs -= mds_1c->num_pixels;
     }
/*
 * apply limb dark correction
 */
     nobs = 0;
     do {
	  register unsigned short npix = 0;
	  do {
	       *signal++ -= (LimbDark[npix] / NumIntgDarkLimb);
	  } while( ++npix < mds_1c->num_pixels );
     } while ( ++nobs < mds_1c->num_obs );
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
void SCIA_ATBD_CAL_DARK( const struct file_rec *fileParam,
			 const struct state1_scia *state,
			 struct mds1c_scia *mds_1c )
{
     const char prognm[] = "SCIA_ATBD_CAL_DARK";

     register unsigned short num = 0u;     /* counter for number of clusters */

     static struct DarkRec DarkData_Save;
     struct DarkRec DarkData;

     const int source = (int) mds_1c->type_mds;
     const bool do_limbdark = 
	  (source == SCIA_LIMB && 
	   (fileParam->calibFlag & DO_CORR_LDARK) != UINT_ZERO);
     const bool do_vardark = 
	((fileParam->calibFlag & (DO_CORR_VDARK|DO_CORR_VSTRAY)) != UINT_ZERO);
/*
 * read calibration data for DarkCurrent correction
 */
     if ( fileParam->flagInitFile ) {
	  readDarkDataADS( fileParam->calibFlag, fileParam->fp, 
			   fileParam->num_dsd, fileParam->dsd, &DarkData_Save );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "DARK" );
     }
     (void) memcpy( &DarkData, &DarkData_Save, sizeof( struct DarkRec ) );
     if ( fileParam->flagInitFile || fileParam->flagInitPhase ) {
          if ( do_vardark ) {
	       addOrbitDarkADS( source, fileParam->calibFlag, fileParam->fp, 
				fileParam->num_dsd, fileParam->dsd, 
				state->orbit_phase, &DarkData );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "OrbitDARK" );
	  }
     }
/*
 * do actual dark current correction
 */
     do {
	  if ( (fileParam->calibFlag & DO_CALC_ERROR) != UINT_ZERO ) {
	       const float electron_bu = 
		    fileParam->electron_bu[mds_1c->chan_id-1];

	       calcShotNoise( electron_bu, &DarkData, mds_1c );
	  }
	  if ( do_limbdark ) {
	       applyDarkCorrLimb( mds_1c );
 	       if ( (fileParam->calibFlag & DO_CALC_ERROR) != UINT_ZERO )
 		    calcDarkErrorLimb( mds_1c );
	  } else {
	       const float intg = getCorrIntg( state->Clcon[num] );

	       applyDarkCorr( intg, &DarkData, mds_1c );
 	       if ( (fileParam->calibFlag & DO_CALC_ERROR) != UINT_ZERO )
		    calcDarkError( intg, &DarkData, mds_1c );
	  }
     } while ( ++mds_1c, ++num < state->num_clus );   
}

void SCIA_get_AtbdDark( FILE *fp, unsigned int calib_flag, float orbit_phase,
			/*@out@*/ float *analogOffs, 
			/*@out@*/ float *darkCurrent, 
			/*@out@*/ /*@null@*/ float *analogOffsError, 
			/*@out@*/ /*@null@*/ float *darkCurrentError,
			/*@out@*/ /*@null@*/ float *meanNoise )
     /*@globals  errno, nadc_stat, nadc_err_stack@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp@*/
{
     const char prognm[] = "SCIA_get_AtbdDark";

     unsigned int num_dsd;

     struct mph_envi   mph;
     struct dsd_envi   *dsd = NULL;

     struct DarkRec DarkData;

     const bool do_vardark = 
	((calib_flag & (DO_CORR_VDARK|DO_CORR_VSTRAY)) != UINT_ZERO);
/*
 * read variable portion of the dark-current parameters
 */
     ENVI_RD_MPH( fp, &mph );
     dsd = (struct dsd_envi *) 
	  malloc( (mph.num_dsd-1) * sizeof(struct dsd_envi) );
     if ( dsd == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "dsd" );
     num_dsd = ENVI_RD_DSD( fp, mph, dsd );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "DSD" );
/*
 * read calibration data for DarkCurrent correction
 */
     readDarkDataADS( calib_flag, fp, num_dsd, dsd, &DarkData );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "DarkADS" );
     if ( do_vardark ) {
	  addOrbitDarkADS( SCIA_NADIR, calib_flag, fp, num_dsd, dsd, 
			   orbit_phase, &DarkData );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "OrbitDARK" );
     }
     (void) memcpy( analogOffs, DarkData.AnalogOffs, nr_byte );
     (void) memcpy( darkCurrent, DarkData.DarkCurrent, nr_byte );
     if ( analogOffsError != NULL )
	  (void) memcpy( analogOffsError, DarkData.AnalogOffsError, nr_byte );
     if ( darkCurrentError != NULL )
	  (void) memcpy( darkCurrentError, DarkData.DarkCurrentError, nr_byte );
     if ( meanNoise != NULL )
	  (void) memcpy( meanNoise, DarkData.MeanNoise, nr_byte );
done:
     if ( dsd != NULL ) free( dsd );
}
