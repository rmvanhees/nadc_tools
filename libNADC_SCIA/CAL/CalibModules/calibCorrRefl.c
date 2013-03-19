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

.IDENTifer   calibAtbdRefl
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy - Level 1b - Calibration
.LANGUAGE    ANSI C
.PURPOSE     convert Radiances to Reflectance on Sciamachy L1b science data
.INPUT/OUTPUT
  call as   SCIA_ATBD_CAL_REFL( fileParam, state, mds_1c );
            SCIA_SRON_CAL_REFL( fileParam, state, mds_1c );
     input:  
             struct file_rec *fileParam : file/calibration parameters
	     struct state1_scia *state  : structure with States of the product
 in/output:  
             struct mds1c_scia *mds_1c  : level 1c MDS records
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      2.1   17-Mar-2011 merged code, RvH
              2.0   07-Dec-2009 read SMR data from SDMF v3.0, PvdM
              1.1   30-Jul-2007 added m-factor correction, RvH & KB
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
.IDENTifer   Get_DopplerCorrSRS
.PURPOSE     correct wavelength of the SRS for Doppler shift
.INPUT/OUTPUT
  call as   Get_DopplerCorrSRS( srs, indx_to_chan, wvlen_sun );

     input:  
            struct srs_scia *srs        : Solar Reference Spectrum (DSD)
	    unsigned int   indx_to_chan : index to wavelength grid in SRS
    output:  
            double *wvlen_sun           : Doppler corrected wavelength grid

.RETURNS     Nothing
.COMMENTS    static function
             apply Doppler correction: 
	           dLambda / Lambda = V_r / c
	     thus the correction is:
	           wv_corr = wv_srs * (1 + V_r / c)
-------------------------*/
static inline
void Get_DopplerCorrSRS( const struct srs_scia *srs, 
			 unsigned int indx_to_chan, 
			 /*@out@*/ double *wvlen_sun )
{
     register unsigned int np = 0;

     register const float *pntr_wvchan = srs->wvlen_sun + indx_to_chan;

     const double DopplerCorr = 1 + srs->dopp_shift / 500.;

     do {
	  *wvlen_sun++ = *pntr_wvchan++ * DopplerCorr;
     } while ( ++np < CHANNEL_SIZE );
}

/*+++++++++++++++++++++++++
.IDENTifer   getSolarSpectrumSRON_30
.PURPOSE     obtain mean Solar spectrum from SDMF (v3.0)
.INPUT/OUTPUT
  call as    stat = getSolarSpectrumSRON_30( fileParam, struct srs_scia *srs );
     input:
           struct file_rec *fileParam : file/calibration parameters
 in/output:
	   struct srs_scia *srs       : Solar Reference Spectrum

.RETURNS     flag: FALSE (no SMR found) or TRUE
.COMMENTS    None
-------------------------*/
static
bool getSolarSpectrumSRON_30( const struct file_rec *fileParam, 
			     struct srs_scia *srs )
      /*@globals  errno, nadc_stat, nadc_err_stack;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack, srs->mean_sun @*/
{
    const char prognm[]  = "getSolarSpectrumSRON_30";

    register unsigned short ip, ipix, ich;

    bool ret = FALSE;

    hid_t fid = -1;

    int   center_idx;
    float asm_pos, esm_pos, sun_zen_ang;
    float smr[SCIENCE_PIXELS];

    int center240_idx[] = {0, 120};
    int slabsize[]      = {1,1};

    /* max 4 sun measurements in this range */
    int numIndx, metaIndx[4];          

    struct wvlen_rec wvlen;

    const char sdmf_smr_db[] = "/SCIA/SDMF30/sdmf_smr.h5";
    
    const int pixel_range[] = {0, SCIENCE_PIXELS-1};

    const unsigned short state62_coadds[SCIENCE_CHANNELS] = {
	 2, 2, 2, 2, 2, 2, 2, 1 
    };

    /* orbits per day = 14.3, rounded up to 15 */
    const int orbit_range[] = {
	 fileParam->absOrbit-15, fileParam->absOrbit+15 };
/*
 * copy wavelength grid of Solar spectrum
 */
    (void) memcpy( wvlen.science, srs->wvlen_sun, 
		   SCIENCE_PIXELS * sizeof(float) );
/*
 * open HDF5-file
 */
    if ( (fid = H5Fopen( sdmf_smr_db, H5F_ACC_RDONLY, H5P_DEFAULT )) < 0 )
	 NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, sdmf_smr_db );
