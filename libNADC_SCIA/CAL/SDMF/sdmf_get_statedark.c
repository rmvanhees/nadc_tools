/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2010 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SDMF_get_StateDark
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF - Dark calibration
.LANGUAGE    ANSI C
.PURPOSE     obtain (state) dark correction parameters
.COMMENTS    contains SDMF_get_StateDark, SDMF_get_StateDark_30
                      SDMF_get_StateDark_24
.ENVIRONment None
.VERSION     2.3     10-Sep-2014   do not fail on missing orbits (v3.0), RvH
             2.2     20-Sep-2012   added option to mimic algorithm of Hans 
                                   Schrijver for dark noise (v2.4), RvH
             2.1     16-May-2012   back-ported SDMF v2.4 & 3.0, RvH
             2.0     14-May-2012   added test program, RvH
             1.0     19-Apr-2010   initial release by R. M. van Hees
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
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#include <nadc_sdmf.h>

/*+++++ Macros +++++*/
#define STR_SZ_H5_GRP        12
#define MAX_NUM_META_INDEX   32
#define MAX_NUM_MONI_REC     10

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
static bool sdmf_mimic_hanss = FALSE;

/*+++++++++++++++++++++++++ SDMF version 2.4 ++++++++++++++++++++++++++++++*/
static
bool SDMF_get_MoniEntry( int orbit, unsigned char stateID,
			 int *num_fl, char *moni_fl )
{
     const char stateList[] = {
	  65, 46, 63, 67, 26, 8, 16, 48, 39, 61, 62, 52, 59, 69, 70, 1
     };
     const size_t DimStateList = (sizeof stateList / sizeof(char));

     const int  MaxDiffOrbitNumber = 14;

     register long nr = 0;

     bool sdmf_select_nrt  = FALSE;
     bool sdmf_select_cons = FALSE;

     FILE *fp;

     char   *pntr = moni_fl;
     char   str_msg[MAX_STRING_LENGTH];
     char   flname[MAX_STRING_LENGTH];

     int    ns;
     long   num, total_rec;
     size_t iState = 0;
     int    delta = 0;
     int    fnd_orbit = -1;

     char *env_str = getenv( "SDMF24_SELECT" );

     const size_t disk_sz_monitor_rec = 182;
     struct monitor_rec *mrec = NULL;
/*
 * initialize returned variables
 */
     *num_fl = 0;
/*
 * determine index to array StateCount
 */
     while ( iState < DimStateList && stateList[iState] != stateID ) iState++;
/*
 * Default selection is to use results based on consolidated products, 
 * use this environment variable to select results based on NRT products
 */
     if ( env_str != NULL ) {
	  if ( strstr( env_str, "NRT" ) != NULL ) sdmf_select_nrt = TRUE;
	  if ( strstr( env_str, "CONS" ) != NULL ) sdmf_select_cons = TRUE;
	  if ( strstr( env_str, "HANS" ) != NULL ) sdmf_mimic_hanss = TRUE;
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
	  if ( sdmf_mimic_hanss ) {
	       if ( mrec[num].QualityNumber <= 90 ) continue; 
	  } else {
	       if ( mrec[num].StateCount[iState] == 0 ) continue;
	  }
	  num++;
     } while( ++nr < total_rec );
     (void) fclose( fp );
/*
 * find file with dark signals, requirements:
 *  - orbit number within a range MaxDiffOrbitNumber
 */
     do {
	  for ( nr = 0; nr < num; nr++ ) {
	       if ( mrec[nr].Orbit == (orbit + delta) ) {
		    fnd_orbit = orbit + delta;
		    ns = snprintf( pntr, MAX_STRING_LENGTH,
				   "%s/Data/%s.monitor", SDMF_PATH("2.4"),
				   mrec[nr].FileName );
		    *num_fl += 1;
		    pntr += (ns + 1);
		    if ( sdmf_mimic_hanss ) goto done;
		    if ( *num_fl == MAX_NUM_MONI_REC ) goto done;
	       }
	  }
	  delta = (delta < 0) ? (-delta) : (-(delta+1));
     } while ( fnd_orbit == -1 && abs( delta ) < MaxDiffOrbitNumber );
done:
     if ( mrec != NULL ) free( mrec );
     if ( fnd_orbit > 0 ) {
	  char msg[] = 
	       "\n\tSDMF(2.4): State Dark correction applied from file %s";

	  (void) snprintf( str_msg, MAX_STRING_LENGTH, msg, moni_fl );
	  NADC_ERROR( NADC_ERR_NONE, str_msg );
	  return TRUE;
     } else {
	  char msg[] = 
	       "\n\tSDMF(2.4): State Dark correction - no applicable data for orbit %-d";
          (void) snprintf( str_msg, SHORT_STRING_LENGTH, msg, orbit );
          NADC_ERROR( NADC_ERR_NONE, str_msg );
          return FALSE;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_StateDark_24
.PURPOSE     obtain dark correction parameters (SDMF v2.4.1)
.INPUT/OUTPUT
  call as    found = SDMF_get_StateDark_24( stateID, channel, absOrbit, 
                                            pet, darkSignal, darkNoise );
     input:  
           unsigned char  stateID  :  state ID
	   unsigned short channel  :  channel ID or zero for all channels
           unsigned short absOrbit :  orbit number
    output:  
           float *pet              :  channel Pixel Exposure Time (sec)
	   float *darkSignal       :  dark signal (BU, memory/non-linearity)
	   float *darkNoise        :  dark noise (BU, standard deviation)

.RETURNS     solution found (True or False)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
bool SDMF_get_StateDark_24( unsigned char stateID, unsigned short channel, 
			    unsigned short absOrbit, float *pet, 
			    float *darkSignal, float *darkNoise )
{
     char   *pntr, *moni_fl;

     register unsigned short nj;
     register unsigned short nr = 0;
     register unsigned short num = 0;

     unsigned short total[SCIENCE_PIXELS];

     bool   found = FALSE;
     int    num_fl;
     size_t num_rec;
     float  orbitPhaseDiff;

     FILE   *fp;

     const int  orbit = (int) absOrbit;

     const size_t disk_sz_moni_rec = 131148;
     struct calib_rec {
	  int    Orbit;
	  int    MagicNumber;
	  int    Flags;
	  int    StateId;
	  double Time;
	  float  Phase;
	  int    Saa;
	  float  EsmAngle;
	  float  AsmAngle;
	  float  Tobm;
	  float  Tdet[SCIENCE_CHANNELS];
	  float  Pet[SCIENCE_PIXELS];
	  float  Signal[SCIENCE_PIXELS];
	  float  Error[SCIENCE_PIXELS];
	  float  Noise[SCIENCE_PIXELS];
     } mrec[MAX_NUM_MONI_REC];
/*
 * initialize returned values
 */
     if ( channel == 0 ) {
	  (void) memset( pet, 0, SCIENCE_CHANNELS * sizeof(float) );
	  (void) memset( total, 0, SCIENCE_PIXELS * sizeof(short) );
	  (void) memset( darkSignal, 0, SCIENCE_PIXELS * sizeof(float) );
	  (void) memset( darkNoise, 0, SCIENCE_PIXELS * sizeof(float) );
     } else {
	  *pet = 0.f;
	  (void) memset( total, 0, CHANNEL_SIZE * sizeof(short) );
	  (void) memset( darkSignal, 0, CHANNEL_SIZE * sizeof(float) );
	  (void) memset( darkNoise, 0, CHANNEL_SIZE * sizeof(float) );
     }
/*
 * find file with dark signals, requirements:
 *  - orbit number within a range MAX_DiffOrbitNumber
 *  - quality number at least equal to MIN_QualityNumber
 */
     sdmf_mimic_hanss = FALSE;
     moni_fl = (char *) malloc( MAX_NUM_MONI_REC * MAX_STRING_LENGTH );
     if ( ! SDMF_get_MoniEntry( orbit, stateID, &num_fl, moni_fl ) )
	  goto done;
/*
 * get correction value for orbitPhase
 */
     orbitPhaseDiff = SDMF_orbitPhaseDiff( orbit );
/*
 * read dark parameters
 */
     pntr = moni_fl;
     for ( nr = 0; nr < num_fl; nr++ )  {
	  register size_t nrr = 0;

	  if ( (fp = fopen( pntr, "r" )) == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_FILE, pntr );
	  (void) fseek( fp, 0L, SEEK_END );
	  num_rec = ftell( fp ) / disk_sz_moni_rec;
	  (void) fseek( fp, 0L, SEEK_SET );
	  pntr = strchr( pntr, (int) '\0' ) + 1;

	  do {
	       if ( fread( mrec+num, disk_sz_moni_rec, 1, fp ) != 1 )
		    NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "mrec" );
	       if ( mrec[num].StateId != (int) stateID ) continue;
	       mrec[num].Phase += orbitPhaseDiff;
	       if ( ! sdmf_mimic_hanss ) {
		    if ( mrec[num].Phase > 0.4f && mrec[num].Phase < 0.975 )
			 continue;
		    /* avoid double entries */
		    nj = 0;
		    while ( nj < num && mrec[nj].Phase != mrec[num].Phase ) 
			 nj++;
		    if ( nj == num ) num++;
	       } else
		    num++;
	  } while( ++nrr < num_rec );
	  (void) fclose( fp );
     }
     if ( num == 0 ) goto done;
     found = TRUE;
/*
 * calculate average dark
 */
     if ( channel == 0 ) {
	  for ( nj = 0; nj < num; nj++ ) {
	       for ( nr = 0; nr < SCIENCE_CHANNELS; nr++ )
		    pet[nr] += mrec[nj].Pet[nr * CHANNEL_SIZE];
	       for ( nr = 0; nr < SCIENCE_PIXELS; nr++ ) {
		    if ( isnormal(mrec[nj].Signal[nr]) ) {
			 darkSignal[nr] += mrec[nj].Signal[nr];
			 if ( sdmf_mimic_hanss )
			      darkNoise[nr] += mrec[nj].Noise[nr];
			 else
			      darkNoise[nr] += 
				   (mrec[nj].Noise[nr] * mrec[nj].Noise[nr]);
			 total[nr]++;
		    }
	       }
	  }
	  for ( nr = 0; nr < SCIENCE_CHANNELS; nr++ )
	       pet[nr] /= num;
	  for ( nr = 0; nr < SCIENCE_PIXELS; nr++ ) {
	       if ( total[nr] > 0 ) {
		    darkSignal[nr] /= total[nr];
		    if ( sdmf_mimic_hanss )
			 darkNoise[nr] /= total[nr];
		    else
			 darkNoise[nr] = sqrtf( darkNoise[nr] / total[nr] );
	       } else
		    darkNoise[nr] = -1.f;
	  }
     } else {
	  const size_t offs = (channel-1) * CHANNEL_SIZE;

	  for ( nj = 0; nj < num; nj++ ) {
	       *pet += mrec[nj].Pet[offs+nr];
	       for ( nr = 0; nr < CHANNEL_SIZE; nr++ ) {
		    register size_t nrr = nr + offs;

		    if ( isnormal(mrec[nj].Signal[nrr]) ) {
			 darkSignal[nr] += mrec[nj].Signal[nrr];
			 if ( sdmf_mimic_hanss )
			      darkNoise[nr] += mrec[nj].Noise[nrr];
			 else
			      darkNoise[nr] += 
				   (mrec[nj].Noise[nrr] * mrec[nj].Noise[nrr]);
			 total[nr]++;
		    }
	       }
	  }
	  *pet /= num;
	  for ( nr = 0; nr < CHANNEL_SIZE; nr++ ) {
	       if ( total[nr] > 0 ) {
		    darkSignal[nr] /= total[nr];
		    if ( sdmf_mimic_hanss )
			 darkNoise[nr] /= total[nr];
		    else
			 darkNoise[nr] = sqrtf( darkNoise[nr] / total[nr] );
	       } else
		    darkNoise[nr] = -1.f;
	  }
     }
done:
     free( moni_fl );
     return found;
}

/*+++++++++++++++++++++++++ SDMF version 3.0 ++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_StateDark_30
.PURPOSE     obtain dark correction parameters (SDMF v3.0)
.INPUT/OUTPUT
  call as    found = SDMF_get_StateDark_30( stateID, channel, absOrbit, 
                                            pet, darkSignal, darkNoise );
     input:  
           unsigned char  stateID  :  state ID
	   unsigned short channel  :  channel ID or zero for all channels
           unsigned short absOrbit :  orbit number
    output:  
           float *pet              :  channel Pixel Exposure Time (sec)
	   float *darkSignal       :  dark signal (BU, memory/non-linearity)
	   float *darkNoise        :  dark noise (BU, standard deviation)

.RETURNS     solution found (True or False)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
bool SDMF_get_StateDark_30( unsigned char stateID, unsigned short channel, 
			    unsigned short absOrbit, float *pet, 
			    float *darkSignal, float *darkNoise )
{
     register int nr, np;

     register unsigned short ival;

     hid_t   fid = -1;
     hid_t   gid = -1;

     int  delta;
     int  numIndx;
     int  metaIndx[MAX_NUM_META_INDEX];

     char str_msg[SHORT_STRING_LENGTH];
     char sdmf_db[MAX_STRING_LENGTH];
     char grpName[STR_SZ_H5_GRP];

     unsigned short num_signal[SCIENCE_PIXELS];
     unsigned short num_noise[SCIENCE_PIXELS];
     float rbuff[SCIENCE_PIXELS];

     struct mtbl_calib_rec *mtbl = NULL;

     struct scia_memcorr memcorr = {{0,0}, NULL};
     struct scia_nlincorr nlcorr = {{0,0}, NULL, NULL};

     const int orbit = (int) absOrbit;
     const int pixelRange[2] = { 
	  (channel == 0) ? 0 : (channel-1) * CHANNEL_SIZE,
	  (channel == 0) ? SCIENCE_PIXELS-1 : channel * CHANNEL_SIZE - 1
     };
     const int  MaxDiffOrbitNumber = 14;
/*
 * initialize returned values
 */
     if ( channel == 0 ) {
	  (void) memset( pet, 0, SCIENCE_CHANNELS * sizeof(float) );
	  (void) memset( num_signal, 0, SCIENCE_PIXELS * sizeof(short) );
	  (void) memset( num_noise, 0, SCIENCE_PIXELS * sizeof(short) );
	  (void) memset( darkSignal, 0, SCIENCE_PIXELS * sizeof(float) );
	  (void) memset( darkNoise, 0, SCIENCE_PIXELS * sizeof(float) );
     } else {
	  *pet = 0.f;
	  (void) memset( num_signal, 0, CHANNEL_SIZE * sizeof(short) );
	  (void) memset( num_noise, 0, CHANNEL_SIZE * sizeof(short) );
	  (void) memset( darkSignal, 0, CHANNEL_SIZE * sizeof(float) );
	  (void) memset( darkNoise, 0, CHANNEL_SIZE * sizeof(float) );
     }
/*
 * open SDMF dark database
 */
     (void) snprintf( sdmf_db, MAX_STRING_LENGTH, "%s/%s", 
                      SDMF_PATH("3.0"), "sdmf_extract_calib.h5" );
     H5E_BEGIN_TRY {
	  fid = H5Fopen( sdmf_db, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( fid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_FILE, sdmf_db );

     (void) snprintf( grpName, STR_SZ_H5_GRP, "State_%02hhu", stateID );
     H5E_BEGIN_TRY {
	  gid = H5Gopen( fid, grpName, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( gid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, grpName );
/*
 * select records and read metaTable info
 */
     numIndx = MAX_NUM_META_INDEX;

     delta = 0;
     do {
	  (void) SDMF_get_metaIndex( gid, orbit+delta, &numIndx, metaIndx );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_get_metaIndex" );
	  if ( numIndx == 0 ) 
	       delta = (delta > 0) ? (-delta) : (1 - delta);
	  else
	       break;
     } while ( abs(delta) < MaxDiffOrbitNumber );

     if ( numIndx < 1 ) {
	  (void) snprintf( str_msg, SHORT_STRING_LENGTH, 
			   "\n\tSDMF Dark data (v3.0) no applicable data found for orbit: %d",
			   orbit );
	  NADC_ERROR( NADC_ERR_NONE, str_msg );
	  return FALSE;
     } else {
	  (void) snprintf( str_msg, SHORT_STRING_LENGTH, 
			   "\n\tapplied SDMF Dark data (v3.0) of orbit: %-d (%d)",
			   orbit+delta, numIndx );
	  NADC_ERROR( NADC_ERR_NONE, str_msg );
     }
/*
 * read metaTable entry of orbit
 */
     SDMF30_rd_metaTable( gid, &numIndx, metaIndx, &mtbl );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_rd_metaTable" );
/*
 * read memory & non-linearity correction tables
 */
     SCIA_RD_H5_MEM( &memcorr );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, "SCIA_RD_H5_MEM" );
     SCIA_RD_H5_NLIN( &nlcorr );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, "SCIA_RD_H5_NLIN" );
/*
 * read data from available Dark states and calculate average
 */
     for ( nr = 0; nr < numIndx; nr++ ) {
	  if ( channel < 6 && mtbl[nr].saaFlag == 1 ) continue;
	  if ( mtbl[nr].orbitPhase > 0.4f && mtbl[nr].orbitPhase < 0.975 )
	       continue;

	  SDMF_rd_float_Array( gid, "readoutPet", 1, metaIndx+nr, 
			       pixelRange, rbuff );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, "readoutPet" );
	  if ( channel == 0 ) {
	       for ( np = 0; np < SCIENCE_CHANNELS; np++ )
		    pet[np] = rbuff[np * CHANNEL_SIZE];
	  } else
	       *pet = rbuff[(channel-1) * CHANNEL_SIZE];
	  
	  SDMF_rd_float_Array( gid, "readoutMean", 1, metaIndx+nr, 
			       pixelRange, rbuff );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, "readoutPet" );
	  if ( channel == 0 ) {
	       for ( np = 0; np < SCIENCE_PIXELS; np++ ) {
		    if ( isnormal( rbuff[np] ) ) {
			 darkSignal[np] += rbuff[np];
			 num_signal[np]++;
		    }
	       }
	  } else {
	       for ( np = 0; np < CHANNEL_SIZE; np++ ) {
		    if ( isnormal( rbuff[np] ) ) {
			 darkSignal[np] += rbuff[np];
			 num_signal[np]++;
		    }
	       }
	  }
	  
	  SDMF_rd_float_Array( gid, "readoutNoise", 1, metaIndx+nr, 
			       pixelRange, rbuff );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, "readoutPet" );
	  if ( channel == 0 ) {
	       for ( np = 0; np < SCIENCE_PIXELS; np++ ) {
		    if ( isnormal( rbuff[np] ) ) {
			 darkNoise[np] += (rbuff[np] * rbuff[np]);
			 num_noise[np]++;
		    }
	       }
	  } else {
	       for ( np = 0; np < CHANNEL_SIZE; np++ ) {
		    if ( isnormal( rbuff[np] ) ) {
			 darkNoise[np] += (rbuff[np] * rbuff[np]);
			 num_noise[np]++;
		    }
	       }
	  }
     }
     if ( channel == 0 ) {
	  for ( np = 0; np < SCIENCE_PIXELS; np++ ) {
	       unsigned short ichan = (unsigned short)(np / CHANNEL_SIZE);
	       unsigned short indx = (unsigned short) nlcorr.curve[np];

	       if ( num_signal[np] > 0 ) {
		    darkSignal[np] /= num_signal[np];
		    ival = __ROUNDf_us( darkSignal[np] );
		    if ( ichan < 5 )
			 darkSignal[np] -= memcorr.matrix[ichan][ival];
		    else
			 darkSignal[np] -= nlcorr.matrix[indx][ival];
	       }
	       if ( num_noise[np] > 0 )
		    darkNoise[np] = sqrtf(darkNoise[np] / num_noise[np] );
	       else
		    darkNoise[np] = -1.f;
	  }
     } else {
	  for ( np = 0; np < CHANNEL_SIZE; np++ ) {
	       unsigned short indx = (unsigned short) 
		    nlcorr.curve[np + (channel-1) * CHANNEL_SIZE];

	       if ( num_signal[np] > 0 ) {
		    darkSignal[np] /= num_signal[np];
		    ival = __ROUNDf_us( darkSignal[np] );
		    if ( channel < 6 )
			 darkSignal[np] -= memcorr.matrix[channel-1][ival];
		    else
			 darkSignal[np] -= nlcorr.matrix[indx][ival];
	       }
	       if ( num_noise[np] > 0 ) 
		    darkNoise[np] = sqrtf( darkNoise[np] / num_noise[np] );
	       else
		    darkNoise[np] = -1.f;
	  }
     }
