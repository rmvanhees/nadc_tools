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

.IDENTifer   calibSronMemLin
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy - Level 1c - Calibration
.LANGUAGE    ANSI C
.PURPOSE     perform 
.INPUT/OUTPUT
  call as   SCIA_SRON_CAL_MEM( num_mds, mds_1c );
	    SCIA_SRON_CAL_NLIN( num_mds, mds_1c );
     input:  
             unsigned short num_mds     : number of level 1c MDS records
 in/output:  
             struct mds1c_scia *mds_1c  : level 1c MDS records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    Based on SRON Sciamachy monitoring facility
.ENVIRONment None
.VERSION      1.3   21-Jan-2009 implemented reset for limb-scans
                                implemented reset at start of State, RvH
              1.2   16-Nov-2006 added SCIA_LV0_CAL_MEM & SCIA_LV0_CAL_NLIN, RvH
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
#include <limits.h>
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
/*+++++++++++++++++++++++++
.IDENTifer   Reset_MemCorrVal
.PURPOSE     Calculate memory correction after a detector reset
.INPUT/OUTPUT
  call as    Reset_MemCorrVal( Table, mds_1c, MemCorrVal );
     input:
	   float *Table              :  Table with memory corrections
	   struct mds1c_scia *mds_1c :  Structure with MDS 1c records
 in/output:
           float *MemCorrVal         :  Value of the memory correction

.RETURNS     nothings
.COMMENTS    none
-------------------------*/
static
void Reset_MemCorrVal( const float *Table, 
		       const struct mds1c_scia *mds_1c, 
		       float *MemCorrVal )
{
     const float setup_it[] = {
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
     const float scale_reset = 
	  setup_it[mds_1c->state_id-1] / (1000 * mds_1c->coaddf * mds_1c->pet);

     register unsigned short npix = 0;
     register unsigned short SignNorm;
     register float          *signal  = mds_1c->pixel_val;

     do {
	  SignNorm = __ROUNDf_us( scale_reset * signal[npix] );
	  MemCorrVal[npix] = Table[SignNorm];
     } while ( ++npix < mds_1c->num_pixels );
}

/*+++++++++++++++++++++++++
.IDENTifer   Reset_Limb_MemCorrVal
.PURPOSE     Calculate memory correction after a detector reset (Limb)
.INPUT/OUTPUT
  call as    Reset_Limb_MemCorrVal( Table, mds_1c, MemCorrVal );
     input:
	   float *Table              :  Table with memory corrections
	   struct mds1c_scia *mds_1c :  Structure with MDS 1c records
 in/output:
           float *MemCorrVal         :  Value of the memory correction

.RETURNS     nothings
.COMMENTS    none
-------------------------*/
static inline
void Reset_Limb_MemCorrVal( const float *Table, unsigned short nobs,
			    const struct mds1c_scia *mds_1c, 
			    float *MemCorrVal )
{
     register unsigned short npix = 0;
     register unsigned short SignNorm;
     register float val;

     const float *signal = 
	  mds_1c->pixel_val + (nobs * mds_1c->num_pixels);
     const float *dark   = 
	  mds_1c->pixel_val + ((mds_1c->num_obs-1) * mds_1c->num_pixels);

     do {
	  val = dark[npix];
	  if ( signal[npix] > dark[npix]  ) 
	       val += (3.f / 16) * (signal[npix] - dark[npix]);

	  SignNorm = __ROUNDf_us( val / mds_1c->coaddf );
	  MemCorrVal[npix] = Table[SignNorm];
     } while ( ++npix < mds_1c->num_pixels );
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_MemCorrSRON
.PURPOSE     apply memory correction to Science data (channel 1 - 5)
.INPUT/OUTPUT
  call as    Apply_MemCorrSRON( Table, mds_1c );
     input:
	   float *Table              : Table with memory corrections
 in/output:
           struct mds1c_scia *mds_1c : level 1c MDS records

.RETURNS     nothings
.COMMENTS    assumes that the signal is constant during integration time
-------------------------*/
static
void Apply_MemCorrSRON( const float *Table, struct mds1c_scia *mds_1c )
     /*@modifies mds_1c->pixel_val@*/
{
     const unsigned int coaddf = mds_1c->coaddf;

     register unsigned short nobs = 0;
     register unsigned short SignNorm;
     register float          *signal  = mds_1c->pixel_val;

     static float MemCorrVal[CHANNEL_SIZE];

     /* correct first readout using State setup time */
     Reset_MemCorrVal( Table, mds_1c, MemCorrVal );

     if ( mds_1c->geoL == NULL ) {
	  do {
	       register unsigned short npix = 0;

	       do {
		    /* normalize current readout value for coadding */
		    SignNorm = __ROUNDf_us( (*signal) / coaddf );

		    /* correct for previous readout */
		    *signal -= MemCorrVal[npix];

		    /* correct for coadding during this readout */
		    if ( coaddf > 1u )
			 *signal -= (coaddf - 1u) * Table[SignNorm];

		    /* store correction for next readout */
		    MemCorrVal[npix] = Table[SignNorm];
	       } while ( ++signal, ++npix < mds_1c->num_pixels ); 
	  } while( ++nobs < mds_1c->num_obs );  
     } else {
	  do {
	       register unsigned short npix = 0;

	       /* reset memory correction add start of new horizontal scan */
	       if ( (mds_1c->geoL[nobs].pixel_type & NEW_TANG_HGHT) != 0 )
		    Reset_Limb_MemCorrVal( Table, nobs, mds_1c, MemCorrVal );
	       do {
		    /* normalize current readout value for coadding */
		    SignNorm = __ROUNDf_us( (*signal) / coaddf );

		    /* correct for previous readout */
		    *signal -= MemCorrVal[npix];

		    /* correct for coadding during this readout */
		    if ( coaddf > 1u )
			 *signal -= (coaddf - 1u) * Table[SignNorm];

		    /* store correction for next readout */
		    MemCorrVal[npix] = Table[SignNorm];
	       } while ( ++signal, ++npix < mds_1c->num_pixels ); 
	  } while( ++nobs < mds_1c->num_obs );  
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_nLinCorrSRON
.PURPOSE     apply non-linearity correction to Science data (channel 6 - 8)
.INPUT/OUTPUT
  call as    Apply_nLinCorrSRON( mds_1c );
 in/output:
           struct mds1c_scia *mds_1c :  level 1c MDS records

.RETURNS     nothings
.COMMENTS    assumes that the signal is constant during integration time
-------------------------*/
static
void Apply_nLinCorrSRON( const struct scia_nlincorr *nlcorr, 
			 struct mds1c_scia *mds_1c )
     /*@modifies mds_1c->pixel_val@*/
{
     register unsigned short nobs = 0;
     register unsigned short curveIndex;
     register unsigned short signNorm;

     register unsigned short *pixelID = mds_1c->pixel_ids;
     register float          *signal  = mds_1c->pixel_val;

     const unsigned short coaddf = mds_1c->coaddf;
/*
 * apply non-Linearity correction on Epitaxx detector data
 */
     do {
	  register unsigned short npix = 0;

	  do {
	       curveIndex = (unsigned short) nlcorr->curve[pixelID[npix]];
	       signNorm = __ROUNDf_us( (*signal) / coaddf );
	       *signal++ -= coaddf * nlcorr->matrix[curveIndex][signNorm];
	  } while ( ++npix < mds_1c->num_pixels );
     } while( ++nobs < mds_1c->num_obs );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_SRON_CAL_MEM( unsigned short num_mds, struct mds1c_scia *mds_1c )
{
     const char prognm[] = "SCIA_LV0_CAL_MEM";

     register unsigned short num = 0u;     /* counter for number of clusters */

     struct scia_memcorr memcorr = {{0, 0}, NULL};
/*
 * initialize array with memory correction value
 */
     SCIA_RD_H5_MEM( &memcorr );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, "SCIA_RD_H5_MEM" );
/*
 * do actual memory correction
 */
     do {
	  if ( mds_1c->chan_id < FirstInfraChan )
	       Apply_MemCorrSRON( memcorr.matrix[mds_1c->chan_id-1], mds_1c );
     } while ( mds_1c++, ++num < num_mds );
     SCIA_FREE_H5_MEM( &memcorr );
}

/*--------------------------------------------------*/
void SCIA_SRON_CAL_NLIN( unsigned short num_mds, struct mds1c_scia *mds_1c )
{
     const char prognm[] = "SCIA_LV0_CAL_NLIN";

     register unsigned short num = 0u;     /* counter for number of clusters */
     
     struct scia_nlincorr nlcorr = {{0, 0}, NULL, NULL};
/*
 * read lookup table for non-linearity correction of Epitaxx detector data
 */
     SCIA_RD_H5_NLIN( NULL, &nlcorr );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_RD, "SCIA_RD_H5_NLIN" );
/*
 * do actual non-Linearity correction
 */
     do {
	  if ( mds_1c->chan_id >= FirstInfraChan )
	       Apply_nLinCorrSRON( &nlcorr, mds_1c );
     } while ( mds_1c++, ++num < num_mds );
     SCIA_FREE_H5_NLIN( &nlcorr );
}
