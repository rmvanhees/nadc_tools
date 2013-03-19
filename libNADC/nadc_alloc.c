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

.IDENTifer   NADC_ALLOC
.AUTHOR      R.M. van Hees
.KEYWORDS    pointers & memory
.LANGUAGE    ANSI-C
.PURPOSE     pointer testing and memory allocation
.COMMENTS    contains ALLOC_C2D, ALLOC_S2D, ALLOC_I2D, ALLOC_R2D, ALLOC_D2D
                      ALLOC_I3D, ALLOC_I4D,
                      FREE_2D, FREE_3D, and FREE_4D
.ENVIRONment None
.EXTERNALs   None
.VERSION     2.3     30-Nov-2011   fixed bug for ypix==1, RvH
             2.2     05-Dec-2003   added ALLOC_D2D, RvH
             2.1     05-Nov-2001   removed ALLOC_ERROR, RvH
             2.0     31-Oct-2001   moved to new Error handling routines, RvH
             1.5     10-Oct-2001   added list of external (NADC) function, RvH
             1.4     19-Sep-1999   added ALLOC_US2D, RvH
             1.3     09-Aug-1999   added ALLOC_UC2D, RvH
             1.2     21-Aug-1997   added ALLOC_S2D and ALLOC_S3D, RvH
             1.1     21-Mar-1997   added ALLOC_I4D, FREE_2D, 
                                         FREE_3D, and FREE_4D, RvH
             1.0     08-Nov-1996   Created by Richard van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System Headers ++++*/
#include <stdio.h>
#include <stdlib.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*++++++++++++++++++++++++++++ FUNCTIONS +++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   ALLOC_UC2D
.PURPOSE     allocate a unsigned character matrix with dimensions [ypix][xpix]
.INPUT/OUTPUT
  call as    plane = ALLOC_UC2D( ypix, xpix );

     input:
             size_t ypix     :   Y dimension (slowest axis)
             size_t xpix     :   X dimension (fastest axis)

.RETURNS     pointer to allocated memory
.COMMENTS    none
-------------------------*/
unsigned char **ALLOC_UC2D( size_t ypix, size_t xpix )
{
     register size_t ny;

     unsigned char  **plane;

     const char prognm[] = "ALLOC_UC2D";

     plane = (unsigned char **) malloc( ypix * sizeof( unsigned char * ));
     if ( plane == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "plane" );

     plane[0] = (unsigned char *) 
	  malloc( (xpix * ypix) * sizeof( unsigned char ));
     if ( plane[0] == NULL ) {
	  free( plane );
	  plane = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "plane[0]" );
     }
     for ( ny = 1; ny < ypix; ny++ )
	  plane[ny] = plane[ny-1] + xpix;
 done:
     return plane;
}

/*+++++++++++++++++++++++++
.IDENTifer   ALLOC_C2D
.PURPOSE     allocate a character data matrix with dimensions [ypix][xpix]
.INPUT/OUTPUT
  call as    plane = ALLOC_C2D( ypix, xpix );

     input:
             size_t ypix     :   Y dimension (slowest axis)
             size_t xpix     :   X dimension (fastest axis)

.RETURNS     pointer to allocated memory
.COMMENTS    none
-------------------------*/
char **ALLOC_C2D( size_t ypix, size_t xpix )
{
     register size_t ny;

     char  **plane;

     const char prognm[] = "ALLOC_C2D";

     plane = (char **) malloc( ypix * sizeof( char * ));
     if ( plane == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "plane" );

     plane[0] = (char *) malloc( (xpix * ypix) * sizeof( char ));
     if ( plane[0] == NULL ) {
	  free( plane );
	  plane = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "plane[0]" );
     }
     for ( ny = 1; ny < ypix; ny++ )
	  plane[ny] = plane[ny-1] + xpix;
 done:
     return plane;
}

