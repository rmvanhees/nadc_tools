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

.IDENTifer   SDMF_get_PPG
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF - Pixel-to-Pixel Gain
.LANGUAGE    ANSI C
.PURPOSE     obtain PPG parameters
.COMMENTS    contains SDMF_get_PPG, SDMF_get_PPG_30
.ENVIRONment None
.VERSION     1.0     04-Jul-2012   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>

/*+++++ Local Headers +++++*/
#include <nadc_sdmf.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ SDMF version 2.4 ++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_PPG_24
.PURPOSE     Read Pixel-to-Pixel Gain factors from Monitoring database (v2.4)
.INPUT/OUTPUT
  call as    SDMF_get_PPG_24( absOrbit, channel, pixelGain );
     input:
           unsigned short absOrbit  :  absolute orbitnumber
           unsigned short channel   :  channel ID or zero for all channels
    output:
           float *pixelGain         :  Pixel-to-Pixel Gain factors

.RETURNS     flag: FALSE (no mask found) or TRUE
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
bool SDMF_get_PPG_24( unsigned short absOrbit, unsigned short channel, 
		      float *pixelGain )
{
     const char prognm[] = "SDMF_get_PPG_24";

     register unsigned np = 0;

     char ppg_fl[MAX_STRING_LENGTH];

     FILE  *db_fp;

     const size_t disk_sz_ppg_rec = 32816;
     struct ppg_rec {
          int    Orbit;
          int    MagicNumber;
          int    Saa;
          float  Tobm;
          float  Tdet[SCIENCE_CHANNELS];
          float  PixelGain[SCIENCE_PIXELS];
     } mrec;
/*
 * initialise output arrays
 */
     if ( channel == 0 ) {
	  do { pixelGain[np] = 1.f; } while ( ++np < SCIENCE_PIXELS );
     } else {
	  do { pixelGain[np] = 1.f; } while ( ++np < CHANNEL_SIZE );
     }
/*
 * find file with Pixel-to-Pixel Gains, requirements:
 *  - orbit number within a range MAX_DiffOrbitNumber
 */
     if ( ! SDMF_get_fileEntry( SDMF24_PPG, (int) absOrbit, ppg_fl ) ) 
          return FALSE;
/*
 * read Pixel-to-Pixel Gain values
 */
     if ( (db_fp = fopen( ppg_fl, "r" )) == NULL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, ppg_fl );
     if ( fread( &mrec, disk_sz_ppg_rec, 1, db_fp ) != 1 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, "mrec" );
     (void) fclose( db_fp );

     if ( channel == 0 ) {
          (void) memcpy( pixelGain, mrec.PixelGain,
                         SCIENCE_PIXELS * sizeof(float) );
     } else {
          const size_t offs = (channel-1) * CHANNEL_SIZE;

          (void) memcpy( pixelGain, mrec.PixelGain+offs,
                         CHANNEL_SIZE * sizeof(float) );
     }
     return TRUE;
 done:
     return FALSE;
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_PPG_30
.PURPOSE     obtain Pixel-to-Pixel Gain factors from SRON Monitoring database
.INPUT/OUTPUT
  call as    SDMF_get_PPG_30( absOrbit, channel, pixelGain );
     input:
           unsigned short absOrbit  :  absolute orbitnumber
           unsigned short channel   :  channel ID or zero for all channels
    output:
	   float *pixelGain         :  Pixel-to-Pixel Gain factors

.RETURNS     flag: FALSE (no PPG found) or TRUE
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
bool SDMF_get_PPG_30( unsigned short absOrbit, unsigned short channel,
		      float *pixelGain )
{
     const char prognm[] = "SDMF_get_PPG_30";

     const char msg_found[] =
          "\n\tapplied SDMF Pixel-to-Pixel Gain (v3.0) of Orbit %-d";
     const char msg_notfound[] =
          "\n\tno applicable Pixel-to-Pixel Gain (v3.0) found for Orbit %-d";

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
/*
 * initialise output arrays
 */
     if ( channel == 0 ) {
	  do { pixelGain[np] = 1.f; } while ( ++np < SCIENCE_PIXELS );
     } else {
	  do { pixelGain[np] = 1.f; } while ( ++np < CHANNEL_SIZE );
     }
/*
 * open SDMF PPG database
 */
     (void) snprintf( sdmf_db, MAX_STRING_LENGTH, "%s/%s", 
                      SDMF_PATH("3.0"), "sdmf_ppg.h5" );
     H5E_BEGIN_TRY {
	  fid = H5Fopen( sdmf_db, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( fid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, sdmf_db );
/*
 * find PPG values, requirements:
 *  - orbit number within a range MAX_DiffOrbitNumber
 */
     metaIndx = -1;
     do {
	  numIndx = 1;
	  (void) SDMF_get_metaIndex( fid, orbit + delta, &numIndx, &metaIndx );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "SDMF_get_metaIndex" );
	  if ( numIndx > 0 ) {
	       found = TRUE;
	  } else {
	       delta = (delta > 0) ? (-delta) : (1 - delta);
	       if ( abs( delta ) > MAX_DiffOrbitNumber )  {
		    (void) snprintf( str_msg, SHORT_STRING_LENGTH, 
				     msg_notfound, orbit );
		    NADC_GOTO_ERROR( prognm, NADC_SDMF_ABSENT, str_msg );
	       }
	  }
     } while ( ! found );

     if ( channel == 0 ) {
          SDMF_rd_float_Array( fid, "pixelGain", 1, &metaIndx, NULL, 
                               pixelGain );
     } else {
          SDMF_rd_float_Array( fid, "pixelGain", 1, &metaIndx, pixelRange, 
                               pixelGain );
     }
     (void) snprintf( str_msg, SHORT_STRING_LENGTH, msg_found, orbit + delta );
     NADC_ERROR( prognm, NADC_ERR_NONE, str_msg );
/*
 * close SDMF pixel-to-pixel gain database
 */
 done:
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
     const char prognm[] = "sdmf_get_ppg";

     register unsigned short np = 0;

     unsigned short orbit;
     unsigned short channel = 0;

     bool  fnd_24, fnd_30;
     float ppg_24[SCIENCE_PIXELS], ppg_30[SCIENCE_PIXELS];
/*
 * initialization of command-line parameters
 */
     if ( argc == 1 || (argc > 1 && strncmp( argv[1], "-h", 2 ) == 0) ) {
          (void) fprintf(stderr, "Usage: %s orbit [channel]\n", argv[0]);
          exit( EXIT_FAILURE );
     }
     orbit = (unsigned short) atoi( argv[1] );
     if ( argc >= 3 ) channel = (unsigned short) atoi( argv[2] );

     fnd_24 = SDMF_get_PPG_24( orbit, channel, ppg_24 );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "SDMF_get_PPG_24" );
     if ( ! fnd_24 ) (void) fprintf( stderr, "# no solution for SDMF v2.4\n" );
     fnd_30 = SDMF_get_PPG_30( orbit, channel, ppg_30 );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "SDMF_get_PPG_30" );
     if ( ! fnd_30 ) (void) fprintf( stderr, "# no solution for SDMF v3.0\n" );

     if ( ! (fnd_24 || fnd_30) ) goto done;
     do {
          (void) printf( "%5hu", np );
          if ( fnd_24 )
               (void) printf( " %12.6g", ppg_24[np] );
          if ( fnd_30 )
               (void) printf( " %12.6g", ppg_30[np] );
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
