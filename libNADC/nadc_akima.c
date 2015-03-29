/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 1999 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   FIT_GRID_AKIMA
.AUTHOR      R.M. van Hees
.KEYWORDS    polynome inter- and extrapolation
.LANGUAGE    ANSI C
.PURPOSE     interpolates values of a given spectrum to another wavelength grid
.INPUT/OUTPUT
  call as   FIT_GRID_AKIMA( FLT32_T, FLT64_T, dim_in, x_in, y_in, 
                            FLT64_T, FLT64_T, dim_out, x_out, y_out );
     input:  
	    int    x_type_in  :  data type of input values (float or double)
	    int    y_type_in  :  data type of input values (float or double)
	    size_t dim_in     :  dimension of x_in, y_in
            void   *x_in      :  input (wavelength) grid
	    void   *y_in      :  y values of the given spectrum
	    int    x_type_out :  data type of output values (float or double)
	    int    y_type_out :  data type of output values (float or double)
	    size_t dim_out    :  dimension of x_out, y_out
	    void   *x_out     :  output (wavelength) grid
    output:  
	    void   *y_out     :  y values of the spectrum fitted to grid
.RETURNS     nothing
.COMMENTS    nothing
.ENVIRONment None
.VERSION     4.3     23-Nov-2003   more BUGs Fixed, RvH
             4.2     03-Jul-2003   BUG Fix: used uninitialised memory
                                   in NADC_AKIMA_EX, RvH
             4.1     23-Jan-2003   several small bugs fixed, RvH
             4.0     10-Jan-2003   fixed many little bugs,
                                   only FIT_GRID_AKIMA is exported, RvH
             3.1     26-Nov-2002   made FIT_GRID_AKIMA faster 
                                   for monotonic input, RvH
             3.0     31-Oct-2001   moved to new Error handling routines, RvH
             2.3     25-Oct-2001   rename modules to NADC_..., RvH
             2.2     15-Aug-2001   debugging, RvH
             2.1     13-Jul-2001   added Fit_Grid_Akima, RvH
             2.0     01-Jul-1999   small improvements, RvH
             1.0     14-Jul-1994   Akima, Implemented in C by Bernd Aberle, 
	                           DLR/WT-DA-SE
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++ Macros +++++*/
#define SMALL_EPSILON        (1e-10)
#define VERY_SMALL_EPSILON   (1e-16)

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   NADC_AKIMA_EX
.PURPOSE     extrapolates the function two points left and right
.INPUT/OUTPUT
  call as   NADC_AKIMA_EX( FLT32_T, dim_in, x_in, y_in, xx, yy, st );

     input:  
	    int    x_type_id :  data type of x values (float or double)
	    int    y_type_id :  data type of y values (float or double)
	    size_t dim_in  :  dimension of x_in and y_in
            void   *x_in   :  x values
	    void   *y_in   :  y values
    output:  
            double *xx     :  x values extrapolated by 2 points left and right
            double *yy     :  y values extrapolated by 2 points left and right
            double *st     :  slope of curve

.RETURNS     Nothing
.COMMENTS    called by NADC_AKIMA_SU extrapolates the function two points left 
             and right call this subroutine once before evaluating the 
	     interpolation value of the new gridpoint (using NADC_AKIMA_PO)
