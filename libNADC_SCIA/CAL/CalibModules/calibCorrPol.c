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

.IDENTifer   calibPolCorr
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy - Level 1b - Calibration
.LANGUAGE    ANSI C
.PURPOSE     perform Polarisation correction on Sciamachy L1b science data
.INPUT/OUTPUT
  call as   SCIA_ATBD_CAL_POL(fileParam, wvlen, state, mds_1b, mds_1c);
     input:  
             struct file_rec *fileParam : file/calibration parameters
	     struct wvlen_rec wvlen     : Solar and science wavelength grid
	     struct state1_scia *state  : structure with States of the product
             struct mds1_scia *mds_1b   : level 1b MDS records
 in/output:  
             struct mds1c_scia *mds_1c  : level 1c MDS records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   02-Sep-2013 fixed memory leak mtx_psplo, RvH
              2.0   05-Nov-2007 nearly complete rewrite, RvH
              1.1   24-Aug-2006 fixed bug the GPF correction, RvH
                                ToDo: handling invalid GPF values gracefully
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
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia_cal.h>

#include "shell.inc"

/*+++++ Macros +++++*/
#define NUM_GDF_STEPS     30
#define DIM_Q_U_ARRAY     (4 + NUM_FRAC_POLV + NUM_GDF_STEPS)

/*+++++ Static Variables +++++*/
static float alpha0_asm;
static float alpha0_esm;
static float lambda_end_gdf;
static char  do_pixelwise[SCIENCE_CHANNELS+1];
static char  do_pol_point[NUM_FRAC_POLV+1];

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   Get_PolarisationValues
.PURPOSE     
.INPUT/OUTPUT
  call as    Get_PolarisationValues(num_clcon, state, mds_1b, polV_out);
     input:
	    unsigned short num_clcon :  index of current cluster
	    struct state1_scia *state:  structure with States of the product
	    struct mds1_scia *mds_1b :  level 1b MDS records
    output:
	    struct polV_scia  **polV :  polarisation values for current cluster

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void Get_PolarisationValues(unsigned short num_clcon,
			    const struct state1_scia *state,
			    const struct mds1_scia *mds_1b,
			    /*@out@*/ struct polV_scia **polV_out)
      /*@globals  nadc_stat, nadc_err_stack;@*/
      /*@modifies nadc_stat, nadc_err_stack, *polV_out@*/
{
     register unsigned short nd, nr;
     register unsigned short indx_1b, indx_1c;

     unsigned short n_pol_all, n_pol_dsr;

     struct polV_scia *polV;
/*
 * initialize some variables
 *         n_pol_all: number of polV per level 1c record (= mds_1c->num_obs)
 *         n_pol_dsr: number of polV per level 1b record
 *         indx_1b:   index to polV in a level 1b record
 */
     nr = 0;
     indx_1b = 0;
     n_pol_all = 0;
     do {
	  n_pol_dsr = state->num_polar[nr] / state->num_dsr;
	  if (state->Clcon[num_clcon].intg_time == state->intg_times[nr]) {
	       n_pol_all = state->num_polar[nr];
	       break;
	  }
	  indx_1b += n_pol_dsr;
     } while (++nr < state->num_intg);
     if (n_pol_all == 0) {
	  polV_out[0] = NULL;
	  NADC_RETURN_ERROR(NADC_ERR_FATAL, "invalid integration time");
     }
/*
 * collect all fractional polarization data
 */
     polV = (struct polV_scia *) 
	  malloc(n_pol_all * sizeof(struct polV_scia));
     if (polV == NULL) {
	  polV_out[0] = NULL;     
	  NADC_RETURN_ERROR(NADC_ERR_ALLOC, "polV");
     }
     nd = 0;
     indx_1c = 0;
     do {
	  (void) memcpy(&polV[indx_1c],
			 &mds_1b[nd].polV[indx_1b],
			 n_pol_dsr * sizeof(struct polV_scia));
	  indx_1c += n_pol_dsr;
     } while(++nd < state->num_dsr);
     polV_out[0] = polV;
}

