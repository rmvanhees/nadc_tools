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

.IDENTifer   SCIA_WR_SQL_HDO_TILE
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA IMAP HDO SQL
.LANGUAGE    ANSI C
.PURPOSE     write tile information of a Sciamachy IMAP product to database
.INPUT/OUTPUT
  call as   SCIA_WR_SQL_HDO_TILE( conn, sciafl, num_rec, rec );
     input:  
             PGconn *conn           : PostgreSQL connection handle
	     char  *sciafl          : filename of Sciamachy level 2 product
	     unsigned int num_rec   : number of tiles to write
	     struct imap_rec *rec   : pointer to IMAP records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     28-Apr-2011   initial release by R. M. van Hees
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
#include <math.h>

#include <libpq-fe.h>

/*+++++ Local Headers +++++*/
#define __IMAP_HDO_PRODUCT
#include <nadc_imap.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
#define SQL_STR_SIZE   512

#define META_TBL_NAME "meta_imap_hdo"
#define TILE_TBL_NAME "tile_imap_hdo"

#define SQL_INSERT_TILE \
"INSERT INTO %s (pk_tile,fk_meta,julianDay,integrationTime,meanElevation,\
 VCD_HDO,VCD_HDO_ERROR,VCD_H2O,VCD_H2O_ERROR,VCD_H2O_MODEL,tile) \
 VALUES (%lld,%d,%.11f,%d,%.3g,%.5g,%.5g,%.5g,%.5g,%.5g,ST_GeomFromText(\
 \'POLYGON((%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f))\', 4326))"

#define NINT(a) ((a) >= 0.f ? (int)((a)+0.5) : (int)((a)-0.5))

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_WR_SQL_HDO_TILE( PGconn *conn, const char *prodName,
			   unsigned int num_rec, 
			   const struct imap_rec *rec )
{
     register unsigned int nr;
     register unsigned int affectedRows = 0u;

     char   sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];

     char   *pntr;
     int    nrow, numChar, meta_id;

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
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     if ( (nrow = PQntuples( res )) == 0 ) {
          NADC_GOTO_ERROR( NADC_ERR_FATAL, prodName );
     }
     pntr = PQgetvalue( res, 0, 0 );
     meta_id = (int) strtol( pntr, (char **) NULL, 10 );     
     PQclear( res );
/*
 * Start a transaction block
 */
     res = PQexec( conn, "BEGIN" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     PQclear( res );
/*
 * insert all tiles in products
 */
     for ( nr = 0; nr < num_rec; nr++ ) {
	  /* obtain next value for serial pk_tile */
	  res = PQexec( conn,
			"SELECT nextval(\'tile_imap_hdo_pk_tile_seq\')" );
	  if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
	       NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	  pntr = PQgetvalue( res, 0, 0 );
	  tile_id = strtoll( pntr, (char **) NULL, 10 );
	  PQclear( res );

	  numChar = snprintf( sql_query, SQL_STR_SIZE, SQL_INSERT_TILE, 
			      TILE_TBL_NAME, tile_id, meta_id, rec[nr].jday,
			      NINT(16 * rec[nr].meta.intg_time),
			      rec[nr].meta.elev, 
			      rec[nr].hdo_vcd, rec[nr].hdo_error,
			      rec[nr].h2o_vcd, rec[nr].h2o_error,
			      rec[nr].h2o_model,
			      rec[nr].lon_corner[0],rec[nr].lat_corner[0],
			      rec[nr].lon_corner[1],rec[nr].lat_corner[1],
			      rec[nr].lon_corner[2],rec[nr].lat_corner[2],
			      rec[nr].lon_corner[3],rec[nr].lat_corner[3],
			      rec[nr].lon_corner[0],rec[nr].lat_corner[0] );

  	  /* (void) fprintf( stderr, "%s [%-d]\n", sql_query, numChar ); */
	  if ( numChar >= SQL_STR_SIZE )
	       NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
	  res = PQexec( conn, sql_query );
	  if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	       NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	       PQclear( res );
	       res = PQexec( conn, "ROLLBACK" );
	       if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
		    NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	       goto done;
	  }
	  PQclear( res );

	  affectedRows += 1;
     }
/*
 * end the transaction
 */
     res = PQexec( conn, "COMMIT" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
 done:
     PQclear( res );
     (void) snprintf( cbuff, SQL_STR_SIZE, "affectedRows=%-u", affectedRows );
     NADC_ERROR( NADC_ERR_NONE, cbuff );
}
