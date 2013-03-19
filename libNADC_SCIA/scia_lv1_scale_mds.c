/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2007 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GET_SCIA_MDS1_DATA
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA Level 1b/1c
.LANGUAGE    ANSI C
.PURPOSE     
.INPUT/OUTPUT
  call as   nobs = SCIA_LV1_SCALE_MDS( PmdScaling, chanID, state, 
                                       mds_1b, pmd_1c, mds_1c, &sign_out );
     input:  
             bool PmdScaling           : boolean flag to indicate PMD scaling 
	                                 (requires mds_1b or pmd_1c)
             unsigned char chanID      : channel ID
	     struct state1_scia *state : structure with States of the product
	     struct mds1_scia *mds_1b  : Level 1b MDS records (or NULL)
	     struct mds1c_pmd *pmd_1c  : Level 1c PMD_MDS records (or NULL)
	     struct mds1c_scia *mds_1c : Level 1c MDS records
    output:  
	     float **sign_out          : Science data of one state

.RETURNS     number of observations (unsigned short)
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     01-Oct-2007   initial release by R. M. van Hees
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

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
/* 
 * Science Channel 2 PMD mapping: 1-A, 2-A, 3-B, 4-C, 5-D, 6-E, 7-F, 8-F
 */
static const unsigned short Chan2PmdIndx[SCIENCE_CHANNELS+1] = { 
     0, 0, 0, 1, 2, 3, 4, 5, 5 
};

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   RESAMPLE_1B_PmdVal
.PURPOSE     re-sample PMD readouts to pixel integration time
.INPUT/OUTPUT
  call as    RESAMPLE_1B_PmdVal( numDsr, mds_1b, IndxPmd, numPmd, pixelPmd );
     input:
            unsigned int numDsr      :  number of dataset records
            struct mds1_scia *mds_1b :  level 1b MDS records
	    unsigned short indxPmd   :  Index to PMD [0...5]
	    unsigned short numPmd    :  requested number of PMD values
    output:
            float *pixelPmd          :  re-sampled PMD values

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
static inline
void RESAMPLE_1B_PmdVal( unsigned short numDsr, 
			 const struct mds1_scia *mds_1b,
			 unsigned short indxPmd, 
			 unsigned short numPmd, 
			 /*@out@*/ float *pixelPmd )
       /*@modifies pixelPmd@*/
{
/*      const char prognm[] = "RESAMPLE_1B_PmdVal"; */

     register unsigned short n_dsr, n_pet, n_pmd, n_pmd_out;

/* Number of Pixel Exposure Times per Dataset record */
     const unsigned short numPetDsr = (unsigned short) (numPmd / numDsr);

/* Number of PMD readouts per Pixel Exposure Time */
     const unsigned short numPmdPet = 
	  (unsigned short) (mds_1b->n_pmd / PMD_NUMBER / numPetDsr);
/*
 * initialize array pixelPmd to zero
 */
     (void) memset( pixelPmd, 0, numPmd * sizeof( float ));
/*
 * average the PMD readouts to PET of detector readouts
 */
     n_pmd_out = 0u;
     n_dsr = 0u;              /* loop over all Data Set Records */
     do {
	  const float *int_pmd = &mds_1b[n_dsr].int_pmd[indxPmd];

	  n_pet = 0u;         /* loop over all Pixel Exposure Times */
	  do {
	       n_pmd = 0u;    /* loop over PMD readouts per PET */
	       do {
		    if ( *int_pmd > FLT_EPSILON ) 
			 pixelPmd[n_pmd_out] += *int_pmd;

		    int_pmd += PMD_NUMBER;
	       } while ( ++n_pmd < numPmdPet );
	  } while ( ++n_pmd_out, ++n_pet < numPetDsr );
     } while ( ++n_dsr < numDsr );
}

