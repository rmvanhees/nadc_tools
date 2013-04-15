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

.IDENTifer   SDMF_get_OrbitalDark
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF - Dark calibration - channel 8
.LANGUAGE    ANSI C
.PURPOSE     obtain orbital dark correction parameters, channel 8
.COMMENTS    contains SDMF_get_OrbitalDark, SDMF_get_OrbitalDark_30
                      SDMF_get_OrbitalDark_24
.ENVIRONment None
.VERSION     1.1     10-Jan-2013   optionally use NRT entries, RvH
             1.0     29-May-2012   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
.IDENTifer   SDMF_get_OrbitalDark_24
.PURPOSE     obtain dark correction parameters (SDMF v2.4.x)
.INPUT/OUTPUT
  call as    found = SDMF_get_OrbitalDark_24( orbit, orbitPhase
                                            analogOffs, darkCurrent, 
					    analogOffsError, darkCurrentError );
     input:
           unsigned short absOrbit   :  orbit number
	   float          orbitPhase :  orbit phase (ESA definition)
 in/output:
           float *analogOffs         :  analog offset (BU)
           float *darkCurrent        :  leakage current (BU/s)
           float *analogOffsError    :  analog offset error (BU)
           float *darkCurrentError   :  leakage current error (BU/s)

.RETURNS     solution found (True or False)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
bool SDMF_get_OrbitalDark_24( unsigned short absOrbit, float orbitPhase, 
			      float *analogOffs, float *darkCurrent, 
			      float *analogOffsError, float *darkCurrentError )
{
     const char prognm[] = "SDMF_get_OrbitalDark_24";

     const int  orbit = (int) absOrbit;
     const unsigned short numOrbitDark = 72;
     const long sz_chan_byte = CHANNEL_SIZE * ENVI_FLOAT;
     const long sz_ds_byte = numOrbitDark * sz_chan_byte;

     register unsigned short nd, np;

     bool   fnd, found = FALSE;

     char   dark_fl[MAX_STRING_LENGTH];
     FILE   *dark_fp;
     float  rbuff[CHANNEL_SIZE], orbitDark[numOrbitDark * CHANNEL_SIZE];

     unsigned short nd_low, nd_high;

     const float orbitPhaseDiff = SDMF_orbitPhaseDiff( orbit );
/*
 * read constant dark parameters from SDMF database
 */
     fnd = SDMF_get_FittedDark_24( 8, absOrbit, analogOffs, darkCurrent,
				   analogOffsError, darkCurrentError, NULL );
     if ( ! fnd ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
			   "No dark entry found in SDMF (v2.4)" );
/*
 * convert orbit phase to SDMF definition:
 *        orbitPhase(ESA) = orbitPhaseDiff + orbitPhase(SDMF);
 */
     orbitPhase -= orbitPhaseDiff;
     if ( orbitPhase < 0.f ) orbitPhase += 1.f;
/*
 * find file with dark signals, requirements:
 *  - orbit number within a range MAX_DiffOrbitNumber
 */
     if ( ! SDMF_get_fileEntry( SDMF24_ORBITAL, orbit, dark_fl ) ) return FALSE;
/*
 * read dark parameters
 */
     if ( (dark_fp = fopen( dark_fl, "r" )) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, dark_fl );

     if ( fread( orbitDark, sz_ds_byte, 1, dark_fp ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, "orbitDark" );
     (void) fclose( dark_fp );
     found = TRUE;
/*
 * linear interpolate to get the variable fraction of the leakage current
 */
     for ( nd = 0; nd < numOrbitDark; nd++ )
          if ( orbitPhase <=  ((float) nd / numOrbitDark) ) break;
     nd_low = (nd > 0u) ? nd - 1u : 0u;
     nd_high = (nd < numOrbitDark) ? nd : nd - 1u;
     NADC_INTERPOL( orbitPhase, ((float) nd_low / numOrbitDark),
		    ((float) nd_high / numOrbitDark), CHANNEL_SIZE, 
                    &orbitDark[nd_low * CHANNEL_SIZE],
                    &orbitDark[nd_high * CHANNEL_SIZE], rbuff );
/*
 * add orbital dark correction to dark current
 */
     for ( np = 0; np < CHANNEL_SIZE; np++ ) {
     	  if ( isnormal( darkCurrent[np] ) && isnormal( rbuff[np] ) )
     	       darkCurrent[np] += rbuff[np];
     }
done:
     return found;
}

/*+++++++++++++++++++++++++ SDMF version 3.0 ++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_OrbitalDark_30
.PURPOSE     obtain dark correction parameters (SDMF v3.0)
.INPUT/OUTPUT
  call as    found = SDMF_get_OrbitalDark_30( orbit, orbitPhase
                                            analogOffs, darkCurrent, 
					    analogOffsError, darkCurrentError );
     input:
           unsigned short absOrbit   :  orbit number
	   float          orbitPhase : orbit phase
 in/output:
           float *analogOffs         :  analog offset (BU)
           float *darkCurrent        :  leakage current (BU/s)
           float *analogOffsError    :  analog offset error (BU)
           float *darkCurrentError   :  leakage current error (BU/s)

.RETURNS     solution found (True or False)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
bool SDMF_get_OrbitalDark_30( unsigned short absOrbit, float orbitPhase, 
			      float *analogOffs, float *darkCurrent, 
			      float *analogOffsError, float *darkCurrentError )
{
     const char prognm[] = "SDMF_get_OrbitalDark_30";

     register unsigned short np;

     hid_t  fid = -1;
     hid_t  gid = -1;

     bool   found = FALSE;
     int    numIndx, metaIndx;
     float  orbvar, orbsig;
     float  amp1[CHANNEL_SIZE], sig_amp1[CHANNEL_SIZE];

     char str_msg[MAX_STRING_LENGTH];
     char sdmf_db[MAX_STRING_LENGTH];

     struct mtbl_simudark_rec *mtbl = NULL;

     const int orbit        = (int) absOrbit;
     const int pixelRange[] = {0, CHANNEL_SIZE-1};
     const long sz_chan_byte = CHANNEL_SIZE * ENVI_FLOAT;
     const char msg_found[] =
          "\n\tapplied SDMF SimuDark (v3.0) of orbit: %-d";
/*
 * initialize returned values
 */
     (void) memset( analogOffs, 0, sz_chan_byte );
     (void) memset( darkCurrent, 0, sz_chan_byte );
     (void) memset( analogOffsError, 0, sz_chan_byte );
     (void) memset( darkCurrentError, 0, sz_chan_byte );
/*
 * open SDMF simu-dark database
 */
     (void) snprintf( sdmf_db, MAX_STRING_LENGTH, "%s/%s", 
                      SDMF_PATH("3.0"), "sdmf_simudark.h5" );
     H5E_BEGIN_TRY {
	  fid = H5Fopen( sdmf_db, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( fid < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, sdmf_db );

     if ( (gid = H5Gopen( fid, "ch8", H5P_DEFAULT )) < 0 )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, "/ch8" );
/*
 * obtain indices relevant entries
 */
     numIndx = 1;
     metaIndx = -1;
     (void) SDMF_get_metaIndex( gid, orbit, &numIndx, &metaIndx );
     if ( numIndx == 0 ) goto done;
     found = TRUE;
/*
 * read simu-dark data
 */
     SDMF_rd_simudarkTable( gid, &numIndx, &metaIndx, &mtbl );
     SDMF_rd_float_Array( gid, "ao", 1, &metaIndx, pixelRange, analogOffs );
     SDMF_rd_float_Array( gid, "lc", 1, &metaIndx, pixelRange, darkCurrent );
     SDMF_rd_float_Array( gid, "sig_ao", 1, &metaIndx, pixelRange, 
			  analogOffsError );
     SDMF_rd_float_Array( gid, "sig_lc", 1, &metaIndx, pixelRange,
			  darkCurrentError );
     SDMF_rd_float_Array( gid, "amp1", 1, &metaIndx, pixelRange, amp1 );
     SDMF_rd_float_Array( gid, "sig_amp1", 1, &metaIndx, pixelRange, sig_amp1 );
/*
 * calculate orbital dark for requested orbit-phase
 */
     orbvar = (float) 
	  (cos(2 * PI * (mtbl->orbitPhase + orbitPhase))
	   + mtbl->amp2 * cos(4 * PI * (mtbl->phase2 + orbitPhase)));
     orbsig = (float) 
	  (cos(2 * PI * (mtbl->orbitPhase + orbitPhase))
	   + mtbl->sig_amp2 * cos(4 * PI * (mtbl->phase2 + orbitPhase)));
     for ( np = 0; np < CHANNEL_SIZE; np++ ) {
	   darkCurrent[np] += orbvar * amp1[np];
	   darkCurrentError[np] += orbsig * sig_amp1[np];
     }
     (void) snprintf( str_msg, SHORT_STRING_LENGTH, msg_found, mtbl->absOrbit );
     NADC_ERROR( prognm, NADC_ERR_NONE, str_msg );
done:
     if ( mtbl != NULL ) free( mtbl );
     if ( gid > 0 ) H5Gclose( gid );
     if ( fid > 0 ) H5Fclose( fid );
     return found;
}

/*+++++++++++++++++++++++++ SDMF version 3.1 ++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_OrbitalDark
.PURPOSE     obtain dark correction parameters (SDMF v3.1)
.INPUT/OUTPUT
  call as    found = SDMF_get_OrbitalDark( orbit, orbitPhase
                                           analogOffs, darkCurrent, 
					   analogOffsError, darkCurrentError );
     input:
           unsigned short absOrbit   :  orbit number
	   float          orbitPhase : orbit phase
 in/output:
           float *analogOffs         :  analog offset (BU)
           float *darkCurrent        :  leakage current (BU/s)
           float *analogOffsError    :  analog offset error (BU)
           float *darkCurrentError   :  leakage current error (BU/s)

.RETURNS     solution found (True or False)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
bool SDMF_get_OrbitalDark( unsigned short absOrbit __attribute ((unused)), 
			   float orbitPhase __attribute ((unused)), 
			   float *analogOffs, float *darkCurrent, 
			   float *analogOffsError, float *darkCurrentError )
{
     /* const char prognm[] = "SDMF_get_OrbitalDark"; */

     bool   found = FALSE;

     const long sz_chan_byte = CHANNEL_SIZE * ENVI_FLOAT;
/*
 * initialize returned values
 */
     (void) memset( analogOffs, 0, sz_chan_byte );
     (void) memset( darkCurrent, 0, sz_chan_byte );
     (void) memset( analogOffsError, 0, sz_chan_byte );
     (void) memset( darkCurrentError, 0, sz_chan_byte );

     return found;
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
#ifdef TEST_PROG
bool Use_Extern_Alloc = FALSE;

int main( int argc, char *argv[] )
{
     const char prognm[] = "sdmf_get_orbitaldark";

     register unsigned short np = 0;

     unsigned short orbit;
     float orbitPhase;

     bool fnd_24, fnd_30;

     float ao_24[CHANNEL_SIZE], lc_24[CHANNEL_SIZE], 
	  ao_err_24[CHANNEL_SIZE], lc_err_24[CHANNEL_SIZE];
     float ao_30[CHANNEL_SIZE], lc_30[CHANNEL_SIZE], 
	  ao_err_30[CHANNEL_SIZE], lc_err_30[CHANNEL_SIZE];
/*
 * initialization of command-line parameters
 */
     if ( argc <= 2 || (argc > 1 && strncmp( argv[1], "-h", 2 ) == 0) ) {
	  (void) fprintf( stderr, "Usage: %s orbit orbitPhase\n", argv[0] );
          exit( EXIT_FAILURE );
     }
     orbit = (unsigned short) atoi( argv[1] );
     orbitPhase = (float) atof( argv[2] );
     
     fnd_24 = SDMF_get_OrbitalDark_24( orbit, orbitPhase, ao_24, lc_24,
				       ao_err_24, lc_err_24 );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR(prognm, NADC_ERR_FATAL, "SDMF_get_OrbitalDark_24");

     fnd_30 = SDMF_get_OrbitalDark_30( orbit, orbitPhase, ao_30, lc_30,
				       ao_err_30, lc_err_30 );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR(prognm, NADC_ERR_FATAL, "SDMF_get_OrbitalDark_30");
/*
 * 
 */
     if ( ! (fnd_24 || fnd_30) ) goto done;
     do {
	  (void) printf( "%5hu", np );
	  if ( fnd_24 ) {
	       (void) printf( " %12.6g %12.6g %12.6g %12.6g", 
			      ao_24[np], lc_24[np], ao_err_24[np], 
			      lc_err_24[np] );
	  }
	  if ( fnd_30 ) {
	       (void) printf( " %12.6g %12.6g %12.6g %12.6g", 
			      ao_30[np], lc_30[np], ao_err_30[np], 
			      lc_err_30[np] );
	  }
	  (void) printf( "\n" );
     } while( ++np < CHANNEL_SIZE );
done:
     NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL )
          exit( EXIT_FAILURE );
     else
          exit( EXIT_SUCCESS );
}
#endif /* TEST_PROG */
