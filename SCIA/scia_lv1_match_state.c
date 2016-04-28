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

.IDENTifer   SCIA_LV1_MATCH_STATE
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA SQL
.LANGUAGE    ANSI C
.PURPOSE     write state information of Sciamachy Lv1b product to database
.INPUT/OUTPUT
  call as   SCIA_LV1_MATCH_STATE( conn, be_verbose, mph, numState, 
                                  lads, sqads, state );
     input:  
             PGconn *conn              : PostgreSQL connection handle
	     bool be_verbose           : be verbose
	     struct mph_envi *mph      : structure for MPH record
	     unsigned short numState   : number of state records
	     struct lads_scia *lads    : structure for LADS records
	     struct sqads1_scia *sqads : structure for SQADS records
	     struct state1_scia *state : structure for STATE records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     2.1     27-Sep-2012   use array for stateinfo keys, RvH
             2.0.1   14-Dec-2010   check numState, RvH
             2.0     08-Jun-2007   port to PostgreSQL by R. M. van Hees
             1.0     30-Jan-2007   initial release by R. M. van Hees
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
#include <math.h>

#include <libpq-fe.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++ Macros +++++*/
#define SQL_STR_SIZE   300

#define NO_DMOP_FOUND \
"No states found in DMOP list (forgot to update?)"

#define NO_MATCHES_FOUND \
"None of the states in product match with DMOP list"

#define GEO_POLY_FORMAT \
"%s %sST_GeomFromText(\
\'POLYGON((%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f,%.6f %.6f))\',%-d)"

#define SELECT_FROM_STATEINFO \
"SELECT pk_stateinfo,stateID,dateTimeStart,muSecStart,softVersion\
 FROM stateinfo WHERE dateTimeStart BETWEEN\
 TIMESTAMP \'%s\' - INTERVAL \'1 minute\'\
 AND TIMESTAMP \'%s\' + INTERVAL \'30 seconds\' ORDER BY dateTimeStart"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static inline
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

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_MATCH_STATE( PGconn *conn, bool be_verbose,
			   const struct mph_envi *mph, unsigned short numState,
			   const struct lads_scia *lads,
			   const struct sqads1_scia *sqads,
			   const struct state1_scia *state )
{
     register unsigned short ni;
     register int nr;

     char date_str1[UTC_STRING_LENGTH], date_str2[UTC_STRING_LENGTH];

     char sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];
     char *sql_long_query;

     char           *pntr;
     unsigned short numMatch;
     int            i_indx, i_state, i_date, i_musec, i_softv;
     int            meta_id, nrow, numRows;
     size_t         nc, numChar, sql_long_sz;
     unsigned int   musec;
     double         delayedBy;

     PGresult *res;

     const double SecPerDay = 24. * 60 * 60;

     struct coord_envi    corners[NUM_CORNERS];
     struct stateinfo_rec *stateRow = NULL;
/*
 * get all potential matching states from table "stateinfo"
 */
     if ( numState == 0 ) return;
     MJD_2_DATETIME( state->mjd.days, state->mjd.secnd, 0, date_str1 );
     MJD_2_DATETIME( state[numState-1].mjd.days, state[numState-1].mjd.secnd, 
		     0, date_str2 );
     numChar = snprintf( sql_query, SQL_STR_SIZE, SELECT_FROM_STATEINFO,
			 date_str1, date_str2 );
     if ( be_verbose )
	  (void) printf( "%s(): %s [%-zd]\n", __func__, sql_query, numChar );
     if ( numChar >= SQL_STR_SIZE )
          NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
	  NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     if ( (numRows = PQntuples( res )) == 0 ) 
	  NADC_GOTO_ERROR( NADC_ERR_WARN, NO_DMOP_FOUND );
     stateRow = (struct stateinfo_rec *) 
	  malloc( numRows * sizeof(struct stateinfo_rec) );
     if ( stateRow == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "stateRow" );

     i_indx  = PQfnumber( res, "pk_stateinfo" );
     i_state = PQfnumber( res, "stateID" );
     i_date  = PQfnumber( res, "dateTimeStart" );
     i_musec = PQfnumber( res, "muSecStart" );
     i_softv = PQfnumber( res, "softVersion" );
     for ( nr = 0; nr < numRows; nr++ ) {
	  pntr = PQgetvalue( res, nr, i_indx );
	  stateRow[nr].indxDMOP = 
	       (unsigned int) strtoul( pntr, (char **) NULL, 10 );
	  pntr = PQgetvalue( res, nr, i_state );
	  stateRow[nr].stateID  = (unsigned char) atoi( pntr );
	  pntr = PQgetvalue( res, nr, i_date );
	  musec = (unsigned int) strtoul( PQgetvalue( res, nr, i_musec ),
					   (char **) NULL, 10 );
	  stateRow[nr].jday = DATETIME_2_JULIAN( pntr, musec );
	  pntr = PQgetvalue( res, nr, i_softv );
	  (void) nadc_strlcpy( stateRow[nr].softVersion, pntr, 4 );
	  stateRow[nr].indxState = USHRT_MAX;
	  stateRow[nr].dtMatch = 30 / SecPerDay;
     }
     PQclear( res );
