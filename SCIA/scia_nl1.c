/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2000 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_NL1
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1b/1c (NRT)
.LANGUAGE    ANSI C
.PURPOSE     Read Envisat Sciamachy NRT level 1b/1c products, extract subsets,
             optionally calibrate the science data, and write in a flexible 
             binary format (HDF5) or dump, in human readable form, 
	     the contents of each PDS data set to a separate ASCII file
.INPUT/OUTPUT
  call as
            scia_nl1 [options] <input-file>

.RETURNS     non-negative on success, negative on failure
.COMMENTS    None
.ENVIRONment None
.VERSION      5.3   19-Jun-2009	remove non-archived file from database, RvH
              5.2   20-Jun-2008	removed HDF4 support, RvH
              5.1   01-Jun-2006	bugfix PROCESS_LV1C_MDS, RvH
              5.0.1 22-Dec-2005	bugfix file open/close, RvH
              5.0   11-Oct-2005	add support for PDS output,
                                combined write (PDS,HDF4,HDF5,ASCII) routines 
                                to one routine per dataset, RvH 
              4.12  23-Dec-2003	debugged MDS selection (L1B), RvH
              4.11  25-Mar-2003	write software version to HDF4 file, RvH
              4.10  25-Mar-2003	write software version to HDF5 file, RvH
              4.9   20-Mar-2003 changes to parameter-list of level 1c 
                                write routines, RvH
              4.8   04-Mar-2003 didn't read Occultation RSP & PSP, RvH
              4.7   18-Sep-2002 use external function to test 1b/1c, RvH
              4.6   09-Aug-2002	do not attempt to read/write level 1c 
			        PMD/polV for monitoring states, RvH
              4.5   02-Aug-2002	added reading of DSD CAL_OPTIONS, RvH 
              4.4   02-Aug-2002	added cluster selection, RvH 
              4.3   25-Jul-2002	test level 1c in separate function, RvH 
              4.2   05-Apr-2002	added routines to handle level 1c, RvH 
              4.1   08-Mar-2002	modified call to SCIA_LV1_SELECT, RvH 
              4.0   13-Nov-2001	moved to the new Error interface, RvH 
              3.2   06-Sep-2001 selection of states, RvH
              3.1   05-Sep-2001 compiles without HDF4/5 library, RvH
              3.0   08-Jun-2001 added HDF4 support, RvH
              2.1   17-Apr-2001 use NADC_HDF5_NADC_BASE, RvH
              2.0   13-Jan-2001 updated to SCIA_L01 01.00 format, RvH
              1.0   12-Aug-1999 created by R. M. van Hees
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
#define _SCIA_LEVEL_1
#include <nadc_scia_cal.h>

/*+++++ Global Variables +++++*/
/* 
 * Most routines to read SCIAMACHY data can allocate memory internally
 * However IDL requires the use of their own memory allocation routines
 */
bool Use_Extern_Alloc = FALSE;

#ifdef _WITH_SQL
     static PGconn *conn = NULL;
#endif
static FILE *fp_out = NULL;

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include <_connect_nadc_db.inc>
#include "scia_lv1_wr_calls.inc"