/*+++++++++++++++++++++++++
.IDENTifer   ALLOC_US2D
.PURPOSE     allocate a unsigned short integer data matrix with 
             dimensions [ypix][xpix]
.INPUT/OUTPUT
  call as    plane = ALLOC_US2D( ypix, xpix );

     input:
             size_t ypix     :   Y dimension (slowest axis)
             size_t xpix     :   X dimension (fastest axis)

.RETURNS     pointer to allocated memory
.COMMENTS    none
-------------------------*/
unsigned 
short **ALLOC_US2D( size_t ypix, size_t xpix )
{
     register size_t ny;

     unsigned short  **plane;

     const char prognm[] = "ALLOC_US2D";

     plane = (unsigned short **) malloc( ypix * sizeof( short * ));
     if ( plane == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "plane" );

     plane[0] = (unsigned short *) malloc( (xpix * ypix) * sizeof( short ));
     if ( plane[0] == NULL ) {
	  free( plane );
	  plane = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "plane[0]" );
     }
     for ( ny = 1; ny < ypix; ny++ )
	  plane[ny] = plane[ny-1] + xpix;
 done:
     return plane;
}

/*+++++++++++++++++++++++++
.IDENTifer   ALLOC_S2D
.PURPOSE     allocate a short integer data matrix with dimensions [ypix][xpix]
.INPUT/OUTPUT
  call as    plane = ALLOC_S2D( ypix, xpix );

     input:
             size_t ypix     :   Y dimension (slowest axis)
             size_t xpix     :   X dimension (fastest axis)

.RETURNS     pointer to allocated memory
.COMMENTS    none
-------------------------*/
short **ALLOC_S2D( size_t ypix, size_t xpix )
{
     register size_t ny;

     short  **plane;

     const char prognm[] = "ALLOC_S2D";

     plane = (short **) malloc( ypix * sizeof( short * ));
     if ( plane == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "plane" );

     plane[0] = (short *) malloc( (xpix * ypix) * sizeof( short ));
     if ( plane[0] == NULL ) {
	  free( plane );
	  plane = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "plane[0]" );
     }
     for ( ny = 1; ny < ypix; ny++ )
	  plane[ny] = plane[ny-1] + xpix;
 done:
     return plane;
}

/*+++++++++++++++++++++++++
.IDENTifer   ALLOC_I2D
.PURPOSE     allocate a integer data matrix with dimensions [ypix][xpix]
.INPUT/OUTPUT
  call as    plane = ALLOC_I2D( ypix, xpix );

     input:
             size_t ypix     :   Y dimension (slowest axis)
             size_t xpix     :   X dimension (fastest axis)

.RETURNS     pointer to allocated memory
.COMMENTS    none
-------------------------*/
int **ALLOC_I2D( size_t ypix, size_t xpix )
{
     register size_t ny;

     int  **plane;

     const char prognm[] = "ALLOC_I2D";

     plane = (int **) malloc( ypix * sizeof( int * ));
     if ( plane == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "plane" );

     plane[0] = (int *) malloc( (xpix * ypix) * sizeof( int ));
     if ( plane[0] == NULL ) {
	  free( plane );
	  plane = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "plane[0]" );
     }
     for ( ny = 1; ny < ypix; ny++ )
	  plane[ny] = plane[ny-1] + xpix;
 done:
     return plane;
}

/*+++++++++++++++++++++++++
.IDENTifer   ALLOC_R2D
.PURPOSE     allocate a float data matrix with dimensions [ypix][xpix]
.INPUT/OUTPUT
  call as    plane = ALLOC_R2D( ypix, xpix );

     input:
             size_t ypix     :   Y dimension (slowest axis)
             size_t xpix     :   X dimension (fastest axis)

.RETURNS     pointer to allocated memory
.COMMENTS    none
-------------------------*/
float **ALLOC_R2D( size_t ypix, size_t xpix )
{
     register size_t ny;

     float **plane;
 
     const char prognm[] = "ALLOC_R2D";

     plane = (float **) malloc( ypix * sizeof( float * ));
     if ( plane == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "plane" );

     plane[0] = (float *) malloc( (xpix * ypix) * sizeof( float ));
     if ( plane[0] == NULL ) {
	  free( plane );
	  plane = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "plane[0]" );
     }
     for ( ny = 1; ny < ypix; ny++ )
	  plane[ny] = plane[ny-1] + xpix;
 done:
     return plane;
}

