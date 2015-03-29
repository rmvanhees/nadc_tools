/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2014 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_SIGMACLIPPED
.AUTHOR      R.M. van Hees
.KEYWORDS    Statistics
.LANGUAGE    ANSI C
.PURPOSE     estimate mean and standard deviation after sigma clipping
.INPUT/OUTPUT
  call as   outlayers = NADC_SIGMACLIPPED( dim, arr, &mean, &sdev );
     input:  
             size_t   dim    :    dimension of array
             float    *arr   :    pointer to array
    output:
             float    mean   :    mean after rejecting outlayers
             float    sdev   :    standard deviation after rejecting outlayers
                                  (not calculated when NULL)

.RETURNS     number of outlayers  (size_t)
.COMMENTS    none
.ENVIRONment None
.VERSION     1.0     19-Jan-2014   initial release, Richard van Hees
-------------------------*/
/*
 * Define _ISOC99_SOURCE to indicat
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++ Macros +++++*/
#define N_SIGMA   7

#define FOREVER    for(;;)
#define SWAP(a,b)  {temp = (a); (a) = (b); (b) = temp;}

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   _SELECT
.PURPOSE     select kk-th smallest value a float array (qsort)
.INPUT/OUTPUT
  call as   val = _SELECT( kk, dim, arr );
     input:  
             size_t kk     :    location to select
             size_t dim    :    dimension of the array (see comments)
 in/output:  
             float *arr    :    pointer to array 

.RETURNS     the kk-th smallest value of a float array
.COMMENTS    static function
             input array is modified, it should have dim+1 elements, the first
             element is ignored.
-------------------------*/
static inline
float _SELECT( const size_t kk, const size_t dim, float *rbuff )
     /*@modifies rbuff@*/
{
     register size_t ll, hh;
     register float  test, temp;

     size_t  low = 1;
     size_t  high = dim;

     FOREVER {
	  if ( high > (low + 1) ) {
	       size_t mid = (low + high) / 2;

	       SWAP( rbuff[mid], rbuff[low+1] );
	       if ( rbuff[low] > rbuff[high] ) {
		    SWAP( rbuff[low], rbuff[high] );
	       }
	       if ( rbuff[low+1] > rbuff[high] ) {
		    SWAP( rbuff[low+1], rbuff[high] );
	       }
	       if ( rbuff[low] > rbuff[low+1] ) {
		    SWAP( rbuff[low], rbuff[low+1] );
	       }
	       ll = low + 1;
	       test = rbuff[ll];
	       hh = high;
	       FOREVER {
		    do ll++; while ( rbuff[ll] < test );
		    do hh--; while ( rbuff[hh] > test );
		    if ( hh < ll ) break;
		    SWAP( rbuff[ll], rbuff[hh] );
	       }
	       rbuff[low+1] = rbuff[hh];
	       rbuff[hh] = test;
	       if ( hh >= kk ) high = hh - 1;
	       if ( hh <= kk ) low = ll;
	  } else {
	       if ( high == (low + 1) && rbuff[low] > rbuff[high] )
		    SWAP( rbuff[low], rbuff[high] );
	       return rbuff[kk];
	  }
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   _getMEDIAN
.PURPOSE     estimate sample median
.INPUT/OUTPUT
  call as   val = _getMEDIAN( dim, arr );
     input:  
             size_t dim    :    dimension of array
             float  *arr   :    pointer to array 

.RETURNS     median value (float)
.COMMENTS    static function
-------------------------*/
static inline
float _getMEDIAN( const size_t dim, const float *arr )
     /*@globals nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/
{
     float med = 0.f;
     float *rbuff;

     if ( dim == 0 ) return 0;
     if ( dim == 1 ) return arr[0];

     /* initialize working buffer */
     rbuff = (float *) malloc( (dim+1) * sizeof(float) );
     if ( rbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );
     rbuff[0] = 0;
     (void) memcpy( rbuff+1, arr, dim * sizeof(float) );

     /* estimate sample median */
     if ( (dim % 2) == 0 ) {
	  med = _SELECT( dim/2, dim, rbuff );
	  if ( dim < 100 ) {
	       med += _SELECT( 1+dim/2, dim, rbuff );
	       med /= 2;
	  }
     } else
	  med = _SELECT( (dim+1)/2, dim, rbuff );

     free( rbuff );
done:
     return med;
}

/*+++++++++++++++++++++++++
.IDENTifer   _getADEV
.PURPOSE     estimate mean absolute deviation of an array
.INPUT/OUTPUT
  call as   val = _getADEV( dim, arr, median );
     input:  
             size_t dim    :    dimension of array
             float  *arr   :    pointer to array
	     float  median :    median value of array

.RETURNS     mean absolute deviation (float)
.COMMENTS    static function
-------------------------*/
static inline
float _getADEV( const size_t dim, const float *arr, const float median )
{
     register size_t nr = 0;
     register float  spread = 0.f;

     do {
          spread += fabsf( arr[nr] - median );
     } while ( ++nr < dim );

     return spread / dim;
}

/*+++++++++++++++++++++++++
.IDENTifer   _getMoment
.PURPOSE     estimate mean and standard deviation of an array
.INPUT/OUTPUT
  call as   val = _getMoment( dim, arr, &ave, &sdev );
     input:  
             size_t dim    :    dimension of array
             float  *arr   :    pointer to array
	     float  *ave   :    average value of array
	     float  *sdev  :    standard deviation of array

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
void _getMoment( size_t dim, const float *buff, 
                 /*@out@*/ float *ave, /*@out@*/ float *sdev )
     /*@globals  errno;@*/
     /*@modifies errno, *ave, *sdev@*/
{
     register size_t nr = 0;
     register double sum = 0.;

     do { sum += buff[nr]; } while( ++nr < dim );
     *ave = (float) (sum / dim);
     *sdev = 0.f;

     if ( dim > 1 ) {
          register double ep  = 0.;
          register double var = 0.;

          nr = 0;
          do {
               register double diff = buff[nr] - (*ave);

               ep += diff;
               var += (diff * diff);
          } while( ++nr < dim );
          *sdev = (float) sqrt( (var - (ep * ep / dim)) / (dim - 1.) );
     }
}

/*+++++++++++++++++++++++++ Main Function +++++++++++++++++++++++*/
size_t NADC_SIGMACLIPPED( const size_t dim, const float *arr, 
			  /*@out@*/ float *mean, /*@out@*/ float *sdev )
{
     register size_t ni;

     const float tmp_median = _getMEDIAN( dim, arr );
     const float tmp_adev   = _getADEV( dim, arr, tmp_median );

     const float max_dist_adev = N_SIGMA * tmp_adev;

     size_t num = 0, rejected = 0;
     float  *buff = NULL;

     *mean = tmp_median;
     if ( sdev != NULL ) *sdev = 0.f;
     if ( dim <= 1 || tmp_adev < FLT_EPSILON ) return 0;

     if ( (buff = (float *) malloc( dim * sizeof(float) )) == NULL ) 
          NADC_GOTO_ERROR( NADC_ERR_ALLOC, "buff" );
     
     /* calculate sigma-clipped mean and standard deviation */
     ni = 0;
     do {
	  if ( fabsf(arr[ni] - tmp_median) <= max_dist_adev )
	       buff[num++] = arr[ni];
	  else
	       rejected++;
     } while ( ++ni < dim );

     /* calculate mean and standard deviation */
     _getMoment( num, buff, mean, sdev );
     free( buff );
done:
     return rejected;
}