-------------------------*/
static inline
void NADC_AKIMA_EX( int x_type_id, int y_type_id, size_t dim_in, 
		    const void *x_in, const void *y_in, 
		    /*@out@*/ double *xx, /*@out@*/ double *yy, 
		    /*@out@*/ double *st )
{
     register size_t ii;
     register double tmp;
/*
 * first part is the interpolation
 *
 * auxiliary points are moved 2 indices up
 */
     if ( x_type_id == FLT32_T ) {
	  const float *x_pntr = (const float *) x_in;

          ii = 0;
          do { xx[ii+2] = (double) (x_pntr[ii]); } while ( ++ii < dim_in );
     } else if ( x_type_id == FLT64_T ) {
	  (void) memcpy( xx+2, x_in, dim_in * sizeof( double ) );
     } else
          NADC_RETURN_ERROR( NADC_ERR_PARAM, "x_type_id" );

     if ( y_type_id == FLT32_T ) {
	  const float *y_pntr = (const float *) y_in;

          ii = 0;
          do { yy[ii+2] = (double) (y_pntr[ii]); } while ( ++ii < dim_in );
     } else if ( y_type_id == FLT64_T ) {
	  (void) memcpy( yy+2, y_in, dim_in * sizeof( double ) );
     } else
          NADC_RETURN_ERROR( NADC_ERR_PARAM, "y_type_id" );
/*
 * calculation of extrapolated x-values left and right
 */
     xx[1] = xx[2] + xx[3] - xx[4];
     xx[0] = xx[1] + xx[2] - xx[3];

     xx[dim_in+2] = xx[dim_in+1] + xx[dim_in] - xx[dim_in-1];
     xx[dim_in+3] = xx[dim_in+2] + xx[dim_in+1] - xx[dim_in];
/*
 * calculation of slopes of curves
 */
     for( ii = 2; ii < dim_in+1; ii++ ) {
	  if ( (tmp = xx[ii+1] - xx[ii]) <= SMALL_EPSILON)
	       st[ii] = 0.0;
	  else
	       st[ii] = (yy[ii+1] - yy[ii]) / tmp;
     }
/*
 * calculation of extrapolated y-values left and right
 */
     tmp = xx[2] - xx[1];
     yy[1] = tmp * (st[3] - 2.0 * st[2]) + yy[2];
     if ( tmp <= SMALL_EPSILON )
	  st[1] = 0.0;
     else
	  st[1] = (yy[2] - yy[1]) / tmp;

     tmp = xx[1] - xx[0];
     yy[0] = tmp * (st[2] - 2.0 * st[1]) + yy[1];
     if ( tmp <= SMALL_EPSILON )
	  st[0] = 0.0;
     else
	  st[0] = (yy[1] - yy[0]) / tmp;

     tmp = xx[dim_in+2] - xx[dim_in+1];
     yy[dim_in+2] = tmp * (2 * st[dim_in] - st[dim_in-1]) + yy[dim_in+1];
     if ( tmp <= SMALL_EPSILON )
	  st[dim_in+1] = 0.0;
     else
	  st[dim_in+1] = (yy[dim_in+2] - yy[dim_in+1]) / tmp;

     tmp = xx[dim_in+3] - xx[dim_in+2];
     yy[dim_in+3] = tmp * (2 * st[dim_in+1] - st[dim_in]) + yy[dim_in+2];
     if ( tmp <= SMALL_EPSILON )
	  st[dim_in+2] = 0.0;
     else
	  
	  st[dim_in+2] = (yy[dim_in+3] - yy[dim_in+2]) / tmp;
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_AKIMA_SU
.PURPOSE     fits a polynome through a given input function for 
             inter- and extrapolation
.INPUT/OUTPUT
  call as   NADC_AKIMA_SU( FLT32_T, FLT64_T, dim_in, x_in, y_in, 
                           a_coef, b_coef, c_coef, d_coef );
     input:  
            int    x_type_id :   data type of x values (float or double)
            int    y_type_id :   data type of y values (float or double)
	    size_t dim_in  :   dimension of x_in, y_in
	    void   *x_in   :   x values
	    void   *y_in   :   y values
    output:  
	    double *a_coef :   polynomial coefficients
	    double *b_coef :   polynomial coefficients
	    double *c_coef :   polynomial coefficients
	    double *d_coef :   polynomial coefficients

.RETURNS     Nothing
.COMMENTS    call this subroutine once before evaluating the interpolation
             value of the new gridpoint (using NADC_AKIMA_PO)
-------------------------*/
void NADC_AKIMA_SU( int x_type_id, int y_type_id, size_t dim_in, 
		    const void *x_in, const void *y_in, 
		    double *a_coef, double *b_coef, 
		    double *c_coef, double *d_coef )
{
     register size_t  ii;
     register double  tmp1, tmp2;

     double   *xx, *yy, *st, *tt;
/*
 * calculate auxiliary points left and right (two on both sides)
 */
     const size_t akima_points = dim_in + 4;

     xx = (double *) malloc( akima_points * sizeof( double ));
     if ( xx == NULL ) 
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "xx" );
     yy = (double *) malloc( akima_points * sizeof( double ));
     if ( yy == NULL ) {
	  free( xx );
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "yy" );
     }
     tt = (double *) calloc( akima_points, sizeof( double ));
     if ( tt == NULL ) {
	  free( xx ); free( yy );
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "tt" );
     }
     st = (double *) calloc( akima_points, sizeof( double ));
     if ( st == NULL ) {
	  free( xx ); free( yy ); free( tt );
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "st" );
     }
     NADC_AKIMA_EX( x_type_id, y_type_id, dim_in, x_in, y_in, xx, yy, st );
     if ( IS_ERR_STAT_FATAL ) {
	  free( xx ); free( yy ); free( tt ); free( st );
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, "NADC_AKIMA_EX" );
     }