/*+++++++++++++++++++++++++
.IDENTifer   Get_GDF_Curve
.PURPOSE     calculate the Generalised Distribution Function (GDF)
.INPUT/OUTPUT
  call as   Get_GDF_Curve(polV, q_wv, q_val, u_wv, u_val);
     input:
            struct polV_scia *polval :  polarisation values for current cluster
    output:
            double *q_wv   :
            double *u_wv   :
            double *q_val  :
            double *u_val  :

.RETURNS     Number of GDF values
.COMMENTS    static function
-------------------------*/
static inline
unsigned short Get_GDF_Curve(const struct polV_scia *polval,
			     /*@out@*/ double *q_wv, /*@out@*/ double *q_val,
			     /*@out@*/ double *u_wv, /*@out@*/ double *u_val)
     /*@globals errno;@*/
{
/*
 * check GDF parameters
 */
     if ((int)(polval->gdf.beta - 0.5f) == -99 
	  || (int)(polval->gdf.p_bar - 0.5f) == -99 
	  || (int)(polval->gdf.w0 - 0.5f) == -99) {
	  *q_wv = *q_val = *u_wv = *u_val = -1.;
          return 0u;
     } else {
	  register unsigned short ng;

	  register double fw;
	  register double gdf_wv_diff = 0.;

	  double beta  = (double) polval->gdf.beta;
	  double p_bar = (double) polval->gdf.p_bar;
	  double w0    = (double) polval->gdf.w0;

	  const double wv_step = lambda_end_gdf / NUM_GDF_STEPS;
	  const double u_ratio = (polval->Q[PMD_THEORY] != 0.f) ? 
	       (-polval->U[PMD_THEORY] / polval->Q[PMD_THEORY]) : 0.0;
/*
 * calculate GDF curve
 */
	  for (ng = 0; ng < NUM_GDF_STEPS; ng++) {
	       gdf_wv_diff -= wv_step;

	       q_wv[ng] = u_wv[ng] = polval->rep_wv[PMD_THEORY] - gdf_wv_diff;

	       fw = exp(beta * gdf_wv_diff);
	       q_val[ng] = p_bar + w0 * fw / ((1. + fw) * (1. + fw));
	       u_val[ng] = u_ratio * q_val[ng];
	  }
	  return NUM_GDF_STEPS;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   Get_QU_Fit
.PURPOSE     Derive Q and U for requested wavelength grid 
.INPUT/OUTPUT
  call as   num = Get_QU_Fit(polV, num_pixels, pixel_wv, q_Fit, u_Fit);
     input:
            struct polV_scia *polval  : polarisation values for current cluster
	    unsigned short num_pixels : number of pixels
	    float pixel_wv            : wavelength grid
    output:
            double *q_Fit             : Q values for wavelength grid
            double *u_Fit             : U values for wavelength grid

.RETURNS     number of QU values (zero on error)
.COMMENTS    static function
-------------------------*/
static
unsigned short Get_QU_Fit(const struct polV_scia *polV, 
			  unsigned short num_pixels, const float *pixel_wv,
			  /*@out@*/ double *q_Fit, /*@out@*/ double *u_Fit)
      /*@globals  errno, nadc_stat, nadc_err_stack;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack, q_Fit, u_Fit@*/
{
     register unsigned short n_pol;

     register unsigned short q_num = 0;
     register unsigned short u_num = 0;

     double wv_min, wv_max;

     double q_val[DIM_Q_U_ARRAY], q_wv[DIM_Q_U_ARRAY];
     double u_val[DIM_Q_U_ARRAY], u_wv[DIM_Q_U_ARRAY];
/*
 * initialize return arrays
 */
     n_pol = 0;
     do {
	  q_Fit[n_pol] = u_Fit[n_pol] = 0.0;
     } while (++n_pol < num_pixels);
/*
 * collect applicable Q and U values
 */
     n_pol = 0;
     do {
	  if (do_pol_point[n_pol] == 'f') {
	       if (polV->error_Q[n_pol] >= 0.f) {
		    q_val[q_num] = -polV->Q[n_pol];
		    q_wv[q_num++] = polV->rep_wv[n_pol];
	       }
	       if (polV->error_U[n_pol] >= 0.f ) {
		    u_val[u_num] = polV->U[n_pol];
		    u_wv[u_num++] = polV->rep_wv[n_pol];
	       }
	  }
     } while (++n_pol < NUM_FRAC_POLV);
/*
 * apply analytic curves
 */
     if (polV->error_Q[0] >= 0.f && polV->error_U[0] >= 0.f) {
	  n_pol = Get_GDF_Curve(polV, q_wv+q_num, q_val+q_num, 
				u_wv+u_num, u_val+u_num);
	  q_num += n_pol;
	  u_num += n_pol;
     }
/*
 * skip remainder of the loop if we didn't find any Q or U values
 */
     if (q_num == 0 || u_num == 0) return 0;
/*
 * sort Q and U values
 */
     SHELLdd(q_num, q_wv, q_val);
     SHELLdd(u_num, u_wv, u_val);
/*
 * append 4 points for straight interpolation
 */
     wv_min = q_wv[0];
     wv_max = q_wv[q_num-1];
     (void) memmove(q_wv+2, q_wv, q_num * sizeof(double));
     (void) memmove(q_val+2, q_val, q_num * sizeof(double));

     q_num += 4;
     q_wv[0]       = wv_min - 40.;
     q_wv[1]       = wv_min - 20.;
     q_wv[q_num-2] = wv_max + 20.;
     q_wv[q_num-1] = wv_max + 40.;
     q_val[0] = (q_val[1] = q_val[2]);
     q_val[q_num-1] = (q_val[q_num-2] = q_val[q_num-3]);
     FIT_GRID_AKIMA(FLT64_T, FLT64_T, (size_t) q_num, q_wv, q_val, 
		     FLT32_T, FLT64_T, (size_t) num_pixels, pixel_wv, q_Fit);
     if (IS_ERR_STAT_FATAL)
	  NADC_GOTO_ERROR(NADC_ERR_FATAL, "FIT_GRID_AKIMA");

     wv_min = u_wv[0];
     wv_max = u_wv[u_num-1];
     (void) memmove(u_wv+2, u_wv, u_num * sizeof(double));
     (void) memmove(u_val+2, u_val, u_num * sizeof(double));

     u_num += 4;
     u_wv[0]       = wv_min - 40.;
     u_wv[1]       = wv_min - 20.;
     u_wv[u_num-2] = wv_max + 20.;
     u_wv[u_num-1] = wv_max + 40.;
     u_val[0] = (u_val[1] = u_val[2]);
     u_val[u_num-1] = (u_val[u_num-2] = u_val[u_num-3]);
     FIT_GRID_AKIMA(FLT64_T, FLT64_T, (size_t) u_num, u_wv, u_val, 
		     FLT32_T, FLT64_T, (size_t) num_pixels, pixel_wv, u_Fit);
     if (IS_ERR_STAT_FATAL)
	  NADC_GOTO_ERROR(NADC_ERR_FATAL, "FIT_GRID_AKIMA");
     return ((u_num > q_num) ? u_num : q_num);
done:
     return 0;
}

/*+++++++++++++++++++++++++ Static Functions (PSPN) +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   Get_PolSensNadir
.PURPOSE     Obtain Polarisation Sensitivity Parameters for viewing geometry 
             and wavelength grid of the science data for a whole state
.INPUT/OUTPUT
  call as    Get_PolSensNadir(fileParam, wvlen, &pspn);
     input:
            struct file_rec *fileParam: file/calibration parameters
	    struct wvlen_rec wvlen    : Solar/science wavelength grid
    output:
	    struct pspn_scia **pspn   : polarisation sensitivity parameters
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
unsigned short Get_PolSensNadir(const struct file_rec *fileParam, 
				 const struct wvlen_rec wvlen, 
				 struct pspn_scia **pspn_out)
      /*@globals  errno, nadc_stat, nadc_err_stack;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp, *pspn_out@*/
{
     register unsigned short n_ch, nr;

     unsigned short num_psp = 0;

     double rbuff[CHANNEL_SIZE];

     struct pspn_scia *pspn;

     pspn_out[0] = NULL;

     num_psp = (unsigned short)
	  SCIA_LV1_RD_PSPN(fileParam->fp, fileParam->num_dsd, 
			    fileParam->dsd, &pspn);
     if (IS_ERR_STAT_FATAL)
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "PSPN");
/*
 * interpolate sensitivities to wavelength grid (when do_pixelwise equals 'f')
 */
     for (n_ch = 0; n_ch < SCIENCE_CHANNELS; n_ch++) {
	  if (do_pixelwise[n_ch] == 'f') {
	       unsigned int offs = n_ch * CHANNEL_SIZE;

	       for (nr = 0; nr < num_psp; nr++) {
		    (void) memcpy(rbuff, &pspn[nr].mu2[offs],
				   CHANNEL_SIZE * sizeof(double));

		    FIT_GRID_AKIMA(FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.solar+offs, rbuff,
				    FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.science+offs, &pspn[nr].mu2[offs]);

		    (void) memcpy(rbuff, &pspn[nr].mu3[offs],
				   CHANNEL_SIZE * sizeof(double));

		    FIT_GRID_AKIMA(FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.solar+offs, rbuff,
				    FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.science+offs, &pspn[nr].mu3[offs]);
	       }
	  }
     }
/*
 * everything went fine, so return the data...
 */
     pspn_out[0] = pspn;
 done:
     return num_psp;
}

/*+++++++++++++++++++++++++
.IDENTifer   InterpolPSPN
.PURPOSE     calculate polarisation sensitivity for give pixel & esm-angle
.INPUT/OUTPUT
  call as    InterpolPSPN(id, ang_esm, pspn, &mu2, &mu3);
     input:
	    unsigned short   id       : pixel ID [0:8191]
	    float            ang_esm  : mirror elevation angle
	    struct pspn_scia *pspn    : polarisation sensitivity parameters
                                         pointer to pspn record with angl_esm
                                         smaller than parameter ang_esm
    output:
            double           *mu2     : mu2 nadir sensitivity [pixelID,ang_esm]
            double           *mu3     : mu3 nadir sensitivity [pixelID,ang_esm]
            
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
void InterpolPSPN(unsigned short id, float ang_esm,
		   const struct pspn_scia *pspn, 
		   /*@out@*/ double *mu2, /*@out@*/ double *mu3)
{
     double frac = (double) (ang_esm - pspn->ang_esm)
	  / (double) (pspn[1].ang_esm - pspn->ang_esm);

     *mu2 = (1 - frac) * pspn->mu2[id] + frac * pspn[1].mu2[id];
     *mu3 = (1 - frac) * pspn->mu3[id] + frac * pspn[1].mu3[id];
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_PolCorrNadir
.PURPOSE     apply polarisation sensitivity correction (Nadir)
.INPUT/OUTPUT
  call as    Apply_PolCorrNadir(num_psp, pspn, polV, mds_1c);
     input:
            unsigned short    num_psp : number of PSPN records
            struct pspn_scia  *pspn   : polarisation sensitivity parameters
	    struct polV_scia  *polV   : polarisation values for current cluster
 in/output:
            struct mds1c_scia *mds_1c : level 1c MDS record

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void Apply_PolCorrNadir(unsigned short num_psp, const struct pspn_scia  *pspn,
			 const struct polV_scia *polV, 
			 struct mds1c_scia *mds_1c)
      /*@globals  nadc_stat, nadc_err_stack;@*/
      /*@modifies nadc_stat, nadc_err_stack, *mds_1c@*/
{
     register unsigned short nobs = 0;

     register float *signal = mds_1c->pixel_val;
     register float angEsm, corrP;

     double mu2, mu3, q_Fit[CHANNEL_SIZE], u_Fit[CHANNEL_SIZE];

     do {
	  register unsigned short np = 0;
	  register unsigned short nr = 0;

	  (void) Get_QU_Fit(polV, mds_1c->num_pixels, mds_1c->pixel_wv,
			    q_Fit, u_Fit);
	  if (IS_ERR_STAT_FATAL)
	       NADC_RETURN_ERROR(NADC_ERR_FATAL, "Get_QU_Fit");

	  angEsm = alpha0_esm + 0.5 * mds_1c->geoN[nobs].pos_esm;

	  /* find pspn record with ang_esm just smaller than angEsm */
	  do {
	       if (angEsm >= pspn[nr].ang_esm) break;
	  } while (++nr < num_psp);
	  if (nr == num_psp) 
	       nr -= 2;
	  else if (nr > 0) 
	       nr--;

	  do {
	       register unsigned short id = mds_1c->pixel_ids[np];

	       InterpolPSPN(id, angEsm, pspn+nr, &mu2, &mu3);

	       corrP = (float) (1 + mu2 * q_Fit[np] + mu3 * u_Fit[np]);

	       *signal /= corrP;
	  } while (++signal, ++np < mds_1c->num_pixels);
     } while (++polV, ++nobs < mds_1c->num_obs);
}

/*+++++++++++++++++++++++++ Static Functions (PSPLO) +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   Get_PolSensLimb
.PURPOSE     Obtain Polarisation Sensitivity Parameters for viewing geometry 
             and wavelength grid of the science data for a whole state
.INPUT/OUTPUT
  call as    Get_PolSensLimb(fileParam, wvlen, &pspl);
     input:
            struct file_rec *fileParam: file/calibration parameters
	    struct wvlen_rec wvlen    : Solar/science wavelength grid
    output:
	    struct psplo_scia **pspl  : polarisation sensitivity parameters
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
unsigned short Get_PolSensLimb(const struct file_rec *fileParam, 
				const struct wvlen_rec wvlen, 
				struct psplo_scia **pspl_out)
      /*@globals  errno, nadc_stat, nadc_err_stack;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp, *pspl_out@*/
{
     register unsigned short n_ch, nr;

     unsigned short num_psp = 0;

     double rbuff[CHANNEL_SIZE];

     struct psplo_scia *pspl;

     pspl_out[0] = NULL;

     num_psp = (unsigned short)
	  SCIA_LV1_RD_PSPL(fileParam->fp, fileParam->num_dsd, 
			    fileParam->dsd, &pspl);
     if (IS_ERR_STAT_FATAL)
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "PSPL");
/*
 * interpolate sensitivities to wavelength grid (when do_pixelwise equals 'f')
 */
     for (n_ch = 0; n_ch < SCIENCE_CHANNELS; n_ch++) {
	  if (do_pixelwise[n_ch] == 'f') {
	       unsigned int offs = n_ch * CHANNEL_SIZE;

	       for (nr = 0; nr < num_psp; nr++) {
		    (void) memcpy(rbuff, &pspl[nr].mu2[offs],
				   CHANNEL_SIZE * sizeof(double));

		    FIT_GRID_AKIMA(FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.solar+offs, rbuff,
				    FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.science+offs, &pspl[nr].mu2[offs]);

		    (void) memcpy(rbuff, &pspl[nr].mu3[offs],
				   CHANNEL_SIZE * sizeof(double));

		    FIT_GRID_AKIMA(FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.solar+offs, rbuff,
				    FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.science+offs, &pspl[nr].mu3[offs]);
	       }
	  }
     }
/*
 * everything went fine, so return the data...
 */
     pspl_out[0] = pspl;
 done:
     return num_psp;
}

