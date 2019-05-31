/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2019 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_NL0
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0
.LANGUAGE    ANSI C
.PURPOSE     read Envisat SCIAMACHY NRT level 0 products, extract subsets, 
             and write in a flexible binary format (HDF5) or dump, in human 
	     readable form, the contents of each PDS data set to a 
	     separate ASCII file
.INPUT/OUTPUT
  call as
            scia_nl0 [options] <input-file>

.RETURNS     non-negative on success, negative on failure
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   21-Mar-2015	new implementation for info-records, RvH
              2.6   08-Oct-2013	[-check] show CRC and Reed-Solomon errors, RvH
              2.5   19-Jun-2009	remove non-archived file from database, RvH
              2.4   20-Jun-2008	removed HDF4 support, RvH
              2.3   16-Jan-2006	adopted new function call to 
                                SCIA_LV0_RD_MDS_INFO, RvH
              2.2   25-Mar-2003	write software version to HDF5 file, RvH
              2.1   19-Feb-2002	made program complied with new libSCIA, RvH
              2.0   01-Nov-2001	moved to new Error handling routines, RvH 
              1.1   05-Sep-2001 compiles without HDF4/5 library, RvH
              1.0   27-Mar-2001 created by R. M. van Hees
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
#ifdef _WITH_SQL
#include <libpq-fe.h>
#endif

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

#ifdef COLOR_TTY
#define RESET    "\033[0m"
#define BOLDRED  "\033[1m\033[31m"
#else
#define RESET    ""
#define BOLDRED  ""
#endif

/*+++++ Global Variables +++++*/
/* 
 * Most routines to read SCIAMACHY data can allocate memory internally
 * However IDL requires the use of their own memory allocation routines
 */
bool Use_Extern_Alloc = FALSE;

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include <_connect_nadc_db.inc>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int main(int argc, char *argv[])
     /*@globals  errno, stderr, stdout, nadc_stat, nadc_err_stack, 
       Use_Extern_Alloc;@*/
     /*@modifies errno, stderr, stdout, nadc_stat, nadc_err_stack@*/
{
     register size_t ns;

     size_t num_state_all = 0;
     size_t num_state = 0;

     unsigned short num;
     unsigned int   num_dsd;

     char  *cpntr;
     FILE  *fd = NULL;
#ifdef _WITH_SQL
     PGconn *conn = NULL;
#endif
     struct mph_envi  mph;
     struct sph0_scia sph;
     struct dsd_envi  *dsd = NULL;
     struct mds0_states *states_all = NULL;
     struct mds0_states *states = NULL;
     struct mds0_aux  *aux;
     struct mds0_det  *det;
     struct mds0_pmd  *pmd;
/*
 * initialization of command-line parameters
 */
     SCIA_SET_PARAM(argc, argv, SCIA_LEVEL_0);
     if (IS_ERR_STAT_FATAL)
	  NADC_GOTO_ERROR(NADC_ERR_PARAM, "SCIA_SET_PARAM");
/*
 * check if we have to display version and exit
 */
     if (nadc_get_param_uint8("flag_version") == PARAM_SET) {
	  SCIA_SHOW_VERSION(stdout, "scia_nl0");
	  exit(EXIT_SUCCESS);
     }
/*
 * dump command-line parameters
 */
     if (nadc_get_param_uint8("flag_show") == PARAM_SET) {
          SCIA_SHOW_PARAM(SCIA_LEVEL_0);
          exit(EXIT_SUCCESS);
     }
/*
 * connect to PostgreSQL database
 */
#ifdef _WITH_SQL
     if (nadc_get_param_uint8("write_sql") == PARAM_SET) {
	  CONNECT_NADC_DB(&conn, "scia");
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_SQL, "PSQL");
	  if (nadc_get_param_uint8("flag_sql_remove") == PARAM_SET 
	      || nadc_get_param_uint8("flag_sql_replace") == PARAM_SET) {
	       cpntr = nadc_get_param_string("infile");
	       SCIA_LV0_DEL_ENTRY(conn, cpntr);
	       free(cpntr);
	       if (IS_ERR_STAT_FATAL)
		    NADC_GOTO_ERROR(NADC_ERR_SQL, "PSQL(remove)");
	  }
	  if (nadc_get_param_uint8("flag_sql_remove") == PARAM_SET)
	       goto done;
     }
#endif
/*
 * open input-file
 */
     cpntr = nadc_get_param_string("infile");
     if ((fd = fopen(cpntr, "r")) == NULL)
	  NADC_GOTO_ERROR(NADC_ERR_FILE, cpntr);
     free(cpntr);
/*
 * create output file
 */
     if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	  SCIA_CRE_H5_FILE(SCIA_LEVEL_0);
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_HDF_CRE, "HDF5 base");
     }