/*
 * loop over states from files to identify matching states from DMOP list
 */
     for ( ni = 0; ni < numState; ni++ ) {
	  register double jday  = state[ni].mjd.days 
	       + (state[ni].mjd.secnd + state[ni].mjd.musec / 1e6) / SecPerDay;
	  
	  for ( nr = 0; nr < numRows; nr++ ) {
	       register double dtime = fabs(jday - stateRow[nr].jday);

	       if ( (unsigned char) state[ni].state_id == stateRow[nr].stateID
		    && stateRow[nr].dtMatch > dtime ) {
		    stateRow[nr].indxState  = ni;
		    stateRow[nr].dtMatch    = dtime;
	       }
	  }
     }
/*
 * remove false/double matches
 */
     for ( nr = 1; nr < numRows; nr++ ) {
	  if ( stateRow[nr-1].indxState == stateRow[nr].indxState ) {
	       if ( stateRow[nr-1].dtMatch > stateRow[nr].dtMatch )
		    stateRow[nr-1].indxState = USHRT_MAX;
	       else
		    stateRow[nr].indxState = USHRT_MAX;
	  }
     }
/*
 * obtain pk_meta from table meta__1P
 */
     numChar = snprintf( sql_query, SQL_STR_SIZE,
			 "SELECT pk_meta FROM meta__1P WHERE name=\'%s\'",
			 mph->product );
     if ( be_verbose )
          (void) printf( "%s(): %s [%-zd]\n", __func__, sql_query, numChar );
     if ( numChar >= SQL_STR_SIZE )
          NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
      res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     if ( (nrow = PQntuples( res )) == 0 ) {
          NADC_GOTO_ERROR( NADC_ERR_FATAL, mph->product );
     }
     pntr = PQgetvalue( res, 0, 0 );
     meta_id = (int) strtol( pntr, (char **) NULL, 10 );
     PQclear( res );
/*
 * allocate memory for SQL string (maximum pk_stateinfo is 2 million ~ 7 digits)
 */
     sql_long_sz = (size_t) (42 + (numRows+1) * (7+1) + 4);
     if ( (sql_long_query = (char *) malloc( sql_long_sz )) == NULL ) {
	  free( stateRow );
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "sql_long_query" );
     }
/*
 * insert all matches in table "stateinfo_meta__1P"
 */
     numChar = snprintf( sql_long_query, sql_long_sz, 
			 "INSERT INTO stateinfo_meta__1P VALUES(%-d,\'{",
			 meta_id );
     numMatch = 0;
     delayedBy = 0.;
     for ( nr = 0; nr < numRows; nr++ ) {
	  if ( (ni = stateRow[nr].indxState) == USHRT_MAX ) continue;

/* do not add entry for states without MDS attached */
	  if ( state[ni].flag_mds == (unsigned char) 1 ) continue;

	  nc = numChar;
	  if ( numMatch > 0 ) {
	       numChar += snprintf( sql_long_query+nc, sql_long_sz-nc, 
				    ",%-u", stateRow[nr].indxDMOP );
	  } else {
	       numChar += snprintf( sql_long_query+nc, sql_long_sz-nc, 
				    "%-u", stateRow[nr].indxDMOP );
	  }
	  if ( numChar >= sql_long_sz ) {
	       free( sql_long_query ); free( stateRow );
	       NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_long_query" );
	  }
	  numMatch++;
	  delayedBy += (stateRow[nr].dtMatch *= SecPerDay);
     }
     nc = numChar;
     numChar += snprintf( sql_long_query+nc, sql_long_sz-nc, 
			  "}\',\'%1s\')", mph->proc_stage );
     if ( numChar >= sql_long_sz ) {
	  free( sql_long_query ); free( stateRow );
	  NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_long_query" );
     }
     if ( be_verbose )
	  (void) printf( "%s(): %s [%-zd]\n", __func__, sql_long_query, numChar );
     res = PQexec( conn, sql_long_query );
     free( sql_long_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     PQclear( res );
/*
 * check number of matches found
 */
     if ( numMatch == 0 ) {
	  free( stateRow );
	  NADC_RETURN_ERROR( NADC_ERR_WARN, NO_MATCHES_FOUND );
     }
     delayedBy /= numMatch;
/*
 * update meta__1P (noEntryDMOP, delayedBy)
 */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "UPDATE meta__1P SET noEntryDMOP=%d,",
		      (int)(numState - numMatch) );
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "%s delayedBy=%.3f", strcpy(cbuff,sql_query), 
		      delayedBy );
     numChar = snprintf( sql_query, SQL_STR_SIZE, "%s WHERE pk_meta=%d",
			 strcpy(cbuff,sql_query), meta_id );
     if ( be_verbose )
	  (void) printf( "%s(): %s [%-zd]\n", __func__, sql_query, numChar );
     if ( numChar >= SQL_STR_SIZE )
          NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
	  NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     PQclear( res );
     (void) snprintf( cbuff, SQL_STR_SIZE,
		      "noEntryDMOP=%-d (States:%-hu DMOP:%-d), delayedBy=%-.3f",
		      numState - numMatch, numState, numRows, delayedBy );
     NADC_ERROR( NADC_ERR_NONE, cbuff );
