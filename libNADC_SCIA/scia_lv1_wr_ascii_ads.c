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

.IDENTifer   SCIA_LV1_WR_ASCII_ADS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1b
.LANGUAGE    ANSI C
.PURPOSE     Dump Annotation Data Sets in ASCII
.RETURNS     Nothing
.COMMENTS    contains SCIA_LV1_WR_ASCII_SQADS, SCIA_LV1_WR_ASCII_STATE, 
                SCIA_LV1_WR_ASCII_PMD, SCIA_LV1_WR_ASCII_AUX, 
		SCIA_LV1_WR_ASCII_LCPN, SCIA_LV1_WR_ASCII_DARK, 
		SCIA_LV1_WR_ASCII_PPGN, SCIA_LV1_WR_ASCII_SCPN, 
		SCIA_LV1_WR_ASCII_SRSN
.ENVIRONment None
.VERSION     3.0     03-Jan-2001   Split the module "write_ascii", RvH
             2.2     21-Dec-2000   Added SCIA_LV1_WR_ASCII_NADIR, RvH
             2.1     20-Dec-2000   Use output filename given by the user, RvH
             2.0     17-Aug-2000   Major rewrite and standardization, RvH
             1.1     14-Jul-2000   Renamed: DEBUG -> SCIA_LV1_WR_ASCII, RvH
             1.0     02-Mar-1999   Created by R. M. van Hees
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
.IDENTifer  SCIA_LV1_WR_ASCII_SQADS
.PURPOSE    dump -- in ASCII Format -- the SQADS
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_SQADS( param, num_dsr, sqads );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct sqads1_scia *sqads : pointer to SQADS records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_SQADS( struct param_record param, unsigned int num_dsr,
			      const struct sqads1_scia *sqads )
{
     register unsigned int nd, nr;

     char date_str[UTC_STRING_LENGTH];
     unsigned int  count[2];

     const char prognm[] = "SCIA_LV1_WR_ASCII_SQADS";

     FILE *outfl = CRE_ASCII_File( param.outfile, "sqads" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of SQADS record
 */
     nadc_write_header( outfl, 0, param.infile,  
			 "Summary of Quality Flags per State" );
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  (void) MJD_2_ASCII( sqads[nd].mjd.days, sqads[nd].mjd.secnd,
			      sqads[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, nr++, "Date", date_str );
	  nadc_write_uchar( outfl, nr++, "MDS DSR attached", 
			     sqads[nd].flag_mds );
	  count[0] = SCIENCE_CHANNELS;
	  nadc_write_arr_float( outfl, nr++, 
				 "Mean wavlen diff (Fraunhofer)", 
				 1, count, 6, sqads[nd].mean_wv_diff );
	  nadc_write_arr_float( outfl, nr++, 
				 "Std dev wavlen diff (Fraunhofer)", 
				 1, count, 6, sqads[nd].sdev_wv_diff );
	  nadc_write_ushort( outfl, nr++, "Number of missing readouts", 
				 sqads[nd].missing_readouts );
	  count[0] = (unsigned int) ALL_CHANNELS;
	  nadc_write_arr_float( outfl, nr++, 
				 "Mean diff. of leakage current", 
				 1, count, 5, sqads[nd].mean_diff_leak );
	  nadc_write_uchar( outfl, nr++, "Flag Sun glint", 
			     sqads[nd].flag_glint );
	  nadc_write_uchar( outfl, nr++, "Flag rainbow", 
			     sqads[nd].flag_rainbow );
	  nadc_write_uchar( outfl, nr++, "Flag SAA region", 
			     sqads[nd].flag_saa_region );
	  count[0] = (unsigned int) ALL_CHANNELS;
	  nadc_write_arr_ushort( outfl, nr++, "Number of hot pixels", 
				  1, count, sqads[nd].hotpixel );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_STATE
.PURPOSE    dump -- in ASCII Format -- the STATE records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_STATE( param, num_dsr, state );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct state1_scia *state : pointer to STATE records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_STATE( struct param_record param, unsigned int num_dsr,
			      const struct state1_scia *state )
{
     register unsigned int nc, nd, nr;

     char date_str[UTC_STRING_LENGTH];
     unsigned char  ucbuff[MAX_CLUSTER];
     unsigned short usbuff[MAX_CLUSTER];
     unsigned int   count[2];
     float rbuff[MAX_CLUSTER];

     const char prognm[] = "SCIA_LV1_WR_ASCII_STATE";

     FILE *outfl = CRE_ASCII_File( param.outfile, "state" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of STATE record
 */
     nadc_write_header( outfl, 0, param.infile, "States of the Product" );
     for ( nd = 0; nd < num_dsr; nd++ ) {
#ifdef DEBUG
	  int   orbit;
	  bool  saa;
	  float orbit_phase;
#endif
	  nr = 1;
	  (void) MJD_2_ASCII( state[nd].mjd.days, state[nd].mjd.secnd,
			      state[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, nr, "MJD (Date Time)", date_str );
	  nadc_write_double( outfl, nr++, "MJD (Julian Day)", 16, 
			     (double) state[nd].mjd.days + 
			     ((state[nd].mjd.secnd + state[nd].mjd.musec / 1e6)
			      / (24. * 60 * 60)) );
	  nadc_write_uchar( outfl, nr++, "MDS DSR attached", 
			     state[nd].flag_mds );
	  nadc_write_uchar( outfl, nr++, "Reason code", 
			     state[nd].flag_reason );
	  nadc_write_float( outfl, nr++, "Orbit phase", 
			     5, state[nd].orbit_phase );
#ifdef DEBUG
	  GET_SCIA_ROE_INFO( FALSE, (double) state[nd].mjd.days + 
			     ((state[nd].mjd.secnd + state[nd].mjd.musec / 1e6)
			      / (24. * 60 * 60)), &orbit, &saa, &orbit_phase );
	  nadc_write_float( outfl, nr, "Orbit phase (ROE)", 
			     3, orbit_phase );
	  nadc_write_int( outfl, nr, "Orbit number (ROE)", orbit );
#endif
	  nadc_write_ushort( outfl, nr++, "Measurement category", 
			      state[nd].category );
	  nadc_write_ushort( outfl, nr++, "State ID", 
			      state[nd].state_id );
	  nadc_write_ushort( outfl, nr++, "Duration of scan phase", 
			      state[nd].dur_scan );
	  nadc_write_ushort( outfl, nr++, "Longest integration time", 
			      state[nd].longest_intg_time );
	  nadc_write_ushort( outfl, nr++, "Number of clusters", 
			     state[nd].num_clus );
/*
 * write Cluster configuration
 */
	  count[0] = state[nd].num_clus;
	  for ( nc = 0; nc < count[0]; nc++ ) 
	       ucbuff[nc] = state[nd].Clcon[nc].id;
	  nadc_write_arr_uchar( outfl, nr, "Cluster ID", 1, count, ucbuff );
	  for ( nc = 0; nc < count[0]; nc++ ) 
	       ucbuff[nc] = state[nd].Clcon[nc].channel;
	  nadc_write_arr_uchar( outfl, nr, "Channel number", 
				 1, count, ucbuff );
	  for ( nc = 0; nc < count[0]; nc++ )
	       usbuff[nc] = state[nd].Clcon[nc].pixel_nr;
	  nadc_write_arr_ushort( outfl, nr, "Start pixel number", 
				  1, count, usbuff );
	  for ( nc = 0; nc < count[0]; nc++ )
	       usbuff[nc] = state[nd].Clcon[nc].length;
	  nadc_write_arr_ushort( outfl, nr, "Cluster length", 
				  1, count, usbuff );
	  for ( nc = 0; nc < count[0]; nc++ )
	       rbuff[nc] = state[nd].Clcon[nc].pet;
	  nadc_write_arr_float( outfl, nr, "Pixel exposure time", 
				 1, count, 5, rbuff );
	  for ( nc = 0; nc < count[0]; nc++ )
	       usbuff[nc] = state[nd].Clcon[nc].intg_time;
	  nadc_write_arr_ushort( outfl, nr, "Integration time", 
				  1, count, usbuff );
	  for ( nc = 0; nc < count[0]; nc++ )
	       usbuff[nc] = state[nd].Clcon[nc].coaddf;
	  nadc_write_arr_ushort( outfl, nr, "Co-adding factor", 
				  1, count, usbuff );
	  for ( nc = 0; nc < count[0]; nc++ ) 
	       usbuff[nc] = state[nd].Clcon[nc].n_read;
	  nadc_write_arr_ushort( outfl, nr, "Number of cluster readouts", 
				  1, count, usbuff );
	  for ( nc = 0; nc < count[0]; nc++ ) 
	       ucbuff[nc] = state[nd].Clcon[nc].type;
	  nadc_write_arr_uchar( outfl, nr++, "Cluster data type", 
				 1, count, ucbuff );
/*
 * End Cluster configuration
 */
	  nadc_write_uchar( outfl, nr++, "MDS for this state", 
			     state[nd].type_mds );
	  nadc_write_ushort( outfl, nr++, "Number of repeated geolocations", 
			      state[nd].num_aux );
	  nadc_write_ushort( outfl, nr++, "Number of integrated PMDs", 
			      state[nd].num_pmd );
	  nadc_write_ushort( outfl, nr++, "Number of different integrations", 
			      state[nd].num_intg );
	  count[0] = state[nd].num_intg;
	  nadc_write_arr_ushort( outfl, nr++, "Different integration times", 
				  1, count, state[nd].intg_times );
	  nadc_write_arr_ushort( outfl, nr++, 
				  "Number of polarisation values", 
				  1, count, state[nd].num_polar );
	  nadc_write_ushort( outfl, nr++, 
			      "Number of frac. polarisations", 
			      state[nd].total_polar );
	  nadc_write_ushort( outfl, nr++, "Number of DSRs", 
			     state[nd].num_dsr );
	  nadc_write_uint( outfl, nr, "Length of DSR", 
			     state[nd].length_dsr );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_PMD
.PURPOSE    dump -- in ASCII Format -- the Lv0 PMD records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_PMD( param, num_dsr, pmd );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct pmd_scia *pmd      : pointer to PMD records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_PMD( struct param_record param, unsigned int num_dsr,
			    const struct pmd_scia *pmd )
{
     register unsigned int nd, nr;

     char date_str[UTC_STRING_LENGTH];

     const char prognm[] = "SCIA_LV1_WR_ASCII_PMD";

     FILE *outfl = CRE_ASCII_File( param.outfile, "pmd" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of PMD record
 */
     nadc_write_header( outfl, 0, param.infile, "PMD Data Packets" );
     for ( nd = 0; nd < num_dsr; nd++, pmd++ ) {
	  nr = 0;
	  (void) MJD_2_ASCII( pmd->mjd.days, pmd->mjd.secnd,
			      pmd->mjd.musec, date_str );
	  nadc_write_text( outfl, ++nr, "Date", date_str );
	  nadc_write_uchar( outfl, ++nr, "MDS DSR attached", pmd->flag_mds );
	  SCIA_WR_ASCII_LV0_PMD( outfl, ++nr, &pmd->mds0 );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_AUX
.PURPOSE    dump -- in ASCII Format -- the Lv0 Auxiliary records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_AUX( param, num_dsr, aux );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct aux_scia *aux      : pointer to Auxiliary records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_AUX( struct param_record param, unsigned int num_dsr,
			    const struct aux_scia *aux )
{
     register unsigned int nd, nr;

     char date_str[UTC_STRING_LENGTH];

     const char prognm[] = "SCIA_LV1_WR_ASCII_AUX";

     FILE *outfl = CRE_ASCII_File( param.outfile, "aux" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of AUX record
 */
     nadc_write_header( outfl, 0, param.infile, "Auxiliary Data Packets" );
     for ( nd = 0; nd < num_dsr; nd++, aux++ ) {
	  nr = 0;
	  (void) MJD_2_ASCII( aux->mjd.days, aux->mjd.secnd,
			      aux->mjd.musec, date_str );
	  nadc_write_text( outfl, ++nr, "Date", date_str );
	  nadc_write_uchar( outfl, ++nr, "MDS DSR attached", aux->flag_mds );
	  SCIA_WR_ASCII_LV0_AUX( outfl, ++nr, &aux->mds0 );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_LCPN
.PURPOSE    dump -- in ASCII Format -- the LCPN records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_LCPN( param, num_dsr, lcpn );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct lcpn_scia *lcpn    : pointer to LCPN records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_LCPN( struct param_record param, unsigned int num_dsr,
			     const struct lcpn_scia *lcpn )
{
     register unsigned int nd, nr;

     char date_str[UTC_STRING_LENGTH];
     unsigned int  count[2];

     const char prognm[] = "SCIA_LV1_WR_ASCII_LCPN";

     FILE *outfl = CRE_ASCII_File( param.outfile, "lcpn" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of LCPN record
 */
     nadc_write_header( outfl, 0, param.infile, 
			 "Leakage Current Parameters (update)" );
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  (void) MJD_2_ASCII( lcpn[nd].mjd.days, lcpn[nd].mjd.secnd,
			      lcpn[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, nr++, "DSR date", date_str );
	  nadc_write_uchar( outfl, nr++, "MDS DSR attached", 
			     lcpn[nd].flag_mds );
	  (void) MJD_2_ASCII( lcpn[nd].mjd_last.days, lcpn[nd].mjd_last.secnd,
			      lcpn[nd].mjd_last.musec, date_str );
	  nadc_write_text( outfl, nr++, "Last DSR date", date_str );
	  nadc_write_float( outfl, nr++, "Orbit phase", 
			     5, lcpn[nd].orbit_phase );
	  count[0] = (unsigned int) (IR_CHANNELS + PMD_NUMBER);
	  nadc_write_arr_float( outfl, nr++, "Obm det PMD", 
				 1, count, 3, lcpn[nd].obm_pmd );
	  count[0] = SCIENCE_CHANNELS;
	  count[1] = CHANNEL_SIZE;
	  nadc_write_arr_float( outfl, nr++, "Fixed pattern noise", 
				 2, count, 5, lcpn[nd].fpn );
	  nadc_write_arr_float( outfl, nr++, "Error on FPN", 
				 2, count, 5, lcpn[nd].fpn_error );
	  nadc_write_arr_float( outfl, nr++, "Leakage current", 
				 2, count, 5, lcpn[nd].lc );
	  nadc_write_arr_float( outfl, nr++, "Error on LC", 
				 2, count, 5, lcpn[nd].lc_error );
	  nadc_write_arr_float( outfl, nr++, "Mean Noise", 
				 2, count, 5, lcpn[nd].mean_noise );
	  count[0] = 2;
	  count[1] = PMD_NUMBER;
	  nadc_write_arr_float( outfl, nr++, "Straylight offset (PMD)", 
				 2, count, 6, lcpn[nd].pmd_off );
	  nadc_write_arr_float( outfl, nr++, 
				 "Error on straylight offset (PMD)", 
				 2, count, 6, lcpn[nd].pmd_off_error );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_DARK
.PURPOSE    dump -- in ASCII Format -- the DARK records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_DARK( param, num_dsr, dark );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct dark_scia *dark    : pointer to DARK records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_DARK( struct param_record param, unsigned int num_dsr,
			     const struct dark_scia *dark )
{
     register unsigned int nd, nr;

     char date_str[UTC_STRING_LENGTH];
     unsigned int  count[2];

     const char prognm[] = "SCIA_LV1_WR_ASCII_DARK";

     FILE *outfl = CRE_ASCII_File( param.outfile, "dark" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of DARK record
 */
     nadc_write_header( outfl, 0, param.infile, 
			 "Dark Measurements per State" );
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  (void) MJD_2_ASCII( dark[nd].mjd.days, dark[nd].mjd.secnd,
			      dark[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, nr++, "DSR date", date_str );
	  nadc_write_uchar( outfl, nr++, "MDS DSR attached", 
			     dark[nd].flag_mds );
	  count[0] = SCIENCE_CHANNELS;
	  count[1] = CHANNEL_SIZE;
	  nadc_write_arr_float( outfl, nr++, "dark measurement", 
				 2, count, 5, dark[nd].dark_spec );
	  nadc_write_arr_float( outfl, nr++, 
				 "StDev of the dark measurement", 
				 2, count, 5, dark[nd].sdev_dark_spec );
	  count[0] = 2;
	  count[1] = PMD_NUMBER;
	  nadc_write_arr_float( outfl, nr++, "PMD offset",
				 2, count, 5, dark[nd].pmd_off );
	  nadc_write_arr_float( outfl, nr++, "Error on the PMD offset", 
				 2, count, 5, dark[nd].pmd_off_error );
	  count[0] = SCIENCE_CHANNELS;
	  count[1] = CHANNEL_SIZE;
	  nadc_write_arr_float( outfl, nr++, "solar straylight", 
				 2, count, 5, dark[nd].sol_stray );
	  nadc_write_arr_float( outfl, nr++, 
				 "Error on the solar straylight", 
				 2, count, 5, dark[nd].sol_stray_error );
	  count[0] = PMD_NUMBER;
	  nadc_write_arr_float( outfl, nr++, "PMD straylight offset", 
				 1, count, 5, dark[nd].pmd_stray );
	  nadc_write_arr_float( outfl, nr++, 
				 "Error on the PMD straylight offset", 
				 1, count, 5, dark[nd].pmd_stray_error );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_PPGN
.PURPOSE    dump -- in ASCII Format -- the PPGN records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_PPGN( param, num_dsr, ppgn );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct ppgn_scia *ppgn    : pointer to PPGN records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_PPGN( struct param_record param, unsigned int num_dsr,
			     const struct ppgn_scia *ppgn )
{
     register unsigned int nd, nr;

     char date_str[UTC_STRING_LENGTH];
     unsigned int  count[2];

     const char prognm[] = "SCIA_LV1_WR_ASCII_PPGN";

     FILE *outfl = CRE_ASCII_File( param.outfile, "ppgn" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of PPGN record
 */
     nadc_write_header( outfl, 0, param.infile, 
			 "PPG/Etalon Parameters (update)" );
     count[0] = SCIENCE_CHANNELS;
     count[1] = CHANNEL_SIZE;
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  (void) MJD_2_ASCII( ppgn[nd].mjd.days, ppgn[nd].mjd.secnd,
			      ppgn[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, nr++, "DSR date", date_str );
	  nadc_write_uchar( outfl, nr++, "MDS DSR attached", 
			     ppgn[nd].flag_mds );
	  nadc_write_arr_float( outfl, nr++, "Pixel-to-pixel gain factor", 
				 2, count, 5, ppgn[nd].gain_fact );
	  nadc_write_arr_float( outfl, nr++, "Etalon correction factor", 
				 2, count, 5, ppgn[nd].etalon_fact );
	  nadc_write_arr_float( outfl, nr++, "Etalon residue", 
				 2, count, 5, ppgn[nd].etalon_resid );
	  nadc_write_arr_float( outfl, nr++, "Average WLS spectral PPG", 
				 2, count, 5, ppgn[nd].avg_wls_spec );
	  nadc_write_arr_float( outfl, nr++, "StDev WLS spectral PPG", 
				 2, count, 5, ppgn[nd].sd_wls_spec );
	  nadc_write_arr_uchar( outfl, nr, "Bad pixel mask",
				 2, count, ppgn[nd].bad_pixel );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_SCPN
.PURPOSE    dump -- in ASCII Format -- the SCPN records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_SCPN( param, num_dsr, scpn );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct scpn_scia *scpn    : pointer to SCPN records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_SCPN( struct param_record param, unsigned int num_dsr,
			     const struct scpn_scia *scpn )
{
     register unsigned int nd, nr;

     char date_str[UTC_STRING_LENGTH];
     unsigned int  count[2];

     const char prognm[] = "SCIA_LV1_WR_ASCII_SCPN";

     FILE *outfl = CRE_ASCII_File( param.outfile, "scpn" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of SCPN record
 */
     nadc_write_header( outfl, 0, param.infile, 
			 "Spectral Calibration Parameters (update)" );
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  (void) MJD_2_ASCII( scpn[nd].mjd.days, scpn[nd].mjd.secnd,
			      scpn[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, nr++, "DSR date", date_str );
	  nadc_write_uchar( outfl, nr++, "MDS DSR attached", 
			     scpn[nd].flag_mds );
	  nadc_write_float( outfl, nr++, "Orbit phase", 
			     5, scpn[nd].orbit_phase );
	  count[0] = SCIENCE_CHANNELS;
	  count[1] = 5;
	  nadc_write_arr_double( outfl, nr++, 
				  "Coeffs of 4th order polynomial", 
				  -2, count, 5, scpn[nd].coeffs );
	  nadc_write_arr_ushort( outfl, nr++, "Number of lines used", 
				  1, count, scpn[nd].num_lines );
	  nadc_write_arr_float( outfl, nr++, "Wavelength calibration error", 
				 1, count, 5, scpn[nd].wv_error_calib );
	  nadc_write_arr_uchar( outfl, nr++, "Source spec calib parameters", 
				 1, count, scpn[nd].srs_param );
	  count[1] = CHANNEL_SIZE;	  
	  nadc_write_arr_float( outfl, nr++, "Average SLS/Solar spectrum", 
				 2, count, 5, scpn[nd].sol_spec );
	  count[0] = 24;
	  nadc_write_arr_float( outfl, nr++, "Selected line position", 
				 1, count, 5, scpn[nd].line_pos );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer  SCIA_LV1_WR_ASCII_SRSN
.PURPOSE    dump -- in ASCII Format -- the SRSN records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_ASCII_SRSN( param, num_dsr, srsn );
     input:
            struct param_record param : struct holding user-defined settings
	    unsigned int num_dsr      : number of data sets
	    struct srsn_scia *srsn    : pointer to SRSN records
            
.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_LV1_WR_ASCII_SRSN( struct param_record param, unsigned int num_dsr,
			     const struct srsn_scia *srsn )
{
     register unsigned int nd, nr;

     char date_str[UTC_STRING_LENGTH];
     unsigned int  count[2];

     const char prognm[] = "SCIA_LV1_WR_ASCII_SRSN";

     FILE *outfl = CRE_ASCII_File( param.outfile, "srsn" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of SRSN record
 */
     nadc_write_header( outfl, 0, param.infile,  
			 "Sun Reference Spectrum (update)" );

     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  (void) MJD_2_ASCII( srsn[nd].mjd.days, srsn[nd].mjd.secnd,
			      srsn[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, nr++, "DSR date", date_str );
	  nadc_write_uchar( outfl, nr++, "MDS DSR attached", 
			     srsn[nd].flag_mds );
	  nadc_write_text( outfl, nr++, "Sun spectrum ID", 
			    srsn[nd].sun_spec_id );
	  nadc_write_uchar( outfl, nr++, "Nue_den_filt_flag", 
			     srsn[nd].flag_neu );
	  count[0] = SCIENCE_CHANNELS;
	  count[1] = CHANNEL_SIZE;
	  nadc_write_arr_float( outfl, nr++, "Wavelength", 
				 2, count, 5, srsn[nd].wvlen_sun );
	  nadc_write_arr_float( outfl, nr++, "Mean reference spectrum", 
				 2, count, 5, srsn[nd].mean_sun );
	  nadc_write_arr_float( outfl, nr++, "Radiometric precision",
				 2, count, 5, srsn[nd].precision_sun );
	  nadc_write_arr_float( outfl, nr++, "Radiometric accuracy",
				 2, count, 5, srsn[nd].accuracy_sun );
	  nadc_write_arr_float( outfl, nr++, "Aperture etalon",
				 2, count, 5, srsn[nd].etalon );
	  nadc_write_float( outfl, nr++, 
			     "Average azimuth mirror position", 
			     5, srsn[nd].avg_asm );
	  nadc_write_float( outfl, nr++, 
			     "Average elevation mirror position", 
			     5, srsn[nd].avg_esm );
	  nadc_write_float( outfl, nr++, 
			     "Average Solar elevation angle", 
			     5, srsn[nd].avg_elev_sun );
	  count[0] = PMD_NUMBER;
	  nadc_write_arr_float( outfl, nr++, "Mean value of PMD", 
				 1, count, 5, srsn[nd].pmd_mean );
	  nadc_write_arr_float( outfl, nr++, "PMD out-of-band signal", 
				 1, count, 5, srsn[nd].pmd_out );
	  nadc_write_float( outfl, nr, "Doppler shift (500nm)", 
			     5, srsn[nd].dopp_shift );
     }
     (void) fclose( outfl );
}
