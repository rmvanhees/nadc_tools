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

.IDENTifer   SCIA_OL2
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 (offline product)
.LANGUAGE    ANSI C
.PURPOSE     read Envisat SCIAMACHY level 2 offline products, extract data, 
             and write in a flexible binary format (HDF5) or dump, in 
	     human readable form, the contents of each PDS data set to a 
	     separate ASCII file
.INPUT/OUTPUT
  call as
            scia_ol2 [options] <input-file>

.RETURNS     non-negative on success, negative on failure
.COMMENTS    None
.ENVIRONment None
.VERSION      1.6   10-Jan-2011	updated to Issue 3/L, RvH
              1.5   19-Jun-2009	remove non-archived file from database, RvH
              1.4   20-Jun-2008	removed HDF4 support, RvH
              1.3   01-Apr-2003	added optional HDF4 output, RvH
              1.2   25-Mar-2003	write software version to HDF5 file, RvH
              1.2   22-Jan-2003	added write routines for MDS records, RvH
              1.1   16-Aug-2002	updated to ENV-ID-DLR-SCI-2200-4, RvH
              1.0   26-Apr-2002	created by R. M. van Hees 
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
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

/*+++++ Global Variables +++++*/
/* 
 * Most routines to read SCIAMACHY data can allocate memory internally
 * However IDL requires the use of their own memory allocation routines
 */
bool Use_Extern_Alloc = FALSE;

/*+++++ Local variables +++++*/
#define MAX_NFIT_SPECIES  16
#define MAX_LFIT_SPECIES  14
#define MAX_OFIT_SPECIES  14

static
const char *nfit_name[MAX_NFIT_SPECIES] = {
     "NAD_UV0_O3", "NAD_UV1_NO2", "NAD_UV2_O3", "NAD_UV3_BRO", "NAD_UV4_H2CO", 
     "NAD_UV5_SO2", "NAD_UV6_OCLO", "NAD_UV7_SO2", "NAD_UV8_H2O", 
     "NAD_UV9_SPARE",  
     "NAD_IR0_H2O", "NAD_IR1_CH4", "NAD_IR2_N2O", "NAD_IR3_CO", 
     "NAD_IR4_CO2", "NAD_IR5_SPARE",
};

static
const char *lfit_name[MAX_LFIT_SPECIES] = {
     "LIM_PTH", "LIM_UV0_O3", "LIM_UV1_NO2", "LIM_UV2_O3", "LIM_UV3_BRO",
     "LIM_UV4_H2CO", "LIM_UV5_SO2", "LIM_UV6_OCLO", "LIM_UV7_SPARE",
     "LIM_IR0_H2O", "LIM_IR1_CH4", "LIM_IR2_N2O", "LIM_IR3_CO",
     "LIM_IR4_SPARE"
};

static
const char *ofit_name[MAX_OFIT_SPECIES] = {
     "OCC_PTH", "OCC_UV0_O3", "OCC_UV1_NO2", "OCC_UV2_O3", "OCC_UV3_BRO",
     "OCC_UV4_H2CO", "OCC_UV5_SO2", "OCC_UV6_OCLO", "OCC_UV7_SPARE",
     "OCC_IR0_H2O", "OCC_IR1_CH4", "OCC_IR2_N2O", "OCC_IR3_CO",
     "OCC_IR4_SPARE"
};

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include <_connect_nadc_db.inc>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int main( int argc, char *argv[] )
     /*@globals  errno, stderr, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, stderr, nadc_stat, nadc_err_stack@*/
{
     register unsigned short n_nfit, n_lfit, n_ofit;

     unsigned int nr_state, num_dsd, num_dsr, num_geo;

     FILE  *fp = NULL;
#ifdef _WITH_SQL
     PGconn *conn = NULL;
#endif
     struct param_record param;
     struct mph_envi     mph;
     struct sph_sci_ol   sph;
     struct dsd_envi     *dsd = NULL;
     struct sqads_sci_ol *sqads;
     struct lads_scia    *lads;
     struct state2_scia  *state;
     struct ngeo_scia    *ngeo = NULL;
     struct lgeo_scia    *lgeo;
     struct cld_sci_ol   *cld;
     struct nfit_scia    *nfit;
     struct lfit_scia    *lfit, *ofit;
     struct lcld_scia   *lcld;
/*
 * initialization of command-line parameters
 */
     SCIA_SET_PARAM( argc, argv, SCIA_LEVEL_2, &param );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_GOTO_ERROR( NADC_ERR_PARAM, "" );
/*
 * check if we have to display version and exit
 */
     if ( param.flag_version == PARAM_SET ) {
	  SCIA_SHOW_VERSION( stdout, "scia_ol2" );
	  exit( EXIT_SUCCESS );
     }
/*
 * dump command-line parameters
 */
     if ( param.flag_show == PARAM_SET ) {
          SCIA_SHOW_PARAM( SCIA_LEVEL_2, param );
          exit( EXIT_SUCCESS );
     }
/*
 * connect to PostgreSQL database
 */
#ifdef _WITH_SQL
     if ( param.write_sql == PARAM_SET ) {
	  CONNECT_NADC_DB( &conn, "scia" );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( NADC_ERR_SQL, "PSQL" );
	  if ( param.flag_sql_remove == PARAM_SET 
	       || param.flag_sql_replace == PARAM_SET )
	       SCIA_OL2_DEL_ENTRY( conn, param.flag_verbose, param.infile );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_SQL, "PSQL(remove)" );
	  if ( param.flag_sql_remove == PARAM_SET ) goto done;
     }
