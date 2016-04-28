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

.IDENTifer   GOME_LV1_WR_SQL_TILE
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME SQL
.LANGUAGE    ANSI C
.PURPOSE     write tile information of Gome Lv1 product to database
.INPUT/OUTPUT
  call as   GOME_LV1_WR_SQL_TILE( conn, be_verbose, meta_id, version, 
                                  num_pcd, indx_pcd, pcd );
     input:  
             PGconn *conn         :  PostgreSQL connection handle
	     bool   be_verbose    :  be verbose
	     int    meta_id       :  value of primary key meta__1p.pk_meta
	     char   *version      :  software version from SPH
	     short  num_pcd       :  number of PCD records
	     short  *indx_pcd     :  indices to relevant PCD records
	     struct pcd_gome *pcd : structures for PCD data

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     3.1     01-Oct-2012   added verbose flag, RvH
             3.0     03-Dec-2007   rewrite to handle same name but 
                                   different release correctly, RvH
             2.0     20-Mar-2007   port to PostgreSQL by R. M. van Hees
             1.0     29-Jan-2007   initial release by R. M. van Hees
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
#include <float.h>
#include <math.h>

#include <libpq-fe.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++ Macros +++++*/
#define SQL_STR_SIZE   768

#define GEO_POLY_FORMAT \
"%s ST_GeomFromText(\
\'POLYGON((%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f))\',4326))"

#define PMD_INGEST_FORMAT \
"ARRAY[%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f]"

#define SELECT_FROM_TILE \
"SELECT pk_tileinfo,julianDay,release[1],release[2] \
FROM tileinfo WHERE julianDay BETWEEN %.11f AND %.11f"

#define SELECT_FROM_PMD \
"SELECT fk_tileinfo FROM tile_pmd WHERE julianDay BETWEEN %.11f AND %.11f"

#define SELECT_FROM_META \
"SELECT fk_tileinfo FROM tileinfo_meta__1P WHERE fk_meta=%-d"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
static int release;

static const double mSecPerDay = 1e3 * 24. * 60 * 60;

struct tileinfo_rec {
     unsigned int indxTile;
     unsigned int indxPMD;
     unsigned int indxMeta;
     int          release[2];
     double       jday;
     short        indxPCD;
     double       dtMatch;
};

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static 
int GET_GOME_LV1_RELEASE( const char *version )
{
     const double rbuff = atof( version );

     if ( rbuff >= (3.0 - DBL_EPSILON) )
	  return 4;
     else if ( rbuff >= (2.2 - DBL_EPSILON) )
	  return 3;
     else if ( rbuff >= (2.0 - DBL_EPSILON) )
	  return 2;
     else
	  return 1;
}

/*
 * The table "tileinfo" is check on the presence entries within a time window
 * [zero entries are found]:
 *  ==> insert all tiles, PMD readouts and update "tileinfo_meta__1P" (ready!)
 * [else]:
 *    [less than nr_pcd entries are found]:
 *    ==> insert missing tiles, PMD readouts and update "tileinfo_meta__1P",
 *  ==> add new entries to table "tile_pmd"
 *  ==> add new entries to table "tileinfo_meta__1P"
 */
/*+++++++++++++++++++++++++
.IDENTifer   _FIND_MATCHES
.PURPOSE     all matched in tables: tileinfo, tile_pmd and tileinfo_meta__1P 
             are listed in structure tileRow
.INPUT/OUTPUT
  call as   num_tiles = _FIND_MATCHES( conn, be_verbose, 
                                       jday_start, jday_end, &tileRow );
     input:
            PGconn *conn        :   PostgreSQL connection handle
	    bool   be_verbose   :   be verbose
	    double jday_start   :   start time of first pixel in product
	    double jday_end     :   end time of last pixel in product
    output:
            struct tileinfo_rec **tileRow :

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static
short _FIND_MATCHES( PGconn *conn, bool be_verbose, double jday_start, 
		     double jday_end, struct tileinfo_rec **tileRow_out )
{
     register int nr, nt;

     char   sql_query[SQL_STR_SIZE];

     char   *pntr;
     int    i_indx, i_date;

     int    numTiles = 0;
     int    numRows = 0;
     int    numChar;

     struct tileinfo_rec *tileRow;

     PGresult   *res;

     const double SecPerDay  = 24. * 60 * 60;

/* initialisation */
     tileRow_out[0] = NULL;

