/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2003 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_PATCH_MDS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 1b
.LANGUAGE    ANSI C
.PURPOSE     modify (patch) SCIA level 1b measurement datasets
.INPUT/OUTPUT
  call as   SCIA_LV1_PATCH_MDS( fp, patch_scia, state, mds );
     input:  
	    FILE *fp                  :  file pointer
            unsigned short patch_scia :  bit-flag which defines what to patch
	    struct state1_scia *state :  structure with State definition
 in/output:  
            struct mds1_scia *mds     :  structure holding level 1b MDS records
.RETURNS     nothing
.COMMENTS    ToDo: handle MONITOR states correctly
                   handle PET < 1/32 correctly
		   handle spikes in the PMD readouts correctly
.ENVIRONment None
.VERSION     5.1     11-Jan-2013   fixed memory corruption bug which occured
                                   when not all channels where processed, RvH
             5.0     23-Aug-2011   re-write to include stray-light correction,
                                   RvH
             4.0     26-May-2010   updated memory correction to operational 
                                   processor version 8.x, RvH
             3.0     21-Jan-2009   updated memory correction
                                   use v6 coding of mem/nlin corr values, RvH
	     2.4     19-Nov-2007   added patching of straylight (ch2), RvH
	     2.2.1   22-Apr-2006   replaced n_pmd,int_pmd by NumIntPMD,IntPMD
	     2.2     14-Mar-2006   fixed stupid bug when n_pmd is zero, RvH
	     2.1     20-Feb-2006   use float version of round-off functions
	     2.0     07-Dec-2005   implemented PMD scaling for co-added pixels,
		                   affects both memory and non-linearity 
		                   correction, RvH
	     1.2     07-Feb-2005   more checks on return status of functions,
                                   added patching of memory correction, RvH
             1.1     31-Mar-2004   added patching of polarisation values, RvH
             1.0     19-Jan-2004   initial release by R. M. van Hees
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
#include <limits.h>
#include <float.h>
#include <math.h>
 
#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#define _MEMNLIN_CORR
#include <nadc_scia_cal.h>
#include <nadc_sdmf.h>

/*+++++ Macros +++++*/
#define VIRTUAL_CHANNELS      (SCIENCE_CHANNELS + 2)
#define FLAG_VALID            ((unsigned char) 0x0U)
#define FLAG_BLINDED          ((unsigned char) 0x1U)
#define FLAG_UNUSED           ((unsigned char) 0x2U)
#define FLAG_DEAD             ((unsigned char) 0x4U)
#define FLAG_SATURATE         ((unsigned char) 0x8U)
#define FLAG_BAD              ((unsigned char) 0x10U)

/*+++++ Static Variables +++++*/
struct scia_cal_rec {
     unsigned char    state_id;
     unsigned char    state_index;
     unsigned short   abs_orbit;
     unsigned short   limb_scans;       /* zero for non-Limb states */
     unsigned short   state_pet_min;
     int              type_mds;
     size_t           num_obs;          /* observation at pet_min */
     size_t           num_channels;     /* SCIENCE_VIR_CHANNELS */
     size_t           num_pixels;       /* SCIENCE_PIXELS */
     float            orbit_phase;
     unsigned char    *coaddf;          /* [num_pixels] co-adding factor */
     unsigned char    *chan_id;         /* [num_pixels] channel ID [1..8] */
     unsigned char    *clus_id;         /* [num_pixels] cluster ID [1..] */
     unsigned char    *quality_flag;    /* [num_pixels] zero is ok */
     unsigned short   *chan_pet;        /* [num_channels] in BCPS */
     unsigned short   *chan_obs;        /* [num_channels] per State execution */
     float            *dark_signal;     /* [num_pixels] dark at PET[chan] */
     float            *pet;             /* [num_pixels] 6-8 not corrected */
     double           **chan_mean;      /* [num_channels][num_obs] */
     double           **spectra;        /* [num_pixels][num_obs] */
     float            **correction;     /* [num_pixels][num_obs] */
};

/*+++++ Global Variables +++++*/

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static inline
unsigned short ABS_PIXELID( unsigned short ipx, struct Clcon_scia Clcon )
{
     short ichan = (short) Clcon.channel - 1;

     return (unsigned short) (ichan * CHANNEL_SIZE + Clcon.pixel_nr + ipx);
}

