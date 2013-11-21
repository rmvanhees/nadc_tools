/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 1996 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_SELECT
.AUTHOR      R.M. van Hees
.KEYWORDS    select, median
.LANGUAGE    ANSI C
.PURPOSE     return the kk-th smallest value of an array
.INPUT/OUTPUT
  call as    val = SELECTx( kk, dim, xa );
     input:  
             size_t kk  :    index number of the array to return
             size_t dim :    dimension of the array to be sorted
             array *xa  :    pointer to array

.RETURNS     value of the kk-th smallest value of an array
.COMMENTS    Contains SELECTuc, SELECTs, SELECTi, SELECTr, SELECTd
	     "x" specifies the data type of the input array, as follows:
	     \hspace*{3ex} "s" \hspace*{2ex} short
	     \hspace*{3ex} "i" \hspace*{2ex} integer
	     \hspace*{3ex} "r" \hspace*{2ex} float
	     \hspace*{3ex} "d" \hspace*{2ex} double
.ENVIRONment None
.VERSION     1.4     10-Nov-2013   added function for unsigned char, RvH
             1.3     29-Sep-2011   little stylistic changes, RvH
	     1.2     29-Dec-1997   little stylistic changes, RvH
             1.1     01-Dec-1997   Changed dimensions to size_t, RvH
             1.0     03-Nov-1995   Created by R.M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <string.h>
#include <stdlib.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++ Macros +++++*/
#define FOREVER    for(;;)
#define SWAP(a,b)  {temp = (a); (a) = (b); (b) = temp;}

/*
 * here start the code of function SELECTs
 */
unsigned char SELECTuc( const size_t kk, const size_t dim, 
			const unsigned char *ca )
{
     register size_t ll, hh;
     register unsigned char test, temp;

     size_t  low = 1;
     size_t  high = dim;

     unsigned char *ucbuff;

     if ( dim == 0 ) return 0;
     if ( dim == 1 ) return ca[0];
     ucbuff = (unsigned char *) malloc( (dim+1) * sizeof(char) );
     ucbuff[0] = 0;
     (void) memcpy( ucbuff+1, ca, dim * sizeof(char) );

     FOREVER {
	  if ( high <= low+1 ) {
	       if ( high == low+1 && ucbuff[low] < ucbuff[high] )
		    SWAP( ucbuff[low], ucbuff[high] );
	       break;
	  } else {
	       size_t mid = (low + high) / 2;

	       SWAP( ucbuff[mid], ucbuff[low+1] );
	       if ( ucbuff[low] > ucbuff[high] ) {
		    SWAP( ucbuff[low], ucbuff[high] );
	       }
	       if ( ucbuff[low+1] > ucbuff[high] ) {
		    SWAP( ucbuff[low+1], ucbuff[high] );
	       }
	       if ( ucbuff[low] > ucbuff[low+1] ) {
		    SWAP( ucbuff[low], ucbuff[low+1] );
	       }
	       ll = low + 1;
	       hh = high;
	       test = ucbuff[ll];
	       FOREVER {
		    do ll++; while ( ucbuff[ll] < test );
		    do hh--; while ( ucbuff[hh] > test );

		    if ( hh < ll ) break;
		    SWAP( ucbuff[ll], ucbuff[hh] );
	       }
	       ucbuff[low+1] = ucbuff[hh];
	       ucbuff[hh] = test;
	       if ( hh <= kk ) low = ll;
	       if ( hh >= kk ) high = hh - 1;
	  }
     }
     temp = ucbuff[kk];
     free( ucbuff );

     return temp;
}

/*
 * here start the code of function SELECTs
 */
short SELECTs( const size_t kk, const size_t dim, const short *sa )
{
     register size_t ll, hh;
     register short  test, temp;

     size_t  low = 1;
     size_t  high = dim;
     short   *sbuff;

     if ( dim == 0 ) return 0;
     if ( dim == 1 ) return sa[0];
     sbuff = (short *) malloc( (dim+1) * sizeof( short ));
     sbuff[0] = 0;
     (void) memcpy( sbuff+1, sa, dim * sizeof( short ));

     FOREVER {
	  if ( high <= low+1 ) {
	       if ( high == low+1 && sbuff[low] < sbuff[high] )
		    SWAP( sbuff[low], sbuff[high] );
	       break;
	  } else {
	       size_t mid = (low + high) / 2;

	       SWAP( sbuff[mid], sbuff[low+1] );
	       if ( sbuff[low] > sbuff[high] ) {
		    SWAP( sbuff[low], sbuff[high] );
	       }
	       if ( sbuff[low+1] > sbuff[high] ) {
		    SWAP( sbuff[low+1], sbuff[high] );
	       }
	       if ( sbuff[low] > sbuff[low+1] ) {
		    SWAP( sbuff[low], sbuff[low+1] );
	       }
	       ll = low + 1;
	       hh = high;
	       test = sbuff[ll];
	       FOREVER {
		    do ll++; while ( sbuff[ll] < test );
		    do hh--; while ( sbuff[hh] > test );

		    if ( hh < ll ) break;
		    SWAP( sbuff[ll], sbuff[hh] );
	       }
	       sbuff[low+1] = sbuff[hh];
	       sbuff[hh] = test;
	       if ( hh <= kk ) low = ll;
	       if ( hh >= kk ) high = hh - 1;
	  }
     }
     temp = sbuff[kk];
     free( sbuff );

     return temp;
}

