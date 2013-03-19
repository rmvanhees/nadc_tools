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

.IDENTifer   NADC_FLIP
.AUTHOR      R.M. van Hees
.KEYWORDS    matrix manipulation
.LANGUAGE    ANSI C
.PURPOSE     flip the data of a matrix
.INPUT/OUTPUT
  call as   NADC_FLIPx( flip, npix, xmatrix );

     input:  
            enum nadc_flip flip    :  specifies flip axis
            unsigned int   npix[2] :  dimensions of the matrix
 in/output:
            matrix matrix[2]       :  array to be transformed

.RETURNS     nothing (check global error status)
.COMMENTS    "flip" specifies the operation to be performed, as follows: 
             \hspace*{3ex} 0 \hspace*{2ex} do not change the matrix
	     \hspace*{3ex} 1 \hspace*{2ex} flip the data along the first axis
	     \hspace*{3ex} 2 \hspace*{2ex} flip the data along the second axis
	     \hspace*{3ex} 3 \hspace*{2ex} flip the data along both axis
	     "x" specifies the data type of the input matrix, as follows:
	     \hspace*{3ex} "c" \hspace*{2ex} signed char
	     \hspace*{3ex} "u" \hspace*{2ex} unsigned char
	     \hspace*{3ex} "s" \hspace*{2ex} short
	     \hspace*{3ex} "r" \hspace*{2ex} float
