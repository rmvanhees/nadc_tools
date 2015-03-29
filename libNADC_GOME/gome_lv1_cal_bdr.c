/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 1999 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GOME_LV1_CAL_BDR
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1
.LANGUAGE    ANSI C
.PURPOSE     calibrate Earth, Moon, Sun Spectral Band Data Records
.INPUT/OUTPUT
  call as   CALIB_PCD_BDR( calib_flag, nband, fcd, pcd, nr_rec, rec );
     input:  
            unsigned short calib_flag :  calibration flag
            short nband               :  number of spectral band [1a=0,1b,2a..]
	    struct fcd_gome   *fcd  :  Fixed Calibration Data Record
	    struct pcd_gome   *pcd  :  Pixel Specific Calibration Records
	    short nr_rec :               number of band data records
    output:  
	    struct rec_gome   *rec  :  Spectral Band Data Records 

  call as   CALIB_SMCD_BDR( calib_flag, nband, fcd, smcd, nr_rec, rec ); 
     input:  
            unsigned short calib_flag :  calibration flag
            short nband               :  number of spectral band [1a=0,1b,2a..]
	    struct fcd_gome   *fcd  :  Fixed Calibration Data Record
	    struct smcd_gome  *smcd :  Sun/Moon Specific Calibration Records
	    short nr_rec :               number of band data records
    output:  
	    struct rec_gome   *rec  :  Spectral Band Data Records 

.RETURNS     Nothing
.COMMENTS    calibration algorithms taken from the DLR GDP version 2.
.ENVIRONment None
.VERSION      3.1   11-Mar-2003	correct raw detector data for co-adding, RvH
              3.0   11-Nov-2001	moved to the new Error handling routines, RvH
              2.2   15-Aug-2001	bugs Bugs BUGS... removed, RvH 
              2.1   27-Jul-2001 added more calibration option, RvH
              2.0   26-Jul-2001 completed implementation of 
                                  GDP calibration, RvH
              1.2   23-Nov-2000 updated documentation, RvH
              1.1   16-May-2000 found (stupid) bug in Triangle, RvH
              1.0   02-Mar-2000 Created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static inline
float LinearInterpol( short X, short X_left, short X_right, 
		      float Y_left, float Y_right )
{
     double Y_delta;

     if ( X_right == X_left ) return Y_right;

     Y_delta = (double)(Y_left - Y_right) / (double)(X_left - X_right);

     return (float)(Y_left + Y_delta * (X - X_left));
}