/* select entries from tileinfo within time-window */
     (void) snprintf( sql_query, SQL_STR_SIZE, SELECT_FROM_TILE,
                      jday_start, jday_end );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
	  NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );

/* return when no entries are found */
     if ( (numTiles = PQntuples( res )) == 0 ) return 0;

/* fill tileRow structure with entries found */
     tileRow_out[0] = (struct tileinfo_rec *)
	  malloc( numTiles * sizeof(struct tileinfo_rec) );
     if ( (tileRow = tileRow_out[0]) == NULL ) {
	  NADC_ERROR( NADC_ERR_ALLOC, "tileRow" );
	  return 0;
     }
     i_indx  = PQfnumber( res, "pk_tileinfo" );
     i_date  = PQfnumber( res, "julianDay" );
     for ( nt = 0; nt < numTiles; nt++ ) {
	  pntr = PQgetvalue( res, nt, i_indx );
	  tileRow[nt].indxTile = strtoul( pntr, (char **)NULL, 10 );
	  tileRow[nt].indxPMD  = UINT_MAX;
	  tileRow[nt].indxMeta = UINT_MAX;
	  pntr = PQgetvalue( res, nt, i_date );
	  tileRow[nt].jday     = strtod( pntr, (char **) NULL );
	  pntr = PQgetvalue( res, nt, 2 );
	  tileRow[nt].release[0] = strtol( pntr, (char **)NULL, 10 );
	  pntr = PQgetvalue( res, nt, 3 );
	  tileRow[nt].release[1] = strtol( pntr, (char **)NULL, 10 );	  
	  tileRow[nt].indxPCD  = SHRT_MAX;
	  tileRow[nt].dtMatch  = 1. / SecPerDay;
     }
     PQclear( res );

/* select entries from tile_pmd within time-window */
     numChar = snprintf( sql_query, SQL_STR_SIZE, SELECT_FROM_PMD,
			 jday_start, jday_end );
     if ( be_verbose )
	  (void) printf( "%s(): %s[%-d]\n", __func__, sql_query, numChar );
     if ( numChar >= SQL_STR_SIZE ) {
	  NADC_ERROR( NADC_ERR_STRLEN, "sql_query" );
	  return 0;
     }
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
	  NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     numRows = PQntuples( res );

     for ( nr = 0; nr < numRows; nr++ ) {
	  register unsigned int nii;

	  pntr = PQgetvalue( res, nr, 0 );
	  nii = strtoul( pntr, (char **)NULL, 10 );

	  for ( nt = 0; nt < numTiles; nt++ ) {
	       if ( nii == tileRow[nt].indxTile ) {
		    tileRow[nt].indxPMD = nii;
		    break;
	       }
	  }
     }
     PQclear( res );

/* select entries from tileinfo_meta__1P for given product */
/*      (void) snprintf( sql_query, SQL_STR_SIZE, SELECT_FROM_META, meta_id ); */
/*      res = PQexec( conn, sql_query ); */
/*      if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) */
/* 	  NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) ); */
/*      numRows = PQntuples( res ); */

/*      for ( nr = 0; nr < numRows; nr++ ) { */
/* 	  register unsigned int nii; */

/* 	  pntr = PQgetvalue( res, nr, 0 ); */
/* 	  nii = (unsigned int) strtoul( pntr, (char **)NULL, 10 ); */

