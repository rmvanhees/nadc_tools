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

.IDENTifer   SDMF_get_FittedDark
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF - Dark calibration
.LANGUAGE    ANSI C
.PURPOSE     obtain (fitted) dark correction parameters
.COMMENTS    contains SDMF_get_FittedDark, SDMF_get_FittedDark_30
                      SDMF_get_FittedDark_24
.ENVIRONment None
.VERSION     2.3     10-Jan-2013   optionally use NRT entries, RvH
             2.2     15-May-2012   greatly improved v2.4 implementation, RvH
             2.1     14-May-2012   added test program, RvH
             2.0     18-Mar-2011   back-ported SDMF v2.4 & 3.0, RvH
             1.2     20-Feb-2011   added parameter meanNoise, RvH
             1.1     19-Feb-2011   added parameter probabilityFit, RvH
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

#include <hdf5.h>
#include <hdf5_hl.h>

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
.IDENTifer   SDMF_get_FittedDark_24
.PURPOSE     obtain dark correction parameters  (SDMF v2.4.1)
.INPUT/OUTPUT
  call as    found = SDMF_get_FittedDark_24( channel, orbit, analogOffs, 
					     darkCurrent, analogOffsError, 
					     darkCurrentError, chiSquareFit );
     input:
	   unsigned short channel  :  channel ID or zero for all channels
           unsigned short absOrbit :  orbit number
 in/output:
           float *analogOffs       :  analog offset (BU)
           float *darkCurrent      :  leakage current (BU/s)
           float *analogOffsError  :  analog offset error (or NULL)
           float *darkCurrentError :  leakage current error (or NULL)
           float *chiSquareFit     :  chiSquare of fit (or NULL)

