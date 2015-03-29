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

.IDENTifer   SCIA_LV1_WR_GADS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1b
.LANGUAGE    ANSI C
.PURPOSE     Dump Global Annotation Data Sets in ASCII
.RETURNS     Nothing
.COMMENTS    contains SCIA_LV1_WR_ASCII_SIP, SCIA_LV1_WR_ASCII_CLCP, 
                SCIA_LV1_WR_ASCII_VLCP, SCIA_LV1_WR_ASCII_PPG, 
		SCIA_LV1_WR_ASCII_BASE, SCIA_LV1_WR_ASCII_SCP, 
		SCIA_LV1_WR_ASCII_SRS, SCIA_LV1_WR_ASCII_PSPN, 
		SCIA_LV1_WR_ASCII_PSPL, SCIA_LV1_WR_ASCII_PSPO, 
		SCIA_LV1_WR_ASCII_RSPN, SCIA_LV1_WR_ASCII_RSPL, 
		SCIA_LV1_WR_ASCII_RSPO, SCIA_LV1_WR_ASCII_EKD, 
		SCIA_LV1_WR_ASCII_SFP, SCIA_LV1_WR_ASCII_ASFP
.ENVIRONment None
.VERSION      4.3   11-Oct-2005	always write only one EKD records per file, RvH
              4.2   08-Aug-2005	fixed typo (Thanks to Jochen Skupin), RvH
              4.1   06-Mar-2002	update to v.4: level_2_smr, RvH
              4.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              3.0   03-Jan-2001	split the module "write_ascii", RvH 
              2.2   21-Dec-2000 added SCIA_LV1_WR_ASCII_NADIR, RvH
              2.1   20-Dec-2000 use output filename given by the user, RvH
              2.0   17-Aug-2000 major rewrite and standardization, RvH
              1.1   14-Jul-2000 renamed: DEBUG -> SCIA_LV1_WR_ASCII, RvH
              1.0   02-Mar-1999 created by R. M. van Hees
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
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_SIP
.PURPOSE    dump -- in ASCII Format -- the SIP record
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_SIP( param, sip );
     input:
            struct param_record param : struct holding user-defined settings
	    struct sip_scia *sip      : pointer to SIP record
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_SIP( struct param_record param, 
			    const struct sip_scia *sip )
{
     register unsigned int nr = 0;

     unsigned int  count[2];

     FILE  *outfl = CRE_ASCII_File( param.outfile, "sip" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of SIP record
 */
     nadc_write_header( outfl, nr, param.infile, 
			 "Static Instrument Parameters" );
     nadc_write_uchar( outfl, ++nr, "n_lc_min", sip->n_lc_min );
     nadc_write_uchar( outfl, ++nr, "ds_n_phases", sip->ds_n_phases );
     count[0] = (unsigned int) sip->ds_n_phases;
     nadc_write_arr_float( outfl, ++nr, "ds_phase_boundaries",
			    1, count, 6, sip->ds_phase_boundaries );
     count[0] = 2;
     nadc_write_arr_float( outfl, ++nr, "lc_stray_indx",
			    1, count, 6, sip->lc_stray_indx );
     nadc_write_uchar( outfl, ++nr, "lc_harm_order", sip->lc_harm_order );
     nadc_write_uchar( outfl, ++nr, "ds_poly_order", sip->ds_poly_order );
     
     nadc_write_text( outfl, ++nr, "do_var_lc_cha", sip->do_var_lc_cha );
     nadc_write_text( outfl, ++nr, "do_stray_lc_cha", sip->do_stray_lc_cha );
     nadc_write_text( outfl, ++nr, "do_var_lc_pmd", sip->do_var_lc_pmd );
     nadc_write_text( outfl, ++nr, "do_stray_lc_pmd", sip->do_stray_lc_pmd );

     count[0] = SCIENCE_CHANNELS;
     nadc_write_arr_float( outfl, ++nr, "electrons_bu",
			    1, count, 6, sip->electrons_bu );
     nadc_write_float( outfl, ++nr, "ppg_error", 5, sip->ppg_error );
     nadc_write_float( outfl, ++nr, "stray_error", 5, sip->stray_error );

     nadc_write_uchar( outfl, ++nr, "sp_n_phases", sip->sp_n_phases );
     count[0] = (unsigned int) sip->sp_n_phases;
     nadc_write_arr_float( outfl, ++nr, "sp_phase_boundaries",
			    1, count, 6, sip->sp_phase_boundaries );
     nadc_write_short( outfl, ++nr, "startpix_6+", sip->startpix_6 );
     nadc_write_short( outfl, ++nr, "startpix_8+", sip->startpix_8 );
     nadc_write_float( outfl, ++nr, "h_toa", 2, sip->h_toa );
     nadc_write_float( outfl, ++nr, "lambda_end_gdf", 
			2, sip->lambda_end_gdf );
     nadc_write_text( outfl, ++nr, "do_pol_point", sip->do_pol_point );

     count[0] = SCIENCE_CHANNELS;
     nadc_write_arr_ushort( outfl, ++nr, "sat_level",
			     1, count, sip->sat_level );
     nadc_write_ushort( outfl, ++nr, "pmd_sat_limit", sip->pmd_sat_limit );

     nadc_write_text( outfl, ++nr, "do_use_limb_dark", 
		       sip->do_use_limb_dark );
     nadc_write_text( outfl, ++nr, "do_pixelwise", sip->do_pixelwise );

     nadc_write_float( outfl, ++nr, "alpha0_asm", 2, sip->alpha0_asm );
     nadc_write_float( outfl, ++nr, "alpha0_esm", 2, sip->alpha0_esm );

     nadc_write_text( outfl, ++nr, "do_fraunhofer", sip->do_fraunhofer );
     nadc_write_text( outfl, ++nr, "do_etalon", sip->do_etalon );
     nadc_write_text( outfl, ++nr, "do_ib_sd_etn", sip->do_ib_sd_etn );
     nadc_write_text( outfl, ++nr, "do_ib_oc_etn", sip->do_ib_oc_etn );

     count[0] = SCIENCE_CHANNELS;
     nadc_write_arr_uchar( outfl, ++nr, "level_2_smr",
			     1, count, sip->level_2_smr );
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_CLCP
.PURPOSE    dump -- in ASCII Format -- the CLCP record
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_CLCP( param, clcp );
     input:
            struct param_record param : struct holding user-defined settings
	    struct clcp_scia *clcp    : pointer to CLCP record
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_CLCP( struct param_record param, 
			     const struct clcp_scia *clcp )
{
     register unsigned int nr = 0;

     unsigned int count[2];

     FILE *outfl = CRE_ASCII_File( param.outfile, "clcp" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of CLCP record
 */
     nadc_write_header( outfl, nr, param.infile, 
			 "Leakage Current Parameters (constant fraction)" );
     count[0] = SCIENCE_CHANNELS;
     count[1] = CHANNEL_SIZE;
     nadc_write_arr_float( outfl, ++nr, "Const. frac. of the FPN", 
			    2, count, 3, clcp->fpn );
     nadc_write_arr_float( outfl, ++nr, "Error on const. frac. of FPN",
			    2, count, 6, clcp->fpn_error );
     nadc_write_arr_float( outfl, ++nr, "Const. frac. of the LC", 
			    2, count, 3, clcp->lc );
     nadc_write_arr_float( outfl, ++nr, "Error on const. frac. of LC", 
			    2, count, 6, clcp->lc_error );
     count[0] = 2;
     count[1] = PMD_NUMBER;
     nadc_write_arr_float( outfl, ++nr, 
			    "Const. frac. of PMD dark offset", 
			    2, count, 3, clcp->pmd_dark );
     nadc_write_arr_float( outfl, ++nr, "Error on PMD dark offset", 
			    2, count, 6, clcp->pmd_dark_error );
     count[0] = SCIENCE_CHANNELS;
     count[1] = CHANNEL_SIZE;
     nadc_write_arr_float( outfl, ++nr, "Mean noise", 
			    2, count, 6, clcp->mean_noise );
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_VLCP
.PURPOSE    dump -- in ASCII Format -- the VLCP records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_VLCP( param, num_dsr, vlcp );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct vlcp_scia *vlcp    : pointer to VLCP record(s)
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_VLCP( struct param_record param, unsigned int num_dsr,
			     const struct vlcp_scia *vlcp )
{
     register unsigned int nd;

     unsigned int  count[2];

     FILE *outfl = CRE_ASCII_File( param.outfile, "vlcp" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of VLCP record
 */
     nadc_write_header( outfl, 0, param.infile, 
			 "Leakage Current Parameters (variable fraction)" );
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  register unsigned int nr = 0;

	  nadc_write_float( outfl, ++nr, "Orbit phase", 
			     5, vlcp[nd].orbit_phase );
	  count[0] = 10;
	  nadc_write_arr_float( outfl, ++nr, "Obm det PMD", 
				 1, count, 3, vlcp[nd].obm_pmd );
	  count[0] = IR_CHANNELS;
	  count[1] = CHANNEL_SIZE;
	  nadc_write_arr_float( outfl, ++nr, "Variable fraction of LC", 
				 2, count, 5, vlcp[nd].var_lc );
	  nadc_write_arr_float( outfl, ++nr, 
				 "Error on variable fraction of LC", 
				 2, count, 5, vlcp[nd].var_lc_error );

	  count[1] = SCIENCE_CHANNELS;
	  nadc_write_arr_float( outfl, ++nr, "Solar straylight", 
				 2, count, 5, vlcp[nd].solar_stray );
	  nadc_write_arr_float( outfl, ++nr, "Error on Solar straylight", 
				 2, count, 5, vlcp[nd].solar_stray_error );
	  count[0] = PMD_NUMBER;
	  nadc_write_arr_float( outfl, ++nr, "Straylight offset PMD", 
				 1, count, 5, vlcp[nd].pmd_stray );
	  nadc_write_arr_float( outfl, ++nr, 
				 "Error on Straylight offset PMD", 
				 1, count, 5, vlcp[nd].pmd_stray_error );
	  count[0] = 2;
	  nadc_write_arr_float( outfl, ++nr, "Dark offset PMD", 
				 1, count, 5, vlcp[nd].pmd_dark );
	  nadc_write_arr_float( outfl, ++nr, 
				 "Error on Dark offset PMD", 
				 1, count, 5, vlcp[nd].pmd_dark_error );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_PPG
.PURPOSE    dump -- in ASCII Format -- the PPG record
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_PPG( param, ppg );
     input:
            struct param_record param : struct holding user-defined settings
	    struct ppg_scia *ppg      : pointer to PPG record
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_PPG( struct param_record param,
			    const struct ppg_scia *ppg )
{
     register unsigned int nr = 0;

     unsigned int  count[2];

     FILE *outfl = CRE_ASCII_File( param.outfile, "ppg" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of PPG record
 */
     nadc_write_header( outfl, nr, param.infile, "PPG/Etalon Parameters" );
     count[0] = SCIENCE_CHANNELS;
     count[1] = CHANNEL_SIZE;
     nadc_write_arr_float( outfl, ++nr, "Pixel-to-pixel gain factor", 
			    2, count, 5, ppg->ppg_fact );
     nadc_write_arr_float( outfl, ++nr, "Etalon correction factor", 
			    2, count, 5, ppg->etalon_fact );
     nadc_write_arr_float( outfl, ++nr, "Etalon residue", 
			    -2, count, 5, ppg->etalon_resid );
     nadc_write_arr_float( outfl, ++nr, "WLS degradation factor", 
			    2, count, 5, ppg->wls_deg_fact );
     nadc_write_arr_uchar( outfl, ++nr, "Bad pixel mask", 
			    2, count, ppg->bad_pixel );
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_BASE
.PURPOSE    dump -- in ASCII Format -- the BASE record
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_BASE( param, base );
     input:
            struct param_record param : struct holding user-defined settings
	    struct base_scia *base    : pointer to BASE record
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_BASE( struct param_record param, 
			     const struct base_scia *base )
{
     unsigned int  count[2];

     FILE *outfl = CRE_ASCII_File( param.outfile, "base" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of SPECTRAL_BASE record
 */
     nadc_write_header( outfl, 0, param.infile, 
	 "Precise Basis of the Spectral Calibration Parameters" );
     count[0] = SCIENCE_CHANNELS;
     count[1] = CHANNEL_SIZE;
     nadc_write_arr_float( outfl, 1, "Wavelength det. Pixel", 
			    2, count, 5, base->wvlen_det_pix );
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_SCP
.PURPOSE    dump -- in ASCII Format -- the SCP records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_SCP( param, num_dsr, scp );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct scp_scia *scp      : pointer to SCP record(s)
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_SCP( struct param_record param, unsigned int num_dsr,
			    const struct scp_scia *scp )
{
     register unsigned int nd, nr;

     unsigned int  count[2];

     FILE *outfl = CRE_ASCII_File( param.outfile, "scp" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of SCP record
 */
     nadc_write_header( outfl, 0, param.infile, 
			 "Spectral Calibration Parameters" );
     count[0] = SCIENCE_CHANNELS;
     count[1] = 5;
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  nadc_write_float( outfl, nr++, "Orbit phase", 5, 
			     scp[nd].orbit_phase );
	  nadc_write_arr_double( outfl, nr++, 
				  "Coeffs of 4th order polynomial", 
				  -2, count, 5, scp[nd].coeffs );
	  nadc_write_arr_ushort( outfl, nr++, "Number of lines used", 
				  1, count, scp[nd].num_lines );
	  nadc_write_arr_float( outfl, nr, "Wavelength calibration error", 
				 1, count, 5, scp[nd].wv_error_calib );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_SRS
.PURPOSE    dump -- in ASCII Format -- the SRS records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_SRS( param, num_dsr, srs );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct srs_scia *srs      : pointer to SRS record(s)
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_SRS( struct param_record param, unsigned int num_dsr,
			    const struct srs_scia *srs )
{
     register unsigned int nd, nr;

     unsigned int  count[2];

     FILE *outfl = CRE_ASCII_File( param.outfile, "srs" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of SRS record
 */
     nadc_write_header( outfl, 0, param.infile, "Sun Reference Spectrum" );

     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  nadc_write_text( outfl, nr++, "Sun spectrum ID", 
			    srs[nd].sun_spec_id );
	  count[0] = SCIENCE_CHANNELS;
	  count[1] = CHANNEL_SIZE;
	  nadc_write_arr_float( outfl, nr++, "Wavelength", 
				 2, count, 5, srs[nd].wvlen_sun );
	  nadc_write_arr_float( outfl, nr++, "Mean reference spectrum", 
				 2, count, 5, srs[nd].mean_sun );
	  nadc_write_arr_float( outfl, nr++, "Radiometric precision",
				 2, count, 5, srs[nd].precision_sun );
	  nadc_write_arr_float( outfl, nr++, "Radiometric accuracy",
				 2, count, 5, srs[nd].accuracy_sun );
	  nadc_write_arr_float( outfl, nr++, "Aperture etalon",
				 2, count, 5, srs[nd].etalon );
	  nadc_write_float( outfl, nr++, 
			     "Average azimuth mirror position", 
			     5, srs[nd].avg_asm );
	  nadc_write_float( outfl, nr++, 
			     "Average elevation mirror position", 
			     5, srs[nd].avg_esm );
	  nadc_write_float( outfl, nr++, 
			     "Average Solar elevation angle", 
			     5, srs[nd].avg_elev_sun );
	  count[0] = PMD_NUMBER;
	  nadc_write_arr_float( outfl, nr++, "Mean value of PMD", 
				 1, count, 5, srs[nd].pmd_mean );
	  nadc_write_arr_float( outfl, nr++, "PMD out_nd_out signal", 
				 1, count, 5, srs[nd].pmd_out_nd_out );
	  nadc_write_arr_float( outfl, nr++, "PMD out_nd_in signal", 
				 1, count, 5, srs[nd].pmd_out_nd_in );
	  nadc_write_float( outfl, nr, "Doppler shift (500nm)", 
			     5, srs[nd].dopp_shift );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_PSPN
.PURPOSE    dump -- in ASCII Format -- the PSPN records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_PSPN( param, num_dsr, pspn );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct pspn_scia *pspn    : pointer to PSPN record(s)
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_PSPN( struct param_record param, unsigned int num_dsr,
			     const struct pspn_scia *pspn )
{
     register unsigned int nd, nr;

     unsigned int  count[2];

     FILE *outfl = CRE_ASCII_File( param.outfile, "pspn" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of PSPN record
 */
     nadc_write_header( outfl, 0, param.infile, 
			 "Polarisation Sensitivity Parameters Nadir" );
     count[0] = SCIENCE_CHANNELS;
     count[1] = CHANNEL_SIZE;
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  nadc_write_float( outfl, nr++, "Elevation mirror position", 
			     5, pspn[nd].ang_esm );
	  nadc_write_arr_double( outfl, nr++, 
				 "Mu2 nadir for elevation mirror", 
				 2, count, 5, pspn[nd].mu2 );
	  nadc_write_arr_double( outfl, nr, 
				 "Mu3 nadir for elevation mirror", 
				 2, count, 5, pspn[nd].mu3 );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_PSPL
.PURPOSE    dump -- in ASCII Format -- the PSPL records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_PSPL( param, num_dsr, pspl );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct psplo_scia *pspl   : pointer to PSPL record(s)
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_PSPL( struct param_record param, unsigned int num_dsr,
			     const struct psplo_scia *pspl )
{
     register unsigned int nd, nr;

     unsigned int  count[2];

     FILE *outfl = CRE_ASCII_File( param.outfile, "pspl" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of PSPL record
 */
     nadc_write_header( outfl, 0, param.infile,  
			 "Polarisation Sensitivity Parameters Limb" );
     count[0] = SCIENCE_CHANNELS;
     count[1] = CHANNEL_SIZE;
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  nadc_write_float( outfl, nr++, "Elevation mirror position", 
			     5, pspl[nd].ang_esm );
	  nadc_write_float( outfl, nr++, "Azimuth mirror position", 
			     5, pspl[nd].ang_asm );
	  nadc_write_arr_double( outfl, nr++, 
				 "Mu2 limb for elevation mirror", 
				 2, count, 5, pspl[nd].mu2 );
	  nadc_write_arr_double( outfl, nr, 
				 "Mu3 limb for elevation mirror", 
				 2, count, 5, pspl[nd].mu3 );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_PSPO
.PURPOSE    dump -- in ASCII Format -- the PSPO records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_PSPO( param, num_dsr, pspo );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct psplo_scia *pspo   : pointer to PSPO record(s)
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_PSPO( struct param_record param, unsigned int num_dsr,
			     const struct psplo_scia *pspo )
{
     register unsigned int nd, nr;

     unsigned int  count[2];

     FILE *outfl = CRE_ASCII_File( param.outfile, "pspo" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of PSPO record
 */
     nadc_write_header( outfl, 0, param.infile,  
			 "Polarisation Sensitivity Parameters Occultation" );
     count[0] = SCIENCE_CHANNELS;
     count[1] = CHANNEL_SIZE;
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  nadc_write_float( outfl, nr++, "Elevation mirror position", 
			     5, pspo[nd].ang_esm );
	  nadc_write_float( outfl, nr++, "Azimuth mirror position", 
			     5, pspo[nd].ang_asm );
	  nadc_write_arr_double( outfl, nr++, 
				 "Mu2 limb for elevation mirror", 
				 2, count, 5, pspo[nd].mu2 );
	  nadc_write_arr_double( outfl, nr, 
				 "Mu3 limb for elevation mirror", 
				 2, count, 5, pspo[nd].mu3 );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_RSPN
.PURPOSE    dump -- in ASCII Format -- the RSPN records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_RSPN( param, num_dsr, rspn );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct rspn_scia *rspn    : pointer to RSPN record(s)
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_RSPN( struct param_record param, unsigned int num_dsr, 
			     const struct rspn_scia *rspn )
{
     register unsigned int nd, nr;

     unsigned int  count[2];

     FILE *outfl = CRE_ASCII_File( param.outfile, "rspn" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of RSPN record
 */
     nadc_write_header( outfl, 0, param.infile, 
			 "Radiance Sensitivity Parameters (nadir)" );
     count[0] = SCIENCE_CHANNELS;
     count[1] = CHANNEL_SIZE;
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  nadc_write_float( outfl, nr++, "Elevation mirror position", 
			    5, rspn[nd].ang_esm );
	  nadc_write_arr_double( outfl, nr, "Radiance sensitivity", 
				 2, count, 10, rspn[nd].sensitivity );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_RSPL
.PURPOSE    dump -- in ASCII Format -- the RSPL records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_RSPL( param, num_dsr, rspl );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct rsplo_scia *rspl   : pointer to RSPL record(s)
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_RSPL( struct param_record param, unsigned int num_dsr, 
			     const struct rsplo_scia *rspl )
{
     FILE *outfl = CRE_ASCII_File( param.outfile, "rspl" );

     register unsigned int nd, nr;

     unsigned int  count[2];

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of RSPL record
 */
     nadc_write_header( outfl, 0, param.infile, 
			 "Radiance Sensitivity Parameters (limb)" );
     count[0] = SCIENCE_CHANNELS;
     count[1] = CHANNEL_SIZE;
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  nadc_write_float( outfl, nr++, "Elevation mirror position", 
			    5, rspl[nd].ang_esm );
	  nadc_write_float( outfl, nr++, "Azimuth mirror position", 
			    5, rspl[nd].ang_asm );
	  nadc_write_arr_double( outfl, nr, "Radiance sensitivity", 
				 2, count, 10, rspl[nd].sensitivity );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_RSPO
.PURPOSE    dump -- in ASCII Format -- the RSPO records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_RSPO( param, num_dsr, rspo );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct rsplo_scia *rspo   : pointer to RSPO record(s)
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_RSPO( struct param_record param, unsigned int num_dsr, 
			     const struct rsplo_scia *rspo )
{
     FILE *outfl = CRE_ASCII_File( param.outfile, "rspo" );

     register unsigned int nd, nr;

     unsigned int  count[2];

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of RSPO record
 */
     nadc_write_header( outfl, 0, param.infile, 
			 "Radiance Sensitivity Parameters (occultation)" );
     count[0] = SCIENCE_CHANNELS;
     count[1] = CHANNEL_SIZE;
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  nadc_write_float( outfl, nr++, "Elevation mirror position", 
			    5, rspo[nd].ang_esm );
	  nadc_write_float( outfl, nr++, "Azimuth mirror position", 
			    5, rspo[nd].ang_asm );
	  nadc_write_arr_double( outfl, nr, "Radiance sensitivity", 
				 2, count, 10, rspo[nd].sensitivity );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_EKD
.PURPOSE    dump -- in ASCII Format -- the EKD record
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_EKD( param, ekd );
     input:
            struct param_record param : struct holding user-defined settings
	    struct ekd_scia *ekd      : pointer to EKD record
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_EKD( struct param_record param,
			    const struct ekd_scia *ekd )
{
     register unsigned int nr = 0;

     unsigned int  count[2];

     FILE *outfl = CRE_ASCII_File( param.outfile, "ekd" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of EKD record
 */
     nadc_write_header( outfl, nr++, param.infile, "Errors on Key Data" );     
     count[0] = SCIENCE_CHANNELS;
     count[1] = CHANNEL_SIZE;
     nadc_write_arr_float( outfl, nr++, "Error on Mu2 nadir", 
			   2, count, 5, ekd->mu2_nadir );
     nadc_write_arr_float( outfl, nr++, "Error on Mu3 nadir", 
			   2, count, 5, ekd->mu3_nadir );
     nadc_write_arr_float( outfl, nr++, "Error on Mu2 limb", 
			   2, count, 5, ekd->mu2_limb );
     nadc_write_arr_float( outfl, nr++, "Error on Mu3 limb", 
			   2, count, 5, ekd->mu3_limb );
     nadc_write_arr_float( outfl, nr++, "Error on radiance sensitivity", 
			   2, count, 5, ekd->radiance_vis );
     nadc_write_arr_float( outfl, nr++, "Additional error (nadir)", 
			   2, count, 5, ekd->radiance_nadir );
     nadc_write_arr_float( outfl, nr++, "Additional error (limb)", 
			   2, count, 5, ekd->radiance_limb );
     nadc_write_arr_float( outfl, nr++, "Additional error (Sun)", 
			   2, count, 5, ekd->radiance_sun );
     nadc_write_arr_float( outfl, nr, "Error on BSDF", 
			   2, count, 5, ekd->bsdf );
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_SFP
.PURPOSE    dump -- in ASCII Format -- the SFP records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_SFP( param, num_dsr, sfp );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct sfp_scia *sfp      : pointer to SFP record(s)
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_SFP( struct param_record param, unsigned int num_dsr,
			    const struct sfp_scia *sfp )
{
     register unsigned int na, nr = 1;

     unsigned char  *cbuff;
     unsigned short *ubuff;
     unsigned int   count[1];
     float          *rbuff;

     FILE *outfl = CRE_ASCII_File( param.outfile, "sfp" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of SFP record
 */
     nadc_write_header( outfl, 0, param.infile, "Slit Function Parameters" );

     count[0] = num_dsr;
     ubuff = (unsigned short *) malloc( num_dsr * sizeof( short ));
     if ( ubuff == NULL ) 
          NADC_RETURN_ERROR( NADC_ERR_ALLOC, "ubuff" );
     na = 0;
     do { ubuff[na] = sfp[na].pix_pos_slit_fun; } while( ++na < num_dsr );
     nadc_write_arr_ushort( outfl, nr++, "Pixel position of slit function", 
			     1, count, ubuff );
     free( ubuff );

     cbuff = (unsigned char *) malloc( (size_t) num_dsr );
     if ( cbuff == NULL )
          NADC_RETURN_ERROR( NADC_ERR_ALLOC, "cbuff" );
     na = 0;
     do { cbuff[na] = sfp[na].type_slit_fun; } while( ++na < num_dsr );
     nadc_write_arr_uchar( outfl, nr++, "Type of slit function", 
			    1, count, cbuff );
     free( cbuff );

     rbuff = (float *) malloc( num_dsr * sizeof( float ));
     if ( rbuff == NULL ) 
          NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );
     na = 0;
     do { rbuff[na] = sfp[na].fwhm_slit_fun; } while( ++na < num_dsr );
     nadc_write_arr_float( outfl, nr++, "fwhm_slit_fun", 1, count, 5, rbuff );
     na = 0;
     do { rbuff[na] = sfp[na].f_voi_fwhm_loren; } while( ++na < num_dsr );
     nadc_write_arr_float( outfl, nr, "f_voi_fwhm_loren", 
			    1, count, 5, rbuff );
     free( rbuff );

     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_ASFP
.PURPOSE    dump -- in ASCII Format -- the ASFP records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_ASFP( param, num_dsr, asfp );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct asfp_scia *asfp    : pointer to ASFP record(s)
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_ASFP( struct param_record param, unsigned int num_dsr,
			     const struct asfp_scia *asfp )
{
     register unsigned int na, nr = 1;

     unsigned char  *cbuff;
     unsigned short *ubuff;
     unsigned int   count[1];
     float          *rbuff;

     FILE *outfl = CRE_ASCII_File( param.outfile, "asfp" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of ASFP record
 */
     nadc_write_header( outfl, 0, param.infile, 
			 "Small Aperture Slit Function Parameters" );
     count[0] = num_dsr;
     ubuff = (unsigned short *) malloc( num_dsr * sizeof( short ));
     if ( ubuff == NULL ) 
          NADC_RETURN_ERROR( NADC_ERR_ALLOC, "ubuff" );
     na = 0;
     do { ubuff[na] = asfp[na].pix_pos_slit_fun; } while( ++na < num_dsr );
     nadc_write_arr_ushort( outfl, nr++, "Pixel position of slit function", 
			     1, count, ubuff );
     free( ubuff );

     cbuff = (unsigned char *) malloc( (size_t) num_dsr );
     if ( cbuff == NULL ) 
          NADC_RETURN_ERROR( NADC_ERR_ALLOC, "cbuff" );
     na = 0;
     do { cbuff[na] = asfp[na].type_slit_fun; } while( ++na < num_dsr );
     nadc_write_arr_uchar( outfl, nr++, "Type of slit function", 
			    1, count, cbuff );
     free( cbuff );

     rbuff = (float *) malloc( num_dsr * sizeof( float ));
     if ( rbuff == NULL ) 
          NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );
     na = 0;
     do { rbuff[na] = asfp[na].fwhm_slit_fun; } while( ++na < num_dsr );
     nadc_write_arr_float( outfl, nr++, "fwhm_slit_fun", 1, count, 5, rbuff );
     na = 0;
     do { rbuff[na] = asfp[na].f_voi_fwhm_gauss; } while( ++na < num_dsr );
     nadc_write_arr_float( outfl, nr, "f_voi_fwhm_gauss", 
			    1, count, 5, rbuff );
     free( rbuff );

     (void) fclose( outfl );
}

