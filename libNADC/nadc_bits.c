/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2002 - 2019 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_BITS
.AUTHOR      R.M. van Hees
.KEYWORDS    bit manipulation routines
.LANGUAGE    ANSI C
.PURPOSE     set/read bits in a unsigned long long variable
.COMMENTS    contains Set_Bit_uc, Get_Bit_uc, Set_Bit_LL and Get_Bit_LL
.ENVIRONment None
.VERSION     1.1     26-May-2019   added Set_Bit_uc() and Get_Bit_uc()
             1.0     26-Jul-2002   Created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdint.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++ Macros +++++*/
#define NUM_BITS_UC   8
#define NUM_BITS_ULL  64

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   Set_Bit_uc
.PURPOSE     set bits in a unsigned char variable
.INPUT/OUTPUT
  call as    Set_Bit_uc(val, pos);
     input:
             unsigned char *val :  input value
	     int pos            :  position of bit to be set or read
                                   valid range: 0 <= pos < 8            
.RETURNS     Nothing
.COMMENTS    None
-------------------------*/
void Set_Bit_uc(unsigned char *val, int pos)
{
     if (pos >= 0 && pos < NUM_BITS_UC)
	  *val = (*val) | ((unsigned char) ~(~0U << 1) << pos);
}

/*+++++++++++++++++++++++++
.IDENTifer   Get_Bit_uc
.PURPOSE     read bits in a unsigned char variable
.INPUT/OUTPUT
  call as    val = Get_Bit_uc(val, pos);
     input:
             unsigned char val  :  input value
	     int pos            :  position of bit to be set or read
                                   valid range: 0 <= pos < 8
            
.RETURNS     value of the requested bit (unsigned char)
.COMMENTS    None
-------------------------*/
unsigned char Get_Bit_uc(unsigned char val, int pos)
{
     if (pos >= 0 && pos < NUM_BITS_UC)
	  return (val >> pos) & ~((unsigned char) ~0U << 1);

     return (unsigned char) 0;
}

/*+++++++++++++++++++++++++
.IDENTifer   Set_Bit_LL
.PURPOSE     set bits in a unsigned long long variable
.INPUT/OUTPUT
  call as    Set_Bit_LL(x_ull, pos);
     input:
             unsigned long long *x_ull :  input value
	     int pos                   :  position of bit to be set or read
                                          valid range: 0 <= pos < 64
.RETURNS     Nothing
.COMMENTS    None
-------------------------*/
void Set_Bit_LL(unsigned long long *x_ull, int pos)
{
     if (pos >= 0 && pos < NUM_BITS_ULL)
	  *x_ull = (*x_ull) | (~(~0ULL << 1) << pos);
}

/*+++++++++++++++++++++++++
.IDENTifer   Get_Bit_LL
.PURPOSE     read bits in a unsigned long long variable
.INPUT/OUTPUT
  call as    x_ull = Get_Bit_LL(x_ull, pos);
     input:
             unsigned long long x_ull  :  input value
	     int pos                   :  position of bit to be set or read
                                          valid range: 0 <= pos < 64
            
.RETURNS     value of the requested bit (unsigned long long)
.COMMENTS    None
-------------------------*/
unsigned long long Get_Bit_LL(unsigned long long x_ull, int pos)
{
     if (pos >= 0 && pos < NUM_BITS_ULL)
	  return (x_ull >> pos) & ~(~0ULL << 1);

     return 0ULL;
}