/*
 * calculation of all polynomial slopes
 */
     ii = 2;
     do {
	  if ( (st[ii-2] == st[ii-1]) && (st[ii] == st[ii+1]) )
	       tt[ii] = 0.5 * (st[ii+1] + st[ii]);
	  else {
	       tmp1 = fabs( st[ii+1] - st[ii]   );
	       tmp2 = fabs( st[ii-1] - st[ii-2] );

	       if ( (tmp1 + tmp2) >= VERY_SMALL_EPSILON )
		    tt[ii] = (tmp1 * st[ii-1] + tmp2 * st[ii]) 
			 / (tmp1 + tmp2);
	       else 
		    tt[ii] = 0.0;
	  }
     } while( ++ii < dim_in+2 );
/*
 * back-indexing auxiliary points by 2 entries
 */
     ii = 0;
     do {
	  xx[ii] = xx[ii+2];
	  yy[ii] = yy[ii+2];
	  tt[ii] = tt[ii+2];
     } while( ++ii < dim_in );
/*
 * calculation of polynomial coefficients
 */
     ii = 0;
     do {
	  a_coef[ii] = yy[ii];
	  b_coef[ii] = tt[ii];

	  if ( (tmp1 = xx[ii+1] - xx[ii]) >= SMALL_EPSILON )
	       c_coef[ii] = (((3. * st[ii+2]) - (2. * tt[ii])) - tt[ii+1]) 
		    / tmp1;
	  else 
	       c_coef[ii] = 0.0;

	  if ( (tmp1 *= tmp1) >= SMALL_EPSILON )
	       d_coef[ii] = ( (tt[ii] + tt[ii+1]) - (2.0 * st[ii+2]) ) / tmp1;
	  else
	       d_coef[ii] = 0.0;
     } while ( ++ii < dim_in-1 );
     free( xx );
     free( yy );
     free( tt );
     free( st );
} 

