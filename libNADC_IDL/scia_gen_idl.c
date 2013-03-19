/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_GEN_IDL
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY, IDL
.LANGUAGE    ANSI C
.PURPOSE     IDL wrapper for reading SCIAMACHY data (general)
.COMMENTS    None
.ENVIRONment None
.VERSION      1.3   25-Sep-2009	added get_scia_quality, RvH
              1.2   12-Oct-2002	consistently return, in case of error, -1, RvH 
              1.1   02-Jul-2002	added more error checking, RvH
              1.0   15-Jan-2002	Created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define POSIX to indicate
 * that this is a POSIX program
 */
#define  POSIX 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#define _SCIA_COMMON
#include <nadc_idl.h>

/*+++++ Global Variables +++++*/
FILE *fd_nadc = NULL;

/* 
 * Most routines to read SCIAMACHY data can allocate memory internally
 * However IDL requires the use of their own memory allocation routines
 */
bool Use_Extern_Alloc = TRUE;

/*+++++ Static Variables +++++*/
static bool File_Is_Open = FALSE;
static const char err_msg[] = "invalid number of function arguments";

/*+++++++++++++++++++++++++ Main Program or Functions +++++++++++++++*/
int IDL_STDCALL OpenFile( int argc, void *argv[] )
{
     const char prognm[] = "OpenFile";

     IDL_STRING *str_descr;

     if ( argc != 1 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     str_descr = (IDL_STRING *) argv[0];

     if ( File_Is_Open ) (void) fclose( fd_nadc );

     if ((fd_nadc = fopen( str_descr[0].s, "r" )) == NULL ) {
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, strerror( errno ) );
     } else
	  File_Is_Open = TRUE;

     return 0;
 done:
     return -1;
}

int IDL_STDCALL  CloseFile( void )
{
     int stat = 0;

     if ( File_Is_Open ) {
	  stat = fclose( fd_nadc );
	  File_Is_Open = FALSE;
     }
     return stat;
}

int IDL_STDCALL  Err_Clear( void )
{
     NADC_Err_Clear();
     return 0;
}

int IDL_STDCALL Err_Trace( int argc, void *argv[] )
{
     const char prognm[] = "Err_Trace";

     int fd;

     FILE *fd_log;

     IDL_STRING *log_name;

     if ( argc != 1 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     log_name = (IDL_STRING *) argv[0];
/*
 * create temporary file
 */
     fd = mkstemp( log_name[0].s );
     close( fd );

     fd_log = fopen( log_name[0].s, "a" );
     NADC_Err_Trace( fd_log );
     (void) fclose( fd_log );

     NADC_Err_Clear();
     return 0;
 done:
     return -1;
}

/* #include "../libSCIA/Inline/Y_interpol.inc" */

/* int IDL_STDCALL _NADC_LIN_INTERPOL( int argc, void *argv[] ) */
/* { */
/*      const char prognm[] = "_NADC_LIN_INTERPOL"; */

/*      unsigned int ynum; */

/*      float xval, x_left, x_right; */
/*      float *yval, *y_left, *y_right; */

/*      if ( argc != 7 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg ); */
/*      xval    = *(float *) argv[0]; */
/*      x_left  = *(float *) argv[1]; */
/*      x_right = *(float *) argv[2]; */
/*      ynum    = *(unsigned int *) argv[3]; */
/*      y_left  = (float *) argv[4]; */
/*      y_right = (float *) argv[5]; */
/*      yval    = (float *) argv[6]; */

/*      Y_LinInterPol( xval, x_left, x_right, ynum, y_left, y_right, yval ); */
/*      return 0; */
/*  done:	        */
/*      return -1;      */
/* } */

unsigned int IDL_STDCALL _SCIA_SET_CALIB( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_SET_CALIB";

     IDL_STRING *calib_str;
     unsigned int calib_mask;

     if ( argc != 1 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     calib_str = (IDL_STRING *) argv[0];

     if ( calib_str[0].slen == 0 ) return 0u;

     SCIA_SET_CALIB( calib_str[0].s, &calib_mask );
     return calib_mask;
 done:
     return 0u;
}

int IDL_STDCALL _ENVI_RD_MPH( int argc, void *argv[] )
{
     const char prognm[] = "_ENVI_RD_MPH";

     struct mph_envi *mph;

     if ( argc != 1 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( ! File_Is_Open ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     mph = (struct mph_envi *) argv[0];
     ENVI_RD_MPH( fd_nadc, mph );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return 1;
 done:
     return -1;
}

int IDL_STDCALL _ENVI_RD_DSD( int argc, void *argv[] )
{
     const char prognm[] = "_ENVI_RD_DSD";

     int nr_dsd = 0;

     struct mph_envi  mph;
     struct dsd_envi  *dsd;

     if ( argc != 2 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( ! File_Is_Open ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     mph = *(struct mph_envi *) argv[0];
     dsd = (struct dsd_envi *) argv[1];

     nr_dsd = (int) ENVI_RD_DSD( fd_nadc, mph, dsd );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_dsd;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_RD_LADS( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_RD_LADS";

     int nr_lads;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct lads_scia *lads;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( ! File_Is_Open ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     lads = (struct lads_scia *) argv[2];

     nr_lads = (int) SCIA_RD_LADS( fd_nadc, num_dsd, dsd, &lads );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_lads;
 done:
     return -1;
}

int IDL_STDCALL _GET_SCIA_ROE_ORBITPHASE( int argc, void *argv[] )
{
     const char prognm[] = "_GET_SCIA_ROE_ORBITPHASE";

     register int nm;

     bool   eclipseMode, saaFlag;
     int    num, orbit;
     float  *orbitPhase;
     double *julianDay;

     if ( argc != 4 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     
     eclipseMode = *(bool *) argv[0];
     num         = *(int *) argv[1];
     julianDay   = (double *) argv[2];
     orbitPhase  = (float *) argv[3];

     for ( nm = 0; nm < num; nm++ )
	  GET_SCIA_ROE_INFO( eclipseMode, julianDay[nm],
			     &orbit, &saaFlag, orbitPhase+nm );

     return 0;
done:
     return -1;
}

int IDL_STDCALL _GET_SCIA_ROE_INFO( int argc, void *argv[] )
{
     const char prognm[] = "_GET_SCIA_ROE_INFO";

     bool   eclipseMode;
     double julianDay;

     bool   *saaFlag;
     int    *absOrbit;
     float  *orbitPhase;

     if ( argc != 5 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     
     eclipseMode = *(bool *) argv[0];
     julianDay   = *(double *) argv[1];
     absOrbit    = (int *) argv[2];
     saaFlag     = (bool *) argv[3];
     orbitPhase  = (float *) argv[4];

     GET_SCIA_ROE_INFO( eclipseMode, julianDay,
			absOrbit, saaFlag, orbitPhase );
     return 0;
done:
     return -1;
}

unsigned short IDL_STDCALL _GET_SCIA_QUALITY( int argc, void *argv[] )
{
     const char prognm[] = "_GET_SCIA_QUALITY";

     int orbit, *range;

     if ( argc != 2 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     orbit  = *(int *) argv[0];
     range  = (int *) argv[1];

     return GET_SCIA_QUALITY( orbit, range );
done:
     return 0xFFFFU;
}