/*+++++++++++++++++++++++++
.IDENTifer   Get_PolSensOccul
.PURPOSE     Obtain Polarisation Sensitivity Parameters for viewing geometry 
             and wavelength grid of the science data for a whole state
.INPUT/OUTPUT
  call as    Get_PolSensOccul(fileParam, wvlen, &pspo);
     input:
            struct file_rec *fileParam: file/calibration parameters
	    struct wvlen_rec wvlen    : Solar/science wavelength grid
    output:
	    struct psplo_scia **pspo  : polarisation sensitivity parameters
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
unsigned short Get_PolSensOccul(const struct file_rec *fileParam, 
				 const struct wvlen_rec wvlen, 
				 struct psplo_scia **pspo_out)
      /*@globals  errno, nadc_stat, nadc_err_stack;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp, *pspo_out@*/
{
     register unsigned short n_ch, nr;

     unsigned short num_psp = 0;

     double rbuff[CHANNEL_SIZE];

     struct psplo_scia *pspo;

     pspo_out[0] = NULL;

     num_psp = (unsigned short)
	  SCIA_LV1_RD_PSPO(fileParam->fp, fileParam->num_dsd, 
			    fileParam->dsd, &pspo);
     if (IS_ERR_STAT_FATAL)
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "PSPO");
/*
 * interpolate sensitivities to wavelength grid (when do_pixelwise equals 'f')
 */
     for (n_ch = 0; n_ch < SCIENCE_CHANNELS; n_ch++) {
	  if (do_pixelwise[n_ch] == 'f') {
	       unsigned int offs = n_ch * CHANNEL_SIZE;

	       for (nr = 0; nr < num_psp; nr++) {
		    (void) memcpy(rbuff, &pspo[nr].mu2[offs],
				   CHANNEL_SIZE * sizeof(double));

		    FIT_GRID_AKIMA(FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.solar+offs, rbuff,
				    FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.science+offs, &pspo[nr].mu2[offs]);

		    (void) memcpy(rbuff, &pspo[nr].mu3[offs],
				   CHANNEL_SIZE * sizeof(double));

		    FIT_GRID_AKIMA(FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.solar+offs, rbuff,
				    FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.science+offs, &pspo[nr].mu3[offs]);
	       }
	  }
     }
