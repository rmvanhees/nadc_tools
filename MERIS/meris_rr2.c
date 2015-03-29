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

.IDENTifer   meris_rr2
.AUTHOR      R.M. van Hees
.KEYWORDS    MERIS reduced resolution (level 2)
.LANGUAGE    ANSI C
.PURPOSE     read Envisat MERIS reduced resolution level 2 products, 
             extract data, and write in a flexible binary format (HDF5) 
             or dump, in human readable form, the contents of each PDS 
	     data set to a separate ASCII file
.INPUT/OUTPUT
  call as
            meris_rr2 [options] <input-file>

.RETURNS     non-negative on success, negative on failure
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0   22-Sep-2008	created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>

/*+++++ Local Headers +++++*/
#define _MERIS_COMMON
#include <nadc_meris.h>

/*+++++ Global Variables +++++*/
/* 
 * Most routines to read SCIAMACHY data can allocate memory internally
 * However IDL requires the use of their own memory allocation routines
 */
bool Use_Extern_Alloc = FALSE;

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int main( int argc, char *argv[] )
     /*@globals  errno, stderr, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, stderr, nadc_stat, nadc_err_stack@*/
{
     register int ni;

     unsigned int num_dsd, num_dsr;

     FILE  *fp = NULL;

     struct param_record     param;
     struct mph_envi         mph;
     struct sph_meris        sph;
     struct dsd_envi         *dsd = NULL;
     struct sqads2_meris     *sqads;
     struct sfgi_meris       sfgi;
     struct tie_meris        *tie;
     struct mds_rr2_13_meris *mds_13;
     struct mds_rr2_14_meris *mds_14;
     struct mds_rr2_15_meris *mds_15;
     struct mds_rr2_16_meris *mds_16;
     struct mds_rr2_17_meris *mds_17;
     struct mds_rr2_18_meris *mds_18;
     struct mds_rr2_19_meris *mds_19;
     struct mds_rr2_20_meris *mds_20;
/*
 * initialization of command-line parameters
 */
     MERIS_SET_PARAM( argc, argv, MERIS_LEVEL_2, &param );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_GOTO_ERROR( NADC_ERR_PARAM, "" );
/*
 * check if we have to display version and exit
 */
     if ( param.flag_version == PARAM_SET ) {
	  MERIS_SHOW_VERSION( stdout, "meris_rr2" );
	  exit( EXIT_SUCCESS );
     }
/*
 * dump command-line parameters
 */
     if ( param.flag_show == PARAM_SET ) {
          MERIS_SHOW_PARAM( MERIS_LEVEL_2, param );
          exit( EXIT_SUCCESS );
     }
/*
 * open input-file
 */
     if ( (fp = fopen( param.infile, "r" )) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE, param.infile );
/*
 * create output file
 */
     if ( param.write_hdf5 == PARAM_SET ) {
          param.hdf_file_id = MERIS_CRE_H5_FILE( MERIS_LEVEL_2, &param );
          if ( IS_ERR_STAT_FATAL )
               NADC_GOTO_ERROR( NADC_ERR_HDF_CRE, "HDF5 base" );
          MERIS_WR_H5_VERSION( param.hdf_file_id );
     }
/*
 * -------------------------
 * read Main Product Header
 */
     ENVI_RD_MPH( fp, &mph );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "MPH" );
     if ( param.write_ascii == PARAM_SET ) {
	  ENVI_WR_ASCII_MPH( param, &mph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "MPH" );
     }
/*      if ( param.write_hdf5 == PARAM_SET ) { */
/* 	  SCIA_WR_H5_MPH( param, &mph ); */
/* 	  if ( IS_ERR_STAT_FATAL ) */
/* 	       NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "MPH" ); */
/*      } */
/*
 * -------------------------
 * read Specific Product Header
 */
     MERIS_RD_SPH( fp, mph, &sph );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "SPH" );
     if ( param.write_ascii == PARAM_SET ) {
	  MERIS_WR_ASCII_SPH( param, &sph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SPH" );
     }
/*      if ( param.write_hdf5 == PARAM_SET ) { */
/* 	  SCIA_OL2_WR_H5_SPH( param, &sph ); */
/* 	  if ( IS_ERR_STAT_FATAL ) */
/* 	       NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "SPH" ); */
/*      } */
/*
 * -------------------------
 * read Data Set Descriptor records
 */
     dsd = (struct dsd_envi *)
	  malloc( (mph.num_dsd-1) * sizeof( struct dsd_envi ) );
     if ( dsd == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dsd" );
     num_dsd = ENVI_RD_DSD( fp, mph, dsd );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "DSD" );
     if ( param.write_ascii == PARAM_SET ) {
	  ENVI_WR_ASCII_DSD( param, num_dsd, dsd );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "DSD" );
     }
