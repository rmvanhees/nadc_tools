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

.IDENTifer   SCIA_LV1_PATCH_BASE
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY
.LANGUAGE    ANSI C
.PURPOSE     copy/patch PDS annotation datasets between files

.INPUT/OUTPUT
  call as   SCIA_LV1_PATCH_BASE( patch_scia, num_dsd, dsd, fp_in, fp_out );
     input:
            unsigned short  patch_scia :  flag defining which patches to apply
            unsigned int num_dsd       :  number of DSDs
            struct dsd_envi *dsd       :  structure for the DSDs
	    FILE            *fp_in     :  file-pointer to input file
	    FILE            *fp_out    :  file-pointer to output file

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     24-Apr-2005   initial release by R. M. van Hees
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
#define _SCIA_PATCH_1
#include <nadc_scia_patch.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static inline
double Eval_Poly( unsigned int xx, const double coeffs[] )
{
     return ((((coeffs[4] * xx + coeffs[3]) * xx + coeffs[2]) 
	      * xx + coeffs[1]) * xx + coeffs[0]);
}

static
void Get_WaveGridSRON( float *wave )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies wave@*/
{
     const char prognm[] = "Get_WaveGridSRON";

     register unsigned int nr;

     const double coeffs_ch7[] = 
	  {1934.376, 0.117475, -1.04148e-5, 0., 0.};
     const double coeffs_ch8[] =
	  {2259.250, 0.135254, -1.19719e-5, 0., 0.};
/*
 * patch channel 7 and 8
 */
     for ( nr = 0; nr < CHANNEL_SIZE; nr++ )
	  *wave++ = (float) Eval_Poly( nr, coeffs_ch7 );
     for ( nr = 0; nr < CHANNEL_SIZE; nr++ )
	  *wave++ = (float) Eval_Poly( nr, coeffs_ch8 );

     NADC_ERROR( prognm, NADC_ERR_NONE, 
                 "\n\tapplied wavelength calibration from J. Schrijver "
                 "(SRON), 03-02-2000" );
}

static 
void __SCIA_LV1_PATCH_BASE( struct base_scia *base )
{
     float  wave[2 * CHANNEL_SIZE];

     const size_t nr_byte = 2 * CHANNEL_SIZE * ENVI_FLOAT;
     const size_t Offset_Ch7 = 6 * CHANNEL_SIZE;

     Get_WaveGridSRON( wave );

     (void) memcpy( &base->wvlen_det_pix[Offset_Ch7], wave, nr_byte  );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_PATCH_BASE( unsigned short patch_scia, unsigned int num_dsd, 
			  const struct dsd_envi *dsd, 
			  FILE *fp_in, FILE *fp_out )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp_in, fp_out@*/
{
     const char prognm[] = "SCIA_LV1_PATCH_BASE";

     struct base_scia base;
/*
 * read (G)ADS
 */
     (void) SCIA_LV1_RD_BASE( fp_in, num_dsd, dsd, &base );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "BASE" );
/*
 * patch spectral base of channel 7 and 8
 */
     if ( (patch_scia & SCIA_PATCH_BASE) != USHRT_ZERO )
	  __SCIA_LV1_PATCH_BASE( &base );
/*
 * write (G)ADS
 */
     (void) SCIA_LV1_WR_BASE( fp_out, &base );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "BASE" );
}
