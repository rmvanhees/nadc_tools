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

.IDENTifer   NADC_FIT
.AUTHOR      R.M. van Hees
.KEYWORDS    linear fit
.LANGUAGE    ANSI C
.PURPOSE     fitting data to a straight line y = a + b * x
.INPUT/OUTPUT
  call as   NADC_FIT( dim_arr, x_arr, y_arr, err_arr, 
                      &fit_a, &fit_b, &sig_a, &sig_b, &chi2_fit, &q_fit  );
     input:  
             size_t dim_arr  :  number of data points
	     float *x_arr    :  independent variable values (X-axis)
	     float *y_arr    :  dependent variable values (Y-axis)
	     float *err_arr  :  measurement errors of the data points Y
    output:  
	     float *fit_a    :  parameter a
	     float *fit_b    :  parameter b, 
                                if NULL then a horizontal line is fitted
	     float *sig_a    :  uncertainty in parameter a
	     float *sig_b    :  uncertainty in parameter b,
	                        not calculated in case fit_b == NULL
	     float *chi2_fit :  chi-square of fit
	     float *q_fit    :  goodness-of-fit probability
	                        (NULL will skip the calculation)
.RETURNS     nothing
.COMMENTS    adopted from Numerical Recipes in C
.ENVIRONment None
.VERSION     2.1     13-Oct-2011   added fit with horizontal line, RvH
             2.0.1   18-Feb-2011   minor enhancements - potential speed-up, RvH
             2.0     10-Sept-2010  added incomplete gamma function
                                   to calculate goodness-of-fit probability, RvH
             1.0     26-Mar-2009   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <float.h>
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
#define ITMAX  100
#define EPS      3e-7
#define FPMIN    1e-30


static
double gammln( double xx )
     /*@globals errno;@*/
     /*@modifies errno;@*/
{
     register unsigned short jj = 0;

     double yval, tmp, ser;

     const double xval = xx;
     const double cof[6]={ 
	  76.18009172947146, -86.50532032941677,
	  24.01409824083091, -1.231739572450155,
	  0.1208650973866179e-2, -0.5395239384953e-5
     };

     yval = xx;
     ser = 1.000000000190015;
     do {
	  ser += cof[jj] / ++yval;
     } while ( ++jj < 6 );

     tmp = xval + 5.5;
     tmp -= (xval + 0.5) * log(tmp);

     return -tmp + log(2.5066282746310005 * ser / xval);
}

static
double gser( double aval, double xval )
     /*@globals errno;@*/
     /*@modifies errno;@*/
{
     register unsigned short jj = 0;

     double sum, del, ap;

     if ( xval < DBL_EPSILON ) return 0.0;

     ap = aval;
     del = sum = 1 / aval;
     do {
	  del *= xval / (++ap);
	  sum += del;
	  if ( fabs(del) < (EPS * fabs(sum)) ) {
	       double gln = gammln(aval);

	       return sum * exp( -xval + aval * log(xval) - gln );
	  }
     } while ( ++jj < ITMAX );

     return 0.0;
}

static
double gcf( double aval, double xval )
     /*@globals errno;@*/
     /*@modifies errno;@*/
{
     register unsigned short jj = 1;

     double an, b, c, d, del, h;

     b = xval + 1 - aval;
     c = 1 / FPMIN;
     h = d = 1 / b;
     do {
	  an = -jj * (jj - aval);
	  b += 2;
	  d = an * d + b;
	  if ( fabs(d) < FPMIN) d = FPMIN;
	  c = b + an / c;
	  if ( fabs(c) < FPMIN) c = FPMIN;
	  d = 1 / d;
	  del = d * c;
	  h *= del;
	  if ( fabs(del-1) < EPS) {
	       double gln = gammln(aval);

	       return h * exp( -xval + aval * log(xval) - gln );
	  }
     }while ( ++jj <= ITMAX );

     return 0.0;
}

