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

.IDENTifer   GOME_LV2_WR_SQL_META
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME SQL
.LANGUAGE    ANSI C
.PURPOSE     write meta data of Gome Lv2 product to table "meta__2P"
.INPUT/OUTPUT
  call as   meta_id = GOME_LV2_WR_SQL_META( conn, be_verbose, flname, sph,fsr );
     input:  
             PGconn *conn          : PostgreSQL connection handle
	     bool be_verbose       : be verbose
	     char *flname          : name of GOME file
	     struct sph2_gome *sph : structure for SPH record
	     struct fsr2_gome *fsr : structure for FSR record

.RETURNS     value of serial variable ``pk_meta''
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     2.3     01-Oct-2012   added verbose flag, RvH
             2.2     22-Nov-2007   fixed bug in dateTimeStart/Stop, RvH
             2.1     22-Nov-2007   added softVersion to primary key, RvH
             2.0     18-Jun-2007   port to PostgreSQL by R. M. van Hees
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
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++ Macros +++++*/
#define SQL_STR_SIZE   340

#define SELECT_FROM_META \
"SELECT pk_meta FROM meta__2P WHERE name = \'%s\' and softVersion = \'%s\'"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int GOME_LV2_WR_SQL_META( PGconn *conn, bool be_verbose, const char *flname, 
			  const struct sph2_gome *sph,
			  const struct fsr2_gome *fsr )
{
     PGresult *res;

     char *cpntr, ctemp[SHORT_STRING_LENGTH];
     char sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];

     int     flsize, nrow;
     int     meta_id;
     size_t  numChar;
/*
 * strip path of file-name
 */
     if ( (cpntr = strrchr( flname, '/' )) != NULL ) {
          (void) nadc_strlcpy( ctemp, ++cpntr, SHORT_STRING_LENGTH );
     } else {
          (void) nadc_strlcpy( ctemp, flname, SHORT_STRING_LENGTH );
     }
/*
 * check if product is already in database
 */
     (void) snprintf( sql_query, MAX_STRING_LENGTH, SELECT_FROM_META,
		      ctemp, sph->soft_version );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     if ( (nrow = PQntuples( res )) != 0 ) {
          NADC_GOTO_ERROR( NADC_ERR_SQL_TWICE, ctemp );
     }
     PQclear( res );
/*
 * no error and not yet stored, then obtain value for pk_meta
 */     
     res = PQexec( conn,
		   "SELECT nextval(\'meta__2p_pk_meta_seq\')" );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
	  NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     cpntr = PQgetvalue( res, 0, 0 );
     meta_id = (int) strtol( cpntr, (char **) NULL, 10 );
     PQclear( res );
/* pk_meta */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "INSERT INTO meta__2P VALUES (%d,", meta_id );
/* name */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), ctemp );
/* fileSize */
     flsize = 38 + 12 + fsr->nr_sph * fsr->sz_sph + fsr->nr_ddr * fsr->sz_ddr;
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%d,",
		      strcpy(cbuff,sql_query), flsize );
/* receiveDate */
     NADC_RECEIVEDATE( flname, ctemp );
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), ctemp );
/* procCenter  */
     (void) nadc_strlcpy( ctemp, sph->inref+22, 3 );
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), ctemp );
/* procDate */
     (void) nadc_strlcpy( ctemp, sph->inref+24, 9 );
     (void) snprintf( sql_query, SQL_STR_SIZE, "%sTIMESTAMP \'%s",
		      strcpy(cbuff,sql_query), ctemp );
/* procTime */
     (void) nadc_strlcpy( ctemp, sph->inref+32, 7 );
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s %s\',",
		      strcpy(cbuff,sql_query), ctemp );
/* softVersion */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), sph->soft_version );
/* keyDataVersion */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), sph->param_version );
/* keyDataVersion */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), sph->format_version );
/* dateTimeStart */
     UTC_2_DATETIME( sph->start_time.days, sph->start_time.msec, ctemp );
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "%sdate_trunc('second', TIMESTAMP \'%s\'),",
		      strcpy(cbuff,sql_query), ctemp );
/* muSecStart */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s000,",
		      strcpy(cbuff,sql_query), ctemp+20 );
/* dateTimeStop */
     UTC_2_DATETIME( sph->stop_time.days, sph->stop_time.msec, ctemp );
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "%sdate_trunc('second', TIMESTAMP \'%s\'),",
		      strcpy(cbuff,sql_query), ctemp );
/* muSecStop */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s000,",
		      strcpy(cbuff,sql_query), ctemp+20 );
/* absOrbit */
     (void) nadc_strlcpy( ctemp, sph->inref+5, 6 );
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), ctemp );
/* numDataSets */
     numChar = snprintf( sql_query, SQL_STR_SIZE, "%s%d)",
			 strcpy(cbuff,sql_query), fsr->nr_ddr );
     if ( be_verbose )
	  (void) printf( "%s(): %s [%-zd]\n", __FUNCTION__, sql_query, numChar );
     if ( numChar >= SQL_STR_SIZE )
       NADC_GOTO_ERROR( NADC_ERR_STRLEN, "sql_query" );
/*
 * do actial insert
 */
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
          NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	  PQclear( res );
	  return -1;
     }
     PQclear( res );

     return meta_id;
 done:
     return -1;
}
