/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2002 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_INTERPOL
.AUTHOR      R.M. van Hees
.KEYWORDS    math functions
.LANGUAGE    ANSI C
.PURPOSE     linear interpolation of an whole array
.CONTAINS    NADC_INTERPOL, NADC_INTERPOL_d
.RETURNS     nothing
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     16-Jan-2013   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <string.h>
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
                                /* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   NADC_INTERPOL
.PURPOSE     linear interpolation of an whole array
.INPUT/OUTPUT
  call as   NADC_INTERPOL( X, X_left, X_right, num_Y, Y_left, Y_right, Y );
     input:  
             float X            :  X value within range [X_left, X_right]
	     float X_left       :  lower boundary X value: X_left < X_right 
	     float X_right      :  upper boundary X value
	     unsigned int num_Y :  number of Y values to be calculated
	     float *Y_left      :  Y values at X_left
	     float *Y_right     :  Y values at X_right
    output:  
	     float  *Y          :  Y values found by linear interpolation

.RETURNS     nothing
.COMMENTS    None
-------------------------*/
void NADC_INTERPOL( float X, float X_left, float X_right, 
		    unsigned int num_Y, const float *Y_left_in, 
		    const float *Y_right_in, /*@unique@*/ float *Y )
{
     register unsigned int nr = 0;

     double xval = X;
     double dX, xleft, xright;
     const float *Y_left, *Y_right;

     if ( fabsf(X_right - X_left) < FLT_EPSILON ) {
	  (void) memcpy( Y, Y_right_in, num_Y * sizeof( float ) );
	  return;
     } 

     if ( X_left > X_right ) {
	  xleft = X_right;
	  xright = X_left;
	  Y_left = Y_right_in;
	  Y_right = Y_left_in;
     } else {
	  xleft = X_left;
	  xright = X_right;
	  Y_left = Y_left_in;
	  Y_right = Y_right_in;
     }
     if ( (xval - xright) > DBL_EPSILON || (xleft - xval) > DBL_EPSILON )
	  NADC_ERROR( NADC_ERR_WARN, "doing Linear Extrapolation" );

     dX = (xright - xval) / (xright - xleft);
     do {
	  *Y++ = (float) ((1 - dX) * Y_right[nr] + dX * Y_left[nr]);
     } while ( ++nr < num_Y );  
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_INTERPOL_d
.PURPOSE     linear interpolation of an whole array
.INPUT/OUTPUT
  call as   NADC_INTERPOL_d( X, X_left, X_right, num_Y, Y_left, Y_right, Y );
     input:  
             float X            :  X value within range [X_left, X_right]
	     float X_left       :  lower boundary X value: X_left < X_right 
	     float X_right      :  upper boundary X value
	     unsigned int num_Y :  number of Y values to be calculated
	     float *Y_left      :  Y values at X_left
	     float *Y_right     :  Y values at X_right
    output:  
	     double  *Y         :  Y values found by linear interpolation

.RETURNS     nothing
.COMMENTS    None
-------------------------*/
void NADC_INTERPOL_d( float X, float X_left, float X_right, 
		      unsigned int num_Y, const float *Y_left_in, 
		      const float *Y_right_in, /*@unique@*/ double *Y )
{
     register unsigned int nr = 0;
     
     double xval = X;
     double dX, xleft, xright;
     const float *Y_left, *Y_right;

     if ( fabsf(X_right - X_left) < FLT_EPSILON ) {
	  do { *Y++ = (double) Y_right_in[nr]; } while ( ++nr < num_Y );
	  return;
     }

     if ( X_left > X_right ) {
	  xleft = X_right;
	  xright = X_left;
	  Y_left = Y_right_in;
	  Y_right = Y_left_in;
     } else {
	  xleft = X_left;
	  xright = X_right;
	  Y_left = Y_left_in;
	  Y_right = Y_right_in;
     }
     if ( (xval - xright) > DBL_EPSILON || (xleft - xval) > DBL_EPSILON )
	  NADC_ERROR( NADC_ERR_WARN, "doing Linear Extrapolation" );

     dX = (xright - xval) / (xright - xleft);
     do {
	  *Y++ = (1 - dX) * Y_right[nr] + dX * Y_left[nr];
     } while ( ++nr < num_Y );  
}
