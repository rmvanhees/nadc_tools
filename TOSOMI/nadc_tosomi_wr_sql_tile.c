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

.IDENTifer   NADC_TOSOMI_WR_SQL_TILE
.AUTHOR      R.M. van Hees
.KEYWORDS    TOSOMI SQL
.LANGUAGE    ANSI C
.PURPOSE     write tile information of a KNMI Tosomi product to database
.INPUT/OUTPUT
  call as   NADC_TOSOMI_WR_SQL_TILE( conn, prodName, num_rec, rec );
     input:  
             PGconn *conn           : PostgreSQL connection handle
	     char *prodName         : filename of the Tosomi product
	     unsigned int num_rec   : number of tiles to write
	     struct tosomi_rec *rec : pointer to Tosomi tile records

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
#include <nadc_tosomi.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
#define SQL_STR_SIZE   384

#define META_TBL_NAME "meta_tosomi"
#define TILE_TBL_NAME "tile_tosomi"

#define SQL_INSERT_TILE \
"INSERT INTO %s (pk_tile,fk_meta,julianDay,integrationTime,\
 cloudFraction,cloudTopPress,amf,amfCloud,ozone,ozoneSlant,tile) \
 VALUES (%lld,%d,%.11f,%hhu,%hhu,%hu,%.5g,%.5g,%.5g,%.6g,ST_GeomFromText(\
 \'POLYGON((%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f))\', 4326))"

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void NADC_TOSOMI_WR_SQL_TILE( PGconn *conn, const char *prodName, 
			      unsigned int num_rec, 
			      const struct tosomi_rec *rec )
{
     const char prognm[] = "NADC_TOSOMI_WR_SQL_TILE";

     register unsigned int nr;
     register unsigned int insertedRows = 0u;
     register unsigned int failedRows = num_rec;

     char   sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];

     char           *pntr;
     int            nrow, numChar, meta_id;

     long long tile_id;

     PGresult *res;
/*
 * check if product is already in database
 */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "SELECT pk_meta FROM %s WHERE name=\'%s\'", 
		      META_TBL_NAME, prodName );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
          NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     if ( (nrow = PQntuples( res )) == 0 ) {
          NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, prodName );
     }
     pntr = PQgetvalue( res, 0, 0 );
     meta_id = (int) strtol( pntr, (char **) NULL, 10 );     
     PQclear( res );
/*
 * Start a transaction block
 */
     res = PQexec( conn, "BEGIN" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
     PQclear( res );
/*
 * insert all tiles in product
 */
     for ( nr = 0; nr < num_rec; nr++ ) {
	  /* obtain next value for serial pk_tile */
	  res = PQexec( conn,
			"SELECT nextval(\'tile_tosomi_pk_tile_seq\')" );
	  if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, 
				PQresultErrorMessage(res) );
	  pntr = PQgetvalue( res, 0, 0 );
	  tile_id = strtoll( pntr, (char **) NULL, 10 );
	  PQclear( res );

	  numChar = snprintf( sql_query, SQL_STR_SIZE, SQL_INSERT_TILE, 
			      TILE_TBL_NAME, tile_id, meta_id, rec[nr].jday,
			      rec[nr].meta.intg_time, 
			      rec[nr].meta.cloudFraction, 
			      rec[nr].meta.cloudTopPress,
			      rec[nr].meta.amfSky, rec[nr].meta.amfCloud,
			      rec[nr].vcd / 10.f, rec[nr].scd / 10.f,
			      rec[nr].lon_corner[0] / 1e2,
			      rec[nr].lat_corner[0] / 1e2,
			      rec[nr].lon_corner[1] / 1e2,
			      rec[nr].lat_corner[1] / 1e2,
			      rec[nr].lon_corner[2] / 1e2,
			      rec[nr].lat_corner[2] / 1e2,
			      rec[nr].lon_corner[3] / 1e2,
			      rec[nr].lat_corner[3] / 1e2,
			      rec[nr].lon_corner[0] / 1e2,
			      rec[nr].lat_corner[0] / 1e2 );

  	  /* (void) fprintf( stderr, "%s [%-d]\n", sql_query, numChar ); */
	  if ( numChar >= SQL_STR_SIZE )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_STRLEN, "sql_query" );
	  res = PQexec( conn, sql_query );
	  if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	       NADC_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
	       PQclear( res );
	       res = PQexec( conn, "ROLLBACK" );
	       if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
		    NADC_ERROR( prognm, NADC_ERR_SQL,
				PQresultErrorMessage(res) );
	       goto done;
	  }
	  PQclear( res );

	  insertedRows++;
	  failedRows--;
     }
/*
 * end the transaction
 */
     res = PQexec( conn, "COMMIT" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          NADC_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
 done:
     PQclear( res );
     (void) snprintf( cbuff, SQL_STR_SIZE, "insertedRows=%-u", insertedRows );
     NADC_ERROR( prognm, NADC_ERR_NONE, cbuff );
     if ( failedRows > 0 ) {
	  (void) snprintf( cbuff, SQL_STR_SIZE, "failedRows=%-u", failedRows );
	  NADC_ERROR( prognm, NADC_ERR_NONE, cbuff );
     }
}