/*+++++++++++++++++++++++++
.IDENTifer   PROCESS_LV1B_MDS
.PURPOSE     read and write selected Measurement Data Sets
.INPUT/OUTPUT
  call as   PROCESS_LV1B_MDS( param, fp, num_state, state );
     input:  
            struct param_record param : struct holding user-defined settings
	    FILE   *fd                : (open) stream pointer
	    unsigned int num_state    : number of state records
	    struct state1_scia state  : structure with States of the product

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void PROCESS_LV1B_MDS( const struct param_record param, FILE *fp, 
		       const unsigned int num_state, 
		       const struct state1_scia *state_in )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies fp, errno, nadc_stat, nadc_err_stack@*/
{
     const char prognm[] = "PROCESS_LV1B_MDS";

     register unsigned short ns;
     
     unsigned int       nr_mds;
     unsigned long long clus_mask;

     struct state1_scia *state;
     struct mds1_scia   *mds;
/*
 * here we assume that all states are of the same type (NADIR, LIMB, ... )
 *  --- this should be checked! ---
 */
     const int source = (int) state_in->type_mds;
/*
 * make a copy of the state records
 */
     state = (struct state1_scia *) 
	  malloc( num_state * sizeof( struct state1_scia ));
     if ( state == NULL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "state" );
     (void) memcpy( state, state_in, num_state * sizeof( struct state1_scia ));
/*
 * read from level 1b and write to level 1b product
 */
     if ( param.write_lv1c == PARAM_UNSET ) {
	  for ( ns = 0; ns < (unsigned short) num_state; ns++ ) {
	       if ( param.flag_silent == PARAM_UNSET 
		    && param.write_sql == PARAM_UNSET )
                    NADC_Info_Update( stdout, 2, ns );

	       clus_mask = SCIA_LV1_CHAN2CLUS( param, state+ns );

	       nr_mds = SCIA_LV1_RD_MDS( fp, clus_mask, state+ns, &mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, 
				     "SCIA_LV1_RD_MDS" );
#ifdef _WITH_SQL
	       if ( param.write_sql == PARAM_SET ) {
#ifdef _KNMI_SQL
		    const unsigned short intg_time = 
			 state[ns].intg_times[state[ns].num_intg - 1];

		    SCIA_LV1_WR_SQL_TILE( conn, param.flag_verbose, 
					  param.infile, intg_time,
					  nr_mds, mds );
#endif
		    SCIA_LV1_FREE_MDS( source, nr_mds, mds );
		    if ( IS_ERR_STAT_FATAL )
			 NADC_GOTO_ERROR(prognm, NADC_ERR_SQL, "SQL_LV1_TILES");
	       } else {
#endif
		    SCIA_WRITE_MDS_1B( param, nr_mds, mds );
		    SCIA_LV1_FREE_MDS( source, nr_mds, mds );
		    if ( IS_ERR_STAT_FATAL )
			 NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, 
					  "SCIA_WR_MDS_1B" );
#ifdef _WITH_SQL
	       }
#endif
	  } /* loop over num_state */
/*
 * read MDS's from level 1b and write MDS's to level 1c product
 */
     } else {
	  char *env_str = getenv( "SCIA_CORR_LOS" );

	  struct mds1c_pmd  *pmd = NULL;
	  struct mds1c_polV *polV = NULL;
	  struct mds1c_scia *mds1c = NULL;

	  unsigned short patch_scia = SCIA_PATCH_NONE;

	  /* set mask with patch algorithms */
	  if ( (param.calib_scia & DO_CORR_VIS_MEM) != UINT_ZERO 
	       && (param.calib_scia & DO_SRON_MEM_NLIN) != UINT_ZERO )
	       patch_scia |= SCIA_PATCH_MEM;
	  if ( (param.calib_scia & DO_CORR_IR_NLIN) != UINT_ZERO 
		    && (param.calib_scia & DO_SRON_MEM_NLIN) != UINT_ZERO )
	       patch_scia |= SCIA_PATCH_NLIN;
	  if ( (param.calib_scia & DO_CORR_STRAY) != UINT_ZERO 
	       && (param.calib_scia & DO_SRON_STRAY) != UINT_ZERO )
	       patch_scia |= SCIA_PATCH_STRAY;

	  for ( ns = 0; ns < (unsigned short) num_state; ns++ ) {
	       unsigned int   nr_mds1c = 0;

	       if ( param.flag_silent == PARAM_UNSET 
		    && param.write_sql == PARAM_UNSET )
                    NADC_Info_Update( stdout, 2, ns );

	       /* read level 1b MDS-records */
	       if ( patch_scia == SCIA_PATCH_NONE ) {
		    clus_mask = SCIA_LV1_CHAN2CLUS( param, state+ns );

		    nr_mds = SCIA_LV1_RD_MDS( fp, clus_mask, state+ns, &mds );
		    if ( IS_ERR_STAT_FATAL )
			 NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, 
					  "SCIA_LV1_RD_MDS" );
		    if ( state[ns].num_clus == 0 ) continue;
	       } else {
		    nr_mds = SCIA_LV1_RD_MDS( fp, ~0ULL, state+ns, &mds );
		    if ( IS_ERR_STAT_FATAL )
			 NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, 
					  "SCIA_LV1_RD_MDS" );
		    /* patch MDS level 1b record */
		    SCIA_LV1_PATCH_MDS( fp, patch_scia, state+ns, mds );
		    if ( IS_ERR_STAT_FATAL ) {
			 SCIA_LV1_FREE_MDS( source, nr_mds, mds );
			 NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
					  "SCIA_LV1_PATCH_MDS" );
		    }
	       }
	       /* correct line-of-sight angles */
	       if ( env_str != NULL && strcmp( env_str, "1" ) == 0 )
		    SCIA_LV1_CORR_LOS( state+ns, mds );

	       /* reconstruct level 1c MDS-records from level 1b MDS-records */
	       mds1c = (struct mds1c_scia *) malloc( 
		    state[ns].num_clus * sizeof(struct mds1c_scia) );
	       if ( mds1c == NULL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mds1c" );

	       nr_mds1c = GET_SCIA_LV1C_MDS( state+ns, mds, mds1c );
	       if ( IS_ERR_STAT_FATAL ) {
		    SCIA_LV1_FREE_MDS( source, nr_mds, mds );
		    SCIA_LV1C_FREE_MDS( source, nr_mds1c, mds1c );
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
				     "GET_SCIA_LV1C_MDS" );
	       }
	       /* select level 1c MDS records on requested clusters */
	       if ( patch_scia != SCIA_PATCH_NONE ) {
		    clus_mask = SCIA_LV1_CHAN2CLUS( param, state+ns );
		    (void) SCIA_LV1C_SELECT_MDS( clus_mask, 
						 state+ns, mds1c );
		    if ( state[ns].num_clus == 0 ) continue;
	       }
	       /* calibrate detector read-outs */
	       SCIA_LV1_CAL( fp, param.calib_scia, state+ns, mds, mds1c );
	       SCIA_LV1_FREE_MDS( source, nr_mds, mds );
	       if ( IS_ERR_STAT_FATAL ) {
		    SCIA_LV1C_FREE_MDS( source, nr_mds1c, mds1c );
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
				     "SCIA_LV1_CALIB" );
	       }
	       /* write level 1c MDS-records */
	       SCIA_WRITE_MDS_1C( param, nr_mds1c, mds1c );
	       SCIA_LV1C_FREE_MDS( source, nr_mds1c, mds1c );
	       if ( IS_ERR_STAT_FATAL ) {
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, 
				     "SCIA_WRITE_MDS_1C" );
	       }
	  } /* loop over num_state */
	  /* 
	   * reconstruct level 1c MDS_PMD-records from level 1b MDS-records
	   */
	  if ( source != SCIA_MONITOR && param.write_pmd == PARAM_SET ) {
	       (void) memcpy( state, state_in, 
			      num_state * sizeof( struct state1_scia ));

	       clus_mask  = 0ULL;         /* do not read any cluster data */
	       for ( ns = 0; ns < (unsigned short) num_state; ns++ ) {
		    nr_mds = SCIA_LV1_RD_MDS( fp, clus_mask, state+ns, &mds );
		    if ( IS_ERR_STAT_FATAL )
			 NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, 
					  "SCIA_LV1_RD_MDS" );
		    pmd = (struct mds1c_pmd *) 
			 malloc( sizeof(struct mds1c_pmd) );
		    if ( pmd == NULL )
			 NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "pmd" );

		    (void) GET_SCIA_LV1C_PMD( state+ns, mds, pmd );
		    SCIA_LV1_FREE_MDS( source, nr_mds, mds );
		    if ( IS_ERR_STAT_FATAL ) {
			 SCIA_LV1C_FREE_MDS_PMD( source, pmd );
			 NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
					  "GET_SCIA_LV1C_PMD" );
		    }
		    SCIA_WRITE_MDS_PMD( param, pmd );
		    SCIA_LV1C_FREE_MDS_PMD( source, pmd );
		    if ( IS_ERR_STAT_FATAL )
			 NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, 
					  "SCIA_WRITE_MDS_PMD" );
	       }
	  }
	  /*
	   * reconstruct level 1c MDS_POLV-records from level 1b MDS-records
	   */
	  if ( source != SCIA_MONITOR && param.write_polV == PARAM_SET ) {
	       (void) memcpy( state, state_in, 
			      num_state * sizeof( struct state1_scia ));

	       clus_mask  = 0ULL;         /* do not read any cluster data */
	       for ( ns = 0; ns < (unsigned short) num_state; ns++ ) {
		    nr_mds = SCIA_LV1_RD_MDS( fp, clus_mask, state+ns, &mds );
		    if ( IS_ERR_STAT_FATAL )
			 NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, 
					  "SCIA_LV1_RD_MDS" );
	       
		    polV = (struct mds1c_polV *) 
			 malloc( sizeof(struct mds1c_polV));
		    if ( polV == NULL )
			 NADC_GOTO_ERROR( prognm,NADC_ERR_ALLOC,"polV" );

		    (void) GET_SCIA_LV1C_POLV( state+ns, mds, polV );
		    SCIA_LV1_FREE_MDS( source, nr_mds, mds );
		    if ( IS_ERR_STAT_FATAL ) {
			 SCIA_LV1C_FREE_MDS_POLV( source, polV );
			 NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
					  "GET_SCIA_LV1C_PMD" );
		    }
		    SCIA_WRITE_MDS_POLV( param, polV );
		    SCIA_LV1C_FREE_MDS_POLV( source, polV );
		    if ( IS_ERR_STAT_FATAL )
			 NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, 
					  "SCIA_WRITE_MDS_POLV" );
	       }
	  }
     }
 done:
     free( state );
}