/*+++++++++++++++++++++++++
.IDENTifer   ALLOC_D2D
.PURPOSE     allocate a double data matrix with dimensions [ypix][xpix]
.INPUT/OUTPUT
  call as    plane = ALLOC_D2D( ypix, xpix );

     input:
             size_t ypix     :   Y dimension (slowest axis)
             size_t xpix     :   X dimension (fastest axis)

.RETURNS     pointer to allocated memory
.COMMENTS    none
-------------------------*/
double **ALLOC_D2D( size_t ypix, size_t xpix )
{
     register size_t ny;

     double **plane;
 
     const char prognm[] = "ALLOC_D2D";

     plane = (double **) malloc( ypix * sizeof( double * ));
     if ( plane == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "plane" );

     plane[0] = (double *) malloc( (xpix * ypix) * sizeof( double ));
     if ( plane[0] == NULL ) {
	  free( plane );
	  plane = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "plane[0]" );
     }
     for ( ny = 1; ny < ypix; ny++ )
	  plane[ny] = plane[ny-1] + xpix;
 done:
     return plane;
}

/*+++++++++++++++++++++++++
.IDENTifer   ALLOC_S3D
.PURPOSE     allocate a short integer data cube with dimensions 
             [zpix][ypix][xpix]
.INPUT/OUTPUT
  call as    cube = ALLOC_I3D( zpix, ypix, xpix );

     input:
             size_t zpix     :   Z dimension (slowest axis)
             size_t ypix     :   Y dimension
             size_t xpix     :   X dimension (fastest axis)

.RETURNS     pointer to allocated memory
.COMMENTS    none
-------------------------*/
short ***ALLOC_S3D( size_t zpix, size_t ypix, size_t xpix )
{
     register size_t ny, nz;

     short    ***cube;

     const char   prognm[] = "ALLOC_S3D";
     const size_t xypix    = xpix * ypix;

     cube = (short ***) malloc( zpix * sizeof( short ** ));
     if ( cube == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "cube" );

     cube[0] = (short **) malloc( (zpix * ypix) * sizeof( short * ));
     if ( cube[0] == NULL ) {
	  free( cube );
	  cube = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "cube[0]" );
     }
     cube[0][0] = (short *) malloc( (zpix * xypix) * sizeof( short ));
     if ( cube[0][0] == NULL ) {
	  free( cube[0] );
	  free( cube );
	  cube = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "cube[0][0]" );
     }
     for ( nz = 1; nz < zpix; nz++ ) { 
	  cube[nz] = cube[nz-1] + ypix;
	  cube[nz][0] = cube[nz-1][0] + xypix;
     }

     nz = 0;
     do { 
	  for ( ny = 1; ny < ypix; ny++ ) 
	       cube[nz][ny] = cube[nz][ny-1] + xpix;
     } while ( ++nz < zpix );
 done:
     return cube;
}

/*+++++++++++++++++++++++++
.IDENTifer   ALLOC_I3D
.PURPOSE     allocate a integer data cube with dimensions [zpix][ypix][xpix]
.INPUT/OUTPUT
  call as    cube = ALLOC_I3D( zpix, ypix, xpix );

     input:
             size_t zpix     :   Z dimension (slowest axis)
             size_t ypix     :   Y dimension
             size_t xpix     :   X dimension (fastest axis)

.RETURNS     pointer to allocated memory
.COMMENTS    none
-------------------------*/
int ***ALLOC_I3D( size_t zpix, size_t ypix, size_t xpix )
{
     register size_t ny, nz;

     int    ***cube;

     const char   prognm[] = "ALLOC_I3D";
     const size_t xypix    = xpix * ypix;

     cube = (int ***) malloc( zpix * sizeof( int ** ));
     if ( cube == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "cube" );

     cube[0] = (int **) malloc( (zpix * ypix) * sizeof( int * ));
     if ( cube[0] == NULL ) {
	  free( cube );
	  cube = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "cube[0]" );
     }
     cube[0][0] = (int *) malloc( (zpix * xypix) * sizeof( int ));
     if ( cube[0][0] == NULL ) {
	  free( cube[0] );
	  free( cube );
	  cube = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "cube[0][0]" );
     }
     for ( nz = 1; nz < zpix; nz++ ) { 
	  cube[nz] = cube[nz-1] + ypix;
	  cube[nz][0] = cube[nz-1][0] + xypix;
     }
     nz = 0;
     do { 
	  for ( ny = 1; ny < ypix; ny++ ) 
	       cube[nz][ny] = cube[nz][ny-1] + xpix;
     } while ( ++nz < zpix );
     nz = 1;
     do { 
	  cube[nz] = cube[nz-1] + ypix;
	  cube[nz][0] = cube[nz-1][0] + xypix;
     } while ( ++nz < zpix );
 done:
     return cube;
}

