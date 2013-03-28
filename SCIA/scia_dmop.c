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

.IDENTifer   SCIA_DMOP
.AUTHOR      R.M. van Hees
.KEYWORDS    DMOP Sciamachy
.LANGUAGE    ANSI C
.PURPOSE     store parameters of a DMOP product SCIA in a PostgreSQL database
.INPUT/OUTPUT
  call as   
            scia_dmop [options] <input-file>

.RETURNS     non-negative on success, negative on failure
.COMMENTS    None
.ENVIRONment None
.VERSION     1.1     06-Nov-2007   fixed help on command-line parameters, RvH
             1.0     01-Jun-2007   initial release by R. M. van Hees
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

#ifdef _WITH_SQL
#include <libpq-fe.h>
#endif

/*+++++ Local Headers +++++*/
#define _SCIA_COMMON
#include <nadc_scia.h>

/*+++++ Macros +++++*/
#define NADC_PARAMS " [-replace|-remove|-debug] <flname>"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include <_connect_nadc_db.inc>

static inline
double GET_JulianDay( const char *sciaDate, const char *sciaTime )
{
     char dateTime[UTC_STRING_LENGTH];
     int  mjd2000;
     unsigned int seconds, mu_sec;

     (void) snprintf( dateTime, UTC_STRING_LENGTH, 
		      "%s %s", sciaDate, sciaTime );
     ASCII_2_MJD( dateTime, &mjd2000, &seconds, &mu_sec );

     return (double) mjd2000 + (seconds + (mu_sec / 1e6)) / (24. * 3600);
}

static inline
void Adjust_JulianDay( const double jday_anx, 
		       char *sciaDate, const char *sciaTime )
{
     const char prognm[] = "Adjust_JulianDay";

     char dateTime[UTC_STRING_LENGTH];
     int  mjd2000;
     unsigned int seconds, mu_sec;

     double jday = GET_JulianDay( sciaDate, sciaTime );

     if ( jday < jday_anx-1 || jday > jday_anx+2 ) {
	  NADC_ERROR( prognm, NADC_ERR_NONE, "wrong Julian Day" );
     } else if ( jday < jday_anx ) {
	  (void) snprintf( dateTime, UTC_STRING_LENGTH, 
			   "%s %s", sciaDate, sciaTime );
	  ASCII_2_MJD( dateTime, &mjd2000, &seconds, &mu_sec );

	  MJD_2_ASCII( mjd2000+1, seconds, mu_sec, dateTime );

	  (void) strlcpy( sciaDate, dateTime, 12 );

	  NADC_ERROR( prognm, NADC_ERR_NONE, "adjust Julian Day(+1)");
     } else if ( jday > jday_anx+1 ) {
	  (void) snprintf( dateTime, UTC_STRING_LENGTH, 
			   "%s %s", sciaDate, sciaTime );
	  ASCII_2_MJD( dateTime, &mjd2000, &seconds, &mu_sec );

	  MJD_2_ASCII( mjd2000-1, seconds, mu_sec, dateTime );

	  (void) strlcpy( sciaDate, dateTime, 12 );

	  NADC_ERROR( prognm, NADC_ERR_NONE, "adjust Julian Day(-1)");
     }
}

/*
 * Processing the Header:
 * - use NUM_STATES to check number of read records
 * - use ANX_TIME to check time read from record (transform to MJD)
 *
 * Processing the Records:
 * - time-line is not always defined. Check length of line!
 * - transform start and stop time to MJD and check consistency
 *   + check & correct (with +/- 1 day): 
 *     dateTime(start/stop) should be larger that ANX_TIME
 * - use ABS_START_ORBIT to correct absOrbit
 * - use elapse-time to calculate first estime of orbitPhase
 *   + elapse time can be out-of-range due to errors in dateTime (start/stop)
 *
 * Obtained from Records:
 * - string         dateTimeStart
 * - unsigned int   muSecStart
 * - string         dateTimeStop
 * - unsigned int   muSecStop
 * - string         timeLine       [default "xx xxxx"]
 * - double         orbitPhase     [derived from elapseTime]
 * - unsigned char  stateID
 * - unsigned short absOrbit
 */
