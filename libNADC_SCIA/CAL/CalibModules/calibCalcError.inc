/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2002 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   calibErrorCalc
.PURPOSE     calculate errors of the SCIA calibration
.DEFINES     4 static functions:
              - calcShotNoise, calcDarkCorrError, calcDarkErrorLimb,
                calcSpectralError, calcSpectralAccuracy
.COMMENTS    ANSI-C inline function, declare in your code as:
             #include <calibErrorCalc.inc>
.VERSION      1.2   21-Aug-2009 calcDarkErrorLimb and update of documention, RvH
              1.2   01-Sep-2004 bugfix: range check 
                                - errors must be positive, RvH
              1.1   09-Jun-2004 added Calc_SpectralRelError, RvH
              1.0   12-May-2004 start logging modifications, RvH
------------------------------------------------------------*/
#ifndef  __CALC_ERROR                            /* Avoid redefinitions */
#define  __CALC_ERROR

#include <errno.h>

#ifdef __NEED_DARK_ERROR__
/*+++++++++++++++++++++++++
.IDENTifer   calcShotNoise
.PURPOSE     calculate precision of SCIA data, which is shot noise limited
.INPUT/OUTPUT
  call as    calcShotNoise( electron_bu, DarkData, mds1c );
     input:
	   float          electron_bu  :number of photon-electrons per BU
	   struct DarkRec *DarkData    :  record holding Dark Calibration data
 in/output:
	   struct mds1c_scia *mds_1c   :  level 1c MDS records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
void calcShotNoise( float electron_bu, const struct DarkRec *DarkData, 
		    struct mds1c_scia *mds_1c )
     /*@modifies errno, mds_1c->pixel_err@*/
{
     register unsigned short nobs = 0;

     register float *e_signal = mds_1c->pixel_err;

     register const float *signal = mds_1c->pixel_val;
     register const float *analog = 
	  DarkData->AnalogOffs + mds_1c->pixel_ids[0];
     register const float *noise  = 
	  DarkData->MeanNoise + mds_1c->pixel_ids[0];

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
.IDENTifer   calcDarkError
.PURPOSE     calculate error on the measured signal due to the substraction
             of the Dark Current correction
.INPUT/OUTPUT
  call as    calcDarkError( intg, DarkData, mds_1c );

     input:
	   float intg                :  integration time
	   struct DarkRec *DarkData  :  record holding Dark Calibration data
 in/output:
	   struct mds1c_scia *mds_1c :  level 1c MDS records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
void calcDarkError( float intg, const struct DarkRec *DarkData, 
		    struct mds1c_scia *mds_1c )
     /*@globals  errno;@*/
     /*@modifies errno, mds_1c->pixel_err@*/
{
     register unsigned short nobs = 0;

     register float *e_signal = mds_1c->pixel_err;
     register double derror;

/* constants */
     register const float *analog_pntr = 
	  DarkData->AnalogOffsError + mds_1c->pixel_ids[0];
     register const float *dark_pntr   = 
	  DarkData->DarkCurrentError + mds_1c->pixel_ids[0];

     do {
	  register unsigned short npix = 0;
	  do {
	       derror = sqrt( (double) mds_1c->coaddf ) * analog_pntr[npix]
		    + intg * dark_pntr[npix];
	       *e_signal += (float) (derror * derror);
	       e_signal++;
	  } while( ++npix < mds_1c->num_pixels );
     } while ( ++nobs < mds_1c->num_obs );
}

/*+++++++++++++++++++++++++
.IDENTifer   calcDarkErrorLimb
.PURPOSE     calculate error on the measured signal due to the substraction
             of the Dark Current correction (Limb, only)
.INPUT/OUTPUT
  call as    calcDarkErrorLimb( mds_1c );

 in/output:
	   struct mds1c_scia *mds_1c :  level 1c MDS records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
void calcDarkErrorLimb( struct mds1c_scia *mds_1c )
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
#endif   /*  __NEED_DARK_ERROR__ */

#ifdef __NEED_PPG_ERROR___
#endif   /* __NEED_PPG_ERROR___ */

#ifdef __NEED_STRAY_ERROR__
#endif   /* __NEED_STRAY_ERROR__ */

#ifdef __NEED_GRID_ACCURACY__
/*+++++++++++++++++++++++++
.IDENTifer   calcSpecAccuracy
.PURPOSE     calculate relative presision error on the spectra, 
             Not including polarisation and Mueller matrix errors
.INPUT/OUTPUT
  call as    calcSpecAccuracy( state, mds_1c );
     input:
           struct state1_scia *state :  structure with States of the product
 in/output:
	   struct mds1c_scia *mds_1c :  level 1c MDS records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
void calcSpectralAccuracy( const struct state1_scia state[],
			   struct mds1c_scia *mds_1c )
     /*@modifies mds_1c->pixel_err@*/
{
     register unsigned short num = 0u;     /* counter for number of clusters */

     do {
	  register unsigned int ns = 0;
	  register float  *signal   = mds_1c->pixel_val;
	  register float  *e_signal = mds_1c->pixel_err;

	  const unsigned int numSignal = mds_1c->num_pixels * mds_1c->num_obs;

	  do {
	       *e_signal += 0.25f;
	       if ( *signal > 0.1f ) *e_signal /= ((*signal) * (*signal));
	       *e_signal = sqrtf( *e_signal );
	  } while ( e_signal++, signal++, ++ns < numSignal );
     } while ( mds_1c++, ++num < state->num_clus );
}
#endif   /* __NEED_GRID_ACCURACY__ */
#endif   /* __CALC_ERROR */
