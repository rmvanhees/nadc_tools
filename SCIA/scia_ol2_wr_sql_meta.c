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

.IDENTifer   SCIA_OL2_WR_SQL_META
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA SQL
.LANGUAGE    ANSI C
.PURPOSE     write meta data of Sciamachy Lv2 offline to table "meta__2P"
.INPUT/OUTPUT
  call as   SCIA_OL2_WR_SQL_META( conn, be_verbose, sciafl, l1b_product, 
                                  mph, sph );
     input:  
             PGconn *conn  :       :   PostgreSQL connection handle
	     bool be_verbose       :  be verbose
	     char   *sciafl        :  name of Sciamachy file
	     char   *l1b_product   :  name of the SCIA L1b product
	     struct mph_envi *mph  :  structure for MPH record
	     struct sph_sci_ol *sph:  structure for SPH record

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     2.1     01-Oct-2012   added verbose flag, RvH
             2.0     18-Jun-2007   port to PostgreSQL by R. M. van Hees
             1.0     29-Jan-2007   initial release by R. M. van Hees
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

#include <libpq-fe.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

/*+++++ Macros +++++*/
#define SQL_STR_SIZE   1024

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include <_scia_check_sql_lv1b_name.inc>

static inline
void NADC_STRIP( const char *str_in, char *str_out )
{
     register size_t ni = strlen( str_in );

     (void) strcpy( str_out, str_in );
     while ( ni-- > 0 )
	  if ( str_in[ni] != ' ' && str_in[ni] != '\t' ) break;
     
     if ( ni != 0 ) str_out[ni+1] = '\0';
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_OL2_WR_SQL_META( PGconn *conn, bool be_verbose, const char *sciafl,
			   const char *l1b_product,
			   const struct mph_envi *mph,
			   const struct sph_sci_ol *sph )
{
     PGresult *res;

     char *pntr;
     char str_quality[6];
     char ctemp[SHORT_STRING_LENGTH];
     char sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];

     bool          first;
     int           nrow, numChar, meta_id;
/*
 * check if product is already in database
 */
     (void) snprintf( sql_query, MAX_STRING_LENGTH, "%s\'%s\'",
		      "SELECT * FROM meta__2P WHERE name=", mph->product );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
          PQclear( res );
          NADC_RETURN_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     if ( (nrow = PQntuples( res )) != 0 ) {
          PQclear( res );
          NADC_RETURN_ERROR( NADC_ERR_SQL_TWICE, mph->product );
     }
     PQclear( res );
/*
 * obtain next value for serial pk_meta
 */
     res = PQexec( conn,
                   "SELECT nextval(\'meta__2p_pk_meta_seq\')" );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     pntr = PQgetvalue( res, 0, 0 );
     meta_id = (int) strtol( pntr, (char **) NULL, 10 );
     PQclear( res );
/*
 * no error and not yet stored, then proceed
 */     
/* pk_meta */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "INSERT INTO meta__2P VALUES (%d,", meta_id );
/* name */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), mph->product );
/* l1b_product */
     (void) nadc_strlcpy( ctemp, l1b_product, ENVI_FILENAME_SIZE );
     SCIA_CHECK_SQL_LV1B_NAME( conn, ctemp );
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), ctemp );
/* fileSize */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%u,",
		      strcpy(cbuff,sql_query), mph->tot_size );
/* receiveDate */
     NADC_RECEIVEDATE( sciafl, ctemp );
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), ctemp );
/* procStage  */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), mph->proc_stage );
/* procCenter */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), mph->proc_center );
/* softVersion */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), mph->soft_version );
/* fittingErrSum */
     nadc_rstrip( str_quality, sph->errorsum );
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s\'%s\',",
		      strcpy(cbuff,sql_query), str_quality );
/* dateTimeStart */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "%sdate_trunc('second', TIMESTAMP \'%s\'),",
		      strcpy(cbuff,sql_query), sph->start_time );
/* muSecStart */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), sph->start_time+21 );
/* dateTimeStop */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "%sdate_trunc('second', TIMESTAMP \'%s\'),",
		      strcpy(cbuff,sql_query), sph->stop_time );
/* muSecStop */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s,",
		      strcpy(cbuff,sql_query), sph->stop_time+21 );
/* absOrbit */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%d,",
		      strcpy(cbuff,sql_query), mph->abs_orbit );
/* relOrbit */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%d,",
		      strcpy(cbuff,sql_query), mph->rel_orbit );
/* numDataSets */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%u,",
		      strcpy(cbuff,sql_query), mph->num_data_sets );
