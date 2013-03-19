/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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

.IDENTifer   NADC_USRINP
.AUTHOR      R.M. van Hees
.KEYWORDS    string convertion
.LANGUAGE    ANSI-C
.PURPOSE     Decode a character string into an array with elements of
             the requested data type 
.INPUT/OUTPUT
  call as    NADC_USRINP( type, string, maxval, pntr, nrval );

        input: int  type     requested type of array elements in pntr,
                             valid values are: UINT8_T, INT16_T, UINT16_T,
                             INT32_T, UINT32_T, FLT32_T, FLT_64_T
               char *string  character string
               int  maxval   maximum number of elements in the output array
       output: void *pntr    void pointer to a pointer of requested type
               int  *nrval   actuel number of values found

.RETURNS     nothing (check global error status)
.COMMENTS    The input-string has to contain values separated by "," or ":".
             The values in the output array are converted to requested type
	       input:         string = "1,4,8"
               \hspace*{7ex}  type = FLT32_T         
               returns:       pointer to 1.0 4.0 8.0
               \hspace*{9ex}  nrval = 3

	       input:         string = ".6:2:.4"
               \hspace*{7ex}  type = FLT32_T or FLT64_T 
	       returns:       pointer to 0.6 1.0 1.4 1.8
               \hspace*{9ex}  nrval = 4

.ENVIRONment none
.VERSION      4.2   13-Dec-2002	removed the usage of ctype, use enumerated
                                elements instead, RvH
              4.1   21-Dec-2001	Added handling of unsigned char, RvH
              4.0   31-Oct-2001	moved to new Error handling routines, RvH 
              3.2   25-Oct-2001 rename modules to NADC_..., RvH
              3.1   06-Sep-2001 added ctype "h" for unsigned short, RvH
              3.0   22-Feb-1999 removed buffer overrun error,
                                  default step=1, RvH
              2.1   15-Sep-1996 Changes as suggested by LClint, RvH
              2.0   28-Sep-1994 Improved original from ESO, RvH
              1.1   23-Mar-1994 Updated the documentation,  RvH
              1.0   27-Jun-1993 Created by R.M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System Headers +++++*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++ Macros & file-scope variables +++++*/
#define NR_CMVAL        3

/*+++++++++++++++++++++++++ FUNCTIONS +++++++++++++++++++++++++*/
void NADC_USRINP( int type, const char *string, int maxval, 
		  void *pntr, int *nrval )
{
     register int  ii;

     int    nr, nrstep;
     char   *str_bgn, *str_ptr, *pntcm, *pnt2d, cval[21], *cmval[NR_CMVAL];
     double dmval[NR_CMVAL];

     union u_buff {
	  unsigned char ucval;
	  short  sval;
	  unsigned short usval;
	  int    ival;
	  unsigned int uval;
	  float  rval;
	  double dval; 
     } *uu;

     const char prognm[] = "NADC_USRINP";

     uu = (union u_buff *) malloc( (size_t) maxval * sizeof( union u_buff )); 
     if ( uu == NULL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "uu" );
/*
 * Initialisation
 */
     *nrval = 0;
     nr = 0;
     do {
	  cmval[nr] = (char *) malloc( strlen( string ) );
	  if ( cmval[nr] == NULL ) {
	       free( uu );
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "cmval" );
	  }
     } while ( ++nr < NR_CMVAL );