/* 	  for ( nt = 0; nt < numTiles; nt++ ) { */
/* 	       if ( nii == tileRow[nt].indxTile ) { */
/* 		    tileRow[nt].indxMeta = nii; */
/* 		    break; */
/* 	       } */
/* 	  } */
/*      } */
     return numTiles;
done:
     PQclear( res );
     return numTiles;
}

/*+++++++++++++++++++++++++
.IDENTifer   CorrectLongitudes
.PURPOSE     keep longitude withing range [-180,180]
.INPUT/OUTPUT
  call as   CorrectLongitudes( lon_in, lon_out );

     input:
            float *lon_in   :
    output:
            float *lon_out  :

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static
void CorrectLongitudes( const float *lon_in, /*@out@*/ float *lon_out )
{
     register unsigned short nc = 0;

     do {
	  lon_out[nc] = LON_IN_RANGE( lon_in[nc] );
	  if ( fabsf(lon_out[0] - lon_out[nc]) > 270.f ) {
	       if ( lon_out[0] > 0.f )
		    lon_out[nc] += 360.f;
	       else
		    lon_out[nc] -= 360.f;
	  }
     } while ( ++nc < NUM_COORDS );
}

/*+++++++++++++++++++++++++
.IDENTifer   _INSERT_ONE_LV1_TILE
.PURPOSE     add new entry to tileinfo, tile_pmd, tileinfo_meta__1P
.INPUT/OUTPUT
  call as   _INSERT_ONE_LV1_TILE( conn, be_verbose, meta_id, pixelNumber, pcd );

     input:
            PGconn *conn         :  PostgreSQL connection handle
	    bool   be_verbose    :  be verbose
	    int    meta_id       :  value of primary key meta__1p.pk_meta
	    short  pixelNumber   :  ground pixel number
	    struct pcd_gome *pcd :  structure with PCD data

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static
void _INSERT_ONE_LV1_TILE( PGconn *conn, bool be_verbose, int meta_id, 
			   short pixelNumber, const struct pcd_gome *pcd )
{
     register short pg;

     char sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];

     char         *pntr;
     int          numChar;
     unsigned int tile_id;
     float        lon[NUM_COORDS];

     PGresult *res;

     const double jday = pcd->glr.utc_date + pcd->glr.utc_time / mSecPerDay;
/*
 * get new value for autoincrement variable "pk_tileinfo"
 */
     res = PQexec( conn, 
		   "SELECT nextval(\'tileinfo_pk_tileinfo_seq\')" );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
	  NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	  PQclear( res );
	  return;
     }
     pntr = PQgetvalue( res, 0, 0 );
     tile_id = strtoul( pntr, (char **) NULL, 10 );
     PQclear( res );
/* 
 * build ingest string, note that pk_tileinfo is declared AUTOINCREMENT
 */
/* pk_tileinfo */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "INSERT INTO tileinfo VALUES (%u,", tile_id );	  
/* julianDay */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%.11f,",
		      strcpy(cbuff,sql_query), jday );
/* release */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%sARRAY[%-d,-1],",
		      strcpy(cbuff,sql_query), release );
/* pixelNum */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%hd,",
		      strcpy(cbuff,sql_query), pixelNumber );
/* subSetCounter */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%hu,",
		      strcpy(cbuff,sql_query), 
		      pcd->ihr.subsetcounter );
/* swathType */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), "NULL" );
/* satZenithAngle */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), "NULL" );
/* sunZenithAngle */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), "NULL" );
/* relAzimuthAngle */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), "NULL" );
/* tile */
     CorrectLongitudes( pcd->glr.lon, lon );
     numChar = snprintf( sql_query, SQL_STR_SIZE, GEO_POLY_FORMAT,
			      strcpy(cbuff,sql_query), 
			      lon[1], pcd->glr.lat[1],
			      lon[3], pcd->glr.lat[3], 
			      lon[2], pcd->glr.lat[2],
			      lon[0], pcd->glr.lat[0], 
			      lon[1], pcd->glr.lat[1] );
     if ( be_verbose )
	  (void) printf( "%s(): %s [%-d]\n", __func__, sql_query, numChar );
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
	  PQclear( res );
          return;
     }
     PQclear( res );
