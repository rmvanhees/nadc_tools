/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2013 SRON (R.M.van.Hees@sron.nl)

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
.VERSION      2.6   08-Oct-2013	[-check] show CRC and Reed-Solomon errors, RvH
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
int main( int argc, char *argv[] )
     /*@globals  errno, stderr, stdout, nadc_stat, nadc_err_stack, 
       Use_Extern_Alloc;@*/
     /*@modifies errno, stderr, stdout, nadc_stat, nadc_err_stack@*/
{
     const char prognm[] = "scia_nl0";

     unsigned int num_dsd, num_info, num_mds, num;

     FILE  *fd = NULL;
#ifdef _WITH_SQL
     PGconn *conn = NULL;
#endif
     struct param_record param;
     struct mph_envi  mph;
     struct sph0_scia sph;
     struct dsd_envi  *dsd = NULL;
     struct mds0_info *mds_info = NULL;
     struct mds0_info *info = NULL;
     struct mds0_aux  *aux;
     struct mds0_det  *det;
     struct mds0_pmd  *pmd;
/*
 * initialization of command-line parameters
 */
     SCIA_SET_PARAM( argc, argv, SCIA_LEVEL_0, &param );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, "" );
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
          SCIA_SHOW_PARAM( SCIA_LEVEL_0, param );
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
	       SCIA_LV0_DEL_ENTRY( conn, param.flag_verbose, param.infile );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, "PSQL(remove)" );
	  if ( param.flag_sql_remove == PARAM_SET ) goto done;
     }
#endif
/*
 * open input-file
 */
     if ( (fd = fopen( param.infile, "r" )) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, param.infile );
/*
 * create output file
 */
     if ( param.write_hdf5 == PARAM_SET ) {
	  param.hdf_file_id = SCIA_CRE_H5_FILE( SCIA_LEVEL_0, &param );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_CRE, "HDF5 base" );
	  SCIA_WR_H5_VERSION( param.hdf_file_id );
     }
/*
 * -------------------------
 * read Main Product Header
 */
     ENVI_RD_MPH( fd, &mph );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "MPH" );
     if ( mph.tot_size != nadc_file_size( param.infile ) )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "file size check failed" );
     if ( param.write_ascii == PARAM_SET ) {
	  ENVI_WR_ASCII_MPH( param, &mph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "MPH" );
     }
     if ( param.write_hdf5 == PARAM_SET ) {
	  SCIA_WR_H5_MPH( param, &mph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "MPH" );
     }
#ifdef _WITH_SQL
     if ( param.write_sql == PARAM_SET ) {
	  SCIA_LV0_WR_SQL_META( conn, param.flag_verbose, param.infile, &mph );
	  if ( IS_ERR_STAT_WARN ) goto done;
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, "SQL_META" );
     }
#endif
/*
 * -------------------------
 * read Specific Product Header
 */
     SCIA_LV0_RD_SPH( fd, mph, &sph );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SPH" );
     if ( param.write_ascii == PARAM_SET ) {
	  SCIA_LV0_WR_ASCII_SPH( param, &sph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "SPH" );
     }
     if ( param.write_hdf5 == PARAM_SET ) {
	  SCIA_LV0_WR_H5_SPH( param, &sph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "SPH" );
     }
/*
 * -------------------------
 * read Data Set Descriptor records
 */
     dsd = (struct dsd_envi *)
	  malloc( (mph.num_dsd-1) * sizeof( struct dsd_envi ) );
     if ( dsd == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "dsd" );
     num_dsd = ENVI_RD_DSD( fd, mph, dsd );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "DSD" );
     if ( param.write_ascii == PARAM_SET ) {
	  ENVI_WR_ASCII_DSD( param, num_dsd, dsd );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "DSD" );
     }
     if ( param.write_meta == PARAM_SET ) goto done;
/*
 * -------------------------
 * read SCIAMACHY source packets
 *
 * first try to read the MDS info data
 */
     num_info = SCIA_LV0_RD_MDS_INFO( fd, num_dsd, dsd, &info );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, "RD_MDS_INFO" );
     if ( param.write_ascii == PARAM_SET ) {
	  SCIA_LV0_WR_ASCII_INFO( param, num_info, info );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, "WR_MDS_INFO" );
     }