/*+++++++++++++++++++++++++
.IDENTifer   PROCESS_LV1C_MDS
.PURPOSE     read and write selected level 1c Measurement Data Sets
.INPUT/OUTPUT
  call as   PROCESS_LV1C_MDS( param, fp, calopt, num_state, state );
     input:  
            struct param_record param : struct holding user-defined settings
	    FILE   *fd                : (open) stream pointer
	    struct cal_options calopt : L1C calibration options
	    unsigned int num_state    : number of state records
	    struct state1_scia state  : structure with States of the product

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void PROCESS_LV1C_MDS( const struct param_record param, FILE *fp, 
		       const struct cal_options calopt,
		       const unsigned int num_state, 
		       const struct state1_scia *state_in )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp@*/
{
     const char prognm[] = "PROCESS_LV1C_MDS";

     register unsigned short nc, ns;

     int                source;
     unsigned int       nr_mds;
     unsigned long long clus_mask;

     struct state1_scia *state;
     struct mds1c_scia  *mds;
     struct mds1c_pmd   *mds_pmd;
     struct mds1c_polV  *mds_polV;
/*
 * make a copy of the state records
 */
     state = (struct state1_scia *) 
	  malloc( num_state * sizeof( struct state1_scia ));
     if ( state == NULL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "state" );
     (void) memcpy( state, state_in, num_state * sizeof( struct state1_scia ));
/*
 * loop over all states
 */
     for ( ns = 0; ns < (unsigned short) num_state; ns++ ) {
	  clus_mask = 0ULL;
	  source = (int) state[ns].type_mds;

	  if ( param.flag_silent == PARAM_UNSET 
	       && param.write_sql == PARAM_UNSET )
	       NADC_Info_Update( stdout, 2, ns );

          switch ( source ) {
          case SCIA_NADIR:
	       if ( calopt.nadir_mds == SCHAR_ZERO ) return;
               for ( nc = 0; nc < MAX_CLUSTER; nc++ )
                    if ( calopt.nadir_cluster[nc] != SCHAR_ZERO )
                         Set_Bit_LL( &clus_mask, (unsigned char) nc );
               break;
          case SCIA_LIMB:
	       if ( calopt.limb_mds == SCHAR_ZERO ) return;
               for ( nc = 0; nc < MAX_CLUSTER; nc++ )
                    if ( calopt.limb_cluster[nc] != SCHAR_ZERO )
                         Set_Bit_LL( &clus_mask, (unsigned char) nc );
               break;
          case SCIA_OCCULT:
	       if ( calopt.occ_mds == SCHAR_ZERO ) return;
               for ( nc = 0; nc < MAX_CLUSTER; nc++ )
                    if ( calopt.occ_cluster[nc] != SCHAR_ZERO )
                         Set_Bit_LL( &clus_mask, (unsigned char) nc );
               break;
          case SCIA_MONITOR:
	       if ( calopt.moni_mds == SCHAR_ZERO ) return;
               for ( nc = 0; nc < MAX_CLUSTER; nc++ )
                    if ( calopt.moni_cluster[nc] != SCHAR_ZERO )
                         Set_Bit_LL( &clus_mask, (unsigned char) nc );
               break;
          }
/*
 * now apply input parameters for this program
 */
	  clus_mask &= SCIA_LV1_CHAN2CLUS( param, state+ns );
/*
 * read the data
 */
	  nr_mds = SCIA_LV1C_RD_MDS( fp, clus_mask, state+ns, &mds );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, 
				  "SCIA_LV1C_RD_MDS" );
/* 
 * correct radiances by a multiplication factor
 */
	  if ( (param.calib_scia & DO_PATCH_L1C) != UINT_ZERO ) {
	       SCIA_LV1C_SCALE( nr_mds, mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR(prognm, NADC_ERR_FATAL, "SCIA_LV1C_CAL");
	  }
/*
 * write measurement data sets
 */
	  SCIA_WRITE_MDS_1C( param, nr_mds, mds );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, 
				  "SCIA_WRITE_MDS_1C" );
	  SCIA_LV1C_FREE_MDS( source, nr_mds, mds );
     }
