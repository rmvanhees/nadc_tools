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

.IDENTifer   NADC_TOGOMI_WR_SQL_TILE
.AUTHOR      R.M. van Hees
.KEYWORDS    TOGOMI SQL
.LANGUAGE    ANSI C
.PURPOSE     write tile information of a KNMI Togomi product to database
.INPUT/OUTPUT
  call as   NADC_TOGOMI_WR_SQL_TILE( conn, prodName, num_rec, rec );
     input:  
             PGconn *conn           : PostgreSQL connection handle
	     char *prodName         : filename of the Togomi product
	     unsigned int num_rec   : number of tiles to write
	     struct togomi_rec *rec : pointer to Togomi tile records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     1.1     12-Aug-2009   bugfix string allocation too small, RvH
             1.0     07-Oct-2008   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>

#include <libpq-fe.h>

/*+++++ Local Headers +++++*/
#include <nadc_togomi.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
#define SQL_STR_SIZE   256

#define META_TBL_NAME "meta_togomi"
#define TILE_TBL_NAME "tile_togomi"

#define SQL_INSERT_TILE \
"INSERT INTO %s (julianDay,fk_tileinfo,integrationTime,\
 cloudFraction,cloudTopHeight,amf,amfCloud,ozone,ozoneSlant) \
 VALUES (%.11f,%lld,%d,%.5g,%.5g,%.5g,%.5g,%.5g,%.6g)"

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void NADC_TOGOMI_WR_SQL_TILE( PGconn *conn, int meta_id, 
				   unsigned int num_rec, 
				   const struct togomi_rec *rec )
{
     register unsigned int nr;
     register unsigned int insertedRows = 0u;
     register unsigned int failedRows = num_rec;

     char   sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];

     char           *pntr;
     int            nrow, numChar;
     long long      tileinfo_id;

     PGresult *res;
/*
 * Start a transaction block
 */
     res = PQexec( conn, "BEGIN" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     PQclear( res );
/*
 * loop over all Togomi records
 */
     for ( nr = 0; nr < num_rec; nr++ ) {
	  register double dtime = rec[nr].meta.intg_time / (16. * 3600 * 24);
/*
 * select records within integration time from table "tileinfo"
 */
	  (void) snprintf( sql_query, SQL_STR_SIZE, 
			   "select pk_tileinfo from tileinfo where"\
			   " julianDay between %.11f and %.11f", 
			   rec[nr].jday - 1e-8, rec[nr].jday + dtime - 1e-8);
/*   	  (void) fprintf( stderr, "%s\n", sql_query ); */
	  res = PQexec( conn, sql_query );
	  if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
	       NADC_GOTO_ERROR( NADC_ERR_SQL, 
				PQresultErrorMessage(res) );
	  }
	  if ( (nrow = PQntuples( res )) < 1 ) continue;
	  pntr = PQgetvalue( res, 0, 0 );
	  tileinfo_id = strtoll( pntr, (char **) NULL, 10 );
	  PQclear( res );
/*
 * inset Togomi tile
 */
	  numChar = snprintf( sql_query, SQL_STR_SIZE, SQL_INSERT_TILE, 
			      TILE_TBL_NAME, rec[nr].jday, tileinfo_id, 
			      rec[nr].meta.intg_time, 
			      rec[nr].meta.cloudFraction, 
			      rec[nr].meta.cloudHeight, 
			      rec[nr].meta.amfSky, rec[nr].meta.amfCloud, 
			      rec[nr].vcd, rec[nr].scd );

/*   	  (void) fprintf( stderr, "%s [%-d]\n", sql_query, numChar ); */
	  if ( numChar >= SQL_STR_SIZE )
	       NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
	  res = PQexec( conn, sql_query );
	  if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	       NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	       PQclear( res );
	       res = PQexec( conn, "ROLLBACK" );
	       if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
		    NADC_ERROR( NADC_ERR_SQL,
				PQresultErrorMessage(res) );
	       goto done;
	  }
	  PQclear( res );
/*
 * insert one-to-many relations
 */
	  numChar = snprintf( sql_query, SQL_STR_SIZE,
			      "INSERT INTO tile_meta_togomi"\
			      " (fk_meta,julianDay) VALUES (%d,%.11f)", 
			      meta_id, rec[nr].jday );
/*  	  (void) fprintf( stderr, "%s [%-d]\n", sql_query, numChar ); */
	  if ( numChar >= SQL_STR_SIZE )
	       NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
	  res = PQexec( conn, sql_query );
	  if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	       NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	       PQclear( res );
	       res = PQexec( conn, "ROLLBACK" );
	       if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
		    NADC_ERROR( NADC_ERR_SQL,
				PQresultErrorMessage(res) );
	       failedRows++;
	       goto done;
	  }
	  PQclear( res );
	  
	  failedRows--;
	  insertedRows++;
     }
/*
 * end the transaction
 */
     res = PQexec( conn, "COMMIT" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
 done:
     PQclear( res );
     (void) snprintf( cbuff, SQL_STR_SIZE, "insertedRows=%-u", insertedRows );
     NADC_ERROR( NADC_ERR_NONE, cbuff );
     if ( failedRows > 0 ) {
	  (void) snprintf( cbuff, SQL_STR_SIZE, "failedRows=%-u", failedRows );
	  NADC_ERROR( NADC_ERR_NONE, cbuff );
     }
}
