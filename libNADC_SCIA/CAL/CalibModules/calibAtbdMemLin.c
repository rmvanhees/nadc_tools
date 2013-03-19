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

.IDENTifer   calibAtbdMemLin
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy - Level 1b - Calibration
.LANGUAGE    ANSI C
.PURPOSE     perform Memory/non-Linearity correction on SCIA L1b science data
.INPUT/OUTPUT
  call as   SCIA_ATBD_CAL_MEM( scalef, state, mds_1b, mds_1c );
            SCIA_ATBD_CAL_NLIN( scalef, state, mds_1b, mds_1c );
     input:  
             struct scale_rec *scalef   : scale factors to decompress 
                                          correction factors
	     struct state1_scia *state  : structure with States of the product
             struct mds1_scia *mds_1b   : level 1b MDS records
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
#include <string.h>
#include <limits.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#define _MEMNLIN_CORR
#include <nadc_scia_cal.h>

/*+++++ Macros +++++*/
        /* NONE */

/*+++++ Static Variables +++++*/
        /* NONE */

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static inline
float MemNlin2Flt( unsigned char scaleFlag, unsigned char chanID, 
		   signed char cval )
{
     float rval = 0.f;

     if ( scaleFlag != SCIA_MEM_SCALE_OLD ) {
	  switch ( (int) chanID ) {
	  case 1: case 2: case 3: case 4: case 5:
	       rval = (1.25f * ((float) cval + 37.f));
	       break;
	  case 6:
	       rval = (1.25f * ((float) cval + 102.f));
	       break;
	  case 7:
	       rval = (1.5f * ((float) cval - 126.f));
	       break;
	  case 8:
	       rval = (1.25f * ((float) cval - 126.f));
	       break;
	  }
     } else { /* scaleFlag == SCIA_MEM_SCALE_OLD */
	  switch ( (int) chanID ) {
	  case 1: case 2: case 3: case 4: case 5:
	       rval = (2.f * (float) cval);
	       break;
	  case 6:
	       rval = (1.25f * ((float) cval + 102.f));
	       break;
	  case 7:
	       rval = (1.5f * ((float) cval - 126.f));
	       break;
	  case 8:
	       rval = (1.25f * ((float) cval - 126.f));
	       break;
	  }
     }
     return rval;
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_MemCorr
.PURPOSE     apply Memory correction to Science data (channel 1 - 5)
.INPUT/OUTPUT
  call as    nr_pix = Apply_MemCorr( scaleFlag, chanID, coaddf, 
                                     clus, pixel_val );
     input:
           unsigned char scaleFlag :  flag to define scale/offset 
                                      for encoded correction values
           unsigned char chanID    :  channel ID
           unsigned char  coaddf   :  coadding factor
           struct Clus_scia *clus  :  MDS cluster record (level 1b)
 in/output:
           float *pixel_val        :  science pixel values (raw counts)

.RETURNS     number of calculated pixel values (unsigned int)
.COMMENTS    none
-------------------------*/
static inline
unsigned int Apply_MemCorr( unsigned char scaleFlag, unsigned char chanID,
			    unsigned char coaddf, 
			    const struct Clus_scia *clus, float *pixel_val )
     /*@modifies pixel_val@*/
{
     register unsigned int nb = 0u;
/*
 * apply memory correction on the Reticon detectors
 */
     if ( clus->n_sig > 0u ) {
          do {
               pixel_val[nb] -= 
		    MemNlin2Flt( scaleFlag, chanID, clus->sig[nb].corr );
          } while ( ++nb < clus->n_sig );
     } else if ( clus->n_sigc > 0u ) {
          do {
               pixel_val[nb] -= (float) coaddf *
                    MemNlin2Flt( scaleFlag, chanID, 
				 clus->sigc[nb].det.field.corr );
          } while ( ++nb < clus->n_sigc );
     }
     return nb;
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_nLinCorr
.PURPOSE     apply non-linearity correction to Science data (channel 6 - 8)
.INPUT/OUTPUT
  call as    nr_pix = Apply_nLinCorr( scaleFlag, chanID, coaddf, 
                                      clus, pixel_val );
     input:
           unsigned char scaleFlag :  flag to define scale/offset 
                                      for encoded correction values
           unsigned char chanID    :  channel ID
           unsigned char  coaddf   :  coadding factor
           struct Clus_scia *clus  :  MDS cluster record (level 1b)
 in/output:
           float *pixel_val        :  science pixel values (raw counts)

.RETURNS     number of calculated pixel values (unsigned int)
.COMMENTS    none
-------------------------*/
static inline
unsigned int Apply_nLinCorr( unsigned char scaleFlag, unsigned char chanID,
			     unsigned char coaddf, 
			     const struct Clus_scia *clus, float *pixel_val )
     /*@modifies pixel_val@*/
{
     register unsigned int nb = 0u;
/*
 * apply non-linearity correction on the Epitaxx detectors
 */
     if ( clus->n_sig > 0u ) {
          do {
               pixel_val[nb] -= 
		    MemNlin2Flt( scaleFlag, chanID, clus->sig[nb].corr );
          } while ( ++nb < clus->n_sig );
     } else if ( clus->n_sigc > 0u ) {
          do {
               pixel_val[nb] -= (float) coaddf *
                    MemNlin2Flt( scaleFlag, chanID, 
				 clus->sigc[nb].det.field.corr );
          } while ( ++nb < clus->n_sigc );
     }
     return nb;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_ATBD_CAL_MEM( unsigned char scaleFlag,
			const struct state1_scia *state,
			const struct mds1_scia *mds_1b,
			struct mds1c_scia *mds_1c )
{
     register unsigned short num = 0u;     /* counter for number of clusters */
     register unsigned short nd;           /* counter for number of DSR's */
     register unsigned int   nc;           /* counter for number of readouts */
/*
 * do actual memory correction
 */
     do {
	  if ( mds_1c->chan_id < FirstInfraChan ) {
	       nc = 0u;
	       nd = 0u;
	       do {
		    nc += Apply_MemCorr( scaleFlag, 
					 mds_1c->chan_id,
					 mds_1c->coaddf, 
					 mds_1b[nd].clus+num, 
					 mds_1c->pixel_val+nc );
	       } while ( ++nd < state->num_dsr );
	  }
     } while ( mds_1c++, ++num < state->num_clus );
}

/*--------------------------------------------------*/
void SCIA_ATBD_CAL_NLIN( unsigned char scaleFlag,
			 const struct state1_scia *state,
			 const struct mds1_scia *mds_1b,
			 struct mds1c_scia *mds_1c )
{
     register unsigned short num = 0u;     /* counter for number of clusters */
     register unsigned short nd;           /* counter for number of DSR's */
     register unsigned int   nc;           /* counter for number of readouts */
/*
 * do actual non-Linearity correction
 */
     do {
	  if ( mds_1c->chan_id >= FirstInfraChan ) {
	       nc = 0u;
	       nd = 0u;
	       do {
		    nc += Apply_nLinCorr( scaleFlag, 
					  mds_1c->chan_id,
					  mds_1c->coaddf, 
					  mds_1b[nd].clus+num, 
					  mds_1c->pixel_val+nc );
	       } while ( ++nd < state->num_dsr );
	  }
     } while ( mds_1c++, ++num < state->num_clus );
}