/*
 * read/write MDS records with PMD values
 */
     if ( calopt.pmd_mds != SCHAR_ZERO && param.write_pmd == PARAM_SET ) {
	  (void) memcpy( state, state_in, 
			 num_state * sizeof( struct state1_scia ));

	  for ( ns = 0; ns < (unsigned short) num_state; ns++ ) {
	       (void) SCIA_LV1C_RD_MDS_PMD( fp, state+ns, &mds_pmd );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, 
				     "SCIA_LV1C_RD_MDS_PMD" );
	       SCIA_WRITE_MDS_PMD( param, mds_pmd );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, 
				     "SCIA_WRITE_MDS_PMD" );
	       SCIA_LV1C_FREE_MDS_PMD( (int) state[ns].type_mds, mds_pmd );
	  }
     }
/*
 * read/write MDS records with fractional polarisation values
 */
     if ( calopt.frac_pol_mds != SCHAR_ZERO && param.write_polV == PARAM_SET ) {
	  (void) memcpy( state, state_in, 
			 num_state * sizeof( struct state1_scia ));

	  for ( ns = 0; ns < (unsigned short) num_state; ns++ ) {
	       (void) SCIA_LV1C_RD_MDS_POLV( fp, state+ns, &mds_polV );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, 
				     "SCIA_LV1C_RD_MDS_POLV" );
	       SCIA_WRITE_MDS_POLV( param, mds_polV );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, 
				     "SCIA_WRITE_MDS_POLV" );
	       SCIA_LV1C_FREE_MDS_POLV( (int) state[ns].type_mds, mds_polV );
	  }
     }
 done:
     free( state );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int main( int argc, char *argv[] )
     /*@globals  errno, stderr, stdout, nadc_stat, nadc_err_stack, 
       Use_Extern_Alloc;@*/
     /*@modifies errno, stderr, stdout, nadc_stat, nadc_err_stack@*/
{
     const char prognm[] = "scia_nl1";

     unsigned int num_dsd, num_dsr, num_state;
     unsigned int num;

     int is_scia_lv1c;

     FILE  *fp = NULL;

     struct param_record param;
     struct base_scia   base;
     struct mph_envi    mph;
     struct sip_scia    sip;
     struct sph1_scia   sph;
     struct dsd_envi    *dsd;
     struct sqads1_scia *sqads;
     struct lads_scia   *lads;
     struct clcp_scia   clcp;
     struct vlcp_scia   *vlcp;
     struct ppg_scia    ppg;
     struct scp_scia    *scp;
     struct srs_scia    *srs;
     struct cal_options calopt;
     struct pspn_scia   *pspn;
     struct psplo_scia  *pspl;
     struct psplo_scia  *pspo;
     struct rspn_scia   *rspn;
     struct rsplo_scia  *rspl;
     struct rsplo_scia  *rspo;
     struct ekd_scia    ekd;
     struct asfp_scia   *asfp;
     struct sfp_scia    *sfp;
     struct state1_scia *state, *mds_state;
     struct pmd_scia    *pmd;
     struct aux_scia    *aux;
     struct lcpn_scia   *lcpn;
     struct ppgn_scia   *ppgn;
     struct dark_scia   *dark;
     struct scpn_scia   *scpn;
     struct srsn_scia   *srsn;
/*
 * initialization of command-line parameters
 */
     SCIA_SET_PARAM( argc, argv, SCIA_LEVEL_1, &param );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, "NADC_INIT_PARAM" );