.RETURNS     solution found (True or False)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
bool SDMF_get_FittedDark_24( unsigned short channel, unsigned short absOrbit,
			     float *analogOffs, float *darkCurrent, 
			     float *analogOffsError, float *darkCurrentError, 
			     float *chiSquareFit )
{
     char   dark_fl[MAX_STRING_LENGTH];

     FILE   *dark_fp;

     int    quality;
     float  rbuff[SCIENCE_PIXELS];

     const int  orbit = (int) absOrbit;
     const long offs = (channel == 0) ? 0 : (channel-1) * CHANNEL_SIZE;
     const long sz_chan_byte = CHANNEL_SIZE * ENVI_FLOAT;
     const long sz_ds_byte = SCIENCE_PIXELS * ENVI_FLOAT;
     const long offset_Quality = (long) 3 * sizeof(int) + sizeof(double)
          + sizeof(float) + SCIENCE_CHANNELS * sizeof(float);
/*
 * initialize returned values
 */
     if ( channel == 0 ) {
	  (void) memset( analogOffs, 0, sz_ds_byte );
	  (void) memset( darkCurrent, 0, sz_ds_byte );
	  if ( analogOffsError != NULL )
	       (void) memset( analogOffsError, 0, sz_ds_byte );
	  if ( darkCurrentError != NULL )
	       (void) memset( darkCurrentError, 0, sz_ds_byte );
	  if ( chiSquareFit != NULL ) 
	       (void) memset( chiSquareFit, 0, sz_ds_byte );
     } else {
	  (void) memset( analogOffs, 0, sz_chan_byte );
	  (void) memset( darkCurrent, 0, sz_chan_byte );
	  if ( analogOffsError != NULL )
	       (void) memset( analogOffsError, 0, sz_chan_byte );
	  if ( darkCurrentError != NULL )
	       (void) memset( darkCurrentError, 0, sz_chan_byte );
	  if ( chiSquareFit != NULL )
	       (void) memset( chiSquareFit, 0, sz_chan_byte );
     }
/*
 * find file with dark signals, requirements:
 *  - orbit number within a range MAX_DiffOrbitNumber
 *  - quality number at least equal to MIN_QualityNumber
 */
     if ( ! SDMF_get_fileEntry( SDMF24_FITTED, orbit, dark_fl ) ) 
	  return FALSE;
/*
 * read dark parameters
 */
     if ( (dark_fp = fopen( dark_fl, "r" )) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE, dark_fl );
     (void) fseek( dark_fp, offset_Quality, SEEK_SET );
     if ( fread( &quality, ENVI_INT, 1, dark_fp ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "quality" );
     if ( channel == 0 ) {
	  if ( fread( analogOffs, sz_ds_byte, 1, dark_fp ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "analogOffs" );
	  if ( fread( darkCurrent, sz_ds_byte, 1, dark_fp ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "darkCurrent" );
	  if ( analogOffsError != NULL ) {
	       if ( fread( analogOffsError, sz_ds_byte, 1, dark_fp ) != 1 )
		    NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "analogOffsError" );
	  } else
	       (void) fseek( dark_fp, sz_ds_byte, SEEK_CUR );

	  if ( darkCurrentError != NULL ) {
	       if ( fread( darkCurrentError, sz_ds_byte, 1, dark_fp ) != 1 )
		    NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "darkCurrentError" );
	  } else
	       (void) fseek( dark_fp, sz_ds_byte, SEEK_CUR );
	  if ( chiSquareFit != NULL ) {
	       if ( fread( chiSquareFit, sz_ds_byte, 1, dark_fp ) != 1 )
		    NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "chiSquareFit" );
	  }
     } else {
	  if ( fread( rbuff, sz_ds_byte, 1, dark_fp ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "analogOffs" );
	  (void) memcpy( analogOffs, rbuff+offs, sz_chan_byte );
	  if ( fread( rbuff, sz_ds_byte, 1, dark_fp ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "darkCurrent" );
	  (void) memcpy( darkCurrent, rbuff+offs, sz_chan_byte );
	  if ( analogOffsError != NULL ) {
	       if ( fread( rbuff, sz_ds_byte, 1, dark_fp ) != 1 )
		    NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "analogOffsError" );
	       (void) memcpy( analogOffsError, rbuff+offs, sz_chan_byte );
	  } else
	       (void) fseek( dark_fp, sz_ds_byte, SEEK_CUR );

	  if ( darkCurrentError != NULL ) {
	       if ( fread( rbuff, sz_ds_byte, 1, dark_fp ) != 1 )
		    NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "darkCurrentError" );
	       (void) memcpy( darkCurrentError, rbuff+offs, sz_chan_byte );
	  } else
	       (void) fseek( dark_fp, sz_ds_byte, SEEK_CUR );
	  if ( chiSquareFit != NULL ) {
	       if ( fread( rbuff, sz_ds_byte, 1, dark_fp ) != 1 )
		    NADC_GOTO_ERROR( NADC_ERR_FILE_RD, "chiSquareFit" );
	       (void) memcpy( chiSquareFit, rbuff+offs, sz_chan_byte );
	  }
     }
     (void) fclose( dark_fp );

     return TRUE;
done:
     return FALSE;
}