static
unsigned int NADC_RD_DMOP( FILE *fp, /*@out@*/ struct dmop_rec **dmop )
{
     char prognm[] = "NADC_RD_DMOP";

     char           line[256];
     char           elapse[9];
     char           startDate[12], startTime[16];
     char           stopDate[12], stopTime[16];
     char           keyword[24], keyval[80];
     unsigned char  stateID;
     unsigned short orbit;
     int            num;

     char tl1[] = "xx";
     char tl2[] = "xxxx";
     bool use_fmt_short = FALSE;

     unsigned short startOrbit = 0;
     unsigned int   numRec = 0u;
     unsigned int   maxRec = 0u;
     double         jday_anx = 0.;

     const double orbitPeriod = 6035.93;

     do {
	  if ( fgets( line, 256, fp ) == NULL ) goto done;
	  if ( strncmp( line, "ENDFILE", 7 ) == 0 ) goto done;

	  switch( strlen(line) ) {
	  case 178: {
	       const char fmt[] = 
		    " %2s %4s %hhu %*s %*s %hu %s %s %s %*s %*s %*s %s %s";
	       const char fmt_short[] = 
		    " %hhu %*s %*s %hu %s %s %s %*s %*s %*s %s %s";
	       const char fmt_pre[] = 
		    " %2s %4s %hhu %*s %*s %*s %hu %s %s %s %*s %*s %*s %s %s";
	       const char fmt_short_pre[] = 
		    " %hhu %*s %*s %*s %hu %s %s %s %*s %*s %*s %s %s";

	       if ( use_fmt_short ) {
		    num = sscanf( line, fmt_short, &stateID, &orbit,
				  elapse, startDate, startTime, stopDate, 
				  stopTime );
		    if ( num != 7 )
			 num = sscanf( line, fmt_short_pre, &stateID,
				       &orbit, elapse, startDate, startTime, 
				       stopDate, stopTime );
	       } else {
		    num = sscanf( line, fmt, tl1, tl2, &stateID, &orbit, 
				  elapse, startDate, startTime, stopDate, 
				  stopTime );
		    if ( num != 9 )
			 num = sscanf( line, fmt_pre, tl1, tl2, &stateID, 
				       &orbit, elapse, startDate, startTime, 
				       stopDate, stopTime );
	       }
	       if ( numRec < maxRec && (num == 9 || num == 7) ) {
		    (void) snprintf( (*dmop)[numRec].timeLine, 8, 
				    "%s %s", tl1, tl2 );

		    Adjust_JulianDay( jday_anx, startDate, startTime );
		    (*dmop)[numRec].muSecStart = (unsigned int) 
			 strtoul( startTime+9, (char **)NULL, 10 );
		    (void) snprintf( (*dmop)[numRec].dateTimeStart, 21,
				     "%s %s", startDate, startTime );

		    Adjust_JulianDay( jday_anx, stopDate, stopTime );
		    (*dmop)[numRec].muSecStop = (unsigned int) 
			 strtoul( stopTime+9, (char **)NULL, 10 );
		    (void) snprintf( (*dmop)[numRec].dateTimeStop, 21,
				     "%s %s", stopDate, stopTime );

		    if ( strncmp( elapse, "********", 8 ) == 0 ) {
			 double jday = GET_JulianDay( startDate, startTime );

			 (*dmop)[numRec].orbitPhase = (float)
			      (24. * 3600 * (jday - jday_anx)) / orbitPeriod;
		    } else {
			 (*dmop)[numRec].orbitPhase = 
			      strtof( elapse, (char **)NULL ) / orbitPeriod;
		    }
                    if ( (*dmop)[numRec].orbitPhase > 1.f 
			 && (orbit == startOrbit || orbit > startOrbit + 1) ) {
			 orbit = startOrbit + 1;
			 (*dmop)[numRec].orbitPhase -= 1.f;
			 NADC_ERROR( prognm,NADC_ERR_NONE,"adjust absOrbit" );
		    } else if ( orbit < startOrbit || orbit > startOrbit+1 )
                         NADC_ERROR( prognm,NADC_ERR_NONE,"wrong absOrbit" );
		    (*dmop)[numRec].stateID = stateID;
		    (*dmop)[numRec].absOrbit = orbit;
		    numRec++;
	       } else
		    NADC_ERROR( prognm,NADC_ERR_NONE,"incomplete record" );
	       break;
	  } case 113: {
	       if ( strncmp( line, " STATE_ID", 9 ) == 0 )
		    use_fmt_short = TRUE;
	  } case 54: {
	       const char fmt[] = "%s = %s %s";

	       (void) sscanf( line, fmt, keyword, startDate, startTime );
	       if ( strncmp( keyword, "ANX_TIME", 9 ) == 0 ) {
		    if ( *startDate == '+' )
			 jday_anx = GET_JulianDay( startDate+1, startTime );
		    else
			 jday_anx = GET_JulianDay( startDate, startTime );
	       }
	       break;
	  } default: {
	       const char fmt[] = "%s = %s";

	       if ( strncmp( line+1, "-----", 5 ) == 0 ) break;

	       (void) sscanf( line, fmt, keyword, keyval );
	       if ( strncmp( keyword, "NUM_STATES", 10 ) == 0 ) {
		    maxRec = (unsigned int) 
			 strtoul( keyval, (char **)NULL, 10 );
		    *dmop = (struct dmop_rec *)
			 malloc( maxRec * sizeof( struct dmop_rec ));
		    if ( *dmop == NULL )
			 NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "dmop" );
	       } else if ( strncmp( keyword, "ABS_START_ORBIT", 15 ) == 0 ) {
		    startOrbit = (unsigned short) 
			 strtoul( keyval, (char **)NULL, 10 );
	       }
	       break;
	  }};
     } while( 1 == 1 );
 done:
     if ( numRec > 0 && numRec < maxRec ) {
	  NADC_ERROR( prognm, NADC_ERR_NONE, 
		      "not all State_summary records read" );
     }
     return numRec;
}

