/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SDMF_get_fileEntry
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF - version 2.4.x
.LANGUAGE    ANSI C
.PURPOSE     select SDMF 2.4 entry
.INPUT/OUTPUT
  call as   found = SDMF_get_fileEntry( sdmfDB, orbit, fileEntry );
     input:  
              enum sdmf24_db sdmfDB : select type of calibration
              int orbit             : requested orbit number
    output:  
              char *fileEntry       : name of the selected file
.RETURNS     boolean value: TRUE when entry is found 
.COMMENTS    SDMF24_STATE mimics data averaging of Hans Schrijver
.ENVIRONment uses environment variable: 
             SDMF24_PATH: default path is /SCIA/SDMF241
	     SDMF24_SELECT: different selection methods from MonitorList.dat
               default:     select first entry encountered
	       NRT:         select only NRT products
	       CONS:        select only Consolidated products
.VERSION     1.1     31-Jan-2013   added SDMF24_STATE, updated test S/W, RvH
             1.0     16-Jan-2013   initial release by R. M. van Hees
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

/*+++++ Local Headers +++++*/
#include <nadc_sdmf.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
bool SDMF_get_fileEntry( enum sdmf24_db sdmfDB,
			 int orbit, char *fileEntry )
{
     register long nr = 0;

     bool sdmf_select_nrt = FALSE;
     bool sdmf_select_cons = FALSE;

     FILE *fp;

     char str_msg[MAX_STRING_LENGTH];
     char flname[MAX_STRING_LENGTH];
     char sdmfID[SHORT_STRING_LENGTH];
     char sdmfPath[MAX_STRING_LENGTH];
     char sdmfExt[18];

     long num, total_rec;
     int  delta = 0;
     int  fnd_orbit = -1;
     int  MaxDiffOrbitNumber;
     int  fnd_quality = 40;

     char *env_str = getenv( "SDMF24_SELECT" );

     const size_t disk_sz_monitor_rec = 182;
     struct monitor_rec *mrec = NULL;
/*
 * initialize return value
 */
     *fileEntry = '\0';
/*
 * different SDMF parameters require different selection criteria
 */
     switch ( sdmfDB ) {
     case SDMF24_STATE: {
	  (void) strcpy( sdmfID, "State Dark correction" );
	  (void) snprintf( sdmfPath, MAX_STRING_LENGTH, "%s/%s",
			   SDMF_PATH("2.4"), "Data" );
	  (void) strcpy( sdmfExt, "monitor" );
	  MaxDiffOrbitNumber = 14;
	  break;
     }
     case SDMF24_FITTED:
	  (void) strcpy( sdmfID, "Fitted Dark correction" );
	  (void) snprintf( sdmfPath, MAX_STRING_LENGTH, "%s/%s",
			   SDMF_PATH("2.4"), "DarkCurrent" );
	  (void) strcpy( sdmfExt, "darkcurrent" );
	  MaxDiffOrbitNumber = 14;
	  break;
     case SDMF24_ORBITAL:
	  (void) strcpy( sdmfID, "Orbital Dark correction" );
	  (void) snprintf( sdmfPath, MAX_STRING_LENGTH, "%s/%s",
			   SDMF_PATH("2.4"), "OrbitalVariation/Transmission" );
	  (void) strcpy( sdmfExt, "orbital" );
	  MaxDiffOrbitNumber = 50;
	  break;
     case SDMF24_BDPM:
	  (void) strcpy( sdmfID, "Bad Dead Pixel Mask" );
	  (void) snprintf( sdmfPath, MAX_STRING_LENGTH, "%s/%s",
			   SDMF_PATH("2.4"), "SmoothMask/ASCII" );
	  (void) strcpy( sdmfExt, "mask" );
	  MaxDiffOrbitNumber = 14;
	  break;
     case SDMF24_PPG:
	  (void) strcpy( sdmfID, "Pixel to Pixel Gain correction" );
	  (void) snprintf( sdmfPath, MAX_STRING_LENGTH, "%s/%s",
			   SDMF_PATH("2.4"), "PixelGain" );
	  (void) strcpy( sdmfExt, "pixelgain" );
	  MaxDiffOrbitNumber = 100;
	  break;
     case SDMF24_WLSTRANS:
	  (void) strcpy( sdmfID, "WLS Transmission correction" );
	  (void) snprintf( sdmfPath, MAX_STRING_LENGTH, "%s/%s",
			   SDMF_PATH("2.4"), "Transmission/WLS" );
	  (void) strcpy( sdmfExt, "WLStransmission" );
	  MaxDiffOrbitNumber = 100;
	  break;
     case SDMF24_TRANS:
	  (void) strcpy( sdmfID, "Transmission correction" );
	  (void) snprintf( sdmfPath, MAX_STRING_LENGTH, "%s/%s",
			   SDMF_PATH("2.4"), "Transmission/SunESM" );
	  (void) strcpy( sdmfExt, "transmission" );
	  MaxDiffOrbitNumber = 100;
	  break;
     default:
	  NADC_GOTO_ERROR( NADC_ERR_PARAM, "sdmfDB" );
     }
/*
 * Default selection is to use results based on consolidated products, 
 * use this environment variable to select results based on NRT products
 */
     if ( env_str != NULL ) {
	  if ( strstr( env_str, "NRT" ) != NULL ) sdmf_select_nrt = TRUE;
	  if ( strstr( env_str, "CONS" ) != NULL ) sdmf_select_cons = TRUE;
     }
/*
 * read database records (first determine number of records in database)
 */
     (void) snprintf( flname, MAX_STRING_LENGTH, "%s/%s", 
		      SDMF_PATH("2.4"), "MonitorList.dat" );
     if ( (fp = fopen( flname, "rb")) == NULL )
          NADC_GOTO_ERROR( NADC_ERR_FILE, flname );
     (void) fseek( fp, 0L, SEEK_END );
     total_rec = ftell( fp ) / disk_sz_monitor_rec;
     (void) fseek( fp, 0L, SEEK_SET );
     mrec = (struct monitor_rec *) 
          malloc( total_rec * sizeof(struct monitor_rec) );
     if ( mrec == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mrec" );

     num = 0;
     do {
          if ( fread( &mrec[num].FileName, 70, 1, fp ) != 1 )
               NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "mrec.FileName" );
          if ( fread( mrec+num, sizeof( struct monitor_rec )-72, 1, fp ) != 1 )
               NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "mrec" );

	  if ( abs(mrec[num].Orbit - orbit) > MaxDiffOrbitNumber ) continue;
	  if ( sdmf_select_nrt && mrec[num].Consolidated != 0 ) continue;
	  if ( sdmf_select_cons && mrec[num].Consolidated != 1 ) continue;

	  switch ( sdmfDB ) {
	  case SDMF24_STATE:
	       if ( mrec[num].QualityNumber <= 90 ) continue;
	       break;
	  case SDMF24_FITTED:
	       if ( mrec[num].QualityNumber <= 40 ) continue;
	       break;
	  case SDMF24_ORBITAL:
	       if ( mrec[num].Orbital == 0 ) continue;
	       break;
	  case SDMF24_BDPM:
	       if ( mrec[num].QualitySmoothMask < 10 ) continue;
	       break;
	  case SDMF24_PPG:
	       if ( mrec[num].PixelGain == 0 ) continue;
	       break;
	  case SDMF24_WLSTRANS:
	       if ( mrec[num].WLSTransmission == 0 ) continue;
	       break;
	  case SDMF24_TRANS:
	       if ( mrec[num].Transmission == 0 ) continue;
	       break;
	  }
	  num++;
     } while( ++nr < total_rec );
     (void) fclose( fp );