/*
 * here start the code of function SELECTi
 */
int SELECTi( const size_t kk, const size_t dim, const int *ia )
{
     register size_t ll, hh;
     register int    test, temp;

     size_t  low = 1;
     size_t  high = dim;
     int     *ibuff;

     if ( dim == 0 ) return 0;
     if ( dim == 1 ) return ia[0];
     ibuff = (int *) malloc( (dim+1) * sizeof( int ));
     ibuff[0] = 0;
     (void) memcpy( ibuff+1, ia, dim * sizeof( int ));

     FOREVER {
	  if ( high <= low+1 ) {
	       if ( high == low+1 && ibuff[low] < ibuff[high] )
		    SWAP( ibuff[low], ibuff[high] );
	       break;
	  } else {
	       size_t mid = (low + high) / 2;

	       SWAP( ibuff[mid], ibuff[low+1] );
	       if ( ibuff[low] > ibuff[high] ) {
		    SWAP( ibuff[low], ibuff[high] );
	       }
	       if ( ibuff[low+1] > ibuff[high] ) {
		    SWAP( ibuff[low+1], ibuff[high] );
	       }
	       if ( ibuff[low] > ibuff[low+1] ) {
		    SWAP( ibuff[low], ibuff[low+1] );
	       }
	       ll = low + 1;
	       hh = high;
	       test = ibuff[ll];
	       FOREVER {
		    do ll++; while ( ibuff[ll] < test );
		    do hh--; while ( ibuff[hh] > test );

		    if ( hh < ll ) break;
		    SWAP( ibuff[ll], ibuff[hh] );
	       }
	       ibuff[low+1] = ibuff[hh];
	       ibuff[hh] = test;
	       if ( hh <= kk ) low = ll;
	       if ( hh >= kk ) high = hh - 1;
	  }
     }
     temp = ibuff[kk];
     free( ibuff );

     return temp;
}

/*
 * here start the code of function SELECTr
 */
float SELECTr( const size_t kk, const size_t dim, const float *ra )
{
     register size_t ll, hh;
     register float  test, temp;

     size_t  low = 1;
     size_t  high = dim;
     float   *rbuff;

     if ( dim == 0 ) return 0.f;
     if ( dim == 1 ) return ra[0];
     rbuff = (float *) malloc( (dim+1) * sizeof( float ) );
     rbuff[0] = 0.f;
     (void) memcpy( rbuff+1, ra, dim * sizeof( float ) );

     FOREVER {
	  if ( high <= low+1 ) {
	       if ( high == low+1 && rbuff[low] < rbuff[high] )
		    SWAP( rbuff[low], rbuff[high] );
	       break;
	  } else {
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
	       hh = high;
	       test = rbuff[ll];
	       FOREVER {
		    do ll++; while ( rbuff[ll] < test );
		    do hh--; while ( rbuff[hh] > test );

		    if ( hh < ll ) break;
		    SWAP( rbuff[ll], rbuff[hh] );
	       }
	       rbuff[low+1] = rbuff[hh];
	       rbuff[hh] = test;
	       if ( hh <= kk ) low = ll;
	       if ( hh >= kk ) high = hh - 1;
	  }
     }
     temp = rbuff[kk];
     free( rbuff );

     return temp;
}

/*
 * here start the code of function SELECTd
 */
double SELECTd( const size_t kk, const size_t dim, const double *da )
{
     register size_t ll, hh;
     register double test, temp;

     size_t  low = 1;
     size_t  high = dim;
     double  *dbuff;

     if ( dim == 0 ) return 0.;
     if ( dim == 1 ) return da[0];
     dbuff = (double *) malloc( (dim+1) * sizeof( double ) );
     dbuff[0] = 0.;
     (void) memcpy( dbuff+1, da, dim * sizeof( double ) );

     FOREVER {
	  if ( high <= low+1 ) {
	       if ( high == low+1 && dbuff[low] < dbuff[high] )
		    SWAP( dbuff[low], dbuff[high] );
	       break;
	  } else {
	       size_t mid = (low + high) / 2;

	       SWAP( dbuff[mid], dbuff[low+1] );
	       if ( dbuff[low] > dbuff[high] ) {
		    SWAP( dbuff[low], dbuff[high] );
	       }
	       if ( dbuff[low+1] > dbuff[high] ) {
		    SWAP( dbuff[low+1], dbuff[high] );
	       }
	       if ( dbuff[low] > dbuff[low+1] ) {
		    SWAP( dbuff[low], dbuff[low+1] );
	       }
	       ll = low + 1;
	       hh = high;
	       test = dbuff[ll];
	       FOREVER {
		    do ll++; while ( dbuff[ll] < test );
		    do hh--; while ( dbuff[hh] > test );

		    if ( hh < ll ) break;
		    SWAP( dbuff[ll], dbuff[hh] );
	       }
	       dbuff[low+1] = dbuff[hh];
	       dbuff[hh] = test;
	       if ( hh <= kk ) low = ll;
	       if ( hh >= kk ) high = hh - 1;
	  }
     }
     temp = dbuff[kk];
     free( dbuff );

     return temp;
}