/*+++++++++++++++++++++++++
.IDENTifer   Convol
.PURPOSE     smoothes data by a triangular convolution
.INPUT/OUTPUT
  call as    Convol( kernelhalfwidth, kernel, bcr_start, bcr_end, 
                     data, data_out );
     input:
            short kernelhalfwidth :
	    float *kernel         :
	    short bcr_start       :
	    short bcr_end         :
	    float *data           :
    output:
	    float *data_out       :
            
.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static
void Convol( short kernelhalfwidth, const float *kernel,
	     short bcr_start, short bcr_end, 
	     const float *data, /*@out@*/ float *data_out )
     /*@modifies data_out@*/
{
     register short nc;
     register const float *pntr_in  = data;
     register float       *pntr_out = data_out;

     for ( nc = bcr_start; nc <= bcr_end; nc++ ) {
	  register short ni = -kernelhalfwidth;

	  register float wght = 0.f;
	  register float sum = 0.f;

	  do {
	       if ( (nc + ni) >= 0 && (nc + ni) < CHANNEL_SIZE ) {
		    wght += kernel[kernelhalfwidth + ni];
		    sum  += kernel[kernelhalfwidth + ni] * pntr_in[nc + ni];
	       }
	  } while ( ++ni <= kernelhalfwidth );
	  pntr_out[nc] = (wght == 0.f) ? 0.f : (sum / wght);
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   Triangle
.PURPOSE     smoothes data by a triangular convolution
.INPUT/OUTPUT
  call as    Triangle( trianglehalfwidth, bcr_start, bcr_end, data, data_out );
     input:
            short trianglehalfwidth :
	    short bcr_start         :
	    short bcr_end           :
	    float *data             :
    output:
            float *data_out         :

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static
void Triangle( short trianglehalfwidth, short bcr_start, short bcr_end, 
	       const float *data, /*@out@*/ float *data_out )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, data_out@*/
{
     register short ni;

     float *kernel;

     const size_t TriangleWidth = (size_t) (2 * trianglehalfwidth + 1);

     kernel = (float *) malloc( TriangleWidth * sizeof( float ));
     if ( kernel == NULL ) 
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "kernel" );
     ni = -trianglehalfwidth;
     do {
	  kernel[trianglehalfwidth+ni] = 
	       (float)(trianglehalfwidth - abs(ni)) / trianglehalfwidth;
     } while ( ++ni <= trianglehalfwidth );

     Convol( trianglehalfwidth, kernel, bcr_start, bcr_end, data, data_out );
     free( kernel );
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_IntgFact
.PURPOSE     correct raw detector signal for co-adding
.INPUT/OUTPUT
  call as    Apply_IntgFact( factor, bcr_start, bcr_count, rec );
     input:
            short factor          :  multiplication factor
	    short bcr_start       :  first pixel in data record
	    short bcr_count       :  number of pixels per spectral band
    output:
            struct rec_gome *rec  :  structure for a spectral band record
            
.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static inline
void Apply_IntgFact( short factor, short bcr_start, short bcr_count, 
		     struct rec_gome *rec )
     /*@modifies rec@*/
{
     register short nc = 0;

     register float *pntr_data = rec->data + bcr_start;

     if ( factor == 1 ) return;

     do { *pntr_data++ *= factor; } while ( ++nc < bcr_count );
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_DarkSignal
.PURPOSE     subtract dark signal
.INPUT/OUTPUT
  call as    Apply_DarkSignal( dark, bcr_start, bcr_count, rec );
     input:
            float *dark           :  dark signal
	    short bcr_start       :  first pixel in data record
	    short bcr_count       :  number of pixels per spectral band
    output:
            struct rec_gome *rec:  structure for a spectral band record
            
.RETURNS     Nothing
.COMMENTS    static function
             The measured spectra are corrected for dark signal by subtraction
             of a Dark Spectrum which for each detector pixel is a combination
	     of readout fixed-pattern noise and leakage current.
-------------------------*/
static inline
void Apply_DarkSignal( const float *dark, short bcr_start, 
		       short bcr_count, struct rec_gome *rec )
     /*@modifies rec@*/
{
     register short nc = 0;

     register float *pntr_data = rec->data + bcr_start;
     register const float *pntr_dark = dark + bcr_start;

     do { *pntr_data++ -= *pntr_dark++; } while ( ++nc < bcr_count );
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_FPAnoise
.PURPOSE     correction for FPA noise
.INPUT/OUTPUT
  call as    Apply_PFAnoise( bcr_start, bcr_count, rec );
     input:
	    short bcr_start       :  first pixel in data record
	    short bcr_count       :  number of pixels per spectral band
    output:
            struct rec_gome *rec:  structure for a spectral band record
            
.RETURNS     Nothing
.COMMENTS    static function
             The correction is calculated as the Peltier output multiplied 
             by a scaling factor. The value of the Peltier output is for each
	     readout calculated as a weighted average of the last 61 Peltier
	     voltage readings. The filter coefficients and the scaling factor 
	     are present in the FCD record. The magnitude of the effect scales
	     with the integration time.
-------------------------*/
static
void Apply_FPAnoise( const struct fcd_gome *fcd,
                     short nr_pcd, const struct pcd_gome *pcd,
		     short nr_rec, short bcr_start, short bcr_count, 
		     struct rec_gome *rec )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, rec@*/
{
     register short ni, nr;

     short groundpixel, nr_intg, num, valid_indx;
     float avg_noise, scalefactor, sum;
     float *FPA_noise, *peltier;
/*
 * calucate FPA noise, which is an offset to the detector signals generated 
 * by switching the peltier cooler
 */
     FPA_noise = (float *) malloc( (size_t) nr_pcd * sizeof( float ));
     if ( FPA_noise == NULL ) 
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "FPA_noise" );
     peltier = (float *) malloc( (size_t) nr_pcd * sizeof( float ));
     if ( peltier == NULL ) {
	  free( FPA_noise );
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "peltier" );
     }
     nr = 0;
     do { 
	  peltier[nr] = (float) pcd[nr].ihr.peltier[0]; 
     } while ( ++nr < nr_pcd );

     Convol( fcd->npeltier, fcd->filter_peltier, 0, nr_pcd-1,
	     peltier, FPA_noise );
/*
 * extent using linear interpolation
 */
     valid_indx = fcd->npeltier / 2;
     for ( nr = 0; nr < valid_indx; nr++ )
	  FPA_noise[nr] = LinearInterpol( nr, valid_indx, valid_indx + 2,
					  FPA_noise[valid_indx],
					  FPA_noise[valid_indx + 2] );
     valid_indx = nr_pcd - fcd->npeltier / 2;
     for ( nr = valid_indx; nr < nr_pcd; nr++ )
	  FPA_noise[nr] = LinearInterpol( nr, valid_indx - 2, valid_indx,
					  FPA_noise[valid_indx - 2],
					  FPA_noise[valid_indx] );
/*
 * subtract low-pass-filtered peltier data
 */
     nr = 0;
     do {
	  FPA_noise[nr] = peltier[nr] - FPA_noise[nr];
     } while ( ++nr < nr_pcd );
     free ( peltier );
/*
 * calculate mean noise during the integration time
 */
     for ( nr = 0; nr < nr_rec; nr++ ) {
	  groundpixel = rec[nr].indx_pcd;
	  switch ( (int) rec[nr].integration[0] ) {
	  case 12: 
	       scalefactor = fcd->scale_peltier[0];
	       break;
	  case 24: 
	       scalefactor = fcd->scale_peltier[1];
	       break;
	  case 30: 
	       scalefactor = fcd->scale_peltier[2];
	       break;
	  case 48: 
	       scalefactor = fcd->scale_peltier[3];
	       break;
	  case 60: 
	       scalefactor = fcd->scale_peltier[4];
	       break;
	  default:
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, "invalid integration time" );
	  }
	  nr_intg = (short) (rec[nr].integration[0] / 1.5);
	  sum = 0.f;
	  num = 0;
	  for ( ni = 0; ni < nr_intg && (groundpixel - ni) >= 0; ni++ ) {
	       sum += FPA_noise[ groundpixel - ni];
	       num++;
	  }
	  if ( num > 0 ) {
	       avg_noise = sum * scalefactor / num;
	       for ( ni = bcr_start; ni < (bcr_start+bcr_count); ni++ )
		    rec[nr].data[ni] -= avg_noise;
	  }
     }
 done:
     free( FPA_noise );
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_PPG
.PURPOSE     Corrects signals for pixel-to-pixel variations
.INPUT/OUTPUT
  call as    Apply_PFAnoise( pixel_gain, bcr_start, bcr_count, rec );
     input:
            float *pixel_gain     :  pixel-to-pixel gain
	    short bcr_start       :  first pixel in data record
	    short bcr_count       :  number of pixels per spectral band
    output:
            struct rec_gome *rec:  structure for a spectral band record
            
.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static inline
void Apply_PPG( const float *pixel_gain, short bcr_start, 
		short bcr_count, struct rec_gome *rec )
     /*@modifies rec@*/
{
     register short nc = 0;
     register float *pntr_data = rec->data + bcr_start;
     register const float *pntr_ppg = pixel_gain + bcr_start;

     do { *pntr_data++ *= *pntr_ppg++; } while ( ++nc < bcr_count );
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_StrayLightCorrection
.PURPOSE     Corrects signals for straylight
.INPUT/OUTPUT
  call as    Apply_StrayLightCorrection( channel, fcd, rec );
     input:
            short channel         :  Science channel array
	    struct fcd_gome *fcd:  fixed calibration record
    output:
            struct rec_gome *rec:  structure for a spectral band record
            
.RETURNS     Nothing
.COMMENTS    static function
             The calibration keydata contain for each channel uniform 
             straylight factors (straylight scales with total intensity in 
	     the channel) and spectral ghost characteristics. The latter 
	     consists of the symmetry/asymmetry pixel, ghost intensity, 
	     and the ghost spectral width.
-------------------------*/
static
void Apply_StrayLightCorrection( short channel, const struct fcd_gome *fcd, 
				 struct rec_gome *rec )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, rec@*/
{
     register short nb, nc, ng, to_left, to_rght;
     register float *pntr, *pntr_data;

     short cntr, bcr_start, bcr_count, nband;
     float energy, uniform;
     float conv_ghost[CHANNEL_SIZE], ghost[CHANNEL_SIZE], signal[CHANNEL_SIZE];
/*
 * initialize arrays to zero
 */
     (void) memset( ghost, 0, CHANNEL_SIZE * sizeof(float) );
     (void) memset( signal, 0, CHANNEL_SIZE * sizeof(float) );
/*
 * normalize signal with integration time
 */
     for ( nb = 0; nb < NUM_BAND_IN_CHANNEL; nb++ ) {
	  if ( (nband = BandChannel(channel, nb)) != -1 
	       && rec->integration[nb] >= FLT_EPSILON ) {

	       nc = fcd->bcr[nband].start;
	       do { 
		    signal[nc] = rec->data[nc] / rec->integration[nb];
	       } while ( ++nc <= fcd->bcr[nband].end );
	  }
     }
/*
 * Uniform straylight level
 */
     uniform = 0.f;
     if ( fcd->stray_level[channel] >= FLT_EPSILON ) {
	  register float sum = 0.f;
	  register short num = 0;

	  for ( nb = 0; nb < NUM_BAND_IN_CHANNEL; nb++ ) {
	       if ( (nband = BandChannel(channel, nb)) != -1
		    && rec->integration[nb] >= FLT_EPSILON ) {

		    nc = fcd->bcr[nband].start;
		    do {
			 sum += signal[nc];
			 num++;
		    } while ( ++nc <= fcd->bcr[nband].end );
	       }
	  }
	  if ( num > 0 )
	       uniform = (sum * fcd->stray_level[channel]) / num;
     }
/*
 * Ghost straylight level
 */
     for ( ng = 0; ng < NUM_STRAY_GHOSTS; ng++ ) {
	  if ( (cntr = fcd->ghost.symmetry[channel][ng]) >= 0 ) {
	       energy = fcd->ghost.energy[channel][ng] / 100.f;
	       ghost[cntr] += signal[cntr] * energy;

	       to_left = cntr - 1;
	       to_rght = cntr + 1;
	       while ( to_left >= 0 && to_rght < CHANNEL_SIZE ) {
		    ghost[to_rght] += signal[to_left] * energy;
		    ghost[to_left] += signal[to_rght] * energy;
		    to_rght++;
		    to_left--;
	       }
	  }
     }
/*
 * subtract uniform and ghost straylight
 */
     for ( nb = 0; nb < NUM_BAND_IN_CHANNEL; nb++ ) {
	  if ( (nband = BandChannel(channel, nb)) != -1
	       && rec->integration[nb] >= FLT_EPSILON ) {

	       bcr_start = fcd->bcr[nband].start;
	       bcr_count = fcd->bcr[nband].end - fcd->bcr[nband].start + 1;
/*
 * smooth ghost straylight
 */
	       Triangle( fcd->width_conv, bcr_start, 
			 fcd->bcr[nband].end, ghost, conv_ghost );
	       if ( IS_ERR_STAT_FATAL )
	       	    NADC_RETURN_ERROR( NADC_ERR_FATAL,
	       			       "Triangle returned fatal error" );
/*
 * and subtract from detector data
 */
	       nc = 0;
	       pntr = conv_ghost + bcr_start;
	       pntr_data = rec->data + bcr_start;
	       do {
		    *pntr_data -= (uniform + *pntr) * rec->integration[nb];
		    pntr++; pntr_data++;
	       } while ( ++nc < bcr_count );
	  }
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_IntegrationNormalization
.PURPOSE     Normalize the signals to 1 second detector integration time.
.INPUT/OUTPUT
  call as    Apply_IntegrationNormalization( bcr_start, bcr_count, rec );
     input:
	    short bcr_start       :  first pixel in data record
	    short bcr_count       :  number of pixels per spectral band
    output:
            struct rec_gome *rec:  structure for a spectral band record
            
.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static inline
void Apply_IntegrationNormalization( short bcr_start, short bcr_count,
				     float integration, 
				     struct rec_gome *rec )
     /*@modifies rec@*/
{
     register short nc = 0;
     register float *pntr_data = rec->data + bcr_start;

     do { *pntr_data++ /= integration; } while ( ++nc < bcr_count ); 
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_PolarisationCorrection
.PURPOSE     Polarization correction
.INPUT/OUTPUT
  call as    Apply_PolarisationCorrection( bcr_start, bcr_count, rec );
     input:
	    short bcr_start       :  first pixel in data record
	    short bcr_count       :  number of pixels per spectral band
    output:
            struct rec_gome *rec:  structure for a spectral band record
            
.RETURNS     Nothing
.COMMENTS    static function
             Interpolate the polarisation fractions to the wavelength of 
             each detector pixel. Multiply the signal of each pixel with 
	     the polarization correction factor derived from this interpolated
	     p-value and calibration key data for the instrument's 
	     polarisation sensitivity.
-------------------------*/
static
void Apply_PolarisationCorrection( const float *grid,
				   const struct fcd_gome *fcd, 
				   const struct pcd_gome *pcd,
				   short bcr_start, short bcr_count, 
				   struct rec_gome *rec )
     /*@modifies rec@*/
{
     register short nc, np;
     register float correction, *pntr_eta, *pntr_pol, *pntr_data;

     short nr_valid_p = 0;
     float val_valid_p = 0.5f; 
     float eta_omega[CHANNEL_SIZE], p_band[CHANNEL_SIZE];
/*
 * initialize the fractional polarisation values to 0.5 (no polarisation)
 */
     nc = 0;
     do { p_band[nc] = 0.5f; } while( ++nc < CHANNEL_SIZE );
/*
 * count number of valid wavelengths and p-values (P7 and PMD's)
 */
     for ( np = 0; np < NUM_POLAR_COEFFS-1; np++ ) {
	  if ( pcd->polar.coeff[np] >= 0.f && pcd->polar.coeff[np] <= 1.f ) {
	       val_valid_p = pcd->polar.coeff[np];
	       nr_valid_p++;
	  }
     }
/*
 * we follow the DLR approach:
 * - no valid p's -> return p_band[] = 0.5
 * - one valid p (except the 7th mean p value of Band 1a)
 *                ->use this value for the whole band
 * - only the 7th mean p value of band 1a -> apply this value only to Band 1a
 * - >= 2 valid  p's -> use akima interpolation
 */
     if ( nr_valid_p == 0 ) {
	  if ( pcd->polar.coeff[PC_P7_BAND_1A] >= 0.f 
	       && pcd->polar.coeff[PC_P7_BAND_1A] <= 1.f ) {
	       nc = 0;
	       pntr_pol = p_band + bcr_start;
	       do { 
		    *pntr_pol++ = pcd->polar.coeff[PC_P7_BAND_1A]; 
	       } while ( ++nc < bcr_count );
	  }
     } else if ( nr_valid_p == 1 ) {
	  nc = 0;
	  pntr_pol = p_band + bcr_start;
	  do { *pntr_pol++ = val_valid_p; } while ( ++nc < bcr_count );
     } else {
	  register size_t akima_nr;

	  double x_arr[NUM_POLAR_COEFFS + AKIMA_EXTRA_POINTS];
	  double y_arr[NUM_POLAR_COEFFS + AKIMA_EXTRA_POINTS];
          double a_arr[NUM_POLAR_COEFFS + AKIMA_EXTRA_POINTS];
          double b_arr[NUM_POLAR_COEFFS + AKIMA_EXTRA_POINTS];
          double c_arr[NUM_POLAR_COEFFS + AKIMA_EXTRA_POINTS];
          double d_arr[NUM_POLAR_COEFFS + AKIMA_EXTRA_POINTS];

	  const float big_delta_w = 20.f;

	  akima_nr = 0;
	  if ( pcd->polar.coeff[PC_P7] >= 0.f 
	       && pcd->polar.coeff[PC_P7] <= 1.f ) {
	       x_arr[akima_nr+2] = pcd->polar.wv[PC_P7];
	       y_arr[akima_nr+2] = pcd->polar.coeff[PC_P7];
	       akima_nr++;
	  }
	  for ( np = PC_PMD_1; np <= PC_PMD_3; np++ ) {
	       if ( pcd->polar.coeff[np] >= 0.f 
		    && pcd->polar.coeff[np] <= 1.f ) {
		    x_arr[akima_nr+2] = pcd->polar.wv[np];
		    y_arr[akima_nr+2] = pcd->polar.coeff[np];
		    akima_nr++;
	       }
	  }
	  x_arr[1] = x_arr[2] - (2 * big_delta_w);
	  y_arr[1] = y_arr[2];
	  x_arr[0] = x_arr[1] - big_delta_w;
	  y_arr[0] = y_arr[1];
	  x_arr[akima_nr+2] = x_arr[akima_nr+1] + big_delta_w;
	  y_arr[akima_nr+2] = y_arr[akima_nr+1];
	  x_arr[akima_nr+3] = x_arr[akima_nr+2] + (2 * big_delta_w);
	  y_arr[akima_nr+3] = y_arr[akima_nr+2];
	  akima_nr += AKIMA_EXTRA_POINTS;

	  NADC_AKIMA_SU( FLT64_T, FLT64_T, akima_nr, x_arr, y_arr, 
			 a_arr, b_arr, c_arr, d_arr );

	  nc = bcr_start;
	  do {
	       p_band[nc] = NADC_AKIMA_PO( akima_nr, x_arr, a_arr, b_arr, 
                                           c_arr, d_arr, rec->wave[nc] );

	       if ( p_band[nc] <= -FLT_EPSILON )
		    p_band[nc] = 0.f;
	       else if ( p_band[nc] > 1.f )
		    p_band[nc] = 1.f;
	  } while ( ++nc < (bcr_start + bcr_count) );
     }

     FIT_GRID_AKIMA( FLT32_T, FLT32_T, CHANNEL_SIZE, 
		     grid, fcd->calib[rec->indx_psp].eta_omega,
		     FLT32_T, FLT32_T, (size_t) bcr_count, 
		     rec->wave+bcr_start, eta_omega+bcr_start );
/*
 * calculate and apply Polarisation correction factor
 */
     nc = 0;
     pntr_pol = p_band + bcr_start;
     pntr_eta = eta_omega + bcr_start;
     pntr_data = rec->data + bcr_start;
     do { 
	  correction = (1. + *pntr_eta)
	       / (2. * (*pntr_pol + *pntr_eta * (1. - *pntr_pol)));
#ifdef DEBUG_2
	  (void) printf( "%5u %12.5f %12.5f %12.5f %12.5f %12.8f", 
			 bcr_start + nc, 
			 rec->wave[bcr_start+nc], *pntr_data, 
			 *pntr_pol, *pntr_eta, correction );
#endif
	  *pntr_data++ *= (float) correction;
	  pntr_pol++; pntr_eta++;
#ifdef DEBUG_2
	  (void) printf( " %12.5f\n", *(pntr_data-1) );
#endif
     } while ( ++nc < bcr_count );
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_Intensity
.PURPOSE     Absolute radiance calibration
.INPUT/OUTPUT
  call as    Apply_Intensity( bcr_start, bcr_count, rec );
     input:
	    short bcr_start       :  first pixel in data record
	    short bcr_count       :  number of pixels per spectral band
    output:
            struct rec_gome *rec:  structure for a spectral band record
            
.RETURNS     Nothing
.COMMENTS    static function
             Earthshine and Moonshine signals are converted from BU/s to 
             Watt/(s cm**2 sr nm). Sunshine from BU sr / s to Watt/(s cm**2 nm)
-------------------------*/
static
void Apply_Intensity( const float *grid,
		      const struct fcd_gome *fcd, 
		      short bcr_start, short bcr_count, 
		      struct rec_gome *rec )
     /*@modifies rec@*/
{
     register short nc;
     register float *pntr_data, *pntr_fit;

     float response_fitted[CHANNEL_SIZE];

     const float EPSILON_INTERPOL = 0.6f;

     FIT_GRID_AKIMA( FLT32_T, FLT32_T, CHANNEL_SIZE, 
		     grid, fcd->calib[rec->indx_psp].response,
		     FLT32_T, FLT32_T, (size_t) bcr_count, 
		     rec->wave+bcr_start, response_fitted+bcr_start );

     nc = 0;
     pntr_fit = response_fitted + bcr_start;
     pntr_data = rec->data + bcr_start;
     do {
	  if ( *pntr_fit <= EPSILON_INTERPOL )
	       *pntr_data++ = 0.f;
	  else
	       *pntr_data++ /= *pntr_fit;
	  pntr_fit++;
#ifdef DEBUG
	  (void) printf( "%-4u %12.4f %15.8f %15.8f\n", bcr_start + nc, 
			 rec->wave[bcr_start+nc], 
			 *(pntr_fit-1), *(pntr_data-1) );
#endif
     } while ( ++nc < bcr_count );
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_UnitConversion
.PURPOSE     Convert units of Watt/... to photons/...
.INPUT/OUTPUT
  call as    Apply_UnitConversion( bcr_start, bcr_count, rec );
     input:
	    short bcr_start       :  first pixel in data record
	    short bcr_count       :  number of pixels per spectral band
    output:
            struct rec_gome *rec:  structure for a spectral band record
            
.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static inline
void Apply_UnitConversion( short bcr_start, short bcr_count, 
			   struct rec_gome *rec )
     /*@modifies rec@*/
{
     register short nc = 0;
     register float *pntr_data = rec->data + bcr_start;
     register float *pntr_wave = rec->wave + bcr_start;

     const float Watt2Photon = (float) 
	  (1e-9 / (1e+7 * (2.997925e+8 * 6.62618e-34)));

     do { 
	  *pntr_data++ *= (float) (*pntr_wave++ * Watt2Photon);
     } while ( ++nc < bcr_count ); 
}

/*+++++++++++++++++++++++++
.IDENTifer   Calc_BSDF
.PURPOSE     calculate BSDF
.INPUT/OUTPUT
  call as    Calc_BSDF(  );
     input:
    output:
            
.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static inline
float Calc_BSDF( const struct fcd_gome *fcd, float wavelength, 
		 double BSDF_Elevation, double BSDF_Azimuth )
{
     register double bsdf = 1.0;

     const double lamba = (wavelength - 500)/500.0;

     bsdf= (((((((fcd->coeffs[7] * lamba
		  + fcd->coeffs[6]) * lamba
		 + fcd->coeffs[5]) * lamba
		+ fcd->coeffs[4]) * lamba
	       + fcd->coeffs[3]) * lamba
	      + fcd->coeffs[2]) * lamba
	     + fcd->coeffs[1]) * lamba
	    + fcd->coeffs[0]);

     return (float)(fcd->bsdf_0 * (1 + (fcd->elevation * BSDF_Elevation))
		    * (1 - (fcd->azimuth * BSDF_Azimuth * BSDF_Azimuth))
		    * bsdf);
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_BSDF
.PURPOSE     convert signals from radiances to irradiances
.INPUT/OUTPUT
  call as    Apply_BSDF( bcr_start, bcr_count, rec );
     input:
	    short bcr_start       :  first pixel in data record
	    short bcr_count       :  number of pixels per spectral band
    output:
            struct rec_gome *rec:  structure for a spectral band record
            
.RETURNS     Nothing
.COMMENTS    static function
             Divide signals from Sun measurements by BSDF of the diffuser
             to convert the radiance measured by the spectrometer into 
	     irradiances.
-------------------------*/
static
void Apply_BSDF( const float *grid, const struct fcd_gome *fcd,
		 short bcr_start, short bcr_count, 
		 const struct smcd_gome *smcd, float *data )
     /*@modifies data@*/
{
     register short  nr;
     register float  bsdf;

     const double BSDF_Zenith = (double) smcd->north_sun_zen;
     const double BSDF_Azimuth = fabs( smcd->north_sun_azim - ((360. - 21.5)));
     const double BSDF_Elevation = BSDF_Zenith - 90.;

     for ( nr = bcr_start; nr < (bcr_start+bcr_count); nr++ ) {
	  bsdf = Calc_BSDF( fcd, grid[nr], BSDF_Elevation, BSDF_Azimuth );

	  if ( bsdf > 1e-10 )
	       data[nr] /= bsdf;
	  else
	       data[nr] = 0.f;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   Convert2Albedo
.PURPOSE     
.INPUT/OUTPUT
  call as    Convert2Albedo( bcr_start, bcr_count, rec );
     input:
	    short bcr_start       :  first pixel in data record
	    short bcr_count       :  number of pixels per spectral band
    output:
            struct rec_gome *rec:  structure for a spectral band record
            
.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static
void Convert2Albedo( short channel, const float *grid, 
		     const struct fcd_gome *fcd,
		     short nr_rec, short bcr_start, short bcr_count,
		     struct rec_gome *rec )
     /*@modifies rec@*/
{
     register short nc, nr;

     float cal_sun_ref[CHANNEL_SIZE], fitted_sun_ref[CHANNEL_SIZE];

     const short indx_offs = channel * CHANNEL_SIZE;

     for ( nc = 0; nc < CHANNEL_SIZE; nc++ ) {
	  if ( fcd->intensity[indx_offs+nc] > 1e-10 )
	       cal_sun_ref[nc] = fcd->sun_ref[indx_offs+nc] 
		    / fcd->intensity[indx_offs+nc];
	  else
	       cal_sun_ref[nc] = 0.f;
     }

     for ( nr = 0; nr < nr_rec; nr++ ) {
	  FIT_GRID_AKIMA( FLT32_T, FLT32_T, CHANNEL_SIZE, grid, cal_sun_ref,
			  FLT32_T, FLT32_T, (size_t) bcr_count, 
			  rec[nr].wave+bcr_start, fitted_sun_ref+bcr_start );

	  for ( nc = bcr_start; nc < (bcr_start+bcr_count); nc++ ) {
	       if ( fitted_sun_ref[nc] > 1e-10 )
		    rec[nr].data[nc] /= fitted_sun_ref[nc];
	       else
		    rec[nr].data[nc] = 0.f;
	  }
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_RadianceJumps
.PURPOSE     
.INPUT/OUTPUT
  call as    Apply_RadianceJumps( bcr_start, bcr_count, rec );
     input:
	    short bcr_start       :  first pixel in data record
	    short bcr_count       :  number of pixels per spectral band
    output:
            struct rec_gome *rec:  structure for a spectral band record
            
.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static
void Apply_RadianceJumps( void )
{
     (void) fprintf( stderr, 
		     "*** (Radiance Jumps) not implemented, Sorry\n" );
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_Degradation
.PURPOSE     
.INPUT/OUTPUT
  call as    Apply_Degradation( bcr_start, bcr_count, rec );
     input:
	    short bcr_start       :  first pixel in data record
	    short bcr_count       :  number of pixels per spectral band
    output:
            struct rec_gome *rec:  structure for a spectral band record
            
.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static
void Apply_Degradation( void )
{
     (void) fprintf( stderr, "*** (Degradation) not implemented, Sorry\n" );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void CALIB_PCD_BDR( unsigned short calib_flag, short nband,
		    const struct fcd_gome *fcd, 
		    short nr_pcd, const struct pcd_gome *pcd, 
		    short nr_rec, struct rec_gome *rec ) 
{
     register short nr;

     float wave_grid[CHANNEL_SIZE];

     const short bcr_channel = fcd->bcr[nband].array_nr - 1;
     const short bcr_start   = fcd->bcr[nband].start;
     const short bcr_count   = fcd->bcr[nband].end - fcd->bcr[nband].start + 1;
/*
 * check number of records
 */
     if ( nr_rec == 0 ) return;
/*
 * correct data given AverageMode and IntegrationStatus
 */
     for ( nr = 0; nr < nr_rec; nr++ ) {
	  short factor = 1;

	  if ( pcd[rec[nr].indx_pcd].ihr.averagemode ) {
	       switch ( nband ) {
	       case BAND_1a:
	       case BAND_1b:
		    break;	       
	       case BAND_2a:
	       case BAND_2b:
		    if ( pcd[rec[nr].indx_pcd].ihr.intg.field.fpa2 == 1 )
			 factor = 2;
		    break;	       
	       case BAND_3:
		    if ( pcd[rec[nr].indx_pcd].ihr.intg.field.fpa3 == 1 )
			 factor = 2;
		    else if ( pcd[rec[nr].indx_pcd].ihr.intg.field.fpa3 == 2 )
			 factor = 4;
		    break;	       
	       case BAND_4:
		    if ( pcd[rec[nr].indx_pcd].ihr.intg.field.fpa4 == 1 )
			 factor = 2;
		    break;	       
	       }
	  }
	  Apply_IntgFact( factor, bcr_start, bcr_count, rec+nr );
     }
     if ( calib_flag == CALIB_NONE ) return;
/*
 * calculate wavelength grid of FCD data
 */
     if ( (calib_flag & GOME_CAL_POLAR) != USHRT_ZERO
	  || (calib_flag & GOME_CAL_INTENS) != USHRT_ZERO ) {
	  for ( nr = 0; nr < CHANNEL_SIZE; nr++ )
	       wave_grid[nr] = (float) (
		    (((fcd->spec[fcd->indx_spec].coeffs[bcr_channel][4] * nr
		       + fcd->spec[fcd->indx_spec].coeffs[bcr_channel][3]) * nr
		      + fcd->spec[fcd->indx_spec].coeffs[bcr_channel][2]) * nr
		     + fcd->spec[fcd->indx_spec].coeffs[bcr_channel][1]) * nr
		    + fcd->spec[fcd->indx_spec].coeffs[bcr_channel][0]);
     }
/*
 * Apply leakage correction
 */
     if ( (calib_flag & GOME_CAL_LEAK) != USHRT_ZERO ) {
	  int indx_offs = bcr_channel * CHANNEL_SIZE;

	  for ( nr = 0; nr < nr_rec; nr++ ) {
	       register short nleak = 
		    pcd[rec[nr].indx_pcd].indx_leak;

	       Apply_DarkSignal( &fcd->leak[nleak].dark[indx_offs],
				 bcr_start, bcr_count, rec+nr );
	  }
     }
/*
 * Correct Band-1a, Blind-1a or Straylight-1a 
 *   for cross-talk from Peltier coolers
 */
     if ( nband == BAND_1a && (calib_flag & GOME_CAL_FPA) != USHRT_ZERO ) {
	  Apply_FPAnoise( fcd, nr_pcd, pcd, 
			  nr_rec, bcr_start, bcr_count, rec );
     }
/*
 * Correct for pixel-to-pixel fixed pattern variations
 */
     if ( (calib_flag & GOME_CAL_FIXED) != USHRT_ZERO ) {
	  int indx_offs = bcr_channel * CHANNEL_SIZE;

	  for ( nr = 0; nr < nr_rec; nr++ )
	       Apply_PPG( &fcd->pixel_gain[indx_offs], 
			  bcr_start, bcr_count, rec+nr );
     }
/*
 * Apply straylight correction
 */
     if ( (calib_flag & GOME_CAL_STRAY) != USHRT_ZERO ) {
	  for ( nr = 0; nr < nr_rec; nr++ )
	       Apply_StrayLightCorrection( bcr_channel, fcd, rec+nr );
     }
/*
 * Normalize signals to 1 second detector integration
 */
     if ( (calib_flag & GOME_CAL_NORM) != USHRT_ZERO ) {
	  register float integrateT;

	  for ( nr = 0; nr < nr_rec; nr++ ) {
	       if ( BandChannel(bcr_channel, 0) == nband )
		    integrateT = rec[nr].integration[0];
	       else
		    integrateT = rec[nr].integration[1];

	       Apply_IntegrationNormalization( bcr_start, bcr_count, 
					       integrateT, rec+nr );
	  }
     }
/*
 * Apply polarization correction
 */
     if ( (calib_flag & GOME_CAL_POLAR) != USHRT_ZERO ) {
	  for ( nr = 0; nr < nr_rec; nr++ )
	       Apply_PolarisationCorrection( wave_grid, fcd, 
					     pcd + rec[nr].indx_pcd,
					     bcr_start, bcr_count, 
					     rec+nr );
     }
/*
 * Apply absolute radiance calibration
 */
     if ( (calib_flag & GOME_CAL_INTENS) != USHRT_ZERO ) {
	  for ( nr = 0; nr < nr_rec; nr++ )
	       Apply_Intensity( wave_grid, fcd, 
				bcr_start, bcr_count, rec+nr );
	  if ( (calib_flag & GOME_CAL_ALBEDO) != USHRT_ZERO ) {
	       Convert2Albedo( bcr_channel, wave_grid, fcd, 
			       nr_rec, bcr_start, bcr_count, rec );
	  }
     }
/*
 * correct for radiance jumps
 */
     if ( (calib_flag & GOME_CAL_JUMPS) != USHRT_ZERO ) {
	  Apply_RadianceJumps();
     }
/*
 * correct for degradation
 */
     if ( (calib_flag & GOME_CAL_AGING) != USHRT_ZERO ) {
	  Apply_Degradation();
     }
/*
 * Apply unit conversion
 */
     if ( (calib_flag & GOME_CAL_UNIT) != USHRT_ZERO ) {
	  for ( nr = 0; nr < nr_rec; nr++ )
	       Apply_UnitConversion( bcr_start, bcr_count, rec+nr );
     }
}


/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void CALIB_SMCD_BDR( unsigned short calib_flag, short nband,
		     const struct fcd_gome *fcd, 
		     const struct smcd_gome *smcd, 
		     short nr_rec, struct rec_gome *rec ) 
{
     register short nr;

     float wave_grid[CHANNEL_SIZE];

     const short bcr_channel = fcd->bcr[nband].array_nr - 1;
     const short bcr_start   = fcd->bcr[nband].start;
     const short bcr_count   = fcd->bcr[nband].end - fcd->bcr[nband].start + 1;
/*
 * check number of records
 */
     if ( nr_rec == 0 ) return;
/*
 * correct data given AverageMode and IntegrationStatus
 */
     for ( nr = 0; nr < nr_rec; nr++ ) {
	  short factor = 1;

	  if ( smcd[rec[nr].indx_pcd].ihr.averagemode ) {
	       switch ( nband ) {
	       case BAND_1a:
	       case BAND_1b:
		    break;
	       case BAND_2a:
	       case BAND_2b:
		    if ( smcd[rec[nr].indx_pcd].ihr.intg.field.fpa2 == 1 )
			 factor = 2;
		    break;
	       case BAND_3:
		    if ( smcd[rec[nr].indx_pcd].ihr.intg.field.fpa3 == 1 )
			 factor = 2;
		    else if ( smcd[rec[nr].indx_pcd].ihr.intg.field.fpa3 == 2 )
			 factor = 4;
		    break;
	       case BAND_4:
		    if ( smcd[rec[nr].indx_pcd].ihr.intg.field.fpa4 == 1 )
			 factor = 2;
		    break;
	       }
	  }
	  Apply_IntgFact( factor, bcr_start, bcr_count, rec+nr );
     }
     if ( calib_flag == CALIB_NONE ) return;
/*
 * calculate wavelength grid of FCD data
 */
     if ( (calib_flag & GOME_CAL_BSDF) != USHRT_ZERO 
	  || (calib_flag & GOME_CAL_INTENS) != USHRT_ZERO ) {
	  for ( nr = 0; nr < CHANNEL_SIZE; nr++ )
	       wave_grid[nr] = (float) (
		    (((fcd->spec[fcd->indx_spec].coeffs[bcr_channel][4] * nr
		       + fcd->spec[fcd->indx_spec].coeffs[bcr_channel][3]) * nr
		      + fcd->spec[fcd->indx_spec].coeffs[bcr_channel][2]) * nr
		     + fcd->spec[fcd->indx_spec].coeffs[bcr_channel][1]) * nr
		    + fcd->spec[fcd->indx_spec].coeffs[bcr_channel][0]);
     }
/*
 * Apply leakage correction
 */
     if ( (calib_flag & GOME_CAL_LEAK) != USHRT_ZERO ) {
	  int indx_offs = bcr_channel * CHANNEL_SIZE;

	  for ( nr = 0; nr < nr_rec; nr++ ) {
	       register short nleak = 
		    smcd[rec[nr].indx_pcd].indx_leak;

	       Apply_DarkSignal( &fcd->leak[nleak].dark[indx_offs], 
				 bcr_start, bcr_count, rec+nr );
	  }
     }
/*
 * Correct for pixel-to-pixel fixed pattern variation
 */
     if ( (calib_flag & GOME_CAL_FIXED) != USHRT_ZERO ) {
	  int indx_offs = bcr_channel * CHANNEL_SIZE;

	  for ( nr = 0; nr < nr_rec; nr++ )
	       Apply_PPG( &fcd->pixel_gain[indx_offs],
			  bcr_start, bcr_count, rec+nr );
     }
/*
 * Apply straylight correction
 */
     if ( (calib_flag & GOME_CAL_STRAY) != USHRT_ZERO ) {
	  for ( nr = 0; nr < nr_rec; nr++ )
	       Apply_StrayLightCorrection( bcr_channel, fcd, rec+nr );
     }
/*
 * Normalize signals to 1 second detector integration
 */
     if ( (calib_flag & GOME_CAL_NORM) != USHRT_ZERO ) {
	  register float integrateT;

	  for ( nr = 0; nr < nr_rec; nr++ ) {
	       if ( BandChannel(bcr_channel, 0) == nband )
                    integrateT = rec[nr].integration[0];
               else
                    integrateT = rec[nr].integration[1];

	       Apply_IntegrationNormalization( bcr_start, bcr_count, 
					       integrateT, rec+nr );
	  }
     }
/*
 * Apply BSDF correction
 */
     if ( (calib_flag & GOME_CAL_BSDF) != USHRT_ZERO ) {
	  for ( nr = 0; nr < nr_rec; nr++ )
	       Apply_BSDF( wave_grid, fcd, bcr_start, bcr_count, 
			   smcd+nr, rec[nr].data );
     }
/*
 * Apply absolute radiance calibration
 */
     if ( (calib_flag & GOME_CAL_INTENS) != USHRT_ZERO ) {
	  for ( nr = 0; nr < nr_rec; nr++ )
	       Apply_Intensity( wave_grid, fcd, 
				bcr_start, bcr_count, rec+nr );
	  if ( (calib_flag & GOME_CAL_ALBEDO) != USHRT_ZERO ) {
	       Convert2Albedo( bcr_channel, wave_grid, fcd, 
			       nr_rec, bcr_start, bcr_count, rec );
	  }
     }
/*
 * correct for radiance jumps
 */
     if ( (calib_flag & GOME_CAL_JUMPS) != USHRT_ZERO ) {
	  Apply_RadianceJumps();
     }
/*
 * correct for degradation
 */
     if ( (calib_flag & GOME_CAL_AGING) != USHRT_ZERO ) {
	  Apply_Degradation();
     }
/*
 * Apply unit conversion
 */
     if ( (calib_flag & GOME_CAL_UNIT) != USHRT_ZERO ) {
	  for ( nr = 0; nr < nr_rec; nr++ )
	       Apply_UnitConversion( bcr_start, bcr_count, rec+nr );
     }
}