/*
 * get indices of nearest orbits
 */
    SDMF_get_metaIndex_range(fid, orbit_range, &numIndx, metaIndx, 1);
    if (IS_ERR_STAT_FATAL)
        NADC_GOTO_ERROR(prognm, NADC_ERR_FATAL, "SDMF_get_metaIndex");
    
/*
 * read SMR and HK information (HK information is on a finer time grid:
 * 240 measurements)
 */
    center_idx       = metaIndx[numIndx/2];
    center240_idx[0] = center_idx;
    SDMF_rd_float_Array( fid, "SMR", 1, &center_idx, pixel_range, smr );
    __Inverse_Chan2( smr );

    SDMF_rd_float_Matrix( fid, "asm", center240_idx, slabsize, 2, &asm_pos );
    SDMF_rd_float_Matrix( fid, "esm", center240_idx, slabsize, 2, &esm_pos );
    SDMF_rd_float_Matrix( fid, "sunel", center240_idx, slabsize, 2, 
			  &sun_zen_ang );
/*
 * return SMR to co-added format (BU). radiometric calibration will do 
 * normalisation per second, assuming non-coadded input.
 */ 
    for ( ip = 0, ich = 0; ich < SCIENCE_CHANNELS; ich++ ) {
        /* state 62 coadding factors per channel */
        for ( ipix = 0; ipix < CHANNEL_SIZE; ipix++ )
	     smr[ip++] *= state62_coadds[ich];
    }
/*
 * Apply radiance calibration
 * correct SZA and ASM angle
 */
    asm_pos = -2. * (asm_pos + 45.);
    sun_zen_ang += 90.;
    SCIA_ATBD_CAL_RAD_DETWIDE( fileParam, wvlen, smr, 
			       &asm_pos, &sun_zen_ang, 1 );
    (void) memcpy( srs->mean_sun, smr, SCIENCE_PIXELS * sizeof(float) );
    ret = TRUE;
/*
 * close HDF5-file
 */
 done:
    if ( fid >= 0 ) H5Fclose( fid );

    return ret;
}


/*+++++++++++++++++++++++++
.IDENTifer   Apply_RadNormNadir
.PURPOSE     convert radiances to reflectances
.INPUT/OUTPUT
  call as    Apply_RadNormNadir( srs, mds_1c );
     input:
	   struct srs_scia *srs      : Solar Reference Spectrum
 in/output:
	   struct mds1c_scia *mds_1c : level 1c MDS records

.RETURNS     nothing
.COMMENTS    Note the spectral radiances are assumed to be normalized
-------------------------*/
static
void Apply_RadNormNadir( const struct srs_scia *srs, 
			 struct mds1c_scia *mds_1c )
{
     register unsigned short no, np;
     register float          *signal = mds_1c->pixel_val;