/*+++++++++++++++++++++++++
.IDENTifer   NADC_AKIMA_PO
.PURPOSE     evaluates the interpolation value on a new gridpoint x_val
.INPUT/OUTPUT
  call as   NADC_AKIMA_PO( dim_in, x_in, 
                           a_coef, b_coef, c_coef, d_coef, x_val );

     input:  
	    size_t dim_in   :     dimension of x_in, y_in
            float  *x_in    :     x values
	    double *a_coef  :     polynomial coefficients
	    double *b_coef  :     polynomial coefficients
	    double *c_coef  :     polynomial coefficients
	    double *d_coef  :     polynomial coefficients
	    float  x_val    :     the abscissa at which you want a y-value

.RETURNS     the function value at point x_val
.COMMENTS    Given an n-dim. array of abscissa x_in and arrays a, b, c, d of
             polynomial coefficients as returned by the NADC_AKIMA_SU, this
             subroutine returns the interpolated value at abscissa x_val

	     The polynomial coefficients a,b,c,d must be known from a
	     call to subroutine 'NADC_AKIMA_SU'.
-------------------------*/
double NADC_AKIMA_PO( size_t dim_in, const double *x_in, 
		      const double *a_coef, const double *b_coef, 
		      const double *c_coef, const double *d_coef, 
		      double x_val )
{
     register size_t xlo, xhi, xi;

     double xdelta;

     const bool ascnd = (x_in[dim_in-1] >= x_in[0]);
/*
 * use bi-section algorithm
 */
     xlo = 0;
     xhi = dim_in - 1;
     while( xhi - xlo > 1 ) {
	  xi = (xhi+xlo) >> 1;
	  if ( (x_in[xi] <= x_val) == ascnd ) 
	       xlo = xi;
	  else 
	       xhi = xi;
     }
     xi = xlo;
     xdelta = x_val - x_in[xi];
/*
 * do the interpolation
 */
     return (a_coef[xi] 
	     + xdelta * (b_coef[xi] 
			 + xdelta * (c_coef[xi] 
				     + xdelta * d_coef[xi])));
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void FIT_GRID_AKIMA( int x_type_in, int y_type_in, size_t dim_in, 
		     const void *x_in, const void *y_in, 
		     int x_type_out, int y_type_out, size_t dim_out, 
		     const void *x_out, void *y_out )
{
     register size_t nr, xi;
     register double xdelta, y_dbl_val;

     bool   ascnd_out;
     bool   mono_out = TRUE;
     double *a_coef, *b_coef, *c_coef, *d_coef;

     double *xbuff_in  = NULL;
     double *xbuff_out = NULL;

     float  *y_flt_out = (float *) y_out;
     double *y_dbl_out = (double *) y_out;

     const double  *x_dbl_in;
     const double  *x_dbl_out;

     if ( (x_type_in != FLT32_T && x_type_in != FLT64_T)
	  || (x_type_out != FLT32_T && x_type_out != FLT64_T)
	  || (y_type_in != FLT32_T && y_type_in != FLT64_T) 
	  || (y_type_out != FLT32_T && y_type_out != FLT64_T) )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, "invalid types used" );
/*
 * do all calculation in double precision
 */
     if ( x_type_in == FLT32_T ) {
	  const float *x_pntr = (const float *) x_in;

	  xbuff_in = (double *) malloc( dim_in * sizeof( double ));
	  if ( xbuff_in == NULL ) {
	       NADC_RETURN_ERROR( NADC_ERR_ALLOC, "xbuff_in" );
	  }

	  nr = 0;
	  do { xbuff_in[nr] = (double)(x_pntr[nr]); } while( ++nr < dim_in );
	  x_dbl_in = xbuff_in;
     } else {
	  x_dbl_in = (const double *) x_in;
     }
     if ( x_type_out == FLT32_T ) {
	  const float *x_pntr = (const float *) x_out;

	  xbuff_out = (double *) malloc( dim_out * sizeof( double ));
	  if ( xbuff_out == NULL ) {
	       NADC_RETURN_ERROR( NADC_ERR_ALLOC, "xbuff_out" );
	  }

	  nr = 0;
	  do { xbuff_out[nr] = (double)(x_pntr[nr]); } while( ++nr < dim_out );
	  x_dbl_out = xbuff_out;
     } else {
	  x_dbl_out = (const double *) x_out;
     }
/*
 * check if x_in is monotonic increasing
 */
     nr = 0;
     do {
	  if ( ! (x_dbl_in[nr+1] >= x_dbl_in[nr]) ) {
	       char msg[64];

	       (void) sprintf(msg, 
			      "x_in is not monotonic increasing at %-zd", nr );
	       NADC_RETURN_ERROR( NADC_ERR_FATAL, msg ); 
	  }
     } while ( ++nr < dim_in-1 );
/*
 * allocate memory for akima polynomials
 */
     a_coef = (double *) malloc( dim_in * sizeof( double ));
     if ( a_coef == NULL ) 
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "a_coef");
     b_coef = (double *) malloc( dim_in * sizeof( double ));
     if ( b_coef == NULL ) {
	  free( a_coef );
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "b_coef");
     }
     c_coef = (double *) malloc( dim_in * sizeof( double ));
     if ( c_coef == NULL ) {
	  free( a_coef ); free( b_coef );
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "c_coef" );
     }
     d_coef = (double *) malloc( dim_in * sizeof( double ));
     if ( d_coef == NULL ) {
	  free( a_coef ); free( b_coef ); free( c_coef );
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "d_coef" );
     }