/*
 * Start a transaction block
 */
     res = PQexec( conn, "BEGIN" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     PQclear( res );
/*
 * update stateinfo (softVersion, orbitPhase, tile)
 */
     for ( nr = 0; nr < numRows; nr++ ) {
	  if ( (ni = stateRow[nr].indxState) == USHRT_MAX ) continue;

          /* do not add entry for states without MDS attached */
	  if ( state[ni].flag_mds == (unsigned char) 1 ) continue;

	  /* only update when procStage of product is higher */
	  if ( mph->proc_stage[0] < stateRow[nr].softVersion[1] ) continue;
	  stateRow[nr].softVersion[1] = mph->proc_stage[0];

	  (void) snprintf( sql_query, SQL_STR_SIZE, 
			   "UPDATE stateinfo SET softVersion=\'%s\',", 
			   stateRow[nr].softVersion );
	  (void) snprintf( sql_query, SQL_STR_SIZE, 
			   "%s orbitPhase=%f,", strcpy(cbuff,sql_query),
			   state[ni].orbit_phase );
	  if ( sqads[ni].flag_saa_region == (unsigned char) 1 )
	       (void) snprintf( sql_query, SQL_STR_SIZE, 
				"%s saaFlag=TRUE,", strcpy(cbuff,sql_query) );
	  else
	       (void) snprintf( sql_query, SQL_STR_SIZE, 
				"%s saaFlag=FALSE,", strcpy(cbuff,sql_query) );
	  CorrectLongitudes( lads[ni].corner, corners ); 
	  (void) snprintf( sql_query, SQL_STR_SIZE, 
			   GEO_POLY_FORMAT, strcpy(cbuff,sql_query), "tile=",
			   corners[0].lon / 1e6, corners[0].lat / 1e6,
			   corners[3].lon / 1e6, corners[3].lat / 1e6,
			   corners[2].lon / 1e6, corners[2].lat / 1e6,
			   corners[1].lon / 1e6, corners[1].lat / 1e6,
			   corners[0].lon / 1e6, corners[0].lat / 1e6,
			   4326 );
	  numChar = snprintf( sql_query, SQL_STR_SIZE, 
			      "%s WHERE pk_stateinfo=%u",
			      strcpy(cbuff,sql_query), stateRow[nr].indxDMOP );
	  if ( be_verbose )
	       (void) printf( "%s(): %s [%-zd]\n", __func__, sql_query, numChar );
	  if ( numChar >= SQL_STR_SIZE ) {
	       NADC_ERROR( NADC_ERR_STRLEN, "sql_query" );
	       PQclear( res );
	       res = PQexec( conn, "ROLLBACK" );
	       if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
		    NADC_ERROR( NADC_ERR_SQL,
				PQresultErrorMessage(res) );
	       goto done;
	  }
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
     }
/*
 * end the transaction
 */
     res = PQexec( conn, "COMMIT" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
 done:
     PQclear( res );
     if ( stateRow != NULL ) free( stateRow );
}