/*
 * check if we have to display version and exit
 */
     if ( param.flag_version == PARAM_SET ) {
	  SCIA_SHOW_VERSION( stdout, prognm );
	  exit( EXIT_SUCCESS );
     }
/*
 * dump command-line parameters
 */
     if ( param.flag_show == PARAM_SET ) {
	  SCIA_SHOW_PARAM( SCIA_LEVEL_1, param );
	  exit( EXIT_SUCCESS );
     }
/*
 * connect to PostgreSQL database
 */
#ifdef _WITH_SQL
     if ( param.write_sql == PARAM_SET ) {
	  CONNECT_NADC_DB( &conn, "scia" );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, "PSQL" );
	  if ( param.flag_sql_remove == PARAM_SET
	       || param.flag_sql_replace == PARAM_SET )
	       SCIA_LV1_DEL_ENTRY( conn, param.flag_verbose, param.infile );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, "PSQL(remove)" );
	  if ( param.flag_sql_remove == PARAM_SET ) goto done;
     }
#endif
/*
 * open input-file
 */
     if ( (fp = fopen( param.infile, "rb" )) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, param.infile );
/*
 * create output HDF5 file
 */
     if ( param.write_hdf5 == PARAM_SET ) {
	  param.hdf_file_id = SCIA_CRE_H5_FILE( SCIA_LEVEL_1, &param );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_CRE, "HDF5 base" );
	  SCIA_WR_H5_VERSION( param.hdf_file_id );
/*
 * create for data structures for SCIAMACHY level 1b data
 */
	  CRE_SCIA_LV1_H5_STRUCTS( param );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_CRE, "STRUCTS" );
     }
/*
 * -------------------------
 * read/write Main Product Header
 */
     ENVI_RD_MPH( fp, &mph );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "MPH" );
     if ( mph.tot_size != NADC_FILESIZE( param.infile ) )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "file size check failed" );
     SCIA_WRITE_MPH( param, &mph );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "MPH" );
/*
 * -------------------------
 * read/write Specific Product Header
 */
     SCIA_LV1_RD_SPH( fp, mph, &sph );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SPH" );
     SCIA_WRITE_SPH( param, mph, &sph );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "SPH" );
/*
 * -------------------------
 * read/write Data Set Descriptor records
 */
     dsd = (struct dsd_envi *)
	  malloc( (mph.num_dsd-1) * sizeof( struct dsd_envi ) );
     if ( dsd == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "dsd" );
     num_dsd = ENVI_RD_DSD( fp, mph, dsd );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "DSD" );
     is_scia_lv1c = IS_SCIA_LV1C( num_dsd, dsd );
     SCIA_WRITE_DSD( param, num_dsd, dsd );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "DSD" );
     if ( param.write_meta == PARAM_SET ) goto done;
#ifdef _WITH_SQL
     if ( param.write_sql == PARAM_SET ) {
	  SCIA_LV1_WR_SQL_META( conn, param.flag_verbose, param.infile, 
				&mph, &sph );
	  if ( IS_ERR_STAT_WARN ) goto done;
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, "SQL_META" );
	  SCIA_LV1_WR_SQL_AUX( conn, param.flag_verbose, &mph, num_dsd, dsd );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, "SQL_AUX" );
     }
#endif
     SCIA_LV1_SET_NUM_ATTACH( param, fp, num_dsd, dsd );
/*
 * -------------------------
 * read/write Summary of Quality Flags per State records
 */
     num_dsr = SCIA_LV1_RD_SQADS( fp, num_dsd, dsd, &sqads );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SQADS" );
     if ( param.write_pds == PARAM_SET )
	  num_dsr = SCIA_LV1_UPDATE_SQADS( sqads );
     SCIA_WRITE_SQADS( param, num_dsr, sqads );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "SQADS" );
/*
 * -------------------------
 * read/write Geolocation of the States
 */
     num_dsr = SCIA_RD_LADS( fp, num_dsd, dsd, &lads );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "LADS" );
     if ( param.write_pds == PARAM_SET )
	  num_dsr = SCIA_LV1_UPDATE_LADS( lads );
     SCIA_WRITE_LADS( param, num_dsr, lads );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "LADS" );
/*
 * -------------------------
 * read/write Static Instrument Parameters
 */
     if ( param.write_gads == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_SIP( fp, num_dsd, dsd, &sip );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SIP" );
	  SCIA_WRITE_SIP( param, num_dsr, sip );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "SIP" );
     }
/*
 * -------------------------
 * read/write Leakage Current Parameters (constant fraction)
 */
     if ( param.write_gads == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_CLCP( fp, num_dsd, dsd, &clcp );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "CLCP" );
	  SCIA_WRITE_CLCP( param, num_dsr, clcp );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "CLCP" );
     }
/*
 * -------------------------
 * read/write Leakage Current Parameters (variable fraction)
 */
     if ( param.write_gads == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_VLCP( fp, num_dsd, dsd, &vlcp );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "VLCP" );
	  SCIA_WRITE_VLCP( param, num_dsr, vlcp );
	  if ( num_dsr > 0 ) free( vlcp );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "VLCP" );
     }