/*
 * -------------------------
 * read/write Summary of Quality Flags
 */
     num_dsr = MERIS_RR2_RD_SQADS( fp, num_dsd, dsd, &sqads );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "SQADS" );
     if ( num_dsr > 0 ) {
	  (void) fprintf( stderr, "num_dsr[SQADS] = %-u\n", num_dsr );
/*           if ( param.write_ascii == PARAM_SET ) { */
/*                MERIS_RR2_WR_ASCII_SQADS( param, num_dsr, sqads ); */
/*                if ( IS_ERR_STAT_FATAL ) */
/*                     NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SQADS" ); */
/*           } */
          free( sqads );
     }
/*
 * -------------------------
 * read/write Scaling Factors and General Info GADS
 */
     num_dsr = MERIS_RR2_RD_SFGI( fp, num_dsd, dsd, &sfgi );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "SFGI" );
     if ( num_dsr > 0 ) {
	  (void) fprintf( stderr, "num_dsr[SFGI] = %-u\n", num_dsr );
/*           if ( param.write_ascii == PARAM_SET ) { */
/*                MERIS_RR2_WR_ASCII_SFGI( param, num_dsr, sfgi ); */
/*                if ( IS_ERR_STAT_FATAL ) */
/*                     NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SFGI" ); */
/*           } */
     }
/*
 * -------------------------
 * read/write Tie point locations and auxiliary data (LADS)
 */
     num_dsr = MERIS_RD_TIE( fp, num_dsd, dsd, &tie );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "TIE" );
     if ( num_dsr > 0 ) {
	  (void) fprintf( stderr, "num_dsr[TIE] = %-u\n", num_dsr );
          if ( param.write_ascii == PARAM_SET ) {
               MERIS_WR_ASCII_TIE( param, num_dsr, tie );
               if ( IS_ERR_STAT_FATAL )
                    NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "TIE" );
          }
	  free( tie );
     }
/*
 * -------------------------
 * read/write RR-MDS 1 to 13
 */
     for ( ni = 1; ni <= 13; ni++ ) {
	  num_dsr = MERIS_RR2_RD_MDS_13( ni, fp, num_dsd, dsd, &mds_13 );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "MDS_13" );
	  if ( num_dsr > 0 ) {
	       (void) fprintf( stderr, "num_dsr[MDS(%-d)] = %-u\n", 
			       ni, num_dsr );
/*           if ( param.write_ascii == PARAM_SET ) { */
/*                MERIS_RR2_WR_ASCII_SFGI( param, num_dsr, sfgi ); */
/*                if ( IS_ERR_STAT_FATAL ) */
/*                     NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SFGI" ); */
/*           } */
	       free( mds_13 );
	  }
     }
/*
 * -------------------------
 * read/write RR-MDS 14
 */
     num_dsr = MERIS_RR2_RD_MDS_14( fp, num_dsd, dsd, &mds_14 );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "MDS_14" );
     if ( num_dsr > 0 ) {
	  (void) fprintf( stderr, "num_dsr[MDS(14)] = %-u\n", num_dsr );
/*           if ( param.write_ascii == PARAM_SET ) { */
/*                MERIS_RR2_WR_ASCII_SFGI( param, num_dsr, sfgi ); */
/*                if ( IS_ERR_STAT_FATAL ) */
/*                     NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SFGI" ); */
/*           } */
	  free( mds_14 );
     }
/*
 * -------------------------
 * read/write RR-MDS 15
 */
     num_dsr = MERIS_RR2_RD_MDS_15( fp, num_dsd, dsd, &mds_15 );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "MDS_15" );
     if ( num_dsr > 0 ) {
	  (void) fprintf( stderr, "num_dsr[MDS(15)] = %-u\n", num_dsr );
/*           if ( param.write_ascii == PARAM_SET ) { */
/*                MERIS_RR2_WR_ASCII_SFGI( param, num_dsr, sfgi ); */
/*                if ( IS_ERR_STAT_FATAL ) */
/*                     NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SFGI" ); */
/*           } */
	  free( mds_15 );
     }
