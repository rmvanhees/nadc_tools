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

.IDENTifer   SCIA_IMAP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA IMAP CH4
.LANGUAGE    ANSI C
.PURPOSE     store parameters of a SCIA IMAP product in a PostgreSQL database
.INPUT/OUTPUT
  call as   
            scia_imap_ch4 [options] <input-file>

.RETURNS     non-negative on success, negative on failure
.COMMENTS    Input product must be in ADAGUC format
.ENVIRONment None
.VERSION     1.1     19-Oct-2009   require ADAGUC format, RvH
             1.0     18-Nov-2008   initial release by R. M. van Hees
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

#include <hdf5.h>
#include <netcdf.h>

#ifdef _WITH_SQL
#include <libpq-fe.h>
#endif

/*+++++ Local Headers +++++*/
#define __IMAP_CH4_PRODUCT
#include <nadc_imap.h>

/*+++++ Macros +++++*/
#define NADC_PARAMS "\n\t" \
  "Usage: scia_imap_ch4 [-replace] [-remove] <flname>"

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include <_connect_nadc_db.inc>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int main( int argc, char *argv[] )
{
     const char prognm[] = "scia_imap_ch4";

#ifdef _WITH_SQL
     PGconn *conn = NULL;
#endif
     
     bool flag_remove  = FALSE;
     bool flag_replace = FALSE;

     char flname[MAX_STRING_LENGTH];
     char *cpntr, ctemp[SHORT_STRING_LENGTH];

     int ncid;
     int retval;

     struct imap_hdr hdr;
     struct imap_rec *rec = NULL;
/*
 * check command-line parameters
 */
     if ( argc > 2 ) {
	  register int nr = 1;
	  do {
	       if ( strncmp( argv[nr], "-remove", 7 ) == 0 )
		    flag_remove = TRUE;
	       else if ( strncmp( argv[nr], "-replace", 8 ) == 0 )
		    flag_replace = TRUE;
	       else
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, NADC_PARAMS );
	  } while( ++nr < (argc-1) );
	  (void) nadc_strlcpy( flname, argv[argc-1], MAX_STRING_LENGTH );
     } else if ( argc == 2 ) {
	  (void) nadc_strlcpy( flname, argv[1], MAX_STRING_LENGTH );
     } else
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, NADC_PARAMS );
/*
 * strip path of file-name
 */
     if ( (cpntr = strrchr( flname, '/' )) != NULL ) {
          (void) nadc_strlcpy( ctemp, ++cpntr, SHORT_STRING_LENGTH );
     } else {
          (void) nadc_strlcpy( ctemp, flname, SHORT_STRING_LENGTH );
     }
/*
 * read the IMAP CH4 product
 */
     if ( (retval = nc_open( flname, NC_NOWRITE, &ncid )) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     SCIA_RD_NC_CH4_META( ncid, &hdr );
     if ( hdr.numProd > 1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
			   "no more than one orbit per ingest" );
     (void) nadc_strlcpy( hdr.product, ctemp, SHORT_STRING_LENGTH );
     NADC_RECEIVEDATE( flname, hdr.receive_date );
     hdr.file_size = nadc_file_size( flname );
     hdr.numRec = SCIA_RD_NC_CH4_REC( ncid, &rec );
     if ( (retval = nc_close( ncid )) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, flname );
/*
 * connect to PostgreSQL database
 */
#ifdef _WITH_SQL
     CONNECT_NADC_DB( &conn, "scia" );
     if ( IS_ERR_STAT_FATAL ) {
	  NADC_ERROR( prognm, NADC_ERR_SQL, "IMAP (connect)" );
	  NADC_Err_Trace( stderr );
	  return NADC_ERR_FATAL;
     }
     if ( flag_remove || flag_replace ) {
	  SCIA_DEL_ENTRY_IMAP_CH4( conn, hdr.product );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, "IMAP (remove)" );
     }
/*
 * write meta-information to database
 */
     if ( ! flag_remove ) {
	  SCIA_WR_SQL_CH4_META( conn, &hdr );
	  if ( IS_ERR_STAT_FATAL ) {
	       NADC_ERROR( prognm, NADC_ERR_SQL, "IMAP (header)" );
	  } else {
	       SCIA_WR_SQL_CH4_TILE( conn, hdr.product, hdr.numRec, rec );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_ERROR( prognm, NADC_ERR_SQL, "IMAP (records)" );
	  }
     }
#endif
 done:
/*
 * close connection to PostgreSQL database
 */
#ifdef _WITH_SQL
     if ( conn != NULL ) PQfinish( conn );
#endif
/*
 * free allocated memory
 */
     if ( rec != NULL ) free( rec );
/*
 * display error messages
 */
     NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL )
          return NADC_ERR_FATAL;
     else
          return NADC_ERR_NONE;
}
