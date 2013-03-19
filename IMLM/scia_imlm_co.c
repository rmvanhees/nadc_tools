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

.IDENTifer   SCIA_IMLM_CO
.AUTHOR      R.M. van Hees
.KEYWORDS    SRON IMLM Sciamachy
.LANGUAGE    ANSI C
.PURPOSE     store parameters of a IMLM-CO product SCIA in a PostgreSQL database
.INPUT/OUTPUT
  call as   
            scia_imlm_co [options] <input-file>

.RETURNS     non-negative on success, negative on failure
.COMMENTS    None
.ENVIRONment None
.VERSION     1.3     07-Apr-2011   differentiate between CO and H2O code, RvH
             1.2     12-Oct-2009   improved product, fixed several bugs, RvH
             1.1     20-Oct-2008   conform to ADAGUC filename format, RvH
             1.0     18-Mar-2008   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * This code needs the GNU version of basename (not POSIX)
 */
#define  _GNU_SOURCE

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
#include <nadc_imlm.h>

/*+++++ Macros +++++*/
#define NADC_PARAMS "\n\t" \
  "Usage: scia_imlm_co [-replace] [-remove] <flname>"

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include <_connect_nadc_db.inc>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int main( int argc, char *argv[] )
{
     const char prognm[] = "scia_imlm_co";

#ifdef _WITH_SQL
     PGconn *conn = NULL;
#endif
     
     bool flag_remove  = FALSE;
     bool flag_replace = FALSE;

     char flname[MAX_STRING_LENGTH];
     char ctemp[SHORT_STRING_LENGTH];

     int ncid;
     int retval;

     struct imlm_hdr hdr;
     struct imlm_rec *rec = NULL;
/*
 * check command-line parameters
 */
     if ( argc > 2 ) {
	  register int nr = 1;
	  do {
	       if ( strncmp( argv[nr], "-remove", 7 ) == 0 )
		    flag_remove = TRUE;
	       if ( strncmp( argv[nr], "-replace", 8 ) == 0 )
		    flag_replace = TRUE;
	       else
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, NADC_PARAMS );
	  } while( ++nr < (argc-1) );
	  (void) strlcpy( flname, argv[argc-1], MAX_STRING_LENGTH );
     } else if ( argc == 2 ) {
	  (void) strlcpy( flname, argv[1], MAX_STRING_LENGTH );
     } else
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, NADC_PARAMS );
/*
 * read the IMLM-CO product
 */
     if ( (retval = nc_open( flname, NC_NOWRITE, &ncid )) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     SCIA_RD_NC_CO_META( ncid, &hdr );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, "ADAGUC meta data" );
     if ( hdr.numProd > 1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
			   "no more than one orbit per ingest" );
     (void) strlcpy( ctemp, basename( flname ), SHORT_STRING_LENGTH );
     (void) strlcpy( hdr.product, ctemp, SHORT_STRING_LENGTH );
     NADC_RECEIVEDATE( flname, hdr.receive_date );
     hdr.file_size = NADC_FILESIZE( flname );
     hdr.numRec = SCIA_RD_NC_CO_REC( ncid, &rec );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, "ADAGUC data-records" );
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
	  NADC_ERROR( prognm, NADC_ERR_SQL, "IMLM-CO (connect)" );
	  NADC_Err_Trace( stderr );
	  return NADC_ERR_FATAL;
     }
     if ( flag_remove || flag_replace ) {
	  SCIA_DEL_ENTRY_IMLM_CO( conn, hdr.product );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, "IMLM-CO (remove)" );
     }
/*
 * write meta-information to database
 */
     if ( ! flag_remove ) {
	  SCIA_WR_SQL_CO_META( conn, &hdr );
	  if ( IS_ERR_STAT_FATAL ) {
	       NADC_ERROR( prognm, NADC_ERR_SQL, "IMLM-CO (header)" );
	  } else {
	       SCIA_WR_SQL_CO_TILE(conn, hdr.product, hdr.numRec, rec);
	       if ( IS_ERR_STAT_FATAL )
		    NADC_ERROR( prognm, NADC_ERR_SQL, "IMLM-CO (records)" );
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
