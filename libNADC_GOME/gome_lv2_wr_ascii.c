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

.IDENTifer   GOME_LV2_WR_ASCII
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 2
.LANGUAGE    ANSI C
.PURPOSE     Dump the contents of a GOME level 2 file in ASCII
.RETURNS     Nothing
.COMMENTS    contains GOME_LV2_WR_ASCII_FSR, GOME_LV2_WR_ASCII_SPH,
                GOME_LV2_WR_ASCII_DDR
.ENVIRONment None
.VERSION      4.0   17-Nov-2005	add start and stop time derived from the MDS
                                records to SPH output and increased the number 
				of digits of several parameters, RvH
              3.0   11-Aug-2005	added ASCII dump of IRR data, RvH
              2.3   11-Aug-2005	bugfix in writing FSR2, RvH
              2.2   10-Aug-2005	write MJD as DateTime & Julian days, RvH
              2.1   21-Aug-2001 pass structures using pointers, RvH
              2.0   18-Aug-2000 major rewrite and standardization, RvH
              1.2   08-Mar-2000 renamed: DEBUG -> GOME_LV2_WR_ASCII, RvH
              1.0   18-Mar-1999 created by R. M. van Hees
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

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   GOME_LV2_WR_ASCII_FSR
.PURPOSE     dump content of File Structure Record
.INPUT/OUTPUT
  call as   GOME_LV2_WR_ASCII_FSR( param, fsr );
     input:
            struct param_record param : struct holding user-defined settings
	    struct fsr2_gome *fsr     : structure for FSR record

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void GOME_LV2_WR_ASCII_FSR( struct param_record param, 
			    const struct fsr2_gome *fsr )
{
     register unsigned int nr = 0;

     FILE *outfl = CRE_ASCII_File( param.outfile, "fsr" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
          NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of FSR record
 */
     nadc_write_header( outfl, nr, param.infile, "File Structure Record" );
     nadc_write_short( outfl, ++nr, "Number of SPH2 records", fsr->nr_sph );
     nadc_write_int( outfl, ++nr, "Length of SPH2 record", fsr->sz_sph );
     nadc_write_short(outfl, ++nr, "Number of DOAS data records", fsr->nr_ddr);
     nadc_write_int( outfl, ++nr, "Length of DOAS data record", fsr->sz_ddr );
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer   GOME_LV2_WR_ASCII_SPH
.PURPOSE     dump content of Specific Product Header
.INPUT/OUTPUT
  call as   GOME_LV2_WR_ASCII_SPH( param, sph );
     input:
            struct param_record param : struct holding user-defined settings
	    struct sph2_gome *sph     : structure for Specific Product Header

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void GOME_LV2_WR_ASCII_SPH( struct param_record param, 
			    const struct sph2_gome *sph )
{
     register int ni;
     register unsigned nr = 0;

     char date_str[DATE_STRING_LENGTH];

     unsigned int count[2];

     FILE *outfl = CRE_ASCII_File( param.outfile, "sph" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
          NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
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
     nadc_write_text( outfl, ++nr, "Input data reference", sph->inref );
     nadc_write_text( outfl, ++nr, "GDP Software version", 
		       sph->soft_version );
     nadc_write_text( outfl, ++nr, "GDP static param. file version", 
		       sph->param_version );
     nadc_write_text( outfl, ++nr, "GDP L2 format version",
		       sph->format_version );
     nadc_write_short( outfl, ++nr, "Number of fitting windows", sph->nwin );
     count[0] = 2;
     count[1] = (unsigned int) sph->nwin;
     nadc_write_arr_float( outfl, ++nr, "Window pair (start/end wavelength)",
			    2, count, 3, sph->window );
     nadc_write_short( outfl, ++nr, "Number of molecules", sph->nmol );
     for ( ni = 0; ni < sph->nmol; ni++ ) {
	  char cbuff[35];

	  (void) snprintf( cbuff, 35, "Molecule %s in fitting window",  
			  sph->mol_name[ni] );
	  nadc_write_short( outfl, nr, cbuff, sph->mol_win[ni] );
     }
     nadc_write_float( outfl, ++nr, "Atmosphere height", 4, sph->height );
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer   GOME_LV2_WR_ASCII_DDR
.PURPOSE     dump content of GOME Lv2 data record
.INPUT/OUTPUT
  call as   GOME_LV2_WR_ASCII_DDR( param, sph, nr_ddr, ddr );
     input:
            struct param_record param : struct holding user-defined settings
	    struct sph2_gome sph      : structure with SPH record
	    short nr_ddr              : number of DDR records
	    struct ddr_gome *ddr      : pointer to structures with DDR records

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void GOME_LV2_WR_ASCII_DDR( struct param_record param, struct sph2_gome sph,
			    short nr_ddr, const struct ddr_gome *ddr )
{
     register short nd;
     register unsigned int nr = 0;

     char date_str[DATE_STRING_LENGTH];

     unsigned int count[2];
     
     const double Msec2DecimalDay = 1000 * 24. * 60. * 60.;

     FILE *outfl = CRE_ASCII_File( param.outfile, "ddr" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
          NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of DDR record
 */
     nadc_write_header( outfl, nr, param.infile, "DOAS Data Record" );
     for ( nd = 0; nd < nr_ddr; nd++ ) {
/*
 * write geolocation record
 */
	  nadc_write_int( outfl, ++nr, "Ground pixel number", 
			  ddr[nd].glr.pixel_nr );
	  nadc_write_uchar( outfl, nr, "Subset counter", 
			    ddr[nd].glr.subsetcounter );
	  (void) UTC_2_ASCII( ddr[nd].glr.utc_date, ddr[nd].glr.utc_time, 
			      date_str );
	  nadc_write_text( outfl, nr, "MJD (Date Time)", date_str );
	  nadc_write_double( outfl, nr, "MJD (Julian Day)", 14,
			     (double) ddr[nd].glr.utc_date +
			     (ddr[nd].glr.utc_time / Msec2DecimalDay) );
	  count[0] = 3;
	  nadc_write_arr_float( outfl, nr, 
				"Solar zenith angles w.r.t. North", 
				1, count, 5, ddr[nd].glr.sat_zenith );
	  nadc_write_arr_float( outfl, nr, 
				"Line-of-Sight zenith w.r.t. satellite", 
				1, count, 5, ddr[nd].glr.sat_sight );
	  nadc_write_arr_float( outfl, nr, 
				"Relative azimuth angles at satellite",
				1, count, 5, ddr[nd].glr.sat_azim );
	  nadc_write_arr_float( outfl, nr, "Solar zenith angles at TOA",
				1, count, 5, ddr[nd].glr.toa_zenith );
	  nadc_write_arr_float( outfl, nr, "Line-of-sight angles at TOA",
				1, count, 5, ddr[nd].glr.toa_sight );
	  nadc_write_arr_float( outfl, nr, "Relative azimuth angles at TOA",
				1, count, 5, ddr[nd].glr.toa_azim);
	  nadc_write_float( outfl, nr, "Satellite geodetic height",
			    5, ddr[nd].glr.sat_geo_height );
	  nadc_write_float( outfl, nr, "Earth radius", 
			    5, ddr[nd].glr.earth_radius );
	  count[0] = NUM_COORDS;
	  nadc_write_arr_float( outfl, nr, "Latitude of ground pixel", 
				1, count, 6, ddr[nd].glr.lat );
	  nadc_write_arr_float( outfl, nr, "Longitude of ground pixel", 
				1, count, 6, ddr[nd].glr.lon );
/*
 * write measurement record
 */
	  nadc_write_float( outfl, nr, "Total column of ozone", 
			     5, ddr[nd].ozone );
	  nadc_write_float( outfl, nr, "Relative error on total column", 
			     5, ddr[nd].error );
/*
 * write intermediate results records (IRR)
 */
	  if ( ddr[nd].irr1 != NULL ) {
	       struct irr1_gome *irr_pntr = ddr[nd].irr1;

	       count[0] = sph.nmol;
	       nadc_write_arr_float( outfl, nr, "Total VCD to ground", 
				     1, count, 5, irr_pntr->total_vcd );
	       nadc_write_arr_float( outfl, nr, "Errors on VCDs", 
				     1, count, 5, irr_pntr->error_vcd );
	       nadc_write_short( outfl, nr, "Flag indexing VCD", 
				 irr_pntr->indx_vcd );
	       count[0] = sph.nmol;
	       nadc_write_arr_float( outfl, nr, "Slant columns (DOAS)", 
				     1, count, 5, irr_pntr->slant_doas );
	       nadc_write_arr_float( outfl, nr, "Slant column errors", 
				     1, count, 5, irr_pntr->error_doas );
	       count[0] = sph.nwin;
	       nadc_write_arr_float( outfl, nr, "RMS (DOAS)", 
				     1, count, 5, irr_pntr->rms_doas );
	       nadc_write_arr_float( outfl, nr, "ChiSquare (DOAS)", 
				     1, count, 5, irr_pntr->chi_doas );
	       nadc_write_arr_float( outfl, nr, "Goodness of fit (DOAS)", 
				     1, count, 5, irr_pntr->fit_doas );
	       nadc_write_arr_float( outfl, nr, "Iteration number (DOAS)", 
				     1, count, 5, irr_pntr->iter_doas );
	       nadc_write_short( outfl, nr, "Flag indexing DOAS", 
				 irr_pntr->indx_doas );
	       count[0] = sph.nmol;
	       nadc_write_arr_float( outfl, nr, "AMF to ground", 
				     1, count, 5, irr_pntr->ground_amf );
	       nadc_write_arr_float( outfl, nr, "AMF to cloud-top", 
				     1, count, 5, irr_pntr->cloud_amf );
	       count[0] = sph.nwin;
	       nadc_write_arr_float( outfl, nr, "Total intensities for ground",
				     1, count, 5, irr_pntr->intensity_ground );
	       nadc_write_arr_float( outfl, nr, 
				     "Total intensities for cloud-top",
				     1, count, 5, irr_pntr->intensity_cloud );
	       nadc_write_arr_float( outfl, nr, "Measured intensity (AMF)", 1,
				     count, 5, irr_pntr->intensity_measured );
	       nadc_write_short( outfl, nr, "Flag indexing AMF", 
				 irr_pntr->indx_amf );
	       nadc_write_float( outfl, nr, "Cloud fraction (ICFA)", 
				 5, irr_pntr->cloud_frac );
	       nadc_write_float( outfl, nr, "Cloud-top pressure (ICFA)", 
				 5, irr_pntr->cloud_pres );
	       nadc_write_float( outfl, nr, "Cloud fraction errors (ICFA)", 
				 5, irr_pntr->err_cloud_frac );
	       nadc_write_float( outfl, nr, "Cloud-top pressure errors (ICFA)", 
				 5, irr_pntr->err_cloud_pres );
	       nadc_write_float( outfl, nr, "Surface pressure (ICFA)", 
				 5, irr_pntr->surface_pres );
	       nadc_write_float( outfl, nr, "CCA Cloud fraction (ICFA)", 
				 5, irr_pntr->cca_cloud_frac );
	       count[0] = 16;
	       nadc_write_arr_schar( outfl, nr, "CCA sub-pixel trace",
				     1, count, irr_pntr->cca_subpixel );
	       nadc_write_short( outfl, nr, "Flag indexing ICFA", 
				 irr_pntr->indx_icfa );
	       count[0] = 3;
	       nadc_write_arr_float( outfl, nr, "Pixel contrast (mean)",
				     1, count, 5, irr_pntr->pmd_avg );
	       nadc_write_arr_float( outfl, nr, "Pixel contrast (sdev)",
				     1, count, 5, irr_pntr->pmd_sdev );
	       count[0] = 16;
	       nadc_write_arr_float( outfl, nr, "Pixel color (sub-pixel)",
				     1, count, 5, irr_pntr->pixel_color );
	       nadc_write_float( outfl, nr, "Pixel color gradient", 
				 5, irr_pntr->pixel_gradient );
	       nadc_write_short( outfl, nr, "Flag indexing Statistics", 
				 irr_pntr->indx_stats );
	  } else {
	       struct irr2_gome *irr_pntr = ddr[nd].irr2;

	       count[0] = sph.nmol;
	       nadc_write_arr_float( outfl, nr, "Total VCD to ground", 
				     1, count, 5, irr_pntr->total_vcd );
	       nadc_write_arr_float( outfl, nr, "Errors on VCDs", 
				     1, count, 5, irr_pntr->error_vcd );
	       nadc_write_short( outfl, nr, "Flag indexing VCD", 
				 irr_pntr->indx_vcd );
	       count[0] = sph.nmol;
	       nadc_write_arr_float( outfl, nr, "Slant columns (DOAS)", 
				     1, count, 5, irr_pntr->slant_doas );
	       nadc_write_arr_float( outfl, nr, "Slant column errors", 
				     1, count, 5, irr_pntr->error_doas );
	       count[0] = sph.nwin;
	       nadc_write_arr_float( outfl, nr, "RMS (DOAS)", 
				     1, count, 5, irr_pntr->rms_doas );
	       nadc_write_arr_float( outfl, nr, "ChiSquare (DOAS)", 
				     1, count, 5, irr_pntr->chi_doas );
	       nadc_write_arr_float( outfl, nr, "Goodness of fit (DOAS)", 
				     1, count, 5, irr_pntr->fit_doas );
	       nadc_write_arr_float( outfl, nr, "Iteration number (DOAS)", 
				     1, count, 5, irr_pntr->iter_doas );
	       nadc_write_float( outfl, nr, "Ozone temperature (DOAS)", 
				 5, irr_pntr->ozone_temperature );
	       nadc_write_float( outfl, nr, "Ozone ring correction (DOAS)", 
				 5, irr_pntr->ozone_ring_corr );
	       nadc_write_short( outfl, nr, "Flag indexing DOAS", 
				 irr_pntr->indx_doas );
	       count[0] = sph.nmol;
	       nadc_write_arr_float( outfl, nr, "AMF to ground", 
				     1, count, 5, irr_pntr->ground_amf );
	       nadc_write_arr_float( outfl, nr, "AMF to ground (error)", 
				     1, count, 5, irr_pntr->ground_amf );
	       nadc_write_arr_float( outfl, nr, "AMF to cloud-top", 
				     1, count, 5, irr_pntr->cloud_amf );
	       nadc_write_arr_float( outfl, nr, "AMF to cloud-top (error)", 
				     1, count, 5, irr_pntr->cloud_amf );
	       nadc_write_short( outfl, nr, "Flag indexing AMF", 
				 irr_pntr->indx_amf );
	       
	       nadc_write_float( outfl, nr, "Ghost vertical column", 
				 5, irr_pntr->ghost_column );
	       nadc_write_float( outfl, nr, "Cloud fraction (OCRA)", 
				 5, irr_pntr->cld_frac );
	       nadc_write_float( outfl, nr, "Cloud fraction error (OCRA)", 
				 5, irr_pntr->error_cld_frac );
	       nadc_write_float( outfl, nr, "Cloud-top height (ROCINN)", 
				 5, irr_pntr->cld_height );
	       nadc_write_float( outfl, nr, "Cloud-top height (ROCINN)", 
				 5, irr_pntr->error_cld_height );
	       nadc_write_float( outfl, nr, "Cloud-top pressure (ROCINN)", 
				 5, irr_pntr->cld_press );
	       nadc_write_float( outfl, nr, 
				 "Cloud-top pressure error (ROCINN)",
				 5, irr_pntr->error_cld_press );
	       nadc_write_float( outfl, nr, "Cloud-top albedo (ROCINN)", 
				 5, irr_pntr->cld_albedo );
	       nadc_write_float( outfl, nr, "Cloud-top albedo error (ROCINN)", 
				 5, irr_pntr->error_cld_albedo );
	       nadc_write_float( outfl, nr, "Surface height", 
				 5, irr_pntr->surface_height );
	       nadc_write_float( outfl, nr, "Surface pressure", 
				 5, irr_pntr->surface_press );
	       nadc_write_float( outfl, nr, "Surface albedo", 
				 5, irr_pntr->surface_albedo );
	  }
     }
     (void) fclose( outfl );
}
