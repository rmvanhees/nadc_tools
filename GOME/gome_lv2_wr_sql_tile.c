/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2007 - 2015 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GOME_LV2_WR_SQL_TILE
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME SQL
.LANGUAGE    ANSI C
.PURPOSE     write tile information of Gome Lv2 product to database
.INPUT/OUTPUT
  call as   GOME_LV2_WR_SQL_TILE( conn, be_verbose, meta_id, version, 
                                  num_ddr, ddr );
     input:  
             PGconn *conn      :  PostgreSQL connection handle
	     bool   be_verbose :  be verbose
	     int    meta_id    :  value of primary key meta__2p.pk_meta
	     char   *version   :  software version from SPH
	     short  num_ddr    :  number of DDR records
	     struct ddr_gome *ddr : structures for DDR data

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     3.1     01-Oct-2012   added verbose flag, RvH
             3.0     02-Dec-2007   rewrite to handle same name but 
                                   different release correctly, RvH
             2.0     27-Jun-2007   port to PostgreSQL by R. M. van Hees
             1.0     31-Jan-2007   initial release by R. M. van Hees
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
#include <limits.h>
#include <float.h>
#include <math.h>

#include <libpq-fe.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++ Macros +++++*/
#define SQL_STR_SIZE   512

#define GEO_POLY_FORMAT \
"%sST_GeomFromText(\
\'POLYGON((%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f))\',4326))"

#define SELECT_FROM_TILE \
"SELECT pk_tileinfo,julianDay,release[1],release[2] \
FROM tileinfo WHERE julianDay BETWEEN %.11f AND %.11f"

#define SELECT_FROM_GDP \
"SELECT fk_tileinfo FROM tile_gdp WHERE julianDay BETWEEN %.11f AND %.11f"

#define SELECT_FROM_META \
"SELECT fk_tileinfo FROM tileinfo_meta__2P WHERE fk_meta = %-d"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
static int release;

static const double mSecPerDay = 1e3 * 24. * 60 * 60;

struct tileinfo_rec {
     unsigned int indxTile;
     unsigned int indxGDP;
     unsigned int indxMeta;
     int          release[2];
     double       jday;
     short        indxDDR;
     double       dtMatch;
};

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static 
int GET_GOME_LV2_RELEASE( const char *version )
{
     const double rbuff = atof( version );

     if ( rbuff >= (4.1 - DBL_EPSILON) )
	  return 5;
     else if ( rbuff >= (4.0 - DBL_EPSILON) )
	  return 4;
     else if ( rbuff >= (3.0 - DBL_EPSILON) )
	  return 3;
     else if ( rbuff >= (2.7 - DBL_EPSILON) )
	  return 2;
     else
	  return 1;
}

/*
 * The table "tileinfo" is check on the presence entries within a time window
 * [zero entries are found]:
 *  ==> insert all tiles, tile_gdp and update "tileinfo_meta__2P" (ready!)
 * [else]:
 *    [less than nr_ddr entries are found]:
 *    ==> insert missing tiles, tile_gdp and update "tileinfo_meta__1P",
 *  ==> add new entries to table "tile_gdp"
 *  ==> add new entries to table "tileinfo_meta__2P"
 */
/*+++++++++++++++++++++++++
.IDENTifer   _FIND_MATCHES
.PURPOSE     all matched in tables: tileinfo, tile_gdp and tileinfo_meta__2P 
             are listed in structure tileRow
.INPUT/OUTPUT
  call as   num_tiles = _FIND_MATCHES( conn, jday_start, jday_end, &tileRow );
     input:
            PGconn *conn        :  PostgreSQL connection handle
	    double jday_start   :  start time of first pixel in product
	    double jday_end     :  end time of last pixel in product
    output:
            struct tileinfo_rec **tileRow :

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static
short _FIND_MATCHES( PGconn *conn, double jday_start, double jday_end, 
		     struct tileinfo_rec **tileRow_out )
{
     register int nr, nt;
     register unsigned int nii;

     char   sql_query[SQL_STR_SIZE];

     char  *pntr;
     int   i_indx, i_date;

     int    numTiles = 0;
     int    numRows = 0;

     struct tileinfo_rec *tileRow;

     PGresult  *res;

     const double SecPerDay  = 24. * 60 * 60;

/* initialization */
     tileRow_out[0] = NULL;