/*
 * everything went fine, so return the data...
 */
     pspo_out[0] = pspo;
 done:
     return num_psp;
}

/*+++++++++++++++++++++++++
.IDENTifer   Get_Matrix_PSPLO
.PURPOSE     create a 2d-matrix to a psplo structure
.INPUT/OUTPUT
  call as   mtx = Get_Matrix_PSPLO(num_dsr, psplo, &n_asm, &n_esm);
     input:
            unsigned short num_dsr   : number of PSPLO records
            struct psplo_scia *psplo : PSPLO structures
    output:
            unsigned short *n_asm    : number of distinct azimuth angles
            unsigned short *n_esm    : number of distinct elevation angles

.RETURNS     pointer to struct psplo_scia
.COMMENTS    static function
-------------------------*/
static /*@null@*/ /*@out@*/ /*@only@*/ const
struct psplo_scia **Get_Matrix_PSPLO(unsigned short num_dsr,
                                      const struct psplo_scia *psplo,
                                      /*@out@*/ unsigned short *n_asm,
                                      /*@out@*/ unsigned short *n_esm)
     /*@globals  nadc_stat, nadc_err_stack;@*/
{
     register unsigned short nr, offs;

     const float tmpAzi = psplo[0].ang_asm;
     const float tmpElev = psplo[0].ang_esm;

     const struct psplo_scia **mtx_psplo;

     *n_asm = *n_esm = 0;
     for (nr = 0; nr < num_dsr; nr++) {
          if (psplo[nr].ang_asm == tmpAzi ) (*n_esm)++;
          if (psplo[nr].ang_esm == tmpElev) (*n_asm)++;
     }
     if ((*n_asm) == 0 || (*n_esm) == 0)
	  NADC_GOTO_ERROR(NADC_ERR_ALLOC, "mtx_psplo");
/*
 * allocate pointers to rows
 */
     mtx_psplo = (const struct psplo_scia **)
          malloc((*n_esm) * sizeof(const struct psplo_scia *));
     if (mtx_psplo == NULL)
          NADC_GOTO_ERROR(NADC_ERR_ALLOC, "mtx_psplo");
/*
 * set pointes to rows
 */
     nr = offs = 0u;
     do {
          mtx_psplo[nr] = psplo + offs;
          offs += (*n_asm);
     } while (++nr < (*n_esm));
     return mtx_psplo;
 done:
     return NULL;
}

