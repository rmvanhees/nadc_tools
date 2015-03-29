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

.IDENTifer   SCIA_LV0_DEL_ENTRY
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA SQL
.LANGUAGE    ANSI C
.PURPOSE     remove entries in database for a Sciamachy level 0 product
.INPUT/OUTPUT
  call as   SCIA_LV0_DEL_ENTRY( conn, be_verbose, flname );
     input:  
             PGconn *conn        :  PostgreSQL connection handle
	     bool be_verbose     :  be verbose
             char *flname        :  filename of the Sciamachy Level 0 product

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    Removes entries related to file "flname" from tables:
             meta__0P, stateinfo_meta__0P and reverse stateinfo
.ENVIRONment None
.VERSION     2.1     04-Oct-2012   added verbose flag, RvH
             2.0     18-Jun-2007   port to PostgreSQL by R. M. van Hees
             1.0     06-Mar-2007   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libpq-fe.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

/*+++++ Macros +++++*/
#define SQL_STR_SIZE   256

#define SELECT_FROM_STATEINFO \
"SELECT unnest(fk_stateinfo) FROM stateinfo_meta__0P WHERE\
 fk_meta=(SELECT pk_meta FROM meta__0P where name=\'%s\')"

#define DELETE_FROM_META \
"DELETE FROM meta__0P WHERE name=\'%s\'"

#define UPDATE_STATEINFO \
"UPDATE stateinfo \
 SET softVersion=\'0\' || SUBSTR(softVersion,2,2) \
 WHERE pk_stateinfo=%u AND SUBSTR(softVersion,1,1)=\'%s\'"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV0_DEL_ENTRY( PGconn *conn, bool be_verbose, const char *flname )
{
     register int nr;

     unsigned int indx;

     char     sql_query[SQL_STR_SIZE];
     char     procStage[2];
     char     *cpntr, sciafl[SHORT_STRING_LENGTH];
     int      nrow, numChar;

     PGresult *res, *res_update;
/*
 * strip path of file-name
 */
     if ( (cpntr = strrchr( flname, '/' )) != NULL ) {
          (void) nadc_strlcpy( sciafl, ++cpntr, SHORT_STRING_LENGTH );
     } else {
          (void) nadc_strlcpy( sciafl, flname, SHORT_STRING_LENGTH );
     }
/*
 * check if we have to reverse field "softVersion[0]" in table "stateinfo"
 */
     numChar = snprintf( sql_query, SQL_STR_SIZE, SELECT_FROM_STATEINFO,
			 sciafl );
     if ( be_verbose )
          (void) printf( "%s(): %s [%-d]\n", __FUNCTION__, sql_query, numChar );
     if ( numChar >= SQL_STR_SIZE )
          NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
	  NADC_RETURN_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
/* return when no entries are found */
     if ( (nrow = PQntuples( res )) == 0 ) goto done;
/*
 * update field "softVersion[0]" in table "stateinfo"
 */
     (void) nadc_strlcpy( procStage, sciafl+10, 2 );
     for ( nr = 0; nr < nrow; nr++ ) {
	  cpntr = PQgetvalue( res, nr, 0 );
          indx = (unsigned int) strtoul( cpntr, (char **)NULL, 10 );
	  numChar = snprintf( sql_query, SQL_STR_SIZE, 
			      UPDATE_STATEINFO, indx, procStage );
	  if ( be_verbose )
	       (void) printf( "%s(): %s [%-d]\n", __FUNCTION__, sql_query, numChar );
	  if ( numChar >= SQL_STR_SIZE )
	       NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
	  res_update = PQexec( conn, sql_query );
	  if ( PQresultStatus( res_update ) != PGRES_COMMAND_OK ) {
	       NADC_ERROR( NADC_ERR_SQL, 
			   PQresultErrorMessage(res_update) );
	       PQclear( res_update );
	       goto done;
	  }
	  PQclear( res_update );
     }
 done:
     PQclear( res );
/*
 * remove entry from table "meta__0P"
 * referenced entries in table "stateinfo_meta__0P" are removed by the engine
 */
     numChar = snprintf( sql_query, SQL_STR_SIZE, DELETE_FROM_META, sciafl );
     if ( be_verbose )
	  (void) printf( "%s(): %s [%-d]\n", __FUNCTION__, sql_query, numChar );
     if ( numChar >= SQL_STR_SIZE )
	  NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     PQclear( res );
}