/* select entries from tileinfo within time-window */
     (void) snprintf( sql_query, MAX_STRING_LENGTH, SELECT_FROM_TILE,
                      jday_start, jday_end );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );

/* return when no entries are found */
     if ( (numTiles  = PQntuples( res )) == 0 ) return 0;

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
	  tileRow[nt].indxGDP  = UINT_MAX;
	  tileRow[nt].indxMeta = UINT_MAX;
	  pntr = PQgetvalue( res, nt, i_date );
	  tileRow[nt].jday     = strtod( pntr, (char **) NULL );
	  pntr = PQgetvalue( res, nt, 2 );
	  tileRow[nt].release[0] = strtol( pntr, (char **)NULL, 10 );
	  pntr = PQgetvalue( res, nt, 3 );
	  tileRow[nt].release[1] = strtol( pntr, (char **)NULL, 10 );
	  tileRow[nt].indxDDR  = SHRT_MAX;
	  tileRow[nt].dtMatch  = 0.5 / SecPerDay;
     }
     PQclear( res );

/* select entries from tile_gdp within time-window */
     (void) snprintf( sql_query, MAX_STRING_LENGTH, SELECT_FROM_GDP,
                      jday_start, jday_end );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     numRows = PQntuples( res );

     for ( nr = 0; nr < numRows; nr++ ) {
	  pntr = PQgetvalue( res, nr, 0 );
	  nii = strtoul( pntr, (char **)NULL, 10 );

	  for ( nt = 0; nt < numTiles; nt++ ) {
	       if ( nii == tileRow[nt].indxTile ) {
		    tileRow[nt].indxGDP = nii;
		    break;
	       }
	  }
     }
     PQclear( res );

/* select entries from tileinfo_meta__2P for given product */
/*      (void) snprintf(sql_query, MAX_STRING_LENGTH, SELECT_FROM_META, meta_id); */
/*      res = PQexec( conn, sql_query ); */
/*      if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) */
/*           NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) ); */
/*      numRows = PQntuples( res ); */

/*      for ( nr = 0; nr < numRows; nr++ ) { */
/*           pntr = PQgetvalue( res, nr, 0 ); */
/* 	  nii = strtoul( pntr, (char **)NULL, 10 ); */

/* 	  for ( nt = 0; nt < numRows; nt++ ) { */
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
.PURPOSE     Keep the longitude in range [-180,180]
.INPUT/OUTPUT
  call as   CorrectLongitudes( lon_in, lon_out );
     input:
            float *lon_in   :  longitude from GOME products
    output:
            float *lon_out  :  corrected longitude

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
.IDENTifer   _INSERT_ONE_LV2_TILE
.PURPOSE     add one new entry to tileinfo, tile_gdp, tileinfo_meta__2P
.INPUT/OUTPUT
  call as   _INSERT_ONE_LV2_TILE( conn, be_verbose, meta_id, 
                                  num_pcd, indx_pcd, pcd );
     input:
            PGconn *conn         :  PostgreSQL connection handle
	    bool   be_verbose    :  be verbose
	    int    meta_id       :  value of primary key meta__2p.pk_meta
	    short  num_pcd       :  number of PCD records
	    short  *indx_pcd     :  indices to relevant PCD records
	    struct pcd_gome *pcd :  structures for PCD data

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static
void _INSERT_ONE_LV2_TILE( PGconn *conn, bool be_verbose, int meta_id, 
			   const struct ddr_gome *ddr )
{
     char sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];

     char         *pntr;
     unsigned int tile_id;
     int          numChar;
     float        lon[NUM_COORDS];

     PGresult     *res;

     const double jday = ddr->glr.utc_date + ddr->glr.utc_time / mSecPerDay;
/*
 * get new value for autoincrement variable "pk_tileinfo"
 */
     res = PQexec( conn, "SELECT nextval(\'tileinfo_pk_tileinfo_seq\')" );
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
     (void) snprintf( sql_query, SQL_STR_SIZE, "%sARRAY[-1,%-d],",
		      strcpy(cbuff,sql_query), release );
/* pixelNum */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%d,",
		      strcpy(cbuff,sql_query), ddr->glr.pixel_nr );