/*+++++++++++++++++++++++++
.IDENTifer   InterpolPSPLO
.PURPOSE     calculate polarisation sensitivity for give pixel & esm/asm-angle
.INPUT/OUTPUT
  call as    InterpolPSPLO(id, ang_asm, ang_esm, psplo_el_mn, psplo_el_mx, 
                                      &mu2, &mu3);
     input:
	    unsigned short   id       :  pixel ID [0:8191]
	    float            ang_asm  :  mirror azimuth angle
	    float            ang_esm  :  mirror elevation angle
            struct psplo_scia *psplo_el_mn  :  PSPLO structure for elev_mn
            struct psplo_scia *psplo_el_mx  :  PSPLO structure for elev_mx
    output:
            double           *mu2     :  mu2 limb/occultation sensitivity
            double           *mu3     :  mu3 limb/occultation sensitivity
            
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
void InterpolPSPLO(unsigned short id, float ang_asm, float ang_esm,
		    const struct psplo_scia *psplo_el_mn, 
		    const struct psplo_scia *psplo_el_mx, 
		    /*@out@*/ double *mu2, /*@out@*/ double *mu3)
{
     double  frac, asm1_mu2, asm2_mu2, asm1_mu3, asm2_mu3;

     frac = (double) (ang_asm - psplo_el_mn->ang_asm) 
	  / (double) (psplo_el_mn[1].ang_asm - psplo_el_mn->ang_asm);
     asm1_mu2 = (1 - frac) * psplo_el_mn->mu2[id]
	  + frac * psplo_el_mn[1].mu2[id];
     asm1_mu3 = (1 - frac) * psplo_el_mn->mu3[id]
	  + frac * psplo_el_mn[1].mu3[id];

     frac = (double) (ang_asm - psplo_el_mx->ang_asm) 
	  / (double) (psplo_el_mx[1].ang_asm - psplo_el_mx->ang_asm);
     asm2_mu2 = (1 - frac) * psplo_el_mx->mu2[id]
	  + frac * psplo_el_mx[1].mu2[id];
     asm2_mu3 = (1 - frac) * psplo_el_mx->mu3[id]
	  + frac * psplo_el_mx[1].mu3[id];

     frac = (double) (ang_esm - psplo_el_mn->ang_esm)
          / (double) (psplo_el_mx->ang_esm - psplo_el_mn->ang_esm);
     *mu2 = (1 - frac) * asm1_mu2 + frac * asm2_mu2;
     *mu3 = (1 - frac) * asm1_mu3 + frac * asm2_mu3;
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_PolCorrLO
.PURPOSE     apply polarisation sensitivity correction (Limb/Occultation)
.INPUT/OUTPUT
  call as    Apply_PolCorrLO(num_psp, psplo, polV, mds_1c);
     input:
            unsigned short    num_psp : number of PSPLO records
            struct psplo_scia *psplo   : polarisation sensitivity parameters
 in/output:
            struct mds1c_scia *mds_1c : level 1c MDS record

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void Apply_PolCorrLO(unsigned short num_psp, const struct psplo_scia *psplo,
		      const struct polV_scia *polV, struct mds1c_scia *mds_1c)
{
     register unsigned short na, ne, nobs = 0;

     register float angAsm, angEsm, corrP;
     register float *signal = mds_1c->pixel_val;

     unsigned short n_asm, n_esm;

     double mu2, mu3, q_Fit[CHANNEL_SIZE], u_Fit[CHANNEL_SIZE];

     const struct psplo_scia
          **mtx_psplo = Get_Matrix_PSPLO(num_psp, psplo, &n_asm, &n_esm);

     do {
	  register unsigned short np = 0;

	  (void) Get_QU_Fit(polV, mds_1c->num_pixels, mds_1c->pixel_wv,
			    q_Fit, u_Fit);
	  if (IS_ERR_STAT_FATAL)
	       NADC_RETURN_ERROR(NADC_ERR_FATAL, "Get_QU_Fit");
	  
	  angAsm = alpha0_asm - 0.5 * mds_1c->geoL[nobs].pos_asm;
	  angEsm = alpha0_esm + 0.5 * mds_1c->geoL[nobs].pos_esm;

          /* find psplo record with ang_esm just smaller than angEsm */
          ne = 0;
          do {
               if (angEsm >= mtx_psplo[ne][0].ang_esm) break;
          } while (++ne < n_esm);
          if (ne == n_esm) 
	       ne -= 2;
          else if (ne > 0) 
	       ne--;

          /* find psplo record with ang_asm just smaller than angAsm */
          na = 0;
          do {
               if (angAsm >= mtx_psplo[ne][na].ang_asm) break;
          } while (++na < n_asm);
          if (na == n_asm) 
	       na -= 2;
          else if (na > 0) 
	       na--;

	  do {
	       register unsigned short id = mds_1c->pixel_ids[np];

	       InterpolPSPLO(id, angAsm, angEsm, mtx_psplo[ne]+na, 
			      mtx_psplo[ne+1]+na, &mu2, &mu3);

	       corrP = (float) (1 + mu2 * q_Fit[np] + mu3 * u_Fit[np]);

	       *signal /= (float) corrP;
	  } while (++signal, ++np < mds_1c->num_pixels);
     } while (++polV, ++nobs < mds_1c->num_obs);

     free(mtx_psplo);
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_ATBD_CAL_POL(const struct file_rec *fileParam,
			const struct wvlen_rec wvlen,
			const struct state1_scia *state, 
			const struct mds1_scia *mds_1b,
			struct mds1c_scia *mds_1c)
{
     register unsigned short num;

     unsigned short num_psp;

     struct polV_scia   *polV;

     struct pspn_scia   *pspn;
     struct psplo_scia  *pspl, *pspo;

     const bool Save_Extern_Alloc = Use_Extern_Alloc;

     if ((int) state->type_mds == SCIA_MONITOR) return;
/*
 * set some global variables
 */
     alpha0_asm = fileParam->alpha0_asm - 360.f;
     alpha0_esm = fileParam->alpha0_esm - 360.f;
     lambda_end_gdf = fileParam->lambda_end_gdf;
     (void) memcpy(do_pixelwise, fileParam->do_pixelwise, SCIENCE_CHANNELS+1);
     (void) memcpy(do_pol_point, fileParam->do_pol_point, NUM_FRAC_POLV+1);
/*
 * read/interpolate polarisation sensitivity parameters
 */
     Use_Extern_Alloc = FALSE;
     switch ((int) state->type_mds) {
     case SCIA_NADIR:
	  num_psp = Get_PolSensNadir(fileParam, wvlen, &pspn);
	  if (IS_ERR_STAT_FATAL)
	    NADC_GOTO_ERROR(NADC_ERR_FATAL, "Get_PolSensNadir");

	  num = 0;
	  do {
	       Get_PolarisationValues(num, state, mds_1b, &polV);
	       if (IS_ERR_STAT_FATAL)
		    NADC_GOTO_ERROR(NADC_ERR_FATAL, "Get_PolVal");
	       Apply_PolCorrNadir(num_psp, pspn, polV, mds_1c);
	       free(polV);
	       if (IS_ERR_STAT_FATAL)
		    NADC_GOTO_ERROR(NADC_ERR_FATAL, "Apply_PolCorr");
	  } while (++mds_1c, ++num < state->num_clus);
 
	  if (pspn != NULL) free(pspn);
	  break;
     case SCIA_LIMB:
	  num_psp = Get_PolSensLimb(fileParam, wvlen, &pspl);
	  if (IS_ERR_STAT_FATAL)
	    NADC_GOTO_ERROR(NADC_ERR_FATAL, "Get_PolSensLimb");

	  num = 0;
	  do {
	       Get_PolarisationValues(num, state, mds_1b, &polV);
	       if (IS_ERR_STAT_FATAL)
		    NADC_GOTO_ERROR(NADC_ERR_FATAL, "Get_PolVal");
	       Apply_PolCorrLO(num_psp, pspl, polV, mds_1c);
	       free(polV);
	       if (IS_ERR_STAT_FATAL)
		    NADC_GOTO_ERROR(NADC_ERR_FATAL, "Apply_PolCorr");
	  } while (++mds_1c, ++num < state->num_clus);

	  if (pspl != NULL) free(pspl);
	  break;
     case SCIA_OCCULT:
	  num_psp = Get_PolSensOccul(fileParam, wvlen, &pspo);
	  if (IS_ERR_STAT_FATAL)
	    NADC_GOTO_ERROR(NADC_ERR_FATAL, "Get_PolSensOccul");

	  num = 0;
	  do {
	       Get_PolarisationValues(num, state, mds_1b, &polV);
	       if (IS_ERR_STAT_FATAL)
		    NADC_GOTO_ERROR(NADC_ERR_FATAL, "Get_PolVal");
	       Apply_PolCorrLO(num_psp, pspo, polV, mds_1c);
	       free(polV);
	       if (IS_ERR_STAT_FATAL)
		    NADC_GOTO_ERROR(NADC_ERR_FATAL, "Apply_PolCorr");
	  } while (++mds_1c, ++num < state->num_clus);

	  if (pspo != NULL) free(pspo);
	  break;
     }
 done:
     Use_Extern_Alloc = Save_Extern_Alloc;
}