#endif
/*
 * open input-file
 */
     if ( (fp = fopen( param.infile, "r" )) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE, param.infile );
/*
 * create output HDF5 file
 */
     if ( param.write_hdf5 == PARAM_SET ) {
	  param.hdf_file_id = SCIA_CRE_H5_FILE( SCIA_LEVEL_2, &param );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_HDF_CRE, "HDF5 base" );
	  SCIA_WR_H5_VERSION( param.hdf_file_id );
/*
 * create for data structures for SCIAMACHY level 1b data
 */
	  CRE_SCIA_OL2_H5_STRUCTS( param );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_HDF_CRE, "STRUCTS" );
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
	  ENVI_WR_ASCII_MPH( param, &mph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "MPH" );
     }
     if ( param.write_hdf5 == PARAM_SET ) {
	  SCIA_WR_H5_MPH( param, &mph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "MPH" );
     }
/*
 * -------------------------
 * read Specific Product Header
 */
     SCIA_OL2_RD_SPH( fp, mph, &sph );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "SPH" );
     if ( param.write_ascii == PARAM_SET ) {
	  SCIA_OL2_WR_ASCII_SPH( param, &sph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SPH" );
     }
     if ( param.write_hdf5 == PARAM_SET ) {
	  SCIA_OL2_WR_H5_SPH( param, &sph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "SPH" );
     }
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
     if ( param.write_meta == PARAM_SET ) goto done;
#ifdef _WITH_SQL
     if ( param.write_sql == PARAM_SET ) {
	  const char dsd_name[] = "LEVEL_1B_PRODUCT";

	  unsigned int indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, dsd_name );
	  SCIA_OL2_WR_SQL_META( conn, param.flag_verbose, param.infile, 
				dsd[indx_dsd].flname, &mph, &sph );
	  if ( IS_ERR_STAT_WARN ) goto done;
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_SQL, "SQL_META" );
     }
#endif
/*
 * -------------------------
 * read/write Summary of Quality Flags per State records
 */
     num_dsr = SCIA_OL2_RD_SQADS( fp, num_dsd, dsd, &sqads );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "SQADS" );
     if ( num_dsr > 0 ) {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_OL2_WR_ASCII_SQADS( param, num_dsr, sqads );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "SQADS" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_OL2_WR_H5_SQADS( param, num_dsr, sqads );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "SQADS" );
	  }
	  free( sqads );
     }
/*
 * -------------------------
 * read/write Geolocation of the States
 */
     num_dsr = SCIA_RD_LADS( fp, num_dsd, dsd, &lads );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "LADS" );
     if ( num_dsr > 0 ) {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_WR_ASCII_LADS( param, num_dsr, lads );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "LADS" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_WR_H5_LADS( param, num_dsr, lads );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "LADS" );
	  }
	  free( lads );
     }
/*
 * -------------------------
 * read/write Static Parameters
 */
/*
 * -------------------------
 * read/write States of the Product
 */
     nr_state = SCIA_LV2_RD_STATE( fp, num_dsd, dsd, &state );
     if ( IS_ERR_STAT_FATAL || nr_state  ==  0 )
          NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "STATE" );

     if ( param.write_ascii == PARAM_SET ) {
	  SCIA_LV2_WR_ASCII_STATE( param, nr_state, state );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "STATE" );
     }
     if ( param.write_hdf5 == PARAM_SET ) {
	  SCIA_LV2_WR_H5_STATE( param, nr_state, state );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "STATE" );
     }
