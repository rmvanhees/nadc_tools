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

.IDENTifer   GOME_lv1
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1
.LANGUAGE    ANSI C
.PURPOSE     read DLR GOME level 1 files, extract data
             write in a flexible binary format (HDF5)
.INPUT/OUTPUT
  call as
            gome_lv1 [options] <input-file>

.RETURNS     non-negative on success, negative on failure
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   10-Nov-2005 add Start and Stop time to SPH, RvH
              2.2   11-Oct-2005 using modified function call of 
                                GOME_LV1_RD_FCD -- debugged call to
				PROCESS_PCD_BDR and PROCESS_SMCD_BDR
              2.1   25-Mar-2003 write software version to HDF5 file, RvH
              2.0   13-Nov-2001 moved to the new Error interface, RvH
              1.5   05-Sep-2001 compiles without HDF5 library, RvH
              1.4   20-Jul-2001 more debugging, RvH
              1.3   17-Jul-2001 modified rec_record (fixed length), RvH
              1.3   17-Apr-2001 use Create_HDF5_NADC_FILE, RvH
              1.2   05-Oct-1999 update of GOME_LV1_RD_SPH, RvH
              1.1   30-Jul-1999 changed to NL_DC convections, RvH
              1.0   28-Jan-1999 created by R. M. van Hees
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
#ifdef _WITH_SQL
#include <libpq-fe.h>
#endif

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++ Global Variables +++++*/
/* 
 * Most routines to read SCIAMACHY data can allocate memory internally
 * However IDL requires the use of their own memory allocation routines
 */
bool Use_Extern_Alloc = FALSE;

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include <_connect_nadc_db.inc>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int main( int argc, char *argv[] )
     /*@globals  errno, stderr, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, stderr, nadc_stat, nadc_err_stack@*/
{
     register short chan, subchan, nr, nr_pcd, nr_mcd, nr_scd;

     short  nband;
#ifdef _WITH_SQL
     int    meta_id = -1;
#endif
     short  num_fcd = 0;
     short  num_pcd = 0;
     short  num_scd = 0;
     short  num_mcd = 0;

     short  *indx_pcd = NULL;
     short  *indx_mcd = NULL;
     short  *indx_scd = NULL;
     FILE   *infl     = NULL;

     struct param_record param;

     struct pir_gome   pir;
     struct fsr1_gome  fsr;
     struct sph1_gome  sph;
     struct fcd_gome   fcd;
     struct pcd_gome  *pcd;
     struct smcd_gome *mcd, *scd;
     struct rec_gome  *rec;
#ifdef _WITH_SQL
     PGconn *conn = NULL;
#endif
/*
 * initialization of command-line parameters
 */
     GOME_SET_PARAM( argc, argv, GOME_LEVEL_1, &param );
     if ( IS_ERR_STAT_FATAL ) NADC_GOTO_ERROR( NADC_ERR_PARAM, "" );
/*
 * check if we have to display version and exit
 */
     if ( param.flag_version == PARAM_SET ) {
	  GOME_SHOW_VERSION( stdout, "gome_lv1" );
	  exit( EXIT_SUCCESS );
     }
/*
 * dump command-line parameters
 */
     if ( param.flag_show == PARAM_SET ) {
          GOME_SHOW_PARAM( GOME_LEVEL_1, param );
          exit( EXIT_SUCCESS );
     }
/*
 * open input-file
 */
     if ( (infl = fopen( param.infile, "r" )) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE, param.infile );
/*
 * -------------------------
 * read/write Product Identifier Content
 */
     GOME_RD_PIR( infl, &pir );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "PIR" );
     if ( strncmp( pir.product, "LVL10", 5 ) != 0 ) {
	  NADC_GOTO_ERROR( NADC_ERR_FATAL,
			   "input is not a valid GOME level 1 file" );
     }
     if ( param.write_hdf5 == PARAM_SET ) {
          param.hdf_file_id = GOME_CRE_H5_FILE( GOME_LEVEL_1, &param );
          if ( IS_ERR_STAT_FATAL )
               NADC_GOTO_ERROR( NADC_ERR_HDF_CRE, "HDF5 base" );

	  CRE_GOME_LV1_H5_STRUCTS( param );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_HDF_CRE, "STRUCTS" );
