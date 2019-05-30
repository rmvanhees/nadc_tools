/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2007 - 2019 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_WR_SQL_AUX
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA SQL
.LANGUAGE    ANSI C
.PURPOSE     
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_SQL_AUX(conn, mph, num_dsd, dsd);
     input:  
             PGconn *conn          :  PostgreSQL connection handle
	     struct mph_envi *mph  :  structure for MPH record
	     unsigned int num_dsd  :  number of DSDs
	     struct dsd_envi dsd   :  structure for the DSDs

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    Nothing
.ENVIRONment None
.VERSION     2.0     01-Oct-2012   added verbose flag, RvH
             1.0     13-Dec-2007   initial release by R. M. van Hees
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
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++ Macros +++++*/
#define SQL_STR_SIZE   256

#define SELECT_FROM_AUXILIARY \
"SELECT pk_id FROM auxiliary WHERE fileName=\'%s\'"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_SQL_AUX(PGconn *conn, const struct mph_envi *mph,
			  unsigned int num_dsd, const struct dsd_envi *dsd)
{
     register unsigned short ni, nd;

     char sql_query[SQL_STR_SIZE], cbuff[SQL_STR_SIZE];

     bool      found;
     char      *pntr;
     int       nrow, numChar;
     long      aux_id;

     PGresult  *res;

     const bool be_verbose = nadc_get_param_uint8("flag_verbose");

     const unsigned short numAuxNames = 7;
     const char *auxNames[] = {
	  "LEAKAGE_FILE", "PPG_ETALON_FILE", "SPECTRAL_FILE", "SUN_REF_FILE", 
	  "KEY_DATA_FILE", "M_FACTOR_FILE", "INIT_FILE" };
     const char *auxID[] = {
	  "leakageFileID", "ppgEtalonFileID", "spectralFileID", 
	  "sunRefFileID", "keyDataFileID", "mFactorFileID", "initFileID" };
/*
 * loop over all reference, external in-flight calibration data files 
 * and other and auxiliary files used for the generation of this product
 */
     for (ni = 0; ni < numAuxNames; ni++) {
/*
 * get index to data set descriptor
 */
	  for (found = FALSE, nd = 0; nd < num_dsd; nd++) {
	       if (strcmp(dsd[nd].name, auxNames[ni]) == 0) {
		    found = TRUE;
		    break;
	       }
	  }
	  if (! found) continue;
/*
 * check if auxName is already in table "auxiliary"
 */
	  (void) snprintf(sql_query, SQL_STR_SIZE, SELECT_FROM_AUXILIARY,
			   dsd[nd].flname);
	  res = PQexec(conn, sql_query);
	  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
	       NADC_ERROR(NADC_ERR_SQL, PQresultErrorMessage(res));
	       PQclear(res);
	       return;
	  }
/* add new entry to table auxiliary */
	  if ((nrow = PQntuples(res)) == 0) {
	       PQclear(res);
	       res = PQexec(conn, 
			     "SELECT nextval(\'auxiliary_pk_id_seq\')");
	       if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		    NADC_ERROR(NADC_ERR_SQL, PQresultErrorMessage(res));
		    PQclear(res);
		    return;
	       }
	       pntr = PQgetvalue(res, 0, 0);
	       aux_id = strtol(pntr, (char **) NULL, 10);
	       PQclear(res);

	       (void) strcpy(sql_query, "INSERT INTO auxiliary VALUES");
	       (void) snprintf(sql_query, SQL_STR_SIZE, "%s (%ld,",
				strcpy(cbuff,sql_query), aux_id);
	       numChar = snprintf(sql_query, SQL_STR_SIZE, "%s\'%s\')",
				   strcpy(cbuff,sql_query), dsd[nd].flname);
	       if (be_verbose)
		    (void) printf("%s(): %s [%-d]\n", 
				   __func__, sql_query, numChar);
	       res = PQexec(conn, sql_query);
	       if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		    NADC_ERROR(NADC_ERR_SQL, PQresultErrorMessage(res));
		    PQclear(res);
		    return;
	       }
	       PQclear(res);
	  } else {
	       pntr = PQgetvalue(res, 0, 0);
	       aux_id = strtol(pntr, (char **) NULL, 10);
	       PQclear(res);
	  }
/* 
 * update field in table meta__1P
 */
	  (void) strcpy(sql_query, "UPDATE meta__1P SET");
	  (void) snprintf(sql_query, SQL_STR_SIZE, "%s %s=%ld",
			   strcpy(cbuff,sql_query), auxID[ni], aux_id);
	  numChar = snprintf(sql_query, SQL_STR_SIZE, "%s WHERE name=\'%s\'",
			      strcpy(cbuff,sql_query), mph->product);
	  if (be_verbose)
	       (void) printf("%s(): %s [%-d]\n", __func__, sql_query, numChar);
	  res = PQexec(conn, sql_query);
	  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
	       NADC_ERROR(NADC_ERR_SQL, PQresultErrorMessage(res));
	       PQclear(res);
	       return;
	  }
	  PQclear(res);
     }
}
