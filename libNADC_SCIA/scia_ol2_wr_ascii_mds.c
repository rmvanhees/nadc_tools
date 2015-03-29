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

.IDENTifer   SCIA_OL2_WR_MDS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA Off-Line level 2
.LANGUAGE    ANSI C
.PURPOSE     dump Measurement Data Sets in ASCII
.RETURNS     Nothing
.COMMENTS    contains SCIA_OL2_WR_ASCII_CLD, SCIA_OL2_WR_ASCII_NFIT, 
                SCIA_OL2_WR_ASCII_LFIT, SCIA_OL2_WR_ASCII_LCLD
.ENVIRONment None
.VERSION      1.2   22-Jan-2003	added LIM_CLOUDS MDS, RvH
              1.2   22-Jan-2003	added SCIA_OL2_WR_ASCII_LFIT, RvH
              1.1   16-Aug-2002	updated to ENV-ID-DLR-SCI-2200-4, RvH
              1.0   22-May-2002	Created by R. M. van Hees 
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

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer  SCIA_OL2_WR_ASCII_CLD
.PURPOSE    dump -- in ASCII Format -- the Cloud/Aerosol datasets
.INPUT/OUTPUT
  call as   SCIA_OL2_WR_ASCII_CLD( param, num_dsr, cld );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct cld_sci_ol *cld    : pointer to Cloud/Aerosol records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_OL2_WR_ASCII_CLD( struct param_record param, unsigned int num_dsr, 
			    const struct cld_sci_ol *cld )
{
     register unsigned int nd, nr;

     char  date_str[UTC_STRING_LENGTH];

     unsigned int ndim;

     FILE *outfl = CRE_ASCII_File( param.outfile, "cld" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of CLOUD_AEROSOL record
 */
     nadc_write_header( outfl, 0, param.infile,  
			 "Clouds and Aerosol Data set(s)" );
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 0;
	  (void) MJD_2_ASCII( cld[nd].mjd.days, cld[nd].mjd.secnd,
			      cld[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, ++nr, "starttime", date_str );
	  nadc_write_uint( outfl, ++nr, "dsrlen", cld[nd].dsrlen );
	  nadc_write_schar( outfl, ++nr, "quality", cld[nd].quality );
	  nadc_write_float( outfl, ++nr, "inttime", 
			     3, cld[nd].intg_time / 16.f );
	  nadc_write_float( outfl, ++nr, "surfpress", 4, cld[nd].surfpress );
	  nadc_write_float( outfl, ++nr, "cloudfrac", 4, cld[nd].cloudfrac );
	  nadc_write_float( outfl, ++nr, "errcldfrac", 4, cld[nd].errcldfrac);
	  nadc_write_ushort( outfl, ++nr, "numpmdpix", cld[nd].numpmdpix );
	  ndim = 2;
	  nadc_write_arr_ushort( outfl, ++nr, "fullfree", 
				  1, &ndim, cld[nd].fullfree );
	  nadc_write_float( outfl, ++nr, "toppress", 4, cld[nd].toppress );
	  nadc_write_float( outfl, ++nr, "errtoppress",
			     4, cld[nd].errtoppress );
	  nadc_write_float( outfl, ++nr, "cldoptdepth",
			     4, cld[nd].cldoptdepth );
	  nadc_write_float( outfl, ++nr, "errcldoptdep",
			     4, cld[nd].errcldoptdep );
	  nadc_write_ushort( outfl, ++nr, "cloudtype", cld[nd].cloudtype );
	  nadc_write_float( outfl, ++nr, "cloudbrdf", 4, cld[nd].cloudbrdf );
	  nadc_write_float( outfl, ++nr, "errcldbrdf", 4, cld[nd].errcldbrdf);
	  nadc_write_float( outfl, ++nr, "effsurfrefl",
			     4, cld[nd].effsurfrefl );
	  nadc_write_float( outfl, ++nr, "erreffsrefl", 
			     4, cld[nd].erreffsrefl );
	  nadc_write_ushort( outfl, ++nr, "cloudflag", cld[nd].cloudflag );
	  nadc_write_float( outfl, ++nr, "aai", 4, cld[nd].aai );
	  nadc_write_float( outfl, ++nr, "aaidiag", 4, cld[nd].aaidiag );
	  nadc_write_ushort( outfl, ++nr, "aaiflag", cld[nd].aaiflag );
	  nadc_write_ushort( outfl, ++nr, "numaeropars",
			      cld[nd].numaeropars );
	  ndim = cld[nd].numaeropars;
	  nadc_write_arr_float( outfl, ++nr, "aeropars",
				 1, &ndim, 3, cld[nd].aeropars );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_OL2_WR_ASCII_NFIT
.PURPOSE    dump -- in ASCII Format -- the Nadir Fitting Window datasets
.INPUT/OUTPUT
  call as   SCIA_OL2_WR_ASCII_NFIT( mds_name, param, num_dsr, nfit );
     input:
            char *mds_name            : name of Nadir Fitting Window
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct nfit_scia *nfit    : pointer to Fitting Window records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_OL2_WR_ASCII_NFIT( const char *mds_name, 
			     struct param_record param, 
			     unsigned int num_dsr, 
			     const struct nfit_scia *nfit )
{
     register unsigned int nd, nr;

     char  date_str[UTC_STRING_LENGTH];

     unsigned int ndim;

     FILE *outfl;

     outfl = CRE_ASCII_File( param.outfile, mds_name );
     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
     nadc_write_header( outfl, 0, param.infile, mds_name );
/*
 * write ASCII dump of NFIT record
 */
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 0;
	  (void) MJD_2_ASCII( nfit[nd].mjd.days, nfit[nd].mjd.secnd,
			      nfit[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, ++nr, "starttime", date_str );
	  nadc_write_uint( outfl, ++nr, "dsrlen", nfit[nd].dsrlen );
	  nadc_write_schar( outfl, ++nr, "quality", nfit[nd].quality );
	  nadc_write_float( outfl, ++nr, "inttime", 
			    3, nfit[nd].intg_time / 16.f );
	  ndim = nfit[nd].numvcd;
	  nadc_write_arr_float( outfl, ++nr, "vcd", 
				1, &ndim, 8, nfit[nd].vcd );
	  nadc_write_arr_float( outfl, ++nr, "vcd_err", 
				1, &ndim, 8, nfit[nd].errvcd );
	  nadc_write_ushort( outfl, ++nr, "vcd_flags", nfit[nd].vcdflag );
	  nadc_write_float( outfl, ++nr, "slant_col_den", 8, nfit[nd].esc );
	  nadc_write_float( outfl, ++nr, "err_slant_col", 8, nfit[nd].erresc );
	  nadc_write_ushort( outfl, ++nr, "numlinfitp", nfit[nd].num_fitp );
	  nadc_write_ushort( outfl, ++nr, "numnlinfitp", nfit[nd].num_nfitp );
	  ndim = nfit[nd].num_fitp;
	  nadc_write_arr_float( outfl, ++nr, "linpars", 
				1, &ndim, 8, nfit[nd].linpars );
	  nadc_write_arr_float( outfl, ++nr, "errlinpars", 
				1, &ndim, 8, nfit[nd].errlinpars );
	  ndim = (nfit[nd].num_fitp * (nfit[nd].num_fitp - 1)) / 2;
	  nadc_write_arr_float( outfl, ++nr, "lincorrm",
				1, &ndim, 8, nfit[nd].lincorrm );
	  ndim = nfit[nd].num_nfitp;
	  nadc_write_arr_float( outfl, ++nr, "nlinpars", 
				1, &ndim, 8, nfit[nd].nlinpars );
	  nadc_write_arr_float( outfl, ++nr, "errnlinpars", 
				1, &ndim, 8, nfit[nd].errnlinpars );
	  ndim = (nfit[nd].num_nfitp * (nfit[nd].num_nfitp - 1)) / 2;
	  nadc_write_arr_float( outfl, ++nr, "nlincorrm", 
				1, &ndim, 8, nfit[nd].nlincorrm );
	  nadc_write_float( outfl, ++nr, "rms", 6, nfit[nd].rms );
	  nadc_write_float( outfl, ++nr, "chi2", 6, nfit[nd].chi2 );
	  nadc_write_float( outfl, ++nr, "goodness", 6, nfit[nd].goodness );
	  nadc_write_ushort( outfl, ++nr, "numiter", nfit[nd].numiter );
	  nadc_write_ushort( outfl, ++nr, "fitflag", nfit[nd].fitflag );
	  nadc_write_float( outfl, ++nr, "amf_gr", 8, nfit[nd].amfgrd );
	  nadc_write_float( outfl, ++nr, "amf_gr_err", 8, nfit[nd].erramfgrd );
	  nadc_write_float( outfl, ++nr, "amf_cl", 8, nfit[nd].amfcld );
	  nadc_write_float( outfl, ++nr, "amf_cl_err", 8, nfit[nd].erramfcld );
	  nadc_write_ushort( outfl, ++nr, "amf_flags", nfit[nd].amfflag );
	  nadc_write_float( outfl, ++nr, "temperature", 
			    4, nfit[nd].temperature );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_OL2_WR_ASCII_LFIT
.PURPOSE    dump -- in ASCII Format -- the Limb Fitting Window datasets
.INPUT/OUTPUT
  call as   SCIA_OL2_WR_ASCII_LFIT( mds_name, param, num_dsr, lfit );
     input:
            char *mds_name            : name of Limb Fitting Window
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct lfit_scia *lfit    : pointer to Fitting Window records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_OL2_WR_ASCII_LFIT( const char *mds_name, 
			     struct param_record param, 
			     unsigned int num_dsr, 
			     const struct lfit_scia *lfit )
{
     register unsigned int nd, nr, ns, nss;

     char  date_str[UTC_STRING_LENGTH];

     signed char  *cbuff;
     unsigned int ndim, dims[2];

     float *rbuff;

     FILE  *outfl;

     outfl = CRE_ASCII_File( param.outfile, mds_name );
     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
     nadc_write_header( outfl, 0, param.infile, mds_name );
/*
 * write ASCII dump of LFIT record
 */
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 0;
	  (void) MJD_2_ASCII( lfit[nd].mjd.days, lfit[nd].mjd.secnd,
			      lfit[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, ++nr, "starttime", date_str );
	  nadc_write_uint( outfl, ++nr, "dsrlen", lfit[nd].dsrlen );
	  nadc_write_schar( outfl, ++nr, "quality", lfit[nd].quality );
	  nadc_write_uchar( outfl, ++nr, "method", lfit[nd].method );
	  nadc_write_float( outfl, ++nr, "inttime", 
			    3, lfit[nd].intg_time / 16.f );
	  nadc_write_float( outfl, ++nr, "refp", 8, lfit[nd].refp );
	  nadc_write_float( outfl, ++nr, "refh", 8, lfit[nd].refh );
	  nadc_write_uchar( outfl, ++nr, "refpsrc", lfit[nd].refpsrc );
	  nadc_write_uchar( outfl, ++nr, "num_rlevel", lfit[nd].num_rlevel );
	  nadc_write_uchar( outfl, ++nr, "num_mlevel", lfit[nd].num_mlevel );
	  nadc_write_uchar( outfl, ++nr, "num_species (n1)", 
			    lfit[nd].num_species );
	  nadc_write_uchar( outfl, ++nr, "num_closure (n2)", 
			    lfit[nd].num_closure );
	  nadc_write_uchar( outfl, ++nr, "num_other (n3)", 
			    lfit[nd].num_other );
	  nadc_write_uchar( outfl, ++nr, "num_scale (n4)", 
			    lfit[nd].num_scale );
	  ndim = (unsigned int) lfit[nd].num_rlevel;
	  nadc_write_arr_float( outfl, ++nr, "tangh", 
				1, &ndim, 6, lfit[nd].tangh );
	  nadc_write_arr_float( outfl, ++nr, "tangp", 
				1, &ndim, 6, lfit[nd].tangp );
	  nadc_write_arr_float( outfl, ++nr, "tangt", 
				1, &ndim, 6, lfit[nd].tangt );
/* mainrec */
	  ndim = (unsigned int) lfit[nd].num_species 
	       * (unsigned int) lfit[nd].num_rlevel;
	  rbuff = (float *) malloc( ndim * sizeof( float ));
	  if ( rbuff == NULL ) 
	       NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );
	  for ( ns = 0; ns < ndim; ns++ )
	       rbuff[ns] = lfit[nd].mainrec[ns].tangvmr;
	  nadc_write_arr_float( outfl, ++nr, "mainrec.tangvmr", 
				1, &ndim, 8, rbuff );
	  for ( ns = 0; ns < ndim; ns++ )
	       rbuff[ns] = lfit[nd].mainrec[ns].errtangvmr;
	  nadc_write_arr_float( outfl, ++nr, "mainrec.errtangvmr", 
				1, &ndim, 8, rbuff );
	  for ( ns = 0; ns < ndim; ns++ )
	       rbuff[ns] = lfit[nd].mainrec[ns].vertcol;
	  nadc_write_arr_float( outfl, ++nr, "mainrec.vertcol", 
				1, &ndim, 8, rbuff );
	  for ( ns = 0; ns < ndim; ns++ )
	       rbuff[ns] = lfit[nd].mainrec[ns].errvertcol;
	  nadc_write_arr_float( outfl, ++nr, "mainrec.errvertcol", 
				1, &ndim, 8, rbuff );
	  free( rbuff );
/* scaledrec */
	  ndim = (unsigned int) lfit[nd].num_scale 
	       * (unsigned int) lfit[nd].num_rlevel;
	  rbuff = (float *) malloc( ndim * sizeof( float ));
	  if ( rbuff == NULL ) 
	       NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );
	  for ( ns = 0; ns < ndim; ns++ )
	       rbuff[ns] = lfit[nd].scaledrec[ns].tangvmr;
	  nadc_write_arr_float( outfl, ++nr, "scaledrec.tangvmr", 
				1, &ndim, 8, rbuff );
	  for ( ns = 0; ns < ndim; ns++ )
	       rbuff[ns] = lfit[nd].scaledrec[ns].errtangvmr;
	  nadc_write_arr_float( outfl, nr, "scaledrec.errtangvmr", 
				1, &ndim, 8, rbuff );
	  for ( ns = 0; ns < ndim; ns++ )
	       rbuff[ns] = lfit[nd].scaledrec[ns].vertcol;
	  nadc_write_arr_float( outfl, nr, "scaledrec.vertcol", 
				1, &ndim, 8, rbuff );
	  for ( ns = 0; ns < ndim; ns++ )
	       rbuff[ns] = lfit[nd].scaledrec[ns].errvertcol;
	  nadc_write_arr_float( outfl, nr, "scaledrec.errvertcol", 
				1, &ndim, 8, rbuff );
	  free( rbuff );
/* mgrid */
	  ndim = (unsigned int) lfit[nd].num_rlevel;
	  cbuff = (signed char *) malloc( (size_t) ndim );
	  if ( cbuff == NULL ) 
	       NADC_RETURN_ERROR( NADC_ERR_ALLOC, "cbuff" );
	  rbuff = (float *) malloc( ndim * sizeof( float ));
	  if ( rbuff == NULL ) {
	       free( cbuff );
	       NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );
	  }
	  for ( ns = 0; ns < ndim; ns++ )
	       rbuff[ns] = lfit[nd].mgrid[ns].tangh;
	  nadc_write_arr_float( outfl, ++nr, "mgrid.tangh", 
				1, &ndim, 6, rbuff );
	  for ( ns = 0; ns < ndim; ns++ )
	       rbuff[ns] = lfit[nd].mgrid[ns].tangp;
	  nadc_write_arr_float( outfl, nr, "mgrid.tangp", 
				1, &ndim, 6, rbuff );
	  for ( ns = 0; ns < ndim; ns++ )
	       rbuff[ns] = lfit[nd].mgrid[ns].tangt;
	  nadc_write_arr_float( outfl, nr, "mgrid.tangt", 
				1, &ndim, 6, rbuff );
	  for ( ns = 0; ns < ndim; ns++ )
	       cbuff[ns] = (signed char) lfit[nd].mgrid[ns].num_win;
	  nadc_write_arr_schar( outfl, nr, "mgrid.num_win", 
				1, &ndim, cbuff );
	  for ( ns = 0; ns < ndim; ns++ )
	       rbuff[ns] = lfit[nd].mgrid[ns].win_limits[0];
	  nadc_write_arr_float( outfl, nr, "mgrid.winmin", 
				1, &ndim, 3, rbuff );
	  for ( ns = 0; ns < ndim; ns++ )
	       rbuff[ns] = lfit[nd].mgrid[ns].win_limits[1];
	  nadc_write_arr_float( outfl, nr, "mgrid.winmax", 
				1, &ndim, 3, rbuff );
	  free( cbuff );
	  free( rbuff );
/* state_vec */
	  if ( lfit[nd].stvec_size > 0 ) {
	       nadc_write_ushort(outfl, ++nr, "stvec_size", lfit[nd].stvec_size);
	       ndim = (unsigned int) lfit[nd].stvec_size;
	       rbuff = (float *) malloc( ndim * sizeof( float ));
	       if ( rbuff == NULL ) 
		    NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );
	       for ( ns = 0; ns < ndim; ns++ )
		    rbuff[ns] = lfit[nd].statevec[ns].value;
	       nadc_write_arr_float( outfl, nr, "statevec.value",
				     1, &ndim, 8, rbuff );
	       for ( ns = 0; ns < ndim; ns++ )
		    rbuff[ns] = lfit[nd].statevec[ns].error;
	       nadc_write_arr_float( outfl, nr, "statevec.error",
				     1, &ndim, 8, rbuff );
	       free( rbuff );

	       dims[1] = (unsigned int) lfit[nd].stvec_size;
	       dims[0] = 4;
	       cbuff = (signed char *) malloc( (size_t) dims[0] * dims[1] );
	       if ( cbuff == NULL ) 
		    NADC_RETURN_ERROR( NADC_ERR_ALLOC, "cbuff" );
	       for ( nss = ns = 0; ns < ndim; ns ++ ) {
		    (void) memcpy( &cbuff[nss], lfit[nd].statevec[ns].type, 4 );
		    nss += 4;
	       }
	       nadc_write_arr_schar(outfl, nr, "statevec.type", 2, dims, cbuff);
	       free( cbuff );
	  }
	  nadc_write_ushort( outfl, ++nr, "cmatrixsize", 
			     lfit[nd].cmatrixsize );
	  ndim = lfit[nd].cmatrixsize;
	  nadc_write_arr_float( outfl, ++nr, "corrmatrix", 
				1, &ndim, 8, lfit[nd].corrmatrix );
	  nadc_write_float( outfl, ++nr, "rms", 8, lfit[nd].rms );
	  nadc_write_float( outfl, ++nr, "chi2", 8, lfit[nd].chi2 );
	  nadc_write_float( outfl, ++nr, "goodness", 8, lfit[nd].goodness );
	  nadc_write_ushort( outfl, ++nr, "iteration", lfit[nd].numiter );
	  ndim = 2;
	  nadc_write_arr_ushort( outfl, ++nr, "summary", 
				 1, &ndim, lfit[nd].summary );
	  nadc_write_schar( outfl, ++nr, "criteria", lfit[nd].criteria );
	  nadc_write_ushort( outfl, ++nr, "ressize", lfit[nd].ressize );
	  ndim = lfit[nd].ressize;
	  nadc_write_arr_float( outfl, ++nr, "residuals", 
				1, &ndim, 8, lfit[nd].residuals );
	  nadc_write_ushort( outfl, ++nr, "num_adddiag", 
			      lfit[nd].num_adddiag );
	  ndim = lfit[nd].num_adddiag;
	  nadc_write_arr_float( outfl, ++nr, "adddiag", 
				1, &ndim, 8, lfit[nd].adddiag );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_OL2_WR_ASCII_LCLD
.PURPOSE    dump -- in ASCII Format -- the Limb Clouds datasets
.INPUT/OUTPUT
  call as   SCIA_OL2_WR_ASCII_LCLD( param, num_dsr, lcld );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct lcld_scia *lcld    : pointer to Limb Clouds records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_OL2_WR_ASCII_LCLD( struct param_record param, unsigned int num_dsr, 
			     const struct lcld_scia *lcld )
{
     register unsigned int nd, nr;

     char  date_str[UTC_STRING_LENGTH];

     unsigned int adim;

     FILE *outfl = CRE_ASCII_File( param.outfile, "lcld" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of LIM_CLOUDS record
 */
     nadc_write_header( outfl, 0, param.infile,  
			 "Limb Clouds Data set(s)" );
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 0;
	  (void) MJD_2_ASCII( lcld[nd].mjd.days, lcld[nd].mjd.secnd,
			      lcld[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, ++nr, "starttime", date_str );
	  nadc_write_uint( outfl, nr, "dsrlen", lcld[nd].dsrlen );
	  nadc_write_schar( outfl, nr, "quality", lcld[nd].quality );
	  nadc_write_float( outfl, nr, "inttime", 
			    3, lcld[nd].intg_time / 16.f );
	  nadc_write_uchar( outfl, nr, "diag_cloud_algo", 
			    lcld[nd].diag_cloud_algo );
	  nadc_write_uchar( outfl, nr, "flag_normal_water", 
			    lcld[nd].flag_normal_water );
	  nadc_write_float( outfl, nr, "max_value_cir", 
			    8, lcld[nd].max_value_cir );
	  nadc_write_float( outfl, nr, "hght_max_value_cir", 
			    8, lcld[nd].hght_max_value_cir );
	  nadc_write_uchar( outfl, nr, "flag_water_clouds", 
			    lcld[nd].flag_water_clouds );
	  nadc_write_uchar( outfl, nr, "flag_ice_clouds", 
			    lcld[nd].flag_ice_clouds );
	  nadc_write_float( outfl, nr, "max_value_cir_ice", 
			    8, lcld[nd].max_value_cir_ice );
	  nadc_write_float( outfl, nr, "hght_max_value_cir_ice", 
			    8, lcld[nd].hght_max_value_cir_ice );
	  nadc_write_uchar( outfl, nr, "hght_index_max_value_ice", 
			    lcld[nd].hght_index_max_value_ice );
	  nadc_write_uchar( outfl, nr, "flag_polar_strato_clouds", 
			    lcld[nd].flag_polar_strato_clouds );
	  nadc_write_float( outfl, nr, "max_value_cir_strato", 
			    8, lcld[nd].max_value_cir_strato );
	  nadc_write_float( outfl, nr, "hght_max_value_cir_strato", 
			    8, lcld[nd].hght_max_value_cir_strato );
	  nadc_write_uchar( outfl, nr, "hght_index_max_value_strato", 
			    lcld[nd].hght_index_max_value_strato );
	  nadc_write_uchar( outfl, nr, "flag_noctilucent_clouds", 
			    lcld[nd].flag_noctilucent_clouds );
	  nadc_write_float( outfl, nr, "hght_max_value_noctilucent", 
			    8, lcld[nd].hght_max_value_noctilucent );
	  nadc_write_uchar( outfl, nr, "hght_index_max_value_noctilucent;", 
			    lcld[nd].hght_index_max_value_noctilucent );
	  nadc_write_ushort( outfl, nr, "num_tangent_hghts", 
			     lcld[nd].num_tangent_hghts );
	  if ( lcld[nd].num_tangent_hghts > 0u ) {
	       adim = lcld[nd].num_tangent_hghts;
	       nadc_write_arr_float( outfl, ++nr, "tangent_hghts", 
				     1, &adim, 6, lcld[nd].tangent_hghts );
	  }
	  nadc_write_ushort( outfl, nr, "num_cir", lcld[nd].num_cir );
	  if ( (lcld[nd].num_tangent_hghts * lcld[nd].num_cir) > 0 ) {
	       adim = lcld[nd].num_tangent_hghts * lcld[nd].num_cir;
	       nadc_write_arr_float( outfl, ++nr, "cir", 
				     1, &adim, 6, lcld[nd].cir );
	  }
	  nadc_write_ushort( outfl, nr, "num_limb_para", 
			     lcld[nd].num_limb_para );
	  if ( lcld[nd].num_limb_para > 0u ) {
	       adim = lcld[nd].num_limb_para;
	       nadc_write_arr_float( outfl, ++nr, "limb_para", 
				     1, &adim, 6, lcld[nd].limb_para );
	  }
     }
     (void) fclose( outfl );
}