/*
 * -------------------------
 * read/write PPG/Etalon Parameters
 */
     if ( param.write_gads == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_PPG( fp, num_dsd, dsd, &ppg );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "PPG" );
	  SCIA_WRITE_PPG( param, num_dsr, ppg );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "PPG" );
     }
/*
 * -------------------------
 * read/write Precise Basis of the Spectral Calibration
 */
     if ( param.write_gads == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_BASE( fp, num_dsd, dsd, &base );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "BASE" );
	  if ( num_dsr > 0 ) SCIA_WRITE_BASE( param, &base );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "BASE" );
     }
/*
 * -------------------------
 * read/write Spectral Calibration Parameters
 */
     if ( param.write_gads == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_SCP( fp, num_dsd, dsd, &scp );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SCP" );
	  SCIA_WRITE_SCP( param, num_dsr, scp );
	  if ( num_dsr > 0 ) free( scp );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "SCP" );
     }
/*
 * -------------------------
 * read/write Sun Reference Spectrum 
 *    always write this GADS, because it is required by most retrievals
 */
     num_dsr = SCIA_LV1_RD_SRS( fp, num_dsd, dsd, &srs );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SRS" );
     /* KB: apply m-factor from ext. database, if requested */
     SCIA_LV1_MFACTOR_SRS( mph.sensing_start, param.calib_scia, num_dsr, srs );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "SRS(mfactor)" );
     SCIA_WRITE_SRS( param, num_dsr, srs );
     if ( num_dsr > 0 ) free( srs );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "SRS" );
/*
 * -------------------------
 * read/write Polarisation Sensitivity Parameters Nadir
 */
     if ( param.write_gads == PARAM_SET ) { 
	  num_dsr = SCIA_LV1_RD_PSPN( fp, num_dsd, dsd, &pspn );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "PSPN" );
	  SCIA_WRITE_PSPN( param, num_dsr, pspn );
	  if ( num_dsr > 0 ) free( pspn );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "PSPN" );
     }
/*
 * -------------------------
 * read/write Polarisation Sensitivity Parameters Limb
 */
     if ( param.write_gads == PARAM_SET ) { 
	  num_dsr = SCIA_LV1_RD_PSPL( fp, num_dsd, dsd, &pspl );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "PSPL" );
	  SCIA_WRITE_PSPL( param, num_dsr, pspl );
	  if ( num_dsr > 0 ) free( pspl );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "PSPL" );
     }
/*
 * -------------------------
 * read/write Polarisation Sensitivity Parameters Occultation
 */
     if ( param.write_gads == PARAM_SET ) { 
	  num_dsr = SCIA_LV1_RD_PSPO( fp, num_dsd, dsd, &pspo );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "PSPO" );
	  SCIA_WRITE_PSPO( param, num_dsr, pspo );
	  if ( num_dsr > 0 ) free( pspo );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "PSPO" );
     }
/*
 * -------------------------
 * read/write Radiation Sensitivity Parameters Nadir
 */
     if ( param.write_gads == PARAM_SET ) { 
	  num_dsr = SCIA_LV1_RD_RSPN( fp, num_dsd, dsd, &rspn );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "RSPN" );
	  SCIA_WRITE_RSPN( param, num_dsr, rspn );
	  if ( num_dsr > 0 ) free( rspn );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "RSPN" );
     }
/*
 * -------------------------
 * read/write Radiation Sensitivity Parameters Limb
 */
     if ( param.write_gads == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_RSPL( fp, num_dsd, dsd, &rspl );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "RSPL" );
	  SCIA_WRITE_RSPL( param, num_dsr, rspl );
	  if ( num_dsr > 0 ) free( rspl );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "RSPL" );
     }
/*
 * -------------------------
 * read/write Radiation Sensitivity Parameters Occultation
 */
     if ( param.write_gads == PARAM_SET ) { 
	  num_dsr = SCIA_LV1_RD_RSPO( fp, num_dsd, dsd, &rspo );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "RSPO" );
	  SCIA_WRITE_RSPO( param, num_dsr, rspo );
	  if ( num_dsr > 0 ) free( rspo );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "RSPO" );
     }
/*
 * -------------------------
 * read/write Errors on Key Data
 */
     if ( param.write_gads == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_EKD( fp, num_dsd, dsd, &ekd );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "EKD" );
	  SCIA_WRITE_EKD( param, num_dsr, ekd );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "EKD" );
     }
/*
 * -------------------------
 * read/write Slit Function Parameters
 */
     if ( param.write_gads == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_SFP( fp, num_dsd, dsd, &sfp );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SFP" );
	  SCIA_WRITE_SFP( param, num_dsr, sfp );
	  if ( num_dsr > 0 ) free( sfp );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "SFP" );
     }
/*
 * -------------------------
 * read/write Small Aperture Slit Function Parameters
 */
     if ( param.write_gads == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_ASFP( fp, num_dsd, dsd, &asfp );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "ASFP" );
	  SCIA_WRITE_ASFP( param, num_dsr, asfp );
	  if ( num_dsr > 0 ) free( asfp );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "ASFP" );
     }
