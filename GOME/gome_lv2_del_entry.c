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

.IDENTifer   GOME_LV2_DEL_ENTRY
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME SQL
.LANGUAGE    ANSI C
.PURPOSE     remove entries in database for a GOME level 2 product
.INPUT/OUTPUT
  call as   GOME_LV2_DEL_ENTRY( conn, flname, softVersion );
     input:  
             PGconn *conn        :  PostgreSQL connection handle
	     char *flname        :  filename of the GOME Level 2 product
             char *softVersion   :  S/W version of the GOME Level 2 product

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    Removes entries related to file "flname" from tables:
             meta__2P and tileinfo_meta__2P
.ENVIRONment None
.VERSION     2.1     22-Apr-2008   use cascade statements, RvH
             2.0     20-Mar-2007   port to PostgreSQL by R. M. van Hees
             1.0     07-Mar-2007   initial release by R. M. van Hees
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
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++ Macros +++++*/
#define SQL_STR_SIZE   128

#define SELECT_FROM_META \
"SELECT pk_meta FROM meta__2P WHERE name = \'%s\' and softVersion = \'%s\'"

#define DELETE_FROM_META \
"DELETE FROM meta__2P WHERE pk_meta = %-d"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_LV2_DEL_ENTRY( PGconn *conn, const char *flname,
			 const char *softVersion )
{
     const char prognm[] = "GOME_LV2_DEL_ENTRY";

     char     sql_query[SQL_STR_SIZE];

     char     *cpntr, ctemp[SHORT_STRING_LENGTH];
     int      meta_id, nrow;

     PGresult *res;
/*
 * strip path of file-name & remove extension ".gz"
 */
     if ( (cpntr = strrchr( flname, '/' )) != NULL ) {
          (void) nadc_strlcpy( ctemp, ++cpntr, SHORT_STRING_LENGTH );
     } else {
          (void) nadc_strlcpy( ctemp, flname, SHORT_STRING_LENGTH );
     }
/*
 * check if file is already stored in meta-table
 */
     (void) snprintf( sql_query, SQL_STR_SIZE, SELECT_FROM_META,
		      ctemp, softVersion );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
	  NADC_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
	  return;
     }
     if ( (nrow = PQntuples( res )) == 0 ) {
	  PQclear( res );
	  return;
     }
     cpntr = PQgetvalue( res, 0, 0 );
     meta_id = (int) strtol( cpntr, (char **) NULL, 10 );
     PQclear( res );
/*
 * remove entry from table "meta__2P", everything else should be removed
 * through the cascade statements
 */
     (void) snprintf( sql_query, SQL_STR_SIZE, DELETE_FROM_META, meta_id );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
	  NADC_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
     PQclear( res );
}
