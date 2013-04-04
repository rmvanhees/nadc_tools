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

.IDENTifer   SCIA_OL2_WR_SQL_NFIT
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA SQL
.LANGUAGE    ANSI C
.PURPOSE     write Nadir fits of SCIA Lv2 offline product to database
.INPUT/OUTPUT
  call as   SCIA_OL2_WR_SQL_NFIT( conn, be_verbose, flname, nfit_name, 
                                  num_geo, geo, num_nfit, nfit );
     input:  
             PGconn *conn           : PostgreSQL connection handle
	     bool be_verbose        : be verbose
	     char   *flname         : name of SCIA lv2 product
	     char   *nfit_name      : name of the Nadir fit specie
	     unsigned int num_ngeo  : number of nadir gelocation records
	     struct ngeo_scia *ngeo : pointer to structure with geolocation info
	     unsigned int num_nfit  : number of nadir fitting window records
	     struct nfit_scia *nfit : pointer to structure with NFIT parameters

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     2.1     01-Oct-2012   added verbose flag, RvH
             2.0     21-Jun-2007   port to PostgreSQL by R. M. van Hees
             1.1     14-Mar-2007   it is save to ignore double entries, RvH
                                   (the caller can use "-replace")
             1.0     30-Jan-2007   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _GNU_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <libpq-fe.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
#define SIZE_MJD  sizeof(struct mjd_envi)

#define SQL_STR_SIZE   512

#define META_TBL_NAME "meta__2P"

#define CHECK_TABLE_FOR_JDAY \
"SELECT pk_tile FROM %s WHERE julianDay=%.11f"

#define GEO_POLY_FORMAT \
 "%s%sST_GeomFromText(\
 \'POLYGON((%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f))\', 4326)%s"

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static
void CorrectLongitudes( const struct coord_envi *corners_in,
                      /*@out@*/ struct coord_envi *corners_out )
{
     register unsigned short nc = 1;
     
     (void) memcpy( corners_out, corners_in, 
                    NUM_CORNERS * sizeof(struct coord_envi) );
     do {
          if ( abs(corners_out[0].lon - corners_out[nc].lon) > 270000000 ) {
               if ( corners_out[0].lon > 0 )
                    corners_out[nc].lon += 360000000;
               else
                    corners_out[nc].lon -= 360000000;
          }
     } while ( ++nc < NUM_CORNERS );
}

static inline
int GET_INSERT_QUERY( const char *tbl_name, char *sql_query, 
		      int meta_id, long long tile_id, double jday,
		      const struct ngeo_scia *ngeo,
		      const struct nfit_scia *nfit )
{
     char cbuff[SQL_STR_SIZE];

     struct coord_envi  corners[NUM_CORNERS];

/* initialise string */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "INSERT INTO %s VALUES (", tbl_name );
/* pk_tile */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%lld,",
		      strcpy(cbuff,sql_query), tile_id );
/* fk_meta */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%d,",
		      strcpy(cbuff,sql_query), meta_id );
/* julianDay */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%.11f,",
		      strcpy(cbuff,sql_query), jday );
/* integrationTime */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%-hu,",
		      strcpy(cbuff,sql_query), nfit->intg_time );
/* quality */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%-hhu,",
		      strcpy(cbuff,sql_query), nfit->quality );
/* verticalColumn */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%.0f,",
		      strcpy(cbuff,sql_query), nfit->vcd[0] );
/* slantColumn */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%.0f,",
		      strcpy(cbuff,sql_query), nfit->esc );
/* amfGround */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%.6f,",
		      strcpy(cbuff,sql_query), nfit->amfgrd );
/* amfCloud */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%.6f,",
		      strcpy(cbuff,sql_query), nfit->amfcld );
/* tile */
     CorrectLongitudes( ngeo->corner, corners );
     return snprintf( sql_query, SQL_STR_SIZE, GEO_POLY_FORMAT,
                      strcpy(cbuff,sql_query), "",
                      corners[0].lon / 1e6, corners[0].lat / 1e6,
                      corners[2].lon / 1e6, corners[2].lat / 1e6,
                      corners[3].lon / 1e6, corners[3].lat / 1e6,
                      corners[1].lon / 1e6, corners[1].lat / 1e6,
                      corners[0].lon / 1e6, corners[0].lat / 1e6, ")" );
}

static inline
int GET_UPDATE_QUERY( const char *tbl_name, char *sql_query, int meta_id,
		      long long tile_id, const struct ngeo_scia *ngeo,
		      const struct nfit_scia *nfit )
{
     char cbuff[SQL_STR_SIZE];

     struct coord_envi  corners[NUM_CORNERS];

/* initialise string */
     (void) snprintf( sql_query, SQL_STR_SIZE, "UPDATE %s SET", tbl_name );
/* fk_meta */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s fk_meta=%d,",
		      strcpy(cbuff,sql_query), meta_id );
/* integrationTime */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s integrationTime=%-hu,",
		      strcpy(cbuff,sql_query), nfit->intg_time );
/* quality */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s quality=%-hhu,",
		      strcpy(cbuff,sql_query), nfit->quality );
/* verticalColumn */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s verticalColumn=%.0f,",
		      strcpy(cbuff,sql_query), nfit->vcd[0] );
/* slantColumn */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s slantColumn=%.0f,",
		      strcpy(cbuff,sql_query), nfit->esc );
/* amfGround */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s amfGround=%.6f,",
		      strcpy(cbuff,sql_query), nfit->amfgrd );
/* amfCloud */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s amfCloud=%.6f,",
		      strcpy(cbuff,sql_query), nfit->amfcld );