#ifdef _WITH_SQL
     if ( param.write_sql == PARAM_SET ) {
	  unsigned short numState = 0u;

	  unsigned int sumAux = 0u;
	  unsigned int sumDet = 0u;
	  unsigned int sumPMD = 0u;

	  struct mds0_info *infoAux = NULL;
	  struct mds0_info *infoDet = NULL;
	  struct mds0_info *infoPMD = NULL;

	  struct mds0_sql sqlState[256];

	  const unsigned int totalAux = 
	       SCIA_LV0_SELECT_MDS( SCIA_AUX_PACKET, param, num_info, 
				    info, &infoAux );
	  const unsigned int totalDet = 
	       SCIA_LV0_SELECT_MDS( SCIA_DET_PACKET, param, num_info, 
				    info, &infoDet );
	  const unsigned int totalPMD = 
	       SCIA_LV0_SELECT_MDS( SCIA_PMD_PACKET, param, num_info, 
				    info, &infoPMD );

	  while ( sumAux < totalAux && sumDet < totalDet ) {
	       /* number of Auxiliary records of current state */
	       sqlState[numState].nrAux = 
		    GET_SCIA_LV0_STATE_AUX( fd, infoAux+sumAux, 
					    totalAux-sumAux, &aux );
	       if ( IS_ERR_STAT_FATAL ) goto failed;

	       /* number of Detector records of current state */
	       sqlState[numState].nrDet = 
		    GET_SCIA_LV0_STATE_DET( BAND_ALL, fd, infoDet+sumDet, 
					    totalDet-sumDet, &det );
	       if ( IS_ERR_STAT_FATAL ) goto failed;

               /* number of PMD records of current state */
	       sqlState[numState].nrPMD = 
		    GET_SCIA_LV0_STATE_PMD( fd, infoPMD+sumPMD, 
					    totalPMD-sumPMD, &pmd );
	       if ( IS_ERR_STAT_FATAL ) goto failed;

	       sqlState[numState].stateID = infoDet[sumDet].state_id;

	       /* set DateTime and StateID of current state */
	       (void) memcpy( &sqlState[numState].mjd, &infoDet[sumDet].mjd,
			      sizeof( struct mjd_envi ) );

	       /* obtain OBM temperature of current state */
	       GET_SCIA_LV0_STATE_OBMtemp( TRUE, sqlState[numState].nrAux, 
					   aux, &sqlState[numState].obmTemp );
	       if ( IS_ERR_STAT_FATAL ) goto failed;

	       /* obtain Detectory array temperature of current state */
	       GET_SCIA_LV0_STATE_DETtemp( sqlState[numState].nrDet, det, 
					   sqlState[numState].chanTemp );
	       if ( IS_ERR_STAT_FATAL ) goto failed;

	       /* obtain Detectory array temperature of current state */
	       GET_SCIA_LV0_STATE_PMDtemp( sqlState[numState].nrPMD, pmd, 
					   &sqlState[numState].pmdTemp );
	       if ( IS_ERR_STAT_FATAL ) goto failed;

	       /* release allocated memory */
	       if ( sqlState[numState].nrAux > 0 ) free( aux );
	       SCIA_LV0_FREE_MDS_DET( sqlState[numState].nrDet, det );
	       if ( sqlState[numState].nrPMD > 0 ) free( pmd );

	       sumAux += sqlState[numState].nrAux;
	       sumDet += sqlState[numState].nrDet;
	       sumPMD += sqlState[numState].nrPMD;
	       if ( ++numState >= 256 ) break;      /* should complain here */
	  }
 failed:
	  if ( totalAux > 0 ) free( infoAux );
	  if ( totalDet > 0 ) free( infoDet );
	  if ( totalPMD > 0 ) free( infoPMD );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "MDS0" );

	  SCIA_LV0_MATCH_STATE( conn, param.flag_verbose,
				&mph, numState, sqlState );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_SQL, "SQL_STATE" );
	  goto done;
     }
#endif
/* 
 * process Auxiliary source packets
 */
     num_mds = SCIA_LV0_SELECT_MDS( SCIA_AUX_PACKET, param, num_info, info,
				    &mds_info );
     (void) fprintf( stdout, "Auxiliary MDS:\t%6u\n", num_mds );
     if ( param.flag_check == PARAM_SET )
	  (void) fprintf( stdout, "CRC/Reed-Solomon errors: " );
     num = 0u;
     while ( num_mds > num ) {
	  unsigned int numAux = 
	       GET_SCIA_LV0_STATE_AUX( fd, mds_info+num, num_mds-num, &aux );
	  if ( IS_ERR_STAT_FATAL ) {
	       free( mds_info );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_AUX" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV0_WR_H5_AUX( param, mds_info[num].state_index, 
				   numAux, aux );
	  } else if ( param.write_ascii == PARAM_SET ) {
 	       SCIA_LV0_WR_ASCII_AUX( param, mds_info[num].state_index, 
				      numAux, aux );
	  } else {
	       register unsigned int nr, total;

	       (void) fprintf( stdout, "%02d:", mds_info[num].state_id );
	       for ( total = nr = 0; nr < numAux; nr++ )
		    total += aux[nr].fep_hdr.crc_errs;
	       if ( total == 0 )
		    (void) fprintf( stdout, "-/" );
	       else
		    (void) fprintf( stdout, BOLDRED"%-d"RESET"/", total );
	       for ( total = nr = 0; nr < numAux; nr++ )
		    total += aux[nr].fep_hdr.rs_errs;
	       if  ( total == 0 )
		    (void) fprintf( stdout, "- " );
	       else
		    (void) fprintf( stdout, BOLDRED"%-d "RESET, total );
	  }

	  free( aux );
	  if ( IS_ERR_STAT_FATAL ) {
	       free( mds_info );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "MDS_AUX" );
	  }
	  num += numAux;
     }
     if ( num_mds > 0 ) free( mds_info );
     if ( param.flag_check == PARAM_SET ) (void) fprintf( stdout, "\n" );