/*+++++++++++++++++++++++++ SDMF version 3.0 ++++++++++++++++++++++++++++++*/
static
int getMetaIndx( hid_t fid, int orbit, /*@out@*/ int *fnd_orbit )
{
     const int   MAX_DiffOrbitNumber = 21;
     const short MIN_QualityNumber   = 40;
     const short GOOD_QualityNumber  = 70;

     register int delta = 0;

     int   metaIndx     = -1;
     int   fnd_metaIndx = -1;
     short fnd_quality  = MIN_QualityNumber;

     struct mtbl_dark_rec *mtbl;

     *fnd_orbit = -1;
     do {
          int numIndx = 1;

          (void) SDMF_get_metaIndex( fid, orbit + delta, &numIndx, &metaIndx );
          if ( IS_ERR_STAT_FATAL )
               NADC_GOTO_ERROR(NADC_ERR_FATAL, "SDMF_get_metaIndex");
          if ( metaIndx > 0 ) {
               SDMF_rd_darkTable( fid, &numIndx, &metaIndx, &mtbl );
               if ( ! mtbl->saaFlag && mtbl->quality >= fnd_quality ) {
                    fnd_quality  = mtbl->quality;
                    *fnd_orbit   = mtbl->absOrbit;
                    fnd_metaIndx = metaIndx;
               }
               free( mtbl );
               if ( fnd_quality >= GOOD_QualityNumber ) break;
          } 
          delta = (delta > 0) ? (-delta) : (1 - delta);
     } while ( abs( delta ) <= MAX_DiffOrbitNumber );
 done:
     return fnd_metaIndx;
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_FittedDark_30
.PURPOSE     obtain dark correction parameters (SDMF v3.0)
.INPUT/OUTPUT
  call as    found = SDMF_get_FittedDark_30( channel, orbit, 
                                             analogOffs, darkCurrent, 
					     analogOffsError, darkCurrentError,
					     chiSquareFit );
     input:
	   unsigned short channel  :  channel ID or zero for all channels
           unsigned short absOrbit :  orbit number
 in/output:
           float *analogOffs       :  analog offset (BU)
           float *darkCurrent      :  leakage current (BU/s)
           float *analogOffsError  :  analog offset error (or NULL)
           float *darkCurrentError :  leakage current error (or NULL)
           float *chiSquareFit     :  chiSquare of fit (or NULL)

.RETURNS     solution found (True or False)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
bool SDMF_get_FittedDark_30( unsigned short channel, unsigned short absOrbit,
			     float *analogOffs, float *darkCurrent, 
			     float *analogOffsError, float *darkCurrentError,
			     float *chiSquareFit )
{
     hid_t  fid = -1;

     int    fnd_orbit;
     int    metaIndx; 

     char str_msg[MAX_STRING_LENGTH];
     char sdmf_db[MAX_STRING_LENGTH];

     const int orbit = (int) absOrbit;
/*
 * initialize returned values
 */
     if ( channel == 0 ) {
	  const size_t nr_bytes = SCIENCE_PIXELS * sizeof(float);

	  (void) memset( analogOffs, 0, nr_bytes );
	  (void) memset( darkCurrent, 0, nr_bytes );
	  if ( analogOffsError != NULL )
	       (void) memset( analogOffsError, 0, nr_bytes );
	  if ( darkCurrentError != NULL )
	       (void) memset( darkCurrentError, 0, nr_bytes );
	  if ( chiSquareFit != NULL ) 
	       (void) memset( chiSquareFit, 0, nr_bytes );
     } else {
	  const size_t nr_bytes = CHANNEL_SIZE * sizeof(float);

	  (void) memset( analogOffs, 0, nr_bytes );
	  (void) memset( darkCurrent, 0, nr_bytes );
	  if ( analogOffsError != NULL )
	       (void) memset( analogOffsError, 0, nr_bytes );
	  if ( darkCurrentError != NULL )
	       (void) memset( darkCurrentError, 0, nr_bytes );
	  if ( chiSquareFit != NULL )
	       (void) memset( chiSquareFit, 0, nr_bytes );
     }
/*
 * open SDMF Dark database
 */
     (void) snprintf( sdmf_db, MAX_STRING_LENGTH, "%s/%s", 
                      SDMF_PATH("3.0"), "sdmf_dark.h5" );
     H5E_BEGIN_TRY {
	  fid = H5Fopen( sdmf_db, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( fid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_FILE, sdmf_db );
/*
 * get index to metaTable records or this state
 */
     metaIndx = getMetaIndx( fid, orbit, &fnd_orbit );
     if ( metaIndx == -1 ) goto done;
     (void) snprintf( str_msg, MAX_STRING_LENGTH, 
		      "\n\tapplied SDMF Dark data (v3.0) of orbit: %-d", 
		      fnd_orbit );
     NADC_ERROR( NADC_ERR_NONE, str_msg );
/*
 * read dark parameters of all channels
 */
     if ( channel == 0 ) {
	  SDMF_rd_float_Array( fid, "analogOffset", 
			       1, &metaIndx, NULL, analogOffs );
          SDMF_rd_float_Array( fid, "darkCurrent", 
			       1, &metaIndx, NULL, darkCurrent );
          if ( analogOffsError != NULL )
	       SDMF_rd_float_Array( fid, "analogOffsetError", 
				    1, &metaIndx, NULL, analogOffsError );
	  if ( darkCurrentError != NULL )
	       SDMF_rd_float_Array( fid, "darkCurrentError", 
				    1, &metaIndx, NULL, darkCurrentError );
	  if ( chiSquareFit != NULL )
	       SDMF_rd_float_Array( fid, "chiSquareFit", 
				    1, &metaIndx, NULL, chiSquareFit );
     } else {                            /* read dark parameters per channel */
          int pixelRange[] = { (channel-1) * CHANNEL_SIZE, 
                               channel * CHANNEL_SIZE - 1 };

          SDMF_rd_float_Array( fid, "analogOffset", 
			       1, &metaIndx, pixelRange, analogOffs );
          SDMF_rd_float_Array( fid, "darkCurrent", 
			       1, &metaIndx, pixelRange, darkCurrent );
          if ( analogOffsError != NULL )
	       SDMF_rd_float_Array( fid, "analogOffsetError", 
				    1, &metaIndx, pixelRange, analogOffsError );
	  if ( darkCurrentError != NULL )
	       SDMF_rd_float_Array( fid, "darkCurrentError", 
				    1, &metaIndx, pixelRange, darkCurrentError );
	  if ( chiSquareFit != NULL )
	       SDMF_rd_float_Array( fid, "chiSquareFit", 
				    1, &metaIndx, pixelRange, chiSquareFit );
     }
/*
 * close SDMF Dark database
 */
     if ( fid != -1 ) (void) H5Fclose( fid );
     return TRUE;
done:
     return FALSE;
}

/*+++++++++++++++++++++++++ SDMF version 3.1 ++++++++++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_FittedDark
.PURPOSE     obtain dark correction parameters (SDMF v3.1)
.INPUT/OUTPUT
  call as    found = SDMF_get_FittedDark( channel, orbit, 
                                          analogOffs, darkCurrent, 
					  analogOffsError, darkCurrentError,
					  meanNoise, chiSquareFit, 
					  probabilityFit );
     input:
	   unsigned short channel  :  channel ID or zero for all channels
           unsigned short absOrbit :  orbit number
 in/output:
           float *analogOffs       :  analog offset (BU)
           float *darkCurrent      :  leakage current (BU/s)
           float *analogOffsError  :  analog offset error (or NULL)
           float *darkCurrentError :  leakage current error (or NULL)
           float *meanNoise        :  mean noise (or NULL)
           float *chiSquareFit     :  chiSquare of fit (or NULL)
           float *probabilityFit   :  probability of fit (or NULL)

.RETURNS     solution found (True or False)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
bool SDMF_get_FittedDark( unsigned short absOrbit, unsigned short channel,
			  float *analogOffs, float *darkCurrent, 
			  float *analogOffsError, float *darkCurrentError,
			  float *meanNoise, float *chiSquareFit, 
			  float *probabilityFit )
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

     const int    orbit = (int) absOrbit;
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
	  const size_t nr_bytes = SCIENCE_PIXELS * sizeof(float);

	  (void) memset( analogOffs, 0, nr_bytes );
	  (void) memset( darkCurrent, 0, nr_bytes );
	  if ( analogOffsError != NULL )
	       (void) memset( analogOffsError, 0, nr_bytes );
	  if ( darkCurrentError != NULL )
	       (void) memset( darkCurrentError, 0, nr_bytes );
	  if ( meanNoise != NULL )
	       (void) memset( meanNoise, 0, nr_bytes );
	  if ( chiSquareFit != NULL )
	       (void) memset( chiSquareFit, 0, nr_bytes );
	  if ( probabilityFit != NULL )
	       (void) memset( probabilityFit, 0, nr_bytes );
     } else {
	  const size_t nr_bytes = CHANNEL_SIZE * sizeof(float);

	  (void) memset( analogOffs, 0, nr_bytes );
	  (void) memset( darkCurrent, 0, nr_bytes );
	  if ( analogOffsError != NULL )
	       (void) memset( analogOffsError, 0, nr_bytes );
	  if ( darkCurrentError != NULL )
	       (void) memset( darkCurrentError, 0, nr_bytes );
	  if ( meanNoise != NULL )
	       (void) memset( meanNoise, 0, nr_bytes );
	  if ( chiSquareFit != NULL )
	       (void) memset( chiSquareFit, 0, nr_bytes );
	  if ( probabilityFit != NULL )
	       (void) memset( probabilityFit, 0, nr_bytes );
     }
/*
 * open SDMF Dark database
 */
     (void) snprintf( sdmf_db, MAX_STRING_LENGTH, "%s/%s", 
                      SDMF_PATH("3.1"), "sdmf_dark.h5" );
     H5E_BEGIN_TRY {
	  fid = H5Fopen( sdmf_db, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( fid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_FILE, sdmf_db );

     H5E_BEGIN_TRY {
	  gid = H5Gopen( fid, "/DarkFit", H5P_DEFAULT );
	  } H5E_END_TRY;
     if ( gid < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, "/DarkFit" );
/*
 * search for good quality fitted dark
 */
     (void) H5TBget_table_info( gid, "metaTable", NULL, &nrecords );
     orbit_max = (unsigned short) nrecords;

     metaIndx = orbit - 1;
     do {
	  metaIndx = orbit - 1 + delta;
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
		      "\n\tapplied SDMF Dark data (v3.1) of orbit: %-hu [%-hu]",
		      mtbl->absOrbit, mtbl->stateCount );
     NADC_ERROR( NADC_ERR_NONE, str_msg );
/*
 * read dark parameters from SDMF database
 */
     if ( channel == 0 ) {
	  NADC_RD_H5_EArray_float( gid, "analogOffset", SCIENCE_PIXELS, 
				   1, &metaIndx, analogOffs );
	  NADC_RD_H5_EArray_float( gid, "darkCurrent", SCIENCE_PIXELS, 
				   1, &metaIndx, darkCurrent );
	  if ( analogOffsError != NULL ) {
	       NADC_RD_H5_EArray_float( gid, "analogOffsetError", 
					SCIENCE_PIXELS, 1, &metaIndx, 
					analogOffsError );
	  }
	  if ( darkCurrentError != NULL ) {
	       NADC_RD_H5_EArray_float( gid, "darkCurrentError", 
					SCIENCE_PIXELS, 1, &metaIndx, 
					darkCurrentError );
	  }
	  if ( meanNoise != NULL ) {
	       NADC_RD_H5_EArray_float( gid, "meanNoise", SCIENCE_PIXELS, 
					1, &metaIndx, meanNoise );
	  }
	  if ( chiSquareFit != NULL ) {
	       NADC_RD_H5_EArray_float( gid, "chiSquareFit", SCIENCE_PIXELS, 
					1, &metaIndx, chiSquareFit );
	  }
	  if ( probabilityFit != NULL ) {
	       NADC_RD_H5_EArray_float( gid, "probabilityFit", SCIENCE_PIXELS, 
					1, &metaIndx, probabilityFit );
	  }
     } else {
	  const size_t offs = (channel-1) * CHANNEL_SIZE;
	  const size_t nbytes = CHANNEL_SIZE * sizeof(float);

	  float rbuff[SCIENCE_PIXELS];
 
	  NADC_RD_H5_EArray_float( gid, "analogOffset", SCIENCE_PIXELS, 
				   1, &metaIndx, rbuff );
 	  (void) memcpy( analogOffs, rbuff+offs, nbytes );
	  NADC_RD_H5_EArray_float( gid, "darkCurrent", SCIENCE_PIXELS, 
				   1, &metaIndx, rbuff );
 	  (void) memcpy( darkCurrent, rbuff+offs, nbytes );
	  if ( analogOffsError != NULL ) {
	       NADC_RD_H5_EArray_float( gid, "analogOffsetError", 
					SCIENCE_PIXELS, 1, &metaIndx, rbuff );
	       (void) memcpy( analogOffsError, rbuff+offs, nbytes );
	  }
	  if ( darkCurrentError != NULL ) {
	       NADC_RD_H5_EArray_float( gid, "darkCurrentError", 
					SCIENCE_PIXELS, 1, &metaIndx, rbuff );
	       (void) memcpy( darkCurrentError, rbuff+offs, nbytes );
	  }
	  if ( meanNoise != NULL ) {
	       NADC_RD_H5_EArray_float( gid, "meanNoise", SCIENCE_PIXELS, 
					1, &metaIndx, rbuff );
	       (void) memcpy( meanNoise, rbuff+offs, nbytes );
	  }
	  if ( chiSquareFit != NULL ) {
	       NADC_RD_H5_EArray_float( gid, "chiSquareFit", SCIENCE_PIXELS, 
					1, &metaIndx, rbuff );
	       (void) memcpy( chiSquareFit, rbuff+offs, nbytes );
	  }
	  if ( probabilityFit != NULL ) {
	       NADC_RD_H5_EArray_float( gid, "probabilityFit", SCIENCE_PIXELS, 
					1, &metaIndx, rbuff );
	       (void) memcpy( probabilityFit, rbuff+offs, nbytes );
	  }
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

     unsigned short orbit;
     unsigned short channel = 0;

     bool fnd_24, fnd_30, fnd_31;

     float ao_24[SCIENCE_PIXELS], lc_24[SCIENCE_PIXELS], 
	  ao_err_24[SCIENCE_PIXELS], lc_err_24[SCIENCE_PIXELS], 
	  chisq_24[SCIENCE_PIXELS];
     float ao_30[SCIENCE_PIXELS], lc_30[SCIENCE_PIXELS], 
	  ao_err_30[SCIENCE_PIXELS], lc_err_30[SCIENCE_PIXELS], 
	  chisq_30[SCIENCE_PIXELS];
     float ao_31[SCIENCE_PIXELS], lc_31[SCIENCE_PIXELS], 
	  ao_err_31[SCIENCE_PIXELS], lc_err_31[SCIENCE_PIXELS], 
	  noise_31[SCIENCE_PIXELS], chisq_31[SCIENCE_PIXELS], 
	  prob_31[SCIENCE_PIXELS];
/*
 * initialization of command-line parameters
 */
     if ( argc == 1 || (argc > 1 && strncmp( argv[1], "-h", 2 ) == 0) ) {
	  (void) fprintf( stderr, "Usage: %s orbit [channel]\n", argv[0] );
          exit( EXIT_FAILURE );
     }
     orbit = (unsigned short) atoi( argv[1] );
     if ( argc == 3 ) channel = (unsigned short) atoi( argv[2] );
/*
 * 
 */
     fnd_24 = SDMF_get_FittedDark_24( channel, orbit, ao_24, lc_24, ao_err_24,
				      lc_err_24, chisq_24 );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR(NADC_ERR_FATAL, "SDMF_get_FittedDark_24");
     if ( ! fnd_24 ) (void) fprintf( stderr, "# no solution for SDMF v2.4\n" );
     fnd_30 = SDMF_get_FittedDark_30( channel, orbit, ao_30, lc_30, ao_err_30,
				      lc_err_30, chisq_30 );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR(NADC_ERR_FATAL, "SDMF_get_FittedDark_30");
     if ( ! fnd_30 ) (void) fprintf( stderr, "# no solution for SDMF v3.0\n" );
     fnd_31 = SDMF_get_FittedDark( channel, orbit, ao_31, lc_31, ao_err_31,
				   lc_err_31, noise_31, chisq_31, prob_31 ); 
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR(NADC_ERR_FATAL, "SDMF_get_FittedDark_31");
     if ( ! fnd_31 ) (void) fprintf( stderr, "# no solution for SDMF v3.1\n" );
/*
 * 
 */
     if ( ! (fnd_24 || fnd_30 || fnd_31) ) goto done;
     do {
	  (void) printf( "%5hu", np );
	  if ( fnd_24 ) {
	       (void) printf( " %12.6g %12.6g %12.6g %12.6g %12.6g", 
			      ao_24[np], lc_24[np], ao_err_24[np], 
			      lc_err_24[np], chisq_24[np] );
	  }
	  if ( fnd_30 ) {
	       (void) printf( " %12.6g %12.6g %12.6g %12.6g %12.6g", 
			      ao_30[np], lc_30[np], ao_err_30[np], 
			      lc_err_30[np], chisq_30[np] );
	  }
	  if ( fnd_31 ) {
	       (void) printf( " %12.6g %12.6g %12.6g %12.6g %12.6g %12.6g %12.6g", 
			      ao_31[np], lc_31[np], ao_err_31[np], 
			      lc_err_31[np], chisq_31[np], noise_31[np],
			      prob_31[np] );
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
