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

.IDENTifer   SCIA_NL2
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 (NRT)
.LANGUAGE    ANSI C
.PURPOSE     read Envisat SCIAMACHY level 2 NRT products, extract data, 
             and write in a flexible binary format (HDF5) or dump, in 
	     human readable form, the contents of each PDS data set to a 
	     separate ASCII file
.INPUT/OUTPUT
  call as
            scia_nl2 [options] <input-file>

.RETURNS     non-negative on success, negative on failure
.COMMENTS    None
.ENVIRONment None
.VERSION      2.6   20-Jun-2008	removed HDF4 support, RvH
              2.5   25-Mar-2003	write software version to HDF4 file, RvH
              2.4   25-Mar-2003	write software version to HDF5 file, RvH
              2.3   14-Aug-2002	bug: tried to release unallocated memory, RvH
              2.2   02-Aug-2002	added "--info" option, RvH
              2.1   02-Aug-2002	added MDS selection, RvH 
              2.0   15-Nov-2001	moved to new Error handling routines, RvH 
              1.3   19-Sep-2001 added optional HDF4 output, RvH
              1.2   05-Sep-2001 compiles without HDF5 library, RvH
              1.1   17-Apr-2001 use Create_HDF5_NADC_FILE, RvH
              1.0   12-Aug-1999 created by R. M. van Hees
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
#define _SCIA_LEVEL_2
#include <defs_nadc.h>

/*+++++ Global Variables +++++*/
/* 
 * Most routines to read SCIAMACHY data can allocate memory internally
 * However IDL requires the use of their own memory allocation routines
 */
bool Use_Extern_Alloc = FALSE;

/*+++++ Local variables +++++*/
static
const char *doas_name[MAX_DOAS_SPECIES] = { 
     "DOAS_0_O3", "DOAS_1_NO2", "DOAS_1_H2O","DOAS_1_O3", "DOAS_2_BRO",
     "DOAS_2_O3_L", "DOAS_2_O3_H", "DOAS_2_NO2", "DOAS_2_OCLO", "DOAS_3_OCLO",
     "DOAS_3_NO2", "DOAS_3_O4", "DOAS_4_SO2", "DOAS_4_O3", "DOAS_5_HCHO", 
     "DOAS_5_BRO", "DOAS_5_O3_L", "DOAS_5_O3_H", "DOAS_5_NO2", "DOAS_5_O4",
     "DOAS_SPARE_1", "DOAS_SPARE_2"
};

static
const char *bias_name[MAX_BIAS_SPECIES] = { 
     "BIAS_0_H2O", "BIAS_0_CO2", "BIAS_1_N2O", "BIAS_1_H2O", "BIAS_1_CH4",
     "BIAS_2_CO", "BIAS_2_H2O", "BIAS_2_CH4", "BIAS_SPARE_1", "BIAS_SPARE_2"
};

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int main( int argc, char *argv[] )
     /*@globals  errno, stderr, nadc_stat, nadc_err_stack,
        doas_name, bias_name;@*/
     /*@modifies errno, stderr, nadc_stat, nadc_err_stack@*/
{
     const char prognm[] = "scia_nl2";

     register unsigned short n_bias, n_doas;
     register unsigned int   nr;

     unsigned int nr_state, num_dsd, num_dsr;

     FILE         *fp = NULL;

     struct param_record param;
     struct mph_envi    mph;
     struct sph2_scia   sph;
     struct dsd_envi    *dsd = NULL;
     struct sqads2_scia *sqads;
     struct lads_scia   *lads;
     struct state2_scia *state;
     struct geo_scia    *geo;
     struct cld_scia    *cld;
     struct doas_scia   *doas;
     struct bias_scia   *bias;
/*
 * initialization of command-line parameters
 */
     NADC_SET_PARAM( argc, argv, SCIA_LEVEL_2, &param );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, "" );
/*
 * check if we have to display version and exit
 */
     if ( param.flag_version == PARAM_SET ) {
	  SCIAshow_Version( stdout, prognm );
	  exit( EXIT_SUCCESS );
     }
/*
 * dump command-line parameters
 */
     if ( param.flag_show == PARAM_SET ) {
          NADC_SHOW_PARAM( SCIA_LEVEL_2, param );
          exit( EXIT_SUCCESS );
     }
/*
 * open input-file
 */
     if ( (fp = fopen( param.infile, "r" )) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, param.infile );
/*
 * create output HDF5 file
 */
     if ( param.write_hdf5 == PARAM_SET ) {
	  Create_HDF5_NADC_FILE( SCIA_LEVEL_2, &param );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_CRE, "HDF5 base" );
	  SCIA_WR_H5_VERSION( param.hdf_file_id );
/*
 * create for data structures for SCIAMACHY level 1b data
 */
	  CRE_SCIA_LV2_H5_STRUCTS( param );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_CRE, "STRUCTS" );
     }
/*
 * -------------------------
 * read Main Product Header
 */
     ENVI_RD_MPH( fp, &mph );
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
/*
 * -------------------------
 * read Specific Product Header
 */
     SCIA_LV2_RD_SPH( fp, mph, &sph );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SPH" );
     if ( param.write_ascii == PARAM_SET ) {
	  SCIA_LV2_WR_ASCII_SPH( param, &sph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "SPH" );
     }
     if ( param.write_hdf5 == PARAM_SET ) {
	  SCIA_LV2_WR_H5_SPH( param, &sph );
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
     num_dsd = ENVI_RD_DSD( fp, mph, dsd );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "DSD" );
     if ( param.write_ascii == PARAM_SET ) {
	  ENVI_WR_ASCII_DSD( param, num_dsd, dsd );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "DSD" );
     }
