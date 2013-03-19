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

.IDENTifer   GET_SCIA_LV1C_MEAN
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 1c - statistics
.LANGUAGE    ANSI C
.PURPOSE     obtain statistics on cluster data from a state
.INPUT/OUTPUT
  call as   GET_SCIA_LV1C_MEAN( do_corr_coaddf, num_mds, mds, 
                                count, coaddf, pet, mean, sdev );
     input:  
             bool do_corr_coaddf    :  correct for co-adding of readouts
             unsigned short num_mds :  number of level 1c MDS records
	     struct mds1c_scia *mds :  level 1c MDS records
    output:  
             unsigned short *count  :  number of readouts
	     unsigned char  *coaddf :  co-adding factor
	     float          *pet    :  pixel exposure time
	     float          *mean   :  average signal
	     float          *sdev   :  standard deviation of the signal

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION     1.5     17-Feb-2008   replaced sigma with co-adding factor, RvH
             1.4     15-Feb-2008   original SDMF value for data-cliping is 
                                   7 sigma, not 4.5, RvH
             1.3     18-Jan-2008   added parameter do_corr_coaddf, RvH
             1.2     06-Dec-2007   some layout improvements, RvH
             1.1     06-Dec-2007   added PET to parameter list 
                                   + several bugs fixed, PvdM   
             1.0     12-Dec-2006   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++ Macros +++++*/
#define N_SIGMA   7       /* original value for this threshold is 7, not 4.5 */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/* static inline */
/* double _getMean( unsigned short dim, const double *buff ) */
/* { */
/*      register unsigned short nr = 0; */
/*      double mean = 0.; */

/*      do { */
/* 	  mean += buff[nr]; */
/*      } while ( ++ nr < dim); */
/*      return mean / dim; */
/* } */

static inline
double _getMeanAbsDev( unsigned short dim, double median, const double *buff )
{
     register unsigned short nr = 0;
     register double spread = 0.;

     do {
	  spread += fabs( buff[nr] - median );
     } while ( ++nr < dim );
     return spread / dim;
}

static inline
void _getMoment( unsigned short dim, const double *buff, 
		 /*@out@*/ double *ave, /*@out@*/ double *sdev )
     /*@globals  errno;@*/
     /*@modifies errno, *ave, *sdev@*/
{
     register unsigned short nr = 0;
     register double sum = 0.;

     do { sum += buff[nr]; } while( ++nr < dim );
     *ave = sum / dim;
     *sdev = 0.;

     if ( dim > 1 ) {
	  register double ep  = 0.;
	  register double var = 0.;

	  nr = 0;
	  do {
	       register double diff = buff[nr] - (*ave);
	       ep += diff;
	       var += (diff * diff);
	  } while( ++nr < dim );
	  *sdev = sqrt( (var - (ep * ep / dim)) / (dim-1) );
     }
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GET_SCIA_LV1C_MEAN( bool do_corr_coaddf, unsigned short num_mds, 
			 const struct mds1c_scia *mds_1c,
			 unsigned short *count, unsigned char *coaddf,
			 float *pet, float *mean, float *sdev )
{
     const char prognm[] = "GET_SCIA_LV1C_MEAN";

     register unsigned short nr = 0;
     register unsigned short no, np;
     register unsigned short nrObs;

     double *buff;
/*
 * initialisation output arrays
 */
     (void) memset( count, 0, SCIENCE_PIXELS * sizeof(short) );
     (void) memset( coaddf, 0, SCIENCE_PIXELS * sizeof(char) );
#ifdef INIT_ZERO
     (void) memset( pet,  0, SCIENCE_PIXELS * sizeof(float) );
     (void) memset( mean, 0, SCIENCE_PIXELS * sizeof(float) );
     (void) memset( sdev, 0, SCIENCE_PIXELS * sizeof(float) );
#else
     for ( np = 0; np < SCIENCE_PIXELS; np++ ) pet[np] = NAN;
     (void) memcpy( mean, pet, SCIENCE_PIXELS * sizeof(float) );
     (void) memcpy( sdev, pet, SCIENCE_PIXELS * sizeof(float) );
#endif
     if ( num_mds == 0 ) return;
/*
 * allocate space for temporary data buffer
 */
     nrObs = 0;
     for ( np = 0; np < num_mds; np++ )
	  if ( nrObs < mds_1c[np].num_obs ) nrObs = mds_1c[np].num_obs;
     if ( (buff = (double *) malloc( nrObs * sizeof(double) )) == NULL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "buff" );
/*
 * loop over all MDS records
 */
     do {
	  for ( np = 0; np < mds_1c[nr].num_pixels; np++ ) {
	       register unsigned short pixelID = mds_1c[nr].pixel_ids[np];
	       register unsigned int   indx = np;

	       coaddf[pixelID] = mds_1c[nr].coaddf;
	       pet[pixelID]    = mds_1c[nr].pet;

	       for ( nrObs = no = 0; no < mds_1c[nr].num_obs; no++ ) {
		    if ( mds_1c[nr].pixel_val[indx] >= FLT_EPSILON )
			 buff[nrObs++] = mds_1c[nr].pixel_val[indx];
		    indx += mds_1c[nr].num_pixels;
	       }

	       if ( nrObs > 0 ) {
		    register unsigned short nrSmooth = 0;
		    register double median = 
			 SELECTd( (nrObs+1)/2, (size_t) nrObs, buff );
		    register double mdev = 
			 _getMeanAbsDev( nrObs, median, buff );
		    double tmp_mean, tmp_sdev;

		    for ( nrSmooth = no = 0; no < nrObs; no++ ) {
			 if ( fabs(buff[no] - median) <= N_SIGMA * mdev )
			      buff[nrSmooth++] = buff[no];
		    }
		    if ( nrSmooth > 0 ) {
			 _getMoment( nrSmooth, buff, &tmp_mean, &tmp_sdev );
			 if ( do_corr_coaddf ) {
			      tmp_mean /= (double) (mds_1c[nr].coaddf);
			      tmp_sdev /= (double) (mds_1c[nr].coaddf);
			 }
			 count[pixelID] = nrSmooth;
			 mean[pixelID] = (float) tmp_mean;
			 sdev[pixelID] = (float) tmp_sdev;
		    }
	       }
	  }
     } while ( ++nr < num_mds );
/*
 * free allocated memory
 */
     free( buff );
}
