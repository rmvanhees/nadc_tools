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

.IDENTifer   NADC_TOGOMI
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME TOGOMI
.LANGUAGE    ANSI C
.PURPOSE     store parameters of a TOGOMI product (GOME) 
             in a PostgreSQL database
.INPUT/OUTPUT
  call as   
            nadc_togomi [options] <input-file>

.RETURNS     non-negative on success, negative on failure
.COMMENTS    None
.ENVIRONment None
.VERSION     1.1     20-Oct-2008   conform to ADAGUC filename format, RvH
             1.0     30-Sep-2008   initial release by R. M. van Hees
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

#ifdef _WITH_NC4
#include <hdf5.h>
#include <netcdf.h>
#endif
#ifdef _WITH_SQL
#include <libpq-fe.h>
#endif

/*+++++ Local Headers +++++*/
#include <nadc_togomi.h>

/*+++++ Macros +++++*/
#define NADC_PARAMS "\n\t" \
  "Usage: nadc_togomi [-sql] [-replace] [-remove] <flname>"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include <_connect_nadc_db.inc>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int main ( int argc, char *argv[] )
{
     register int          na;

#ifdef _WITH_NC4
     bool  flag_sql     = FALSE;
#else
     bool  flag_sql     = TRUE;
#endif
     bool  flag_remove  = FALSE;
     bool  flag_replace = FALSE;

     char  flname[MAX_STRING_LENGTH];
     unsigned int numRec = 0u;

     struct togomi_hdr hdr;
     struct togomi_rec *togomi = NULL;

#ifdef _WITH_SQL
     PGconn *conn;
#endif
/*
 * check command-line parameters
 */
     if ( argc == 1 ) NADC_GOTO_ERROR( NADC_ERR_PARAM, NADC_PARAMS );
     na = 1;
     do {
	  if ( strncmp( argv[na], "-sql", 4 ) == 0 )
	       flag_sql = TRUE;
	  else if ( strncmp( argv[na], "-remove", 7 ) == 0 )
	       flag_remove = TRUE;
	  else if ( strncmp( argv[na], "-replace", 8 ) == 0 )
	       flag_replace = TRUE;
     } while ( ++na < argc-1 );
     (void) nadc_strlcpy( flname, argv[argc-1], MAX_STRING_LENGTH );
/*
 * read records from TOGOMI product
 */
     numRec = NADC_RD_TOGOMI( flname, &hdr, &togomi );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE_RD, flname );
     if ( hdr.numRec == 0 )
	  NADC_GOTO_ERROR( NADC_ERR_WARN, "empty product");
/*
 * connect to PostgreSQL database
 */
     if ( flag_sql ) {
#ifdef _WITH_SQL
	  CONNECT_NADC_DB( &conn, "gome" );
	  if ( IS_ERR_STAT_FATAL ) {
	       NADC_ERROR( NADC_ERR_SQL, "TOGOMI (connect)" );
	       NADC_Err_Trace( stderr );
	       return NADC_ERR_FATAL;
	  }
	  if ( flag_remove || flag_replace ) {
	       NADC_TOGOMI_DEL_ENTRY( conn, hdr.product );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_SQL, "TOGOMI (remove)" );
	  }
/*
 * write meta-information to database
 */
	  if ( ! flag_remove ) {
	       int meta_id = NADC_TOGOMI_WR_SQL_META( conn, &hdr );
	       if ( IS_ERR_STAT_FATAL ) {
		    NADC_ERROR( NADC_ERR_SQL, "TOGOMI (meta)" );
	       } else {
		    NADC_TOGOMI_WR_SQL_TILE( conn, meta_id,
					     numRec, togomi );
		    if ( IS_ERR_STAT_FATAL )
			 NADC_ERROR( NADC_ERR_SQL, "TOGOMI (tiles)" );
	       }
	  }
/*
 * close connection to PostgreSQL database
 */
	  if ( conn != NULL ) PQfinish( conn );
#endif
     } else {
#ifdef _WITH_NC4
          int  ncid;
          int  retval;

	  (void) snprintf( flname, MAX_STRING_LENGTH, ADAGUC_PROD_TEMPLATE, 
			   "CONS", hdr.validity_start, hdr.validity_stop, 1 );

          if ( (retval = nc_create( flname, NC_NETCDF4, &ncid ))!= NC_NOERR )
               NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );

          NADC_TOGOMI_WR_NC_META( ncid, &hdr );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "TOGOMI header" );

          NADC_TOGOMI_WR_NC_REC( ncid, numRec, togomi );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "TOGOMI records" );

          if ( nc_close( ncid ) != NC_NOERR )
               NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
#endif
     }
/*
 * free allocated memory
 */
 done:
     if ( togomi != NULL ) free( togomi );
/*
 * display error messages
 */
     NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL )
          return NADC_ERR_FATAL;
     else
          return NADC_ERR_NONE;
}