/*
 * calculate polynomials
 */
     NADC_AKIMA_SU( x_type_in, y_type_in, dim_in, x_in, y_in, 
		    a_coef, b_coef, c_coef, d_coef );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, "NADC_AKIMA_SU" );
/*
 * check if the x_out is monotonic increasing or decreasing
 */
     ascnd_out = (x_dbl_out[1] >= x_dbl_out[0]);
     nr = 1;
     do {
	  if ( ascnd_out != (x_dbl_out[nr+1] >= x_dbl_out[nr]) ) {
	       mono_out = FALSE;
	       break;
	  }
     } while ( ++nr < dim_out-1 );
/*
 * interpolate
 * special case:
 *    1) only one value requested
 *    2) x_out monotonic increasing
 *    3) x_out monotonic decreasing
 *    else use slower the NADC_AKIMA_PO function
 */
     if ( dim_out == 1 ) {
	  y_dbl_val = NADC_AKIMA_PO( dim_in, x_dbl_in, a_coef, b_coef, 
				     c_coef, d_coef, *x_dbl_out );
	  if ( y_type_out == FLT32_T )
	       *y_flt_out = (float) y_dbl_val;
	  else
	       *y_dbl_out = y_dbl_val;
     } else if ( mono_out && ascnd_out ) {
	  dim_in--;
	  xi = 0;
	  nr = 0;
	  do {
	       while( x_dbl_in[xi+1] < x_dbl_out[nr] && (xi+1) < dim_in ) xi++;
	       xdelta = x_dbl_out[nr] - x_dbl_in[xi];
	       y_dbl_val = (a_coef[xi] 
			    + xdelta * (b_coef[xi] 
					+ xdelta * (c_coef[xi] 
						    + xdelta * d_coef[xi])));
	       if ( y_type_out == FLT64_T )
		    y_dbl_out[nr] = y_dbl_val;
	       else
		    y_flt_out[nr] = (float) y_dbl_val;
	  } while ( ++nr < dim_out );
     } else if ( mono_out && ! ascnd_out ) {
	  dim_in--;
	  xi = dim_in - 1;
	  nr = 0;
	  do {
	       while( x_dbl_in[xi] > x_dbl_out[nr] && xi > 0 ) xi--;
	       xdelta = x_dbl_out[nr] - x_dbl_in[xi];
	       y_dbl_val = (a_coef[xi] 
			    + xdelta * (b_coef[xi] 
					+ xdelta * (c_coef[xi] 
						    + xdelta * d_coef[xi])));
	       if ( y_type_out == FLT32_T )
		    y_flt_out[nr] = (float) y_dbl_val;
	       else
		    y_dbl_out[nr] = y_dbl_val;
	  } while ( ++nr < dim_out );
     } else {
	  nr = 0;
	  do {
	       y_dbl_val = NADC_AKIMA_PO( dim_in, x_dbl_in, a_coef, b_coef, 
					  c_coef, d_coef, x_dbl_out[nr] );
	       if ( y_type_out == FLT32_T )
		    y_flt_out[nr] = (float) y_dbl_val;
	       else
		    y_dbl_out[nr] = y_dbl_val;
	  } while ( ++nr < dim_out );
     }
 done:
     free( a_coef );
     free( b_coef );
     free( c_coef );
     free( d_coef );
     if ( x_type_in == FLT32_T ) free( xbuff_in );
     if ( x_type_out == FLT32_T ) free( xbuff_out );
}
