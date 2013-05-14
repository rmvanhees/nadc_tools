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

.IDENTifer   SCIA_LV1_CAL
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 1b/c product
.LANGUAGE    ANSI C
.PURPOSE     calibrate Science data
.INPUT/OUTPUT
  call as    SCIA_LV1_CAL( fp, calib_flag, state, mds_1b, mds_1c );
     input:
	    FILE   *fd                : (open) stream pointer
	    unsigned int calib_flag   : bit-flag which defines how to calibrate
	    struct state1_scia *state : structure with States of the product
            struct mds1_scia *mds_1b  : structure holding level 1b MDS records
    output:
            struct mds1c_scia *mds_1c : level 1c MDS records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      6.4   15-Mar-2011  add USE_SDMF_VERSION & fileParam.sdmf_version
                                 free allocated memory in fileParam, RvH
              6.3   30-Jul-2007  added m-factor correction, KB (Ife Bremen)
              6.2   14-Mar-2006  fixed stupid bug when n_pmd is zero, RvH
              6.1   24-Jan-2006  code clean-ups, RvH
                                 BUG FIX: data corruption in Coadd_PmdVal, RvH
              6.0   06-Dec-2005  implemented scaling of co-added readouts with 
                                 PMD measurements (non-linearity), RvH
              5.1   23-Nov-2005  implemented scaling of co-added readouts with 
                                 PMD measurements (memory correction), RvH
              5.0.1 17-Nov-2005  first steps towards implementing the usage of
                                 PMD signals for coadded pixels for 
				 Memory/Non-Linearity correction, RvH
              5.0   17-Feb-2005 do not try to read from not initialised 
                                SIP-record :-(, RvH
              4.9   23-Feb-2005 Apply SRON BDPM only on Epitaxx channels, RvH
              4.8   03-Feb-2005 added more Error checkings, 
                                use new calibration flags, RvH
              4.7   15-Dec-2004 Apply_RadNorm*: reflectivity are assumed to 
                                be normalized, therefore, no integration time 
                                is needed.
              4.6   20-Oct-2004 option 0+ requires nadc_tools with HDF5 support
              4.5   06-Oct-2004 modified structure mds1c_scia, RvH
              4.4   01-Sep-2004 added rough error estimation, RvH
              4.3   01-Sep-2004 implemented correction of the Epitaxx PET 
                                detectors, see TN-SCIA-0000DO/19,10.03.1999
              4.2.2 15-Aug-2004 minor bug fixes when reading key-datasets, RvH
              4.2.1 26-May-2004 bugfix: index offset with PMD_4 in SIP, RvH
              4.2   21-May-2004 make this module compile without HDF5, RvH
              4.1   21-May-2004 follow ATBD for limb-darkcorrection, RvH
                                (using the last limb-measurement at 150/250km)
              4.0   19-May-2004 added precission error calculation, RvH
              3.99  03-Dec-2003 bugfixes & several improvements, RvH
                                - mds_1b -> mds_1c done in separate module
                                - improved memory correction (nadc)
                                - added non-linearity correction (nadc)
                                - improved dark correction (atbd & nadc)
              3.6   08-Apr-2003 optimisation for pol/rad correction (10%), RvH
              3.5   28-Feb-2003 updated to latest dead pixel mask (Q.K.), RvH
              3.4   25-Feb-2003 added dark correction from database (Q.K.), RvH
              3.3   28-Jan-2003 added Doppler correction to SRS, RvH
              3.2   23-Jan-2003 added calculation of relectances, RvH
              3.1   15-Nov-2002 the calibration option may change from one call
                        to the next -> reinitialize the static variables, RvH
              3.0   07-Nov-2002 moved the geolocation routines to separate 
                        module (get_scia_lv1c_geo.c), RvH
              2.9   06-Nov-2002 handle nadir pixel which include the backscan
                        correctly, GET_SCIA_LV1C_GEON still not finished, RvH
              2.8   03-Nov-2002	finished interpolation of gelocation, RvH
                        EXCEPT for sub_sat_point & tang_ground_point
              2.7   01-Nov-2002	made inc(lude)-files safe against multiple 
                        inludes, RvH
              2.6   31-Oct-2002	sort polV struct by integration time, RvH
              2.5   30-Oct-2002	implemented interpolation of geolocations, RvH
              2.4   24-Oct-2002	bug fix: initialize fractional polarisation 
                        values correctly, RvH
              2.3   22-Oct-2002	bug fix: initialize geolocation of polV 
                        correctly, RvH
              2.2   15-Oct-2002	added documentation to included modules, RvH 
              2.1   15-Oct-2002	speed up the MDS calibration 
			(use of static variables), RvH
              2.0   15-Oct-2002	debugged the calibration routines 
			(Not Polarisation and Radiance), RvH
              1.9   06-Sep-2002	calculate geolocation for PMD correctly, RvH 
              1.8   09-Aug-2002	debugged extraction of level 1c 
                                PMD/PolV MDS, RvH
              1.7   07-Aug-2002	not count mds_pmd (=1) and mds_polV (=1), RvH 
              1.6   17-Jul-2002	put all calib-functions in includes, RvH
              1.5   16-Jul-2002	calibration routines in includes, RvH 
              1.4   26-Jun-2002	bug fixes + variable Leakage correction, RvH
              1.3   21-Jun-2002	added Leakage, PPG and Etalon correction, RvH
              1.2   19-Jun-2002	bug fixes + straylight correction, RvH 
              1.1   13-Jun-2002	implemented memory correction, RvH 
              1.0   17-May-2002	Initial release(!), Richard van Hees (SRON)
                                no selection, no calibration
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
#include <limits.h>
#include <math.h>
 
/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia_cal.h>

#define __NEED_GRID_ACCURACY__
#include "CalibModules/calibCalcError.inc"

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static
void initFileParam( const unsigned int calib_flag, FILE *fp, 
		    struct file_rec *fileParam )
{
     const char prognm[] = "initFileParam";

     struct mph_envi   mph;
     struct sph1_scia  sph;
     struct sip_scia   sip;

     int  previous_sdmf_version = fileParam->sdmf_version;

     char *sdmf_version = getenv( "USE_SDMF_VERSION" );
/*
 * always read MPH
 */
     ENVI_RD_MPH( fp, &mph );
     fileParam->dsd = (struct dsd_envi *)
	  malloc( (mph.num_dsd-1) * sizeof( struct dsd_envi ) );
     if ( fileParam->dsd == NULL )
          NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "fileParam->dsd" );
     fileParam->num_dsd = ENVI_RD_DSD( fp, mph, fileParam->dsd );
/*
 * get scale factor for memory/non-linearity correction
 */
     SCIA_LV1_RD_SPH( fp, mph, &sph );
     if ( strlen( sph.init_version ) > 0 ) {
	  if ( (calib_flag & DO_CORR_VIS_MEM) != UINT_ZERO 
	       && (calib_flag & DO_SRON_MEM_NLIN) != UINT_ZERO )
	       fileParam->memScale = SCIA_MEM_SCALE_PATCH;
	  else 
	       fileParam->memScale = SCIA_MEM_SCALE_ATBD;
	  if ( (calib_flag & DO_CORR_IR_NLIN) != UINT_ZERO 
	       && (calib_flag & DO_SRON_MEM_NLIN) != UINT_ZERO )
	       fileParam->nlinScale = SCIA_NLIN_SCALE_PATCH;
	  else 
	       fileParam->nlinScale = SCIA_NLIN_SCALE_ATBD;
     } else if ( strcmp( mph.proc_stage, "B" ) == 0 ) {
	  fileParam->memScale  = SCIA_MEM_SCALE_PATCH;
	  fileParam->nlinScale = SCIA_NLIN_SCALE_PATCH;
          NADC_ERROR( prognm, NADC_ERR_NONE,
                      "\n\tassume this is a SRON patched Sciamachy product" );
     } else {
	  fileParam->memScale  = SCIA_MEM_SCALE_OLD;
	  fileParam->nlinScale = SCIA_NLIN_SCALE_OLD;
	  NADC_ERROR( prognm, NADC_ERR_NONE,
		    "\n\tassume this is a Sciamachy product (version <= 5.x)" );
     }
/*
 * set SDMF version
 */
     if ( sdmf_version == NULL ) {
	  fileParam->sdmf_version = 30;                     /* default value */
     } else {
	  if ( strncmp( sdmf_version, "3.1", 3 ) == 0 )
	       fileParam->sdmf_version = 31;
	  else if ( strncmp( sdmf_version, "3.0", 3 ) == 0 )
	       fileParam->sdmf_version = 30;
	  else if ( strncmp( sdmf_version, "2.4", 3 ) == 0 )
	       fileParam->sdmf_version = 24;
	  else {
	       fileParam->sdmf_version = 30;
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, 
				  "Valid values for USE_SDMF_VERSION:"
				  " 2.4, 3.0 or 3.1" );
	  }
     }
