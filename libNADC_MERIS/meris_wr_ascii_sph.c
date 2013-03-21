/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2008 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   MERIS_WR_ASCII_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    PDS MERIS headers
.LANGUAGE    ANSI C
.PURPOSE     Dump Specific Product Header of the level 1b or 2 product
.INPUT/OUTPUT
  call as    MERIS_WR_ASCII_SPH( param, sph );
     input:  
             struct param_record param : struct holding user-defined settings
	     struct sph_meris *sph     : pointer to SPH structure

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     22-Sep-2008   Created by R. M. van Hees
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
#define _MERIS_COMMON
#include <nadc_meris.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void MERIS_WR_ASCII_SPH( struct param_record param, 
			 const struct sph_meris *sph )
{
     register unsigned int  nr = 0;

     const char prognm[] = "MERIS_WR_ASCII_SPH";

     FILE *outfl = CRE_ASCII_File( param.outfile, "sph" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of SPH record
 */
     nadc_write_header( outfl, nr, param.infile, 
			 "Specific Product Header of Level 1b Product" );
     nadc_write_text( outfl, ++nr, "SPH_DESCRIPTOR", sph->descriptor );
     nadc_write_short( outfl, ++nr, "STRIPLINE_CONTINUITY_INDICATOR", 
		       sph->stripline );
     nadc_write_short( outfl, ++nr, "SLICE_POSITION", sph->slice_pos );
     nadc_write_ushort( outfl, ++nr, "NUM_SLICES", sph->num_slices );
     nadc_write_text( outfl, ++nr, "FIRST_LINE_TIME", sph->start_time );
     nadc_write_text( outfl, ++nr, "LAST_LINE_TIME", sph->stop_time );
     nadc_write_double( outfl, ++nr, "FIRST_FIRST_LAT", 
			6, sph->first_first_lat );
     nadc_write_double( outfl, ++nr, "FIRST_FIRST_LONG", 
			6, sph->first_first_lon );
     nadc_write_double( outfl, ++nr, "FIRST_MID_LAT", 6, sph->first_mid_lat );
     nadc_write_double( outfl, ++nr, "FIRST_MID_LONG", 6, sph->first_mid_lon );
     nadc_write_double( outfl, ++nr, "FIRST_LAST_LAT", 
			6, sph->first_last_lat );
     nadc_write_double( outfl, ++nr, "FIRST_LAST_LONG", 
			6, sph->first_last_lon );
     nadc_write_double( outfl, ++nr, "LAST_FIRST_LAT", 
			6, sph->last_first_lat );
     nadc_write_double( outfl, ++nr, "LAST_FIRST_LONG", 
			6, sph->last_first_lon );
     nadc_write_double( outfl, ++nr, "LAST_MID_LAT", 6, sph->last_mid_lat );
     nadc_write_double( outfl, ++nr, "LAST_MID_LONG", 6, sph->last_mid_lon );
     nadc_write_double( outfl, ++nr, "LAST_LAST_LAT", 6, sph->last_last_lat );
     nadc_write_double( outfl, ++nr, "LAST_LAST_LONG", 6, sph->last_last_lon );
     nr++;    /* spare_1 */
     nadc_write_bool( outfl, ++nr, "TRANS_ERR_FLAG", sph->trans_err );
     nadc_write_bool( outfl, ++nr, "FORMAT_ERR_FLAG", sph->format_err );
     nadc_write_bool( outfl, ++nr, "DATABASE_FLAG", sph->database );
     nadc_write_bool( outfl, ++nr, "COARSE_ERR_FLAG", sph->coarse_err );
     nadc_write_bool( outfl, ++nr, "ECMWF_TYPE", sph->ecmwf_type );
     nadc_write_ushort( outfl, ++nr, "NUM_TRANS_ERR", sph->num_trans_err );
     nadc_write_ushort( outfl, ++nr, "NUM_FORMAT_ERR", sph->num_format_err );
     nadc_write_float( outfl, ++nr, "TRANS_ERR_THRESH", 
		       6, sph->thres_trans_err );
     nadc_write_float( outfl, ++nr, "FORMAT_ERR_THRESH", 
		       6, sph->thres_format_err );
     nr++;    /* spare_2 */
     nadc_write_ushort( outfl, ++nr, "NUM_BANDS", sph->num_bands );
     nadc_write_text( outfl, ++nr, "BAND_WAVELEN", sph->band_wavelen );
     nadc_write_text( outfl, ++nr, "BANDWIDTH", sph->bandwidth );
     nadc_write_double( outfl, ++nr, "INST_FOV", 6, sph->inst_fov );
     nadc_write_bool( outfl, ++nr, "PROC_MODE", sph->proc_mode );
     nadc_write_bool( outfl, ++nr, "OFFSET_COMP", sph->offset_comp );
     nadc_write_double( outfl, ++nr, "LINE_TIME_INTERVAL", 
			6, sph->line_time_interval );
     nadc_write_ushort( outfl, ++nr, "LINE_LENGTH", sph->line_length );
     nadc_write_ushort( outfl, ++nr, "LINES_PER_TIE_PT", sph->lines_per_tie );
     nadc_write_ushort( outfl, ++nr, "SAMPLES_PER_TIE_PT", 
			sph->samples_per_tie );
     nadc_write_float( outfl, ++nr, "COLUMN_SPACING", 6, sph->column_spacing );
     nr++;    /* spare_3 */
     (void) fclose( outfl );
}