     double wvlen_sun[CHANNEL_SIZE];
     double SunRefFit[CHANNEL_SIZE];

/* constants */
     const unsigned short ichan = mds_1c->chan_id - (unsigned short) 1;
     const unsigned short offs  = ichan * CHANNEL_SIZE;
/*
 * obtain wavelength of the Sun reference spectrum corrected for Doppler shift
 */
     Get_DopplerCorrSRS( srs, offs, wvlen_sun );
/*
 * Interpolate Solar spectrum to wavelength grid
 * And correct the cluster data with radiance response and integration time
 */
     FIT_GRID_AKIMA( FLT64_T, FLT32_T, CHANNEL_SIZE,  
		     wvlen_sun, srs->mean_sun + offs, 
		     FLT32_T, FLT64_T, (size_t) mds_1c->num_pixels, 
		     mds_1c->pixel_wv, SunRefFit );
/*
 * convert radiances to reflectances
 */
     no = 0;                                 /* number of observations */
     do {
	  double norm_mu0 = cos( mds_1c->geoN[no].sun_zen_ang[1] * DEG2RAD );
	  
	  np = 0;                            /* number of pixels per cluster */
	  do {
	       if ( isnormal( *signal ) && isnormal( SunRefFit[np] )
		    && SunRefFit[np] > 10e-15 ) {
		    *signal = (float) 
			 (PI * (*signal) / (norm_mu0 * SunRefFit[np]));
	       } else
		    *signal = NAN;
	  } while ( ++signal, ++np < mds_1c->num_pixels );
     } while ( ++no < mds_1c->num_obs );
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_RadNormLimb
.PURPOSE     convert radiances to reflectances
.INPUT/OUTPUT
  call as    Apply_RadNormLimb( srs, mds_1c );
     input:
	   struct srs_scia *srs      : Solar Reference Spectrum
 in/output:
	   struct mds1c_scia *mds_1c : level 1c MDS records

.RETURNS     nothing
.COMMENTS    Note the spectral radiances are assumed to be normalized
-------------------------*/
static
void Apply_RadNormLimb( const struct srs_scia *srs, struct mds1c_scia *mds_1c )
{
     register unsigned short no, np;
     register float          *signal = mds_1c->pixel_val;

     double SunRefFit[CHANNEL_SIZE];

/* constants */
     const unsigned short ichan = mds_1c->chan_id - (unsigned short ) 1;
     const unsigned short offs  = ichan * CHANNEL_SIZE;
/*
 * obtain wavelength of the Sun reference spectrum corrected for Doppler shift
 */
/*      Get_DopplerCorrSRS( srs, offs, wvlen_sun ); */
/*
 * Interpolate Solar spectrum to wavelength grid
 * And correct the cluster data with radiance response and integration time
 */
     FIT_GRID_AKIMA( FLT32_T, FLT32_T, CHANNEL_SIZE,  
		     srs->wvlen_sun + offs, srs->mean_sun + offs, 
		     FLT32_T, FLT64_T, (size_t) mds_1c->num_pixels, 
		     mds_1c->pixel_wv, SunRefFit );
/*
 * convert radiances to reflectances
 */
     no = 0;                                 /* number of observations */
     do {
	  np = 0;                            /* number of pixels per cluster */
	  do {
	       if ( isnormal( *signal ) && isnormal( SunRefFit[np] )
		    && SunRefFit[np] > 10e-15 ) {
		    *signal = (float) 
			 (PI * (*signal) / SunRefFit[np]);
	       } else
		    *signal = NAN;
	  } while ( ++signal, ++np < mds_1c->num_pixels );
     } while ( ++no < mds_1c->num_obs );
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_RadNormMoni
.PURPOSE     convert radiances to reflectances
.INPUT/OUTPUT
  call as    Apply_RadNormMoni( srs, mds_1c );
     input:
	   struct srs_scia *srs      : Solar Reference Spectrum
 in/output:
	   struct mds1c_scia *mds_1c : level 1c MDS records

.RETURNS     nothing
.COMMENTS    Note the spectral radiances are assumed to be normalized
-------------------------*/
static
void Apply_RadNormMoni( const struct srs_scia *srs, struct mds1c_scia *mds_1c )
{
     register unsigned short no, np;
     register float          *signal = mds_1c->pixel_val;

     double SunRefFit[CHANNEL_SIZE];

/* constants */
     const unsigned short ichan = mds_1c->chan_id - (unsigned short) 1;
     const unsigned int   offs  = ichan * CHANNEL_SIZE;
/*
 * obtain wavelength of the Sun reference spectrum corrected for Doppler shift
 */
/*      Get_DopplerCorrSRS( srs, offs, wvlen_sun ); */
/*
 * Interpolate Solar spectrum to wavelength grid
 * And correct the cluster data with radiance response and integration time
 */
     FIT_GRID_AKIMA( FLT32_T, FLT32_T, CHANNEL_SIZE,  
		     srs->wvlen_sun + offs, srs->mean_sun + offs, 
		     FLT32_T, FLT64_T, (size_t) mds_1c->num_pixels, 
		     mds_1c->pixel_wv, SunRefFit );
/*
 * convert radiances to reflectances
 */
     no = 0;                                 /* number of observations */
     do {
	  np = 0;                            /* number of pixels per cluster */
	  do {
	       if ( isnormal( *signal ) && isnormal( SunRefFit[np] )
		    && SunRefFit[np] > 10e-15 ) {
		    *signal = (float) (PI * (*signal) / SunRefFit[np]);
	       } else
		    *signal = NAN;
	  } while ( ++signal, ++np < mds_1c->num_pixels );
     } while ( ++no < mds_1c->num_obs );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_ATBD_CAL_REFL( const struct file_rec *fileParam,
			 const struct state1_scia *state,
			 struct mds1c_scia *mds_1c )
{
     const char prognm[] = "SCIA_ATBD_CAL_REFL";

     register unsigned short num = 0u;
     static struct srs_scia srs;

     if ( fileParam->flagInitFile ) {
	  unsigned int num_dsr;
	  struct srs_scia *srs_all;

	  const bool Save_Extern_Alloc = Use_Extern_Alloc;

	  Use_Extern_Alloc = FALSE;
	  num_dsr = SCIA_LV1_RD_SRS( fileParam->fp, fileParam->num_dsd, 
				     fileParam->dsd, &srs_all );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "SRS" );
	  /* KB: Apply mfactor, if needed */
	  if ( (fileParam->calibFlag & DO_MFACTOR_RAD) != UINT_ZERO ) {
	       SCIA_LV1_MFACTOR_SRS( fileParam->sensing_start, 
				     fileParam->calibFlag, num_dsr, srs_all );
	  }
	  (void) memcpy( &srs, &srs_all[fileParam->level_2_smr[0]],
			 sizeof(struct srs_scia) );
	  free ( srs_all );
	  Use_Extern_Alloc = Save_Extern_Alloc;
     }
/*
 * apply radiance sensitivy correction
 */
     do {
	  switch ( (int) state->type_mds ) {
	  case SCIA_NADIR:
	       Apply_RadNormNadir( &srs, mds_1c );
	       break;
	  case SCIA_MONITOR:
	       Apply_RadNormMoni( &srs, mds_1c );
	       break;
	  default:
	       Apply_RadNormLimb( &srs, mds_1c );
	       break;
	  }
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "AKIMA" );
     } while ( ++mds_1c, ++num < state->num_clus );
}

void SCIA_SRON_CAL_REFL( const struct file_rec *fileParam,
			 const struct state1_scia *state,
			 struct mds1c_scia *mds_1c )
{
     const char prognm[] = "SCIA_SRON_CAL_REFL";