/*
 * -------------------------
 * read/write Summary of Quality Flags per State records
 */
     num_dsr = SCIA_LV2_RD_SQADS( fp, num_dsd, dsd, &sqads );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SQADS" );
     if ( num_dsr > 0 ) {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV2_WR_ASCII_SQADS( param, num_dsr, sqads );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "SQADS" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV2_WR_H5_SQADS( param, num_dsr, sqads );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "SQADS" );
	  }
	  free( sqads );
     }
/*
 * -------------------------
 * read/write Geolocation of the States
 */
     num_dsr = SCIA_RD_LADS( fp, num_dsd, dsd, &lads );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "LADS" );
     if ( num_dsr > 0 ) {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_WR_ASCII_LADS( param, num_dsr, lads );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "LADS" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_WR_H5_LADS( param, num_dsr, lads );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "LADS" );
	  }
	  free( lads );
     }
/*
 * -------------------------
 * read/write States of the Product
 */
     nr_state = SCIA_LV2_RD_STATE( fp, num_dsd, dsd, &state );
     if ( IS_ERR_STAT_FATAL || nr_state  ==  0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "STATE" );

     if ( param.write_ascii == PARAM_SET ) {
	  SCIA_LV2_WR_ASCII_STATE( param, nr_state, state );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "STATE" );
     }
     if ( param.write_hdf5 == PARAM_SET ) {
	  SCIA_LV2_WR_H5_STATE( param, nr_state, state );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "STATE" );
     }
     free( state );
/*
 * -------------------------
 * read/write Geolocation Data Set
 */
     num_dsr = SCIA_LV2_RD_GEO( fp, num_dsd, dsd, &geo );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "GEO" );
     if ( num_dsr > 0 ) {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV2_WR_ASCII_GEO( param, num_dsr, geo );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "GEO" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV2_WR_H5_GEO( param, num_dsr, geo );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "GEO" );
	  }
	  free( geo );
     }
/*
 * -------------------------
 * read/write Clouds and Aerosol data sets
 */
     if ( param.write_cld == PARAM_SET ) {
	  num_dsr = SCIA_LV2_RD_CLD( fp, num_dsd, dsd, &cld );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "CLD" );
	  if ( num_dsr > 0 ) {
	       if ( param.write_ascii == PARAM_SET ) {
		    SCIA_LV2_WR_ASCII_CLD( param, num_dsr, cld );
		    if ( IS_ERR_STAT_FATAL )
			 NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, "CLD" );
	       }
	       if ( param.write_hdf5 == PARAM_SET ) {
		    SCIA_LV2_WR_H5_CLD( param, num_dsr, cld );
		    if ( IS_ERR_STAT_FATAL )
			 NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "CLD" );
	       }
	       for ( nr = 0; nr < num_dsr; nr++ ) {
		    if ( cld[nr].numpmd > 0u ) free( cld[nr].pmdcloudfrac );
	       }
	       free( cld );
	  }
     }
/*
 * -------------------------
 * read/write DOAS records
 */
     if ( param.write_doas == PARAM_SET ) {
	  for ( n_doas = 0; n_doas < MAX_DOAS_SPECIES; n_doas++ ) {
	       (void) fprintf( stdout, "%s:", doas_name[n_doas] );
	       num_dsr = SCIA_LV2_RD_DOAS( fp, doas_name[n_doas], 
					   num_dsd, dsd, &doas );
	       (void) fprintf( stdout, "\t%6u\n", num_dsr );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD,
				     doas_name[n_doas] );
	       if ( num_dsr > 0 ) {
		    if ( param.write_ascii == PARAM_SET ) {
			 SCIA_LV2_WR_ASCII_DOAS( doas_name[n_doas], param, 
						 num_dsr, doas );
			 if ( IS_ERR_STAT_FATAL )
			      NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, 
					       doas_name[n_doas] );
		    }
		    if ( param.write_hdf5 == PARAM_SET ) {
			 SCIA_LV2_WR_H5_DOAS( doas_name[n_doas], param, 
					      num_dsr, doas );
			 if ( IS_ERR_STAT_FATAL )
			      NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, 
					       doas_name[n_doas] );
		    }
		    for ( nr = 0; nr < num_dsr; nr++ ) {
			 if ( doas[nr].numfitp > 0u ) free( doas[nr].corrpar );
		    }
		    free( doas );
	       }
	  }
     }
/*
 * -------------------------
 * read/write BIAS records
 */
     if ( param.write_bias == PARAM_SET ) {
	  for ( n_bias = 0; n_bias < MAX_BIAS_SPECIES; n_bias++ ) {
	       (void) fprintf( stdout, "%s:", bias_name[n_bias] );
	       num_dsr = SCIA_LV2_RD_BIAS( fp, bias_name[n_bias], 
					   num_dsd, dsd, &bias );
	       (void) fprintf( stdout, "\t%6u\n", num_dsr );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, 
				     bias_name[n_bias] );
	       if ( num_dsr > 0 ) {
		    if ( param.write_ascii == PARAM_SET ) {
			 SCIA_LV2_WR_ASCII_BIAS( bias_name[n_bias], param, 
						 num_dsr, bias );
			 if ( IS_ERR_STAT_FATAL )
			      NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, 
					       bias_name[n_bias] );
		    }
		    if ( param.write_hdf5 == PARAM_SET ) {
			 SCIA_LV2_WR_H5_BIAS( bias_name[n_bias], param, 
					      num_dsr, bias );
			 if ( IS_ERR_STAT_FATAL )
			      NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, 
					       bias_name[n_bias] );
		    }
		    for ( nr = 0; nr < num_dsr; nr++ ) {
			 if ( bias[nr].numfitp > 0u ) free( bias[nr].corrpar );
		    }
		    free( bias );
	       }
	  }
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
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, param.hdf5_name );
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
