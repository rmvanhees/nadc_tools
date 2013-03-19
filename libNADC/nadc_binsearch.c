/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2008 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   BinarySearch
.AUTHOR      R.M. van Hees
.KEYWORDS    binary search
.LANGUAGE    ANSI C
.PURPOSE     searches an array[index[]] for presence of "value"
.INPUT/OUTPUT
  call as    res = BinarySearch( dim, index, array, value );
     input:
           int dim    :    dimension of the array to be sorted
           int *index :    indices to sort "array"
	   int *array :    array to be searched
	   int value  :    value to be found in array

.RETURNS     first occurences where array[index[res]] >= value
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     07-May-2008   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
        /* NONE */

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
int BinarySearch( int dim, const int *index, const int *array, int value )
{
     register int low  = 0;
     register int high = dim;

     while ( low < high ) {
	  register int mid = (low + high) / 2;

	  if ( array[index[mid]] < value)
               low = mid + 1; 
	  else
	       /* 
		* can't be high = mid-1: here A[mid] >= value,
		* so high can't be < mid if A[mid] == value
		*/
	       high = mid;
     }
     while ( low > 0 && array[index[low-1]] == value ) low--;
     return low;
}

