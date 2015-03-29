/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2011 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_IMAP_WR_SQL_META
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA IMAP HDO SQL
.LANGUAGE    ANSI C
.PURPOSE     write meta data of SRON IMAP product to table "meta_imap_hdo"
.INPUT/OUTPUT
  call as   SCIA_WR_SQL_HDO_META( conn, hdr );
     input:  
             PGconn *conn            : PostgreSQL connection handle
	     struct imap_hdr *hdr    : header of Imap product

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     28-Apr-2011   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _GNU_SOURCE

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

#include <libpq-fe.h>

/*+++++ Local Headers +++++*/
#define __IMAP_HDO_PRODUCT
#include <nadc_imap.h>

/*+++++ Macros +++++*/
#define SQL_STR_SIZE   640

#define META_TBL_NAME "meta_imap_hdo"

#define SQL_INSERT_META \
"INSERT INTO %s (pk_meta,name,fk_name_l1b,filesize,receivedate,creationDate,\
 datetimestart,musecstart,datetimestop,musecstop,softversion,absOrbit,\
 numDataSets) VALUES (							\
 %d,\'%s\',\'%s\',%u,\'%s\',\'%s\',date_trunc('second',TIMESTAMP \'%s\'),\
 \'0\',date_trunc('second',TIMESTAMP \'%s\'),\'0\',\'%s\',%u,%u)"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_WR_SQL_HDO_META( PGconn *conn, const struct imap_hdr *hdr )
{
     PGresult *res;

     char  *pntr, sql_query[SQL_STR_SIZE];

     int   nrow, numChar, meta_id;
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
                   "SELECT nextval(\'meta_imap_hdo_pk_meta_seq\')" );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     pntr = PQgetvalue( res, 0, 0 );
     meta_id = (int) strtol( pntr, (char **) NULL, 10 );
     PQclear( res );
/*
 * no error and not yet stored, then proceed
 */
     numChar = snprintf( sql_query, SQL_STR_SIZE, SQL_INSERT_META, 
			 META_TBL_NAME, meta_id,
			 hdr->product, hdr->l1b_product, hdr->file_size, 
			 hdr->receive_date, hdr->creation_date,
			 hdr->validity_start, hdr->validity_stop,
			 hdr->software_version, hdr->orbit[0], 
			 hdr->numRec );
/*      (void) fprintf( stderr, "%s [%-d]\n", sql_query, numChar ); */
     if ( numChar >= SQL_STR_SIZE )
	  NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
/*
 * do the actual insert
 */
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
 done:
     PQclear( res );
}