/*
 * Copy only non-blanks from input string
 */
     str_bgn = (char *) malloc( strlen( string ) + 1 );
     if ( (str_ptr = str_bgn) == NULL ) {
	  free( uu );
	  for ( nr = 0; nr < NR_CMVAL; nr++ ) free( cmval[nr] );
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "str_bgn" );
     }
     (void) strcpy( str_ptr, string );
     (void) strtok( str_ptr, " " );
     while( strlen( str_ptr ) > (size_t) 0 && *nrval < maxval ) {
	  nr = 0;
	  pntcm = strchr( str_ptr, ',' );
	  pnt2d = strchr( str_ptr, ':' );
/*
 * Does the string contain zero or more characters and no "," or ":" ?
 *   or does it contain a "," before a ":" ?
 */
	  if ( pnt2d == NULL || (pntcm != NULL && pntcm < pnt2d) ) {
	       if ( pntcm == NULL ) {
		    (void) strcpy( cval, str_ptr );
		    *str_ptr = '\0';
	       } else { 
		    (void) strlcpy( cval, str_ptr, 
				    (size_t)(pntcm - str_ptr)+1 );
		    str_ptr = pntcm + 1;
	       }
	       switch( type ) {
	       case UINT8_T:
		    uu[*nrval].ucval = (unsigned char) atoi( cval );
		    break;
	       case INT16_T:
		    uu[*nrval].sval = (short) atoi( cval );
		    break;
	       case UINT16_T:
		    uu[*nrval].usval = (unsigned short) atoi( cval );
		    break;
               case INT32_T:
		    uu[*nrval].ival = atoi( cval );
		    break;
               case UINT32_T: 
		    uu[*nrval].uval = (unsigned int) atol( cval );
		    break;
               case FLT32_T: 
		    uu[*nrval].rval = (float) atof( cval );
		    break;
               case FLT64_T:
		    uu[*nrval].dval = atof( cval );
		    break;
	       }
	       (*nrval)++;
	  } else {                                            /*store CSTART*/
/*
 * It contains a ":" before a "," !
 */
	       (void) strlcpy( cmval[nr], str_ptr, 
			       (size_t)(pnt2d - str_ptr)+1 );
	       nr++;
	       str_ptr = pnt2d + 1;
	       pnt2d = strchr( str_ptr, ':' );
/*
 * There are two cases after a:b:c there is no futher input: pntcm == NULL
 * or there is more something like a:b:c,d...
 */
	       if ( pntcm == NULL ) {
		    if ( pnt2d != NULL ) {                        /*get CEND*/
			 (void) strlcpy( cmval[nr], str_ptr, 
					 (size_t)(pnt2d - str_ptr)+1 );
			 nr++;
			 str_ptr = pnt2d + 1;
			 pnt2d = strchr( str_ptr, ':' );
			 if ( pnt2d == NULL ) {                /*store CINCR*/
			      (void) strcpy( cmval[nr], str_ptr );
			      *str_ptr = '\0';
			 } else {                  /*wrong syntax a:b:c:d...*/
			      NADC_RETURN_ERROR( prognm, NADC_ERR_PARAM,
					       "a:b:c:d..." );
			 }
		    } else {                                 /*set CINCR = 1*/
			 (void) strcpy( cmval[nr++], str_ptr );
			 (void) strcpy( cmval[nr], "1" );
			 *str_ptr = '\0';
		    }
	       } else {
/*
 * The string still contains a "," 
 */
		    if ( pnt2d != NULL && pnt2d < pntcm ) {       /*get CEND*/
			 (void) strlcpy( cmval[nr], str_ptr, 
					 (size_t)(pnt2d - str_ptr)+1 );
			 nr++;
			 str_ptr = pnt2d + 1;
			 pnt2d = strchr( str_ptr, ':' );
		    } else {                                 /*set CINCR = 1*/
			 (void) strlcpy( cmval[nr], str_ptr,
					 (size_t)(pntcm - str_ptr)+1 );
			 nr++;
			 (void) strcpy( cmval[nr], "1" );
		    }
		    if ( pnt2d != NULL && pnt2d < pntcm ) {
			      NADC_RETURN_ERROR( prognm, NADC_ERR_PARAM,
					       "a:b:c:d,.." );
		    } else {                                     /*get CINCR*/
			 (void) strlcpy( cmval[nr], str_ptr, 
					 (size_t) (pntcm - str_ptr)+1 );
			 nr++;
			 str_ptr = pntcm + 1;
		    }
	       }
/*
 * We've found CSTART, CEND and CINCR
 */
	       dmval[0] = atof( cmval[0] );
	       dmval[1] = atof( cmval[1] );
	       dmval[2] = atof( cmval[2] );
	       if ( dmval[2] >= -DBL_EPSILON && dmval[2] <= DBL_EPSILON ) 
		    dmval[2] = 1.0;
	       else if ( dmval[2] > DBL_EPSILON &&
			 (dmval[0] - dmval[1]) >= DBL_EPSILON )
		    dmval[2] *= -1.;
	       else if ( dmval[2] < DBL_EPSILON &&
			 (dmval[1] - dmval[0]) >= DBL_EPSILON )
		    dmval[2] *= -1.;
	       nrstep = abs( (int)((dmval[0] - dmval[1])/dmval[2]) ) + 1;

	       if ( nrstep + *nrval > maxval ) {
		    NADC_ERROR( prognm, NADC_ERR_PARAM, 
				"exceed array (truncated)" );
		    nrstep = maxval - *nrval;
	       }

	       switch( type ) {
	       case UINT8_T:
		    ii = 0;
		    do { 
			 uu[*nrval].ucval = 
			      (unsigned char) (dmval[0] + ii * dmval[2]);
			 (*nrval)++;
		    } while ( ++ii < nrstep );
		    break;
	       case INT16_T:
		    ii = 0;
		    do { 
			 uu[*nrval].sval = 
			      (short) (dmval[0] + ii * dmval[2]);
			 (*nrval)++;
		    } while ( ++ii < nrstep );
		    break;
	       case UINT16_T:
		    ii = 0;
		    do { 
			 uu[*nrval].usval = 
			      (unsigned short) (dmval[0] + ii * dmval[2]);
			 (*nrval)++;
		    } while ( ++ii < nrstep );
		    break;
	       case INT32_T:
		    ii = 0;
		    do { 
			 uu[*nrval].ival = 
			      (int) (dmval[0] + ii * dmval[2]);
			 (*nrval)++;
		    } while ( ++ii < nrstep );
		    break;
	       case UINT32_T: 
		    ii = 0;
		    do { 
			 uu[*nrval].uval = 
			      (unsigned int) (dmval[0] + ii * dmval[2]);
			 (*nrval)++;
		    } while ( ++ii < nrstep );
		    break;
	       case FLT32_T: 
		    ii = 0;
		    do { 
			 uu[*nrval].rval = 
			      (float) (dmval[0] + ii * dmval[2]);
			 (*nrval)++;
		    } while ( ++ii < nrstep );
		    break;
	       default:
		    ii = 0;
		    do { 
			 uu[*nrval].dval = dmval[0] + ii * dmval[2];
			 (*nrval)++;
		    } while ( ++ii < nrstep );
		    break;
	       }
	  }
     }