/* tile */
     CorrectLongitudes( ngeo->corner, corners );
     (void) snprintf( sql_query, SQL_STR_SIZE, GEO_POLY_FORMAT,
                      strcpy(cbuff,sql_query), ", tile=",
                      corners[0].lon / 1e6, corners[0].lat / 1e6,
                      corners[2].lon / 1e6, corners[2].lat / 1e6,
                      corners[3].lon / 1e6, corners[3].lat / 1e6,
                      corners[1].lon / 1e6, corners[1].lat / 1e6,
                      corners[0].lon / 1e6, corners[0].lat / 1e6, "" );
/* add WHERE statement */
     return snprintf( sql_query, SQL_STR_SIZE, "%s WHERE pk_tile=%lld",
		      strcpy(cbuff,sql_query), tile_id );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_OL2_WR_SQL_NFIT( PGconn *conn, bool be_verbose, const char *flname, 
			   const char *nfit_name, unsigned int num_ngeo, 
			   const struct ngeo_scia *ngeo, unsigned int num_nfit, 
			   const struct nfit_scia *nfit )
{
     const char prognm[] = "SCIA_OL2_WR_SQL_NFIT";

     register unsigned int nf, ng;
     register unsigned int affectedRows = 0u;

     char tbl_name[16];
     char sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];

     bool       do_update;
     char       *cpntr, sciafl[SHORT_STRING_LENGTH];
     int        nrow, numChar, meta_id;
     long long  tile_id;

     PGresult   *res;

     const double SecPerDay = 24. * 60 * 60;

     if ( strncmp( nfit_name, "NAD_UV0_O3", 10 ) == 0 ) {
	  (void) strcpy( tbl_name, "tile_o3_ol" );
     } else if ( strncmp( nfit_name, "NAD_UV1_NO2", 11 ) == 0 ) {
	  (void) strcpy( tbl_name, "tile_no2_ol" );
     } else
	  return;
/*
 * strip path of file-name
 */
     if ( (cpntr = strrchr( flname, '/' )) != NULL ) {
          (void) strlcpy( sciafl, ++cpntr, SHORT_STRING_LENGTH );
     } else {
          (void) strlcpy( sciafl, flname, SHORT_STRING_LENGTH );
     }
/*
 * check if product is already in database
 */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "SELECT pk_meta FROM %s WHERE name=\'%s\'", 
		      META_TBL_NAME, sciafl );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
          NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     if ( (nrow = PQntuples( res )) == 0 ) {
          NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, sciafl );
     }
     cpntr = PQgetvalue( res, 0, 0 );
     meta_id = (int) strtol( cpntr, (char **) NULL, 10 );     
     PQclear( res );
/*
 * Start a transaction block
 */
     res = PQexec( conn, "BEGIN" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
     PQclear( res );
/*
 * insert all Nadir Fit records
 */
     for ( ng = nf = 0; nf < num_nfit; nf++ ) {
	  register double jday = nfit[nf].mjd.days
	       + (nfit[nf].mjd.secnd + nfit[nf].mjd.musec / 1e6) / SecPerDay;

/* check if entry is already present */
	  do_update = FALSE;
          (void) snprintf( sql_query, SQL_STR_SIZE, 
			   CHECK_TABLE_FOR_JDAY, tbl_name, jday );
          res = PQexec( conn, sql_query );
          if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
               NADC_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
	       PQclear( res );
	       res = PQexec( conn, "ROLLBACK" );
	       if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
		    NADC_ERROR( prognm, NADC_ERR_SQL,
				PQresultErrorMessage(res) );
	       goto done;
          }
	  while ( memcmp( &nfit[nf].mjd, &ngeo[ng].mjd, SIZE_MJD ) != 0 
		  && ng < num_ngeo ) ng++;
	  if ( ng == num_ngeo ) {
	       NADC_ERROR( prognm, NADC_ERR_FATAL, "no matching geolocation" );
	       goto done;
	  }

	  if ( PQntuples( res ) != 0 ) {
               do_update = TRUE;
               cpntr = PQgetvalue( res, 0, 0 );
               tile_id = strtoll( cpntr, (char **) NULL, 10 );
               PQclear( res );
               numChar = GET_UPDATE_QUERY( tbl_name, sql_query, 
					   meta_id, tile_id, ngeo+ng, nfit+nf );
          } else {
               PQclear( res );
               /* obtain next value for serial pk_tile */
	       res = PQexec( conn,
			     "SELECT nextval(\'tile_no2_ol_pk_tile_seq\')" );
	       if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, 
				     PQresultErrorMessage(res) );
	       cpntr = PQgetvalue( res, 0, 0 );
	       tile_id = strtoll( cpntr, (char **) NULL, 10 );
	       PQclear( res );
               numChar = GET_INSERT_QUERY( tbl_name, sql_query, meta_id, 
					   tile_id, jday, ngeo+nf, nfit+nf );
          }
	  if ( be_verbose )
	       (void) printf( "%s(): %s [%-d]\n", prognm, sql_query, numChar );
	  if ( numChar >= SQL_STR_SIZE ) {
               NADC_ERROR( prognm, NADC_ERR_STRLEN, "sql_query" );
	       res = PQexec( conn, "ROLLBACK" );
	       if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
		    NADC_ERROR( prognm, NADC_ERR_SQL,
				PQresultErrorMessage(res) );
	       goto done;
	  }
/* do actual insert */
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
	  if ( do_update ) continue;

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
     (void) snprintf( cbuff, SQL_STR_SIZE, "[%s] affectedRows=%-u", 
		      nfit_name, affectedRows );
     NADC_ERROR( prognm, NADC_ERR_NONE, cbuff );
}
