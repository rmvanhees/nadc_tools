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

.IDENTifer   meris_fr1
.AUTHOR      R.M. van Hees
.KEYWORDS    MERIS full resolution (level 1)
.LANGUAGE    ANSI C
.PURPOSE     read Envisat MERIS full resolution level 1 products, extract data,
             and write in a flexible binary format (HDF5) or dump, in 
	     human readable form, the contents of each PDS data set to a 
	     separate ASCII file
.INPUT/OUTPUT
  call as
            meris_fr1 [options] <input-file>

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
     unsigned int num_dsd;

     FILE  *fp = NULL;

     struct param_record param;
     struct mph_envi     mph;
     struct sph_meris    sph;
     struct dsd_envi     *dsd = NULL;
/*
 * initialization of command-line parameters
 */
     MERIS_SET_PARAM( argc, argv, MERIS_LEVEL_1, &param );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_GOTO_ERROR( NADC_ERR_PARAM, "" );
/*
 * check if we have to display version and exit
 */
     if ( param.flag_version == PARAM_SET ) {
	  MERIS_SHOW_VERSION( stdout, "meris_fr1" );
	  exit( EXIT_SUCCESS );
     }
/*
 * dump command-line parameters
 */
     if ( param.flag_show == PARAM_SET ) {
          MERIS_SHOW_PARAM( MERIS_LEVEL_1, param );
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
          param.hdf_file_id = MERIS_CRE_H5_FILE( MERIS_LEVEL_1, &param );
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
     if ( mph.tot_size != nadc_file_size( param.infile ) )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, "file size check failed" );
     if ( param.write_ascii == PARAM_SET ) {
	  ENVI_WR_ASCII_MPH(&mph);
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
	  ENVI_WR_ASCII_DSD(num_dsd, dsd);
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "DSD" );
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
