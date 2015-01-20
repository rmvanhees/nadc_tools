/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2009 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_MEDFIT
.AUTHOR      R.M. van Hees
.KEYWORDS    linear fit
.LANGUAGE    ANSI C
.PURPOSE     fitting data to a straight line y = a + b * x
.INPUT/OUTPUT
  call as   NADC_MEDFIT( dim_arr, x_arr, y_arr, err_arr, 
                         &fit_a, &fit_b, &abdev  );
     input:  
             size_t dim_arr  :  number of data points
             float *x_arr    :  independent variable values (X-axis)
             float *y_arr    :  dependent variable values (Y-axis)
             float *err_arr  :  measurement errors of the data points Y
    output:  
             float *fit_a    :  parameter a
             float *fit_b    :  parameter b 
             float *abdev    :  uncertainty in parameter a & b

.RETURNS     nothing
.COMMENTS    adopted from Numerical Recipes in C
.ENVIRONment None
.VERSION     1.0     16-Jan-2015   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <math.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++ Macros +++++*/
        /* NONE */

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++ Static Variables +++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#define EPS 1.0e-7

static size_t  dim;
static const float *xt, *yt;

static
float _rofunc( float bval, /*@out@*/ float *aa, /*@out@*/float *abdevt )
     /*@globals errno;@*/
     /*@modifies errno;@*/
{
     register size_t nd;

     float sum = 0.f;

     float *dbuff;

     dbuff = (float *) malloc( dim * sizeof(float) );
     for ( nd = 0; nd < dim; nd++ )
	  dbuff[nd] = yt[nd] - bval * xt[nd];

     if ( (dim % 2) == 0 ) {
	  nd = dim >> 1;
	  *aa = SELECTr( nd, dim, dbuff );
	  if ( dim < 100 ) {
	       *aa += SELECTr( nd+1, dim, dbuff );
	       *aa /= 2;
	  }
     } else {
	  *aa = SELECTr( (dim + 1) >> 1, dim, dbuff );
     }

     *abdevt = 0.f;
     for ( nd = 0; nd < dim; nd++ ) {
	  register float dval = yt[nd] - (bval * xt[nd] + *aa);

	  *abdevt += fabsf(dval);
	  if ( yt[nd] != 0.f ) dval /= fabsf(yt[nd]);
	  if ( fabsf(dval) > EPS ) 
	       sum += (dval >= 0.f ? xt[nd] : -xt[nd]);
     }
     free( dbuff );

     return sum;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void NADC_MEDFIT( size_t dim_arr,
		  const float *x_arr, const float *y_arr,
		  float *fit_a, float *fit_b, float *abdev )
{
     register size_t nd;

     float aa, bb, del, abdevt;
     float b1, b2, f, f1, f2, sigb;

     float sx = 0.f, sy = 0.f, sxy = 0.f, sxx = 0.f, chisq = 0.f;

     // initialize external variables
     dim = dim_arr;
     xt  = x_arr;
     yt  = y_arr;

     /*
      * As a first guess for a and b, 
      * we will find the least squares fitting line
      */
     for ( nd = 0; nd < dim_arr; nd++ ) {
	  sx += x_arr[nd];
	  sy += y_arr[nd];
	  sxy += x_arr[nd] * y_arr[nd];
	  sxx += x_arr[nd] * x_arr[nd];
     }

     del = dim_arr * sxx - sx * sx;
     aa = (sxx * sy - sx * sxy) / del;
     bb = (dim_arr * sxy - sx * sy) / del;
     for ( nd = 0; nd < dim_arr; nd++ ) {
	  register float temp = y_arr[nd] - (aa + bb * x_arr[nd]);

	  chisq += temp * temp;
     }

     /*
      * The standard deviations will give some idea 
      * of how big an iteration step to take
      */
     sigb = sqrtf(chisq / del);

     /*
      * Guess bracket as 3-sigma away in the downhill direction known from f1
      */
     b1 = bb;
     f1 = _rofunc( b1, &aa, &abdevt );
     b2 = bb + 3.0 * ((f1 >= 0.f) ? sigb : -sigb);
     f2 = _rofunc( b2, &aa, &abdevt );
     while ( f1 * f2 > 0.f ) {                // Bracketing
	  bb = 2.0 * b2 - b1;
	  b1 = b2;
	  f1 = f2;
	  b2 = bb;
	  f2 = _rofunc( b2, &aa, &abdevt );
     }

     /*
      * Refine until error a negligible number of standard deviation
      */
     sigb = 0.01f * sigb;
     while ( fabsf(b2 - b1) > sigb ) {
	  bb = 0.5f * (b1 + b2);              // Bisection
	  if ( bb == b1 || bb == b2 ) break;
	  f = _rofunc( bb, &aa, &abdevt );
	  if ( f * f1 >= 0.f ) {
	       f1 = f;
	       b1 = bb;
	  } else {
	       f2 = f;
	       b2 = bb;
	  }
     }
     *fit_a = (float) aa;
     *fit_b = (float) bb;
     *abdev = (float) (abdevt / dim_arr);
}

/*
 * compile code with
 * gcc -Wall -Os -DTEST_PROG -I../include -o nadc_medfit nadc_medfit.c -lm
 */
#ifdef TEST_PROG
#include <stdio.h>

int main()
{
     float fit_a, fit_b, abdev;

     size_t dim = 12;
     const float xx[] = { -3.20, 4.49, -1.66, 0.64, -2.43, -0.89,
			  -0.12, 1.41, 2.95, 2.18, 3.72, 5.26 };
     const float yy[] = { -7.14, -1.30, -4.26, -1.90, -6.19, -3.98, 
			  -2.87, -1.66, -0.78, -2.61,  0.31,  1.74 };

     NADC_MEDFIT( dim, xx, yy, &fit_a, &fit_b, &abdev );
     (void) printf( "medfit1 %f %f %f\n", fit_a, fit_b, abdev );

     return 0;
}
#endif