/*
 * check if we have to initialize all parameters
 */
     if ( fp != fileParam->fp 
	  || strncmp( mph.product, fileParam->procName, 62 ) != 0 ) {
	  fileParam->flagInitFile = TRUE;             
     } else {                                              /* not a new file */
	  if ( calib_flag != fileParam->calibFlag
	       || fileParam->sdmf_version != previous_sdmf_version ) {
	       fileParam->flagInitFile = TRUE;
	  } else {
	       /* no initialisation needed */
	       return;
	  }
     }
/*
 * initialize all parameters
 */
     fileParam->flagInitPhase = TRUE;                      /* not used, yet */
     fileParam->fp = fp;
     (void) strlcpy( fileParam->procName, mph.product, ENVI_FILENAME_SIZE );
     (void) strlcpy( fileParam->procStage, mph.proc_stage, 2 );
     /* KB: Added for mfactors, needed for selection according to start time */
     (void) strlcpy( fileParam->sensing_start, mph.sensing_start, 
		     UTC_STRING_LENGTH );
     fileParam->absOrbit = mph.abs_orbit;
     fileParam->calibFlag = calib_flag;
/*
 * get static instrument parameters
 */
     (void) SCIA_LV1_RD_SIP( fp, fileParam->num_dsd, fileParam->dsd, &sip );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "SIP" );
     if ( (fileParam->calibFlag & DO_CALC_ERROR) != UINT_ZERO ) {
	  fileParam->ppgError   = sip.ppg_error;
	  fileParam->strayError = sip.stray_error;
     } else {
	  fileParam->ppgError   = -1.f;
	  fileParam->strayError = -1.f;
     }
     fileParam->alpha0_asm = sip.alpha0_asm;
     fileParam->alpha0_esm = sip.alpha0_esm;
     fileParam->lambda_end_gdf = sip.lambda_end_gdf;
     (void) strlcpy( fileParam->do_use_limb_dark, sip.do_use_limb_dark, 2 );
     (void) strlcpy( fileParam->do_pixelwise, sip.do_pixelwise, 
		     SCIENCE_CHANNELS+1 );
     (void) strlcpy( fileParam->do_pol_point, sip.do_pol_point, 
		     NUM_FRAC_POLV+1 );
     (void) strlcpy( fileParam->do_var_lc_cha, sip.do_var_lc_cha, 
		     4 * SCIENCE_CHANNELS + 1 );
     (void) strlcpy( fileParam->do_stray_lc_cha, sip.do_stray_lc_cha, 
		     4 * SCIENCE_CHANNELS + 1 );
     (void) memcpy( fileParam->electron_bu, sip.electrons_bu,
		    SCIENCE_CHANNELS * sizeof(float) );
     (void) memcpy( fileParam->level_2_smr, sip.level_2_smr, 
		    SCIENCE_CHANNELS + 1 );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_CAL( FILE *fp, 
		   unsigned int calib_flag, const struct state1_scia state[],
		   const struct mds1_scia *mds_1b, struct mds1c_scia *mds_1c )
{
     const char prognm[] = "SCIA_LV1_CAL";

     register unsigned short num = 0;

     static struct wvlen_rec wvlen;

     static struct file_rec fileParam = {
	 /* KB: empty string for sensing start added */
	  NULL, " ", " ", " ", " ", " ", " ", " ", " ",
	  TRUE, TRUE, TRUE, 30, (unsigned char) 0, (unsigned char) 0, 
	  -1, 0u, 0u, 0.f, 0.f, 0.f, 0.f, 0.f,
	  {(unsigned char) 0, (unsigned char) 0, (unsigned char) 0, 
	   (unsigned char) 0, (unsigned char) 0, (unsigned char) 0, 
	   (unsigned char) 0, (unsigned char) 0},
	  {1e10, 1e10, 1e10, 1e10, 1e10, 1e10, 1e10, 1e10}, NULL
     };

     const int do_calc_wave =
         ((calib_flag & (DO_CALIB_WAVE|DO_CORR_POL|DO_CORR_RAD)) != UINT_ZERO);
     const int do_corr_dark =
         ((calib_flag & (DO_CORR_AO|DO_CORR_DARK|DO_CORR_VDARK|DO_CORR_VSTRAY))
          != UINT_ZERO);
/*
 * Any calibration needed?
 */
     if ( calib_flag == CALIB_NONE ) return;
/*
 * initialize file parameter structure
 */
     initFileParam( calib_flag, fp, &fileParam );
/*
 * set radiation unit flag
 */
     if ( (calib_flag & DO_CORR_RAD) != UINT_ZERO ) {
	  do {
	       mds_1c[num].rad_units_flag = (char) -1;
	  } while ( ++num < state->num_clus );
     }
/*
 * recalculate only when orbit_phase has changed
 *
 * read Sun Reference Spectra DSDs
 */
     if ( (fileParam.flagInitFile || fileParam.flagInitPhase) && do_calc_wave )
	  SCIA_ATBD_INIT_WAVE( &fileParam, state->orbit_phase, &wvlen );
/*
 * apply memory correction (chan 1-5)
 */
     if ( (calib_flag & DO_CORR_VIS_MEM) != UINT_ZERO ) {
	  SCIA_ATBD_CAL_MEM( fileParam.memScale, state, mds_1b, mds_1c );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "MEM" );
     }
