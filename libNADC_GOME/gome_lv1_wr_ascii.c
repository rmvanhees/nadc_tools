/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 1999 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GOME_LV1_WR_ASCII
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1
.LANGUAGE    ANSI C
.PURPOSE     Dump the contents of a GOME level 1b file in ASCII
.RETURNS     Nothing
.COMMENTS    contains GOME_LV1_WR_ASCII_FSR, GOME_LV1_WR_ASCII_SPH, 
               GOME_LV1_WR_ASCII_FCD, GOME_LV1_WR_ASCII_PCD, 
	       GOME_LV1_WR_ASCII_SMCD, GOME_LV1_WR_ASCII_REC
.ENVIRONment None
.VERSION      5.0   08-Jun-2009 update to product version 2, RvH
              4.0   17-Nov-2005	add start and stop time derived from the PCD
	                        records to SPH output and increased the number 
				of digits of several parameters.
				Improved the dump of the PCD_PMD records, RvH
              3.2   10-Aug-2005	write MJD as DateTime & Julian days, RvH
              3.1   10-Nov-2002	use new CalibStr routine, RvH
              3.0   11-Nov-2001	moved to the new Error handling routines, RvH 
              2.3   19-Jul-2001	pass structures using pointers, RvH 
              2.2   11-Jul-2001 removed bugs in GOME_LV1_WR_ASCII_REC, RvH
              2.1   18-May-2001 added wavelength to GOME_LV1_WR_ASCII_REC
              2.0   29-Aug-2000 major rewrite and standardization, RvH
              1.2   18-Feb-2000 renamed: DEBUG -> GOME_LV1_WR_ASCII, RvH
              1.1   17-Feb-1999 write debug info to file, RvH
              1.0   02-Mar-1999 created by R. M. van Hees
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
#define _GOME_COMMON
#include <nadc_gome.h>

#define CBUFF_SIZE     50