/*
 * -------------------------
 * read Main Product Header
 */
     ENVI_RD_MPH(fd, &mph);
     if (IS_ERR_STAT_FATAL) 
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "MPH");
     cpntr = nadc_get_param_string("infile");
     if (mph.tot_size != nadc_file_size(cpntr))	  
	  NADC_GOTO_ERROR(NADC_ERR_FATAL, "file size check failed");
     free(cpntr);
     if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	  ENVI_WR_ASCII_MPH(&mph);
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "MPH");
     }
     if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	  SCIA_WR_H5_MPH(&mph);
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_HDF_WR, "MPH");
     }
#ifdef _WITH_SQL
     if (nadc_get_param_uint8("write_sql") == PARAM_SET) {
	  cpntr = nadc_get_param_string("infile");
	  SCIA_LV0_WR_SQL_META(conn, cpntr, &mph);
	  free(cpntr);
	  if (IS_ERR_STAT_WARN) goto done;
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_SQL, "SQL_META");
     }
#endif
/*
 * -------------------------
 * read Specific Product Header
 */
     SCIA_LV0_RD_SPH(fd, mph, &sph);
     if (IS_ERR_STAT_FATAL) 
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "SPH");
     if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	  SCIA_LV0_WR_ASCII_SPH(&sph);
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "SPH");
     }
     if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	  SCIA_LV0_WR_H5_SPH(&sph);
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_HDF_WR, "SPH");
     }
/*
 * -------------------------
 * read Data Set Descriptor records
 */
     dsd = (struct dsd_envi *)
	  malloc((mph.num_dsd-1) * sizeof(struct dsd_envi));
     if (dsd == NULL) NADC_GOTO_ERROR(NADC_ERR_ALLOC, "dsd");
     num_dsd = ENVI_RD_DSD(fd, mph, dsd);
     if (IS_ERR_STAT_FATAL) 
	  NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "DSD");
     if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	  ENVI_WR_ASCII_DSD(num_dsd, dsd);
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "DSD");
     }
     if (nadc_get_param_uint8("write_meta") == PARAM_SET)
	  goto done;
/*
 * -------------------------
 * read SCIAMACHY source packets
 *
 * first read MDS info data
 */
     num_state_all = SCIA_LV0_RD_MDS_INFO(fd, num_dsd, dsd, &states_all);
     if (IS_ERR_STAT_FATAL)
	  NADC_GOTO_ERROR(NADC_ERR_FILE_RD, "RD_MDS_INFO");
     if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	  SCIA_LV0_WR_ASCII_INFO(num_state_all, states_all);
	  if (IS_ERR_STAT_FATAL) 
	       NADC_GOTO_ERROR(NADC_ERR_FILE_RD, "WR_MDS_INFO");
     }
     num_state = SCIA_LV0_SELECT_MDS(num_state_all, states_all, &states);
     SCIA_LV0_FREE_MDS_INFO(num_state_all, states_all);

     if (num_state == 0) goto done;

#ifdef _WITH_SQL
     if (nadc_get_param_uint8("write_sql") == PARAM_SET) {
	  struct mds0_sql sqlState[256];

	  for (ns = 0; ns < num_state && ns < 256; ns++) {
	       sqlState[ns].nrAux = 
		    SCIA_LV0_RD_AUX(fd, states[ns].info_aux,
				     states[ns].num_aux, &aux);
	       if (IS_ERR_STAT_FATAL) goto failed;

	       sqlState[ns].nrDet = 
		    SCIA_LV0_RD_DET(fd, states[ns].info_det,
				    states[ns].num_det, &det);
	       if (IS_ERR_STAT_FATAL) goto failed;

	       sqlState[ns].nrPMD = 
		    SCIA_LV0_RD_PMD(fd, states[ns].info_pmd,
				     states[ns].num_pmd, &pmd);
	       if (IS_ERR_STAT_FATAL) goto failed;

	       /* set DateTime and StateID of current state */
	       (void) memcpy(&sqlState[ns].mjd, &states[ns].mjd,
			      sizeof(struct mjd_envi));
	       sqlState[ns].stateID = states[ns].state_id;

	       /* obtain OBM temperature of current state */
	       GET_SCIA_LV0_STATE_OBMtemp(TRUE, sqlState[ns].nrAux, aux,
					   &sqlState[ns].obmTemp);
	       if (IS_ERR_STAT_FATAL) goto failed;

	       /* obtain Detectory array temperature of current state */
	       GET_SCIA_LV0_STATE_DETtemp(sqlState[ns].nrDet, det, 
					   sqlState[ns].chanTemp);
	       if (IS_ERR_STAT_FATAL) goto failed;

	       /* obtain Detectory array temperature of current state */
	       GET_SCIA_LV0_STATE_PMDtemp(sqlState[ns].nrPMD, pmd, 
					   &sqlState[ns].pmdTemp);
	       if (IS_ERR_STAT_FATAL) goto failed;

	       /* release allocated memory */
	       if (sqlState[ns].nrAux > 0) free(aux);
	       SCIA_LV0_FREE_MDS_DET(sqlState[ns].nrDet, det);
	       if (sqlState[ns].nrPMD > 0) free(pmd);
	  }
 failed:
	  SCIA_LV0_MATCH_STATE(conn, &mph, num_state, sqlState);
	  if (IS_ERR_STAT_FATAL)
	       NADC_GOTO_ERROR(NADC_ERR_SQL, "SQL_STATE");
	  goto done;
     }
