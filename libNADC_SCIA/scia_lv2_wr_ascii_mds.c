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

.IDENTifer   SCIA_LV2_WR_MDS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2
.LANGUAGE    ANSI C
.PURPOSE     dump Measurement Data Sets in ASCII
.RETURNS     Nothing
.COMMENTS    contains SCIA_LV2_WR_ASCII_CLD, SCIA_LV2_WR_ASCII_BIAS, 
                SCIA_LV2_WR_ASCII_DOAS
.ENVIRONment None
.VERSION     1.0     15-Sep-2001   Created by R. M. van Hees
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
.IDENTifer  SCIA_LV2_WR_ASCII_CLD
.PURPOSE    dump -- in ASCII Format -- the Cloud/Aerosol datasets
.INPUT/OUTPUT
  call as   SCIA_LV2_WR_ASCII_CLD( param, num_dsr, cld );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct cld_scia *cld      : pointer to Cloud/Aerosol records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV2_WR_ASCII_CLD( struct param_record param, unsigned int num_dsr,
			    const struct cld_scia *cld )
{
     register unsigned int nd, nr;

     char  date_str[UTC_STRING_LENGTH];

     unsigned int ndim;

     const char prognm[] = "SCIA_LV2_WR_ASCII_CLD";

     FILE *outfl = CRE_ASCII_File( param.outfile, "cld" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
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
	  nadc_write_ushort( outfl, ++nr, "numpmd", cld[nd].numpmd );
	  ndim = cld[nd].numpmd;
	  nadc_write_arr_float( outfl, ++nr, "PMDcloudfrac",
				 1, &ndim, 3, cld[nd].pmdcloudfrac );
	  nadc_write_float( outfl, ++nr, "cloudfrac", 3, cld[nd].cloudfrac );
	  nadc_write_uchar( outfl, ++nr, "quality cloudfrac",
			     cld[nd].quality_cld );
	  nadc_write_float( outfl, ++nr, "toppress", 4, cld[nd].toppress );
	  nadc_write_float( outfl, ++nr, "aai", 4, cld[nd].aai );
	  nadc_write_float( outfl, ++nr, "albedo", 4, cld[nd].albedo );
	  nadc_write_ushort( outfl, ++nr, "Flag describing the output above",
			     cld[nd].outputflag );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV2_WR_ASCII_BIAS
.PURPOSE    dump -- in ASCII Format -- the BIAS datasets
.INPUT/OUTPUT
  call as   SCIA_LV2_WR_ASCII_BIAS( mds_name, param, num_dsr, bias );
     input:
            char *mds_name            : name of DOAS Fitting Window Application
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct bias_scia *bias    : pointer to BIAS records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV2_WR_ASCII_BIAS( const char *mds_name, struct param_record param, 
			     unsigned int num_dsr, 
			     const struct bias_scia *bias )
{
     register unsigned int nd, nr;

     static bool init = TRUE;

     char  date_str[UTC_STRING_LENGTH];

     unsigned int ndim;

     const char prognm[] = "SCIA_LV2_WR_ASCII_BIAS";

     FILE *outfl;

     if ( init ) {
	  outfl = CRE_ASCII_File( param.outfile, "bias" );
	  if ( outfl == NULL || IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, 
				  param.outfile );
	  nadc_write_header( outfl, 0, param.infile, mds_name );
	  init = FALSE;
     } else {
	  outfl = CAT_ASCII_File( param.outfile, "bias" );
	  if ( outfl == NULL || IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, 
				  param.outfile );
	  nadc_write_text( outfl, 0, "MDS name", mds_name );
     }
/*
 * write ASCII dump of BIAS record
 */
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 0;
	  (void) MJD_2_ASCII( bias[nd].mjd.days, bias[nd].mjd.secnd,
			      bias[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, ++nr, "starttime", date_str );
	  nadc_write_uint( outfl, ++nr, "dsrlen", bias[nd].dsrlen );
	  nadc_write_schar( outfl, ++nr, "quality", bias[nd].quality );
	  nadc_write_float( outfl, ++nr, "inttime", 
			      3, bias[nd].intg_time / 16.f );
	  nadc_write_ushort( outfl, ++nr, "numfitp", bias[nd].numfitp );
	  nadc_write_ushort( outfl, ++nr, "numsegm", bias[nd].numsegm );
	  nadc_write_float( outfl, ++nr, "hght", 3, bias[nd].hght );
	  nadc_write_float( outfl, ++nr, "errhght", 3, bias[nd].errhght );
	  nadc_write_ushort( outfl, ++nr, "hghtflag", bias[nd].hghtflag );
	  nadc_write_float( outfl, ++nr, "vcd", 3, bias[nd].vcd );
	  nadc_write_float( outfl, ++nr, "errvcd", 3, bias[nd].errvcd );
	  nadc_write_float( outfl, ++nr, "closure", 3, bias[nd].closure );
	  nadc_write_float( outfl, ++nr, "errclosure", 
			     3, bias[nd].errclosure );
	  nadc_write_float( outfl, ++nr, "rms", 3, bias[nd].rms );
	  nadc_write_float( outfl, nr, "chi2", 3, bias[nd].chi2 );
	  nadc_write_float( outfl, nr, "goodness", 3, bias[nd].goodness );
	  nadc_write_float( outfl, nr, "cutoff", 3, bias[nd].cutoff );

	  nadc_write_ushort( outfl, ++nr, "numiter", bias[nd].numiter );
	  ndim = (bias[nd].numfitp * (bias[nd].numfitp - 1u)) / 2u;
	  nadc_write_arr_float( outfl, ++nr, "corrpar", 
				 1, &ndim, 3, bias[nd].corrpar );
	  nadc_write_ushort( outfl, ++nr, "vcdflag", bias[nd].vcdflag );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV2_WR_ASCII_DOAS
.PURPOSE    dump -- in ASCII Format -- the DOAS datasets
.INPUT/OUTPUT
  call as   SCIA_LV2_WR_ASCII_DOAS( mds_name, param, num_dsr, doas );
     input:
            char *mds_name            : name of DOAS Fitting Window Application
	    struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct doas_scia *doas    : pointer to DOAS records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV2_WR_ASCII_DOAS( const char *mds_name, struct param_record param, 
			     unsigned int num_dsr, 
			     const struct doas_scia *doas )
{
     register unsigned int nd, nr;

     static bool init = TRUE;

     char  date_str[UTC_STRING_LENGTH];

     unsigned int ndim;

     const char prognm[] = "SCIA_LV2_WR_ASCII_DOAS";

     FILE *outfl;

     if ( init ) {
	  outfl = CRE_ASCII_File( param.outfile, "doas" );
	  if ( outfl == NULL || IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, 
				  param.outfile );
	  nadc_write_header( outfl, 0, param.infile, mds_name );
	  init = FALSE;
     } else {
	  outfl = CAT_ASCII_File( param.outfile, "doas" );
	  if ( outfl == NULL || IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, 
				  param.outfile );
	  nadc_write_text( outfl, 0, "MDS name", mds_name );
     }
/*
 * write ASCII dump of DOAS record
 */
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 0;
	  (void) MJD_2_ASCII( doas[nd].mjd.days, doas[nd].mjd.secnd,
			      doas[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, ++nr, "starttime", date_str );
	  nadc_write_uint( outfl, ++nr, "dsrlen", doas[nd].dsrlen );
	  nadc_write_schar( outfl, ++nr, "quality", doas[nd].quality );
	  nadc_write_float( outfl, ++nr, "inttime", 
			      3, doas[nd].intg_time / 16.f );
	  nadc_write_ushort( outfl, ++nr, "numfitp", doas[nd].numfitp );
	  nadc_write_float( outfl, ++nr, "vcd", 3, doas[nd].vcd );
	  nadc_write_float( outfl, ++nr, "errvcd", 3, doas[nd].errvcd );
	  nadc_write_ushort( outfl, ++nr, "vcdflag", doas[nd].vcdflag );
	  nadc_write_float( outfl, ++nr, "esc", 3, doas[nd].esc );
	  nadc_write_float( outfl, ++nr, "erresc", 3, doas[nd].erresc );
	  nadc_write_float( outfl, ++nr, "rms", 3, doas[nd].rms );
	  nadc_write_float( outfl, nr, "chi2", 3, doas[nd].chi2 );
	  nadc_write_float( outfl, nr, "goodness", 3, doas[nd].goodness );
	  nadc_write_ushort( outfl, ++nr, "numiter", doas[nd].numiter );
	  ndim = (doas[nd].numfitp * (doas[nd].numfitp - 1u)) / 2u;
	  nadc_write_arr_float( outfl, ++nr, "corrpar", 
				 1, &ndim, 3, doas[nd].corrpar );
	  nadc_write_ushort( outfl, ++nr, "escflag", doas[nd].escflag );
	  nadc_write_float( outfl, ++nr, "amfgnd", 3, doas[nd].amfgnd );
	  nadc_write_float( outfl, ++nr, "amfcld", 3, doas[nd].amfcld );
	  nadc_write_float( outfl, ++nr, "reflgnd", 3, doas[nd].reflgnd );
	  nadc_write_float( outfl, ++nr, "reflcld", 3, doas[nd].reflcld );
	  nadc_write_float( outfl, ++nr, "refl", 3, doas[nd].refl );
	  nadc_write_ushort( outfl, ++nr, "amfflag", doas[nd].amfflag );
     }
     (void) fclose( outfl );
}
