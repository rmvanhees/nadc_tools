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
.COMMENTS    None
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

#define FLAG_VALID     ((unsigned char) 0x0U)
#define FLAG_BLINDED   ((unsigned char) 0x1U)
#define FLAG_UNUSED    ((unsigned char) 0x2U)
#define FLAG_DEAD      ((unsigned char) 0x4U)
#define FLAG_SATURATE  ((unsigned char) 0x8U)

/*+++++ Macros +++++*/
#define absPixelID(Clcon,ipx) ((unsigned short)				\
			       ((Clcon.pixel_nr + ipx +                 \
				 CHANNEL_SIZE * ((int) Clcon.channel - 1))))

/*+++++ Static Variables +++++*/
 /* science channel 2 PMD mapping: 1-A, 2-A, 3-B, 4-C, 5-D, 6-E, 7-F, 8-F */
static const unsigned short Chan2PmdIndx[SCIENCE_CHANNELS] = { 
     0, 0, 1, 2, 3, 4, 5, 5 
};

struct scia_cal_rec {
     unsigned char    state_id;
     unsigned char    state_index;
     unsigned short   abs_orbit;
     unsigned short   limb_scans;       /* zero for non-Limb states */
     int              type_mds;
     size_t           num_obs;          /* observation at pet_min */
     size_t           num_channels;     /* SCIENCE_CHANNELS */
     size_t           num_xpixels;      /* SCIENCE_PIXELS */
     size_t           num_ypixels;      /* zero */
     float            pet_min;
     float            orbit_phase;
     unsigned char    *coaddf;          /* [num_xpixels] co-adding factor */
     unsigned char    *chan_id;         /* [num_xpixels] channel ID [1..8] */
     unsigned char    *coadd_chan_min;  /* [num_channels] */
     unsigned char    *quality_flag;    /* [num_xpixels] zero is ok */
     float            *dark_signal;     /* [num_xpixels] dark at PET[chan] */
     float            *pet;             /* [num_xpixels] 6-8 not corrected */
     float            *pet_chan_min;    /* [num_channels] */
     double           **chan_mean;      /* [num_channels][num_obs] */
     double           **spectra;        /* [num_xpixels][num_obs] */
     float            **correction;     /* [num_xpixels][num_obs] */
};