/* subSetCounter */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%d,",
		      strcpy(cbuff,sql_query), ddr->glr.subsetcounter );
/* swathType */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), "NULL" );
/* satZenithAngle */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%.6f,",
		      strcpy(cbuff,sql_query), 
		      ddr->glr.sat_zenith[1] );
/* sunZenithAngle */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%.6f,",
		      strcpy(cbuff,sql_query), 
		      ddr->glr.toa_zenith[1] );
/* relAzimuthAngle */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%.6f,",
		      strcpy(cbuff,sql_query), 
		      ddr->glr.toa_azim[1] );
/* tile */
     CorrectLongitudes( ddr->glr.lon, lon );
     numChar = snprintf( sql_query, SQL_STR_SIZE, GEO_POLY_FORMAT,
			 strcpy(cbuff,sql_query), lon[1], ddr->glr.lat[1],
			 lon[3], ddr->glr.lat[3], lon[2], ddr->glr.lat[2],
			 lon[0], ddr->glr.lat[0], lon[1], ddr->glr.lat[1] );
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
 * add entry to tile_gdp
 */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "INSERT INTO tile_gdp VALUES (%.11f,", jday );
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s %u,",
		      strcpy(cbuff,sql_query), tile_id );
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s %.6f,",
		      strcpy(cbuff,sql_query), ddr->ozone  );
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s %.6f,",
		      strcpy(cbuff,sql_query), ddr->error  );
     if ( ddr->irr1 != NULL ) {
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s %.6f,",
			   strcpy(cbuff,sql_query), ddr->irr1->cloud_frac  );
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s %.6f,",
			   strcpy(cbuff,sql_query), ddr->irr1->cloud_pres  );
	  numChar = snprintf( sql_query, SQL_STR_SIZE, "%s %.6f)",
			      strcpy(cbuff,sql_query), 
			      ddr->irr1->surface_pres );
     } else {
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s %.6f,",
			   strcpy(cbuff,sql_query), ddr->irr2->cld_frac  );
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s %.6f,",
			   strcpy(cbuff,sql_query), ddr->irr2->cld_press  );
	  numChar = snprintf( sql_query, SQL_STR_SIZE, "%s %.6f)",
			      strcpy(cbuff,sql_query), 
			      ddr->irr2->surface_press );
     }
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
 * add entry to tileinfo_meta__2P
 */
     numChar = snprintf( sql_query, SQL_STR_SIZE, 
			 "INSERT INTO tileinfo_meta__2P VALUES (%-u,%-d)", 
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
.IDENTifer   _UPDATE_ONE_LV2_TILE
.PURPOSE     update tileinfo, add one new entry to tile_gdp
.INPUT/OUTPUT
  call as   _UPDATE_ONE_LV2_TILE( conn, be_verbose, tileRow, ddr );
     input:
            PGconn *conn         :  PostgreSQL connection handle
	    bool   be_verbose    :  be verbose
	    struct tileinfo_rec *tileRow :
	    struct ddr_gome *ddr :

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static
void _UPDATE_ONE_LV2_TILE( PGconn *conn, bool be_verbose, 
			   const struct tileinfo_rec *tileRow,
			   const struct ddr_gome *ddr )
{
     char     sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];
     int      numChar;
     float    lon[NUM_COORDS];

     PGresult *res;

     const short  nii = tileRow->indxDDR;
     const double jday = 
	  ddr[nii].glr.utc_date + ddr[nii].glr.utc_time / mSecPerDay;
/* 
 * update existing tileinfo record (all fields!)
 */
     (void) strcpy( sql_query, "UPDATE tileinfo SET "
		    "(julianDay,release[2],pixelNumber,satZenithAngle,"
		    "sunZenithAngle,relAzimuthAngle,tile)" );

     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "%s = (%.11f,%-d,%-d,%.6f,%.6f,%.6f,", 
		      strcpy(cbuff,sql_query), jday, release, 
		      ddr[nii].glr.pixel_nr, ddr[nii].glr.sat_zenith[1],
		      ddr[nii].glr.toa_zenith[1], ddr[nii].glr.toa_azim[1] );
     CorrectLongitudes( ddr[nii].glr.lon, lon );
     (void) snprintf( sql_query, SQL_STR_SIZE, GEO_POLY_FORMAT,
		      strcpy(cbuff,sql_query), 
		      lon[1], ddr[nii].glr.lat[1],
		      lon[3], ddr[nii].glr.lat[3], 
		      lon[2], ddr[nii].glr.lat[2],
		      lon[0], ddr[nii].glr.lat[0], 
		      lon[1], ddr[nii].glr.lat[1] );
     numChar = snprintf( sql_query, SQL_STR_SIZE, "%s WHERE pk_tileinfo=%u",
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
/*
 * add or update entry to tile_gdp
 */
     if ( tileRow->indxGDP == UINT_MAX ) {
	  (void) snprintf( sql_query, SQL_STR_SIZE, 
			   "INSERT INTO tile_gdp VALUES (%.11f,", jday );
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s %u,",
			   strcpy(cbuff,sql_query), 
			   tileRow->indxTile );
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s %.6f,",
			   strcpy(cbuff,sql_query), ddr[nii].ozone  );
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s %.6f,",
			   strcpy(cbuff,sql_query), ddr[nii].error  );
	  if ( ddr[nii].irr1 != NULL ) {
	       (void) snprintf( sql_query, SQL_STR_SIZE, "%s %.6f,",
				strcpy(cbuff,sql_query), 
				ddr[nii].irr1->cloud_frac );
	       (void) snprintf( sql_query, SQL_STR_SIZE, "%s %.6f,",
				strcpy(cbuff,sql_query), 
				ddr[nii].irr1->cloud_pres );
	       numChar = snprintf( sql_query, SQL_STR_SIZE, "%s %.6f)",
				   strcpy(cbuff,sql_query),
				   ddr[nii].irr1->surface_pres );
	  } else {
	       (void) snprintf( sql_query, SQL_STR_SIZE, "%s %.6f,",
				strcpy(cbuff,sql_query), 
				ddr[nii].irr2->cld_frac );
	       (void) snprintf( sql_query, SQL_STR_SIZE, "%s %.6f,",
				strcpy(cbuff,sql_query), 
				ddr[nii].irr2->cld_press );
	       numChar = snprintf( sql_query, SQL_STR_SIZE, "%s %.6f)",
				   strcpy(cbuff,sql_query),
				   ddr[nii].irr2->surface_press );
	  }
     } else { /* update */
	  (void) strcpy( sql_query, "UPDATE tile_gdp SET "
			 "(o3_gdp,o3_gdp_err,cloudFraction,"
			 "cloudTopPress,surfacePress)" );
	  if ( ddr[nii].irr1 != NULL ) {
	       (void) snprintf( sql_query, SQL_STR_SIZE, 
				"%s = (%.6f,%.6f,%.6f,%.6f,%.6f)",
				strcpy(cbuff,sql_query), ddr[nii].ozone, 
				ddr[nii].error, ddr[nii].irr1->cloud_frac, 
				ddr[nii].irr1->cloud_pres, 
				ddr[nii].irr1->surface_pres );
	  } else {
	       (void) snprintf( sql_query, SQL_STR_SIZE, 
				"%s = (%.6f,%.6f,%.6f,%.6f,%.6f)",
				strcpy(cbuff,sql_query), ddr[nii].ozone, 
				ddr[nii].error, ddr[nii].irr2->cld_frac, 
				ddr[nii].irr2->cld_press, 
				ddr[nii].irr2->surface_press );
	  }
	  numChar = snprintf( sql_query, SQL_STR_SIZE, 
			      "%s WHERE fk_tileinfo=%u",
			      strcpy(cbuff,sql_query), tileRow->indxTile );
     }
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