/*
 * create for data structures for GOME level 1b data
 */
	  GOME_WR_H5_PIR( param, &pir );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "PIR" );
     }
/*
 * -------------------------
 * read/write File Structure Record
 */
     GOME_LV1_RD_FSR( infl, &fsr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "FSR" );
     if ( param.write_ascii == PARAM_SET ) {
	  GOME_LV1_WR_ASCII_FSR( param, &fsr );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "FSR" );
     }
     GOME_LV1_CHK_SIZE( fsr, param.infile );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, param.infile );
/*
 * -------------------------
 * read Specific Product Header
 */
     GOME_LV1_RD_SPH( infl, &fsr, &sph );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "SPH" );
/*
 * -------------------------
 * read Fixed Calibration Data Record
 */
     num_fcd = GOME_LV1_RD_FCD( infl, &fsr, &fcd );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "FCD" );
/*
 * -------------------------
 * read Pixel Specific Calibration Data Records
 */
     num_pcd = GOME_LV1_RD_PCD( infl, &fsr, &sph, &pcd );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "PCD" );
/*
 * write SPH and FCD
 */
     sph.start_time.days = pcd->glr.utc_date;
     if ( pcd->glr.utc_time >= 1500u )
	  sph.start_time.msec = pcd->glr.utc_time - 1500u;
     else {
	  sph.start_time.days--;
	  sph.start_time.msec = 
	       (3600u * 24u * 1000u) - pcd->glr.utc_time - 1500u;
     }
     sph.stop_time.days = pcd[num_pcd-1].glr.utc_date;
     sph.stop_time.msec = pcd[num_pcd-1].glr.utc_time;
     if ( param.write_ascii == PARAM_SET ) {
	  GOME_LV1_WR_ASCII_SPH( param, &sph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SPH" );
	  GOME_LV1_WR_ASCII_FCD( param, &fcd );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "FCD" );
     }
     if ( param.write_hdf5 == PARAM_SET ) {
	  GOME_LV1_WR_H5_SPH( param, &sph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "SPH" );
	  GOME_LV1_WR_H5_FCD( param, &fcd );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "FCD" );
     }
/*
 * connect to PostgreSQL database
 */
#ifdef _WITH_SQL
     if ( param.write_sql == PARAM_SET ) {
	  CONNECT_NADC_DB( &conn, "gome" );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_SQL, "PSQL" );

	  if ( param.flag_sql_remove == PARAM_SET 
	       || param.flag_sql_replace == PARAM_SET )
	       GOME_LV1_DEL_ENTRY( conn, param.infile, sph.soft_version );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_SQL, "PSQL(remove)" );

	  if ( param.flag_sql_remove == PARAM_SET ) goto done;

	  meta_id = GOME_LV1_WR_SQL_META( conn, param.flag_verbose, 
					  param.infile, &sph, &fsr );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_SQL, "SQL_META" );
     }
#endif
/*
 * write PCD data
 */
     nr_pcd = 0;
     if ( num_pcd > 0 ) {
/*
 * initialize indices to selected PCD's
 */
	  indx_pcd = (short *) malloc( num_pcd * sizeof( short ));
	  if ( indx_pcd == NULL )
 	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "indx_pcd" );
	  *indx_pcd = 0;
	  if ( param.flag_pselect == PARAM_SET ) {
	       short nr_indices, *indices;

	       indices = (short *) malloc( num_pcd * sizeof(short));
	       if ( indices == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "indices" );
	       nr_indices = NADC_USRINDX( param.pselect, num_pcd, indices );

	       nr = 0;
	       do {
		    if ( SELECT_PCD( param, pcd + indices[nr] ) != 0 ) 
			 indx_pcd[nr_pcd++] = indices[nr];
	       } while ( ++nr < nr_indices );
	       free( indices );
	  } else {
	       nr = 0;
	       do {
		    if ( SELECT_PCD( param, pcd + nr ) != 0 ) 
			 indx_pcd[nr_pcd++] = nr;
	       } while ( ++nr < num_pcd );
	  }
