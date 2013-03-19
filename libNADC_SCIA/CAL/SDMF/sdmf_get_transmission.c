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

.IDENTifer   SDMF_TRANSMISSION
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF - Transmission
.LANGUAGE    ANSI C
.PURPOSE     read transmission based on Sun or WLS measurements
.COMMENTS    contains SDMF_get_Transmission_24 and SDMF_get_Transmission_30
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
.IDENTifer   SDMF_get_Transmission_24
.PURPOSE     obtain transmission from SRON Monitoring database (v2.4)
.INPUT/OUTPUT
  call as    SDMF_get_Transmission_24( wls, absOrbit, channel, transmission );
     input:
           bool           wls       :  transmission based on WLS measurements
           unsigned short absOrbit  :  absolute orbitnumber
	   unsigned short channel   :  channel ID or zero for all channels
    output:
           float *transmission      :  transmission factors

.RETURNS     flag: FALSE (no mask found) or TRUE
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
bool SDMF_get_Transmission_24( bool wlsFlag, unsigned short absOrbit, 
			       unsigned short channel, 
			       /*@out@*/ float *transmission )
{
     const char prognm[] = "SDMF_get_Transmission_24";

     register unsigned short np = 0;

     char trans_fl[MAX_STRING_LENGTH];

     FILE  *db_fp;

     const size_t disk_sz_trans_rec = 32836;
     struct transmission_rec {
	  int    Orbit;
	  int    MagicNumber;
	  int    Flags;
	  int    StateID;
	  double Time;
	  float  Phase;
	  int    Saa;
	  float  Tobm;
	  float  Tdet[SCIENCE_CHANNELS];
	  float  Transmission[SCIENCE_PIXELS];
     } mrec;
/*
 * initialise output arrays
 */
     if ( channel == 0 ) {
	  do { transmission[np] = 1.f; } while ( ++np < SCIENCE_PIXELS );
     } else {
	  do { transmission[np] = 1.f; } while ( ++np < CHANNEL_SIZE );
     }
/*
 * find file with transmission correction, requirements:
 *  - orbit number within a range MAX_DiffOrbitNumber
 */
     if ( wlsFlag ) {
	  if ( ! SDMF_get_fileEntry(SDMF24_WLSTRANS, (int) absOrbit, trans_fl)) 
	       return FALSE;
     } else {
	  if ( ! SDMF_get_fileEntry( SDMF24_TRANS, (int) absOrbit, trans_fl) ) 
	       return FALSE;
     }
/*
 * read transmission parameters
 */
     if ( (db_fp = fopen( trans_fl, "r" )) == NULL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, trans_fl );
     if ( fread( &mrec, disk_sz_trans_rec, 1, db_fp ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, "mrec" );
     (void) fclose( db_fp );

     if ( channel == 0 ) {
	  (void) memcpy( transmission, mrec.Transmission,
			 SCIENCE_PIXELS * sizeof(float) );
     } else {
	  const size_t offs = (channel-1) * CHANNEL_SIZE;

	  (void) memcpy( transmission, mrec.Transmission+offs,
			 CHANNEL_SIZE * sizeof(float) );
     }
     return TRUE;
 done:
     return FALSE;
}

/*+++++++++++++++++++++++++ SDMF version 3.0 ++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_Transmission_30
.PURPOSE     obtain transmission from SRON Monitoring database (v3.0)
.INPUT/OUTPUT
  call as    SDMF_get_Transmission_30( wls, absOrbit, channel, transmission );
     input:
           bool           wlsFlag   :  transmission based on WLS measurements
           unsigned short absOrbit  :  absolute orbitnumber
	   unsigned short channel   :  channel ID or zero for all channels
    output:
           float *transmission      :  transmission factors

.RETURNS     flag: FALSE (no mask found) or TRUE
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
bool SDMF_get_Transmission_30( bool wlsFlag, unsigned short absOrbit, 
			       unsigned short channel, 
			       /*@out@*/ float *transmission )
{
     const char prognm[] = "SDMF_get_Transmission_30";

     const char msg_found[] =
          "\n\tapplied SDMF transmission (v3.0) of Orbit: %-d";
     const char msg_notfound[] =
          "\n\tno applicable SDMF transmission (v3.0) found for Orbit: %-d";

     const int  MAX_DiffOrbitNumber = 100;
     const int  orbit = (int) absOrbit;
     const int pixelRange[2] = { 
          (channel == 0) ? 0 : (channel-1) * CHANNEL_SIZE,
          (channel == 0) ? SCIENCE_PIXELS-1 : channel * CHANNEL_SIZE - 1
     };

     register unsigned short np = 0;
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
     if ( channel == 0 ) {
	  do { transmission[np] = 1.f; } while ( ++np < SCIENCE_PIXELS );
     } else {
	  do { transmission[np] = 1.f; } while ( ++np < CHANNEL_SIZE );
     }
/*
 * open output HDF5-file
 */
     (void) snprintf( sdmf_db, MAX_STRING_LENGTH, "%s/%s", 
                      SDMF_PATH("3.0"), "sdmf_transmission.h5" );
     H5E_BEGIN_TRY {
	  fid = H5Fopen( sdmf_db, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( fid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, sdmf_db );

     if ( wlsFlag ) {
	  if ( (gid = H5Gopen( fid, "/WLStransmission", H5P_DEFAULT )) < 0 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, "/WLStransmission" );
     } else {
	  if ( (gid = H5Gopen( fid, "/Transmission", H5P_DEFAULT )) < 0 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, "/Transmission" );
     }
/*
 * find transmission factors, requirements:
 *  - orbit number within a range MAX_DiffOrbitNumber
 */
     metaIndx = -1;
     do {
          numIndx = 1;
          (void) SDMF_get_metaIndex( gid, orbit + delta, &numIndx, &metaIndx );
          if ( IS_ERR_STAT_FATAL )
               NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "SDMF_get_metaIndex" );
          if ( numIndx > 0 ) {
               found = TRUE;
          } else {
               delta = (delta > 0) ? (-delta) : (1 - delta);
               if ( abs( delta ) > MAX_DiffOrbitNumber ) {
                    (void) snprintf( str_msg, SHORT_STRING_LENGTH, 
                                     msg_notfound, orbit );
                    NADC_GOTO_ERROR( prognm, NADC_SDMF_ABSENT, str_msg );
               }
          }
     } while ( ! found );

     if ( channel == 0 ) {
	  SDMF_rd_float_Array( gid, "transmission", 1, &metaIndx, NULL, 
			       transmission );
     } else {
	  SDMF_rd_float_Array( gid, "transmission", 1, &metaIndx, pixelRange, 
			       transmission );
     }

     (void) snprintf( str_msg, SHORT_STRING_LENGTH, msg_found, orbit + delta );
     NADC_ERROR( prognm, NADC_ERR_NONE, str_msg );
/*
 * close SDMF Transmission database
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
     const char prognm[] = "sdmf_transmission";

     register unsigned short np = 0;

     bool           wlsFlag = FALSE;
     unsigned short orbit;
     unsigned short channel = 0;

     bool  fnd_24, fnd_30;
     float trans_24[SCIENCE_PIXELS], trans_30[SCIENCE_PIXELS];
/*
 * initialization of command-line parameters
 */
     if ( argc == 1 || (argc > 1 && strncmp( argv[1], "-h", 2 ) == 0) ) {
          (void) fprintf(stderr, "Usage: %s orbit [channel] [wls]\n", argv[0]);
          exit( EXIT_FAILURE );
     }
     orbit = (unsigned short) atoi( argv[1] );
     if ( argc >= 3 ) channel = (unsigned short) atoi( argv[2] );
     if ( argc == 4 ) wlsFlag = TRUE;

     fnd_24 = SDMF_get_Transmission_24( wlsFlag, orbit, channel, trans_24 );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "SDMF_get_Transmission_24" );
     if ( ! fnd_24 ) (void) fprintf( stderr, "# no solution for SDMF v2.4\n" );
     fnd_30 = SDMF_get_Transmission_30( wlsFlag, orbit, channel, trans_30 );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "SDMF_get_Transmission_30" );
     if ( ! fnd_30 ) (void) fprintf( stderr, "# no solution for SDMF v3.0\n" );

     if ( ! (fnd_24 || fnd_30) ) goto done;
     do {
	  (void) printf( "%5hu", np );
	  if ( fnd_24 )
	       (void) printf( " %12.6g", trans_24[np] );
	  if ( fnd_30 )
	       (void) printf( " %12.6g", trans_30[np] );
	  (void) printf( "\n" );
     } while( ++np < ((channel == 0) ? SCIENCE_PIXELS : CHANNEL_SIZE) );
done:
     NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL )
          exit( EXIT_FAILURE );
     else
          exit( EXIT_SUCCESS );
}
#endif