/*+++++ Global Variables +++++*/

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
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
     ubuff = (unsigned int) scia_cal->num_channels;
     (void) H5LTset_attribute_uint( fileID, "/", "numChannels", &ubuff, 1 );
     ubuff = (unsigned int) scia_cal->num_xpixels;
     (void) H5LTset_attribute_uint( fileID, "/", "numPixelsX", &ubuff, 1 );
     ubuff = (unsigned int) scia_cal->num_ypixels;
     (void) H5LTset_attribute_uint( fileID, "/", "numPixelsY", &ubuff, 1 );
     (void) H5LTset_attribute_float( fileID, "/", "PET_min", 
				      &scia_cal->pet_min, 1 );
     (void) H5LTset_attribute_float( fileID, "/", "orbitPhase;", 
				      &scia_cal->orbit_phase, 1 );

     dims[0] = (hsize_t) scia_cal->num_xpixels;
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
     dims[0] = (hsize_t) scia_cal->num_channels;
     dims[1] = (hsize_t) scia_cal->num_obs;
     (void) H5LTmake_dataset_double( fileID, "channelMean", 2, dims, 
				     scia_cal->chan_mean[0] );
     (void) H5LTset_attribute_double( fileID, "channelMean", "_FillValue", 
				      &dbuff, 1 );
     (void) H5LTset_attribute_uchar( fileID, "channelMean", "coaddf_min", 
				     scia_cal->coadd_chan_min, 
				     SCIENCE_CHANNELS );
     (void) H5LTset_attribute_float( fileID, "channelMean", "pet_min", 
				     scia_cal->pet_chan_min, 
				     SCIENCE_CHANNELS );
     dims[0] = (hsize_t) scia_cal->num_xpixels;
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

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_CAL_GET_DARK
.PURPOSE     obtain dark correction values 
.INPUT/OUTPUT
  call as    SCIA_CAL_GET_DARK( fp, calib_flag, scia_cal );
     input:
            FILE *fp                      : file pointer to SCIA L1b product
	    unsigned int calib_flag       : calibration flags (bitfield)
 in/output:
	    struct scia_cal_rec *scia_cal :  record with SCIA data of one state

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void SCIA_CAL_GET_DARK( FILE *fp, unsigned int calib_flag, 
			struct scia_cal_rec *scia_cal )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, 
	 scia_cal->dark_signal@*/
{
     const char prognm[] = "SCIA_CAL_GET_DARK";

     register size_t np;

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

     for ( np = 0; np < scia_cal->num_xpixels; np++ ) {
	  float pet = scia_cal->pet[np];

	  if ( pet < FLT_EPSILON ) continue;
	  if ( np >= (VIS_CHANNELS * CHANNEL_SIZE) 
	       && pet >= (PET_noHotMode - FLT_EPSILON) ) pet -= PET_Offset;
	       
	  scia_cal->dark_signal[np] = analogOffs[np] + pet * darkCurrent[np];
     }
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
     register size_t np, no;

     /* initialize spectra with NaN values */
     for ( np = 0; np < scia_cal->num_xpixels; np++ ) {
	  for ( no = 0; no < scia_cal->num_obs; no++ )
	       scia_cal->spectra[np][no] = NAN;
     }

     do {
	  const size_t fillings = 
		NINT(state->Clcon[ncl].pet / scia_cal->pet_min);

	  np = 0;
	  do {
	       register unsigned short nr;
	       register unsigned short nd = 0;
	       register size_t nobs = fillings - 1;

	       const unsigned short ipx = absPixelID(state->Clcon[ncl],np);

	       const double dark = 
		    state->Clcon[ncl].coaddf * scia_cal->dark_signal[ipx];
	  
	       do {
		    if ( mds_1b[nd].clus[ncl].n_sig > 0 ) {
			 register size_t nb = np;

			 for ( nr = 0; nr < state->Clcon[ncl].n_read; nr++ ) {
			      scia_cal->spectra[ipx][nobs] =
				   mds_1b[nd].clus[ncl].sig[nb].sign - dark;
			      nb += state->Clcon[ncl].length;
			      nobs += fillings;
			 }
		    }
		    if ( mds_1b[nd].clus[ncl].n_sigc > 0 ) {
			 register size_t nb = np;

			 for ( nr = 0; nr < state->Clcon[ncl].n_read; nr++ ) {
			      register unsigned short nc = 0;
			      do {
				   scia_cal->spectra[ipx][nobs] = 
					mds_1b[nd].clus[ncl].sigc[nb].det.field.sign
					- dark;
				   nobs += fillings;
			      } while ( ++nc < state->Clcon[ncl].coaddf );
			      nb += state->Clcon[ncl].length;
			 }
		    }
	       } while ( ++nd < state->num_dsr );
	  } while ( ++np < (size_t) state->Clcon[ncl].length );
      } while ( ++ncl < state->num_clus );
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
     register size_t nch = 0;
     register size_t no;

     size_t NumPmdPet;
     size_t offs;

     if ( scia_cal->type_mds == SCIA_MONITOR ) {
	  do { 
	       no = 0;
	       do {
		    scia_cal->chan_mean[nch][no] = 1.;
	       } while ( ++no < scia_cal->num_obs );
	  } while ( ++nch < scia_cal->num_channels );
	  return;
     }

     /* initialize channel mean to zero */
     (void) memset( scia_cal->chan_mean[0], 0, sizeof(double) 
		    * scia_cal->num_obs * scia_cal->num_channels );
     do {
	  register unsigned short nd  = 0;

	  if ( scia_cal->pet_chan_min[nch] <= FLT_EPSILON ) continue;

          NumPmdPet = NINT( 32. * scia_cal->pet_chan_min[nch] );
	  offs = NINT( scia_cal->pet_chan_min[nch] / scia_cal->pet_min );

	  no = offs-1;
	  do {
	       register unsigned short indx = Chan2PmdIndx[nch];

	       while ( indx < mds_1b[nd].n_pmd ) {
		    register size_t np = 0;
		    register size_t num_valid = 0;

		    do {
			 if ( mds_1b[nd].int_pmd[indx] > FLT_EPSILON ) {
			      num_valid++;
			      scia_cal->chan_mean[nch][no] += 
				   mds_1b[nd].int_pmd[indx];
			 }
			 indx += PMD_NUMBER;
		    } while ( ++np < NumPmdPet );
		    if ( num_valid > 0 ) 
			 scia_cal->chan_mean[nch][no] /= num_valid;
		    no += offs;
	       }
	  } while ( ++nd < state->num_dsr );
     } while ( ++nch < scia_cal->num_channels );
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
     register size_t no, np;

     for ( np = 0; np < scia_cal->num_xpixels; np++ ) {
	  register unsigned short num_valid = 0;
	  register double mean = 0.;

	  if ( (np % CHANNEL_SIZE) < 10 
	       || (np % CHANNEL_SIZE) >= (CHANNEL_SIZE - 10) ) {
	       scia_cal->quality_flag[np] = FLAG_BLINDED;
	       continue;
	  }
	       
	  for ( no = 0; no < scia_cal->num_obs; no++ ) {
	       if ( fpclassify( scia_cal->spectra[np][no] ) != FP_NAN ) {
		    mean += scia_cal->spectra[np][no];
		    num_valid++;
	       }
	  }
	  if ( num_valid == 0 ) {
	       scia_cal->quality_flag[np] = FLAG_UNUSED;
	  } else if ( (mean /= num_valid) > (scia_cal->coaddf[np] * 60000.) ) {
	       scia_cal->quality_flag[np] = FLAG_SATURATE;
	  } else {
	       register double adev = 0.;

	       for ( no = 0; no < scia_cal->num_obs; no++ ) {
		    if ( fpclassify( scia_cal->spectra[np][no] ) != FP_NAN )
			 adev += fabs( scia_cal->spectra[np][no] - mean );
	       }
	       adev /= num_valid;

	       if ( mean <= DBL_EPSILON || adev <= DBL_EPSILON )
		    scia_cal->quality_flag[np] = FLAG_DEAD;
	       else
		    scia_cal->quality_flag[np] = FLAG_VALID;
	  }
     }
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
     register size_t no, np;

     size_t offs;

     for ( np = 0; np < scia_cal->num_xpixels; np++ ) {
	  const short nchan = (short) scia_cal->chan_id[np] - 1;

	  if ( nchan < 0 || scia_cal->pet_chan_min[nchan] <= FLT_EPSILON )
	       continue;

	  offs  = NINT( scia_cal->pet_chan_min[nchan] / scia_cal->pet_min );

	  no = offs-1;
	  do {
	       register unsigned char nc;

	       register size_t noo  = no;
	       register double wght = 0.;

	       for ( nc = 0; nc < scia_cal->coaddf[np]; nc++ ) {
		    wght += scia_cal->chan_mean[nchan][noo];
		    noo += offs;
	       }

	       for ( nc = 0; nc < scia_cal->coaddf[np]; nc++ ) {
		    /* scia_cal->spectra[np][no] *=  */
		    /* 	 (1. / scia_cal->coaddf[np]); */
		    scia_cal->spectra[np][no] *= 
			 (scia_cal->chan_mean[nchan][no] / wght);
		    no += offs;
	       }
	  } while ( no < scia_cal->num_obs );
     }
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

     register size_t         no, np;
     register unsigned short signNorm;
     register double         sign_before, sign_after;

     struct scia_memcorr memcorr = {{0,0}, NULL};

     const size_t num_tan_h = (scia_cal->limb_scans == 0) ? 0 : 
	  scia_cal->num_obs / scia_cal->limb_scans;
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

     for ( np = 0; np < (VIS_CHANNELS * CHANNEL_SIZE); np++ ) {
	  const short nchan = (short) scia_cal->chan_id[np] - 1;
	  const size_t offs = (nchan < 0) ? 0 :
	       NINT( scia_cal->pet[np] / scia_cal->pet_min );

	  /* before reset */
	  const double scale_before = 3 * (0.5 - (np % 1024) / 6138.)
	       / (16 * scia_cal->pet[np]);
	  /* after reset */
	  const double scale_after  = 3 * (0.5 + (np % 1024) / 6138.)
	       / (16 * scia_cal->pet[np]);
	  /* nadir reset */
	  const double scale_reset = (nchan < 0) ? 0. :
	       setup_it[scia_cal->state_id] / (1000. * scia_cal->pet[np]);

	  /* skip un-used pixels */
	  if ( offs == 0 ) continue;

	  /* set index to first readout */
	  no = offs - 1;

	  /* calculate memory correction for the first readout of a state */
	  signNorm = __ROUND_us( scia_cal->dark_signal[np]
				 + scale_reset * scia_cal->spectra[np][no] );
	  scia_cal->correction[np][no] = memcorr.matrix[nchan][signNorm];

	  /* use previous readout to calculate correction next readout */
	  while ( (no += offs) < scia_cal->num_obs ) {
	       if ( scia_cal->limb_scans != 0 
		    && ((no - (offs-1)) % num_tan_h) == 0 ) {
		    sign_before = scale_before * scia_cal->spectra[np][no-offs];
		    sign_after  = scale_after * scia_cal->spectra[np][no];

		    signNorm = __ROUND_us( scia_cal->dark_signal[np]
					   + sign_before + sign_after );
	       } else {
		    signNorm = __ROUND_us( scia_cal->dark_signal[np]
					   + scia_cal->spectra[np][no-offs] );
	       }
	       scia_cal->correction[np][no] = memcorr.matrix[nchan][signNorm];
	  }
     }
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

     register size_t         no, np;
     register unsigned short signNorm;
     register unsigned short curveIndx;

     struct scia_nlincorr nlcorr = {{0,0}, NULL, NULL};

     SCIA_RD_H5_NLIN( NULL, &nlcorr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, "SCIA_RD_H5_NLIN" );

     for ( np = (VIS_CHANNELS * CHANNEL_SIZE); np < SCIENCE_PIXELS; np++ ) {
	  const size_t offs = NINT( scia_cal->pet[np] / scia_cal->pet_min );
	  
	  /* skip un-used pixels */
	  if ( offs == 0 ) continue;

	  /* get index to curve to be used */
	  curveIndx = (unsigned short) nlcorr.curve[np];

	  /* set index to first readout */
	  no = offs-1;
	  do {
	       signNorm = __ROUND_us( scia_cal->spectra[np][no] 
				      + scia_cal->dark_signal[np] );

	       scia_cal->correction[np][no] = 
		    nlcorr.matrix[curveIndx][signNorm];

	       no += offs;
	  } while ( no < scia_cal->num_obs );
     }
done:
     SCIA_FREE_H5_NLIN( &nlcorr );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_PATCH_MEM_CORR
.PURPOSE     convert level 1b readouts to spectral grid
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
	  register size_t np = 0;

	  const size_t fillings = 
		NINT(state->Clcon[ncl].pet / scia_cal->pet_min);

	  do {
	       register unsigned short nr;
	       register unsigned short nd = 0;
	       register size_t nobs = fillings - 1;

	       const unsigned short ipx = absPixelID(state->Clcon[ncl],np);

	       if ( state->Clcon[ncl].channel > VIS_CHANNELS ) continue;
	       do {
		    if ( mds_1b[nd].clus[ncl].n_sig > 0 ) {
			 register size_t nb = np;

			 for ( nr = 0; nr < state->Clcon[ncl].n_read; nr++ ) {
			      mds_1b[nd].clus[ncl].sig[nb].corr =
				   MemNlin2Byte( state->Clcon[ncl].channel, 
						 scia_cal->correction[ipx][nobs] );
			      nb += state->Clcon[ncl].length;
			      nobs += fillings;
			 }
		    }
		    if ( mds_1b[nd].clus[ncl].n_sigc > 0 ) {
			 register size_t nb = np;

			 for ( nr = 0; nr < state->Clcon[ncl].n_read; nr++ ) {
			      register unsigned short nc = 0;
			      register float corrval = 0.f;

			      do {
				   corrval += scia_cal->correction[ipx][nobs];
				   nobs += fillings;
			      } while ( ++nc < state->Clcon[ncl].coaddf );
			      mds_1b[nd].clus[ncl].sigc[nb].det.field.corr =
				   MemNlin2Byte( state->Clcon[ncl].channel, 
						 corrval / state->Clcon[ncl].coaddf );
			      
			      nb += state->Clcon[ncl].length;
			 }
		    }
	       } while ( ++nd < state->num_dsr );
	  } while ( ++np < (size_t) state->Clcon[ncl].length );
      } while ( ++ncl < state->num_clus );
}

