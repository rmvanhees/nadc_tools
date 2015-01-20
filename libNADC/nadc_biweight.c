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

.IDENTifer   NADC_BIWEIGHT
.AUTHOR      R.M. van Hees
.KEYWORDS    Statistics
.LANGUAGE    ANSI C
.PURPOSE     estimate biweight location and scale of an array
.INPUT/OUTPUT
  call as   outlayers = NADC_BIWEIGHT( dim, arr, &median, &scale );
     input:  
             size_t   dim    :    dimension of array
             float    *arr   :    pointer to array
    output:
             float    median :    biweight location estimation
             float    scale  :    biweight scale  estimation
                                  (not calculated when NULL)

.RETURNS     number of outlayers  (size_t)
.COMMENTS    none
.ENVIRONment None
.VERSION     1.0     15-Jan-2014   initial release, Richard van Hees
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
static
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
.IDENTifer   _MEDIAN
.PURPOSE     estimate sample median
.INPUT/OUTPUT
  call as   val = _MEDIAN( dim, arr );
     input:  
             size_t dim    :    dimension of array
             float  *arr   :    pointer to array 

.RETURNS     median value (float)
.COMMENTS    static function
-------------------------*/
static
float _MEDIAN( const size_t dim, const float *arr )
     /*@globals nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/
{
     const char prognm[] = "_MEDIAN";

     float med = 0.f;
     float *rbuff;

     if ( dim == 0 ) return 0;
     if ( dim == 1 ) return arr[0];

     /* initialize working buffer */
     rbuff = (float *) malloc( (dim+1) * sizeof(float) );
     if ( rbuff == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
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
.IDENTifer   _MAD
.PURPOSE     estimate median absolute deviation of an array
.INPUT/OUTPUT
  call as   val = _MAD( dim, arr, med );
     input:  
             size_t dim    :    dimension of array
             float  *arr   :    pointer to array
	     float  med    :    median value of array

.RETURNS     median absolute deviation (float)
.COMMENTS    static function
-------------------------*/
static
float _MAD( const size_t dim, const float *arr, const float med )
     /*@globals nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/
{
     const char prognm[] = "_MAD";

     register size_t ni = 1;

     float mad = 0.f;
     float *rbuff;

     if ( dim <= 1 ) return 0;

     /* initialize working buffer */
     rbuff = (float *) malloc( (dim+1) * sizeof(float) );
     if ( rbuff == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
     rbuff[0] = 0.f;
     do {
	  rbuff[ni] = fabsf(*arr - med);
     } while( ++arr, ++ni <= dim );

     /* estimate median absolute deviation */
     if ( (dim % 2) == 0 ) {
	  mad = _SELECT( dim/2, dim, rbuff );
	  if ( dim < 100 ) {
	       mad += _SELECT( 1+dim/2, dim, rbuff );
	       mad /= 2;
	  }
     } else
	  mad = _SELECT( (dim+1)/2, dim, rbuff );

     free( rbuff );
done:
     return mad;
}

/*+++++++++++++++++++++++++ Main Function +++++++++++++++++++++++*/
size_t NADC_BIWEIGHT( const size_t dim, const float *arr, 
		      /*@out@*/ float *median,
		      /*@out@*/ /*@null@*/ float *scale )
{
     register size_t ni;
     register double dist, uu, wght;

     const float med = _MEDIAN( dim, arr );
     const float mad = _MAD( dim, arr, med );

     const float max_dist_loc   = 6 * mad;
     const float max_dist_scale = 9 * mad;

     size_t rejected = 0;
     double sum1 = 0.;
     double sum2 = 0.;

     *median = med;
     if ( dim <= 1 || mad < FLT_EPSILON ) {
	  if ( scale != NULL ) *scale = 0.f;
	  return 0;
     }
     /* calculate one-step biweight location estimator */
     ni = 0;
     do {
	  dist = arr[ni] - med;
	  uu   = dist / max_dist_loc;

	  if ( (uu *= uu) > 1 ) {
	       rejected++;
	  } else {
	       wght = (1 - uu) * (1 - uu);
	  
	       sum1 += wght * dist;
	       sum2 += wght;
	  }
     } while( ++ni < dim );

     if ( sum2 > DBL_EPSILON )
	  *median += (float)(sum1 / sum2);

     /* calculate one-step biweight scale estimator */
     if ( scale != NULL ) {
	  double sum3 = 0.;
	  double sum4 = 0.;

	  ni = 0;
	  do {
	       dist = arr[ni] - med;
	       uu   = dist / max_dist_scale;

	       if ( (uu *= uu) > 1 ) continue;
	       wght = 1 - uu;

	       sum3 += dist * dist * wght * wght * wght * wght;
	       sum4 += wght * (1 - 5 * uu);
	  } while( ++ni < dim );
	  *scale = (float)(sqrt((dim * sum3) / (sum4 * sum4)));
     }
     return rejected;
}
