/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2003 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_PATCH_NL1
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1b (NRT)
.LANGUAGE    ANSI C
.PURPOSE     patch a level 1b dataset by modifying (key-) datasets
.INPUT/OUTPUT
  call as   
            scia_patch_nl1 [options] <input-file>

.RETURNS     non-negative on success, negative on failure
.COMMENTS    None
.ENVIRONment None
.VERSION     2.1     26-Jan-2006   use NADC_Info routines, RvH
             2.0     07-Dec-2005   many small bugfixes, 
                                   use the new routines to handle DSD write,
                                   bugfix file open/close, RvH
             1.0     25-Aug-2003   initial release by R. M. van Hees
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

/*+++++ Local Headers +++++*/
#define _SCIA_PATCH_1
#include <defs_nadc.h>
#include <defs_scia_cal.h>

/*+++++ Global Variables +++++*/
/* 
 * Most routines to read SCIAMACHY data can allocate memory internally
 * However IDL requires the use of their own memory allocation routines
 */
bool Use_Extern_Alloc = FALSE;

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int main( int argc, char *argv[] )
     /*@globals  errno, stderr, stdout, nadc_stat, nadc_err_stack, 
       Use_Extern_Alloc;@*/
     /*@modifies errno, stderr, stdout, nadc_stat, nadc_err_stack@*/
{
     const char prognm[] = "scia_patch_nl1";

     register unsigned int nr, ns;

     unsigned int num_dsd = 0;
     unsigned int num_state;
     unsigned int num_mds;

     FILE  *fp_in  = NULL;
     FILE  *fp_out = NULL;

     struct param_record param;

     struct mph_envi    mph;
     struct dsd_envi    *dsd = NULL;
     struct state1_scia *state;
     struct mds1_scia   *mds;

     struct keydata_rec *keydata = NULL;

#if __WORDSIZE == 64
     const unsigned long long clus_mask = ~0UL;
#else
     const unsigned long long clus_mask = ~0ULL;
#endif
/*
 * initialization of command-line parameters
 */
     NADC_SET_PARAM( argc, argv, SCIA_PATCH_1, &param );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, "NADC_INIT_PARAM" );
/*
 * check if we have to display version and exit
 */
     if ( param.flag_version == PARAM_SET ) {
	  SCIAshow_Version( stderr, prognm );
	  exit( EXIT_SUCCESS );
     }
/*
 * dump command-line parameters
 */
     if ( param.flag_show == PARAM_SET ) {
	  NADC_SHOW_PARAM( SCIA_PATCH_1, param );
	  exit( EXIT_SUCCESS );
     }
/*
 * open input-file
 */
     if ( (fp_in = fopen( param.infile, "rb" )) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, param.infile );
/*
 * open output-file
 */
     if ( (fp_out = fopen( param.outfile, "wb+" )) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, param.outfile );
/*
 * -------------------------
 * read Main Product Header
 */
     ENVI_RD_MPH( fp_in, &mph );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "MPH" );
/*
 * read, if required patched polarisation, radiance & Sun keydata
 */
     if ( (param.patch_scia & SCIA_PATCH_RAD) != USHRT_ZERO ) {
	  keydata = (struct keydata_rec *) 
	       malloc( sizeof( struct keydata_rec ) );
	  if ( keydata == NULL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "keydata" );
	  SCIA_H5_RD_KEYDATA( mph.product, keydata );
	  if ( IS_ERR_STAT_FATAL ) {
	       NADC_Err_Trace( stderr );
	       exit( EXIT_FAILURE );
	  }
     }
/*
 * -------------------------
 * copy the PDS headers: MPH and SPH
 */
     SCIA_LV1_PATCH_HDR( fp_in, fp_out );
/*
 * -------------------------
 * read Data Set Descriptor records
 */
     dsd = (struct dsd_envi *)
	  malloc( (mph.num_dsd-1) * sizeof( struct dsd_envi ) );
     if ( dsd == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "dsd" );
     num_dsd = ENVI_RD_DSD( fp_in, mph, dsd );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "DSD" );
     SCIA_LV1_WR_DSD_INIT( param, fp_out, num_dsd, dsd );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, "DSD" );
     SCIA_LV1_SET_NUM_ATTACH( param, fp_in, num_dsd, dsd );