#endif
/* 
 * process Auxiliary source packets
 */
     if (nadc_get_param_uint8("flag_silent") == PARAM_UNSET)
	  NADC_Info_Proc(stdout, "Auxiliary MDS", num_state);

     for (ns = 0; ns < num_state; ns++) {
	  if (states[ns].num_aux == 0) continue;
	  
	  if (nadc_get_param_uint8("flag_silent") == PARAM_UNSET)
	       NADC_Info_Update(stdout, 3, ns);

	  num = SCIA_LV0_RD_AUX(fd, states[ns].info_aux, states[ns].num_aux,
				 &aux);
	  if (IS_ERR_STAT_FATAL) {
	       NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "MDS_AUX");
	  }
	  
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV0_WR_H5_AUX(ns, num, aux);
	  } else if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
 	       SCIA_LV0_WR_ASCII_AUX(ns, num, aux);
	  }
	  free(aux);
	  if (IS_ERR_STAT_FATAL) {
	       NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "MDS_AUX");
	  }
     }
     if (nadc_get_param_uint8("flag_silent") == PARAM_UNSET)
	  NADC_Info_Finish(stdout, 3, ns);
/* 
 * process Detector source packets
 */
     if (nadc_get_param_uint8("flag_silent") == PARAM_UNSET)
	  NADC_Info_Proc(stdout, "Detector MDS", num_state);
     
     for (ns = 0; ns < num_state; ns++) {
	  if (states[ns].num_det == 0) continue;

	  if (nadc_get_param_uint8("flag_silent") == PARAM_UNSET)
	       NADC_Info_Update(stdout, 3, ns);

	  num = SCIA_LV0_RD_DET(fd, states[ns].info_det,
				states[ns].num_det, &det);
	  if (IS_ERR_STAT_FATAL) {
	       NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "MDS_DET");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV0_WR_H5_DET(ns, num, det);
	  } else if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV0_WR_ASCII_DET(ns, num, det);
	  }
	  SCIA_LV0_FREE_MDS_DET(num, det);
	  if (IS_ERR_STAT_FATAL) {
	       NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "MDS_DET");
	  }
     }
     if (nadc_get_param_uint8("flag_silent") == PARAM_UNSET)
	  NADC_Info_Finish(stdout, 3, ns);
/* 
 * process PMD source packets
 */
     if (nadc_get_param_uint8("flag_silent") == PARAM_UNSET)
	  NADC_Info_Proc(stdout, "PMD MDS", num_state);

     for (ns = 0; ns < num_state; ns++) {
	  if (states[ns].num_pmd == 0) continue;

	  if (nadc_get_param_uint8("flag_silent") == PARAM_UNSET)
	       NADC_Info_Update(stdout, 3, ns);

	  num = SCIA_LV0_RD_PMD(fd, states[ns].info_pmd, states[ns].num_pmd,
				 &pmd);
	  if (IS_ERR_STAT_FATAL) {
	       NADC_GOTO_ERROR(NADC_ERR_PDS_RD, "MDS_PMD");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV0_WR_H5_PMD(ns, num, pmd);
	  } else if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV0_WR_ASCII_PMD(ns, num, pmd);
	  }
	  free(pmd);
	  if (IS_ERR_STAT_FATAL) {
	       NADC_GOTO_ERROR(NADC_ERR_FILE_WR, "MDS_PMD");
	  }
     }
     if (nadc_get_param_uint8("flag_silent") == PARAM_UNSET)
	  NADC_Info_Finish(stdout, 3, ns);
/*
 * when an error has occurred we jump to here:
 */
 done:
/*
 * close input file
 */
     if (fd != NULL) (void) fclose(fd);
/*
 * close connection to PostgreSQL database
 */
#ifdef _WITH_SQL
     if (nadc_get_param_uint8("write_sql") == PARAM_SET && conn != NULL)
	  PQfinish(conn);
#endif
/*
 * close HDF5 output file
 */
     if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	  hid_t fid = nadc_get_param_hid("hdf_file_id");
	  
	  if (fid >= 0 && H5Fclose(fid) < 0) {
	       cpntr = nadc_get_param_string("outfile");
	       NADC_ERROR(NADC_ERR_HDF_FILE, cpntr);
	       free(cpntr);
	  }
     }
/*
 * free allocated memory
 */
     if (dsd != NULL) free(dsd);
     SCIA_LV0_FREE_MDS_INFO(num_state, states);
/*
 * display error messages?
 */
     if (nadc_get_param_uint8("flag_silent") == PARAM_UNSET)
	  NADC_Err_Trace(stderr);
/*
 * clean up space to store command-line parameters
 */
     nadc_free_param_string();
     if (IS_ERR_STAT_FATAL) 
	  return NADC_ERR_FATAL;
     else
	  return NADC_ERR_NONE;
}