/*
 * add entry to tile_pmd
 */
/* julianDay & fk_tileinfo */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "INSERT INTO tile_pmd VALUES ( %.11f,%u",
		      jday, tile_id );
/* PMD values */
     for ( pg = 0; pg < 3; pg++ ) {
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s,"PMD_INGEST_FORMAT,
			   strcpy(cbuff,sql_query), pcd->pmd[0].value[pg], 
			   pcd->pmd[1].value[pg], pcd->pmd[2].value[pg], 
			   pcd->pmd[3].value[pg], pcd->pmd[4].value[pg], 
			   pcd->pmd[5].value[pg], pcd->pmd[6].value[pg], 
			   pcd->pmd[7].value[pg], pcd->pmd[8].value[pg], 
			   pcd->pmd[9].value[pg], pcd->pmd[10].value[pg], 
			   pcd->pmd[11].value[pg], pcd->pmd[12].value[pg], 
			   pcd->pmd[13].value[pg], pcd->pmd[14].value[pg], 
			   pcd->pmd[15].value[pg] );
     }
     numChar = nadc_strlcat( sql_query, ")", SQL_STR_SIZE );
     if ( be_verbose )
	  (void) printf( "%s(): %s [%-d]\n", __func__, sql_query, numChar );
     if ( numChar >= SQL_STR_SIZE )
	  NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	  NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	  PQclear( res );
	  res = PQexec( conn, "ROLLBACK" );
	  if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
	       NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	  PQclear( res );
          return;
     }
     PQclear( res );
/*
 * add entry to tileinfo_meta__1P
 */
     numChar = snprintf( sql_query, SQL_STR_SIZE, 
			 "INSERT INTO tileinfo_meta__1P VALUES (%-u,%-d",
			 tile_id, meta_id );
     if ( be_verbose )
	  (void) printf( "%s(): %s [%-d]\n", __func__, sql_query, numChar );
     if ( numChar >= SQL_STR_SIZE )
	  NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	  NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	  PQclear( res );
	  res = PQexec( conn, "ROLLBACK" );
	  if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
	       NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     PQclear( res );
}

/*+++++++++++++++++++++++++
.IDENTifer   _UPDATE_ONE_LV1_TILE
.PURPOSE     update tileinfo, add new entries to tile_pmd, tileinfo_meta__2P
.INPUT/OUTPUT
  call as   _UPDATE_ONE_LV1_TILE( conn, be_verbose, tileRow, pcd );
     input:
            PGconn *conn         :  PostgreSQL connection handle
	    bool   be_verbose    :   be verbose
	    struct tileinfo_rec *tileRow :
	    struct pcd_gome *pcd :

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static
void _UPDATE_ONE_LV1_TILE( PGconn *conn, bool be_verbose,
			   const struct tileinfo_rec *tileRow,
			   const struct pcd_gome *pcd )
{
     register short pg;

     char     sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];
     int      numChar;
     float    lon[NUM_COORDS];

     PGresult *res;

     const short  nii  = tileRow->indxPCD;
     const double jday = 
	  pcd[nii].glr.utc_date + pcd[nii].glr.utc_time / mSecPerDay;
/* 
 * update existing tileinfo record (Lv1b fields only)
 */
     if ( tileRow->release[1] == -1 ) {
	  (void) strcpy( sql_query, 
			 "UPDATE tileinfo SET (julianDay,release[1],tile)" );

	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s = (%.11f,%-d,", 
			   strcpy(cbuff,sql_query), jday, release );
	  CorrectLongitudes( pcd[nii].glr.lon, lon );
	  (void) snprintf( sql_query, SQL_STR_SIZE, GEO_POLY_FORMAT,
			   strcpy(cbuff,sql_query), 
			   lon[1], pcd[nii].glr.lat[1],
			   lon[3], pcd[nii].glr.lat[3], 
			   lon[2], pcd[nii].glr.lat[2],
			   lon[0], pcd[nii].glr.lat[0], 
			   lon[1], pcd[nii].glr.lat[1] );
	  numChar = snprintf( sql_query, SQL_STR_SIZE, 
			      "%s WHERE pk_tileinfo=%u",
			      strcpy(cbuff,sql_query), tileRow->indxTile );
	  if ( be_verbose )
	       (void) printf( "%s(): %s [%-d]\n", __func__, sql_query, numChar );
	  if ( numChar >= SQL_STR_SIZE )
	       NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
	  res = PQexec( conn, sql_query );
	  if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	       NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	       PQclear( res );
	       res = PQexec( conn, "ROLLBACK" );
	       if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
		    NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	       PQclear( res );
	       return;
	  }
	  PQclear( res );
     }
