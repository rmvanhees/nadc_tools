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

.IDENTifer   MERIS_RD_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    MERIS level 1 & 2 data
.LANGUAGE    ANSI C
.PURPOSE     read Specific Product Header of Level 1b or 2 Product
.INPUT/OUTPUT
  call as   MERIS_RD_SPH( fd, mph, &sph );
     input:  
            FILE   *fd             : (open) stream pointer
	    struct mph_envi mph    : main product header
    output:  
            struct sph_meris *sph : structure for the SPH

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
.ENVIRONment None
.EXTERNALs   ENVI_RD_PDS_INFO
.VERSION      1.0   22-Sep-2008 created by R. M. van Hees
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
#define _MERIS_COMMON
#include <nadc_meris.h>

#define NUM_SPH_ITEMS 41

static NADC_pds_hdr_t sph_items[NUM_SPH_ITEMS] = {
     {"SPH_DESCRIPTOR", PDS_String, 29, "", NULL},
     {"STRIPLINE_CONTINUITY_INDICATOR", PDS_Short, 4, "", NULL},
     {"SLICE_POSITION", PDS_Short, 4, "", NULL},
     {"NUM_SLICES", PDS_uShort, 4, "", NULL},
     {"FIRST_LINE_TIME", PDS_String, UTC_STRING_LENGTH, "", NULL},
     {"LAST_LINE_TIME", PDS_String, UTC_STRING_LENGTH, "", NULL},
     {"FIRST_FIRST_LAT", PDS_Long, 11, "", "<10-6degN>"},
     {"FIRST_FIRST_LONG", PDS_Long, 11, "", "<10-6degE>"},
     {"FIRST_MID_LAT", PDS_Long, 11, "", "<10-6degN>"},
     {"FIRST_MID_LONG", PDS_Long, 11, "", "<10-6degE>"},
     {"FIRST_LAST_LAT", PDS_Long, 11, "", "<10-6degN>"},
     {"FIRST_LAST_LONG", PDS_Long, 11, "", "<10-6degE>"},
     {"LAST_FIRST_LAT", PDS_Long, 11, "", "<10-6degN>"},
     {"LAST_FIRST_LONG", PDS_Long, 11, "", "<10-6degE>"},
     {"LAST_MID_LAT", PDS_Long, 11, "", "<10-6degN>"},
     {"LAST_MID_LONG", PDS_Long, 11, "", "<10-6degE>"},
     {"LAST_LAST_LAT", PDS_Long, 11, "", "<10-6degN>"},
     {"LAST_LAST_LONG", PDS_Long, 11, "", "<10-6degE>"},
     {"", PDS_Spare, 47, "", NULL},
     {"TRANS_ERR_FLAG", PDS_uShort, 1, "", NULL},
     {"FORMAT_ERR_FLAG", PDS_uShort, 1, "", NULL},
     {"DATABASE_FLAG", PDS_uShort, 1, "", NULL},
     {"COARSE_ERR_FLAG", PDS_uShort, 1, "", NULL},
     {"ECMWF_TYPE", PDS_uShort, 1, "", NULL},
     {"NUM_TRANS_ERR", PDS_uLong, 11, "", NULL},
     {"NUM_FORMAT_ERR", PDS_uLong, 11, "", NULL},
     {"TRANS_ERR_THRESH", PDS_Ado18e, 15, "", "<%>"},
     {"FORMAT_ERR_THRESH", PDS_Ado18e, 15, "", "<%>"},
     {"", PDS_Spare, 77, "", NULL},
     {"NUM_BANDS", PDS_uShort, 4, "", NULL},
     {"BAND_WAVELEN", PDS_String, 165, "", "<10-3nm>"},
     {"BANDWIDTH", PDS_String, 90, "", "<10-3nm>"},
     {"INST_FOV", PDS_uLong, 11, "", "<10-6deg>"},
     {"PROC_MODE", PDS_uShort, 1, "", NULL},
     {"OFFSET_COMP", PDS_uShort, 1, "", NULL},
     {"LINE_TIME_INTERVAL", PDS_uLong, 11, "", "<10-6s>"},
     {"LINE_LENGTH", PDS_uShort, 6, "", "<samples>"},
     {"LINES_PER_TIE_PT", PDS_uShort, 4, "", NULL},
     {"SAMPLES_PER_TIE_PT", PDS_uShort, 4, "", NULL},
     {"COLUMN_SPACING", PDS_Ado18e, 15, "", "<m>"},
     {"", PDS_Spare, 41, "", NULL}
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
#define __PDS_RD_ONLY
#include <_envi_pds_hdr.inc>

/*+++++++++++++++++++++++++
.IDENTifer   MERIS_RD_SPH
.PURPOSE     read Specific Product Header of Level 1b or 2 Product
.INPUT/OUTPUT
  call as   MERIS_RD_SPH( fd, mph, &sph );
     input:  
            FILE   *fd             : (open) stream pointer
	    struct mph_envi mph    : main product header
    output:  
            struct sph_meris *sph : structure for the SPH

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void MERIS_RD_SPH( FILE *fd, const struct mph_envi mph,
		      struct sph_meris *sph )
       /*@globals sph_items;@*/
{
     const char prognm[] = "MERIS_RD_SPH";

     int    ibuff;

     unsigned int nr_byte;

     const unsigned int Length_SPH = mph.sph_size - mph.num_dsd * mph.dsd_size;
/*
 * always rewind the file, skipping the MPH
 */
     (void) fseek( fd, (long) PDS_MPH_LENGTH, SEEK_SET );
/*
 * read PDS header
 */
     nr_byte = NADC_RD_PDS_HDR( fd, NUM_SPH_ITEMS, sph_items );
     if ( nr_byte != Length_SPH ) {
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_SIZE, "SPH size" );
     }
/*
 * fill sph_meris struct
 */
     (void) nadc_strlcpy( sph->descriptor, sph_items[0].value+1, 
			  (size_t) sph_items[0].length );
     (void) sscanf( sph_items[1].value, "%hd", &sph->stripline );
     (void) sscanf( sph_items[2].value, "%hd", &sph->slice_pos );
     (void) sscanf( sph_items[3].value, "%hu", &sph->num_slices );
     (void) nadc_strlcpy( sph->start_time, sph_items[4].value+1, 
			  (size_t) sph_items[4].length );
     (void) nadc_strlcpy( sph->stop_time, sph_items[5].value+1, 
			  (size_t) sph_items[5].length );
     (void) sscanf( sph_items[6].value, "%d", &ibuff );
     sph->first_first_lat = ibuff / 1e6;
     (void) sscanf( sph_items[7].value, "%d", &ibuff );
     sph->first_first_lon = ibuff / 1e6;
     (void) sscanf( sph_items[8].value, "%d", &ibuff );
     sph->first_mid_lat = ibuff / 1e6;
     (void) sscanf( sph_items[9].value, "%d", &ibuff );
     sph->first_mid_lon = ibuff / 1e6;
     (void) sscanf( sph_items[10].value, "%d", &ibuff );
     sph->first_last_lat = ibuff / 1e6;
     (void) sscanf( sph_items[11].value, "%d", &ibuff );
     sph->first_last_lon = ibuff / 1e6;
     (void) sscanf( sph_items[12].value, "%d", &ibuff );
     sph->last_first_lat = ibuff / 1e6;
     (void) sscanf( sph_items[13].value, "%d", &ibuff );
     sph->last_first_lon = ibuff / 1e6;
     (void) sscanf( sph_items[14].value, "%d", &ibuff );
     sph->last_mid_lat = ibuff / 1e6;
     (void) sscanf( sph_items[15].value, "%d", &ibuff );
     sph->last_mid_lon = ibuff / 1e6;
     (void) sscanf( sph_items[16].value, "%d", &ibuff );
     sph->last_last_lat = ibuff / 1e6;
     (void) sscanf( sph_items[17].value, "%d", &ibuff );
     sph->last_last_lon = ibuff / 1e6;

     (void) sscanf( sph_items[19].value, "%d", &ibuff );
     sph->trans_err = (bool) ibuff;
     (void) sscanf( sph_items[20].value, "%d", &ibuff );
     sph->format_err = (bool) ibuff;
     (void) sscanf( sph_items[21].value, "%d", &ibuff );
     sph->database = (bool) ibuff;
     (void) sscanf( sph_items[22].value, "%d", &ibuff );
     sph->coarse_err = (bool) ibuff;
     (void) sscanf( sph_items[23].value, "%d", &ibuff );
     sph->ecmwf_type = (bool) ibuff;
     (void) sscanf( sph_items[24].value, "%hu", &sph->num_trans_err );
     (void) sscanf( sph_items[25].value, "%hu", &sph->num_format_err );
     (void) sscanf( sph_items[26].value, "%f", &sph->thres_trans_err );
     (void) sscanf( sph_items[27].value, "%f", &sph->thres_format_err );

     (void) sscanf( sph_items[29].value, "%hu", &sph->num_bands );
     (void) nadc_strlcpy( sph->band_wavelen, sph_items[30].value+1, 
			  (size_t) sph_items[30].length );
     (void) nadc_strlcpy( sph->bandwidth, sph_items[31].value+1, 
			  (size_t) sph_items[31].length );
     (void) sscanf( sph_items[32].value, "%d", &ibuff );
     sph->inst_fov = ibuff / 1e6;
     (void) sscanf( sph_items[33].value, "%d", &ibuff );
     sph->proc_mode = (bool) ibuff;
     (void) sscanf( sph_items[34].value, "%d", &ibuff );
     sph->offset_comp = (bool) ibuff;
     (void) sscanf( sph_items[35].value, "%d", &ibuff );
     sph->line_time_interval = ibuff / 1e6;
     (void) sscanf( sph_items[36].value, "%hu", &sph->line_length );
     (void) sscanf( sph_items[37].value, "%hu", &sph->lines_per_tie );
     (void) sscanf( sph_items[38].value, "%hu", &sph->samples_per_tie );
     (void) sscanf( sph_items[39].value, "%f", &sph->column_spacing );
}