/*
 * find SDMF (v2.4) Entry
 */
     do {
          for ( nr = 0; nr < num; nr++ ) {
	       if ( mrec[nr].Orbit == (orbit + delta) 
		    && mrec[nr].QualityNumber > fnd_quality ) {
		    fnd_orbit = orbit + delta;
		    fnd_quality = mrec[nr].QualityNumber;
		    (void) snprintf( fileEntry, MAX_STRING_LENGTH, "%s/%s.%s",
				     sdmfPath, mrec[nr].FileName, sdmfExt );
		    if ( sdmfDB == SDMF24_FITTED ) {
			 if ( fnd_quality >= 70 ) goto done;
		    } else
			 goto done;
	       }
	  }
          delta = (delta > 0) ? (-delta) : (1 - delta);
     } while ( abs( delta ) <= MaxDiffOrbitNumber );
done:
     if ( mrec != NULL ) free( mrec );
     if ( fnd_orbit > 0 ) {
	  char msg[] = "\n\tSDMF(2.4): %s read from file: %s";

          (void) snprintf(str_msg, MAX_STRING_LENGTH, msg, sdmfID, fileEntry);
          NADC_ERROR( NADC_ERR_NONE, str_msg );
          return TRUE;
     } else {
	  char msg[] = "\n\tSDMF(2.4): %s - no applicable data for orbit %-d";

          (void) snprintf( str_msg, SHORT_STRING_LENGTH, msg, sdmfID, orbit );
          NADC_ERROR( NADC_ERR_NONE, str_msg );
          return FALSE;
     }
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
#ifdef TEST_PROG
bool Use_Extern_Alloc = FALSE;