/*
 * -------------------------
 * copy/patch all (global) annotation data sets
 */
     for ( nr = 0; nr < num_dsd; nr++ ) {
	  if ( param.flag_silent == PARAM_UNSET && dsd[nr].dsr_size > 0 ) 
	       NADC_Info_Proc( stderr, dsd[nr].name, dsd[nr].num_dsr );
	  if ( strcmp( dsd[nr].name, "SPECTRAL_BASE" ) == 0 ) {
	       SCIA_LV1_PATCH_BASE( param.patch_scia, num_dsd, dsd, 
				    fp_in, fp_out );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, dsd[nr].name );
	  } else if ( strcmp( dsd[nr].name, "LEAKAGE_CONSTANT" ) == 0 ) {
	       SCIA_LV1_PATCH_DARK( param.patch_scia, mph.abs_orbit,
				    num_dsd, dsd, fp_in, fp_out );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, dsd[nr].name );
	  } else if ( strcmp( dsd[nr].name, "PPG_ETALON" ) == 0 ) {
	       SCIA_LV1_PATCH_PPG( param.patch_scia, mph.abs_orbit,
				   num_dsd, dsd, fp_in, fp_out );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, dsd[nr].name );
	  } else if ( strcmp( dsd[nr].name, "RAD_SENS_NADIR" ) == 0 ) {
	       SCIA_LV1_PATCH_RSPN( keydata, num_dsd, dsd, fp_in, fp_out );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, dsd[nr].name );
	  } else if ( strcmp( dsd[nr].name, "RAD_SENS_LIMB" ) == 0 ) {
	       SCIA_LV1_PATCH_RSPL( keydata, num_dsd, dsd, fp_in, fp_out );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, dsd[nr].name );
	  } else if ( strcmp( dsd[nr].name, "POL_SENS_NADIR" ) == 0 ) {
	       SCIA_LV1_PATCH_PSPN( keydata, num_dsd, dsd, fp_in, fp_out );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, dsd[nr].name );
	  } else if ( strcmp( dsd[nr].name, "POL_SENS_LIMB" ) == 0 ) {
	       SCIA_LV1_PATCH_PSPL( keydata, num_dsd, dsd, fp_in, fp_out );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, dsd[nr].name );
	  } else if ( strcmp( dsd[nr].name, "SUN_REFERENCE" ) == 0 ) {
	       SCIA_LV1_PATCH_SRS( keydata, num_dsd, dsd, fp_in, fp_out );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, dsd[nr].name );
	  } else if ( strcmp( dsd[nr].name, "LEAKAGE_VARIABLE" ) != 0 ) {
	       SCIA_LV1_PATCH_ADS( param.patch_scia, dsd[nr], fp_in, fp_out );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_WR, dsd[nr].name );
	  }
	  if ( param.flag_silent == PARAM_UNSET && dsd[nr].dsr_size > 0 ) 
	       NADC_Info_Finish( stderr, 1, dsd[nr].num_dsr );
     }
/*
 * -------------------------
 * copy/patch Measurement Data Sets (Nadir)
 */
     num_state = SCIA_LV1_SELECT_MDS( SCIA_NADIR, param, 
				      fp_in, num_dsd, dsd, &state );
     if ( param.flag_silent == PARAM_UNSET )
	  NADC_Info_Proc( stderr, "MDS (nadir)", num_state );
     if ( num_state > 0u ) {
	  for ( ns = 0; ns < num_state; ns++ ) {
	       if ( param.flag_silent == PARAM_UNSET )
		    NADC_Info_Update( stderr, 2, ns );
	       num_mds = SCIA_LV1_RD_MDS( fp_in, clus_mask, state+ns, &mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, 
				     "SCIA_LV1_RD_MDS");
	       SCIA_LV1_PATCH_MDS( fp_in, param.patch_scia, state+ns, mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_CALIB, 
				     "SCIA_LV1_PATCH_MDS" );
	       SCIA_LV1_WR_MDS( fp_out, num_mds, mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, 
				     "SCIA_LV1_WR_MDS" );
	       SCIA_LV1_FREE_MDS( SCIA_NADIR, num_mds, mds );
	  }
	  free( state );
	  if ( param.flag_silent == PARAM_UNSET )
	       NADC_Info_Finish( stderr, 2, ns );
     } else if ( param.flag_silent == PARAM_UNSET )
	  NADC_Info_Finish( stderr, 2, num_state );