/*+++++++++++++++++++++++++
.IDENTifer   RESAMPLE_1C_PmdVal
.PURPOSE     re-sample PMD readouts to pixel integration time
.INPUT/OUTPUT
  call as    RESAMPLE_1C_PmdVal( pmd_1c, IndxPmd, numPmd, pixelPmd );
     input:
            struct mds1c_pmd *pmd_1c :  Level 1c PMD_MDS records
	    unsigned short indxPmd   :  Index to PMD [0...5]
	    unsigned short numPmd    :  requested number of PMD values
    output:
            float *pixelPmd          :  re-sampled PMD values

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
static inline
void RESAMPLE_1C_PmdVal( const struct mds1c_pmd *pmd_1c,
			 unsigned short indxPmd, 
			 unsigned short numPmd, 
			 /*@out@*/ float *pixelPmd )
       /*@modifies pixelPmd@*/
{
/*      const char prognm[] = "RESAMPLE_1C_PmdVal"; */

     register unsigned short n_pmd_out = 0;

     const float *int_pmd = &pmd_1c->int_pmd[indxPmd];

/* Number of PMD readouts per detector readout */
     const unsigned short sampling = numPmd / (pmd_1c->num_pmd / PMD_NUMBER);
/*
 * initialize array pixelPmd to zero
 */
     (void) memset( pixelPmd, 0, numPmd * sizeof( float ));
/*
 * average the PMD readouts to PET of detector readouts
 */
     do {
	  register unsigned short ns = 0;

	  do {
	       if ( *int_pmd > FLT_EPSILON ) pixelPmd[n_pmd_out] += *int_pmd;

	       int_pmd += PMD_NUMBER;
	  } while ( ++ns < sampling );
     } while ( ++n_pmd_out < numPmd );
}