/*
 * -------------------------
 * read/write RR-MDS 16
 */
     num_dsr = MERIS_RR2_RD_MDS_16( fp, num_dsd, dsd, &mds_16 );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "MDS_16" );
     if ( num_dsr > 0 ) {
	  (void) fprintf( stderr, "num_dsr[MDS(16)] = %-u\n", num_dsr );
/*           if ( param.write_ascii == PARAM_SET ) { */
/*                MERIS_RR2_WR_ASCII_SFGI( param, num_dsr, sfgi ); */
/*                if ( IS_ERR_STAT_FATAL ) */
/*                     NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SFGI" ); */
/*           } */
	  free( mds_16 );
     }
/*
 * -------------------------
 * read/write RR-MDS 17
 */
     num_dsr = MERIS_RR2_RD_MDS_17( fp, num_dsd, dsd, &mds_17 );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "MDS_17" );
     if ( num_dsr > 0 ) {
	  (void) fprintf( stderr, "num_dsr[MDS(17)] = %-u\n", num_dsr );
/*           if ( param.write_ascii == PARAM_SET ) { */
/*                MERIS_RR2_WR_ASCII_SFGI( param, num_dsr, sfgi ); */
/*                if ( IS_ERR_STAT_FATAL ) */
/*                     NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SFGI" ); */
/*           } */
	  free( mds_17 );
     }
/*
 * -------------------------
 * read/write RR-MDS 18
 */
     num_dsr = MERIS_RR2_RD_MDS_18( fp, num_dsd, dsd, &mds_18 );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "MDS_18" );
     if ( num_dsr > 0 ) {
	  (void) fprintf( stderr, "num_dsr[MDS(18)] = %-u\n", num_dsr );
/*           if ( param.write_ascii == PARAM_SET ) { */
/*                MERIS_RR2_WR_ASCII_SFGI( param, num_dsr, sfgi ); */
/*                if ( IS_ERR_STAT_FATAL ) */
/*                     NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SFGI" ); */
/*           } */
	  free( mds_18 );
     }
/*
 * -------------------------
 * read/write RR-MDS 19
 */
     num_dsr = MERIS_RR2_RD_MDS_19( fp, num_dsd, dsd, &mds_19 );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "MDS_19" );
     if ( num_dsr > 0 ) {
	  (void) fprintf( stderr, "num_dsr[MDS(19)] = %-u\n", num_dsr );
/*           if ( param.write_ascii == PARAM_SET ) { */
/*                MERIS_RR2_WR_ASCII_SFGI( param, num_dsr, sfgi ); */
/*                if ( IS_ERR_STAT_FATAL ) */
/*                     NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SFGI" ); */
/*           } */
	  free( mds_19 );
     }
/*
 * -------------------------
 * read/write RR-MDS 20
 */
     num_dsr = MERIS_RR2_RD_MDS_20( fp, num_dsd, dsd, &mds_20 );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "MDS_20" );
     if ( num_dsr > 0 ) {
	  (void) fprintf( stderr, "num_dsr[MDS(20)] = %-u\n", num_dsr );
/*           if ( param.write_ascii == PARAM_SET ) { */
/*                MERIS_RR2_WR_ASCII_SFGI( param, num_dsr, sfgi ); */
/*                if ( IS_ERR_STAT_FATAL ) */
/*                     NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SFGI" ); */
/*           } */
	  free( mds_20 );
     }
/*
 * when an error has occurred we jump to here:
 */
 done:
/*
 * close input file
 */
     if ( fp != NULL ) (void) fclose( fp );
/*
 * close HDF5 output file
 */
     if ( param.write_hdf5 == PARAM_SET ) {
	  if ( param.hdf_file_id >= 0 && H5Fclose( param.hdf_file_id ) < 0 )
	       NADC_GOTO_ERROR( NADC_ERR_HDF_FILE, param.hdf5_name );
     }
/*
 * free allocated memory
 */
     if ( dsd != NULL ) free( dsd );
/*
 * display error messages?
 */
     if ( param.flag_silent == PARAM_UNSET ) NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL ) 
	  return NADC_ERR_FATAL;
     else
	  return NADC_ERR_NONE;
}