/*
 * add or update entry to gdp__2P
 */
     if ( tileRow->indxPMD == UINT_MAX ) {
/* julianDay & fk_tileinfo */
	  (void) snprintf( sql_query, SQL_STR_SIZE, 
			   "INSERT INTO tile_pmd VALUES (%.11f,%u",
			   jday, tileRow->indxTile );
	  for ( pg = 0; pg < 3; pg++ ) {
	       (void) snprintf( sql_query, SQL_STR_SIZE, 
				"%s,"PMD_INGEST_FORMAT, 
				strcpy(cbuff,sql_query),
				pcd[nii].pmd[0].value[pg], 
				pcd[nii].pmd[1].value[pg], 
				pcd[nii].pmd[2].value[pg], 
				pcd[nii].pmd[3].value[pg], 
				pcd[nii].pmd[4].value[pg], 
				pcd[nii].pmd[5].value[pg], 
				pcd[nii].pmd[6].value[pg], 
				pcd[nii].pmd[7].value[pg], 
				pcd[nii].pmd[8].value[pg], 
				pcd[nii].pmd[9].value[pg], 
				pcd[nii].pmd[10].value[pg], 
				pcd[nii].pmd[11].value[pg], 
				pcd[nii].pmd[12].value[pg], 
				pcd[nii].pmd[13].value[pg], 
				pcd[nii].pmd[14].value[pg], 
				pcd[nii].pmd[15].value[pg] );
	  }
	  numChar = nadc_strlcat( sql_query, ")", SQL_STR_SIZE );
	  if ( be_verbose )
	       (void) printf( "%s(): %s [%-d]\n", __func__, sql_query, numChar );
	  if ( numChar >= SQL_STR_SIZE )
	       NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
     } else { /* update */
	  (void) strcpy( sql_query, 
			 "UPDATE tile_pmd SET (pmd_1,pmd_2,pmd_3) = (" );
	  for ( pg = 0; pg < 3; pg++ ) {
	       if ( pg > 0 )
		    (void) snprintf( sql_query, SQL_STR_SIZE, "%s,",
				     strcpy(cbuff,sql_query) );
	       (void) snprintf( sql_query, SQL_STR_SIZE, 
				"%s"PMD_INGEST_FORMAT,
				strcpy(cbuff,sql_query), 
				pcd[nii].pmd[0].value[pg], 
				pcd[nii].pmd[1].value[pg], 
				pcd[nii].pmd[2].value[pg], 
				pcd[nii].pmd[3].value[pg], 
				pcd[nii].pmd[4].value[pg], 
				pcd[nii].pmd[5].value[pg], 
				pcd[nii].pmd[6].value[pg], 
				pcd[nii].pmd[7].value[pg], 
				pcd[nii].pmd[8].value[pg], 
				pcd[nii].pmd[9].value[pg], 
				pcd[nii].pmd[10].value[pg], 
				pcd[nii].pmd[11].value[pg], 
				pcd[nii].pmd[12].value[pg], 
				pcd[nii].pmd[13].value[pg], 
				pcd[nii].pmd[14].value[pg], 
				pcd[nii].pmd[15].value[pg] );
	  }
	  numChar = snprintf( sql_query, SQL_STR_SIZE, 
			      "%s) WHERE fk_tileinfo=%u",
			      strcpy(cbuff,sql_query), tileRow->indxTile );
	  if ( be_verbose )
	       (void) printf( "%s(): %s [%-d]\n", __func__, sql_query, numChar );
	  if ( numChar >= SQL_STR_SIZE )
	       NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
     }
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	  NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	  PQclear( res );
	  res = PQexec( conn, "ROLLBACK" );
	  if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
	       NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	  PQclear( res );
	  return;
     }
     PQclear( res );
}