/*+++++++++++++++++++++++++
.IDENTifer   get_weight_Factors
.PURPOSE     calculate weight factors
.INPUT/OUTPUT
  call as    get_weight_Factors( sampling, numVal, Val, wghtFactors );
     input:
	    unsigned short sampling   :  number of values in sample
            unsigned short numVal     :  number of Values
	    float *Val                :  values
    output:
            float *wghtFactor         :  weight Factors

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
static inline
void get_weight_Factors( unsigned short sampling, unsigned short numVal,
			 const float *Val, /*@out@*/ float *wghtFactor )
       /*@modifies wghtFactor@*/
{
/*      const char prognm[] = "get_weight_Factors"; */

     register unsigned short n_val = 0;

     const float Val_CutOff = sampling * 10.f;

     do {
	  register unsigned short ii;
	  register float sumVal = 0.f;

	  for ( ii = 0; ii < sampling; ii++ )
	       sumVal += Val[n_val + ii];

	  if ( sumVal >= Val_CutOff ) {
	       ii = 0;
	       do {
		    wghtFactor[n_val] = Val[n_val] / sumVal;
		    n_val++;
	       } while( ++ii < sampling );
	  } else {
	       ii = 0;
	       do {
		    wghtFactor[n_val++] = 1.f / sampling;
	       } while( ++ii < sampling );
	  }
     } while( n_val < numVal );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned short SCIA_LV1_SCALE_MDS( bool PmdScaling, unsigned char chanID, 
				   const struct state1_scia *state,
				   const struct mds1_scia *mds_1b,
				   const struct mds1c_pmd *pmd_1c,
				   const struct mds1c_scia *mds_1c, 
				   float **sign_out )
{
     const char prognm[] = "SCIA_LV1_SCALE_MDS";

     register unsigned short nm, ns;

     unsigned short dim_Y = 0;

     unsigned short sampling;
     float *pixelPmd = NULL;
     float *wghtFactor = NULL;

     const unsigned short dim_X = CHANNEL_SIZE;

     for ( nm = 0; nm < state->num_clus; nm++ ) {
	  if ( mds_1c[nm].chan_id == chanID && dim_Y < mds_1c[nm].num_obs ) 
	    dim_Y = mds_1c[nm].num_obs;
     }
     if ( dim_Y == 0 ) return dim_Y;
/*
 * allocate memory to store the data at shortest integration time
 */
     if ( ! Use_Extern_Alloc )
       sign_out[0] = (float *) malloc((size_t) dim_X * dim_Y * sizeof(float) );
     if ( sign_out[0] == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "sign_out" );
/*
 * check for Monitor MDS, these have no PMD measurements attached
 */
     if ( (int) state->type_mds == SCIA_MONITOR ) PmdScaling = FALSE;
     if ( mds_1b == NULL && pmd_1c == NULL ) PmdScaling = FALSE;
/*
 *** the whole procedure without PMD scaling is straigth forward ***
 */
     if ( ! PmdScaling ) {
	  for ( nm = 0; nm < state->num_clus; nm++ ) {
	       register float *signal = sign_out[0];

	       if ( mds_1c[nm].chan_id == chanID ) {
		    register unsigned short ny      = 0;
		    register unsigned short nobs    = 0;
		    register float          *value  = mds_1c[nm].pixel_val;

		    const unsigned short    pixelID = 
			 mds_1c[nm].pixel_ids[0] % CHANNEL_SIZE;
	       
		    sampling = dim_Y / mds_1c[nm].num_obs;
		    if ( (dim_Y % mds_1c[nm].num_obs) != 0 )
			 (void) fprintf( stderr, "Fatal error: sampling..." );

		    do {
			 for ( ns = 0; ns < sampling; ns++ ) {
			      register unsigned short nx = 0;

			      do {
				   signal[pixelID+nx] = value[nx] / sampling;
			      } while ( ++nx < mds_1c[nm].num_pixels );
			      signal += dim_X;
			      ny++;
			 }
			 value += mds_1c[nm].num_pixels;
		    } while ( ++nobs < mds_1c[nm].num_obs );
	       }
	  }
	  return dim_Y;
     }
/*
 *** caller requested PMD scaling ***
 *
 * allocate memory for re-sampled PMD readouts
 */
     if ( (pixelPmd = (float *) malloc( dim_Y * sizeof(float) )) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "pixelPmd" );
     if ( (wghtFactor = (float *) malloc( dim_Y * sizeof(float) )) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "wghtFactor" );
/*
 * re-sample PMD readouts to pixel integration time
 */
     if ( mds_1b != NULL )
	  RESAMPLE_1B_PmdVal( state->num_dsr, mds_1b, Chan2PmdIndx[chanID],
			      dim_Y, pixelPmd );
     else
	  RESAMPLE_1C_PmdVal( pmd_1c, Chan2PmdIndx[chanID], dim_Y, pixelPmd ); 
/*
 * fill array with data
 */
     for ( nm = 0; nm < state->num_clus; nm++ ) {
	  register float *signal = sign_out[0];

	  if ( mds_1c[nm].chan_id == chanID ) {
	       register unsigned short ny      = 0;
	       register unsigned short nobs    = 0;
	       register float          *value  = mds_1c[nm].pixel_val;

	       const unsigned short    pixelID = 
		    mds_1c[nm].pixel_ids[0] % CHANNEL_SIZE;
	       
	       sampling = dim_Y / mds_1c[nm].num_obs;
	       if ( (dim_Y % mds_1c[nm].num_obs) != 0 )
		    (void) fprintf( stderr, "Fatal error: sampling..." );

	       get_weight_Factors( sampling, dim_Y, pixelPmd, wghtFactor );
	       do {
		    for ( ns = 0; ns < sampling; ns++ ) {
			 register unsigned short nx = 0;

			 do {
			      signal[pixelID+nx] = value[nx] * wghtFactor[ny];
			 } while ( ++nx < mds_1c[nm].num_pixels );
			 signal += dim_X;
			 ny++;
		    }
		    value += mds_1c[nm].num_pixels;
	       } while ( ++nobs < mds_1c[nm].num_obs );
	  }
     }
 done:
     if ( pixelPmd != NULL ) free( pixelPmd );
     if ( wghtFactor != NULL ) free( wghtFactor );

     return dim_Y;
}