/*
 * -------------------------
 * copy/patch Measurement Data Sets (Limb)
 */
     num_state = SCIA_LV1_SELECT_MDS( SCIA_LIMB, param,
				      fp_in, num_dsd, dsd, &state );
     if ( param.flag_silent == PARAM_UNSET )
	  NADC_Info_Proc( stderr, "MDS (limb)", num_state );
     if ( num_state > 0u ) {
	  for ( ns = 0; ns < num_state; ns++ ) {
	       if ( param.flag_silent == PARAM_UNSET )
		    NADC_Info_Update( stderr, 2, ns );
	       num_mds = SCIA_LV1_RD_MDS( fp_in, clus_mask, state+ns, &mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, 
				     "SCIA_LV1_RD_MDS");
	       SCIA_LV1_PATCH_MDS( fp_in, param.patch_scia, state+ns, mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, 
				     "SCIA_LV1_PATCH_MDS");
	       SCIA_LV1_WR_MDS( fp_out, num_mds, mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, 
				     "SCIA_LV1_WR_MDS");
	       SCIA_LV1_FREE_MDS( SCIA_LIMB, num_mds, mds );
	  }
	  free( state );
	  if ( param.flag_silent == PARAM_UNSET )
	       NADC_Info_Finish( stderr, 2, ns );
     } else if ( param.flag_silent == PARAM_UNSET )
	  NADC_Info_Finish( stderr, 2, num_state );
/*
 * -------------------------
 * copy/patch Measurement Data Sets (Occultation)
 */
     num_state = SCIA_LV1_SELECT_MDS( SCIA_OCCULT, param, 
				fp_in, num_dsd, dsd, &state );
     if ( param.flag_silent == PARAM_UNSET )
	  NADC_Info_Proc( stderr, "MDS (occult)", num_state );
     if ( num_state > 0u ) {
	  for ( ns = 0; ns < num_state; ns++ ) {
	       if ( param.flag_silent == PARAM_UNSET )
		    NADC_Info_Update( stderr, 2, ns );
	       num_mds = SCIA_LV1_RD_MDS( fp_in, clus_mask, state+ns, &mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, 
				     "SCIA_LV1_RD_MDS");
	       SCIA_LV1_PATCH_MDS( fp_in, param.patch_scia, state+ns, mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, 
				     "SCIA_LV1_PATCH_MDS");
	       SCIA_LV1_WR_MDS( fp_out, num_mds, mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, 
				     "SCIA_LV1_WR_MDS");
	       SCIA_LV1_FREE_MDS( SCIA_OCCULT, num_mds, mds );
	  }
	  free( state );
	  if ( param.flag_silent == PARAM_UNSET )
	       NADC_Info_Finish( stderr, 2, ns );
     } else if ( param.flag_silent == PARAM_UNSET )
	  NADC_Info_Finish( stderr, 2, num_state );
/*
 * -------------------------
 * copy/patch Measurement Data Sets (Monitor)
 */
     num_state = SCIA_LV1_SELECT_MDS( SCIA_MONITOR, param,  
				fp_in, num_dsd, dsd, &state );
     if ( param.flag_silent == PARAM_UNSET )
	  NADC_Info_Proc( stderr, "MDS (monitor)", num_state );
     if ( num_state > 0u ) {
	  for ( ns = 0; ns < num_state; ns++ ) {
	       if ( param.flag_silent == PARAM_UNSET )
		    NADC_Info_Update( stderr, 2, ns );
	       num_mds = SCIA_LV1_RD_MDS( fp_in, clus_mask, state+ns, &mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, 
				     "SCIA_LV1_RD_MDS" );
	       SCIA_LV1_PATCH_MDS( fp_in, param.patch_scia, state+ns, mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, 
				     "SCIA_LV1_PATCH_MDS");
	       SCIA_LV1_WR_MDS( fp_out, num_mds, mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_WR, 
				     "SCIA_LV1_WR_MDS" );
	       SCIA_LV1_FREE_MDS( SCIA_MONITOR, num_mds, mds );
	  }
	  free( state );
	  if ( param.flag_silent == PARAM_UNSET )
	       NADC_Info_Finish( stderr, 2, ns );
     } else if ( param.flag_silent == PARAM_UNSET )
	  NADC_Info_Finish( stderr, 2, num_state );
/*
 * when an error has occurred we jump to here:
 */
 done:
/*
 * close original file and patched file
 */
     if ( fp_in != NULL ) {
	  if ( fp_out != NULL ) {
	       SCIA_LV1_WR_DSD_UPDATE( fp_in, fp_out );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_ERROR( prognm, NADC_ERR_FATAL, "LV1_WR_DSD_UPDATE" );
	  }
	  (void) fclose( fp_in );
     }
/*
 * free allocated memory
 */
     if ( dsd != NULL ) free( dsd );
/*
 * display error messages?
 */
     NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL ) 
	  return NADC_ERR_FATAL;
     else
	  return NADC_ERR_NONE;
}