/*+++++++++++++++++++++++++
.IDENTifer   _INSERT_ONE_LV1_TILE2META
.PURPOSE     add one entry to table tileinfo_meta__1P
.INPUT/OUTPUT
  call as   _INSERT_ONE_LV1_TILE2META( conn, meta_id, tileRow );
     input:
            PGconn *conn         :  PostgreSQL connection handle
	    int    meta_id       :  value of primary key meta__1p.pk_meta
	    struct tileinfo_rec *tileRow :

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static
void _INSERT_ONE_LV1_TILE2META( PGconn *conn, bool be_verbose, int meta_id, 
				const struct tileinfo_rec *tileRow )
{
     char sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];

     int numChar;

     PGresult *res;
/*
 * add entry to tileinfo_meta__2P
 */
     if ( tileRow->indxMeta == UINT_MAX ) {
	  (void) snprintf( sql_query, SQL_STR_SIZE, 
			   "INSERT INTO tileinfo_meta__1P VALUES (%-u,",
			   tileRow->indxTile );
	  numChar = snprintf( sql_query, SQL_STR_SIZE, "%s%-d)",
			   strcpy(cbuff,sql_query), meta_id );
	  if ( be_verbose )
	       (void) printf( "%s(): %s [%-d]\n", __func__, sql_query, numChar );
	  res = PQexec( conn, sql_query );
	  if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	       NADC_ERROR( NADC_ERR_SQL,
			   PQresultErrorMessage(res) );
	       PQclear( res );
	       res = PQexec( conn, "ROLLBACK" );
	       if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
		    NADC_ERROR( NADC_ERR_SQL,
				PQresultErrorMessage(res) );
	       PQclear( res );
	       return;
	  }
	  PQclear( res );
     }
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_LV1_WR_SQL_TILE( PGconn *conn, bool be_verbose, int meta_id,
			   const char *version, short num_pcd, 
			   const short *indx_pcd, const struct pcd_gome *pcd )
{
     register short  np, nr;
     register double jday;

     register unsigned int affectedRows = 0u;

     char    cbuff[SHORT_STRING_LENGTH];
     short   numRows;

     PGresult *res;

     struct tileinfo_rec *tileRow = NULL;

     const double jdayStart = pcd[indx_pcd[0]].glr.utc_date
	  + pcd[indx_pcd[0]].glr.utc_time / mSecPerDay;
     const double jdayEnd   = pcd[indx_pcd[num_pcd-1]].glr.utc_date
	  + (pcd[indx_pcd[num_pcd-1]].glr.utc_time + 10) / mSecPerDay;
/*
 * check number of pcd-records
 */
     if ( num_pcd == 0 )
	  NADC_RETURN_ERROR( NADC_ERR_NONE, 
			     "product does not contain any MDS records" );
/*
 * set global variable "release"
 */
     release = GET_GOME_LV1_RELEASE( version );
/*
 * get all potential matching tiles from table "tileinfo"
 */
     numRows = _FIND_MATCHES( conn, be_verbose, jdayStart, jdayEnd, &tileRow );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, "_FIND_MATCHES" );