/*
 * calibrate/write Pixel PMD data
 */
	  if ( param.write_pmd == PARAM_SET )
	       CALIB_PCD_PMD( param.write_pmd_geo, param.calib_pmd, 
			      &fcd, nr_pcd, indx_pcd, pcd );
/*
 * write PCD records
 */
	  if ( param.write_ascii == PARAM_SET )
	       GOME_LV1_WR_ASCII_PCD( param, nr_pcd, indx_pcd, pcd );
	  if ( param.write_hdf5 == PARAM_SET )
	       GOME_LV1_WR_H5_PCD( param, nr_pcd, indx_pcd, pcd );
#ifdef _WITH_SQL
	  if ( param.write_sql == PARAM_SET ) {
	       GOME_LV1_WR_SQL_TILE( conn, param.flag_verbose, meta_id, 
				     sph.soft_version, nr_pcd, indx_pcd, pcd );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_SQL, "SQL_PCD" );
	  }
#endif
     }
/*
 * -------------------------
 * read Sun Specific Calibration Data Records
 */
     num_scd = GOME_LV1_RD_SMCD( FLAG_SUN, infl, &fsr, &sph, &scd );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "SCD" );
/*
 * write SCD data
 */
     nr_scd = 0;
     if ( num_scd > 0 ) {
/*
 * initialize indices to selected SCD's
 */
	  indx_scd = (short *) malloc( num_scd * sizeof( short ));
	  if ( indx_scd == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "indx_scd" );
	  nr = 0;
	  *indx_scd = 0;
	  do { 
  	       if ( SELECT_SMCD(param, scd+nr) != 0 ) indx_scd[nr_scd++] = nr;
	  } while ( ++nr < num_scd );
/*
 * calibrate/write Moon PMD data
 */
	  if ( param.write_pmd == PARAM_SET )
	       CALIB_SMCD_PMD( param.calib_pmd, &fcd, nr_scd, indx_scd, scd );
/*
 * write SCD data to file
 */
	  if ( param.write_ascii == PARAM_SET )
	       GOME_LV1_WR_ASCII_SMCD(FLAG_SUN, param, nr_scd, indx_scd, scd);
	  if ( param.write_hdf5 == PARAM_SET )
	       GOME_LV1_WR_H5_SMCD( FLAG_SUN, param, nr_scd, indx_scd, scd );
     }
/*
 * -------------------------
 * read Moon Specific Calibration Data Records
 */
     num_mcd = GOME_LV1_RD_SMCD( FLAG_MOON, infl, &fsr, &sph, &mcd );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "MCD" );
/*
 * write MCD data
 */
     nr_mcd = 0;
     if ( num_mcd > 0 ) {
/*
 * initialize indices to selected MCD's
 */
	  indx_mcd = (short *) malloc( num_mcd * sizeof( short ));
	  if ( indx_mcd == NULL )
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "indx_mcd" );
	  nr = 0;
	  *indx_mcd = 0;
	  do {
	       if ( SELECT_SMCD(param, mcd+nr) != 0 ) indx_mcd[nr_mcd++] = nr;
	  } while ( ++nr < num_mcd );
/*
 * calibrate/write Moon PMD data
 */
	  if ( param.write_pmd == PARAM_SET )
	       CALIB_SMCD_PMD( param.calib_pmd, &fcd, nr_mcd, indx_mcd, mcd );
/*
 * write MCD data to file
 */
	  if ( param.write_ascii == PARAM_SET )
	       GOME_LV1_WR_ASCII_SMCD( FLAG_MOON, param, 
				       nr_mcd, indx_mcd, mcd );
	  if ( param.write_hdf5 == PARAM_SET )
	       GOME_LV1_WR_H5_SMCD( FLAG_MOON, param, nr_mcd, indx_mcd, mcd );
     }
