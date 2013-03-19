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

.IDENTifer   SWAP_BYTES
.AUTHOR      R.M. van Hees
.KEYWORDS    byte swapping routines
.LANGUAGE    ANSI C
.PURPOSE     swap bytes of 16, 32 and 64 bit integers
             convert (Sun) floating-point values to local representation

.COMMENTS    - the assembler short/integer byte-swap routine are from 
             the Linux source tree (<linux-tree>/include/asm/byteorder.h>
             - the double byte-swap routine needs long long,
	     but long long is  not  specified  in  ANSI  C  and
             therefore not portable to all architectures.

.ENVIRONment this is an include file with "inline" functions
.VERSION      2.7   15-May-2003	fixed a few typos in byte_swap_u64
              2.6   31-Mar-2003	speed up real conversion
              2.5   31-Mar-2003	modified include for C++ code 
              2.4   31-Aug-2001	ported to the PGI compiler(s), RvH 
              2.3   31-Aug-2001 rewrote float/double conversion routines,
                                to compile with the PGI compiler, RvH
              2.2   29-Aug-2001 moved architecture dependent stuff 
                                to defs_arch.h, RvH
              2.1   19-Feb-1999 use ldexp, which is faster, RvH
              2.0   16-Feb-1999 ported to the DEC-Alpha, RvH
              1.1   12-Feb-1999 made this an include file, RvH
              1.0   06-Feb-1999 created by R.M. van Hees
------------------------------------------------------------*/

#ifndef __SWAP_ROUTINES                       /* Avoid redefinitions */
#define __SWAP_ROUTINES

#ifdef __cplusplus
extern "C" {
#endif

/*+++++ Local Macros and Definitions +++++*/
#define _SWAP_MASK_1in2	((unsigned short) 0xff00U)
#define _SWAP_MASK_2in2	((unsigned short) 0x00ffU)
#define _SWAP_MASK_1in4	0xff000000U
#define _SWAP_MASK_2in4	0x00ff0000U
#define _SWAP_MASK_3in4	0x0000ff00U
#define _SWAP_MASK_4in4	0x000000ffU

                                  /* are long's 32 bit or 64 bit integers */
#if __WORDSIZE == 64
typedef signed   long __s64;                     /* 64-bit signed integer */ 
typedef unsigned long __u64;                   /* 64-bit unsigned integer */ 
#define _SWAP_MASK_1in8	0xff00000000000000UL
#define _SWAP_MASK_2in8	0x00ff000000000000UL
#define _SWAP_MASK_3in8	0x0000ff0000000000UL
#define _SWAP_MASK_4in8	0x000000ff00000000UL
#define _SWAP_MASK_5in8	0x00000000ff000000UL
#define _SWAP_MASK_6in8	0x0000000000ff0000UL
#define _SWAP_MASK_7in8	0x000000000000ff00UL
#define _SWAP_MASK_8in8	0x00000000000000ffUL
#else
typedef signed   long long __s64;                /* 64-bit signed integer */ 
typedef unsigned long long __u64;              /* 64-bit unsigned integer */ 
#define _SWAP_MASK_1in8	0xff00000000000000ULL
#define _SWAP_MASK_2in8	0x00ff000000000000ULL
#define _SWAP_MASK_3in8	0x0000ff0000000000ULL
#define _SWAP_MASK_4in8	0x000000ff00000000ULL
#define _SWAP_MASK_5in8	0x00000000ff000000ULL
#define _SWAP_MASK_6in8	0x0000000000ff0000ULL
#define _SWAP_MASK_7in8	0x000000000000ff00ULL
#define _SWAP_MASK_8in8	0x00000000000000ffULL
#endif

/*++++++++++++++++++++++++++++ FUNCTIONS +++++++++++++++++++++++++++*/

#undef __arch_swap16
#undef __arch_swap32
#undef __arch_swap64

#if defined(__GNUC__)
/*
 * Intel architecture byte-swap routines (GNU-assembler code)
 */
#if defined(__i386)
#define __arch_swap16
#define __arch_swap32
#define __arch_swap64

static inline
unsigned short byte_swap_u16( unsigned short x )
{
     __asm__("xchgb %b0,%h0"                /* swap bytes        */
	     : "=q" (x)
	     :  "0" (x));
     return x;
}

static inline
unsigned int byte_swap_u32( unsigned int x )
{
#ifdef __USE_X86_BSWAP
     __asm__("bswap %0" : "=r" (x) : "0" (x));
#else
     __asm__("xchgb %b0,%h0\n\t"            /* swap lower bytes  */
	     "rorl $16,%0\n\t"              /* swap words        */
	     "xchgb %b0,%h0"                /* swap higher bytes */
	     :"=q" (x)
	     : "0" (x));
#endif
     return x;
}

static inline
__u64 byte_swap_u64( __u64 x )
{
     union { 
	  struct { unsigned int a,b; } s;
	  __u64 u;
     } v;

     v.u = x;
#ifdef __USE_X86_BSWAP
     __asm__("bswapl %0 ; bswapl %1 ; xchgl %0,%1"
	     : "=r" (v.s.a), "=r" (v.s.b)
	     : "0" (v.s.a), "1" (v.s.b));
#else
     v.s.a = byte_swap_u32(v.s.a);
     v.s.b = byte_swap_u32(v.s.b);
     __asm__("xchgl %0,%1" 
	     : "=r" (v.s.a), "=r" (v.s.b) 
	     : "0" (v.s.a), "1" (v.s.b));
#endif
     return v.u;
}

/*
 * PARICS architecture byte-swap routines (GNU-assembler code)
 */
#elif defined(__hp9000s800)
#define __arch_swap16
#define __arch_swap32
#define __arch_swap64

static inline
unsigned short byte_swap_u16( unsigned short x )
{
     __asm__( "dep %0, 15, 8, %0\n\t"         /* deposit 00ab -> 0bab */
	      "shd %r0, %0, 8, %0"            /* shift 000000ab -> 00ba */
	      : "=r" (x)
	      : "0" (x) );
     return x;
}

static inline
unsigned int byte_swap_u32( unsigned int x )
{
     unsigned int temp;
     __asm__( "shd %0, %0, 16, %1\n\t"        /* shift abcdabcd -> cdab */
	      "dep %1, 15, 8, %1\n\t"         /* deposit cdab -> cbab */
	      "shd %0, %1, 8, %0"             /* shift abcdcbab -> dcba */
	      : "=r" (x), "=&r" (temp)
	      : "0" (x) );
     return x;
}

static inline
__u64 byte_swap_u64( __u64 x )
{
     __u64 temp;
     __asm__( "permh 3210, %0, %0\n\t"
	      "hshl %0, 8, %1\n\t"
	      "hshr u, %0, 8, %0\n\t"
	      "or %1, %0, %0"
	      : "=r" (x), "=&r" (temp)
	      : "0" (x) );
     return x;
}

/*
 * DEC Alpha architecture byte-swap routines (GNU-assembler code)
 */
#elif defined(__alpha)
#define __arch_swap16
#define __arch_swap32

static inline
unsigned short byte_swap_u16( unsigned short x )
{
     __u64 t1, t2;

     __asm__(
	  "insbl  %2,1,%1         # %1 = bb00\n\t"
	  "extbl  %2,1,%0         # %0 = 00aa"
	  : "=r"(t1), "=&r"(t2) : "r"(x));

     return t1 | t2;
}

static inline
unsigned int byte_swap_u32( unsigned int x )
{
     __u64 t0, t1, t2, t3;

     __asm__("inslh %1, 7, %0"       /* t0 : 0000000000AABBCC */
	     : "=r"(t0) : "r"(x));
     __asm__("inswl %1, 3, %0"       /* t1 : 000000CCDD000000 */
	     : "=r"(t1) : "r"(x));

     t1 |= t0;                       /* t1 : 000000CCDDAABBCC */
     t2 = t1 >> 16;                  /* t2 : 0000000000CCDDAA */
     t0 = t1 & 0xFF00FF00;           /* t0 : 00000000DD00BB00 */
     t3 = t2 & 0x00FF00FF;           /* t3 : 0000000000CC00AA */
     t1 = t0 + t3;                   /* t1 : ssssssssDDCCBBAA */

     return t1;
}
#endif
#endif  /* __GNUC__ */

#ifndef __arch_swap16
#define __arch_swap16

static inline
unsigned short byte_swap_u16( unsigned short x )
{
     return (unsigned short)(
	  ((x & _SWAP_MASK_2in2) << 8) | ((x & _SWAP_MASK_1in2) >> 8) );
}
#endif

#ifndef __arch_swap32
#define __arch_swap32

static inline
unsigned int byte_swap_u32( unsigned int x )
{
     return ((unsigned int)(
	  ((x & _SWAP_MASK_4in4) << 24) | ((x & _SWAP_MASK_3in4) <<  8) |
	  ((x & _SWAP_MASK_2in4) >>  8) | ((x & _SWAP_MASK_1in4) >> 24) ));
}
#endif

#ifndef __arch_swap64
#define __arch_swap64

static inline
__u64 byte_swap_u64( __u64 x )
{
     return ((__u64)(
	  ((x & _SWAP_MASK_8in8) << 56) | ((x & _SWAP_MASK_7in8) << 40) |
	  ((x & _SWAP_MASK_6in8) << 24) | ((x & _SWAP_MASK_5in8) <<  8) |
	  ((x & _SWAP_MASK_4in8) >>  8) | ((x & _SWAP_MASK_3in8) >> 24) |
	  ((x & _SWAP_MASK_2in8) >> 40) | ((x & _SWAP_MASK_1in8) >> 56) ));
}
#endif

static inline
short byte_swap_16( short x )
{
     return (short) byte_swap_u16( (unsigned short) x );
}

static inline
int byte_swap_32( int x )
{
     return (int) byte_swap_u32( (unsigned int) x );
}


static inline
__s64 byte_swap_64( __s64 x )
{
     return (__s64) byte_swap_u64( (__u64) x );
}

/*--------------------------------------------------
 * Convert Big-Endian ANSI IEEE 754-1985 float representation to the 
 * Little-Endian Intel standard
 */
static inline
unsigned int _Get_Bits( unsigned int uval, 
			unsigned int pos, unsigned int num )
{
     return (uval >> pos) & ~(~0U << num);
}

static inline
void IEEE_Swap__FLT( float *rval )
{
     unsigned int uval;
/*
 * swap bites to Little/Big Endian
 */
     (void) memcpy( &uval, rval, sizeof( int ));
     uval = byte_swap_u32( uval );
     (void) memcpy( rval, &uval, sizeof( int ));
}

/*--------------------------------------------------
 * Convert Big-Endian ANSI IEEE 754-1985 double representation to the 
 * Little-Endian Intel standard
 */
static inline
unsigned long long _Get_LL_Bits( unsigned long long uval, 
				 unsigned int pos, unsigned int num )
{
#if __WORDSIZE == 64
     return (uval >> pos) & ~(~0UL << num);
#else
     return (uval >> pos) & ~(~0ULL << num);
#endif
}

static inline
void IEEE_Swap__DBL( double *dval )
{
     unsigned long long llval;
/*
 * swap bites to Little/Big Endian
 */
     (void) memcpy( &llval, dval, sizeof( long long ));
     llval = byte_swap_u64( llval );
     (void) memcpy( dval, &llval, sizeof( long long ));
}

#ifdef __cplusplus
  }
#endif

#endif /* __SWAP_ROUTINES */