static const double Msec2DecimalDay = 1000 * 24. * 60. * 60.;

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   GOME_LV1_WR_ASCII_FSR
.PURPOSE     dump content of File Structure Record
.INPUT/OUTPUT
  call as   GOME_LV1_WR_ASCII_FSR( param, fsr );
     input:
            struct param_record param : struct holding user-defined settings
	    struct fsr1_gome *fsr     : structure for FSR record

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void GOME_LV1_WR_ASCII_FSR( struct param_record param, 
			    const struct fsr1_gome *fsr )
{
     const char prognm[] = "GOME_LV1_WR_ASCII_FSR";

     register unsigned short nb;
     register unsigned int nr = 0;

     char cbuff[CBUFF_SIZE];

     const char *band_names[] = { 
	  "Band-1a", "Band-1b", "Band-2a", "Band-2b", "Band-3", 
	  "Band-4", "Blind", "Stray-1a", "Stray-1b", "Stray-2a"
     };

     FILE *outfl = CRE_ASCII_File( param.outfile, "fsr" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
          NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of FSR record
 */
     nadc_write_header( outfl, nr, param.infile, "File Structure Record" );
     nadc_write_short( outfl, ++nr, "Number of SPH1 records", fsr->nr_sph );
     nadc_write_int( outfl, ++nr, "Length of SPH1 record", fsr->sz_sph );
     nadc_write_short( outfl, ++nr, "Number of FCD records", fsr->nr_fcd );
     nadc_write_int( outfl, ++nr, "Length of FCD record", fsr->sz_fcd );
     nadc_write_short( outfl, ++nr, "Number of PCD records", fsr->nr_pcd );
     nadc_write_int( outfl, ++nr, "Length of PCD record", fsr->sz_pcd );
     nadc_write_short( outfl, ++nr, "Number of SCD records", fsr->nr_scd );
     nadc_write_int( outfl, ++nr, "Length of SCD record", fsr->sz_scd );
     nadc_write_short( outfl, ++nr, "Number of MCD records", fsr->nr_mcd );
     nadc_write_int( outfl, ++nr, "Length of MCD record", fsr->sz_mcd );
     for ( nb = 0; nb < (unsigned short) NUM_SPEC_BANDS; nb++ ) {
	  (void) snprintf( cbuff, CBUFF_SIZE, "Number of %s data records", 
			   band_names[nb] );
	  nadc_write_short( outfl, ++nr, cbuff, fsr->nr_band[nb] );
	  (void) snprintf( cbuff, CBUFF_SIZE, "Length of %s data record", 
			  band_names[nb] );
	  nadc_write_int( outfl, nr, cbuff, fsr->sz_band[nb] );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer   GOME_LV1_WR_ASCII_SPH
.PURPOSE     dump content of Specific Product Header
.INPUT/OUTPUT
  call as   GOME_LV1_WR_ASCII_SPH( param, sph );
     input:
            struct param_record param : struct holding user-defined settings
	    struct sph1_gome *sph     : structure for Specific Product Header

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void GOME_LV1_WR_ASCII_SPH( struct param_record param, 
			    const struct sph1_gome *sph )
{
     const char prognm[] = "GOME_LV1_WR_ASCII_SPH";

     register short ni;
     register unsigned int nr = 0;

     char date_str[DATE_STRING_LENGTH];

     unsigned int count[2];

     FILE *outfl = CRE_ASCII_File( param.outfile, "sph" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
          NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of SPH record
 */
     nadc_write_header( outfl, nr, param.infile, "Specific Product Header" );
     (void) UTC_2_ASCII( sph->start_time.days, sph->start_time.msec, 
			 date_str );
     nadc_write_text( outfl, nr, "Start Time", date_str );
     (void) UTC_2_ASCII( sph->stop_time.days, sph->stop_time.msec, 
			 date_str );
     nadc_write_text( outfl, nr, "Stop Time", date_str );
/* 01 */
     nadc_write_short( outfl, ++nr, "Number of Input Data References",
			sph->nr_inref );
/* 02 */
     for ( ni = 0; ni < sph->nr_inref; ni++ )
	  nadc_write_text(outfl, ++nr, "Input data reference", sph->inref[ni]);
/* 03 */
     nadc_write_text( outfl, ++nr, "Software version", sph->soft_version );
/* 04 */
     nadc_write_text( outfl, ++nr, "Calibration version", sph->calib_version );
/* 05 */
     nadc_write_short( outfl, ++nr, "Product Format version", 
			sph->prod_version );
/* 06 */
     nadc_write_uint( outfl, ++nr, "Orbit number", sph->time_orbit );
     (void) UTC_2_ASCII( sph->time_utc_day, sph->time_utc_ms, 
			 date_str );
     nadc_write_text( outfl, nr, "MJD (Date Time)", date_str );
     nadc_write_double( outfl, nr, "MJD (Julian Day)", 14,
			(double) sph->time_utc_day +
			(sph->time_utc_ms / Msec2DecimalDay) );
     nadc_write_uint( outfl, nr, "Satellite binary counter",
		       sph->time_counter );
     nadc_write_uint( outfl, nr, "Satellite binary counter period", 
		       sph->time_period );
/* 07 */
     nadc_write_short( outfl, ++nr, "SDP PMD entry", sph->pmd_entry );
/* 08 */
     nadc_write_short( outfl, ++nr, "SDP SubSet counter entry", 
			sph->subset_entry );
/* 09 */
     nadc_write_short( outfl, ++nr, "SDP Integration Status entry", 
			sph->intgstat_entry );
/* 10 */
     nadc_write_short( outfl, ++nr, "SDP Peltier entry", sph->peltier_entry );
/* 11 */
     nadc_write_short( outfl, ++nr, "SDP Instrument Status_2 entry", 
			sph->status2_entry );
/* 12 */
     count[0] = 2;
     count[1] = PMD_NUMBER;
     nadc_write_arr_float( outfl, ++nr, "PMD's conversion Factors",
			    2, count, 4, sph->pmd_conv );
/* 13 */
     nadc_write_uint( outfl, ++nr, "STATE UTC day", sph->state_utc_day );
     nadc_write_uint( outfl, nr, "STATE UTC ms", sph->state_utc_ms );
     nadc_write_uint( outfl, nr, "STATE orbit", sph->state_orbit );
     nadc_write_float( outfl, nr, "STATE x", 4, sph->state_x );
     nadc_write_float( outfl, nr, "STATE y", 4, sph->state_y );
     nadc_write_float( outfl, nr, "STATE z", 4, sph->state_z );
     nadc_write_float( outfl, nr, "STATE dx", 8, sph->state_dx );
     nadc_write_float( outfl, nr, "STATE dy", 8, sph->state_dy );
     nadc_write_float( outfl, nr, "STATE dz", 8, sph->state_dz );
/* 14 */
     nadc_write_double( outfl, ++nr, "ATT yaw", 8, sph->att_yaw );
     nadc_write_double( outfl, nr, "ATT pitch", 8, sph->att_pitch );
     nadc_write_double( outfl, nr, "ATT roll", 8, sph->att_roll );
     nadc_write_double( outfl, nr, "ATT yaw rate", 8, sph->att_dyaw );
     nadc_write_double( outfl, nr, "ATT pitch rate", 8, sph->att_dpitch );
     nadc_write_double( outfl, nr, "ATT roll rate", 8, sph->att_droll );
     nadc_write_int( outfl, nr, "ATT attitude flag", sph->att_flag );
     nadc_write_int( outfl, nr, "ATT status", sph->att_stat );
/* 15 */
     nadc_write_double( outfl, ++nr, "Julian (MJD50)", 8, sph->julian );
     nadc_write_double( outfl, nr, "Semi Major axis", 8, sph->semi_major );
     nadc_write_double( outfl, nr, "Excentricity", 8, sph->excen );
     nadc_write_double( outfl, nr, "Inclination", 8, sph->incl );
     nadc_write_double( outfl, nr, "Right ascention", 8, sph->right_asc );
     nadc_write_double( outfl, nr, "Perigee", 8, sph->perigee );
     nadc_write_double( outfl, nr, "Mean anomaly", 8, sph->mn_anom );
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer   GOME_LV1_WR_ASCII_FCD
.PURPOSE     dump content of Fixed Calibration Data
.INPUT/OUTPUT
  call as   GOME_LV1_WR_ASCII_FCD( param, fcd );
     input:
            struct param_record param : struct holding user-defined settings
            struct fcd_gome *fcd      : structure for Fixed Calibration Data

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void GOME_LV1_WR_ASCII_FCD( struct param_record param, 
			    const struct fcd_gome *fcd )
{
     const char prognm[] = "GOME_LV1_WR_ASCII_FCD";

     register short nl;
     register unsigned nr = 0;

     char date_str[25];
     unsigned int count[2];
     unsigned int uibuff[3];

     FILE *outfl = CRE_ASCII_File( param.outfile, "fcd" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
          NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of FCD record
 */
     nadc_write_header( outfl, nr, param.infile, 
			 "Fixed Calibration Record Header" );
/* 01 */
     nadc_write_uint( outfl, ++nr, "Channel 1", 
		      fcd->detector_flags.flag_fields.array_1 );
     nadc_write_uint( outfl, nr, "Channel 2", 
		      fcd->detector_flags.flag_fields.array_2 );
     nadc_write_uint( outfl, nr, "Channel 3", 
		      fcd->detector_flags.flag_fields.array_3 );
     nadc_write_uint( outfl, nr, "Channel 4", 
		      fcd->detector_flags.flag_fields.array_4 );
     nadc_write_uint( outfl, nr, "PMD 1", 
		      fcd->detector_flags.flag_fields.pmd_1 );
     nadc_write_uint( outfl, nr, "PMD 2", 
		      fcd->detector_flags.flag_fields.pmd_2 );
     nadc_write_uint( outfl, nr, "PMD 3", 
		      fcd->detector_flags.flag_fields.pmd_3 );
/* 02 */
     count[0] = 3;
     uibuff[0] = (unsigned int) fcd->bcr[0].array_nr;
     uibuff[1] = (unsigned int) fcd->bcr[0].start;
     uibuff[2] = (unsigned int) fcd->bcr[0].end;
     nadc_write_arr_uint( outfl, ++nr, "BCR band 1a", 1, count, uibuff );
     uibuff[0] = (unsigned int) fcd->bcr[1].array_nr;
     uibuff[1] = (unsigned int) fcd->bcr[1].start;
     uibuff[2] = (unsigned int) fcd->bcr[1].end;
     nadc_write_arr_uint( outfl, nr, "BCR band 1b", 1, count, uibuff );
     uibuff[0] = (unsigned int) fcd->bcr[2].array_nr;
     uibuff[1] = (unsigned int) fcd->bcr[2].start;
     uibuff[2] = (unsigned int) fcd->bcr[2].end;
     nadc_write_arr_uint( outfl, nr, "BCR band 2a", 1, count, uibuff );
     uibuff[0] = (unsigned int) fcd->bcr[3].array_nr;
     uibuff[1] = (unsigned int) fcd->bcr[3].start;
     uibuff[2] = (unsigned int) fcd->bcr[3].end;
     nadc_write_arr_uint( outfl, nr, "BCR band 2b", 1, count, uibuff );
     uibuff[0] = (unsigned int) fcd->bcr[4].array_nr;
     uibuff[1] = (unsigned int) fcd->bcr[4].start;
     uibuff[2] = (unsigned int) fcd->bcr[4].end;
     nadc_write_arr_uint( outfl, nr, "BCR band 3", 1, count, uibuff );
     uibuff[0] = (unsigned int) fcd->bcr[5].array_nr;
     uibuff[1] = (unsigned int) fcd->bcr[5].start;
     uibuff[2] = (unsigned int) fcd->bcr[5].end;
     nadc_write_arr_uint( outfl, nr, "BCR band 4", 1, count, uibuff );
     uibuff[0] = (unsigned int) fcd->bcr[6].array_nr;
     uibuff[1] = (unsigned int) fcd->bcr[6].start;
     uibuff[2] = (unsigned int) fcd->bcr[6].end;
     nadc_write_arr_uint( outfl, nr, "BCR blind 1a", 1, count, uibuff );
     uibuff[0] = (unsigned int) fcd->bcr[7].array_nr;
     uibuff[1] = (unsigned int) fcd->bcr[7].start;
     uibuff[2] = (unsigned int) fcd->bcr[7].end;
     nadc_write_arr_uint( outfl, nr, "BCR stray 1a", 1, count, uibuff );
     uibuff[0] = (unsigned int) fcd->bcr[8].array_nr;
     uibuff[1] = (unsigned int) fcd->bcr[8].start;
     uibuff[2] = (unsigned int) fcd->bcr[8].end;
     nadc_write_arr_uint( outfl, nr, "BCR stray 1b", 1, count, uibuff );
     uibuff[0] = (unsigned int) fcd->bcr[9].array_nr;
     uibuff[1] = (unsigned int) fcd->bcr[9].start;
     uibuff[2] = (unsigned int) fcd->bcr[9].end;
     nadc_write_arr_uint( outfl, nr, "BCR stray 2a", 1, count, uibuff );
/* 03 */
     count[0] = SCIENCE_CHANNELS;
     nadc_write_arr_float( outfl, ++nr, "KDE_BSDF_1", 1, count, 4,
			   fcd->kde.bsdf_1 );
     nadc_write_arr_float( outfl, nr, "KDE_BSDF_2", 1, count, 4,
			   fcd->kde.bsdf_2 );
     nadc_write_arr_float( outfl, nr, "KDE_RESPONSE_1", 1, count, 4,
			   fcd->kde.resp_1 );
     nadc_write_arr_float( outfl, nr, "KDE_RESPONSE_2", 1, count, 4,
			   fcd->kde.resp_2 );
     nadc_write_arr_float( outfl, nr, "KDE_F2_1", 1, count, 4,
			   fcd->kde.f2_1 );
     nadc_write_arr_float( outfl, nr, "KDE_F2_2", 1, count, 4, 
			   fcd->kde.f2_2 );
     nadc_write_arr_float( outfl, nr, "KDE_SMDEP_1", 1, count, 4, 
			   fcd->kde.smdep_1 );
     nadc_write_arr_float( outfl, nr, "KDE_SMDEP_2", 1, count, 4, 
			   fcd->kde.smdep_2 );
     nadc_write_arr_float( outfl, nr, "KDE_CHI_1", 1, count, 4, 
			   fcd->kde.chi_1 );
     nadc_write_arr_float( outfl, nr, "KDE_CHI_2", 1, count, 4, 
			   fcd->kde.chi_2 );
     nadc_write_arr_float( outfl, nr, "KDE_ETA_1", 1, count, 4, 
			   fcd->kde.eta_1 );
     nadc_write_arr_float( outfl, nr, "KDE_ETA_2", 1, count, 4, 
			   fcd->kde.eta_2 );
     nadc_write_arr_float( outfl, nr, "KDE_KSI_1", 1, count, 4, 
			   fcd->kde.ksi_1 );
     nadc_write_arr_float( outfl, nr, "KDE_KSI_2", 1, count, 4, 
			   fcd->kde.ksi_2 );
     count[0] = CHANNEL_SIZE;
     count[1] = SCIENCE_CHANNELS;
     nadc_write_arr_float( outfl, nr, "KDE_Response_f2_SMDep_1", 2, count, 
			   6, fcd->kde.rfs );
/* 04 */
     nadc_write_float( outfl, ++nr, "BSDF_0", 6, fcd->bsdf_0 );
     nadc_write_float( outfl, nr, "Elevation", 6, fcd->elevation );
     nadc_write_float( outfl, nr, "Azimuth", 6, fcd->azimuth );
     count[0] = 2;
     count[1] = SCIENCE_CHANNELS;
     nadc_write_arr_float( outfl, nr, "Coefficients", 2, count, 6, 
			   fcd->coeffs );
/* 05 */
     count[0] = SCIENCE_CHANNELS;
     nadc_write_arr_float( outfl, ++nr, "Uniform straylight level", 
			   1, count, 4, fcd->stray_level );
/* 06 */
     count[1] = NUM_STRAY_GHOSTS;
     nadc_write_arr_short( outfl, nr, "Ghost symmetry", 
			   2, count, (short *) fcd->ghost.symmetry );
     nadc_write_arr_short( outfl, nr, "Ghost centre", 
			   2, count, (short *) fcd->ghost.center );
     nadc_write_arr_float( outfl, nr, "Ghost Defocusing", 
			   2, count, 4, (float *) fcd->ghost.defocus );
     nadc_write_arr_float( outfl, nr, "Ghost Energy", 
			   2, count, 4, (float *) fcd->ghost.energy );
/* 07 */
     nadc_write_short( outfl, ++nr, "TriangleConvolutionWidth", 
			fcd->width_conv );
/* 08 */
     count[0] = NUM_FPA_SCALE;
     nadc_write_arr_float( outfl, ++nr, "FPA ScaleFactor", 
			   1, count, 4, fcd->scale_peltier );
/* 09 */
     nadc_write_short( outfl, ++nr, "FPA CoefficientsUsed", fcd->npeltier );
/* 10 */
     count[0] = (unsigned int) fcd->npeltier;
     nadc_write_arr_float( outfl, ++nr, "FPA FilterCoefficient", 
			   1, count, 6, fcd->filter_peltier );
/* 11 */
     nadc_write_short( outfl, ++nr, "Number of Leakage parameters", 
			fcd->nleak );
/* 12 */
     for ( nr++, nl = 0; nl < fcd->nleak; nl++ ) {
	  nadc_write_short( outfl, nr, "Leakage Parameter", nl );
	  nadc_write_float( outfl, nr, "Array Noise", 
			    6, fcd->leak[nl].noise );
	  count[0] = PMD_NUMBER;
	  nadc_write_arr_float( outfl, nr, "PMD Offset", 1, count, 6,
				fcd->leak[nl].pmd_offs );
	  nadc_write_float( outfl, nr, "PMD Noise", 6, 
			    fcd->leak[nl].pmd_noise );
	  count[0] = CHANNEL_SIZE;
	  count[1] = SCIENCE_CHANNELS;
	  nadc_write_arr_float( outfl, nr, "Dark Current", 2, count, 6,
				fcd->leak[nl].dark );
     }
/* 13 */
     count[0] = CHANNEL_SIZE;
     count[1] = SCIENCE_CHANNELS;
     nadc_write_arr_float( outfl, ++nr, "Pixel-to-Pixel Gain", 2, count, 6,
			   fcd->pixel_gain );
/* 14 */
     nadc_write_short( outfl, ++nr, "Number of Hot Pixel(s)", fcd->nhot );
/* 15 */
     for ( nr++, nl = 0; nl < fcd->nhot; nl++ ) {
	  nadc_write_short( outfl, nr, "Hot Pixel Occurrence", nl );
	  nadc_write_short( outfl, nr, "Record", fcd->hot[nl].record);
	  nadc_write_short( outfl, nr, "Array", fcd->hot[nl].array);
	  nadc_write_short( outfl, nr, "Pixel", fcd->hot[nl].pixel );
     }
/* 16 */
     nadc_write_short( outfl, ++nr, "Number of Spectral Paramater", 
		       fcd->nspec );
/* 17 */
     for ( ++nr, nl = 0; nl < fcd->nspec; nl++ ) {
	  nadc_write_short( outfl, nr, "Spectral Parameter", nl );
	  count[0] = NUM_SPEC_COEFFS;
	  count[1] = SCIENCE_CHANNELS;
	  nadc_write_arr_double( outfl, nr, "Polynomial coefficients", 2, 
				 count, 6, (double *)fcd->spec[nl].coeffs );
	  count[0] = SCIENCE_CHANNELS;
	  nadc_write_arr_double( outfl, nr, "Average pixel deviation", 1, 
				 count, 6, (double *) fcd->spec[nl].error );
     }
/* 18 */
     nadc_write_int( outfl, ++nr, "Spectral Calibration Index", 
		      fcd->indx_spec );
/* 19 */
     count[0] = CHANNEL_SIZE;
     count[1] = SCIENCE_CHANNELS;
     nadc_write_arr_float( outfl, ++nr, "Instrument Response", 
			   2, count, 6, fcd->intensity );
/* 20 */
     nadc_write_arr_float( outfl, ++nr, "Sun Mean Reference Spectrum", 
			   2, count, 6, fcd->sun_ref );
/* 21 */
     nadc_write_arr_float( outfl, ++nr, "Sun Mean Reference Relative Error", 
			   2, count, 6, fcd->sun_precision );
/* 22  & 23 */
     count[0] = PMD_NUMBER;
     nadc_write_arr_float( outfl, ++nr, "Sun mean PMD value", 
			   1, count, 6, fcd->sun_pmd );
     nadc_write_arr_float( outfl, ++nr, "Wavelength of Sun mean PMD value", 
			   1, count, 6, fcd->sun_pmd_wv );
/* 24 */
     (void) UTC_2_ASCII( fcd->sun_date, fcd->sun_time, date_str );
     nadc_write_text( outfl, ++nr, "Sun Mean Reference Spectrum Date",
		      date_str );
/* 25 */
     nadc_write_short( outfl, ++nr, "Number of scan mirror angle(s)", 
		       fcd->nang );
/* 26 */
     count[0] = CHANNEL_SIZE;
     for ( ++nr, nl = 0; nl < fcd->nang; nl++ ) {
	  nadc_write_short( outfl, nr, "Pre-flight Calibration", nl );
	  nadc_write_arr_float( outfl, nr, "Eta Omega", 
				1, count, 6, fcd->calib[nl].eta_omega );
	  nadc_write_arr_float( outfl, nr, "Radiance Response", 
				1, count, 10, fcd->calib[nl].response );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer   GOME_LV1_WR_ASCII_PCD
.PURPOSE     dump content of (Earth) Pixel Calibration Data
.INPUT/OUTPUT
  call as   GOME_LV1_WR_ASCII_PCD( param, nr_pcd, indx_pcd, pcd );
     input:
            struct param_record param : struct holding user-defined settings
            short nr_pcd              : number of Pixel Calibration Records
	    short *indx_pcd           : indices to selected PCDs
	    struct pcd_gome *pcd      : structure for Pixel Calibration Records

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void GOME_LV1_WR_ASCII_PCD( struct param_record param, short nr_pcd, 
			    const short *indx_pcd, 
			    const struct pcd_gome *pcd )
{
     register short ni;
     register unsigned nr = 0;

     char date_str[25];
     unsigned int count[2];

     const char prognm[] = "GOME_LV1_WR_ASCII_PCD";

     FILE *outfl = CRE_ASCII_File( param.outfile, "pcd" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
          NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of PCD record
 */
     nadc_write_header( outfl, nr, param.infile, 
			 "Pixel Specific Calibration Records" );

     for ( ni = 0; ni < nr_pcd; ni++ ) {
	  register short nii = indx_pcd[ni];

	  nadc_write_short( outfl, 0, "Index of PCD record", nii );
/* GLR-01 */
	  nr = 1;
	  (void) UTC_2_ASCII( pcd[nii].glr.utc_date, 
			      pcd[nii].glr.utc_time, date_str );
	  nadc_write_text( outfl, nr, "MJD (Date Time)", date_str );
	  nadc_write_double( outfl, nr, "MJD (Julian Day)", 14,
			     (double) pcd[nii].glr.utc_date +
			     (pcd[nii].glr.utc_time / Msec2DecimalDay) );
/* GLR-02 */
	  count[0] = 3;
	  nadc_write_arr_float( outfl, nr, "Solar zenith angles w.r.t. North", 
				1, count, 5, pcd[nii].glr.sun_zen_sat_north );
	  nadc_write_arr_float( outfl, nr, "Solar azimuth angles w.r.t. North",
				1, count, 5, pcd[nii].glr.sun_azim_sat_north );
/* GLR-03 */
	  nadc_write_arr_float( outfl, nr, 
				"Line-of-Sight zenith w.r.t. North", 
				1, count, 5, pcd[nii].glr.los_zen_sat_north );
	  nadc_write_arr_float( outfl, nr, 
				"Line-of-Sight azimuth w.r.t. North",
				1, count, 5, pcd[nii].glr.los_azim_sat_north );
/* GLR-04 */
	  nadc_write_arr_float( outfl, nr, "Solar zenith w.r.t. satellite", 
				1, count, 5, pcd[nii].glr.sun_zen_sat_ers );
	  nadc_write_arr_float( outfl, nr, "Solar azimuth w.r.t. satellite",
				1, count, 5, pcd[nii].glr.sun_azim_sat_ers );
/* GLR-05 */
	  nadc_write_arr_float( outfl, nr, 
				"Line-of-Sight zenith w.r.t. satellite", 
				1, count, 5, pcd[nii].glr.los_zen_sat_ers );
	  nadc_write_arr_float( outfl, nr, 
				"Line-of-Sight azimuth w.r.t. satellite",
				1, count, 5, pcd[nii].glr.los_azim_sat_ers );
/* GLR-06 */
	  nadc_write_arr_float( outfl, nr, 
				"Solar zenith at surface w.r.t. North", 
				1, count, 5, pcd[nii].glr.los_zen_surf_north );
	  nadc_write_arr_float( outfl, nr, 
				"Solar azimuth at surface w.r.t. North",
				1, count, 5, pcd[nii].glr.los_azim_surf_north );
/* GLR-07 */
	  nadc_write_arr_float( outfl, nr, 
				"Line-of-Sight zenith at surface w.r.t. North", 
				1, count, 5, pcd[nii].glr.los_zen_surf_north );
	  nadc_write_arr_float( outfl, nr, 
				"Line-of-Sight azimuth at surface w.r.t. North",
				1, count, 5, pcd[nii].glr.los_azim_surf_north );
/* GLR-08 */
	  nadc_write_float( outfl, nr, "Satellite geodetic height",
			    5, pcd[nii].glr.sat_geo_height );
/* GLR-09 */
	  nadc_write_float( outfl, nr, "Earth radius of curvature",
			    5, pcd[nii].glr.earth_radius );
/* GLR-10 */
	  nadc_write_uchar( outfl, nr, "Posible Sun-glint",
			    (unsigned char) pcd[nii].glr.sun_glint );
/* GLR-11 */
	  count[0] = NUM_COORDS;
	  nadc_write_arr_float( outfl, nr, "Latitude of ground pixel", 
				1, count, 6, pcd[nii].glr.lat );
	  nadc_write_arr_float( outfl, nr, "Longitude of ground pixel", 
				1, count, 6, pcd[nii].glr.lon );
/* CR1-01 */
	  nadc_write_short( outfl, ++nr, "Cloud mode", pcd[nii].cld.mode );
/* CR1-02 */
	  nadc_write_float( outfl, nr, "Cloud surfaceHeight",
			    6, pcd[nii].cld.surfaceHeight );
/* CR1-03 */
	  nadc_write_float( outfl, nr, "Cloud fraction",
			    6, pcd[nii].cld.fraction );
/* CR1-04 */
	  nadc_write_float( outfl, nr, "Cloud fractionError",
			    6, pcd[nii].cld.fractionError );
/* CR1-05 */
	  nadc_write_float( outfl, nr, "Cloud albedo",
			    6, pcd[nii].cld.albedo );
/* CR1-06 */
	  nadc_write_float( outfl, nr, "Cloud albedoError",
			    6, pcd[nii].cld.albedoError );
/* CR1-07 */
	  nadc_write_float( outfl, nr, "Cloud height",
			    6, pcd[nii].cld.height );
/* CR1-08 */
	  nadc_write_float( outfl, nr, "Cloud heightError",
			    6, pcd[nii].cld.heightError );
/* CR1-09 */
	  nadc_write_float( outfl, nr, "Cloud thickness",
			    6, pcd[nii].cld.thickness );
/* CR1-10 */
	  nadc_write_float( outfl, nr, "Cloud thicknessError",
			    6, pcd[nii].cld.thicknessError );
/* CR1-11 */
	  nadc_write_float( outfl, nr, "Cloud topPress",
			    6, pcd[nii].cld.topPress );
/* CR1-12 */
	  nadc_write_float( outfl, nr, "Cloud topPressError",
			    6, pcd[nii].cld.topPressError );
/* CR1-13 */
	  nadc_write_short( outfl, nr, "Cloud type", pcd[nii].cld.type );
/* 03 */
	  nadc_write_float( outfl, ++nr, "Dark Current", 
			     6, pcd[nii].dark_current );
	  nadc_write_float( outfl, nr, "Noise Correction Factor", 
			     6, pcd[nii].noise_factor );
/* 04 */
	  nadc_write_short( outfl, ++nr, "Spectral Calibration Index", 
			     pcd[nii].indx_spec );
/* 05 */
	  nadc_write_short( outfl, ++nr, "Leakage Parameter Index", 
			     pcd[nii].indx_leak );
/* 06 */
	  count[0] = NUM_POLAR_COEFFS;
	  nadc_write_arr_float( outfl, ++nr, "Polarisation wavelength",
				 1, count, 6, pcd[nii].polar.wv );
	  nadc_write_arr_float( outfl, nr, "Polarisation coefficients",
				 1, count, 6, pcd[nii].polar.coeff );
	  nadc_write_arr_float( outfl, nr, "Polarisation errors",
				 1, count, 6, pcd[nii].polar.error );
	  nadc_write_float( outfl, nr, "Polarisation Chi",
				 6, pcd[nii].polar.chi );
/* 07 */
	  nadc_write_text( outfl, ++nr, "Level 0 MPH ProductConfidenceData", 
			    pcd[nii].mph0.ProductConfidenceData );
	  nadc_write_text( outfl, nr, "Level 0 MPH UTC_MPH_Generation", 
			    pcd[nii].mph0.UTC_MPH_Generation );
	  nadc_write_text( outfl, nr, "Level 0 MPH SoftwareVersion", 
			    pcd[nii].mph0.ProcessorSoftwareVersion );
/* 08 */
	  nadc_write_text( outfl, ++nr, "Level 0 SPH sph.sph_5", 
			    pcd[nii].sph0.sph_5 );
	  nadc_write_text( outfl, nr, "Level 0 sph.sph_6", 
			    pcd[nii].sph0.sph_6 );
/* 09 IHR */
	  nadc_write_ushort( outfl, ++nr, "Subset counter",
			      pcd[nii].ihr.subsetcounter );
	  nadc_write_ushort( outfl, nr, "Average mode",
			      pcd[nii].ihr.averagemode );
	  nadc_write_uchar( outfl, nr, "IntegrStatus_FPA4", 
			     pcd[nii].ihr.intg.stat.fpa4 );
	  nadc_write_uchar( outfl, nr, "IntegrStatus_FPA3", 
			     pcd[nii].ihr.intg.stat.fpa3 );
	  nadc_write_uchar( outfl, nr, "IntegrStatus_FPA2", 
			     pcd[nii].ihr.intg.stat.fpa2 );
	  nadc_write_uchar( outfl, nr, "IntegrStatus_Band1a", 
			     pcd[nii].ihr.intg.stat.ch1a );
	  nadc_write_uchar( outfl, nr, "IntegrStatus_Band1b", 
			     pcd[nii].ihr.intg.stat.ch1b );
	  nadc_write_uchar( outfl, nr, "IntegrStatus_Band2a", 
			     pcd[nii].ihr.intg.stat.ch2a );
	  nadc_write_uchar( outfl, nr, "IntegrStatus_Band2b", 
			     pcd[nii].ihr.intg.stat.ch2b );
	  nadc_write_uchar( outfl, nr, "IntegrStatus_Band3", 
			     pcd[nii].ihr.intg.stat.ch3 );
	  nadc_write_uchar( outfl, nr, "IntegrStatus_Band4", 
			     pcd[nii].ihr.intg.stat.ch4 );
	  nadc_write_double( outfl, nr, "PreDisperserTemperature", 3,
			     -1.721 + 0.006104 * 
			     pcd[nii].ihr.prism_temp );
	  count[0] = PMD_IN_GRID;
	  count[1] = PMD_NUMBER;
	  nadc_write_arr_ushort( outfl, nr, "PMD values", 2, count, 
				  (unsigned short *) pcd[nii].ihr.pmd );
	  count[0] = SCIENCE_CHANNELS;
	  nadc_write_arr_short( outfl, nr, "Peltier values", 1, count, 
				  pcd[nii].ihr.peltier );
/* 10 */
	  count[0] = NUM_SPEC_BANDS;
	  nadc_write_arr_short( outfl, ++nr, "Index to spectral bands", 
				 1, count, pcd[nii].indx_bands );

	  if ( param.calib_pmd != CALIB_NONE ) {
	       register short nj, np;
	       register float *pntr;

	       float rbuff[PMD_NUMBER * PMD_IN_GRID];

/* write PMD geolocation data */
	       if ( param.write_pmd_geo == PARAM_SET ) {
		    float latlon[NUM_COORDS * PMD_IN_GRID];

		    count[1] = PMD_IN_GRID;
		    count[0] = NUM_COORDS;
		    pntr = latlon;
		    for ( np = 0; np < PMD_IN_GRID; np++ ) {
			 for ( nj = 0; nj < NUM_COORDS; nj++ )
			      *pntr++ = pcd[nii].pmd[np].glr.lat[nj];
		    }
		    nadc_write_arr_float( outfl, ++nr, "PMD Latitude", 
					  2, count, 6, latlon );

		    pntr = latlon;
		    for ( np = 0; np < PMD_IN_GRID; np++ ) {
			 for ( nj = 0; nj < NUM_COORDS; nj++ )
			      *pntr++ = pcd[nii].pmd[np].glr.lon[nj];
		    }
		    nadc_write_arr_float( outfl, nr, "PMD Longitude",
 					  2, count, 6, latlon );
	       }
/* write PMD data */
	       count[1] = PMD_IN_GRID;
	       count[0] = PMD_NUMBER;
	       pntr = rbuff;
	       for ( np = 0; np < PMD_IN_GRID; np++ ) {
		    for ( nj = 0; nj < PMD_NUMBER; nj++ )
			 *pntr++ = pcd[nii].pmd[np].value[nj];
	       }
	       nadc_write_arr_float( outfl, nr, "PMD (calibrated)", 
				     2, count, 6, rbuff );
	  }
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer   GOME_LV1_WR_ASCII_SMCD
.PURPOSE     dump content of Sun/Moon Pixel Calibration Data records
.INPUT/OUTPUT
  call as   GOME_LV1_WR_ASCII_SMCD( flag_origin, param, nr_smcd, indx_smcd, smcd );
     input:
            unsigned char flag_origin : Sun or Moon measurements
	    struct param_record param : struct holding user-defined settings
	    short nr_smcd             : number of Pixel Calibration Records
	    short *indx_smcd          : indices to selected SMCDs
	    struct smcd_gome *smcd    : Sun/Moon Specific Calibration Records

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void GOME_LV1_WR_ASCII_SMCD( unsigned char flag_origin, 
			     struct param_record param, 
			     short nr_smcd, const short *indx_smcd, 
			     const struct smcd_gome *smcd )
{
     const char prognm[] = "GOME_LV1_WR_ASCII_SMCD";

     register short ni;
     register unsigned int nr = 0;
     char date_str[25];
     unsigned int count[2];

     FILE *outfl;
/*
 * write ASCII dump of SMCD record
 */
     if ( flag_origin == FLAG_SUN ) {
	  if ( (outfl = CRE_ASCII_File( param.outfile, "scd" )) == NULL 
	       || IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
	  nadc_write_header( outfl, nr, param.infile, 
			      "Sun Specific Calibration Record" );
     } else {
	  if ( (outfl = CRE_ASCII_File( param.outfile, "mcd" )) == NULL 
	       || IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
	  nadc_write_header( outfl, nr, param.infile, 
			      "Moon Specific Calibration Record" );
     }

     for ( ni = 0; ni < nr_smcd; ni++ ) {
	  register short nii = indx_smcd[ni];
/* 01 */
	  nr = 1;
	  (void) UTC_2_ASCII( smcd[nii].utc_date, smcd[nii].utc_time, 
			      date_str );
	  nadc_write_text( outfl, nr, "MJD (Date Time)", date_str );
	  nadc_write_double( outfl, nr, "MJD (Julian Day)", 14,
			     (double) smcd[nii].utc_date +
			     (smcd[nii].utc_time / Msec2DecimalDay) );
/* 02 */
	  nadc_write_float( outfl, ++nr, "Sun zenith w.r.t. North", 
			     3, smcd[nii].north_sun_zen );
	  nadc_write_float( outfl, nr, "Sun azimuth w.r.t. North",
			     3, smcd[nii].north_sun_azim );
/* 03 */
	  if ( flag_origin == FLAG_SUN ) {
	       nadc_write_float( outfl, ++nr, "BSDF zenith w.r.t. North", 
				  3, smcd[nii].north_sm_zen );
	       nadc_write_float( outfl, nr, "BSDF azimuth w.r.t. North",
				  3, smcd[nii].north_sm_azim );
	  } else {
	       nadc_write_float( outfl, ++nr, "Moon zenith w.r.t. North", 
				  3, smcd[nii].north_sm_zen );
	       nadc_write_float( outfl, nr, "Moon azimuth w.r.t. North",
				  3, smcd[nii].north_sm_azim );
	  }
/* 04 */
	  if ( flag_origin == FLAG_SUN )
	       nadc_write_short( outfl, ++nr, 
				  "Used for Sun reference spectrum", 
				  (short) smcd[nii].sun_or_moon );
	  else
	       nadc_write_float( outfl, ++nr, 
				  "Illuminated fraction of the Moon", 
				  3, smcd[nii].sun_or_moon );
/* 05 */
	  nadc_write_float( outfl, ++nr, "Dark Current", 
			     6, smcd[nii].dark_current );
	  nadc_write_float( outfl, nr, "Noise Correction Factor",
			     6, smcd[nii].noise_factor );
/* 06 */
	  nadc_write_short( outfl, ++nr, "Spectral Calibration Index", 
			     smcd[nii].indx_spec );
/* 07 */
	  nadc_write_short( outfl, ++nr, "Leakage Parameter Index", 
			     smcd[nii].indx_leak );
/* 08 */
	  nadc_write_text( outfl, ++nr, "Level 0 MPH ProductConfidenceData", 
			    smcd[nii].mph0.ProductConfidenceData );
	  nadc_write_text( outfl, nr, "Level 0 MPH UTC_MPH_Generation", 
			    smcd[nii].mph0.UTC_MPH_Generation );
	  nadc_write_text( outfl, nr, "Level 0 MPH SoftwareVersion", 
			    smcd[nii].mph0.ProcessorSoftwareVersion );
/* 09 */
	  nadc_write_text( outfl, ++nr, "Level 0 SPH sph.sph_5", 
			    smcd[nii].sph0.sph_5 );
	  nadc_write_text( outfl, nr, "Level 0 sph.sph_6", 
			    smcd[nii].sph0.sph_6 );
/* 10 */
	  nadc_write_ushort( outfl, ++nr, "SubSetCounter",
			      smcd[nii].ihr.subsetcounter );
	  nadc_write_ushort( outfl, nr, "AverageMode",
			      smcd[nii].ihr.averagemode );
	  nadc_write_double( outfl, nr, "PreDisperserTemperature", 3,
			     -1.721 + 0.006104 * 
			     smcd[nii].ihr.prism_temp );
	  count[0] = PMD_IN_GRID;
	  count[1] = PMD_NUMBER;
	  nadc_write_arr_ushort( outfl, nr, "PMD values", 2, count, 
				  (unsigned short *) smcd[nii].ihr.pmd );
	  count[0] = SCIENCE_CHANNELS;
	  nadc_write_arr_short( outfl, nr, "Peltier values", 1, count, 
				  smcd[nii].ihr.peltier );
/* 11 */
	  count[0] = NUM_SPEC_BANDS;
	  nadc_write_arr_short( outfl, ++nr, "Index to spectral bands", 
				 1, count, smcd[nii].indx_bands );
     }
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer   GOME_LV1_WR_ASCII_REC
.PURPOSE     
.INPUT/OUTPUT
  call as   GOME_LV1_WR_ASCII_REC( unsigned char flag_origin, short nband, 
			    struct param_record param, 
			    short nr_rec, short bcr_start, short bcr_count,
			    const struct rec_gome *rec );
     input:
            unsigned char flag_origin : Earth, Sun or Moon measurements
	    short  nband              : number of spectral band [1a=0,1b,2a..]
            struct param_record param : struct holding user-defined settings
            short nr_rec              : number of measurement data records
	    short bcr_start           : first pixel of spectral band
	    short bcr_count           : number of pixels per spectral band
	    struct rec_gome *rec      : measurement data records

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void GOME_LV1_WR_ASCII_REC( unsigned char flag_origin, short nband, 
			    struct param_record param, 
			    short nr_rec, short bcr_start, short bcr_count,
			    const struct rec_gome *rec )
{
     const char prognm[] = "GOME_LV1_WR_ASCII_REC";

     register unsigned int ni, nx, ny;
     register unsigned int nr = 0;

     char  *cpntr, string[16], calib_str[10];
     short *sbuff;
     float *rbuff;
     unsigned char  *cbuff;
     unsigned short calib_flag, *usbuff;
     unsigned int   nrpix, count[2];

     FILE *outfl;

     const char *band_names[] = { 
	  "Band-1a", "Band-1b", "Band-2a", "Band-2b", "Band-3", 
	  "Band-4", "Blind", "Stray-1a", "Stray-1b", "Stray-2a"
     };
/*
 * check number of PCD records
 */
     if ( nr_rec == 0 ) return;
/*
 * write ASCII dump of spectral band records
 */
     if ( flag_origin == FLAG_EARTH ) {
	  calib_flag = param.calib_earth;
	  (void) snprintf( string, 16, "earth.%s", band_names[nband] );
	  if ( (outfl = CRE_ASCII_File( param.outfile, string )) == NULL
	       || IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
	  nadc_write_header( outfl, nr, param.infile, 
			      "Earth Spectral Band Records" );
     } else if ( flag_origin == FLAG_SUN ) {
	  calib_flag = param.calib_moon;
	  (void) snprintf( string, 16, "sun.%s", band_names[nband] );
	  if ( (outfl = CRE_ASCII_File( param.outfile, string )) == NULL
	       || IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
	  nadc_write_header( outfl, nr, param.infile, 
			      "Sun Spectral Band Records" );
     } else {
	  calib_flag = param.calib_sun;
	  (void) snprintf( string, 16, "moon.%s", band_names[nband] );
	  if ( (outfl = CRE_ASCII_File( param.outfile, string )) == NULL
	       || IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
	  nadc_write_header( outfl, nr, param.infile, 
			      "Moon Spectral Band Records" );
     }
     nadc_write_short( outfl, nr, "IntegrationIntervals", nr_rec );
     nadc_write_short( outfl, nr, "LengthSpectralBand", bcr_count );
/*
 * Calibration
 */
     GOME_GET_CALIB( calib_flag, calib_str );
     if ( flag_origin == FLAG_EARTH 
	  && (cpntr = strchr( calib_str, 'B' )) != NULL )
	  *cpntr = 'A';
     if ( strlen(calib_str) == 0 )
	  (void) strcpy( calib_str, "none" );
     nadc_write_text( outfl, nr, "Calibration", calib_str );
/*
 * Quality flags
 */     
     count[0] = (unsigned int) nr_rec;
     cbuff = (unsigned char *) malloc( count[0] * sizeof( unsigned char ));
     if ( cbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "cbuff" );
     for ( nx = 0; nx < count[0]; nx++ )
	  cbuff[nx] = (unsigned char) rec[nx].pixel_flags.flag_fields.dead;
     nadc_write_arr_uchar( outfl, ++nr, "FlagDeadPixels", 1, count, cbuff );
     for ( nx = 0; nx < count[0]; nx++ )
	  cbuff[nx] = (unsigned char) rec[nx].pixel_flags.flag_fields.hot;
     nadc_write_arr_uchar( outfl, nr, "FlagHotPixels", 1, count, cbuff );
     for ( nx = 0; nx < count[0]; nx++ )
	  cbuff[nx] = (unsigned char) rec[nx].pixel_flags.flag_fields.saturate;
     nadc_write_arr_uchar( outfl, nr, "FlagSaturatePixels", 1, count, cbuff );
     for ( nx = 0; nx < count[0]; nx++ )
	  cbuff[nx] = (unsigned char) rec[nx].pixel_flags.flag_fields.spectral;
     nadc_write_arr_uchar( outfl, nr, "FlagSpectralPixels", 1, count, cbuff );
     free( cbuff );
/*
 * Indices to Polarisation Sensitivity Parameters
 */
     sbuff = (short *) malloc( count[0] * sizeof( short ));
     if ( sbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "sbuff" );
     for ( nx = 0; nx < count[0]; nx++ )
	  sbuff[nx] = rec[nx].indx_psp;
     nadc_write_arr_short( outfl, ++nr, "IndexPolarisation", 1, count, 
			    sbuff );
/*
 * Indices to Pixel Specific Calibration Parameters
 */
     for ( nx = 0; nx < count[0]; nx++ )
	  sbuff[nx] = rec[nx].indx_pcd;
     nadc_write_arr_short( outfl, ++nr, "IndexPCD (Pixel Number)", 1, count, 
			    sbuff );
     free( sbuff );
/*
 * Integration Times
 */
     rbuff = (float *) malloc( count[0] * sizeof( float ));
     if ( rbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
     if ( nband == BAND_1b || nband == BAND_2b ) { 
	  for ( nx = 0; nx < count[0]; nx++ )
	       rbuff[nx] = rec[nx].integration[1];
     } else {
	  for ( nx = 0; nx < count[0]; nx++ )
	       rbuff[nx] = rec[nx].integration[0];
     }
     nadc_write_arr_float( outfl, ++nr, "IntegrationTime", 
			    1, count, 3, rbuff );
     free( rbuff );
/*
 * wavelength
 */
     count[0] = (unsigned int) bcr_count;
     nadc_write_arr_float( outfl, ++nr, "WaveLength", 
			    1, count, 4, &rec[0].wave[bcr_start] );
/*
 * Spectral data
 */
     count[0] = (unsigned int) bcr_count;
     count[1] = (unsigned int) nr_rec;
     nrpix = count[0] * count[1];
     
     if ( (flag_origin == FLAG_EARTH && param.calib_earth == CALIB_NONE)
	  || (flag_origin == FLAG_MOON && param.calib_moon == CALIB_NONE)
	  || (flag_origin == FLAG_SUN && param.calib_sun == CALIB_NONE)) {
	  usbuff = (unsigned short *) 
	       malloc( nrpix * sizeof( unsigned short ));
	  if ( usbuff == NULL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "usbuff" );
	  for ( ni = ny = 0; ny < count[1]; ny++ )
	       for ( nx = 0; nx < count[0]; nx++ )
		    usbuff[ni++] = (unsigned short) rec[ny].data[nx+bcr_start];
	  nadc_write_arr_ushort( outfl, nr, band_names[nband], 2, count, 
				  usbuff );
	  free( usbuff );
     } else {
	  rbuff = (float *) malloc( nrpix * sizeof( float ));
	  if ( rbuff == NULL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
	  for ( ni = ny = 0; ny < count[1]; ny++ )
	       for ( nx = 0; nx < count[0]; nx++ )
		    rbuff[ni++] = rec[ny].data[nx+bcr_start];
	  nadc_write_arr_float( outfl, nr, band_names[nband], 2, count, 
				 6, rbuff );
	  free( rbuff );
     }
     (void) fclose( outfl );
}