/* naditFittingWindows */
     first = TRUE;
     (void) strcat( sql_query, "\'" );
     if ( strncmp( sph->nadir_win_uv0, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->nadir_win_uv0, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->nadir_win_uv1, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->nadir_win_uv1, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->nadir_win_uv2, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->nadir_win_uv2, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->nadir_win_uv3, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->nadir_win_uv3, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->nadir_win_uv4, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->nadir_win_uv4, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->nadir_win_uv5, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->nadir_win_uv5, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->nadir_win_uv6, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->nadir_win_uv6, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->nadir_win_uv7, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->nadir_win_uv7, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->nadir_win_ir0, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->nadir_win_ir0, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->nadir_win_ir1, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->nadir_win_ir1, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->nadir_win_ir2, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->nadir_win_ir2, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->nadir_win_ir3, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->nadir_win_ir3, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->nadir_win_ir4, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->nadir_win_ir4, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->nadir_win_ir5, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->nadir_win_ir5, ctemp );
	  if ( ! first ) strcat( sql_query, "," );
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     (void) strcat( sql_query, "\'," );
/* limbFittingWindows */
     first = TRUE;
     (void) strcat( sql_query, "\'" );
     if ( strncmp( sph->limb_win_pth, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->limb_win_pth, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->limb_win_uv0, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->limb_win_uv0, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->limb_win_uv1, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->limb_win_uv1, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->limb_win_uv2, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->limb_win_uv2, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->limb_win_uv3, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->limb_win_uv3, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->limb_win_uv4, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->limb_win_uv4, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->limb_win_uv5, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->limb_win_uv5, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->limb_win_uv6, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->limb_win_uv6, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->limb_win_uv7, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->limb_win_uv7, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->limb_win_ir0, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->limb_win_ir0, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->limb_win_ir1, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->limb_win_ir1, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->limb_win_ir2, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->limb_win_ir2, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->limb_win_ir3, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->limb_win_ir3, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->limb_win_ir4, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->limb_win_ir4, ctemp );
	  if ( ! first ) strcat( sql_query, "," );
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     (void) strcat( sql_query, "\'," );
/* occultFittingWindows */
     first = TRUE;
     (void) strcat( sql_query, "\'" );
     if ( strncmp( sph->occl_win_pth, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->occl_win_pth, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->occl_win_uv0, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->occl_win_uv0, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->occl_win_uv1, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->occl_win_uv1, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->occl_win_uv2, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->occl_win_uv2, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->occl_win_uv3, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->occl_win_uv3, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->occl_win_uv4, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->occl_win_uv4, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->occl_win_uv5, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->occl_win_uv5, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->occl_win_uv6, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->occl_win_uv6, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->occl_win_uv7, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->occl_win_uv7, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->occl_win_ir0, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->occl_win_ir0, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->occl_win_ir1, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->occl_win_ir1, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->occl_win_ir2, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->occl_win_ir2, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->occl_win_ir3, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->occl_win_ir3, ctemp );
	  if ( ! first ) strcat( sql_query, "," ); else first = FALSE;
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     if ( strncmp( sph->occl_win_ir4, "EMPTY", 5 ) != 0 ) {
	  NADC_STRIP( sph->occl_win_ir4, ctemp );
	  if ( ! first ) strcat( sql_query, "," );
	  (void) snprintf( sql_query, SQL_STR_SIZE, "%s%s",
			   strcpy(cbuff,sql_query), ctemp );
     }
     (void) strcat( sql_query, "\'," );
/* numNadirFW */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%hu,",
		      strcpy(cbuff,sql_query), sph->no_nadir_win );
/* numLimbFW */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%hu,",
		      strcpy(cbuff,sql_query), sph->no_limb_win );
/* numOccultFW */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%hu,",
		      strcpy(cbuff,sql_query), sph->no_occl_win );
/* noEntryDMOP */
     (void) snprintf( sql_query, SQL_STR_SIZE, "%s%d,",
		      strcpy(cbuff,sql_query), 0 );
/* delayedBy */
     numChar = snprintf( sql_query, SQL_STR_SIZE, "%s%s)",
			 strcpy( cbuff, sql_query ), "NULL" );
     if ( be_verbose )
	  (void) printf( "%s(): %s [%-d]\n", __FUNCTION__, sql_query, numChar );
     if ( numChar >= SQL_STR_SIZE )
	  NADC_RETURN_ERROR( NADC_ERR_STRLEN, "sql_query" );
/*
 * do the actual insert
 */
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
 done:
     PQclear( res );
}