static inline
unsigned short VIRTUAL_CHANNEL( unsigned char chan_id, unsigned char clus_id )
{
     unsigned short vchan = USHRT_MAX;

     switch ( (int) chan_id ) {
     case 1:
	  vchan = ((int) clus_id < 4) ? 0 : 1;
	  break;
     case 2:
	  vchan = ((int) clus_id < 10) ? 3 : 2;
	  break;
     case 3:
	  vchan = 4;
	  break;
     case 4:
	  vchan = 5;
	  break;
     case 5:
	  vchan = 6;
	  break;
     case 6:
	  vchan = 7;
	  break;
     case 7:
	  vchan = 8;
	  break;
     case 8:
	  vchan = 9;
	  break;
     }
     return vchan;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_FILL_SPECTRA
.PURPOSE     convert level 1b readouts to spectral grid
.INPUT/OUTPUT
  call as    SCIA_FILL_SPECTRA( state, mds_1b, scia_cal );
     input:
            struct state1_scia *state     :  structure with State definition
	    struct mds1_scia *mds_1b      :
 in/output:
	    struct scia_cal_rec *scia_cal :  record with SCIA data of one state

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SCIA_FILL_SPECTRA( const struct state1_scia *state, 
			const struct mds1_scia *mds_1b,
			struct scia_cal_rec *scia_cal )
       /*@modifies scia_cal->spectra@*/
{
     register unsigned short ncl = 0;
     register unsigned short np;
     register unsigned short nr;
     register size_t nobs;

     /* initialize spectra with NaN values */
     for ( np = 0; np < SCIENCE_PIXELS; np++ ) {
	  for ( nobs = 0; nobs < scia_cal->num_obs; nobs++ )
	       scia_cal->spectra[np][nobs] = NAN;
     }

     do {
	  const unsigned short coaddf = state->Clcon[ncl].coaddf;
	  const unsigned short length = state->Clcon[ncl].length;
	  const unsigned short n_read = state->Clcon[ncl].n_read;

	  np = 0;
	  do {
	       register unsigned short nd = 0;

	       const unsigned short ipx = ABS_PIXELID(np, state->Clcon[ncl]);

	       nobs = 0;
	       do {
		    register size_t nb = np;

		    if ( mds_1b[nd].clus[ncl].n_sig > 0 ) {
			 for ( nr = 0; nr < n_read; nr++ ) {
			      scia_cal->spectra[ipx][nobs++] =
				   mds_1b[nd].clus[ncl].sig[nb].sign;
			      nb += length;
			 }
		    }
		    if ( mds_1b[nd].clus[ncl].n_sigc > 0 ) {
			 for ( nr = 0; nr < n_read; nr++ ) {
			      register unsigned short nc = 0;
			      register double val =
				   mds_1b[nd].clus[ncl].sigc[nb].det.field.sign;
			      do {
				   scia_cal->spectra[ipx][nobs++] = val;
			      } while ( ++nc < coaddf );
			      nb += length;
			 }
		    }
	       } while ( ++nd < state->num_dsr );
	  } while ( ++np < length );
     } while ( ++ncl < state->num_clus );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_SET_FLAG_QUALITY
.PURPOSE     set quality flags for spectral data
.INPUT/OUTPUT
  call as    SCIA_SET_FLAG_QUALITY( scia_cal );
 in/output:
	    struct scia_cal_rec *scia_cal :  record with SCIA data of one state

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SCIA_SET_FLAG_QUALITY( struct scia_cal_rec *scia_cal )
       /*@modifies scia_cal->quality_flag@*/
{
     register unsigned short np = 0;
     register size_t nobs;

     do {
	  register unsigned short num_valid = 0;
	  register double mean = 0.;

	  const short nchan = (short) scia_cal->chan_id[np] - 1;

	  if ( nchan < 0 ) {
	       scia_cal->quality_flag[np] = FLAG_UNUSED;
	       continue;
	  }

	  if ( (np % CHANNEL_SIZE) < 10 
	       || (np % CHANNEL_SIZE) >= (CHANNEL_SIZE - 10) ) {
	       scia_cal->quality_flag[np] = FLAG_BLINDED;
	       continue;
	  }
	       
	  for ( nobs = 0; nobs < scia_cal->num_obs; nobs++ ) {
	       if ( isnormal( scia_cal->spectra[np][nobs] ) ) {
		    mean += scia_cal->spectra[np][nobs];
		    num_valid++;
	       }
	  }
	  if ( num_valid == 0 ) {
	       scia_cal->quality_flag[np] = FLAG_DEAD;
	  } else if ( (mean /= num_valid) > (scia_cal->coaddf[np] * 60000.) ) {
	       scia_cal->quality_flag[np] = FLAG_SATURATE;
	  } else {
	       register double adev = 0.;

	       for ( nobs = 0; nobs < scia_cal->num_obs; nobs++ ) {
		    if ( isnormal( scia_cal->spectra[np][nobs] ) )
			 adev += fabs( scia_cal->spectra[np][nobs] - mean );
	       }
	       adev /= num_valid;

	       if ( mean <= DBL_EPSILON || adev <= DBL_EPSILON )
		    scia_cal->quality_flag[np] = FLAG_DEAD;
	       else
		    scia_cal->quality_flag[np] = FLAG_VALID;
	  }
     } while ( ++np < SCIENCE_PIXELS );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_APPLY_DARK
.PURPOSE     apply dark correction on (non-limb) spectra
.INPUT/OUTPUT
  call as    SCIA_APPLY_DARK( fp, calib_flag, scia_cal );
     input:
            FILE *fp                      : file pointer to SCIA L1b product
	    unsigned int calib_flag       : calibration flags (bitfield)
 in/output:
	    struct scia_cal_rec *scia_cal :  record with SCIA data of one state

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static inline
double _getMeanAbsDev( unsigned short dim, float median, const float *buff )
{
     register unsigned short nr = 0;
     register float spread = 0.;

     do {
          spread += fabsf( buff[nr] - median );
     } while ( ++nr < dim );
     return spread / dim;
}

static
void SCIA_APPLY_DARK( FILE *fp, unsigned int calib_flag, 
		      struct scia_cal_rec *scia_cal )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, 
	 scia_cal->dark_signal, scia_cal->spectra@*/
{
     const char prognm[] = "SCIA_CAL_GET_DARK";

     register unsigned short np = 0;
     register unsigned short nch;

     float analogOffs[SCIENCE_PIXELS], darkCurrent[SCIENCE_PIXELS];
     
     const float PET_Offset = 1.18125e-3;
     const float PET_noHotMode = 0.03125f;

     if ( (calib_flag & DO_SRON_DARK) == UINT_ZERO ) {
	  SCIA_get_AtbdDark( fp, calib_flag, scia_cal->orbit_phase,
			     analogOffs, darkCurrent, NULL, NULL, NULL );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "SCIA_get_AtbdDark" );
     } else {
	  (void) SDMF_get_FittedDark( scia_cal->abs_orbit, 0, 
				      analogOffs, darkCurrent, 
				      NULL, NULL, NULL, NULL, NULL );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR(prognm, NADC_ERR_FATAL, "SDMF_get_FittedDark");
     }
/*
 * calculate dark signal
 */
     if ( (calib_flag & DO_CORR_DARK) == UINT_ZERO ) {
	  do {
	       if ( isnormal(analogOffs[np]) )
		    scia_cal->dark_signal[np] = analogOffs[np];
	  } while ( ++np < SCIENCE_PIXELS );
     } else {
	  do {
	       float pet = scia_cal->pet[np];

	       if ( ! (isnormal(analogOffs[np]) && isnormal(darkCurrent[np])) )
		    continue;

	       if ( pet < FLT_EPSILON ) continue;

	       if ( np >= (VIS_CHANNELS * CHANNEL_SIZE) 
		    && pet >= (PET_noHotMode - FLT_EPSILON) ) pet -= PET_Offset;

	       scia_cal->dark_signal[np] = 
		    analogOffs[np] + pet * darkCurrent[np];
	  } while ( ++np < SCIENCE_PIXELS );
     }
/*
 * set quality flag for real outlayers
 */
     for ( nch = VIS_CHANNELS; nch < SCIENCE_CHANNELS; nch++ ) {
	  const float *pntr = scia_cal->dark_signal + (nch * CHANNEL_SIZE);

	  const float median = SELECTr( (CHANNEL_SIZE)/2, CHANNEL_SIZE, pntr );
	  const float limit = 4 * _getMeanAbsDev( CHANNEL_SIZE, median, pntr );

	  np = 0;
	  do {
	       unsigned short ipx = nch * CHANNEL_SIZE + np;

	       if ( scia_cal->quality_flag[ipx] != FLAG_VALID ) continue;
	       if ( pntr[np] <= FLT_EPSILON 
		    || fabsf( pntr[np] - median ) > limit )
		    scia_cal->quality_flag[ipx] = FLAG_BAD;
	  } while ( ++np < CHANNEL_SIZE );
     }
/*
 * subtract dark from spectra
 */
     np = 0;
     do {
	  register size_t nobs = 0;
	  register double dark =
	       scia_cal->coaddf[np] * scia_cal->dark_signal[np];

	  do {
	       if ( isnormal( scia_cal->spectra[np][nobs] ) )
		    scia_cal->spectra[np][nobs] -= dark;
	  } while ( ++nobs < scia_cal->num_obs );
     } while ( ++np < SCIENCE_PIXELS );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_CALC_CHAN_MEAN
.PURPOSE     estimate channel average signal based on PMD measurements
.INPUT/OUTPUT
  call as    SCIA_CALC_CHAN_MEAN( state, mds_1b, scia_cal );
     input:
            struct state1_scia *state     :  structure with State definition
	    struct mds1_scia *mds_1b      :
 in/output:
	    struct scia_cal_rec *scia_cal :  record with SCIA data of one state

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SCIA_CALC_CHAN_MEAN( const struct state1_scia *state, 
			  const struct mds1_scia *mds_1b, 
			  struct scia_cal_rec *scia_cal )
{
     register unsigned short nch;

     float pmd_offs[VIRTUAL_CHANNELS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

     /* science channel 2 PMD mapping: 1-A, 2-A, 3-B, 4-C, 5-D, 6-E, 7-F, 8-F */
     const unsigned short Chan2PmdIndx[VIRTUAL_CHANNELS] = { 
	  0, 0, 0, 0, 1, 2, 3, 4, 5, 5 
     };

     /* initialize channel average to zero (or one for Monitoring states */ 
     if ( scia_cal->type_mds == SCIA_MONITOR ) {
	  nch = 0;
	  do { 
	       register size_t nobs = 0;
	       do {
		    scia_cal->chan_mean[nch][nobs] = 1.;
	       } while ( ++nobs < scia_cal->num_obs );
	  } while ( ++nch < VIRTUAL_CHANNELS );
	  return;                                      /* nothing left todo */
     } else {
	  nch = 0;
	  do { 
	       register size_t nobs = 0;
	       do {
		    scia_cal->chan_mean[nch][nobs] = 0.;
	       } while ( ++nobs < scia_cal->num_obs );
	  } while ( ++nch < VIRTUAL_CHANNELS );
     }

     /* determine minimum of PMD values */
     nch = 0;
     do {
	  register unsigned short nd  = 0;

	  do {
	       register unsigned short indx = Chan2PmdIndx[nch];

	       while ( indx < mds_1b[nd].n_pmd ) {
		    if ( mds_1b[nd].int_pmd[indx] < pmd_offs[nch] ) 
			 pmd_offs[nch] = floorf(mds_1b[nd].int_pmd[indx]);

		    indx += PMD_NUMBER;
	       }
	  } while ( ++nd < state->num_dsr );
     } while ( ++nch < VIRTUAL_CHANNELS );

     /* calculate scaling factors for ca-added readouts */
     nch = 0;
     do {
	  register unsigned short nd  = 0;
	  register unsigned short nobs = 0;

	  const unsigned short NumPmdPet = 2 * scia_cal->chan_pet[nch];

	  do {
	       register unsigned short indx = Chan2PmdIndx[nch];

	       while ( indx < mds_1b[nd].n_pmd ) {
		    register unsigned short np = 0;

		    do {
			 scia_cal->chan_mean[nch][nobs] += 
			      (mds_1b[nd].int_pmd[indx] - pmd_offs[nch]);

			 indx += PMD_NUMBER;
		    } while ( ++np < NumPmdPet );

		    scia_cal->chan_mean[nch][nobs] /= NumPmdPet;
		    nobs++;
	       }
	  } while ( ++nd < state->num_dsr );
     } while ( ++nch < VIRTUAL_CHANNELS );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_CAL_DE_COADD
.PURPOSE     de-coadd the science data read-outs using PMD weigth values
.INPUT/OUTPUT
  call as    SCIA_CAL_DE_COADD( scia_cal );
 in/output:
	    struct scia_cal_rec *scia_cal :  record with SCIA data of one state

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SCIA_CAL_DE_COADD( struct scia_cal_rec *scia_cal )
       /*@modifies scia_cal->spectra@*/
{
     register unsigned short np = 0;

     do {
	  register unsigned short no = 0;

	  unsigned short vchan = VIRTUAL_CHANNEL( scia_cal->chan_id[np],
						  scia_cal->clus_id[np] );

	  if ( vchan == USHRT_MAX ) continue;         /* skip un-used pixels */
	  if ( scia_cal->coaddf[np] <= 1 ) continue;      /* only coaddf > 1 */

	  do {
	       register unsigned char nc;

	       register unsigned short noo = no;

	       register double wght = 0.;

	       for ( nc = 0; nc < scia_cal->coaddf[np]; nc++ ) {
		    wght += scia_cal->chan_mean[vchan][noo];
		    noo++;
	       }

	       for ( nc = 0; nc < scia_cal->coaddf[np]; nc++ ) {
		    if ( wght > 100. ) {
			 scia_cal->spectra[np][no] *= 
			      (scia_cal->chan_mean[vchan][no] / wght);
		    } else {
			 scia_cal->spectra[np][no] *= 
			      (1. / scia_cal->coaddf[np]);
		    }
		    no++;
	       }
	  } while ( no < scia_cal->chan_obs[vchan] );
     } while ( ++np < SCIENCE_PIXELS );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_CALC_MEM_CORR
.PURPOSE     correct measurements for memory effects
.INPUT/OUTPUT
  call as    SCIA_CALC_MEM_CORR( scia_cal );
 in/output:
	    struct scia_cal_rec *scia_cal :  record with SCIA data of one state

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void SCIA_CALC_MEM_CORR( struct scia_cal_rec *scia_cal )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, scia_cal->correction@*/
{
     const char prognm[] = "SCIA_CALC_MEM_CORR";

     register unsigned short np = 0;
     register unsigned short signNorm;
     register double         sign_before, sign_after;

     struct scia_memcorr memcorr = {{0,0}, NULL};

     const size_t num_tan_h = (scia_cal->limb_scans == 0) ? 0 : 
	  scia_cal->num_obs / scia_cal->limb_scans;

     /* state set-up time in milli-seconds */
     const float setup_it[] = { 0.f,
          421.875, 421.875, 421.875, 421.875, 421.875, 421.875,
          421.875, 421.875, 421.875, 421.875, 421.875, 421.875,
          421.875, 421.875, 421.875, 1269.53125, 421.875, 421.875,
          421.875, 421.875, 421.875, 421.875, 421.875, 421.875,
          421.875, 421.875, 421.875, 421.875, 421.875, 421.875,
          421.875, 421.875, 421.875, 421.875, 421.875, 421.875,
          421.875, 421.875, 421.875, 421.875, 421.875, 421.875,
          421.875, 421.875, 421.875, 421.875, 421.875, 1269.53125,
          421.875, 421.875, 421.875, 421.875, 421.875, 421.875,
          421.875, 421.875, 421.875, 421.875, 519.53125, 421.875,
          1269.53125, 421.875, 421.875, 421.875, 335.9375, 421.875,
          421.875, 421.875, 519.53125, 1269.53125
     };

     /* read memory correction values */
     SCIA_RD_H5_MEM( &memcorr );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, "SCIA_RD_H5_MEM" );

     do {
	  register unsigned short no = 0;

	  const short nchan = (short) scia_cal->chan_id[np] - 1;

	  /* limb only, before reset at new tangent height */
	  const double scale_before = (nchan < 0) ? 0. :
	       3 * (0.5 - (np % 1024) / 6138.) / (16 * scia_cal->pet[np]);
	  /* limb only, after reset at new tangent height */
	  const double scale_after  = (nchan < 0) ? 0. :
	       3 * (0.5 + (np % 1024) / 6138.) / (16 * scia_cal->pet[np]);

	  /* reset at start of new state execution */
	  const double scale_reset = (nchan < 0) ? 0. :
	       setup_it[scia_cal->state_id] / (1000. * scia_cal->pet[np]);

	  unsigned short vchan = VIRTUAL_CHANNEL( scia_cal->chan_id[np],
						  scia_cal->clus_id[np] );

	  if ( vchan == USHRT_MAX ) continue;         /* skip un-used pixels */

	  /* calculate memory correction for the first readout of a state */
	  signNorm = __ROUND_us( scia_cal->dark_signal[np]
				 + scale_reset * scia_cal->spectra[np][0] );
	  scia_cal->correction[np][0] = memcorr.matrix[nchan][signNorm];

	  /* use previous readout to calculate correction next readout */
	  while ( ++no < scia_cal->chan_obs[vchan] ) {
	       if ( ! isnormal(scia_cal->spectra[np][no]) ) continue;

	       if ( scia_cal->limb_scans != 0 && (no % num_tan_h) == 0 ) {
		    sign_before = scale_before * scia_cal->spectra[np][no-1];
		    sign_after  = scale_after * scia_cal->spectra[np][no];

		    signNorm = __ROUND_us( scia_cal->dark_signal[np]
					   + (sign_before + sign_after) );
	       } else {
		    signNorm = __ROUND_us( scia_cal->dark_signal[np]
					   + scia_cal->spectra[np][no-1] );
	       }
	       scia_cal->correction[np][no] = memcorr.matrix[nchan][signNorm];
	  }
     } while ( ++np < (VIS_CHANNELS * CHANNEL_SIZE) );

     SCIA_FREE_H5_MEM( &memcorr );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_CALC_NLIN_CORR
.PURPOSE     correct Nadir science measurements for non-linearity effects
.INPUT/OUTPUT
  call as    SCIA_CALC_NLIN_CORR( scia_cal );
 in/output:
	    struct scia_cal_rec *scia_cal :  record with SCIA data of one state

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void SCIA_CALC_NLIN_CORR( struct scia_cal_rec *scia_cal )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, scia_cal->correction@*/
{
     const char prognm[] = "SCIA_CALC_NLIN_CORR";

     register unsigned short np = (VIS_CHANNELS * CHANNEL_SIZE);

     register unsigned short signNorm;
     register unsigned short curveIndx;

     struct scia_nlincorr nlcorr = {{0,0}, NULL, NULL};

     SCIA_RD_H5_NLIN( NULL, &nlcorr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, "SCIA_RD_H5_NLIN" );

     do {
	  register unsigned short nobs = 0;

	  unsigned short vchan = VIRTUAL_CHANNEL( scia_cal->chan_id[np],
						  scia_cal->clus_id[np] );

	  if ( vchan == USHRT_MAX ) continue;         /* skip un-used pixels */

	  /* get index to curve to be used */
	  curveIndx = (unsigned short) nlcorr.curve[np];

	  /* set index to first readout */
	  do {
	       if ( ! isnormal(scia_cal->spectra[np][nobs]) ) continue;

	       signNorm = __ROUND_us( scia_cal->spectra[np][nobs] 
				      + scia_cal->dark_signal[np] );

	       scia_cal->correction[np][nobs] = 
		    nlcorr.matrix[curveIndx][signNorm];
	  } while ( ++nobs < scia_cal->chan_obs[vchan] );
     } while ( ++np < SCIENCE_PIXELS );

done:
     SCIA_FREE_H5_NLIN( &nlcorr );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_PATCH_MEM_CORR
.PURPOSE     patch memory correction values in SCIA L1b MDS
.INPUT/OUTPUT
  call as    SCIA_PATCH_MEM_CORR( scia_cal, state, mds_1b );
     input:
	    struct scia_cal_rec *scia_cal :  record with SCIA data of one state
            struct state1_scia *state     :  structure with State definition
 in/output:
	    struct mds1_scia *mds_1b      :  structure holding level 1b MDS

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
signed char MemNlin2Byte( unsigned char channel, float rval )
{
     int ibuff = 0;

     switch ( (int) channel ) {
     case 1: case 2: case 3: case 4: case 5:
	  ibuff = lroundf( rval / 1.25f - 37.f );
	  break;
     case 6:
	  ibuff = lroundf( rval / 1.25f - 102.f );
	  break;
     case 7:
	  ibuff = lroundf( rval / 1.5f + 126.f );
	  break;
     case 8:
	  ibuff = lroundf( rval / 1.25f + 126.f );
	  break;
     }
     return ((ibuff < SCHAR_MIN) ? (signed char) SCHAR_MIN :
	     (ibuff < SCHAR_MAX) ? (signed char) ibuff : 
	     (signed char) SCHAR_MAX);
}

static
void SCIA_PATCH_MEM_CORR( struct scia_cal_rec *scia_cal,
			  const struct state1_scia *state, 
			  struct mds1_scia *mds_1b )
       /*@modifies mds_1b@*/
{
     register unsigned short ncl = 0;

     do {
	  register unsigned short np = 0;

	  const unsigned short coaddf = state->Clcon[ncl].coaddf;
	  const unsigned short length = state->Clcon[ncl].length;
	  const unsigned short n_read = state->Clcon[ncl].n_read;

	  if ( state->Clcon[ncl].channel > VIS_CHANNELS ) continue;

	  do {
	       register unsigned short nd = 0;
	       register size_t nobs = 0;

	       const unsigned short ipx = ABS_PIXELID(np, state->Clcon[ncl]);

	       do {
		    register unsigned short nr;
		    register unsigned short nb = np;

		    if ( mds_1b[nd].clus[ncl].n_sig > 0 ) {
			 for ( nr = 0; nr < n_read; nr++ ) {
			      mds_1b[nd].clus[ncl].sig[nb].corr =
				   MemNlin2Byte( state->Clcon[ncl].channel, 
						 scia_cal->correction[ipx][nobs] );
			      nobs++;
			      nb += length;
			 }
		    }
		    if ( mds_1b[nd].clus[ncl].n_sigc > 0 ) {
			 for ( nr = 0; nr < n_read; nr++ ) {
			      register unsigned short nc = 0;
			      register float corrval = 0.f;

			      do {
				   corrval += scia_cal->correction[ipx][nobs++];
			      } while ( ++nc < coaddf );

			      mds_1b[nd].clus[ncl].sigc[nb].det.field.corr =
				   MemNlin2Byte( state->Clcon[ncl].channel, 
						 corrval / coaddf );
			      nb += length;
			 }
		    }
	       } while ( ++nd < state->num_dsr );
	  } while ( ++np < length );
      } while ( ++ncl < state->num_clus );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_PATCH_NL_CORR
.PURPOSE     patch non-linearity correction values in SCIA L1b MDS
.INPUT/OUTPUT
  call as    SCIA_PATCH_NL_CORR( scia_cal, state, mds_1b );
     input:
	    struct scia_cal_rec *scia_cal :  record with SCIA data of one state
            struct state1_scia *state     :  structure with State definition
 in/output:
	    struct mds1_scia *mds_1b      :  structure holding level 1b MDS

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SCIA_PATCH_NL_CORR( struct scia_cal_rec *scia_cal,
			 const struct state1_scia *state, 
			 struct mds1_scia *mds_1b )
       /*@modifies mds_1b@*/
{
     register unsigned short ncl = 0;

     do {
	  register unsigned short np = 0;

	  const unsigned short coaddf = state->Clcon[ncl].coaddf;
	  const unsigned short length = state->Clcon[ncl].length;
	  const unsigned short n_read = state->Clcon[ncl].n_read;

	  if ( state->Clcon[ncl].channel <= VIS_CHANNELS ) continue;

	  do {
	       register unsigned short nd = 0;
	       register size_t nobs = 0;

	       const unsigned short ipx = ABS_PIXELID(np, state->Clcon[ncl]);

	       if ( scia_cal->quality_flag[ipx] > FLAG_BLINDED ) continue;

	       do {
		    register unsigned short nr;
		    register size_t nb = np;

		    if ( mds_1b[nd].clus[ncl].n_sig > 0 ) {
			 for ( nr = 0; nr < n_read; nr++ ) {
			      mds_1b[nd].clus[ncl].sig[nb].corr =
				   MemNlin2Byte( state->Clcon[ncl].channel, 
						 scia_cal->correction[ipx][nobs] );
			      nobs++;
			      nb += length;
			 }
		    }
		    if ( mds_1b[nd].clus[ncl].n_sigc > 0 ) {
			 for ( nr = 0; nr < n_read; nr++ ) {
			      register unsigned short nc = 0;
			      register float corrval = 0.f;

			      do {
				   corrval += scia_cal->correction[ipx][nobs++];
			      } while ( ++nc < coaddf );

			      mds_1b[nd].clus[ncl].sigc[nb].det.field.corr =
				   MemNlin2Byte( state->Clcon[ncl].channel, 
						 corrval / coaddf );
			      nb += length;
			 }
		    }
	       } while ( ++nd < state->num_dsr );
	  } while ( ++np < length );
      } while ( ++ncl < state->num_clus );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_APPLY_MEMCORR
.PURPOSE     apply memory/non-linearity correction
.INPUT/OUTPUT
  call as    SCIA_APPLY_MEMCORR( struct scia_cal_rec *scia_cal )
 in/output:
	    struct scia_cal_rec *scia_cal :  record with SCIA data of one state

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
float MemNlin2Flt( short chanID, float corr )
{
     float rval = 0.f;

     switch ( chanID ) {
     case 1: case 2: case 3: case 4: case 5:
	  rval = (1.25f * (corr + 37.f));
	  break;
     case 6:
	  rval = (1.25f * (corr + 102.f));
	  break;
     case 7:
	  rval = (1.5f * (corr - 126.f));
	  break;
     case 8:
	  rval = (1.25f * (corr - 126.f));
	  break;
     }
     return rval;
}

static
void SCIA_APPLY_MemNlin( const struct state1_scia *state, 
			 const struct mds1_scia *mds_1b,
			 struct scia_cal_rec *scia_cal )
       /*@modifies scia_cal->spectra@*/
{
     register unsigned short ncl = 0;

     do {
	  register unsigned short np = 0;

	  const short nchan = (short) scia_cal->chan_id[np] - 1;

	  const unsigned short coaddf = state->Clcon[ncl].coaddf;
	  const unsigned short length = state->Clcon[ncl].length;
	  const unsigned short n_read = state->Clcon[ncl].n_read;

	  if ( nchan < 0 ) continue;

	  do {
	       register unsigned short nr;
	       register unsigned short nd = 0;
	       register size_t nobs = 0;

	       const unsigned short ipx = ABS_PIXELID(np, state->Clcon[ncl]);

	       do {
		    register size_t nb = np;

		    if ( mds_1b[nd].clus[ncl].n_sig > 0 ) {
			 for ( nr = 0; nr < n_read; nr++ ) {
			      register float corr =
				   mds_1b[nd].clus[ncl].sig[nb].corr;

			      scia_cal->spectra[ipx][nobs++] -=
				   MemNlin2Flt( nchan, corr );
			      nb += length;
			 }
		    }
		    if ( mds_1b[nd].clus[ncl].n_sigc > 0 ) {
			 for ( nr = 0; nr < n_read; nr++ ) {
			      register unsigned short nc = 0;
			      register float corr =
				   mds_1b[nd].clus[ncl].sigc[nb].det.field.corr;
			      do {
				   scia_cal->spectra[ipx][nobs++] -= 
					MemNlin2Flt( nchan, corr );
			      } while ( ++nc < coaddf );
			      nb += length;
			 }
		    }
	       } while ( ++nd < state->num_dsr );
	  } while ( ++np < length );
     } while ( ++ncl < state->num_clus );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_CALC_STRAY_CORR
.PURPOSE     calculate straylight correction
.INPUT/OUTPUT
  call as    SCIA_CALC_STRAY_CORR( scia_cal );
 in/output:
	    struct scia_cal_rec *scia_cal :  record with SCIA data of one state

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SCIA_CALC_STRAY_CORR( struct scia_cal_rec *scia_cal )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, scia_cal->correction@*/
{
     const char prognm[] = "SCIA_CALC_STRAY_CORR";

     register unsigned short nch, ng, nr, np;
     register size_t ni;

     register size_t nobs;

     float grid_f[SCIENCE_PIXELS], spec_f[SCIENCE_PIXELS], 
	  stray_f[SCIENCE_PIXELS];

     unsigned short *grid_in_ll = NULL;
     unsigned short *grid_in_ul = NULL;

     float *deriv_out = NULL;
     float *spec_r    = NULL;
     float *stray_r   = NULL;
#ifdef DEBUG
     FILE *fp_full, *fp_grid;
     FILE *fp_corr_full, *fp_corr_grid;
#endif
     struct scia_straycorr stray = {{0,0}, NULL, NULL, NULL};

     /* reset correction values */
     (void) memset( scia_cal->correction[0], 0,
		    sizeof(float) * scia_cal->num_obs * scia_cal->num_pixels );

     /* read straylight correction matrix */
     SCIA_RD_H5_STRAY( &stray );
     if ( IS_ERR_STAT_FATAL )
     	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, "SCIA_RD_H5_STRAY" );

     /* set each element of the array equal to its subscript */
     for ( nr = 0; nr < SCIENCE_PIXELS; nr++ ) grid_f[nr] = (float) nr;

     /* calculate derivative of stray.grid_out */
     deriv_out = (float *) malloc( stray.dims[0] * sizeof(float) );
     if ( deriv_out == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "deriv_out" );
     for ( nr = 1; nr < (stray.dims[0]-1); nr++ )
	  deriv_out[nr] = (stray.grid_out[nr+1] - stray.grid_out[nr-1]) / 2.f;
     deriv_out[0] = (4 * stray.grid_out[1] 
		     - 3 * stray.grid_out[0] 
		     - stray.grid_out[2]) / 2.f;
     deriv_out[stray.dims[0]-1] = (3 * stray.grid_out[stray.dims[0]-1] 
			       - 4 * stray.grid_out[stray.dims[0]-2] 
			       + stray.grid_out[stray.dims[0]-3]) / 2.f;

     /* obtain lower and upper indices for regridding, per channel */
     grid_in_ll = (unsigned short *) malloc( stray.dims[1] * sizeof(short) );
     if ( grid_in_ll == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "grid_in_ll" );
     grid_in_ul = (unsigned short *) malloc( stray.dims[1] * sizeof(short) );
     if ( grid_in_ul == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "grid_in_ul" );

     for ( nch = 0; nch < SCIENCE_CHANNELS; nch++ ) {
	  unsigned short ipix_ch_mn = nch * CHANNEL_SIZE;
	  unsigned short ipix_ch_mx = (nch+1) * CHANNEL_SIZE - 1;

	  bool found = FALSE;

	  for ( nr = 0; nr < stray.dims[1]; nr++ ) {
	       if ( stray.grid_in[nr] < ipix_ch_mn 
		    || stray.grid_in[nr] > ipix_ch_mx ) 
		    continue;

	       if ( ! found ) {
		    grid_in_ll[nr] = ipix_ch_mn;
		    if ( (nr+1u) < stray.dims[1] ) {
			 grid_in_ul[nr] = 
			      __ROUNDf_us( (stray.grid_in[nr] 
					    + stray.grid_in[nr+1]) / 2 ) - 1;
		    } else {
			 grid_in_ul[nr] = ipix_ch_mx;
		    }
		    found = TRUE;
	       } else {
		    grid_in_ll[nr] = 
			 __ROUNDf_us( (stray.grid_in[nr-1] 
				       + stray.grid_in[nr]) / 2 );
		    if ( (nr+1u) < stray.dims[1]
			 && stray.grid_in[nr+1] <= ipix_ch_mx ) {
			 grid_in_ul[nr] = 
			      __ROUNDf_us( (stray.grid_in[nr] 
					    + stray.grid_in[nr+1]) / 2 ) - 1;
		    } else {
			 grid_in_ul[nr] = ipix_ch_mx;
		    }
	       }
	  }
     }
#ifdef DEBUG
     fp_full = fopen( "tmp_spectrum_full.dat", "w" );
     fp_grid = fopen( "tmp_spectrum_grid.dat", "w" );
     fp_corr_full = fopen( "tmp_correction_full.dat", "w" );
     fp_corr_grid = fopen( "tmp_correction_grid.dat", "w" );
#endif
     spec_r = (float *) malloc( stray.dims[1] * sizeof(float) );
     if ( spec_r == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "spec_r" );
     stray_r = (float *) malloc( stray.dims[0] * sizeof(float) );
     if ( stray_r == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "stray_r");

     /* loop over spectra*/
     nobs = 0;
     do {
	  register unsigned short np = 0;

	  /* recontruct full spectra (8192 pixels) */
	  do {
	       unsigned short vchan = VIRTUAL_CHANNEL( scia_cal->chan_id[np],
						       scia_cal->clus_id[np] );

	       unsigned short fillings = (vchan == USHRT_MAX) ? 0 :
		    (scia_cal->num_obs / scia_cal->chan_obs[vchan]);

	       if ( fillings > 1 )
		    spec_f[np] = (float) 
			 (scia_cal->spectra[np][nobs/fillings] / fillings);
	       else if ( fillings == 1 )
		    spec_f[np] = (float) scia_cal->spectra[np][nobs];
	       else
		    spec_f[np] = 0.f;
	  } while ( ++np < SCIENCE_PIXELS );
#ifdef DEBUG
	  (void) fwrite( spec_f, sizeof(float), SCIENCE_PIXELS, fp_full );
#endif
	  /* reduce dimension of spectrum to stray.grid_in */
	  for ( nr = 0; nr < stray.dims[1]; nr++ ) {
	       unsigned short numval = 0;
	       unsigned short allval = grid_in_ul[nr] - grid_in_ll[nr] + 1;

	       spec_r[nr] = 0;
	       for ( ng = grid_in_ll[nr]; ng <= grid_in_ul[nr]; ng++ ) {
		    if ( scia_cal->quality_flag[ng] == FLAG_VALID
			 && isnormal(spec_f[ng]) ) {
			 numval++;
			 spec_r[nr] += spec_f[ng];
		    }
	       }
	       if ( numval > 0 && numval < allval )
		    spec_r[nr] *= ((float) allval / numval);
	  }
#ifdef DEBUG
	  (void) fwrite( spec_r, sizeof(float), stray.dims[1], fp_grid );
#endif
	  /* multiply straylight matrix with spectrum */
          /* and divide by output sampling distance */
	  for ( ni = 0; ni < stray.dims[0]; ni++ ) {
	       stray_r[ni] = 0.;
	       for ( nr = 0; nr < stray.dims[1]; nr++ ) {
		    stray_r[ni] += stray.matrix[ni][nr] * spec_r[nr];
	       }
	       stray_r[ni] /= deriv_out[ni];
	  }
#ifdef DEBUG
	  (void) fwrite( stray_r, sizeof(float), stray.dims[0], fp_corr_grid );
#endif
	  /* resample straylight spectrum to original input grid */
	  FIT_GRID_AKIMA( FLT32_T, FLT32_T, stray.dims[0], stray.grid_out, 
			  stray_r, 
			  FLT32_T, FLT32_T, SCIENCE_PIXELS, grid_f, 
			  stray_f );
#ifdef DEBUG
	  (void) fwrite( stray_f, sizeof(float), SCIENCE_PIXELS, fp_corr_full );
#endif
          /* blank out blinded pixels, use quality_flag */
          np = 0;
          do { 
               if ( scia_cal->quality_flag[np] == FLAG_BLINDED
		    || scia_cal->quality_flag[np] == FLAG_UNUSED )
                    scia_cal->correction[np][nobs] = 0.f;
               else
                    scia_cal->correction[np][nobs] = stray_f[np];
          } while ( ++np < SCIENCE_PIXELS );
     } while ( ++nobs < scia_cal->num_obs );
#ifdef DEBUG
     (void) fclose( fp_full );
     (void) fclose( fp_grid );
     (void) fclose( fp_corr_full );
     (void) fclose( fp_corr_grid );
#endif
     /* reduce correction to pixel exposure time */
     np = 0;
     do {
	  register unsigned short no = 0;
	  register size_t nobs = 0;

	  unsigned short vchan = VIRTUAL_CHANNEL( scia_cal->chan_id[np],
						  scia_cal->clus_id[np] );

	  unsigned short fillings = (vchan == USHRT_MAX) ? 0 :
		    (scia_cal->num_obs / scia_cal->chan_obs[vchan]);

	  if ( fillings <= 1 ) continue;

	  do {
	       register unsigned char nf = 0;

	       register float corrval = 0.;

	       do {
		    corrval += scia_cal->correction[np][nobs++];
	       } while ( ++nf < fillings );
	       scia_cal->correction[np][no] = corrval;
	  } while ( ++no < scia_cal->chan_obs[vchan] );
     } while ( ++np < SCIENCE_PIXELS );
done:
     if ( grid_in_ll != NULL ) free( grid_in_ll );
     if ( grid_in_ul != NULL ) free( grid_in_ul );
     if ( deriv_out != NULL ) free( deriv_out );
     if ( spec_r  != NULL ) free( spec_r );
     if ( stray_r != NULL ) free( stray_r );
     SCIA_FREE_H5_STRAY( &stray );
}

static
void SCIA_CALC_STRAY_SCALE( const struct scia_cal_rec *scia_cal,
			    const struct state1_scia *state, 
			    struct mds1_scia *mds_1b )
       /*@modifies mds_1b->scale_factor@*/
{
     /*  const char prognm[] = "SCIA_CALC_STRAY_SCALE"; */

     register unsigned short ncl = 0;
     register unsigned short nd, nr;

     float **corrMax = ALLOC_R2D( (size_t) state->num_dsr, SCIENCE_CHANNELS );

     (void) memset( corrMax[0], 0,
		    sizeof(float) * state->num_dsr * SCIENCE_CHANNELS );

     do {
	  register unsigned short np = 0;

	  const short ichan = state->Clcon[ncl].channel - 1;
	  const unsigned short coaddf = state->Clcon[ncl].coaddf;
	  const unsigned short length = state->Clcon[ncl].length;
	  const unsigned short n_read = state->Clcon[ncl].n_read;

	  /* do not patch straylight of channel 1 */
	  if ( ichan < 1 ) continue;

	  do {
	       register size_t nobs = 0;

	       unsigned short ipx = ABS_PIXELID(np, state->Clcon[ncl]);

	       if ( scia_cal->quality_flag[ipx] != FLAG_VALID ) continue;

	       nd = 0;
	       do {
		    for ( nr = 0; nr < n_read; nr++ ) {
			 register unsigned short nc = 0;
			 register float corrval = 0.;

			 do {
			      corrval += scia_cal->correction[ipx][nobs++];
			 } while ( ++nc < coaddf );
			 if ( corrval > corrMax[nd][ichan] ) 
			      corrMax[nd][ichan] = corrval;
		    }
	       } while ( ++nd < state->num_dsr );
	  } while ( ++np < length );
      } while ( ++ncl < state->num_clus );

     nd = 0;
     do {
	  mds_1b[nd].scale_factor[1] = (corrMax[nd][1] < 25.5f) ? 1 : 
	       (unsigned char) ceilf(10 * corrMax[nd][1] / UCHAR_MAX );
	  mds_1b[nd].scale_factor[2] = (corrMax[nd][2] < 25.5f) ? 1 :
	       (unsigned char) ceilf(10 * corrMax[nd][2] / UCHAR_MAX );
	  mds_1b[nd].scale_factor[3] = (corrMax[nd][3] < 25.5f) ? 1 :
	       (unsigned char) ceilf(10 * corrMax[nd][3] / UCHAR_MAX );
	  mds_1b[nd].scale_factor[4] = (corrMax[nd][4] < 25.5f) ? 1 :
	       (unsigned char) ceilf(10 * corrMax[nd][4] / UCHAR_MAX );
	  mds_1b[nd].scale_factor[5] = (corrMax[nd][5] < 25.5f) ? 1 :
	       (unsigned char) ceilf(10 * corrMax[nd][5] / UCHAR_MAX );
	  mds_1b[nd].scale_factor[6] = (corrMax[nd][6] < 25.5f) ? 1 :
	       (unsigned char) ceilf(10 * corrMax[nd][6] / UCHAR_MAX );
	  mds_1b[nd].scale_factor[7] = (corrMax[nd][7] < 25.5f) ? 1 :
	       (unsigned char) ceilf(10 * corrMax[nd][7] / UCHAR_MAX );
     } while ( ++nd < state->num_dsr );

     FREE_2D((void **) corrMax);
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_PATCH_STRAY_CORR
.PURPOSE     patch straylight correction values in SCIA L1b MDS
.INPUT/OUTPUT
  call as    SCIA_PATCH_STRAY_CORR( struct scia_cal_rec *scia_cal,
			const struct state1_scia *state, 
			struct mds1_scia *mds_1b )
     input:
            struct scia_cal_rec *scia_cal :  record with SCIA data of one state
            struct state1_scia *state     :  structure with State definition
 in/output:
	    struct mds1_scia *mds_1b      :  structure holding level 1b MDS

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SCIA_PATCH_STRAY_CORR( const struct scia_cal_rec *scia_cal,
			    const struct state1_scia *state, 
			    struct mds1_scia *mds_1b )
       /*@modifies mds_1b@*/
{
     register unsigned short ncl = 0;
     register unsigned short nd, nr;

     register float scaleFactor;

     /*
      * calculate new scaleFactors
      */
     SCIA_CALC_STRAY_SCALE( scia_cal, state, mds_1b );

     /*
      * store new straylight correction factors
      */
     ncl = 0;
     do {
	  register unsigned short np = 0;

	  const short ichan = state->Clcon[ncl].channel - 1;
	  const unsigned short coaddf = state->Clcon[ncl].coaddf;
	  const unsigned short length = state->Clcon[ncl].length;
	  const unsigned short n_read = state->Clcon[ncl].n_read;

	  /* do not patch straylight of channel 1 */
	  if ( ichan < 1 ) continue;

	  do {
	       register size_t nobs = 0;

	       unsigned short ipx = ABS_PIXELID(np, state->Clcon[ncl]);

	       if ( scia_cal->quality_flag[ipx] != FLAG_VALID ) continue;

	       nd = 0;
	       do {
		    register size_t nb = np;

		    scaleFactor = mds_1b[nd].scale_factor[ichan] / 10.f;

		    if ( mds_1b[nd].clus[ncl].n_sig > 0 ) {
			 for ( nr = 0; nr < n_read; nr++ ) {
			      mds_1b[nd].clus[ncl].sig[nb].stray =
				   __ROUNDf_uc( scia_cal->correction[ipx][nobs]
						/ scaleFactor );
			      nobs++;
			      nb += length;
			 }
		    }
		    if ( mds_1b[nd].clus[ncl].n_sigc > 0 ) {
			 for ( nr = 0; nr < n_read; nr++ ) {
			      register unsigned short nc = 0;
			      register float corrval = 0.;

			      do {
				   corrval += scia_cal->correction[ipx][nobs++];
			      } while ( ++nc < coaddf );
			      mds_1b[nd].clus[ncl].sigc[nb].stray =
				  __ROUNDf_uc( corrval / scaleFactor );

			      nb += length;
			 }
		    }
	       } while ( ++nd < state->num_dsr );
	  } while ( ++np < length );
      } while ( ++ncl < state->num_clus );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_WR_CAL_REC
.PURPOSE     write intermediate results to HDF5 file
.INPUT/OUTPUT
  call as    SCIA_WR_CAL_REC( patch_flag, scia_cal );
     input:
            unsigned int patch_flag  :  bit-flag which defines what to patch
	    struct scia_cal_rec *scia_cal :  record with SCIA data of one state

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void SCIA_WR_CAL_REC( unsigned short patch_flag,
		      const struct scia_cal_rec *scia_cal )
{
     const char prognm[] = "SCIA_WR_CAL_REC";

     hid_t   fileID = 0;
     hsize_t dims[2];

     char flname[29];
     unsigned int ubuff;

     const unsigned char  ucbuff = 0;
     const float          rbuff = 0.f;
     const double         dbuff = 0.;
     const double         nanbuff = NAN;

     (void) snprintf( flname, 29, "scia_cal_rec_%05hu_%02hhu_%03hhu.h5",
		      scia_cal->abs_orbit,  scia_cal->state_id,  
		      scia_cal->state_index );
     fileID = H5Fcreate( flname, H5F_ACC_TRUNC, 
			 H5P_DEFAULT, H5P_DEFAULT );
     if ( fileID < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_CRE, flname );

     (void) H5LTset_attribute_ushort( fileID, "/", "PatchFlag", 
				      &patch_flag, 1 );
     (void) H5LTset_attribute_uchar( fileID, "/", "stateID", 
				      &scia_cal->state_id, 1 );
     (void) H5LTset_attribute_uchar( fileID, "/", "stateIndex", 
				      &scia_cal->state_index, 1 );
     ubuff = (unsigned int) scia_cal->num_obs;
     (void) H5LTset_attribute_uint( fileID, "/", "numObs", &ubuff, 1 );
     ubuff = (unsigned int) VIRTUAL_CHANNELS;
     (void) H5LTset_attribute_uint( fileID, "/", "numChannels", &ubuff, 1 );
     ubuff = (unsigned int) SCIENCE_PIXELS;
     (void) H5LTset_attribute_uint( fileID, "/", "numPixels", &ubuff, 1 );
     (void) H5LTset_attribute_ushort( fileID, "/", "PET_min", 
				      &scia_cal->state_pet_min, 1 );
     (void) H5LTset_attribute_float( fileID, "/", "orbitPhase;", 
				      &scia_cal->orbit_phase, 1 );

     dims[0] = (hsize_t) SCIENCE_PIXELS;
     (void) H5LTmake_dataset( fileID, "channelID", 1, dims, 
			      H5T_NATIVE_UCHAR, scia_cal->chan_id );
     (void) H5LTmake_dataset( fileID, "clusterID", 1, dims, 
			      H5T_NATIVE_UCHAR, scia_cal->clus_id );
     (void) H5LTmake_dataset( fileID, "coaddFactor", 1, dims, 
			      H5T_NATIVE_UCHAR, scia_cal->coaddf );
     (void) H5LTset_attribute_uchar( fileID, "coaddFactor", "_FillValue", 
				     &ucbuff, 1 );
     (void) H5LTmake_dataset( fileID, "qualityFlags", 1, dims, 
			      H5T_NATIVE_UCHAR, scia_cal->quality_flag );
     (void) H5LTset_attribute_uchar( fileID, "qualityFlags", "_FillValue", 
				     &ucbuff, 1 );
     (void) H5LTmake_dataset_float( fileID, "PET", 1, dims, scia_cal->pet );
     (void) H5LTset_attribute_float( fileID, "PET", "_FillValue", &rbuff, 1 );
     (void) H5LTmake_dataset_float( fileID, "darkSignal", 1, dims, 
				    scia_cal->dark_signal );
     (void) H5LTset_attribute_float( fileID, "darkSignal", "_FillValue", 
				     &rbuff, 1 );
     dims[0] = (hsize_t) VIRTUAL_CHANNELS;
     dims[1] = (hsize_t) scia_cal->num_obs;
     (void) H5LTmake_dataset_double( fileID, "channelMean", 2, dims, 
				     scia_cal->chan_mean[0] );
     (void) H5LTset_attribute_double( fileID, "channelMean", "_FillValue", 
				      &dbuff, 1 );
     (void) H5LTset_attribute_ushort( fileID, "channelMean", "channelPET", 
				      scia_cal->chan_pet, VIRTUAL_CHANNELS );
     (void) H5LTset_attribute_ushort( fileID, "channelMean", "channelObs", 
				      scia_cal->chan_obs, VIRTUAL_CHANNELS );
     dims[0] = (hsize_t) SCIENCE_PIXELS;
     dims[1] = (hsize_t) scia_cal->num_obs;
     (void) H5LTmake_dataset_double( fileID, "spectra", 2, dims, 
				     scia_cal->spectra[0] );
     (void) H5LTset_attribute_double( fileID, "spectra", "_FillValue", 
				      &nanbuff, 1 );
     (void) H5LTmake_dataset_float( fileID, "correction", 2, dims, 
				    scia_cal->correction[0] );
done:
     if ( fileID > 0 ) (void) H5Fclose( fileID );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_PATCH_MDS( FILE *fp, unsigned short patch_flag,
			 const struct state1_scia *state, 
			 struct mds1_scia *mds_1b )
{
     const char prognm[] = "SCIA_LV1_PATCH_MDS";

     register unsigned short nch, ncl, np;

     char *write_h5_scia_cal = getenv( "SCIA_WR_PATCH_INTERIM" );

     unsigned int calib_flag;

     struct mph_envi   mph;
     struct scia_cal_rec scia_cal = { 0, 0, 0, 0, 0, 0, 0, 0, 0.f, 0.f, 
				      NULL, NULL, NULL, NULL, NULL, NULL,  
				      NULL, NULL, NULL, NULL, NULL
     };
     const bool verbose = FALSE;
/*
 * return when nothing has to be done
 */
     if ( patch_flag == SCIA_PATCH_NONE ) return;
/*
 * always read MPH
 */
     if ( verbose )
	  (void) fprintf( stderr, "\n Start of module: %s\n", prognm );
     ENVI_RD_MPH( fp, &mph );
/*
 * initialize structure with SCIA calibration parameters and data
 */
     scia_cal.state_id     = state->state_id;
     scia_cal.state_index  = state->indx;
     scia_cal.abs_orbit    = (unsigned short) mph.abs_orbit;
     scia_cal.type_mds     = (int) state->type_mds;
     scia_cal.limb_scans   = (mds_1b->geoL == NULL) ? 0 : state->num_dsr;
     scia_cal.num_channels = VIRTUAL_CHANNELS;
     scia_cal.num_pixels   = SCIENCE_PIXELS;
     scia_cal.orbit_phase  = state->orbit_phase;
/*
 * obtain maximum number of read-outs
 */
     scia_cal.num_obs = 0;
     for ( ncl = 0; ncl < state->num_clus; ncl++ ) {
	  size_t nobs = state->Clcon[ncl].n_read * state->Clcon[ncl].coaddf;

	  if ( nobs > scia_cal.num_obs ) scia_cal.num_obs = nobs;
     }
     scia_cal.num_obs *= state->num_dsr;
/*
 * allocate space for the SCIA spectra
 */
     scia_cal.chan_pet = (unsigned short *)
	  calloc( sizeof( short ), scia_cal.num_channels );
     if ( scia_cal.chan_pet == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.chan_pet" );
     scia_cal.chan_obs = (unsigned short *)
	  calloc( sizeof( short ), scia_cal.num_channels );
     if ( scia_cal.chan_obs == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.chan_obs" );
     scia_cal.coaddf = (unsigned char *) 
	  calloc( sizeof( char ), scia_cal.num_pixels );
     if ( scia_cal.coaddf == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.coaddf" );
     scia_cal.chan_id = (unsigned char *) 
	  calloc( sizeof( char ), scia_cal.num_pixels );
     if ( scia_cal.chan_id == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.chan_id" );
     scia_cal.clus_id = (unsigned char *) 
	  calloc( sizeof( char ), scia_cal.num_pixels );
     if ( scia_cal.clus_id == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.clus_id" );
     scia_cal.quality_flag = (unsigned char *) 
	  calloc( sizeof( char ), scia_cal.num_pixels );
     if ( scia_cal.quality_flag == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.quality_flag" );
     scia_cal.dark_signal = (float *) 
	  calloc( sizeof( float ), scia_cal.num_pixels );
     if ( scia_cal.dark_signal == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.dark_signal" );
     scia_cal.pet = (float *) calloc( sizeof( float ), scia_cal.num_pixels );
     if ( scia_cal.pet == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.pet" );
     scia_cal.chan_mean = 
	  ALLOC_D2D( scia_cal.num_channels, scia_cal.num_obs );
     if ( scia_cal.chan_mean == NULL ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.chan_mean" );
     scia_cal.spectra = ALLOC_D2D( scia_cal.num_pixels, scia_cal.num_obs );
     if ( scia_cal.spectra == NULL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.spectra" );
     scia_cal.correction = ALLOC_R2D( scia_cal.num_pixels, scia_cal.num_obs );
     if ( scia_cal.correction == NULL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.correction" );
     if ( verbose )
	  (void) fprintf(stderr,"Allocated memory for calibration structure\n");
     (void) memset( scia_cal.correction[0], 0,
		    sizeof(float) * scia_cal.num_obs * scia_cal.num_pixels );

     /* write for each pixel: channel ID, coadding-factor, PET */
     for ( ncl = 0; ncl < state->num_clus; ncl++ ) {
	  unsigned short pet = __ROUNDf_us( 16 * state->Clcon[ncl].pet );

	  for ( np = 0; np < state->Clcon[ncl].length; np++ ) {
	       unsigned short ipx = ABS_PIXELID(np, state->Clcon[ncl]);
	  
	       scia_cal.coaddf[ipx] = state->Clcon[ncl].coaddf;
	       scia_cal.chan_id[ipx] = state->Clcon[ncl].channel;
	       scia_cal.clus_id[ipx] = state->Clcon[ncl].id;
	       scia_cal.pet[ipx] = state->Clcon[ncl].pet;
	  }
	  switch ( (int) state->Clcon[ncl].channel ) {
	  case 1:
	       if ( state->Clcon[ncl].id < 4 )
		    scia_cal.chan_pet[0] = pet;
	       else
		    scia_cal.chan_pet[1] = pet;
	       break;
	  case 2:
	       if ( state->Clcon[ncl].id < 10 )
		    scia_cal.chan_pet[3] = pet;
	       else
		    scia_cal.chan_pet[2] = pet;
	       break;
	  case 3:
	       scia_cal.chan_pet[4] = pet;
	       break;
	  case 4:
	       scia_cal.chan_pet[5] = pet;
	       break;
	  case 5:
	       scia_cal.chan_pet[6] = pet;
	       break;
	  case 6:
	       scia_cal.chan_pet[7] = pet;
	       break;
	  case 7:
	       scia_cal.chan_pet[8] = pet;
	       break;
	  case 8:
	       scia_cal.chan_pet[9] = pet;
	       break;
	  }
     }
     /* obtain per virtual-channel: minimum coadding-factor and PET */
     scia_cal.state_pet_min = USHRT_MAX;
     for ( nch = 0; nch < VIRTUAL_CHANNELS; nch++ ) {
	  if ( scia_cal.chan_pet[nch] < scia_cal.state_pet_min )
	       scia_cal.state_pet_min = scia_cal.chan_pet[nch];
     }

     for ( nch = 0; nch < VIRTUAL_CHANNELS; nch++ ) {
	  scia_cal.chan_obs[nch] = (scia_cal.num_obs * scia_cal.state_pet_min)
	       / scia_cal.chan_pet[nch];
     }
     /*
      * fill spectra array...
      */
     SCIA_FILL_SPECTRA( state, mds_1b, &scia_cal );
     if ( verbose )
	  (void) fprintf( stderr, "Wrote data to calibration structure\n" );
     /*
      * obtain quality flags
      */
     SCIA_SET_FLAG_QUALITY( &scia_cal );
     if ( verbose )
	  (void) fprintf( stderr, "Performed quality flagging\n" );
     /* 
      * apply darkcurrent correction
      */
     calib_flag = (DO_CORR_AO|DO_CORR_DARK);
     SCIA_APPLY_DARK( fp, calib_flag, &scia_cal );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "SCIA_CAL_GET_DARK" );
     if ( verbose )
	  (void) fprintf( stderr, "Read Dark current parameters\n" );
     /*
      * compute estimates of channel averages at PET_min 
      * based on PMD read-outs
      */
     SCIA_CALC_CHAN_MEAN( state, mds_1b, &scia_cal );
     if ( verbose )
	  (void) fprintf( stderr, "Calculated channel averages\n" );
     /*
      * de-coadd values using channel averages
      */
     SCIA_CAL_DE_COADD( &scia_cal );
     if ( verbose )
	  (void) fprintf( stderr, "Performed de-coadd of spectral data\n" );
     /*
      * apply Memory correction
      */
     if ( (patch_flag & SCIA_PATCH_MEM) != USHRT_ZERO ) {
	  SCIA_CALC_MEM_CORR( &scia_cal );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "MEM" );
	  SCIA_PATCH_MEM_CORR( &scia_cal, state, mds_1b );
	  if ( verbose )
	       (void) fprintf( stderr, "Performed memory correction\n" );
     }
     /*
      * apply non-Linearity correction
      */
     if ( (patch_flag & SCIA_PATCH_NLIN) != USHRT_ZERO ) {
	  SCIA_CALC_NLIN_CORR( &scia_cal );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "NLIN" );
	  SCIA_PATCH_NL_CORR( &scia_cal, state, mds_1b );
	  if ( verbose )
	       (void) fprintf( stderr, "Performed non-linearity correction\n" );
     }
     /*
      * Stray-light correction
      */
     if ( (patch_flag & SCIA_PATCH_STRAY) != USHRT_ZERO ) {
	  SCIA_APPLY_MemNlin( state, mds_1b, &scia_cal );
	  SCIA_CALC_STRAY_CORR( &scia_cal );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "STRAY" );
	  SCIA_PATCH_STRAY_CORR( &scia_cal, state, mds_1b );
	  if ( verbose )
	       (void) fprintf( stderr, "Performed Straylight correction\n" );
     }
     /*
      * Write calibration record to HDF5 file
      */
     if ( write_h5_scia_cal != NULL && *write_h5_scia_cal != '0' ) {
	  SCIA_WR_CAL_REC( patch_flag, &scia_cal );
	  if ( verbose )
	       (void) fprintf( stderr, "Wrote data for HDF5 file\n" );
     }
/*
 * release allocated memory
 */
done:
     if ( scia_cal.coaddf != NULL ) free( scia_cal.coaddf );
     if ( scia_cal.chan_id != NULL ) free( scia_cal.chan_id );
     if ( scia_cal.clus_id != NULL ) free( scia_cal.clus_id );
     if ( scia_cal.quality_flag != NULL ) free( scia_cal.quality_flag );
     if ( scia_cal.chan_pet != NULL ) free( scia_cal.chan_pet );
     if ( scia_cal.chan_obs != NULL ) free( scia_cal.chan_obs );
     if ( scia_cal.dark_signal != NULL ) free( scia_cal.dark_signal );
     if ( scia_cal.pet != NULL ) free( scia_cal.pet );
     if ( scia_cal.chan_mean != NULL ) FREE_2D( (void **) scia_cal.chan_mean );
     if ( scia_cal.spectra != NULL ) FREE_2D( (void **) scia_cal.spectra );
     if ( scia_cal.correction != NULL ) FREE_2D((void **) scia_cal.correction);
     if ( verbose )
	  (void) fprintf( stderr, "Finished module: %s\n", prognm );
}