int main( int argc, char *argv[] )
{
     register int orbit;

     int nrval, orbit_range[2];

     bool found;
     char fileEntry[MAX_STRING_LENGTH];

     char *env24_str = getenv( "USE_SDMF_VERSION" );
/*
 * initialization of command-line parameters
 */
     if ( argc == 1 || (argc > 1 && strncmp( argv[1], "-h", 2 ) == 0) ) {
          (void) fprintf( stderr, "Usage: %s orbit_reange\n", argv[0] );
          exit( EXIT_FAILURE );
     }
     NADC_USRINP( INT32_T, argv[1], 2, orbit_range, &nrval );
     (void) printf( "%d %d %d\n", nrval, orbit_range[0], orbit_range[1] );
     if ( nrval == 1 ) orbit_range[1] = orbit_range[0];
/*
 * make sure we use the right version of SDMF
 */
     (void) setenv( "USE_SDMF_VERSION", "2.4", 1 );
/*
 * find all solutions
 */
     for ( orbit = orbit_range[0]; orbit <= orbit_range[1]; orbit++ ) {
	  found = SDMF_get_fileEntry( SDMF24_STATE, orbit, fileEntry );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_get_fileEntry" );
	  if ( found ) (void) printf( "%s\n", fileEntry );

	  found = SDMF_get_fileEntry( SDMF24_FITTED, orbit, fileEntry );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_get_fileEntry" );
	  if ( found ) (void) printf( "%s\n", fileEntry );

	  found = SDMF_get_fileEntry( SDMF24_ORBITAL, orbit, fileEntry );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_get_fileEntry" );
	  if ( found ) (void) printf( "%s\n", fileEntry );

	  found = SDMF_get_fileEntry( SDMF24_BDPM, orbit, fileEntry );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_get_fileEntry" );
	  if ( found ) (void) printf( "%s\n", fileEntry );

	  found = SDMF_get_fileEntry( SDMF24_PPG, orbit, fileEntry );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_get_fileEntry" );
	  if ( found ) (void) printf( "%s\n", fileEntry );

	  found = SDMF_get_fileEntry( SDMF24_WLSTRANS, orbit, fileEntry );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_get_fileEntry" );
	  if ( found ) (void) printf( "%s\n", fileEntry );

	  found = SDMF_get_fileEntry( SDMF24_TRANS, orbit, fileEntry );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_get_fileEntry" );
	  if ( found ) (void) printf( "%s\n", fileEntry );
     }
done:
     (void) setenv( "USE_SDMF_VERSION", env24_str, 1 );
     if ( IS_ERR_STAT_FATAL ) {
	  NADC_Err_Trace( stderr );
	  exit( EXIT_FAILURE );
     } else
	  exit( EXIT_SUCCESS );
}
#endif
