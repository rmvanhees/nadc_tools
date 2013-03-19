/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2008 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_TOGOMI_DEL_ENTRY
.AUTHOR      R.M. van Hees
.KEYWORDS    TOGOMI SQL
.LANGUAGE    ANSI C
.PURPOSE     remove entries in database for a Togomi product
.INPUT/OUTPUT
  call as   NADC_TOGOMI_DEL_ENTRY( conn, prodName );
     input:  
             PGconn *conn        :  PolstgreSQL connection handle
	     char *prodName      :  filename of the Togomi product

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    Removes entries related to file "prodName" from tables:
             meta_togomi, tile_togomi, tile_meta_togomi
.ENVIRONment None
.VERSION     1.1     12-Aug-2009   bugfix now using cascade correctly, RvH
             1.0     07-Oct-2008   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>

#include <libpq-fe.h>

/*+++++ Local Headers +++++*/
#include <nadc_togomi.h>

/*+++++ Macros +++++*/
#define SQL_STR_SIZE   256

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void NADC_TOGOMI_DEL_ENTRY( PGconn *conn, const char *prodName )
{
     const char prognm[] = "NADC_TOGOMI_DEL_ENTRY";

     char sql_query[SQL_STR_SIZE];

     PGresult *res;
/*
 * remove rows from "tile_togomi"
 */
     (void) snprintf( sql_query, SQL_STR_SIZE,
		      "delete from tile_togomi where julianDay in "\
		      "(select julianDay from tile_meta_togomi where "\
		      "fk_meta = (select pk_meta from meta_togomi where "\
		      "name=\'%s\'))", prodName );
/*      (void) fprintf( stderr, "%s\n", sql_query ); */
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
	  NADC_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
     PQclear( res );
/*
 * remove entry from table "meta_togomi" and "tile_meta_togomi" (by cascade)
 */
     (void) snprintf( sql_query, SQL_STR_SIZE,
		      "delete from meta_togomi where name=\'%s\'", prodName );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
	  NADC_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
     PQclear( res );
}