/*+++++++++++++++++++++++++
.IDENTifer   _INSERT_ONE_LV2_TILE2META
.PURPOSE     add one entry to table tileinfo_meta__2P
.INPUT/OUTPUT
  call as   _INSERT_ONE_LV2_TILE2META( conn, be_verbose, meta_id, tileRow );
     input:
            PGconn *conn         :  PostgreSQL connection handle
	    bool   be_verbose    :  be verbose
	    int    meta_id       :  value of primary key meta__1p.pk_meta
	    struct tileinfo_rec *tileRow :

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
static
void _INSERT_ONE_LV2_TILE2META( PGconn *conn, bool be_verbose, int meta_id, 
				const struct tileinfo_rec *tileRow )
{
     char sql_query[SQL_STR_SIZE];

     int numChar;

     PGresult *res;
/*
 * add entry to tileinfo_meta__2P
 */
     if ( tileRow->indxMeta == UINT_MAX ) {
	  numChar = snprintf( sql_query, SQL_STR_SIZE, 
			      "INSERT INTO tileinfo_meta__2P VALUES (%-u,%-d)",
			      tileRow->indxTile, meta_id );
	  if ( be_verbose )
	       (void) printf( "%s() %s [%-d]\n", __func__, sql_query, numChar );
	  if ( numChar >= SQL_STR_SIZE )
	       NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
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
void GOME_LV2_WR_SQL_TILE( PGconn *conn, bool be_verbose, int meta_id, 
			   const char *version,
			   short num_ddr, const struct ddr_gome *ddr )
{
     register short  nd, nr;
     register double jday;

     register unsigned int affectedRows;

     char    cbuff[SHORT_STRING_LENGTH];
     short   numRows;

     const short lastRec = (num_ddr == 0) ? 0 : num_ddr - 1;
     const double jdayStart = 
	  ddr->glr.utc_date + ddr->glr.utc_time / mSecPerDay;
     const double jdayEnd   = ddr[lastRec].glr.utc_date 
	  + (ddr[lastRec].glr.utc_time + 10) / mSecPerDay;

     PGresult *res;

     struct tileinfo_rec *tileRow = NULL;
/*
 * check number of ddr-records
 */
     if ( num_ddr == 0 )
	  NADC_RETURN_ERROR( NADC_ERR_NONE, 
			     "product does not contain any MDS records" );
/*
 * set global variable "release"
 */
     release = GET_GOME_LV2_RELEASE( version );
/*
 * get all potential matching tiles from table "tileinfo"
 */
     numRows = _FIND_MATCHES( conn, jdayStart, jdayEnd, &tileRow );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, "_FIND_MATCHES" );
/*
 *--------------------------------------------------
 * all ddr-records are new: do a simple insert and exit
 */
     if ( numRows == 0 ) {
	  res = PQexec( conn, "BEGIN" );
          if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
               NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
               PQclear( res );
               return;
          }
          PQclear( res );
	  affectedRows = 0u;
	  for ( nd = 0; nd < num_ddr; nd++ ) {
	       _INSERT_ONE_LV2_TILE( conn, be_verbose, meta_id, ddr+nd );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_RETURN_ERROR( NADC_ERR_FATAL, 
				       "_INSERT_ONE_LV2_TILE" );
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
 * add new entries to tables tileinfo, tile_gdp and tileinfo_meta__2P
 */
     res = PQexec( conn, "BEGIN" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
          NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
          PQclear( res );
          goto done;
     }
     affectedRows = 0;
     for ( nd = 0; nd < num_ddr; nd++ ) {
	  bool found = FALSE;

	  jday = ddr[nd].glr.utc_date + ddr[nd].glr.utc_time / mSecPerDay;

	  for ( nr = 0; nr < numRows; nr++ ) {
	       register double dtime = fabs(jday - tileRow[nr].jday);

	       if ( tileRow[nr].dtMatch > dtime ) {
		    found = TRUE;
		    tileRow[nr].indxDDR = nd;
		    tileRow[nr].dtMatch = dtime;
	       }
	  }
	  if ( ! found ) {
	       _INSERT_ONE_LV2_TILE( conn, be_verbose, meta_id, ddr+nd );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_FATAL, 
				     "_INSERT_ONE_LV2_TILE" );
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
 * update tileinfo and add entries to tile_gdp and tileinfo_meta__2P
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
	  if ( tileRow[nr].indxDDR == SHRT_MAX ) continue;
	  if ( tileRow[nr].release[1] <= release ) {
	       _UPDATE_ONE_LV2_TILE( conn, be_verbose, tileRow+nr, ddr );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_FATAL, 
				     "_UPDATE_ONE_LV2_TILE" );
	       affectedRows++;
	  }
	  _INSERT_ONE_LV2_TILE2META( conn, be_verbose, meta_id, tileRow+nr );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, 
				"_INSERT_ONE_LV2_TILE2META" );
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