     register unsigned short num = 0u;
     static struct srs_scia srs;

     if ( fileParam->flagInitFile ) {
	  unsigned int num_dsr;
	  unsigned short indx_smr = fileParam->level_2_smr[0];
	  struct srs_scia *srs_all;

	  const bool Save_Extern_Alloc = Use_Extern_Alloc;

	  Use_Extern_Alloc = FALSE;
	  num_dsr = SCIA_LV1_RD_SRS( fileParam->fp, fileParam->num_dsd, 
				     fileParam->dsd, &srs_all );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "SRS" );

	  /* read SDMF Solar spectrum */
	  if ( ! getSolarSpectrumSRON_30( fileParam, srs_all+indx_smr ) )
              NADC_RETURN_ERROR(prognm, NADC_ERR_FATAL, "SMR_v3.0");

	  /* KB: Apply mfactor, if needed */
	  if ( (fileParam->calibFlag & DO_MFACTOR_RAD) != UINT_ZERO ) {
	       SCIA_LV1_MFACTOR_SRS( fileParam->sensing_start, 
				     fileParam->calibFlag, num_dsr, srs_all );
	  }
	  (void) memcpy( &srs, &srs_all[indx_smr], sizeof(struct srs_scia) );
	  free ( srs_all );
	  Use_Extern_Alloc = Save_Extern_Alloc;
     }
/*
 * apply radiance sensitivy correction
 */
     do {
	  switch ( (int) state->type_mds ) {
	  case SCIA_NADIR:
	       Apply_RadNormNadir( &srs, mds_1c );
	       break;
	  case SCIA_MONITOR:
	       Apply_RadNormMoni( &srs, mds_1c );
	       break;
	  default:
	       Apply_RadNormLimb( &srs, mds_1c );
	       break;
	  }
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "AKIMA" );
     } while ( ++mds_1c, ++num < state->num_clus );
}
