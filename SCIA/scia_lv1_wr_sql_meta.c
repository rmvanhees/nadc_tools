/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2007 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_WR_SQL_META
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA SQL
.LANGUAGE    ANSI C
.PURPOSE     write meta data of Sciamachy Lv1b product to table "meta__1P"
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_SQL_META( conn, be_verbose, sciafl, mph, sph );
     input:  
             PGconn *conn         :  PostgreSQL connection handle
	     bool be_verbose      :  be verbose
	     char  *sciafl        :  name of Sciamachy file
	     struct mph_envi *mph :  structure for MPH record
	     struct sph1_scia *sph:  structure for SPH record

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     2.1     01-Oct-2012   added verbose flag, RvH
             2.0     08-Jun-2007   port to PostgreSQL by R. M. van Hees
             1.0     29-Jan-2007   initial release by R. M. van Hees
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

#include <libpq-fe.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++ Macros +++++*/
#define SQL_STR_SIZE   1024

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_SQL_META( PGconn *conn, bool be_verbose, const char *sciafl, 
			   const struct mph_envi *mph,
			   const struct sph1_scia *sph )
{
     const char prognm[] = "SCIA_LV1_WR_SQL_META";

     PGresult *res;

     char *pntr;
     char str_quality[6];
     char ctemp[SHORT_STRING_LENGTH];
     char sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];

     int          nrow, numChar, meta_id;
/*
 * check if product is already in database
 */
     (void) snprintf( sql_query, MAX_STRING_LENGTH, "%s\'%s\'",
		      "SELECT * FROM meta__1P WHERE name=", mph->product );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
	  NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     if ( (nrow = PQntuples( res )) != 0 ) {
	  NADC_GOTO_ERROR( prognm, NADC_ERR_SQL_TWICE, mph->product );
     }
     PQclear( res );
/*
 * obtain next value for serial pk_meta
 */
     res = PQexec( conn,
                   "SELECT nextval(\'meta__1p_pk_meta_seq\')" );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
          NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
     pntr = PQgetvalue( res, 0, 0 );
     meta_id = (int) strtol( pntr, (char **) NULL, 10 );
     PQclear( res );
/*
 * no error and not yet stored, then proceed
 */     
/* pk_meta */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "INSERT INTO meta__1P VALUES (%d,", meta_id );
/* name */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), mph->product );
/* fileSize */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%u,",
		      strcpy(cbuff,sql_query), mph->tot_size );
/* receiveDate */
     NADC_RECEIVEDATE( sciafl, ctemp );
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), ctemp );
/* procStage  */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), mph->proc_stage );
/* procCenter */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), mph->proc_center );
/* softVersion */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), mph->soft_version );
/* keyDataVersion */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), sph->key_data );
/* mFactorVersion */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), sph->m_factor );
/* spectralCal */
     rstrip( str_quality, sph->spec_cal );
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), str_quality );
/* saturatedPix */
     rstrip( str_quality, sph->saturate );
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), str_quality );
/* deadPixels */
     rstrip( str_quality, sph->dead_pixel );
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), str_quality );
/* dateTimeStart */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "%sdate_trunc('second', TIMESTAMP \'%s\'),",
		      strcpy(cbuff,sql_query), sph->start_time );
/* muSecStart */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), sph->start_time+21 );
/* dateTimeStop */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "%sdate_trunc('second', TIMESTAMP \'%s\'),",
		      strcpy(cbuff,sql_query), sph->stop_time );
/* muSecStop */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), sph->stop_time+21 );
/* absOrbit */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%d,",
		      strcpy(cbuff,sql_query), mph->abs_orbit );
/* relOrbit */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%d,",
		      strcpy(cbuff,sql_query), mph->rel_orbit );
/* numDataSets */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%u,",
		      strcpy(cbuff,sql_query), mph->num_data_sets );
/* LEAKAGE_FILE */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), "NULL" );
/* PPG_ETALON_FILE */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), "NULL" );
/* SPECTRAL_FILE */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), "NULL" );
/* SUN_REF_FILE */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), "NULL" );
/* KEY_DATA_FILE */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), "NULL" );
/* M_FACTOR_FILE */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), "NULL" );
/* INIT_FILE */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), "NULL" );
/* nadirStates */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%hu,",
		      strcpy(cbuff,sql_query), sph->no_nadir );
/* limbStates */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%hu,",
		      strcpy(cbuff,sql_query), sph->no_limb );
/* occultStates */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%hu,",
		      strcpy(cbuff,sql_query), sph->no_occult );
/* monitorStates */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%hu,",
		      strcpy(cbuff,sql_query), sph->no_monitor );
/* noProcStates */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%hu,",
		      strcpy(cbuff,sql_query), sph->no_noproc );
/* noEntryDMOP */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%d,",
		      strcpy(cbuff,sql_query), 0 );
/* delayedBy */
     numChar = snprintf( sql_query, SQL_STR_SIZE, "%s%s)",
			 strcpy(cbuff,sql_query), "NULL" );
     if ( be_verbose )
	  (void) printf( "%s(): %s [%-d]\n", prognm, sql_query, numChar );
     if ( numChar >= SQL_STR_SIZE )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_STRLEN, "sql_query" );
/*
 * do the actual insert
 */
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
 done:
     PQclear( res );
}