/*
 * -------------------------
 * read, calibrate and write Science Channel Spectral Band Data
 */
     for ( chan = 0; chan < SCIENCE_CHANNELS; chan++ ) {
	  for ( subchan = 0; subchan < NUM_BAND_IN_CHANNEL; subchan++ ) {
	       if ( (nband = BandChannel( chan, subchan )) != -1
		    && SELECT_BAND( nband, param, &fcd ) ) {
/* 
 * read spectral Band Data Records
 */
		 (void) GOME_LV1_RD_BDR( infl, nband, &fsr, &fcd, &rec );
/*
 * process/write Earth Band Data Records
 */
		    if (  param.write_nadir == PARAM_SET )
			 PROCESS_PCD_BDR( nband, param, &fsr, &fcd, 
					  nr_pcd, indx_pcd, pcd, rec );
/*
 * process/write Moon Band Data Records
 */
		    if (  param.write_moon == PARAM_SET )
			 PROCESS_SMCD_BDR( FLAG_MOON, nband, param, &fcd,  
					   nr_mcd, indx_mcd, mcd, rec );
/*
 * process/write Sun Band Data Records
 */
		    if (  param.write_sun == PARAM_SET )
			 PROCESS_SMCD_BDR( FLAG_SUN, nband, param, &fcd,  
					   nr_scd, indx_scd, scd, rec );
/*
 * free allocated memory
 */
		    if ( rec != NULL ) free( rec );
	       }
	  }
     }
/* -------------------------
 * read, calibrate and write additional Spectral Band Data
 */
     if ( param.write_blind == PARAM_SET ) {
/* 
 * read spectral Band Data Records
 */
       (void) GOME_LV1_RD_BDR( infl, BLIND_1a, &fsr, &fcd, &rec );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "BDR[blind]" );
/*
 * process/write Earth Band Data Records
 */
	  PROCESS_PCD_BDR( BLIND_1a, param, &fsr, &fcd, 
			   nr_pcd, indx_pcd, pcd, rec );
	  free( rec );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "BDR[blind]" );
     }

     if ( param.write_stray == PARAM_SET ) {
	  for ( nband = STRAY_1a; nband <= STRAY_2a; nband++ ) {
/* 
 * read spectral Band Data Records
 */
	    (void) GOME_LV1_RD_BDR( infl, nband, &fsr, &fcd, &rec );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "BDR[blind]" );
/*
 * process/write Earth Band Data Records
 */
	       PROCESS_PCD_BDR( nband, param, &fsr, &fcd, 
				nr_pcd, indx_pcd, pcd, rec );
	       free( rec );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "BDR[blind]" );
	  }
     }
/*
 * when an error has occurred we jump to here:
 */
 done:
/*
 * close connection to PostgreSQL database
 */
#ifdef _WITH_SQL
     if ( param.write_sql == PARAM_SET && conn != NULL ) PQfinish( conn );
#endif
/*
 * close input-file
 */
     if ( infl != NULL ) (void) fclose( infl );
/*
 * close HDF5 output file
 */
     if ( param.write_hdf5 == PARAM_SET ) {
	  if ( param.hdf_file_id >= 0 && H5Fclose( param.hdf_file_id ) < 0 )
	       NADC_GOTO_ERROR( NADC_ERR_HDF_FILE, param.hdf5_name );
     }
/*
 * free allocated memory
 */
     if ( num_fcd > 0 ) {
	  if ( fcd.nleak > 0 ) free( fcd.leak );
	  if ( fcd.nspec > 0 ) free( fcd.spec );
	  if ( fcd.nhot > 0 ) free( fcd.hot );
	  if ( fcd.nang > 0 ) free( fcd.calib );
     }
     if ( num_pcd > 0 ) {
	  free( pcd );
	  if ( indx_pcd != NULL ) free( indx_pcd );
     }
     if ( num_mcd > 0 ) {
	  free( mcd );
	  if ( indx_mcd != NULL ) free( indx_mcd );
     }
     if ( num_scd > 0 ) {
	  free( scd );
	  if ( indx_scd != NULL ) free( indx_scd );
     }
/*
 * display error messages?
 */
     if ( param.flag_silent == PARAM_UNSET ) NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL ) 
	  return NADC_ERR_FATAL;
     else
	  return NADC_ERR_NONE;
}
