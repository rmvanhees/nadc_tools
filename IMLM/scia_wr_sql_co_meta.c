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

.IDENTifer   SCIA_WR_SQL_CO_META
.AUTHOR      R.M. van Hees
.KEYWORDS    SRON IMLM CO SQL
.LANGUAGE    ANSI C
.PURPOSE     write meta data of SRON IMLM product to table "meta_imlm_co"
.INPUT/OUTPUT
  call as   SCIA_WR_SQL_CO_META( conn, hdr );
     input:  
             PGconn *conn            : PostgreSQL connection handle
	     struct imlm_hdr *hdr    : header of Imlm product

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     18-Mar-2008   initial release by R. M. van Hees
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
#include <nadc_imlm.h>

/*+++++ Macros +++++*/
#define SQL_STR_SIZE   640

#define META_TBL_NAME "meta_imlm_co"

#define SQL_INSERT_META \
"INSERT INTO %s (pk_meta,name,fk_name_l1b,filesize,receivedate,creationDate,\
 datetimestart,musecstart,datetimestop,musecstop,softversion,pixelMaskVersion,\
 cloudMaskVersion,absOrbit,windowPixel,windowWave,numDataSets) VALUES (\
 %d,\'%s\',\'%s\',%u,\'%s\',\'%s\',date_trunc('second',TIMESTAMP \'%s\'),\
 \'0\',date_trunc('second',TIMESTAMP \'%s\'),\'0\',\'%s\',\'%s\',\'%s\',\
 %u,\'{%hu,%hu}\',\'{%.2f,%.2f}\',%u)"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include <_scia_check_sql_lv1b_name.inc>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_WR_SQL_CO_META( PGconn *conn, const struct imlm_hdr *hdr )
{
     const char prognm[] = "SCIA_WR_SQL_CO_META";

     PGresult *res;

     char  l1b_product[ENVI_FILENAME_SIZE];

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
          NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     if ( (nrow = PQntuples( res )) != 0 ) {
          NADC_GOTO_ERROR( prognm, NADC_ERR_SQL_TWICE, hdr->product );
     }
     PQclear( res );
/*
 * check presence of SCIA L1b in database
 */
     (void) strlcpy( l1b_product, hdr->l1b_product, ENVI_FILENAME_SIZE );
     SCIA_CHECK_SQL_LV1B_NAME( conn, l1b_product );
/* 
 * obtain next value for serial pk_meta
 */
     res = PQexec( conn,
                   "SELECT nextval(\'meta_imlm_co_pk_meta_seq\')" );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
          NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
     pntr = PQgetvalue( res, 0, 0 );
     meta_id = (int) strtol( pntr, (char **) NULL, 10 );
     PQclear( res );
/*
 * no error and not yet stored, then proceed
 */
     numChar = snprintf( sql_query, SQL_STR_SIZE, SQL_INSERT_META, 
			 META_TBL_NAME, meta_id,
			 hdr->product, l1b_product, hdr->file_size, 
			 hdr->receive_date, hdr->creation_date,
			 hdr->validity_start, hdr->validity_stop,
			 hdr->software_version, hdr->pixelmask_version, 
			 hdr->cloudmask_version, hdr->orbit[0], 
			 hdr->window_pixel[0], hdr->window_pixel[1], 
			 hdr->window_wave[0], hdr->window_wave[1],
			 hdr->numRec );
/*      (void) fprintf( stderr, "%s [%-d]\n", sql_query, numChar ); */
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