/*
 * apply non-Linearity correction (chan 6-8)
 */
     if ( (calib_flag & DO_CORR_IR_NLIN) != UINT_ZERO ) {
	  SCIA_ATBD_CAL_NLIN( fileParam.nlinScale, state, mds_1b, mds_1c );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "NLIN" );
     }
/*
 * apply Dark correction
 */
     if ( do_corr_dark ) {
	  if ( (calib_flag & DO_CORR_ADARK) != UINT_ZERO )
	       SCIA_STATE_CAL_DARK( &fileParam, state, mds_1c );
	  else if ( (calib_flag & DO_SRON_DARK) != UINT_ZERO )
	       SCIA_SRON_CAL_DARK( &fileParam, state, mds_1c );
	  else
	       SCIA_ATBD_CAL_DARK( &fileParam, state, mds_1c );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "DARK" );
     }
/*
 * estimate measurement noise (channel 8)
 */
     if ( (calib_flag & DO_SRON_NOISE) != UINT_ZERO ) {
	  SCIA_SRON_CAL_NOISE( &fileParam, state, mds_1c );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "NOISE" );
     }
/*
 * apply PPG Correction
 */
     if ( (calib_flag & DO_CORR_PPG) != UINT_ZERO ) {
	  if ( (calib_flag & DO_SRON_PPG) != UINT_ZERO )
	       SCIA_SRON_CAL_PPG( &fileParam, state, mds_1c );
	  else
	       SCIA_ATBD_CAL_PPG( &fileParam, state, mds_1c );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "PPG" );
     }