#ifdef _WITH_SQL
     if ( param.write_sql == PARAM_SET ) {
	  SCIA_OL2_MATCH_STATE( conn, param.flag_verbose, 
				&mph, nr_state, state );
	  if ( IS_ERR_STAT_FATAL )
               NADC_GOTO_ERROR( NADC_ERR_SQL, "SQL_STATE" );
     }
#endif
     free( state );
/*
 * -------------------------
 * read/write Nadir Geolocation
 */
     num_geo = SCIA_OL2_RD_NGEO( fp, num_dsd, dsd, &ngeo );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "NGEO" );
     if ( num_geo > 0 ) {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_OL2_WR_ASCII_NGEO( param, num_geo, ngeo );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "NGEO" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_OL2_WR_H5_NGEO( param, num_geo, ngeo );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "NGEO" );
	  }
     }
/*
 * -------------------------
 * read/write Limb Geolocation
 */
     num_dsr = SCIA_OL2_RD_LGEO( fp, num_dsd, dsd, &lgeo );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "LGEO" );
     if ( num_dsr > 0 ) {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_OL2_WR_ASCII_LGEO( param, num_dsr, lgeo );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "LGEO" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_OL2_WR_H5_LGEO( param, num_dsr, lgeo );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "LGEO" );
	  }
	  free( lgeo );
     }
/*
 * -------------------------
 * read/write Clouds and Aerosol data
 */
     if ( param.write_cld == PARAM_SET ) {
	  num_dsr = SCIA_OL2_RD_CLD( fp, num_dsd, dsd, &cld );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "CLD" );
	  if ( num_dsr > 0 ) {
	       if ( param.write_ascii == PARAM_SET ) {
                    SCIA_OL2_WR_ASCII_CLD( param, num_dsr, cld );
                    if ( IS_ERR_STAT_FATAL )
                         NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "CLD" );
               }
               if ( param.write_hdf5 == PARAM_SET ) {
                    SCIA_OL2_WR_H5_CLD( param, num_dsr, cld );
                    if ( IS_ERR_STAT_FATAL )
                         NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "CLD" );
               }
#ifdef _WITH_SQL
	       if ( param.write_sql == PARAM_SET ) {
#ifdef _KNMI_SQL
		    SCIA_OL2_WR_SQL_CLD( conn, param.infile, num_dsr, cld );
#else
		    SCIA_OL2_WR_SQL_CLD( conn, param.flag_verbose, 
					 param.infile, 
					 num_dsr, ngeo, cld );
#endif
		    if ( IS_ERR_STAT_FATAL )
			 NADC_GOTO_ERROR( NADC_ERR_SQL, "SQL_CLD" );
	       }
#endif
               free( cld );
	  }
     }
/*
 * -------------------------
 * read/write Nadir fitting window application data
 */
     if ( param.write_nadir == PARAM_SET ) {
	  for ( n_nfit = 0; n_nfit < MAX_NFIT_SPECIES; n_nfit++ ) {
	       num_dsr = SCIA_OL2_RD_NFIT( fp, nfit_name[n_nfit], 
					   num_dsd, dsd, &nfit );
	       if ( param.flag_silent == PARAM_UNSET 
		    && param.write_sql == PARAM_UNSET ) 
		    (void) fprintf( stdout, "%s:\t%6u\n", 
				    nfit_name[n_nfit] , num_dsr );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_GOTO_ERROR( NADC_ERR_PDS_RD, 
				     nfit_name[n_nfit] );
	       if ( num_dsr > 0 ) {
		    if ( param.write_ascii == PARAM_SET ) {
			 SCIA_OL2_WR_ASCII_NFIT( nfit_name[n_nfit], param, 
						 num_dsr, nfit );
			 if ( IS_ERR_STAT_FATAL )
			      NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "NFIT" );
		    }
		    if ( param.write_hdf5 == PARAM_SET ) {
			 SCIA_OL2_WR_H5_NFIT( nfit_name[n_nfit], param, 
					      num_dsr, nfit );
			 if ( IS_ERR_STAT_FATAL )
			      NADC_GOTO_ERROR( NADC_ERR_HDF_WR, 
					       nfit_name[n_nfit] );
		    }
#ifdef _WITH_SQL
		    if ( param.write_sql == PARAM_SET ) {
#ifdef _KNMI_SQL
			 SCIA_OL2_WR_SQL_NFIT( conn, param.infile,
					       nfit_name[n_nfit], 
					       num_dsr, nfit );

#else
			 SCIA_OL2_WR_SQL_NFIT( conn, param.flag_verbose, 
					       param.infile, nfit_name[n_nfit], 
					       num_geo, ngeo, num_dsr, nfit );
#endif
			 if ( IS_ERR_STAT_FATAL )
			      NADC_GOTO_ERROR( NADC_ERR_SQL, "SQL_NFIT" );
		    }
#endif
		    free( nfit );
	       }
	  }
     }