static
void SCIA_PATCH_NL_CORR( struct scia_cal_rec *scia_cal,
			const struct state1_scia *state, 
			struct mds1_scia *mds_1b )
       /*@modifies mds_1b@*/
{
     register unsigned short ncl = 0;

     do {
	  register size_t np = 0;

	  const size_t fillings = 
		NINT(state->Clcon[ncl].pet / scia_cal->pet_min);

	  do {
	       register unsigned short nr;
	       register unsigned short nd = 0;
	       register size_t nobs = fillings - 1;

	       const unsigned short ipx = absPixelID(state->Clcon[ncl],np);

	       if ( state->Clcon[ncl].channel <= VIS_CHANNELS ) continue;
	       do {
		    if ( mds_1b[nd].clus[ncl].n_sig > 0 ) {
			 register size_t nb = np;

			 for ( nr = 0; nr < state->Clcon[ncl].n_read; nr++ ) {
			      mds_1b[nd].clus[ncl].sig[nb].corr =
				   MemNlin2Byte( state->Clcon[ncl].channel, 
						 scia_cal->correction[ipx][nobs] );
			      nb += state->Clcon[ncl].length;
			      nobs += fillings;
			 }
		    }
		    if ( mds_1b[nd].clus[ncl].n_sigc > 0 ) {
			 register size_t nb = np;

			 for ( nr = 0; nr < state->Clcon[ncl].n_read; nr++ ) {
			      register unsigned short nc = 0;
			      register float corrval = 0.f;

			      do {
				   corrval += scia_cal->correction[ipx][nobs];
				   nobs += fillings;
			      } while ( ++nc < state->Clcon[ncl].coaddf );
			      mds_1b[nd].clus[ncl].sigc[nb].det.field.corr =
				   MemNlin2Byte( state->Clcon[ncl].channel,
						 corrval / state->Clcon[ncl].coaddf );
			      
			      nb += state->Clcon[ncl].length;
			 }
		    }
	       } while ( ++nd < state->num_dsr );
	  } while ( ++np < (size_t) state->Clcon[ncl].length );
      } while ( ++ncl < state->num_clus );
}