#ifdef _WITH_SQL
/*+++++ Macros +++++*/
#define DELETE_FROM_STATEINFO \
"DELETE FROM stateinfo WHERE dateTimeStart >=\'%s\' AND dateTimeStart <=\'%s\'"

#define INSERT_TO_STATEINFO \
"INSERT INTO stateinfo (dateTimeStart,muSecStart,dateTimeStop,muSecStop,\
timeLine,stateID,absOrbit,orbitPhase,tile) VALUES"

#define TILE_DEFAULT \
"ST_GeomFromText(\'POLYGON((0 0, 0 0, 0 0, 0 0, 0 0))\',4326)"

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_DEL_SQL_DMOP
.PURPOSE     remove entries in database for a Sciamachy DMOP product (SOST)
.INPUT/OUTPUT
  call as   SCIA_DEL_SQL_DMOP( conn, numRec, dmop )
     input:  
             PGconn *conn        :  PostgreSQL connection handle
	     unsigned int numRec :   number of DMOP records
	     struct dmop_rec *dmop : records with DMOP data	     

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    Removes entries from tables: stateinfo
-------------------------*/
static
void SCIA_DEL_SQL_DMOP( PGconn *conn, unsigned int numRec, 
			const struct dmop_rec *dmop )
{
     const char prognm[] = "SCIA_DEL_SQL_DMOP";

     const size_t SQL_STR_SIZE = 128;

     char sql_query[SQL_STR_SIZE];

     PGresult *res_del;
/*
 * remove entries from table "stateinfo"
 */
     (void) snprintf( sql_query, SQL_STR_SIZE, DELETE_FROM_STATEINFO,
		      dmop[0].dateTimeStart, dmop[numRec-1].dateTimeStop );
/*    (void) fprintf( stderr, "%s\n", sql_query ); */
     res_del = PQexec( conn, sql_query );
     if ( PQresultStatus( res_del ) != PGRES_COMMAND_OK )
	  NADC_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res_del) );
     PQclear( res_del );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_WR_SQL_DMOP
.PURPOSE     write Sciamachy DMOP information to PostgreSQL database
.INPUT/OUTPUT
  call as   SCIA_WR_SQL_DMOP( conn, numRec, dmop );
     input:
             PGconn *conn  :         PostgreSQL connection handle
	     unsigned int numRec :   number of DMOP records
	     struct dmop_rec *dmop : records with DMOP data

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
static
void SCIA_WR_SQL_DMOP( PGconn *conn, unsigned int numRec, 
		       const struct dmop_rec *dmop )
{
     const char prognm[] = "SCIA_WR_SQL_DMOP";

     const size_t SQL_STR_SIZE = 512;

     register unsigned int nr;

     char     sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];

     size_t   numChar;

     PGresult *res;
/*
 * Start a transaction block
 */
     res = PQexec( conn, "BEGIN" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	  NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     PQclear( res );
/*
 * create insert-query
 */
     for ( nr = 0; nr < numRec; nr++ ) {
	  (void) strcpy( sql_query, INSERT_TO_STATEINFO );
/* dateTimeStart */
	  (void) snprintf( sql_query, SQL_STR_SIZE,
			   "%s (\'%s\',", strcpy(cbuff,sql_query), 
			   dmop[nr].dateTimeStart );
/* muSecStart */
	  (void) snprintf( sql_query, SQL_STR_SIZE,
			   "%s%6u,", strcpy(cbuff,sql_query),
			   dmop[nr].muSecStart );
/* dateTimeStop */
	  (void) snprintf( sql_query, SQL_STR_SIZE,
			   "%s\'%s\',", strcpy(cbuff,sql_query),
			   dmop[nr].dateTimeStop );
/* muSecStop */
	  (void) snprintf( sql_query, SQL_STR_SIZE,
			   "%s%6u,", strcpy(cbuff,sql_query), 
			   dmop[nr].muSecStop );
/* timeLine */
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
			   strcpy(cbuff,sql_query), dmop[nr].timeLine );
/* stateID */
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%hhu,",
			   strcpy(cbuff,sql_query), dmop[nr].stateID );
