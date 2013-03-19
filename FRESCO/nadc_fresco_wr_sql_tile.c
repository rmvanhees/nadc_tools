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

.IDENTifer   NADC_FRESCO_WR_SQL_TILE
.AUTHOR      R.M. van Hees
.KEYWORDS    FRESCO SQL
.LANGUAGE    ANSI C
.PURPOSE     write tile information of a KNMI Fresco product to database
.INPUT/OUTPUT
  call as   NADC_FRESCO_WR_SQL_TILE( conn, prodName, num_rec, rec );
     input:  
             PGconn *conn           : PostgreSQL connection handle
	     char *prodName         : filename of the Fresco product
	     unsigned int num_rec   : number of tiles to write
	     struct fresco_rec *rec : pointer to Fresco tile records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     3.2     12-Aug-2009   bugfix string allocation too small, RvH
             3.1     11-Apr-2008   add GOME implementation, RvH
             3.0     20-Mar-2008   rename/rewrite, RvH
             2.1     06-Aug-2007   fixed segementation fault on empty products
             2.0     27-Jun-2007   port to PostgreSQL by RvH
             1.0     15-Feb-2007   initial release by R. M. van Hees
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
#define _SCIA_LEVEL_2
#include <nadc_fresco.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
#define SQL_STR_SIZE   432

#define META_TBL_NAME "meta_fresco"
#define TILE_TBL_NAME "tile_fresco"

#define SQL_INSERT_GOME_TILE \
"INSERT INTO %s (julianDay,fk_tileinfo,integrationTime,errorFlag,\
 cloudFraction,cloudTopHeight,cloudTopPressure,cloudAlbedo,surfaceHeight,\
 surfacePressure,surfaceAlbedo) \
 VALUES (%.11f,%lld,%d,%hhu,%.7g,%.7g,%.7g,%.7g,%.7g,%.7g,%.7g)"

#define SQL_INSERT_SCIA_TILE \
"INSERT INTO %s (pk_tile,fk_meta,julianDay,integrationTime,errorFlag,\
 cloudFraction,cloudTopHeight,cloudTopPressure,cloudAlbedo,surfaceHeight,\
 surfacePressure,surfaceAlbedo,tile) \
 VALUES (%lld,%d,%.11f,%d,%hhu,%.7g,%.7g,%.7g,%.7g,%.7g,%.7g,%.7g,\
 ST_GeomFromText(\
 \'POLYGON((%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f))\', 4326))"

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void NADC_FRESCO_WR_SQL_GOME_TILE( PGconn *conn, int meta_id, 
				   unsigned int num_rec, 
				   const struct fresco_rec *rec )
{
     const char prognm[] = "NADC_FRESCO_WR_SQL_GOME_TILE";

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
          NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
     PQclear( res );
/*
 * loop over all Fresco records
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
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, 
				PQresultErrorMessage(res) );
	  }
	  if ( (nrow = PQntuples( res )) < 1 ) continue;
	  pntr = PQgetvalue( res, 0, 0 );
	  tileinfo_id = strtoll( pntr, (char **) NULL, 10 );
	  PQclear( res );
/*
 * inset Fresco tile
 */
	  numChar = snprintf( sql_query, SQL_STR_SIZE, SQL_INSERT_GOME_TILE, 
			      TILE_TBL_NAME, rec[nr].jday, tileinfo_id, 
			      rec[nr].meta.intg_time, 
			      rec[nr].meta.errorFlag, rec[nr].cloudFraction, 
			      rec[nr].cloudTopHeight, rec[nr].cloudTopPress, 
			      rec[nr].cloudAlbedo, rec[nr].surfaceHeight, 
			      rec[nr].groundPress, rec[nr].surfaceAlbedo );

/*  	  (void) fprintf( stderr, "%s [%-d]\n", sql_query, numChar ); */
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
/*
 * insert one-to-many relations
 */
	  numChar = snprintf( sql_query, SQL_STR_SIZE,
			      "INSERT INTO tile_meta_fresco"\
			      " (fk_meta,julianDay) VALUES (%d,%.11f)", 
			      meta_id, rec[nr].jday );
/*  	  (void) fprintf( stderr, "%s [%-d]\n", sql_query, numChar ); */
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

void NADC_FRESCO_WR_SQL_SCIA_TILE( PGconn *conn, const char *prodName,
				   unsigned int num_rec, 
				   const struct fresco_rec *rec )
{
     const char prognm[] = "NADC_FRESCO_WR_SQL_SCIA_TILE";

     register unsigned int nr;
     register unsigned int insertedRows = 0u;
     register unsigned int failedRows = num_rec;

     char   sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];

     char       *pntr;
     int        nrow, numChar, meta_id;

     long long  tile_id;

     PGresult   *res;
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
 * insert all tiles in products
 */
     for ( nr = 0; nr < num_rec; nr++ ) {
	  /* obtain next value for serial pk_tile */
	  res = PQexec( conn,
			"SELECT nextval(\'tile_fresco_pk_tile_seq\')" );
	  if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, 
				PQresultErrorMessage(res) );
	  pntr = PQgetvalue( res, 0, 0 );
	  tile_id = strtoll( pntr, (char **) NULL, 10 );
	  PQclear( res );

	  numChar = snprintf( sql_query, SQL_STR_SIZE, SQL_INSERT_SCIA_TILE, 
			      TILE_TBL_NAME, tile_id, meta_id, rec[nr].jday,
			      rec[nr].meta.intg_time, 
			      rec[nr].meta.errorFlag, rec[nr].cloudFraction, 
			      rec[nr].cloudTopHeight, rec[nr].cloudTopPress,
			      rec[nr].cloudAlbedo, rec[nr].surfaceHeight, 
			      rec[nr].groundPress, rec[nr].surfaceAlbedo,
			      rec[nr].lon_corner[0],rec[nr].lat_corner[0],
			      rec[nr].lon_corner[1],rec[nr].lat_corner[1],
			      rec[nr].lon_corner[2],rec[nr].lat_corner[2],
			      rec[nr].lon_corner[3],rec[nr].lat_corner[3],
			      rec[nr].lon_corner[0],rec[nr].lat_corner[0] );

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