/* -------------------------
 * read/write States of the Product
 */
     num_state = SCIA_LV1_RD_STATE( fp, num_dsd, dsd, &state );
     if ( IS_ERR_STAT_FATAL || num_state  ==  0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "STATE" );
     if ( param.write_pds == PARAM_SET )
	  num_state = SCIA_LV1_UPDATE_STATE( state );
     SCIA_WRITE_STATE( param, num_state, state );
     if ( IS_ERR_STAT_FATAL ) {
	  free( sqads );
	  free( lads );
	  free( state );
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "STATE" );
     }
#ifdef _WITH_SQL
     if ( param.write_sql == PARAM_SET ) {
	  SCIA_LV1_MATCH_STATE( conn, param.flag_verbose, 
				&mph, num_state, lads, sqads, state );
	  if ( IS_ERR_STAT_FATAL ) {
	       free( sqads );
	       free( lads );
	       free( state );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, "SQL_STATE" );
	  }
     }
#endif
     free( sqads );
     free( lads );
     free( state );
#ifdef _WITH_SQL
#ifndef _KNMI_SQL
     if ( param.write_sql == PARAM_SET ) goto done;
#endif
#endif
/*
 * -------------------------
 * read Calibration parameters (level 1c, only)
 */
     if ( is_scia_lv1c ) {
	  num_dsr = SCIA_LV1C_RD_CALOPT( fp, num_dsd, dsd, &calopt );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "CALOPT" );
	  if ( num_dsr > 0 ) {
	       if ( param.write_ascii == PARAM_SET ) {
		    SCIA_LV1C_WR_ASCII_CALOPT( param, &calopt );
		    if ( IS_ERR_STAT_FATAL ) 
			 NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "CALOPT" );
	       }
	       if ( param.write_pds == PARAM_SET ) {
		    SCIA_LV1C_UPDATE_CALOPT( is_scia_lv1c, param, &calopt );
		    SCIA_LV1C_WR_CALOPT( fp_out, num_dsr, calopt );
	       }
	  }
     } else if ( param.write_pds == PARAM_SET 
		 && param.write_lv1c == PARAM_SET ) {
	  SCIA_LV1C_UPDATE_CALOPT( is_scia_lv1c, param, &calopt );
	  SCIA_LV1C_WR_CALOPT( fp_out, num_dsr, calopt );
     }
/*
 * -------------------------
 * read/write PMD Data Packets
 */
     if ( param.write_ads == PARAM_SET && param.write_pmd0 == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_PMD( fp, num_dsd, dsd, &pmd );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "PMD" );
	  SCIA_WRITE_PMD( param, num_dsr, pmd );
	  if ( num_dsr > 0 ) free( pmd );
     }
/*
 * -------------------------
 * read/write Auxiliary Data Packets
 */
     if ( param.write_ads == PARAM_SET && param.write_aux0 == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_AUX( fp, num_dsd, dsd, &aux );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "AUX" );
	  SCIA_WRITE_AUX( param, num_dsr, aux );
	  if ( num_dsr > 0 ) free( aux );
     }
/*
 * -------------------------
 * read/write Leakage Current Parameters (newly calculated)
 */
     if ( param.write_ads == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_LCPN( fp, num_dsd, dsd, &lcpn );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "LCPN" );
	  SCIA_WRITE_LCPN( param, num_dsr, lcpn );
	  if ( num_dsr > 0 ) free( lcpn );
     }
/*
 * -------------------------
 * read/write Average of the Dark Measurements per State
 */
     if ( param.write_ads == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_DARK( fp, num_dsd, dsd, &dark );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "DARK" );
	  SCIA_WRITE_DARK( param, num_dsr, dark );
	  if ( num_dsr > 0 ) free( dark );
     }
/*
 * -------------------------
 * read/write PPG/Etalon Parameters (newly calculated)
 */
     if ( param.write_ads == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_PPGN( fp, num_dsd, dsd, &ppgn );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "PPGN" );
	  SCIA_WRITE_PPGN( param, num_dsr, ppgn );
	  if ( num_dsr > 0 ) free( ppgn );
     }
/*
 * -------------------------
 * read/write Spectral Calibration Parameters (newly calculated)
 */
     if ( param.write_ads == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_SCPN( fp, num_dsd, dsd, &scpn );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SCPN" );
	  SCIA_WRITE_SCPN( param, num_dsr, scpn );
	  if ( num_dsr > 0 ) free( scpn );
     }
/*
 * -------------------------
 * read/write Sun Reference Spectrum (newly calculated)
 */
     if ( param.write_ads == PARAM_SET ) {
	  num_dsr = SCIA_LV1_RD_SRSN( fp, num_dsd, dsd, &srsn );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SRSN" );
	  SCIA_WRITE_SRSN( param, num_dsr, srsn );
	  if ( num_dsr > 0 ) free( srsn );
     }