/* absOrbit */
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%hu,",
			   strcpy(cbuff,sql_query), dmop[nr].absOrbit );
/* orbitPhase */
          (void) snprintf( sql_query, SQL_STR_SIZE, "%s %.5f,",
			   strcpy(cbuff,sql_query), dmop[nr].orbitPhase );
/* tile */
	  numChar = snprintf( sql_query, SQL_STR_SIZE, "%s%s)",
			      strcpy(cbuff,sql_query), TILE_DEFAULT );
/* 	  (void) fprintf( stderr, "%s [%-zd]\n", sql_query, numChar ); */
	  if ( numChar >= SQL_STR_SIZE ) {
	       res = PQexec( conn, "ROLLBACK" );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_STRLEN,  "sql_query" );
	  }
/*
 * do the actual insert
 */
	  res = PQexec( conn, sql_query );
	  if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	       NADC_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
	       PQclear( res );
	       res = PQexec( conn, "ROLLBACK" );
	       if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
		    NADC_ERROR(prognm,NADC_ERR_SQL,PQresultErrorMessage(res));
	       goto done;
	  }
	  PQclear( res );
     }
/*
 * end the transaction
 */
     res = PQexec( conn, "COMMIT" );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	  NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
 done:
     PQclear( res );
}
#endif /* _WITH_SQL */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int main( int argc, char *argv[] )
{
     const char prognm[] = "scia_dmop";

     FILE *fp = NULL;

     bool flag_debug   = FALSE;
     bool flag_remove  = FALSE;
     bool flag_replace = FALSE;

     char         flname[MAX_STRING_LENGTH];
     unsigned int nr, numRec = 0u;

     struct dmop_rec *dmop = NULL;
#ifdef _WITH_SQL
     PGconn *conn;
#endif
/*
 * check command-line parameters
 */
     if ( argc == 3 ) {
	  if ( strncmp( argv[1], "-remove", 7 ) == 0 )
	       flag_remove = TRUE;
	  else if ( strncmp( argv[1], "-replace", 8 ) == 0 )
	       flag_replace = TRUE;
	  else if ( strncmp( argv[1], "-debug", 6 ) == 0 )
	       flag_debug = TRUE;
	  else
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, NADC_PARAMS );

	  (void) strlcpy( flname, argv[2], MAX_STRING_LENGTH );
     } else if ( argc == 2 ) {
	  (void) strlcpy( flname, argv[1], MAX_STRING_LENGTH );
     } else
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, NADC_PARAMS );
/*
 * read records from DMOP product
 */
     if ( (fp = fopen( flname, "r" )) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, flname );
     numRec = NADC_RD_DMOP( fp, &dmop );
     if ( numRec == 0 || dmop == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_NONE, "State_summary is empty" );
     if ( flag_debug ) {
	  for ( nr = 0; nr < numRec; nr++ ) {
	       (void) printf( "[%03u] %s %s %s %6u %6u %10.6f %3hhu %5hu\n",
			      nr, dmop[nr].timeLine, dmop[nr].dateTimeStart,
			      dmop[nr].dateTimeStop, dmop[nr].muSecStart,
			      dmop[nr].muSecStop, dmop[nr].orbitPhase,
			      dmop[nr].stateID, dmop[nr].absOrbit );
	  }
	  goto done;
     }
/*
 * connect to PostgreSQL database
 */
#ifdef _WITH_SQL
     CONNECT_NADC_DB( &conn, "scia" );
     if ( IS_ERR_STAT_FATAL ) {
	  NADC_ERROR( prognm, NADC_ERR_SQL, "PSQL" );
	  NADC_Err_Trace( stderr );
	  return NADC_ERR_FATAL;
     }
     if ( flag_remove || flag_replace ) {
	  SCIA_DEL_SQL_DMOP( conn, numRec, dmop );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_ERROR( prognm, NADC_ERR_SQL, "DMOP (remove)" );
     }
/*
 * write meta-information to database
 */
     if ( ! flag_remove ) {
	  SCIA_WR_SQL_DMOP( conn, numRec, dmop );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_ERROR( prognm, NADC_ERR_SQL, "DMOP (stateinfo)" );
     }
/*
 * close connection to PostgreSQL database
 */
     PQfinish( conn );
#endif
/*
 * close input file
 */
 done:
     if ( fp != NULL ) fclose( fp );
/*
 * free allocated memory
 */
     if ( dmop != NULL ) free( dmop );
/*
 * display error messages
 */
     NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL )
          return NADC_ERR_FATAL;
     else
          return NADC_ERR_NONE;
}
