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

.IDENTifer   GOME_lv2
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 2
.LANGUAGE    ANSI C
.PURPOSE     read DLR GOME level 2 files, extract data
             write in a flexible binary format (HDF5)
.INPUT/OUTPUT
  call as
            gome_lv2 [options] <input-file>

.RETURNS     non-negative on success, negative on failure
.COMMENTS    None
.ENVIRONment None
.VERSION      3.1   02-Oct-2006 R04 changed pir.product value, RvH
              3.0   10-Nov-2005 add Start and Stop time to SPH, RvH
              2.1   25-Mar-2003 write software version to HDF5 file, RvH
              2.0   13-Nov-2001 moved to the new Error interface, RvH
              1.3   05-Sep-2001 compiles without HDF5 library, RvH
              1.2   17-Apr-2001 use Create_HDF5_NADC_FILE, RvH
              1.1   30-Jul-1999 changed to NL_DC convections, RvH
              1.0   18-Mar-1999 created by R. M. van Hees
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
     const char prognm[]   = "gome_lv2";

     register short nr, nr_ddr;

     short  num_ddr = 0;

     FILE   *infl = NULL;

     struct param_record param;

     struct pir_gome   pir;
     struct fsr2_gome  fsr;
     struct sph2_gome  sph;
     struct ddr_gome   *ddr  = NULL;

#ifdef _WITH_SQL
     PGconn *conn = NULL;
#endif
/*
 * initialization of command-line parameters
 */
     GOME_SET_PARAM( argc, argv, GOME_LEVEL_2, &param );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, "" );
/*
 * check if we gave to give version and exit
 */
     if ( param.flag_version == PARAM_SET ) {
	  GOME_SHOW_VERSION( stdout, prognm );
	  exit( EXIT_SUCCESS );
     }
/*
 * dump command-line parameters
 */
     if ( param.flag_show == PARAM_SET ) {
          GOME_SHOW_PARAM( GOME_LEVEL_2, param );
          exit( EXIT_SUCCESS );
     }
/*
 * open input-file
 */
     if ( (infl = fopen( param.infile, "r" )) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, param.infile );
/*
 *-------------------------
 * read/write Product Identifier Content
 */
     GOME_RD_PIR( infl, &pir );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "PIR" );
     if ( strncmp( pir.product, "LVL20", 5 ) != 0 
	  && strncmp( pir.product, "TCDO3", 5 ) != 0 ) {
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL,
			  "input is not a valid GOME level 2 file" );
     }
     if ( param.write_hdf5 == PARAM_SET ) {
          param.hdf_file_id = GOME_CRE_H5_FILE( GOME_LEVEL_2, &param );
          if ( IS_ERR_STAT_FATAL )
               NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_CRE, "HDF5 base" );

	  GOME_WR_H5_PIR( param, &pir );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "PIR" );
     }
/*
 * -------------------------
 * read File Structure Record
 */
     GOME_LV2_RD_FSR( infl, &fsr );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "FSR" );
     if ( param.write_ascii == PARAM_SET ) {
	  GOME_LV2_WR_ASCII_FSR( param, &fsr );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "FSR" );
     }
     GOME_LV2_CHK_SIZE( fsr, param.infile );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, param.infile );
/*
 * -------------------------
 * read Specific Product Header
 */
     GOME_LV2_RD_SPH( infl, &fsr, &sph );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SPH" );
/*
 * -------------------------
 * read DOAS Data Records
 */
     num_ddr = GOME_LV2_RD_DDR( infl, &fsr, &sph, &ddr ); 
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "DDR" );
     if ( param.flag_pselect == PARAM_SET ) {
       short nr_indices, *ddr_indices;

	  ddr_indices = (short *) malloc( num_ddr * sizeof(short));
	  if ( ddr_indices == NULL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "ddr_indices" );
	  nr_indices = NADC_USRINDX( param.pselect, num_ddr, ddr_indices );
	  for ( nr = 0; nr < nr_indices; nr++ ) {
	       (void) memcpy( &ddr[nr], &ddr[ddr_indices[nr]], 
			      sizeof(struct ddr_gome) );
	  }
	  num_ddr = nr_indices;
	  free( ddr_indices );
     }

     nr_ddr = 0;
     nr = 0;
     do {
	  if ( SELECT_DDR( param, &ddr[nr].glr ) == 1 ) {
	       if ( nr != nr_ddr )
		    (void) memcpy( ddr + nr_ddr, ddr + nr, 
				   sizeof(struct ddr_gome) );
	       nr_ddr++;
	  }
     } while ( ++nr < num_ddr );
/*
 * write SPH
 */
     if ( ddr != NULL ) {
	  sph.start_time.days = ddr->glr.utc_date;
	  if ( ddr->glr.utc_time >= 1500u )
	       sph.start_time.msec = ddr->glr.utc_time - 1500u;
	  else {
	       sph.start_time.days--;
	       sph.start_time.msec = 
		    (3600u * 24u * 1000u) - ddr->glr.utc_time - 1500u;
	  }
	  sph.stop_time.days = ddr[num_ddr-1].glr.utc_date;
	  sph.stop_time.msec = ddr[num_ddr-1].glr.utc_time;
     } else {
	  sph.start_time.days = 0;
	  sph.start_time.msec = 0;
	  sph.stop_time.days = 0;
	  sph.stop_time.msec = 0;
     }
     if ( param.write_ascii == PARAM_SET ) {
	  GOME_LV2_WR_ASCII_SPH( param, &sph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "SPH" );
     }
     if ( param.write_hdf5 == PARAM_SET ) {
	  GOME_LV2_WR_H5_SPH( param, &sph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "SPH" );
     }
/*
 * write DDR records
 */
     if ( param.write_ascii == PARAM_SET ) {
	  GOME_LV2_WR_ASCII_DDR( param, sph, nr_ddr, ddr );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "DDR" );
     }
     if ( param.write_hdf5 == PARAM_SET ) {
	  GOME_LV2_WR_H5_DDR( param, nr_ddr, ddr );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "DDR" );
	  GOME_LV2_WR_H5_IRR( param, nr_ddr, ddr );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "IRR" );
     }
/*
 * connect to PostgreSQL database and write meta data
 */
#ifdef _WITH_SQL
     if ( param.write_sql == PARAM_SET ) {
	  int    meta_id = -1;

	  CONNECT_NADC_DB( &conn, "gome" );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, "PSQL" );

	  if ( param.flag_sql_remove == PARAM_SET 
	       || param.flag_sql_replace == PARAM_SET )
	       GOME_LV2_DEL_ENTRY( conn, param.infile, sph.soft_version );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, "PSQL(remove)" );

	  if ( param.flag_sql_remove == PARAM_SET ) goto done;

	  meta_id = GOME_LV2_WR_SQL_META( conn, param.flag_verbose, 
					  param.infile, &sph, &fsr );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, "SQL_META" );

	  GOME_LV2_WR_SQL_TILE( conn, param.flag_verbose, meta_id, 
				sph.soft_version, nr_ddr, ddr );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, "SQL_DDR" );
     }
#endif
/*
 * when an error has occurred we jump to here:
 */
 done:
/*
 * close connection to PostgresSQL database
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
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, param.hdf5_name );
     }
/*
 * free allocated memory
 */
     if ( ddr != NULL ) {
	  for ( nr = 0; nr < num_ddr; nr++ ) {
	       if ( ddr[nr].irr1 != NULL ) free( ddr[nr].irr1 );
	       if ( ddr[nr].irr2 != NULL ) free( ddr[nr].irr2 );
	  }
	  free( ddr );
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