/*
 *--------------------------------------------------
 * all pcd-records are new: do a simple insert and exit
 */
     if ( numRows == 0 ) {
	  res = PQexec( conn, "BEGIN" );
          if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
               NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
               PQclear( res );
               return;
          }
          PQclear( res );
	  for ( np = 0; np < num_pcd; np++ ) {
	       register short nii = indx_pcd[np];

	       _INSERT_ONE_LV1_TILE(conn, be_verbose, meta_id, nii+1, pcd+nii);
	       if ( IS_ERR_STAT_FATAL )
		    NADC_RETURN_ERROR( NADC_ERR_FATAL, 
				       "_INSERT_ONE_LV1_TILE" );
	       affectedRows++;
	  }
	  res = PQexec( conn, "COMMIT" );
          if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
               NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
          PQclear( res );
	  (void) snprintf( cbuff, SHORT_STRING_LENGTH, 
			   "added Rows=%-u", affectedRows );
	  NADC_RETURN_ERROR( NADC_ERR_NONE, cbuff );
     }
/*
 *--------------------------------------------------
 * add new entries to tables tileinfo, gdp__2P and tileinfo_meta__2P
 */
     res = PQexec( conn, "BEGIN" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
          NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
          PQclear( res );
          goto done;
     }
     affectedRows = 0;
     for ( np = 0; np < num_pcd; np++ ) {
	  bool found = FALSE;
	  register short nii = indx_pcd[np];

	  jday = pcd[nii].glr.utc_date + pcd[nii].glr.utc_time / mSecPerDay;

	  for ( nr = 0; nr < numRows; nr++ ) {
	       register double dtime = fabs(jday - tileRow[nr].jday);

	       if ( tileRow[nr].dtMatch > dtime ) {
		    found = TRUE;
		    tileRow[nr].indxPCD = nii;
		    tileRow[nr].dtMatch = dtime;
	       }
	  }
	  if ( ! found ) {
	       _INSERT_ONE_LV1_TILE(conn, be_verbose, meta_id, nii+1, pcd+nii);
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_FATAL, 
				     "_INSERT_ONE_LV1_TILE" );
	       affectedRows++;
	  }
     }
     res = PQexec( conn, "COMMIT" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
          NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	  PQclear( res );
          goto done;
     }
     PQclear( res );
     if ( affectedRows != 0u ) {
	  (void) snprintf( cbuff, SHORT_STRING_LENGTH, 
			   "added Rows=%-u", affectedRows );
	  NADC_ERROR( NADC_ERR_NONE, cbuff );
     }
/*
 * --------------------------------------------------
 * update tileinfo and add entries to tile_pmd and tileinfo_meta__1P
 *
 */
     res = PQexec( conn, "BEGIN" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
          NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
          PQclear( res );
          goto done;
     }
     affectedRows = 0;     
     for ( nr = 0; nr < numRows; nr++ ) {
	  if ( tileRow[nr].indxPCD == SHRT_MAX ) continue;
	  if ( tileRow[nr].release[0] <= release ) {
	       _UPDATE_ONE_LV1_TILE( conn, be_verbose, tileRow+nr, pcd );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_FATAL, 
				     "_INSERT_ONE_LV1_TILE" );
	       affectedRows++;
	  }
	  _INSERT_ONE_LV1_TILE2META( conn, be_verbose, meta_id, tileRow+nr );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, 
				"_INSERT_ONE_LV1_TILE2META" );
     }
     res = PQexec( conn, "COMMIT" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
          NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
	  PQclear( res );
          goto done;
     }
     PQclear( res );
     if ( affectedRows != 0u ) {
	  (void) snprintf( cbuff, SHORT_STRING_LENGTH, 
			   "updated Rows=%-u", affectedRows );
	  NADC_ERROR( NADC_ERR_NONE, cbuff );
     }
 done:
     free( tileRow );
}