/*
 * apply Etalon Correction
 */
     if ( (calib_flag & DO_CORR_ETALON) != UINT_ZERO ) {
	  SCIA_ATBD_CAL_ETALON( &fileParam, state, mds_1c );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "Etalon" );
     }
/*
 * apply Straylight Correction
 */
     if ( (calib_flag & DO_CORR_STRAY) != UINT_ZERO ) {
	  SCIA_ATBD_CAL_STRAY( fileParam.strayError, state, mds_1b, mds_1c );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "StrayLight" );
     }
/*
 * calculate Precision Error on spectra
 */
     if ( (calib_flag & DO_SRON_NOISE) == UINT_ZERO
	  && (calib_flag & DO_CALC_ERROR) != UINT_ZERO )
	  calcSpectralAccuracy( state, mds_1c );
/*
 * calculate Wavelength Grid
 */
     if ( (calib_flag & DO_CALIB_WAVE) != UINT_ZERO ) {
	  SCIA_ATBD_CAL_WAVE( wvlen, state, mds_1c );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "WAVE" );
     }
/*
 * apply Polarisation Correction
 */
     if ( (calib_flag & DO_CORR_POL) != UINT_ZERO ) {
	  SCIA_ATBD_CAL_POL( &fileParam, wvlen, state, mds_1b, mds_1c );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "POL" );
     }
/*
 * apply Radiance Sensitivity Correction
 */
     if ( (calib_flag & DO_CORR_RAD) != UINT_ZERO ) {
	  SCIA_ATBD_CAL_RAD( &fileParam, wvlen, state, mds_1c );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "RAD" );
     }
/*
 * covert Radiances to Reflectances
 */
     if ( (calib_flag & DO_DIVIDE_SUN) != UINT_ZERO ) {
	  if ( (calib_flag & DO_SRON_SUN) != UINT_ZERO ) {
               SCIA_SRON_CAL_REFL( &fileParam, state, mds_1c );
	  } else {
               SCIA_ATBD_CAL_REFL( &fileParam, state, mds_1c );
          }
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "REFL" );
     }
/*
 * apply Bad Pixel Mask
 */
     if ( (calib_flag & DO_MASK_BDPM) != UINT_ZERO ) {
	  if ( (calib_flag & DO_SRON_BDPM) != UINT_ZERO )
	       SCIA_SRON_FLAG_BDPM( &fileParam, state, mds_1c );
	  else
	       SCIA_ATBD_FLAG_BDPM( &fileParam, state, mds_1c );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "BDPM" );
     }
/*
 * apply Transmission correction
 */
     if ( (calib_flag & DO_SRON_TRANS) != UINT_ZERO ) {
	  SCIA_SRON_CAL_TRANS( &fileParam, state, mds_1c );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "TRANSMISSION" );
     }
done:
     fileParam.flagInitFile = FALSE;
     free( fileParam.dsd );
}
