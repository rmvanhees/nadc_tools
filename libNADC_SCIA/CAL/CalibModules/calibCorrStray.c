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

.IDENTifer   calibStraylight
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy - Level 1b - Calibration
.LANGUAGE    ANSI C
.PURPOSE     perform Straylight correction on Sciamachy L1b science data
.INPUT/OUTPUT
  call as   SCIA_ATBD_CAL_STRAY( strayError, state, mds_1b, mds_1c );
     input:  
             float strayError           : taken from SIP GADS
	     struct state1_scia *state  : structure with States of the product
             struct mds1_scia *mds_1b   : level 1b MDS records
 in/output:  
             struct mds1c_scia *mds_1c  : level 1c MDS records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   16-Nov-2007 combined calibAtbdStray and calibSronStray, RvH
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
unsigned int applyStrayCorr( float scaleFactor, const struct Clus_scia *clus,
			     float *pixel_val )
     /*@modifies pixel_val@*/
{
     register unsigned int nb = 0;

     if ( clus->n_sig > 0u ) {
	  do {
	       pixel_val[nb] -= (scaleFactor * clus->sig[nb].stray);
	  } while ( ++nb < clus->n_sig );
     } else if ( clus->n_sigc > 0u ) {
	  do {
	       pixel_val[nb] -= (scaleFactor * clus->sigc[nb].stray);
	  } while ( ++nb < clus->n_sigc );
     }
     return nb;
}

static inline
unsigned int applyStrayError( float strayError, float scaleFactor,
			      const struct Clus_scia *clus,
			      float *pixel_err )
     /*@modifies pixel_err@*/
{
     register unsigned int nb = 0;

     if ( clus->n_sig > 0u ) {
	  do {
	       register double c_stray = scaleFactor * clus->sig[nb].stray;
	       register double e_stray = strayError * c_stray;

	       pixel_err[nb] += (float) (e_stray * e_stray);
	  } while ( ++nb < clus->n_sig );
     } else if ( clus->n_sigc > 0u ) {
	  do {
	       register double c_stray = scaleFactor * clus->sigc[nb].stray;
	       register double e_stray = strayError * c_stray;

	       pixel_err[nb] += (float) (e_stray * e_stray);
	  } while ( ++nb < clus->n_sigc );
     }
     return nb;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_ATBD_CAL_STRAY
.PURPOSE     perform Straylight correction on Sciamachy L1b science data
.INPUT/OUTPUT
  call as   SCIA_ATBD_CAL_STRAY( strayError, state, mds_1b, mds_1c );
     input:  
             float strayError           : taken from SIP GADS
	     struct state1_scia *state  : structure with States of the product
             struct mds1_scia *mds_1b   : level 1b MDS records
 in/output:  
             struct mds1c_scia *mds_1c  : level 1c MDS records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_ATBD_CAL_STRAY( float strayError, 
			  const struct state1_scia *state,
			  const struct mds1_scia *mds_1b,
			  struct mds1c_scia *mds_1c )
{
     /* const char prognm[] = "SCIA_ATBD_CAL_STRAY"; */

     register unsigned short num = 0u;     /* counter for number of clusters */
     register unsigned short nd;           /* counter for number of DSR's */
     register unsigned int   nc;           /* counter for number of readouts */
/*
 * do actual straylight correction 
 */
     do {
	  nc = 0u; nd = 0u;
	  do {
	       const float scaleFactor = 
		    mds_1b[nd].scale_factor[mds_1c->chan_id-1] / 10.f;

	       nc += applyStrayCorr( scaleFactor, mds_1b[nd].clus+num, 
				     mds_1c->pixel_val+nc );
	  } while ( ++nd < state->num_dsr );

	  if ( strayError > FLT_EPSILON ) {
	       nc = 0u; nd = 0u;
	       do {
		    const float scaleFactor = 
			 mds_1b[nd].scale_factor[mds_1c->chan_id-1] / 10.f;

		    nc += applyStrayError( strayError, scaleFactor, 
					   mds_1b[nd].clus+num, 
					   mds_1c->pixel_err+nc );
	       } while ( ++nd < state->num_dsr );
	  }
     } while ( mds_1c++, ++num < state->num_clus );   
}
