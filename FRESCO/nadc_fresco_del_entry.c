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

.IDENTifer   NADC_FRESCO_DEL_ENTRY
.AUTHOR      R.M. van Hees
.KEYWORDS    FRESCO SQL
.LANGUAGE    ANSI C
.PURPOSE     remove entries in database for a Fresco product
.INPUT/OUTPUT
  call as   NADC_FRESCO_DEL_ENTRY( conn, prodName );
     input:  
             PGconn *conn        :  PolstgreSQL connection handle
	     char *prodName      :  filename of the Fresco product

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    Removes entries related to file "prodName" from tables:
             meta_fresco, tile_fresco, tile_meta_fresco
.ENVIRONment None
.VERSION     3.1     12-Aug-2009   bugfix now using cascade correctly, RvH
             3.0     20-Mar-2008   rewite, RvH 
             2.0     21-Jun-2007   port to PostgreSQL by R. M. van Hees
             1.0     01-Mar-2007   initial release by R. M. van Hees
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
#include <nadc_fresco.h>

/*+++++ Macros +++++*/
#define SQL_STR_SIZE   256

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void NADC_FRESCO_DEL_GOME_ENTRY( PGconn *conn, const char *prodName )
{
     char sql_query[SQL_STR_SIZE];

     PGresult *res;
/*
 * remove rows from "tile_fresco" 
 */
     (void) snprintf( sql_query, SQL_STR_SIZE,
		      "delete from tile_fresco where julianDay in "\
		      "(select julianDay from tile_meta_fresco where "\
		      "fk_meta = (select pk_meta from meta_fresco where "\
		      "name=\'%s\'))", prodName );
/*      (void) fprintf( stderr, "%s\n", sql_query ); */
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
	  NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     PQclear( res );
/*
 * remove entry from table "meta_fresco" and "tile_meta_fresco" (by cascade)
 */
     (void) snprintf( sql_query, SQL_STR_SIZE,
		      "delete from meta_fresco where name=\'%s\'", prodName );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
	  NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     PQclear( res );
}

void NADC_FRESCO_DEL_SCIA_ENTRY( PGconn *conn, const char *prodName )
{
     char sql_query[SQL_STR_SIZE];

     PGresult *res;
/*
 * remove entry from table "meta_fresco" which will cascade...
 */
     (void) snprintf( sql_query, SQL_STR_SIZE,
		      "delete from meta_fresco where name=\'%s\'", prodName );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
	  NADC_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     PQclear( res );
}