done:
     SCIA_FREE_H5_MEM( &memcorr );
     SCIA_FREE_H5_NLIN( &nlcorr );

     if ( mtbl != NULL ) free( mtbl );
     if ( gid > 0 ) H5Gclose( gid );
     if ( fid > 0 ) H5Fclose( fid );

     return TRUE;
}

/*+++++++++++++++++++++++++ SDMF version 3.1 ++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_StateDark
.PURPOSE     obtain dark correction parameters (SDMF v3.1)
.INPUT/OUTPUT
  call as    found = SDMF_get_StateDark( stateID, channel, absOrbit, 
                                         pet, darkSignal, darkNoise );
     input:  
           unsigned char  stateID  :  state ID
           unsigned short absOrbit :  orbit number
	   unsigned short channel  :  channel ID or zero for all channels
    output:  
           float *pet              :  channel Pixel Exposure Time (sec)
	   float *darkSignal       :  dark signal (BU, memory/non-linearity)
	   float *darkNoise        :  dark noise (BU, absolute mean deviation)

.RETURNS     solution found (True or False)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
bool SDMF_get_StateDark( unsigned char stateID, unsigned short channel, 
			 unsigned short absOrbit, float *pet, 
			 float *darkSignal, float *darkNoise )
{
     register int delta = 0;

     bool found = FALSE;

     hid_t   fid = -1;
     hid_t   gid = -1;
     herr_t  stat;
     hsize_t nrecords;

     int   orbit_max, metaIndx;

     struct mtbl_dark2_rec mtbl[1];

     char str_msg[SHORT_STRING_LENGTH];
     char sdmf_db[MAX_STRING_LENGTH];
     char grpName[STR_SZ_H5_GRP];

     const size_t mtbl_size = sizeof( struct mtbl_dark2_rec );

     const size_t mtbl_offs[DIM_MTBL_DARK2] = {
	  HOFFSET( struct mtbl_dark2_rec, julianDay ),
	  HOFFSET( struct mtbl_dark2_rec, entryDate ),
	  HOFFSET( struct mtbl_dark2_rec, absOrbit ),
	  HOFFSET( struct mtbl_dark2_rec, quality ),
	  HOFFSET( struct mtbl_dark2_rec, stateCount ),
	  HOFFSET( struct mtbl_dark2_rec, statesSelected ),
	  HOFFSET( struct mtbl_dark2_rec, orbitRange ),
	  HOFFSET( struct mtbl_dark2_rec, obmTemp ),
	  HOFFSET( struct mtbl_dark2_rec, detTemp )
     };
     mtbl->stateCount = 0;
/*
 * initialize returned values
 */
     if ( channel == 0 ) {
	  (void) memset( pet, 0, SCIENCE_CHANNELS * sizeof(float) );
	  (void) memset( darkSignal, 0, SCIENCE_PIXELS * sizeof(float) );
	  (void) memset( darkNoise, 0, SCIENCE_PIXELS * sizeof(float) );
     } else {
	  *pet = 0.f;
	  (void) memset( darkSignal, 0, CHANNEL_SIZE * sizeof(float) );
	  (void) memset( darkNoise, 0, CHANNEL_SIZE * sizeof(float) );
     }
