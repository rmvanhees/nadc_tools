/*+++++++++++++++++++++++++
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

.IDENTifer   Eval_Poly
.PURPOSE     evaluate a 5-th order polynomial
.INPUT/OUTPUT
  call as   yy = Eval_Poly( xx, coeffs )
     input:  
            unsigned int xx  :   
            double coeffs[5] :   polynomial coefficients

.RETURNS     polynomial evaluated at xx (double)
.COMMENTS    ANSI-C inline function, declare in your code as:
             #include <eval_poly.inc>
-------------------------*/
#ifndef  __EVAL_POLY                            /* Avoid redefinitions */
#define  __EVAL_POLY

static inline
double Eval_Poly( unsigned int xx, const double coeffs[] )
{
     return (((coeffs[4] * xx + coeffs[3]) * xx + coeffs[2]) 
	     * xx + coeffs[1]) * xx + coeffs[0];
}
#endif
