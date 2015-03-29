/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2008 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_TOGOMI_WR_SQL_META
.AUTHOR      R.M. van Hees
.KEYWORDS    TOGOMI SQL
.LANGUAGE    ANSI C
.PURPOSE     write meta data of KNMI TOGOMI product to table "meta_togomi"
.INPUT/OUTPUT
  call as   metaID = NADC_TOGOMI_WR_SQL_META( conn, hdr );
     input:  
             PGconn *conn              : PostgreSQL connection handle
	     struct togomi_hdr *hdr    : header of TOGOMI product

.RETURNS     value of the primary key of the new entry
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     07-Oct-2008   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

#include <libpq-fe.h>

/*+++++ Local Headers +++++*/
#include <nadc_togomi.h>

/*+++++ Macros +++++*/
#define SQL_STR_SIZE   512

#define META_TBL_NAME "meta_togomi"

#define SQL_FIND_ORBIT \
"select absOrbit,softVersion from meta__1P where name=\'%s\'"

#define SQL_INSERT_ALL \
"INSERT INTO %s (pk_meta,name,fk_name_l1b,filesize,receivedate,creationdate,\
 datetimestart,musecstart,datetimestop,musecstop,softversion,fk_version_l1b,\
 absorbit,numdatasets)\
 VALUES (%d,\'%s\',\'%s\',%u,\'%s\',\'%s\',\
 date_trunc('second',TIMESTAMP \'%s\'),%u000,\
 date_trunc('second',TIMESTAMP \'%s\'),%u000,\'%s\',\'%s\',%d,%u)"

#define SQL_INSERT_NOREC \
"INSERT INTO %s (pk_meta,name,fk_name_l1b,filesize,receivedate,creationdate,\
 softVersion,fk_version_l1b,absorbit,numdatasets) \
 VALUES (%d,\'%s\',\'%s\',%u,\'%s\',\'%s\',\'%s\',\'%s\',%d,%u)"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include <_gome_check_sql_lv1b_name.inc>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int NADC_TOGOMI_WR_SQL_META( PGconn *conn, const struct togomi_hdr *hdr )
{
     PGresult *res;

     char prod_version[8], l1b_version[8], l1b_product[ERS2_FILENAME_SIZE];

     char *pntr, sql_query[SQL_STR_SIZE];

     int  meta_id = -1;
     int  absOrbit, nrow, numChar;
/*
 * check if product is already in database
 */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "SELECT * FROM %s WHERE name=\'%s\'", 
		      META_TBL_NAME, hdr->product );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     if ( (nrow = PQntuples( res )) != 0 ) {
          NADC_GOTO_ERROR( NADC_ERR_SQL_TWICE, hdr->product );
     }
     PQclear( res );
/*
 * obtain next value for serial pk_meta
 */
     res = PQexec( conn,
                   "SELECT nextval(\'meta_togomi_pk_meta_seq\')" );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     pntr = PQgetvalue( res, 0, 0 );
     meta_id = (int) strtol( pntr, (char **) NULL, 10 );
     PQclear( res );
/*
 * check presence of GOME L1b in database
 */
     GOME_CHECK_SQL_LV1B_NAME( conn, hdr->l1b_product, hdr->validity_start, 
			       hdr->validity_stop, l1b_product );
/*
 * obtain orbit number from meta__1P table
 */
     absOrbit = 0;
     if ( hdr->numRec > 0 ) {
	  (void) snprintf( sql_query, SQL_STR_SIZE, SQL_FIND_ORBIT,
			   l1b_product );
	  res = PQexec( conn, sql_query );
	  if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
	       NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );

	  if ( (nrow = PQntuples( res )) > 0 ) {
	       pntr = PQgetvalue( res, 0, 0 );
	       absOrbit = (int) strtol( pntr, (char **)NULL, 10 );
	       pntr = PQgetvalue( res, 0, 1 );
	       (void) nadc_strlcpy( l1b_version, pntr, 8 );
	  } else 
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, 
				"input level 1b product not available" );
	  PQclear( res );
     }
/*
 * no error and not yet stored, then proceed
 */
     (void) nadc_strlcpy( prod_version, hdr->software_version, 8 );
     if ( (pntr = strchr( prod_version, '(' )) != NULL ) *pntr = '\0';
     if ( hdr->numRec > 0 ) {
	  numChar = snprintf( sql_query, SQL_STR_SIZE, SQL_INSERT_ALL,
			      META_TBL_NAME, meta_id, 
			      hdr->product, l1b_product, hdr->file_size, 
			      hdr->receive_date, hdr->creation_date, 
			      hdr->validity_start, 0u,
			      hdr->validity_stop, 0u,
			      prod_version, l1b_version,
			      absOrbit, hdr->numRec );
     } else {
	  numChar = snprintf( sql_query, SQL_STR_SIZE, SQL_INSERT_NOREC,
			      META_TBL_NAME, meta_id, 
			      hdr->product, l1b_product, hdr->file_size, 
			      hdr->receive_date, hdr->creation_date, 
			      prod_version, l1b_version, 
			      absOrbit, hdr->numRec );
     }
/*      (void) fprintf( stderr, "%s [%-d]\n", sql_query, numChar ); */
     if ( numChar >= SQL_STR_SIZE )
	  NADC_GOTO_ERROR( NADC_ERR_STRLEN, "sql_query" );
/*
 * do the actual insert
 */
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
 done:
     PQclear( res );
     return meta_id;
}