/*
 * -------------------------
 * read/write Nadir MDS
 */
     num = SCIA_LV1_SELECT_MDS( SCIA_NADIR, param, 
				fp, num_dsd, dsd, &mds_state );
     if ( param.flag_silent == PARAM_UNSET && param.write_sql == PARAM_UNSET )
          NADC_Info_Proc( stdout, "MDS (nadir)", num );
     if ( num > 0u ) {
	  if ( is_scia_lv1c ) {
	       (void) SCIA_LV1C_RD_CALOPT( fp, num_dsd, dsd, &calopt );
	       PROCESS_LV1C_MDS( param, fp, calopt, num, mds_state );
	  } else
	       PROCESS_LV1B_MDS( param, fp, num, mds_state );
	  free( mds_state );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "PROCESS_LV1x_MDS" );
	  if ( param.flag_silent == PARAM_UNSET 
	       && param.write_sql == PARAM_UNSET )
               NADC_Info_Finish( stdout, 2, num );
     } else if ( param.flag_silent == PARAM_UNSET 
		 && param.write_sql == PARAM_UNSET )
	  NADC_Info_Finish( stdout, 2, num );
/*
 * -------------------------
 * read/write Limb MDS
 */
     num = SCIA_LV1_SELECT_MDS( SCIA_LIMB, param,
				fp, num_dsd, dsd, &mds_state );
     if ( param.flag_silent == PARAM_UNSET && param.write_sql == PARAM_UNSET )
          NADC_Info_Proc( stdout, "MDS (limb)", num );
     if ( num > 0u ) {
	  if ( is_scia_lv1c ) {
	       (void) SCIA_LV1C_RD_CALOPT( fp, num_dsd, dsd, &calopt );
	       PROCESS_LV1C_MDS( param, fp, calopt, num, mds_state );
	  } else
	       PROCESS_LV1B_MDS( param, fp, num, mds_state );
	  free( mds_state );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "PROCESS_LV1x_MDS" );
	  if ( param.flag_silent == PARAM_UNSET 
	       && param.write_sql == PARAM_UNSET )
               NADC_Info_Finish( stdout, 2, num );
     } else if ( param.flag_silent == PARAM_UNSET 
		 && param.write_sql == PARAM_UNSET )
	  NADC_Info_Finish( stdout, 2, num );
/*
 * -------------------------
 * read/write Occultation MDS
 */
     num = SCIA_LV1_SELECT_MDS( SCIA_OCCULT, param, 
				fp, num_dsd, dsd, &mds_state );
     if ( param.flag_silent == PARAM_UNSET && param.write_sql == PARAM_UNSET )
          NADC_Info_Proc( stdout, "MDS (occult)", num );
     if ( num > 0u ) {
	  if ( is_scia_lv1c ){
	       (void) SCIA_LV1C_RD_CALOPT( fp, num_dsd, dsd, &calopt );	       
	       PROCESS_LV1C_MDS( param, fp, calopt, num, mds_state );
	  } else
	       PROCESS_LV1B_MDS( param, fp, num, mds_state );
	  free( mds_state );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "PROCESS_LV1x_MDS" );
	  if ( param.flag_silent == PARAM_UNSET 
	       && param.write_sql == PARAM_UNSET )
               NADC_Info_Finish( stdout, 2, num );
     } else if ( param.flag_silent == PARAM_UNSET 
		 && param.write_sql == PARAM_UNSET )
	  NADC_Info_Finish( stdout, 2, num );
/*
 * -------------------------
 * read/write Monitoring MDS
 */
     num = SCIA_LV1_SELECT_MDS( SCIA_MONITOR, param,  
				fp, num_dsd, dsd, &mds_state );
     if ( param.flag_silent == PARAM_UNSET && param.write_sql == PARAM_UNSET )
          NADC_Info_Proc( stdout, "MDS (monitor)", num );
     if ( num > 0u ) {
	  if ( is_scia_lv1c ){
	       (void) SCIA_LV1C_RD_CALOPT( fp, num_dsd, dsd, &calopt );
	       PROCESS_LV1C_MDS( param, fp, calopt, num, mds_state );
	  } else
	       PROCESS_LV1B_MDS( param, fp, num, mds_state );
	  free( mds_state );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "PROCESS_LV1x_MDS" );
	  if ( param.flag_silent == PARAM_UNSET 
	       && param.write_sql == PARAM_UNSET )
               NADC_Info_Finish( stdout, 2, num );
     } else if ( param.flag_silent == PARAM_UNSET 
		 && param.write_sql == PARAM_UNSET )
	  NADC_Info_Finish( stdout, 2, num );
/*
 * free allocated memory
 */
     free( dsd );
/*
 * when an error has occurred we jump to here:
 */
 done:
/*
 * close input file and PDS file
 */
     if ( fp != NULL ) {
	  if ( ! IS_ERR_STAT_FATAL && param.write_pds == PARAM_SET ) {
	       SCIA_LV1_WR_DSD_UPDATE( fp, fp_out );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_ERROR( prognm, NADC_ERR_FATAL, "LV1_WR_DSD_UPDATE" );
	  }
	  (void) fclose( fp );
     }
/*
 * close connection to PostgreSQL database
 */
#ifdef _WITH_SQL
     if ( param.write_sql == PARAM_SET && conn != NULL ) PQfinish( conn );
#endif
/*
 * close HDF5 output file
 */
     if ( param.write_hdf5 == PARAM_SET ) {
	  if ( param.hdf_file_id >= 0 && H5Fclose( param.hdf_file_id ) < 0 )
	       NADC_ERROR( prognm, NADC_ERR_HDF_FILE, param.hdf5_name );
     }
/*
 * display error messages?
 */
     if ( param.flag_silent == PARAM_UNSET ) NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL ) 
	  return NADC_ERR_FATAL;
     else
	  return NADC_ERR_NONE;
}
