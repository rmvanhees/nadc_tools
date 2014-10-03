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

.IDENTifer   calibAtbdWave
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy - Level 1b - Calibration
.LANGUAGE    ANSI C
.PURPOSE     perform Wavelength calibration on Sciamachy L1b science data
.INPUT/OUTPUT
  call as   SCIA_ATBD_INIT_WAVE( fileParam, orbit_phase, wvlen );
     input:  
             struct file_rec *fileParam : file/calibration parameters
	     float orbit_phase          : (average) orbit phase of science data
    output:
             struct wvlen_rec wvlen     : Solar and science wavelength grid

   call as  SCIA_ATBD_CAL_WAVE( wvlen, state, mds_1c )
     input:  
             struct wvlen_rec wvlen     : Solar and science wavelength grid 
	     struct state1_scia *state  : structure with States of the product
 in/output:  
             struct mds1c_scia *mds_1c  : level 1c MDS records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0   06-Jun-2006 initial release by R. M. van Hees
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
#include <float.h>
#include <math.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia_cal.h>

/*+++++ Macros +++++*/
        /* NONE */

/*+++++ Static Variables +++++*/
        /* NONE */

/*+++++ Global Variables +++++*/
        /* NONE */
/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static inline
double Eval_Poly( unsigned int xx, const double coeffs[] )
{
     return ((((coeffs[4] * xx + coeffs[3]) * xx + coeffs[2])
              * xx + coeffs[1]) * xx + coeffs[0]);
}

/*+++++++++++++++++++++++++ Main Program or Function(s) +++++++++++++++*/
void SCIA_ATBD_INIT_WAVE( const struct file_rec *fileParam,
			  float orbit_phase, struct wvlen_rec *wvlen )
{
     const char prognm[] = "InitAtbdWave";

     register unsigned int  n_ch, nd, np;

     unsigned int num_scp;

     float *wave = wvlen->science;
     float *wave_err = wvlen->error;

     struct srs_scia  *srs;
     struct base_scia base;
     struct scp_scia  *scp;

     const bool Save_Extern_Alloc = Use_Extern_Alloc;
/*
 * obtain Solar reference spectrum
 */
     Use_Extern_Alloc = FALSE;
     (void) SCIA_LV1_RD_SRS( fileParam->fp, fileParam->num_dsd, 
			     fileParam->dsd, &srs );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "SRS" );
     for ( n_ch = 0; n_ch < SCIENCE_CHANNELS; n_ch++ ) {
          unsigned short offs = n_ch * CHANNEL_SIZE;

	  (void) memcpy( wvlen->solar + offs,
			 srs[fileParam->level_2_smr[n_ch]].wvlen_sun + offs,
			 CHANNEL_SIZE * sizeof( float ));
     }
     free ( srs );
/*
 * read science wavelength spectrum (base)
 */
     (void) SCIA_LV1_RD_BASE( fileParam->fp, fileParam->num_dsd, 
			      fileParam->dsd, &base );
     if ( IS_ERR_STAT_FATAL )
          NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "BASE" );
     num_scp = SCIA_LV1_RD_SCP( fileParam->fp, fileParam->num_dsd, 
				fileParam->dsd, &scp );
     if ( IS_ERR_STAT_FATAL )
          NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "SCP" );
     Use_Extern_Alloc = Save_Extern_Alloc;
/*
 * copy base values to wavelength array
 */
     (void) memcpy( wvlen->science, base.wvlen_det_pix,
		    SCIENCE_PIXELS * sizeof( float ));
/*
 * correct wavelength grid, according to orbit phase
 */
     nd = num_scp - 1;
     while ( nd > 0 && orbit_phase <= scp[nd].orbit_phase ) nd--;

     for ( n_ch = 0; n_ch < SCIENCE_CHANNELS; n_ch++ ) {
          unsigned short coeff_offs = n_ch * NUM_SPEC_COEFFS;

	  if ( scp[nd].wv_error_calib[n_ch] >= FLT_EPSILON ) {
               for ( np = 0; np < CHANNEL_SIZE; np++ ) {
                    wave[np] += (float) 
			 Eval_Poly( np, &scp[nd].coeffs[coeff_offs] );
                    wave_err[np] = scp[nd].wv_error_calib[n_ch];
               }
          }
          wave += CHANNEL_SIZE;
          wave_err += CHANNEL_SIZE;
     }
     free( scp );
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++ */
void SCIA_ATBD_CAL_WAVE( const struct wvlen_rec wvlen, 
			 const struct state1_scia *state,
			 struct mds1c_scia *mds_1c )
{
     /* const char prognm[] = "SCIA_ATBD_CAL_WAVE"; */

     register unsigned short num = 0u;

     do {
	  register unsigned short npix     = 0u;
	  register unsigned short *pixelID = mds_1c->pixel_ids;

	  do {
	       mds_1c->pixel_wv[npix]     = wvlen.science[pixelID[npix]];
	       mds_1c->pixel_wv_err[npix] = wvlen.error[pixelID[npix]];
	  } while( ++npix < mds_1c->num_pixels );
     } while ( mds_1c++, ++num < state->num_clus );
}