/*+++++++++++++++++++++++++
.IDENTifer   ALLOC_I4D
.PURPOSE     allocate a integer data cube with dimensions 
                      [zpix][ypix][xpix][wpix]
.INPUT/OUTPUT
  call as    cube = ALLOC_I4D( zpix, ypix, xpix, wpix );

     input:
             size_t zpix     :   Z dimension (slowest axis)
             size_t ypix     :   Y dimension
             size_t xpix     :   X dimension
             size_t wpix     :   W dimension (fastest axis) 

.RETURNS     pointer to allocated memory
.COMMENTS    none
-------------------------*/
int ****ALLOC_I4D( size_t zpix, size_t ypix, size_t xpix, size_t wpix )
{
     register size_t nx, ny, nz;

     int  ****cube;

     const char   prognm[] = "ALLOC_I4D";
     const size_t wxpix    = wpix * xpix,
	  xypix    = xpix * ypix,
	  wxypix   = wpix * xypix;

     cube = (int ****) malloc( zpix * sizeof( int *** ));
     if ( cube == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "cube" );

     cube[0] = (int ***) malloc( (zpix * ypix) * sizeof( int ** ));
     if ( cube[0] == NULL ) {
	  free( cube );
	  cube = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "cube[0]" );
     }
     cube[0][0] = (int **) malloc( (zpix * xypix) * sizeof( int * ));
     if ( cube[0][0] == NULL ) {
	  free( cube[0] );
	  free( cube );
	  cube = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "cube[0][0]" );
     }
     cube[0][0][0] = (int *) malloc( (zpix * wxypix) * sizeof( int ));
     if ( cube[0][0][0] == NULL )  {
	  free( cube[0][0] );
	  free( cube[0] );
	  free( cube );
	  cube = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "cube[0][0][0]" );
     }
     nz = 1;
     do { 
	  cube[nz] = cube[nz-1] + ypix;
	  cube[nz][0] = cube[nz-1][0] + xypix;
	  cube[nz][0][0] = cube[nz-1][0][0] + wxypix;
     } while ( ++nz < zpix );

     nz = 0;
     do { 
	  nx = 1;
	  do 
	       cube[nz][0][nx] = cube[nz][0][nx-1] + wpix;
	  while ( ++nx < xpix );

	  ny = 1; 
	  do { 
	       cube[nz][ny] = cube[nz][ny-1] + xpix; 
	       cube[nz][ny][0] = cube[nz][ny-1][0] + wxpix; 

	       nx = 1;
	       do 
		    cube[nz][ny][nx] = cube[nz][ny][nx-1] + wpix;
	       while ( ++nx < xpix );
	  } while ( ++ny < ypix );
     } while ( ++nz < zpix );
 done:
     return cube;
}

/*+++++++++++++++++++++++++
.IDENTifer   FREE_2D
.PURPOSE     free memory allocated with ALLOC_x2D
.INPUT/OUTPUT
  call as     FREE_2D( plane );
     input:
             void **plane : memory allocated with ALLOC_x2D

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void FREE_2D( void **plane )
{
     free( plane[0] );
     free( plane );
}

/*+++++++++++++++++++++++++
.IDENTifer   FREE_3D
.PURPOSE     free memory allocated with ALLOC_x3D
.INPUT/OUTPUT
  call as     FREE_2D( plane );
     input:
             void ***cube : memory allocated with ALLOC_x3D

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void FREE_3D( void ***cube )
{
     free( cube[0][0] );
     free( cube[0] );
     free( cube );
}

/*+++++++++++++++++++++++++
.IDENTifer   FREE_4D
.PURPOSE     free memory allocated with ALLOC_x4D
.INPUT/OUTPUT
  call as     FREE_4D( cube );
     input:
             void ****cube :  memory allocated with ALLOC_x4D

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void FREE_4D( void ****cube )
{
     free( cube[0][0][0] );
     free( cube[0][0] );
     free( cube[0] );
     free( cube );
}