.ENVIRONment None
.VERSION     2.0     31-Oct-2001   moved to new Error handling routines, RvH
             1.2     11-Mar-1999   added FLIPc, RvH
             1.1     30-Jan-1999   added FLIPu, RvH
             1.0     14-Mar-1996   Created by RvHees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void NADC_FLIPc( enum nadc_flip flip, const unsigned int npix[],
		 signed char *matrix )
{
     register unsigned int xx, yy, px;
     register signed char  *pntr;

     signed char  *buff;
     unsigned int nrpix;
   
     const char prognm[] = "NADC_FLIPc";

     if ( flip == NADC_FLIP_NO ) return;
/*
 * make a copy of the input buffer
 */
     nrpix = npix[0] * npix[1];
     buff = (signed char *) malloc( (size_t) nrpix );
     if ( buff == NULL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "buff" );
     (void) memcpy( buff, matrix, (size_t) nrpix );

     switch ( flip ) {
     case NADC_FLIP_X:
	  pntr = buff;

	  yy = 0;
          do { 
	       xx = npix[0]-1;
 	       while ( xx > 0 ) *matrix++ = pntr[--xx];
	  } while ( ++yy < npix[1] );
	  pntr = NULL;
	  break;
     case NADC_FLIP_Y:
	  pntr = buff + (nrpix - npix[0]);

	  yy = 0;
	  do { 
	       xx = 0;
	       do { *matrix++ = pntr[xx]; } while ( ++xx < npix[0] );
	       pntr -= npix[0];
	  } while ( ++yy < npix[1] );
	  pntr = NULL;
	  break;
     case NADC_FLIP_XY:
	  pntr = buff + nrpix;

	  px = 0;
          do { *matrix++ = *(--pntr); } while ( ++px < nrpix );
	  pntr = NULL;
	  break;
     default:
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, "flip" );
     }
 done:
     free( buff );
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void NADC_FLIPu( enum nadc_flip flip, const unsigned int npix[], 
		 unsigned char *matrix )
{
     register unsigned int  xx, yy, px;
     register unsigned char *pntr;

     unsigned char *buff;
     unsigned int  nrpix;
   
     const char prognm[] = "NADC_FLIPu";

     if ( flip == NADC_FLIP_NO ) return;
/*
 * make a copy of the input buffer
 */
     nrpix = npix[0] * npix[1];
     buff = (unsigned char *) malloc( (size_t) nrpix );
     if ( buff == NULL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "buff" );
     (void) memcpy( buff, matrix, (size_t) nrpix );

     switch ( flip ) {
     case NADC_FLIP_X:
	  pntr = buff;

	  yy = 0;
          do { 
	       xx = npix[0]-1;
 	       while ( xx > 0 ) *matrix++ = pntr[--xx];
	  } while ( ++yy < npix[1] );
	  pntr = NULL;
	  break;
     case NADC_FLIP_Y:
	  pntr = buff + (nrpix - npix[0]);

	  yy = 0;
	  do { 
	       xx = 0;
	       do { *matrix++ = pntr[xx]; } while ( ++xx < npix[0] );
	       pntr -= npix[0];
	  } while ( ++yy < npix[1] );
	  pntr = NULL;
	  break;
     case NADC_FLIP_XY:
	  pntr = buff + nrpix;

	  px = 0;
          do { *matrix++ = *(--pntr); } while ( ++px < nrpix );
	  pntr = NULL;
	  break;
     default:
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, "flip" );
     }
 done:
     free( buff );
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++*/
void NADC_FLIPs( enum nadc_flip flip, const unsigned int npix[], short *matrix )
{
     register unsigned int xx, yy, px;
     register short        *pntr;

     short        *buff;
     unsigned int nrpix;

     const char prognm[] = "NADC_FLIPs";

     if ( flip == NADC_FLIP_NO ) return;
/*
 * make a copy of the input buffer
 */
     nrpix = npix[0] * npix[1];
     buff = (short *) malloc( (size_t) nrpix * sizeof( short ));
     if ( buff == NULL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "buff" );
     (void) memcpy( buff, matrix, (size_t) nrpix * sizeof( short ) );

     switch ( flip ) {
     case NADC_FLIP_X:
	  pntr = buff;

	  yy = 0;
          do { 
	       xx = npix[0]-1;
 	       while ( xx > 0 ) *matrix++ = pntr[--xx];
	  } while ( ++yy < npix[1] );
	  pntr = NULL;
	  break;
     case NADC_FLIP_Y:
	  pntr = buff + (nrpix - npix[0]);

	  yy = 0;
	  do { 
	       xx = 0;
	       do { *matrix++ = pntr[xx]; } while ( ++xx < npix[0] );
	       pntr -= npix[0];
	  } while ( ++yy < npix[1] );
	  pntr = NULL;
	  break;
     case NADC_FLIP_XY:
	  pntr = buff + nrpix;

	  px = 0;
          do { *matrix++ = *(--pntr); } while ( ++px < nrpix );
	  pntr = NULL;
	  break;
     default:
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, "flip" );
     }
 done:
     free( buff );
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++*/
void NADC_FLIPr( enum nadc_flip flip, const unsigned int npix[], float *matrix )
{
     register unsigned int xx, yy, px;
     register float        *pntr;

     unsigned int nrpix;
     float        *buff;

     const char prognm[] = "NADC_FLIPr";

     if ( flip == NADC_FLIP_NO ) return;
/*
 * make a copy of the input buffer
 */
     nrpix = npix[0] * npix[1];
     buff = (float *) malloc( (size_t) nrpix * sizeof( float ));
     if ( buff == NULL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "buff" );
     (void) memcpy( buff, matrix, (size_t) nrpix * sizeof( float ) );

     switch ( flip ) {
     case NADC_FLIP_X:
	  pntr = buff;

	  yy = 0;
          do { 
	       xx = npix[0]-1;
 	       while ( xx > 0 ) *matrix++ = pntr[--xx];
	  } while ( ++yy < npix[1] );
	  pntr = NULL;
	  break;
     case NADC_FLIP_Y:
	  pntr = buff + (nrpix - npix[0]);

	  yy = 0;
	  do { 
	       xx = 0;
	       do { *matrix++ = pntr[xx]; } while ( ++xx < npix[0] );
	       pntr -= npix[0];
	  } while ( ++yy < npix[1] );
	  pntr = NULL;
	  break;
     case NADC_FLIP_XY:
	  pntr = buff + nrpix;

	  px = 0;
          do { *matrix++ = *(--pntr); } while ( ++px < nrpix );
	  pntr = NULL;
	  break;
     default:
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, "flip" );
     }
 done:
     free( buff );
}
