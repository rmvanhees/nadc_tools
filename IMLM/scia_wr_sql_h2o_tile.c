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

.IDENTifer   SCIA_WR_SQL_H2O_TILE
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA IMLM H2O SQL
.LANGUAGE    ANSI C
.PURPOSE     write tile information of a Sciamachy IMLM product to database
.INPUT/OUTPUT
  call as   SCIA_WR_SQL_H2O_TILE( conn, sciafl, num_rec, rec );
     input:  
             PGconn *conn           : PostgreSQL connection handle
	     char  *sciafl          : filename of Sciamachy level 2 product
	     unsigned int num_rec   : number of tiles to write
	     struct imlm_rec *rec   : pointer to IMLM records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     12-Apr-2008   initial release by R. M. van Hees
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
#include <nadc_imlm.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
#define SQL_STR_SIZE   512

#define META_TBL_NAME "meta_imlm_h2o"
#define TILE_TBL_NAME "tile_imlm_h2o"

#define SQL_INSERT_TILE \
"INSERT INTO %s (pk_tile,fk_meta,julianDay,integrationTime,errorFlag,\
 meanElevation,cloudFraction,surfaceAlbedo,VCD_H2O,VCD_H2O_ERROR,VCD_CH4,\
 VCD_CH4_ERROR,tile) \
 VALUES (%lld,%d,%.11f,%d,%hu,%.3g,%.3g,%.3g,%.5g,%.5g,%.5g,%.5g,\
 ST_GeomFromText(\
 \'POLYGON((%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f))\', 4326))"

#define SQL_INSERT_TILE_NAN \
"INSERT INTO %s (pk_tile,fk_meta,julianDay,integrationTime,errorFlag,\
 meanElevation,cloudFraction,surfaceAlbedo,VCD_H2O,VCD_H2O_ERROR,tile)\
 VALUES (%lld,%d,%.11f,%d,%hu,%.3g,%.3g,%.3g,%.5g,%.5g,ST_GeomFromText(\
 \'POLYGON((%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f))\', 4326))"

#define NINT(a) ((a) >= 0.f ? (int)((a)+0.5) : (int)((a)-0.5))

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_WR_SQL_H2O_TILE( PGconn *conn, const char *prodName,
			    unsigned int num_rec, 
			    const struct imlm_rec *rec )
{
     const char prognm[] = "SCIA_WR_SQL_H2O_TILE";

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
			"SELECT nextval(\'tile_imlm_h2o_pk_tile_seq\')" );
	  if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, 
				PQresultErrorMessage(res) );
	  pntr = PQgetvalue( res, 0, 0 );
	  tile_id = strtoll( pntr, (char **) NULL, 10 );
	  PQclear( res );

	  if ( isnormal(rec[nr].CH4) && isnormal(rec[nr].CH4_err)  )
	       numChar = snprintf( sql_query, SQL_STR_SIZE, SQL_INSERT_TILE, 
				   TILE_TBL_NAME, tile_id, meta_id,
				   rec[nr].dsr_time,
				   NINT(16 * rec[nr].meta.intg_time),
				   rec[nr].meta.eflag, rec[nr].mean_elev,
				   rec[nr].cl_fr, rec[nr].albedo, 
				   rec[nr].H2O, rec[nr].H2O_err, 
				   rec[nr].CH4, rec[nr].CH4_err,
				   rec[nr].lon_corner[0],rec[nr].lat_corner[0],
				   rec[nr].lon_corner[1],rec[nr].lat_corner[1],
				   rec[nr].lon_corner[2],rec[nr].lat_corner[2],
				   rec[nr].lon_corner[3],rec[nr].lat_corner[3],
				   rec[nr].lon_corner[0],rec[nr].lat_corner[0]);
	  else 
	       numChar = snprintf( sql_query, SQL_STR_SIZE, 
				   SQL_INSERT_TILE_NAN, TILE_TBL_NAME, 
				   tile_id, meta_id, rec[nr].dsr_time,
				   NINT( 16.f * rec[nr].meta.intg_time), 
				   rec[nr].meta.eflag, rec[nr].mean_elev,
				   rec[nr].cl_fr, rec[nr].albedo, 
				   rec[nr].H2O, rec[nr].H2O_err,
				   rec[nr].lon_corner[0],rec[nr].lat_corner[0],
				   rec[nr].lon_corner[1],rec[nr].lat_corner[1],
				   rec[nr].lon_corner[2],rec[nr].lat_corner[2],
				   rec[nr].lon_corner[3],rec[nr].lat_corner[3],
				   rec[nr].lon_corner[0],rec[nr].lat_corner[0]);

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

	  affectedRows += 1;
     }
/*
 * end the transaction
 */
     res = PQexec( conn, "COMMIT" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          NADC_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
 done:
     PQclear( res );
     (void) snprintf( cbuff, SQL_STR_SIZE, "affectedRows=%-u", affectedRows );
     NADC_ERROR( prognm, NADC_ERR_NONE, cbuff );
}