/* 
 * process Detector source packets
 */
     num_mds = SCIA_LV0_SELECT_MDS( SCIA_DET_PACKET, param, num_info, info,
				    &mds_info );
     (void) fprintf( stdout, "Detector MDS:\t%6u\n", num_mds );
     if ( param.flag_check == PARAM_SET )
	  (void) fprintf( stdout, "CRC/Reed-Solomon errors: " );
     num = 0u;
     while ( num_mds > num ) {
	  unsigned int numDet = 
	       GET_SCIA_LV0_STATE_DET( param.chan_mask, fd, 
				       mds_info+num, num_mds-num, &det );
	  if ( IS_ERR_STAT_FATAL ) {
	       free( mds_info );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_DET" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV0_WR_H5_DET( param, mds_info[num].state_index, 
				   numDet, det );
	  } else if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV0_WR_ASCII_DET( param, mds_info[num].state_index, 
				      numDet, det );
	  } else {
	       register unsigned int nr, total;

	       (void) fprintf( stdout, "%02d:", mds_info[num].state_id );
	       for ( total = nr = 0; nr < numDet; nr++ )
		    total += det[nr].fep_hdr.crc_errs;
	       if ( total == 0 )
		    (void) fprintf( stdout, "-/" );
	       else
		    (void) fprintf( stdout, BOLDRED"%-d"RESET"/", total );
	       for ( total = nr = 0; nr < numDet; nr++ )
		    total += det[nr].fep_hdr.rs_errs;
	       if  ( total == 0 )
		    (void) fprintf( stdout, "- " );
	       else
		    (void) fprintf( stdout, BOLDRED"%-d "RESET, total );
	  }

	  SCIA_LV0_FREE_MDS_DET( numDet, det );
	  if ( IS_ERR_STAT_FATAL ) {
	       free( mds_info );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "MDS_DET" );
	  }
	  num += numDet;
     }
     if ( num_mds > 0 ) free( mds_info );
     if ( param.flag_check == PARAM_SET ) (void) fprintf( stdout, "\n" );
/* 
 * process PMD source packets
 */
     num_mds = SCIA_LV0_SELECT_MDS( SCIA_PMD_PACKET, param, num_info, info,
				    &mds_info );
     (void) fprintf( stdout, "PMD MDS:\t%6u\n", num_mds );
     if ( param.flag_check == PARAM_SET )
	  (void) fprintf( stdout, "CRC/Reed-Solomon errors: " );
     num = 0u;
     while ( num_mds > num ) {
	  unsigned int numPMD = 
	       GET_SCIA_LV0_STATE_PMD( fd, mds_info+num, num_mds-num, &pmd );
	  if ( IS_ERR_STAT_FATAL ) {
	       free( mds_info );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_PMD" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV0_WR_H5_PMD( param, mds_info[num].state_index, 
				   numPMD, pmd );
	  } else if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV0_WR_ASCII_PMD( param, mds_info[num].state_index, 
				      numPMD, pmd );
	  } else {
	       register unsigned int nr, total;

	       (void) fprintf( stdout, "%02d:", mds_info[num].state_id );
	       for ( total = nr = 0; nr < numPMD; nr++ )
		    total += pmd[nr].fep_hdr.crc_errs;
	       if ( total == 0 )
		    (void) fprintf( stdout, "-/" );
	       else
		    (void) fprintf( stdout, BOLDRED"%-d"RESET"/", total );
	       for ( total = nr = 0; nr < numPMD; nr++ )
		    total += pmd[nr].fep_hdr.rs_errs;
	       if  ( total == 0 )
		    (void) fprintf( stdout, "- " );
	       else
		    (void) fprintf( stdout, BOLDRED"%-d "RESET, total );
	  }
	  free( pmd );
	  if ( IS_ERR_STAT_FATAL ) {
	       free( mds_info );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "MDS_PMD" );
	  }
	  num += numPMD;
     }
     if ( num_mds > 0 ) free( mds_info );
     if ( param.flag_check == PARAM_SET ) (void) fprintf( stdout, "\n" );
/*
 * when an error has occurred we jump to here:
 */
 done:
/*
 * close input file
 */
     if ( fd != NULL ) (void) fclose( fd );
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
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, param.hdf5_name );
     }
/*
 * free allocated memory
 */
     if ( dsd != NULL ) free( dsd );
     if ( info != NULL ) free( info );
/*
 * display error messages?
 */
     if ( param.flag_silent == PARAM_UNSET ) NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL ) 
	  return NADC_ERR_FATAL;
     else
	  return NADC_ERR_NONE;
}