/*
 * -------------------------
 * read/write Limb fitting window application data
 */
     if ( param.write_limb == PARAM_SET ) {
	  for ( n_lfit = 0; n_lfit < MAX_LFIT_SPECIES; n_lfit++ ) {
	       num_dsr = SCIA_OL2_RD_LFIT( fp, lfit_name[n_lfit],
					   num_dsd, dsd, &lfit );
	       if ( param.flag_silent == PARAM_UNSET 
		    && param.write_sql == PARAM_UNSET ) 
		    (void) fprintf( stdout, "%s:\t%6u\n", 
				    lfit_name[n_lfit], num_dsr );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_PDS_RD, 
				     lfit_name[n_lfit] );
	       if ( num_dsr > 0 ) {
		    if ( param.write_ascii == PARAM_SET ) {
			 SCIA_OL2_WR_ASCII_LFIT( lfit_name[n_lfit], param, 
						 num_dsr, lfit );
			 if ( IS_ERR_STAT_FATAL )
			      NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "LFIT" );
		    }
		    if ( param.write_hdf5 == PARAM_SET ) {
			 SCIA_OL2_WR_H5_LFIT( lfit_name[n_lfit], param, 
					      num_dsr, lfit );
			 if ( IS_ERR_STAT_FATAL )
			      NADC_GOTO_ERROR( NADC_ERR_HDF_WR, 
					       lfit_name[n_lfit] );
		    }
		    free( lfit );
	       }
	  }
     }
/*
 * -------------------------
 * read/write Occultation fitting window application data
 */
     if ( param.write_occ != PARAM_SET ) {
	  for ( n_ofit = 0; n_ofit < MAX_OFIT_SPECIES; n_ofit++ ) {
	       num_dsr = SCIA_OL2_RD_LFIT( fp, ofit_name[n_ofit],
					   num_dsd, dsd, &ofit );
	       if ( param.flag_silent == PARAM_UNSET
		    && param.write_sql == PARAM_UNSET ) 
		    (void) fprintf( stdout, "%s:\t%6u\n", 
				    ofit_name[n_ofit], num_dsr );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_PDS_RD, 
				     ofit_name[n_ofit] );
	       if ( num_dsr > 0 ) {
		    if ( param.write_ascii == PARAM_SET ) {
			 SCIA_OL2_WR_ASCII_LFIT( ofit_name[n_ofit], param, 
						 num_dsr, ofit );
			 if ( IS_ERR_STAT_FATAL )
			      NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "OFIT" );
		    }
		    if ( param.write_hdf5 == PARAM_SET ) {
			 SCIA_OL2_WR_H5_LFIT( ofit_name[n_ofit], param, 
					      num_dsr, ofit );
			 if ( IS_ERR_STAT_FATAL )
			      NADC_GOTO_ERROR( NADC_ERR_HDF_WR, 
					       ofit_name[n_ofit] );
		    }
		    free( ofit );
	       }
	  }
     }
/*
 * -------------------------
 * read/write Limb Clouds data sets
 */
     num_dsr = SCIA_OL2_RD_LCLD( fp, num_dsd, dsd, &lcld );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "LCLD" );
     if ( num_dsr > 0 ) {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_OL2_WR_ASCII_LCLD( param, num_dsr, lcld );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_FILE_WR, "LCLD" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_OL2_WR_H5_LCLD( param, num_dsr, lcld );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "LCLD" );
	  }
	  free( lcld );
     }
/*
 * -------------------------
 * read/write Ozone profile from Nadir measurements (TBD)
 */
/*
 * when an error has occurred we jump to here:
 */
 done:
/*
 * close input file
 */
     if ( fp != NULL ) (void) fclose( fp );
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
	       NADC_GOTO_ERROR( NADC_ERR_HDF_FILE, param.hdf5_name );
     }
/*
 * free allocated memory
 */
     if ( dsd != NULL ) free( dsd );
     if ( ngeo != NULL ) free( ngeo );
/*
 * display error messages?
 */
     if ( param.flag_silent == PARAM_UNSET ) NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL ) 
	  return NADC_ERR_FATAL;
     else
	  return NADC_ERR_NONE;
}