/*
 * Fill the output array
 */
     switch( type ) {
     case UINT8_T: {
	  unsigned char *buff = (unsigned char *) pntr;
	  ii = 0;
	  do { buff[ii] = uu[ii].ucval; } while ( ++ii < *nrval );
	  break;
     }
     case INT16_T: {
	  short *buff = (short *) pntr;
	  ii = 0;
	  do { buff[ii] = uu[ii].sval; } while ( ++ii < *nrval );
	  break;
     }
     case UINT16_T: {
	  unsigned short *buff = (unsigned short *) pntr;
	  ii = 0;
	  do { buff[ii] = uu[ii].usval; } while ( ++ii < *nrval );
	  break;
     }
     case INT32_T: {
	  int *buff = (int *) pntr;
	  ii = 0;
	  do { buff[ii] = uu[ii].ival; } while ( ++ii < *nrval );
	  break;
     }
     case UINT32_T: {
	  unsigned int *buff = (unsigned int *) pntr;
	  ii = 0;
	  do { buff[ii] = uu[ii].uval; } while ( ++ii < *nrval );
	  break;
     }
     case FLT32_T: {
	  float *buff = (float *) pntr;
	  ii = 0;
	  do { buff[ii] = uu[ii].rval; } while ( ++ii < *nrval );
	  break;
     }
     default: {
	  double *buff = (double *) pntr;
	  ii = 0;
	  do { buff[ii] = uu[ii].dval; } while ( ++ii < *nrval );
	  break;
     }}
/*
 * free allocated memory
 */
     if ( str_bgn != NULL ) free( str_bgn );
     free( uu );
     for ( nr = 0; nr < NR_CMVAL; nr++ ) free( cmval[nr] );
}
