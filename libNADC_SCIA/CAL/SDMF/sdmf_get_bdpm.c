/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2012 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SDMF_get_BDPM
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF - Bad Dead Pixel Mask
.LANGUAGE    ANSI C
.PURPOSE     read Dead/Bad pixels mask from SDMF databases
.COMMENTS    contains SDMF_get_BDPM_24 and SDMF_get_BDPM_30
.ENVIRONment None
.VERSION      1.0   20-May-2012 initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>

/*+++++ Local Headers +++++*/
#include <nadc_sdmf.h>

/*+++++ Macros +++++*/
        /* NONE */

/*+++++ Static Variables +++++*/
        /* NONE */

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++++++++++++++++++++++ SDMF version 2.4 ++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_BDPM_24
.PURPOSE     obtain bad/dead pixel mask from SRON Monitoring database (v2.4)
.INPUT/OUTPUT
  call as    SDMF_get_BDPM_24( orbit, bdpm );
     input:
           unsigned short absOrbit  :  absolute orbitnumber
    output:
           unsigned char *bdpm      :  bad/dead pixel mask

.RETURNS     flag: FALSE (no mask found) or TRUE
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
bool SDMF_get_BDPM_24( unsigned short absOrbit, /*@out@*/ unsigned char *bdpm )
{
     register unsigned short nr = 0;

     char str_buf[SHORT_STRING_LENGTH];
     char bdpm_fl[MAX_STRING_LENGTH];

     unsigned short db;
     FILE           *db_fp;
/*
 * initialise output arrays
 */
     (void) memset( bdpm, 0, (size_t) SCIENCE_PIXELS  );
/*
 * find file with dead/bad pixel mask, requirements:
 *  - orbit number within a range MAX_DiffOrbitNumber
 */
     if ( ! SDMF_get_fileEntry( SDMF24_BDPM, (int) absOrbit, bdpm_fl ) ) 
	  return FALSE;
/*
 * read bad-dead pixel flags
 */
     if ( (db_fp = fopen( bdpm_fl, "r" )) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE, bdpm_fl );

     while ( fgets(str_buf, SHORT_STRING_LENGTH, db_fp) != NULL ){
	  if ( *str_buf != '#' && nr < SCIENCE_PIXELS ) {
	       (void) sscanf( str_buf, "%1hu", &db );
	       bdpm[nr++] = (unsigned char) db;
	  }
     }
     (void) fclose( db_fp );

     return TRUE;
 done:
     return FALSE;
}

/*+++++++++++++++++++++++++ SDMF version 3.0 ++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_BDPM_30
.PURPOSE     obtain bad/dead pixel mask from SRON Monitoring database (v3.0)
.INPUT/OUTPUT
  call as    SDMF_get_BDPM_30( absOrbit, bdpm );
     input:
           unsigned short absOrbit  :  absolute orbitnumber
    output:
           unsigned char *bdpm      :  bad/dead pixel mask

.RETURNS     flag: FALSE (no mask found) or TRUE
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
bool SDMF_get_BDPM_30( unsigned short absOrbit, /*@out@*/ unsigned char *bdpm )
{
     const int  MAX_DiffOrbitNumber = 14;
     const int  orbit = (int) absOrbit;

     const char msg_found[] =
          "\n\tapplied SDMF Bad Dead pixel mask (v3.0) of Orbit: %-d";
     const char msg_notfound[] =
          "\n\tno applicable Bad Dead pixel mask (v3.0) found for Orbit: %-d";

     register int delta = 0;

     char   str_msg[SHORT_STRING_LENGTH];
     char   sdmf_db[MAX_STRING_LENGTH];

     bool   found = FALSE;

     int    numIndx, metaIndx;

     hid_t  fid = -1;
     hid_t  gid = -1;
/*
 * initialise output arrays
 */
     (void) memset( bdpm, 0, (size_t) SCIENCE_PIXELS );
/*
 * open SDMF PixelMask database
 */
     (void) snprintf( sdmf_db, MAX_STRING_LENGTH, "%s/%s", 
                      SDMF_PATH("3.0"), "sdmf_pixelmask.h5" );
     H5E_BEGIN_TRY {
	  fid = H5Fopen( sdmf_db, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( fid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_FILE, sdmf_db );

     if ( (gid = H5Gopen( fid, "/smoothMask", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, "/smoothMask" );
/*
 * find masker with dead/bad pixel mask, requirements:
 *  - orbit number within a range MAX_DiffOrbitNumber
 */
     metaIndx = -1;
     do {
          numIndx = 1;
          (void) SDMF_get_metaIndex( gid, orbit + delta, &numIndx, &metaIndx );
          if ( IS_ERR_STAT_FATAL )
               NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_get_metaIndex" );
          if ( numIndx > 0 ) {
               found = TRUE;
          } else {
               delta = (delta > 0) ? (-delta) : (1 - delta);
               if ( abs( delta ) > MAX_DiffOrbitNumber ) {
		    (void) snprintf( str_msg, SHORT_STRING_LENGTH, 
				     msg_notfound, orbit );
		    NADC_GOTO_ERROR( NADC_SDMF_ABSENT, str_msg );
	       }
          }
     } while ( ! found );

     /* Oeps, mask is stored using short integers, conversion on the fly! */
     SDMF_rd_uint8_Array( gid, "combined", 1, &metaIndx, NULL, bdpm );

     (void) snprintf( str_msg, SHORT_STRING_LENGTH, msg_found, orbit + delta );
     NADC_ERROR( NADC_ERR_NONE, str_msg );
/*
 * close SDMF pixelMask database
 */
 done:
     if ( gid != -1 ) (void) H5Gclose( gid );
     if ( fid != -1 ) (void) H5Fclose( fid );

     return found;
}

/*+++++++++++++++++++++++++ SDMF version 3.1 ++++++++++++++++++++++++++++++*/
                         /* not available, yet */
/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
#ifdef TEST_PROG
bool Use_Extern_Alloc = FALSE;

int main( int argc, char *argv[] )
{
     register unsigned short np = 0;

     unsigned short orbit;

     bool fnd_24, fnd_30;
     unsigned char bdpm_24[SCIENCE_PIXELS], bdpm_30[SCIENCE_PIXELS];
/*
 * initialization of command-line parameters
 */
     if ( argc == 1 || (argc > 1 && strncmp( argv[1], "-h", 2 ) == 0) ) {
          (void) fprintf( stderr, "Usage: %s orbit\n", argv[0] );
          exit( EXIT_FAILURE );
     }
     orbit = (unsigned short) atoi( argv[1] );

     fnd_24 = SDMF_get_BDPM_24( orbit, bdpm_24 );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_get_BDPM_24" );
     if ( ! fnd_24 ) (void) fprintf( stderr, "# no solution for SDMF v2.4\n" );
     fnd_30 = SDMF_get_BDPM_30( orbit, bdpm_30 );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_get_BDPM_30" );
     if ( ! fnd_30 ) (void) fprintf( stderr, "# no solution for SDMF v3.0\n" );

     if ( ! (fnd_24 || fnd_30) ) goto done;
     do {
	  (void) printf( "%5hu", np );
	  if ( fnd_24 )
	       (void) printf( " %2hhu", bdpm_24[np] );
	  if ( fnd_30 )
	       (void) printf( " %2hhu", bdpm_30[np] );
	  (void) printf( "\n" );
     } while( ++np < SCIENCE_PIXELS );
done:
     NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL )
	  exit( EXIT_FAILURE );
     else
	  exit( EXIT_SUCCESS );
}
#endif
