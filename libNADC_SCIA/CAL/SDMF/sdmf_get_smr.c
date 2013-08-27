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

.IDENTifer   SDMF_get_SMR
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF - Sun-Mean_Reference
.LANGUAGE    ANSI C
.PURPOSE     obtain SMR parameters
.COMMENTS    contains SDMF_get_SMR, SDMF_get_SMR_30
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
                         /* not available, yet */

/*+++++++++++++++++++++++++ SDMF version 3.0 ++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_SMR_30
.PURPOSE     obtain Sun Mean Reference spectrum from SRON Monitoring database
.INPUT/OUTPUT
  call as    SDMF_get_SMR_30( calibRad, absOrbit, channel, solarMean );
     input:
           unsigned short absOrbit  :  absolute orbitnumber
           unsigned short channel   :  channel ID or zero for all channels
    output:
	   float *solarMean         :  Sun Mean Reference spectrum

.RETURNS     flag: FALSE (no SMR found) or TRUE
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
static inline
void __Inverse_Chan2( float *rbuff )
{
     register unsigned short nr;
     register float rtemp;

     rbuff += CHANNEL_SIZE;                   /* move to channel 2 data */
     for ( nr = 0; nr <  CHANNEL_SIZE / 2; nr++ ) {
          rtemp = rbuff[nr];
          rbuff[nr] = rbuff[(CHANNEL_SIZE-1)-nr];
          rbuff[(CHANNEL_SIZE-1)-nr] = rtemp;
     }
}

bool SDMF_get_SMR_30( bool calibRad,
		      unsigned short absOrbit, unsigned short channel,
		      /*@null@*/ const float *wvlen, float *solarMean )
{
     const char prognm[] = "SDMF_get_SMR_30";

     const char msg_found[] =
          "\n\tapplied SDMF Sun-Mean-Reference spectrum (v3.0) of Orbit %-d";
     const char msg_notfound[] =
          "\n\tno applicable Sun-Mean-Reference (v3.0) found for Orbit %-d";

     const int  MAX_DiffOrbitNumber = 50;  /* within one week */
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
	  do { solarMean[np] = 0.f; } while ( ++np < SCIENCE_PIXELS );
     } else {
	  do { solarMean[np] = 0.f; } while ( ++np < CHANNEL_SIZE );
     }
/*
 * open SDMF Sun-Mean-Reference database
 */
     (void) snprintf( sdmf_db, MAX_STRING_LENGTH, "%s/%s", 
                      SDMF_PATH("3.0"), "sdmf_smr.h5" );
     H5E_BEGIN_TRY {
	  fid = H5Fopen( sdmf_db, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( fid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, sdmf_db );
/*
 * find SMR values, requirements:
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
     (void) snprintf( str_msg, SHORT_STRING_LENGTH, msg_found, orbit + delta );
     NADC_ERROR( prognm, NADC_ERR_NONE, str_msg );
/*
 * read SMR data
 */
     if ( channel == 0 ) {
          SDMF_rd_float_Array( fid, "SMR", 1, &metaIndx, NULL, 
                               solarMean );
	  __Inverse_Chan2( solarMean );
     } else {
          SDMF_rd_float_Array( fid, "SMR", 1, &metaIndx, pixelRange, 
                               solarMean );
     }
/*
 * calibrate SMR spectrum
 */
     if ( calibRad ) {
	  const int indx[]     = {metaIndx, 240/2-1};
	  const int slabsize[] = {1,1};
	  float asm_pos, esm_pos, sunel;

	  SDMF_rd_float_Matrix( fid, "asm", indx, slabsize, 2, &asm_pos );
	  SDMF_rd_float_Matrix( fid, "esm", indx, slabsize, 2, &esm_pos );
	  SDMF_rd_float_Matrix( fid, "sunel", indx, slabsize, 2, &sunel );

	  SCIA_SMR_CAL_RAD( absOrbit, channel, asm_pos, sunel, wvlen, 
			    solarMean );
     }
/*
 * close SDMF Sun-Mean-Reference database
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
     const char prognm[] = "sdmf_get_smr";

     register unsigned short np = 0;

     unsigned short orbit;
     unsigned short channel = 0;

     bool  fnd_30;
     bool  calibRad = FALSE;
     float smr_30[SCIENCE_PIXELS];
/*
 * initialization of command-line parameters
 */
     if ( argc == 1 || (argc > 1 && strncmp( argv[1], "-h", 2 ) == 0) ) {
          (void) fprintf(stderr, "Usage: %s orbit [channel] [calibRad]\n", argv[0]);
          exit( EXIT_FAILURE );
     }
     orbit = (unsigned short) atoi( argv[1] );
     if ( argc >= 3 ) channel = (unsigned short) atoi( argv[2] );
     if ( argc >= 4 && strncmp( argv[3], "calibRad", 7 ) == 0 )
	  calibRad = TRUE;

     fnd_30 = SDMF_get_SMR_30( calibRad, orbit, channel, NULL, smr_30 );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "SDMF_get_SMR_30" );
     if ( ! fnd_30 ) (void) fprintf( stderr, "# no solution for SDMF v3.0\n" );

     if ( ! (fnd_30) ) goto done;
     do {
          (void) printf( "%5hu", np );
          if ( fnd_30 )
               (void) printf( " %12.6g", smr_30[np] );
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
