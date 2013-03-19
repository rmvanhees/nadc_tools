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

.IDENTifer   SCIA_DEL_ENTRY_IMAP_HDO
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA IMAP HDO SQL
.LANGUAGE    ANSI C
.PURPOSE     remove entries in database for a SCIA SRON IMAP product
.INPUT/OUTPUT
  call as   SCIA_DEL_ENTRY_IMAP_HDO( conn, prodName );
     input:  
             PGconn *conn        :  PolstgreSQL connection handle
	     char *prodName      :  filename of the SCIA IMAP product

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    Removes entries related to file "prodName" from tables:
             meta_imap, tile_imap
.ENVIRONment None
.VERSION     1.0     28-Apr-2011   initial release by R. M. van Hees
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
#define __IMAP_HDO_PRODUCT
#include <nadc_imap.h>

/*+++++ Macros +++++*/
#define SQL_STR_SIZE   128

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_DEL_ENTRY_IMAP_HDO( PGconn *conn, const char *prodName )
{
     const char prognm[] = "SCIA_DEL_ENTRY_IMAP_HDO";

     char sql_query[SQL_STR_SIZE];

     PGresult *res;
/*
 * remove entry from table "meta_imap" which will cascade...
 */
     (void) snprintf( sql_query, SQL_STR_SIZE,
		      "delete from meta_imap_hdo where name=\'%s\'", prodName );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
	  NADC_ERROR( prognm, NADC_ERR_SQL, PQresultErrorMessage(res) );
     PQclear( res );
}