static
void SCIA_APPLY_CORR( struct scia_cal_rec *scia_cal )
{
     register size_t np = 0;

     do {
	  register size_t no = 0;

	  if ( scia_cal->quality_flag[np] != 0 ) continue;
	  do {
	       if ( fpclassify( scia_cal->spectra[np][no] ) != FP_NAN )
		    scia_cal->spectra[np][no] += scia_cal->correction[np][no];
	  } while ( ++no < scia_cal->num_obs );
     } while ( ++np < scia_cal->num_xpixels );
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
{
     const char prognm[] = "SCIA_CALC_STRAY_CORR";

     register unsigned short nch, ng;
     register size_t ni, nr;

     register size_t no = 0;
     register size_t fillings, nobs;

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

     /* set each element of the array equal to its subscript */
     for ( nr = 0; nr < SCIENCE_PIXELS; nr++ ) grid_f[nr] = (float) nr;

     /* read straylight correction matrix */
     SCIA_RD_H5_STRAY( &stray );
     if ( IS_ERR_STAT_FATAL )
     	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, "SCIA_RD_H5_STRAY" );

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
		    if ( (nr+1 ) < stray.dims[1] ) {
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
		    if ( (nr+1) < stray.dims[1]
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
     /* loop over spectra*/
     spec_r = (float *) malloc( stray.dims[1] * sizeof(float) );
     if ( spec_r == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "spec_r" );
     stray_r = (float *) malloc( stray.dims[0] * sizeof(float) );
     if ( stray_r == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "stray_r");
     do {
	  register size_t np = 0;

	  /* recontruct full spectra (8192 pixels) */
	  do {
	       fillings = NINT(scia_cal->pet[np] / scia_cal->pet_min);

	       if ( fillings > 1 ) {
		    nobs = (fillings - 1) + fillings * (no / fillings);

		    spec_f[np] = (float) scia_cal->spectra[np][nobs] / fillings;
	       } else if ( fillings == 1 ) {
		    spec_f[np] = (float) scia_cal->spectra[np][no];
	       } else {
		    spec_f[np] = 0.f;
	       }
	  } while ( ++np < scia_cal->num_xpixels );
#ifdef DEBUG
	  (void) fwrite( spec_f, sizeof(float), SCIENCE_PIXELS, fp_full );
#endif
	  /* reduce dimension of spectrum to stray.grid_in */
	  for ( nr = 0; nr < stray.dims[1]; nr++ ) {
	       unsigned short numval = 0;
	       unsigned short allval = grid_in_ul[nr] - grid_in_ll[nr] + 1;

	       spec_r[nr] = 0;
	       for ( ng = grid_in_ll[nr]; ng <= grid_in_ul[nr]; ng++ ) {
		    if ( scia_cal->quality_flag[ng] != FLAG_DEAD
			 && scia_cal->quality_flag[ng] != FLAG_SATURATE
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
	  for ( np = 0; np < scia_cal->num_xpixels; np++ ) {
	       if ( scia_cal->quality_flag[np] == FLAG_BLINDED )
		    scia_cal->correction[np][no] = 0;
	       else
		    scia_cal->correction[np][no] = stray_f[np];
	  }
     } while ( ++no < scia_cal->num_obs );
#ifdef DEBUG
     (void) fclose( fp_full );
     (void) fclose( fp_grid );
     (void) fclose( fp_corr_full );
     (void) fclose( fp_corr_grid );
#endif
done:
     if ( grid_in_ll != NULL ) free( grid_in_ll );
     if ( grid_in_ul != NULL ) free( grid_in_ul );
     if ( deriv_out != NULL ) free( deriv_out );
     if ( spec_r  != NULL ) free( spec_r );
     if ( stray_r != NULL ) free( stray_r );
     SCIA_FREE_H5_STRAY( &stray );
}

static
void SCIA_PATCH_STRAY_CORR( struct scia_cal_rec *scia_cal,
			const struct state1_scia *state, 
			struct mds1_scia *mds_1b )
       /*@modifies mds_1b@*/
{
     register unsigned short ncl = 0;

     float corr[SCIENCE_CHANNELS];

     /*
      * calculate new scaleFactors
      */
     (void) memset( corr, 0, SCIENCE_CHANNELS * sizeof(float) );
     do {
	  register size_t np = 0;
	  register size_t nf;

	  const size_t fillings = 
		NINT(state->Clcon[ncl].pet / scia_cal->pet_min);

	  do {
	       register unsigned short nd = 0;

	       const unsigned short ich = state->Clcon[ncl].channel - 1;
	       const unsigned short ipx = absPixelID(state->Clcon[ncl], np);

	       do {
		    register unsigned short nr;

		    register size_t nobs = 0;

		    if ( mds_1b[nd].clus[ncl].n_sig > 0 ) {
			 for ( nr = 0; nr < state->Clcon[ncl].n_read; nr++ ) {
			      register float corrval = 0.f;

			      for ( nf = 0; nf < fillings; nf++ ) {
				   corrval += scia_cal->correction[ipx][nobs];
				   nobs++;
			      }
			      if ( corr[ich] < corrval ) corr[ich] = corrval;
			 }
		    }
		    if ( mds_1b[nd].clus[ncl].n_sigc > 0 ) {
			 for ( nr = 0; nr < state->Clcon[ncl].n_read; nr++ ) {
			      register unsigned short nc = 0;
			      register float corrval = 0.f;

			      do {
				   for ( nf = 0; nf < fillings; nf++ ) {
					corrval += 
					     scia_cal->correction[ipx][nobs];
					nobs++;
				   }
			      } while ( ++nc < state->Clcon[ncl].coaddf );
			      if ( corr[ich] < corrval ) corr[ich] = corrval;
			 }
		    }
	       } while ( ++nd < state->num_dsr );
	  } while ( ++np < (size_t) state->Clcon[ncl].length );
      } while ( ++ncl < state->num_clus );
     /*
      * store new scaleFactors
      */
     {
	  register unsigned short nd = 0;

	  do {
	       mds_1b[nd].scale_factor[0] = 
		    (unsigned char) ceilf(10 * corr[0] / UCHAR_MAX);
	       mds_1b[nd].scale_factor[1] = 
		    (unsigned char) ceilf(10 * corr[1] / UCHAR_MAX);
	       mds_1b[nd].scale_factor[2] = 
		    (unsigned char) ceilf(10 * corr[2] / UCHAR_MAX);
	       mds_1b[nd].scale_factor[3] = 
		    (unsigned char) ceilf(10 * corr[3] / UCHAR_MAX);
	       mds_1b[nd].scale_factor[4] = 
		    (unsigned char) ceilf(10 * corr[4] / UCHAR_MAX);
	       mds_1b[nd].scale_factor[5] = 
		    (unsigned char) ceilf(10 * corr[5] / UCHAR_MAX);
	       mds_1b[nd].scale_factor[6] = 
		    (unsigned char) ceilf(10 * corr[6] / UCHAR_MAX);
	       mds_1b[nd].scale_factor[7] = 
		    (unsigned char) ceilf(10 * corr[7] / UCHAR_MAX);
	  } while ( ++nd < state->num_dsr );
     }
     /*
      * store new straylight correction factors
      */
     ncl = 0;
     do {
	  register size_t np = 0;
	  register size_t nf;

	  const size_t fillings = 
		NINT(state->Clcon[ncl].pet / scia_cal->pet_min);

	  do {
	       register unsigned short nd = 0;

	       const unsigned short ich = state->Clcon[ncl].channel - 1;
	       const unsigned short ipx = absPixelID(state->Clcon[ncl], np);
	       const float scaleFactor = mds_1b[nd].scale_factor[ich] / 10.f;

	       /* do not patch straylight of channel 1 */
	       if ( ich == 0 ) continue;

	       do {
		    register unsigned short nr;

		    register size_t nobs = 0;

		    if ( mds_1b[nd].clus[ncl].n_sig > 0 ) {
			 register size_t nb = np;

			 for ( nr = 0; nr < state->Clcon[ncl].n_read; nr++ ) {
			      register float corrval = 0.f;

			      for ( nf = 0; nf < fillings; nf++ ) {
				   corrval += scia_cal->correction[ipx][nobs];
				   nobs++;
			      }
			      mds_1b[nd].clus[ncl].sig[nb].stray =
				   __ROUNDf_uc( corrval / scaleFactor );
			      nb += state->Clcon[ncl].length;
			 }
		    }
		    if ( mds_1b[nd].clus[ncl].n_sigc > 0 ) {
			 register size_t nb = np;

			 for ( nr = 0; nr < state->Clcon[ncl].n_read; nr++ ) {
			      register unsigned short nc = 0;
			      register float corrval = 0.f;

			      do {
				   for ( nf = 0; nf < fillings; nf++ ) {
					corrval += 
					     scia_cal->correction[ipx][nobs];
					nobs++;
				   }
			      } while ( ++nc < state->Clcon[ncl].coaddf );
			      mds_1b[nd].clus[ncl].sigc[nb].stray =
				  __ROUNDf_uc( corrval / scaleFactor );
			      nb += state->Clcon[ncl].length;
			 }
		    }
	       } while ( ++nd < state->num_dsr );
	  } while ( ++np < (size_t) state->Clcon[ncl].length );
      } while ( ++ncl < state->num_clus );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_PATCH_MDS( FILE *fp, unsigned short patch_flag,
			 const struct state1_scia *state, 
			 struct mds1_scia *mds_1b )
{
     const char prognm[] = "SCIA_LV1_PATCH_MDS";

     register unsigned short nch, ncl, np;

     char *write_h5_scia_cal = getenv( "SCIA_WR_PATCH_INTERIM" );

     unsigned int      calib_flag;

     struct mph_envi   mph;
     struct scia_cal_rec scia_cal = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.f, 0.f, 
				      NULL, NULL, NULL, NULL, NULL,  
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
     scia_cal.num_channels = SCIENCE_CHANNELS;
     scia_cal.num_xpixels  = SCIENCE_PIXELS;
     scia_cal.num_ypixels  = 0;
     scia_cal.orbit_phase  = state->orbit_phase;
/*
 * obtain minimum PET and maximum number of read-outs
 */
     scia_cal.num_obs = 0;
     scia_cal.pet_min = 1000.f;
     for ( ncl = 0; ncl < state->num_clus; ncl++ ) {
	  if ( state->Clcon[ncl].pet >= 0.0624f
	       && scia_cal.pet_min > state->Clcon[ncl].pet ) {
               scia_cal.pet_min = state->Clcon[ncl].pet;
	       scia_cal.num_obs = 
		    state->Clcon[ncl].n_read * state->Clcon[ncl].coaddf;
	  }
     }
     scia_cal.num_obs *= state->num_dsr;
/*
 * allocate space for the SCIA spectra
 */
     scia_cal.coaddf = (unsigned char *) 
	  calloc( sizeof( char ), scia_cal.num_xpixels );
     if ( scia_cal.coaddf == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.coaddf" );
     scia_cal.chan_id = (unsigned char *) 
	  calloc( sizeof( char ), scia_cal.num_xpixels );
     if ( scia_cal.chan_id == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.chan_id" );
     scia_cal.quality_flag = (unsigned char *) 
	  calloc( sizeof( char ), scia_cal.num_xpixels );
     if ( scia_cal.quality_flag == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.quality_flag" );
     scia_cal.dark_signal = (float *) 
	  calloc( sizeof( float ), scia_cal.num_xpixels );
     if ( scia_cal.dark_signal == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.dark_signal" );
     scia_cal.pet = (float *) calloc( sizeof( float ), scia_cal.num_xpixels );
     if ( scia_cal.pet == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.pet" );
     scia_cal.coadd_chan_min = (unsigned char *)
	  calloc( sizeof( char ), scia_cal.num_channels );
     if ( scia_cal.coadd_chan_min == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.coadd_chan_min" );
     scia_cal.pet_chan_min = (float *)
	  calloc( sizeof( float ), scia_cal.num_channels );
     if ( scia_cal.pet_chan_min == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.pet_chan_min" );
     scia_cal.chan_mean = 
	  ALLOC_D2D( scia_cal.num_channels, scia_cal.num_obs );
     if ( scia_cal.chan_mean == NULL ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.chan_mean" );
     scia_cal.spectra = ALLOC_D2D( scia_cal.num_xpixels, scia_cal.num_obs );
     if ( scia_cal.spectra == NULL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.spectra" );
     scia_cal.correction = ALLOC_R2D( scia_cal.num_xpixels, scia_cal.num_obs );
     if ( scia_cal.correction == NULL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "scia_cal.correction" );
     if ( verbose )
	  (void) fprintf(stderr,"Allocated memory for calibration structure\n");

     /* write for each pixel: channel ID, coadding-factor, PET */
     for ( ncl = 0; ncl < state->num_clus; ncl++ ) {
	  for ( np = 0; np < state->Clcon[ncl].length; np++ ) {
	       unsigned short ipx = absPixelID(state->Clcon[ncl],np);
	  
	       scia_cal.coaddf[ipx] = state->Clcon[ncl].coaddf;
	       scia_cal.chan_id[ipx] = state->Clcon[ncl].channel;
	       scia_cal.pet[ipx] = state->Clcon[ncl].pet;
	  }
     }
     /* obtain per channel: minimum coadding-factor and PET */
     for ( nch = 0; nch < SCIENCE_CHANNELS; nch++ ) {
	  unsigned char coaddf_chan_min = UCHAR_MAX;
	  float pet_chan_min = 999.f;

	  for ( ncl = 0; ncl < state->num_clus; ncl++ ) {
	       if ( nch == (state->Clcon[ncl].channel-1) ) {
		    if ( coaddf_chan_min > state->Clcon[ncl].coaddf )
			 coaddf_chan_min = state->Clcon[ncl].coaddf;
		    if ( pet_chan_min > state->Clcon[ncl].pet )
			 pet_chan_min = state->Clcon[ncl].pet;
	       }
	  }
	  scia_cal.coadd_chan_min[nch] = 
	       (coaddf_chan_min == UCHAR_MAX) ? 0 : coaddf_chan_min;
	  scia_cal.pet_chan_min[nch]   = 
	       (pet_chan_min < 999.f) ? pet_chan_min : -1.f;
     }
     /* 
      * read darkcurrent parameters
      */
     calib_flag = (DO_CORR_AO | DO_CORR_DARK | DO_SRON_DARK);
     SCIA_CAL_GET_DARK( fp, calib_flag, &scia_cal );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "SCIA_CAL_GET_DARK" );
     if ( verbose )
	  (void) fprintf( stderr, "Read Dark current parameters\n" );
     /*
      * fill spectra array...
      */
     SCIA_FILL_SPECTRA( state, mds_1b, &scia_cal );
     if ( verbose )
	  (void) fprintf( stderr, "Wrote data to calibration structure\n" );
     /*
      * compute estimates of channel averages at PET_chan_min 
      * based on PMD read-outs
      */
     SCIA_CALC_CHAN_MEAN( state, mds_1b, &scia_cal );
     if ( verbose )
	  (void) fprintf( stderr, "Calculated channel averages\n" );
     /*
      * obtain quality flags
      */
     SCIA_SET_FLAG_QUALITY( &scia_cal );
     if ( verbose )
	  (void) fprintf( stderr, "Performed quality flagging\n" );
     /*
      * de-coadd values using channel averages
      */
     SCIA_CAL_DE_COADD( &scia_cal );
     if ( verbose )
	  (void) fprintf( stderr, "Performed de-coadd of spectral data\n" );
     /*
      * apply Memory correction
      */
     (void) memset( scia_cal.correction[0], 0,
		    sizeof(float) * scia_cal.num_obs * scia_cal.num_xpixels );
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
	  SCIA_APPLY_CORR( &scia_cal );
	  if ( verbose )
	       (void) fprintf( stderr, "Before Straylight correction\n" );
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
     if ( scia_cal.coadd_chan_min != NULL ) free( scia_cal.coadd_chan_min );
     if ( scia_cal.quality_flag != NULL ) free( scia_cal.quality_flag );
     if ( scia_cal.dark_signal != NULL ) free( scia_cal.dark_signal );
     if ( scia_cal.pet != NULL ) free( scia_cal.pet );
     if ( scia_cal.pet_chan_min != NULL ) free( scia_cal.pet_chan_min );
     if ( scia_cal.chan_mean != NULL ) FREE_2D( (void **)scia_cal.chan_mean );
     if ( scia_cal.spectra != NULL ) FREE_2D( (void **) scia_cal.spectra );
     if ( scia_cal.correction != NULL ) FREE_2D((void **) scia_cal.correction);
     if ( verbose )
	  (void) fprintf( stderr, "Finished module: %s\n", prognm );
}