/*
 * open SDMF dark database
 */
     (void) snprintf( sdmf_db, MAX_STRING_LENGTH, "%s/%s", 
                      SDMF_PATH("3.1"), "sdmf_dark.h5" );
     H5E_BEGIN_TRY {
	  fid = H5Fopen( sdmf_db, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( fid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_FILE, sdmf_db );

     (void) snprintf( grpName, STR_SZ_H5_GRP, "State_%02hhu", stateID );
     H5E_BEGIN_TRY {
	  gid = H5Gopen( fid, grpName, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( gid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, grpName );
/*
 * search for good quality fitted dark
 */
     (void) H5TBget_table_info( gid, "metaTable", NULL, &nrecords );
     orbit_max = (unsigned short) nrecords;

     metaIndx = (int) absOrbit - 1;
     do {
	  metaIndx = (int) absOrbit - 1 + delta;
	  if ( metaIndx <= 0 || metaIndx >= orbit_max ) break;

	  stat = H5TBread_records( gid, "metaTable", metaIndx, 1,
				   mtbl_size, mtbl_offs, mtbl_dark2_sizes, 
				   mtbl );
	  if ( stat < 0 )
               NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_rd_metaTable" );
	  delta = (delta > 0) ? (-delta) : (1 - delta);
     } while ( mtbl->stateCount < 3 );
     if ( mtbl->stateCount < 3 ) goto done;
     found = TRUE;

     (void) snprintf( str_msg, SHORT_STRING_LENGTH, 
		      "\n\tapplied SDMF Dark data (v3.1) of orbit: %-hu (%-hu)",
		      mtbl->absOrbit, mtbl->stateCount );
     NADC_ERROR( NADC_ERR_NONE, str_msg );
/*
 * read dark parameters from SDMF database
 */
     if ( channel == 0 ) {
	  NADC_RD_H5_EArray_float( gid, "darkSignal", SCIENCE_PIXELS, 
				   1, &metaIndx, darkSignal );
	  NADC_RD_H5_EArray_float( gid, "darkNoise", SCIENCE_PIXELS, 
				   1, &metaIndx, darkNoise );
     } else {
	  const size_t offs = (channel-1) * CHANNEL_SIZE;
	  const size_t nbytes = CHANNEL_SIZE * sizeof(float);

	  float rbuff[SCIENCE_PIXELS];
 
	  NADC_RD_H5_EArray_float( gid, "darkSignal", SCIENCE_PIXELS, 
				   1, &metaIndx, rbuff );
 	  (void) memcpy( darkSignal, rbuff+offs, nbytes );
	  NADC_RD_H5_EArray_float( gid, "darkNoise", SCIENCE_PIXELS, 
				   1, &metaIndx, rbuff );
 	  (void) memcpy( darkNoise, rbuff+offs, nbytes );
     }
done:
     if ( gid > 0 ) H5Gclose( gid );
     if ( fid > 0 ) H5Fclose( fid );

     return found;
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
#ifdef TEST_PROG
bool Use_Extern_Alloc = FALSE;

int main( int argc, char *argv[] )
{
     register unsigned short np = 0;

     unsigned char  stateID;
     unsigned short orbit;
     unsigned short channel = 0;

     bool fnd_24, fnd_30, fnd_31;

     float pet_24[SCIENCE_PIXELS], dc_24[SCIENCE_PIXELS], 
	  dc_err_24[SCIENCE_PIXELS];
     float pet_30[SCIENCE_PIXELS], dc_30[SCIENCE_PIXELS], 
	  dc_err_30[SCIENCE_PIXELS];
     float pet_31[SCIENCE_PIXELS], dc_31[SCIENCE_PIXELS], 
	  dc_err_31[SCIENCE_PIXELS];
/*
 * initialization of command-line parameters
 */
     if ( argc <= 2 || (argc > 1 && strncmp( argv[1], "-h", 2 ) == 0) ) {
	  (void) fprintf( stderr, "Usage: %s orbit state [channel]\n", argv[0] );
          exit( EXIT_FAILURE );
     }
     orbit = (unsigned short) atoi( argv[1] );
     stateID = (unsigned char) atoi( argv[2] );
     if ( argc == 4 ) channel = (unsigned short) atoi( argv[3] );
/*
 * 
 */
     fnd_24 = SDMF_get_StateDark_24( stateID, channel, orbit, 
				     pet_24, dc_24, dc_err_24 );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_get_StateDark_24" );
     if ( ! fnd_24 ) (void) fprintf( stderr, "# no solution for SDMF v2.4\n" );
     fnd_30 = SDMF_get_StateDark_30( stateID, channel, orbit,
     				     pet_30, dc_30, dc_err_30 );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_get_StateDark_30" );
     if ( ! fnd_30 ) (void) fprintf( stderr, "# no solution for SDMF v3.0\n" );
     fnd_31 = SDMF_get_StateDark( stateID, channel, orbit,
     				  pet_31, dc_31, dc_err_31 );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, "SDMF_get_StateDark_31" );
     if ( ! fnd_31 ) (void) fprintf( stderr, "# no solution for SDMF v3.1\n" );
/*
 * 
 */
     if ( ! (fnd_24 || fnd_30 || fnd_31) ) goto done;
     do {
	  (void) printf( "%5hu", np );
	  if ( fnd_24 ) {
	       (void) printf( "%12.6g %12.6g", dc_24[np], dc_err_24[np] );
	  }
	  if ( fnd_30 ) {
	       (void) printf( "%12.6g %12.6g", dc_30[np], dc_err_30[np] );
	  }
	  if ( fnd_31 ) {
	       (void) printf( "%12.6g %12.6g", dc_31[np], dc_err_31[np] );
	  }
	  (void) printf( "\n" );
     } while( ++np < ((channel == 0) ? SCIENCE_PIXELS : CHANNEL_SIZE) );
done:
     NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL )
          exit( EXIT_FAILURE );
     else
          exit( EXIT_SUCCESS );
}
#endif /* TEST_PROG */