static
double gammq( double aval, double xval )
     /*@globals errno;@*/
     /*@modifies errno;@*/
{
     if ( xval < (aval + 1) ) {
	  return 1 - gser( aval, xval );
     } else {
	  return gcf( aval, xval );
     }
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void NADC_FIT( size_t dim_arr,
	       const float *x_arr, const float *y_arr, const float *err_arr,
	       float *fit_a, float *fit_b, float *sig_a, float *sig_b, 
	       float *chi2_fit, float *q_fit  )
{
     register size_t nd;

     double ss = 0., sx = 0., sy = 0., st2 = 0.;
     double aa, bb, chi2, sxoss;

     /* return if dimension is zero or one */
     if ( dim_arr == 0 ) {
	  *fit_a = *sig_a = *sig_b = 0.f;
	  if ( fit_b != NULL ) *fit_b = 0.f;
	  *chi2_fit = -999.f;
	  if ( q_fit != NULL ) *q_fit = 0.f;
	  return;
     } else if ( dim_arr == 1 ) {
	  *fit_a = *y_arr;
	  *sig_a = *err_arr;
	  if ( fit_b != NULL ) *fit_b = 0.f;
	  *sig_b = 0.f;
	  *chi2_fit = -999.f;
	  if ( q_fit != NULL ) *q_fit = 0.f;
	  return;
     }

     /* accumulate sums */
     nd = 0;
     do {
	  const double err_y = (double) err_arr[nd];
	  const double wt = 1. / (err_y * err_y);

	  ss += wt;
	  sx += x_arr[nd] * wt;
	  sy += y_arr[nd] * wt;
     } while ( ++nd < dim_arr );

     if ( fit_b == NULL ) {
	  aa = sy / ss;
	  bb = 0.;

	  *fit_a = (float) aa;
	  *sig_a = (float) sqrt( 1. / ss );
	  *sig_b = 0.f;
     } else {
	  sxoss = sx / ss;

          /* first iteration to solve b */
	  nd = 0;
	  bb = 0.;
	  do {
	       const double err_y = (double) err_arr[nd];
	       const double tmp = (x_arr[nd] - sxoss) / err_y;

	       st2 += tmp * tmp;
	       bb += tmp * y_arr[nd] / err_y;
	  } while ( ++nd < dim_arr );

          /* solve for a, b */
	  bb /= st2;
	  aa = (sy - sx * bb) / ss;

	  *fit_a = (float) aa;
	  *fit_b = (float) bb;
	  *sig_a = (float) sqrt( (1. + sx * sx / (ss * st2)) / ss );
	  *sig_b = (float) sqrt( 1. / st2 );
     }

     /* calculate chi2 and the quality of the fit */
     nd = 0;
     chi2 = 0.;
     do {
	  const double err_y = (double) err_arr[nd];
	  const double tmp = 
	       (y_arr[nd] - (aa + bb * x_arr[nd])) / err_y;

	  chi2 += (tmp * tmp);	  
     } while ( ++nd < dim_arr );

     *chi2_fit = (float) chi2;
     if ( q_fit != NULL ) {
	  if ( dim_arr == 2 || chi2 < DBL_EPSILON )
	       *q_fit = 0.f;
	  else
	       *q_fit = (float) gammq( 0.5 * (dim_arr - 2), 0.5 * chi2 );
     }
}

/*
 * compile code with
 * gcc -Wall -Os -DTEST_PROG -I$HOME/include -o nadc_fit nadc_fit.c -lm
 */
#ifdef TEST_PROG
#include <stdio.h>

int main()
{
     register size_t ii;
     float fit_a, fit_b, sig_a, sig_b, chi2_fit, q_fit;

     size_t dim = 12;
     const float xx[] = { -3.20, 4.49, -1.66, 0.64, -2.43, -0.89,
			  -0.12, 1.41, 2.95, 2.18, 3.72, 5.26 };
     const float yy[] = { -7.14, -1.30, -4.26, -1.90, -6.19, -3.98, 
			  -2.87, -1.66, -0.78, -2.61,  0.31,  1.74 };
     float sdev[12];

     for ( ii = 0; ii < dim; ii++ ) sdev[ii] = 0.1;
     NADC_FIT( dim, xx, yy, sdev, 
	       &fit_a, &fit_b, &sig_a, &sig_b, &chi2_fit, &q_fit );
     (void) printf( "fit1[2] %f %f %f %f %f %f\n", 
		    fit_a, fit_b, sig_a, sig_b, chi2_fit, q_fit );
     NADC_FIT( dim, xx, yy, sdev, 
	       &fit_a, NULL, &sig_a, &sig_b, &chi2_fit, &q_fit );
     (void) printf( "fit1[1] %f %f %f %f %f %f\n", 
		    fit_a, 0., sig_a, sig_b, chi2_fit, q_fit );

     for ( ii = 0; ii < dim; ii++ ) sdev[ii] = 1.0;
     NADC_FIT( dim, xx, yy, sdev, 
	       &fit_a, &fit_b, &sig_a, &sig_b, &chi2_fit, &q_fit );
     (void) printf( "fit2[2] %f %f %f %f %f %f\n", 
		    fit_a, fit_b, sig_a, sig_b, chi2_fit, q_fit );
     NADC_FIT( dim, xx, yy, sdev, 
	       &fit_a, NULL, &sig_a, &sig_b, &chi2_fit, &q_fit );
     (void) printf( "fit2[1] %f %f %f %f %f %f\n", 
		    fit_a, 0., sig_a, sig_b, chi2_fit, q_fit );

     sdev[0]  = sqrt( fabs( yy[0] ));
     sdev[1]  = sqrt( fabs( yy[1] ));
     sdev[2]  = sqrt( fabs( yy[2] ));
     sdev[3]  = sqrt( fabs( yy[3] ));
     sdev[4]  = sqrt( fabs( yy[4] ));
     sdev[5]  = sqrt( fabs( yy[5] ));
     sdev[6]  = sqrt( fabs( yy[6] ));
     sdev[7]  = sqrt( fabs( yy[7] ));
     sdev[8]  = sqrt( fabs( yy[8] ));
     sdev[9]  = sqrt( fabs( yy[9] ));
     sdev[10] = sqrt( fabs( yy[10] ));
     sdev[11] = sqrt( fabs( yy[11] ));
     NADC_FIT( dim, xx, yy, sdev, 
	       &fit_a, &fit_b, &sig_a, &sig_b, &chi2_fit, &q_fit );
     (void) printf( "fit3[2] %f %f %f %f %f %f\n", 
		    fit_a, fit_b, sig_a, sig_b, chi2_fit, q_fit );
     NADC_FIT( dim, xx, yy, sdev, 
	       &fit_a, NULL, &sig_a, &sig_b, &chi2_fit, &q_fit );
     (void) printf( "fit3[1] %f %f %f %f %f %f\n", 
		    fit_a, 0., sig_a, sig_b, chi2_fit, q_fit );
     return 0;
}
#endif
