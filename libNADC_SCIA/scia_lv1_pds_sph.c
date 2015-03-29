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

.IDENTifer   SCIA_LV1_PDS_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Specific Product Header of Level 1b Product
.COMMENTS    contains SCIA_LV1_RD_SPH and SCIA_LV1_WR_SPH
.ENVIRONment None
.EXTERNALs   ENVI_RD_PDS_INFO
.VERSION      7.0   19-Apr-2005	complete rewrite & added write routine, RvH
              6.3   12-Apr-2005	obtain SPH size from MPH, RvH
              6.2   03-Feb-2005	handle keyword "INIT_VERSION" gracefully, RvH
              6.1   08-Jan-2005	for now we skip the  non-documented 
                                keyword "INIT_VERSION", RvH
              6.0   08-Nov-2001	moved to the new Error handling routines, RvH
              5.0   01-Nov-2001	moved to new Error handling routines, RvH 
              4.0   24-Oct-2001 removed bytes_read parameter, RvH
              3.0   23-Aug-2001 moved to separate module, RvH
              2.0   13-Oct-1999 combined in one module and rewritten 
                                 SCIA_LV1_RD_MPH, READ_SPH and READ_DSD, RvH
              1.0   13-Aug-1999 created by R. M. van Hees
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
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

#define NUM_SPH1_ITEMS 25

static NADC_pds_hdr_t sph1_items[NUM_SPH1_ITEMS] = {
     {"SPH_DESCRIPTOR", PDS_String, 29, "", NULL},
     {"STRIPLINE_CONTINUITY_INDICATOR", PDS_Short, 4, "", NULL},
     {"SLICE_POSITION", PDS_Short, 4, "", NULL},
     {"NUM_SLICES", PDS_uShort, 4, "", NULL},
     {"START_TIME", PDS_String, UTC_STRING_LENGTH, "", NULL},
     {"STOP_TIME", PDS_String, UTC_STRING_LENGTH, "", NULL},
     {"START_LAT", PDS_Long, 11, "", "<10-6degN>"},
     {"START_LONG", PDS_Long, 11, "", "<10-6degE>"},
     {"STOP_LAT", PDS_Long, 11, "", "<10-6degN>"},
     {"STOP_LONG", PDS_Long, 11, "", "<10-6degE>"},
     {"INIT_VERSION", PDS_Optional, 37, "", NULL},
     {"KEY_DATA_VERSION", PDS_String, 6, "", NULL},
     {"M_FACTOR_VERSION", PDS_String, 6, "", NULL},
     {"SPECTRAL_CAL_CHECK_SUM", PDS_String, 5, "", NULL},
     {"SATURATED_PIXEL", PDS_String, 5, "", NULL},
     {"DEAD_PIXEL", PDS_String, 5, "", NULL},
     {"DARK_CHECK_SUM", PDS_String, 5, "", NULL},
     {"NO_OF_NADIR_STATES", PDS_uShort, 4, "", NULL},
     {"NO_OF_LIMB_STATES", PDS_uShort, 4, "", NULL},
     {"NO_OF_OCCULTATION_STATES", PDS_uShort, 4, "", NULL},
     {"NO_OF_MONI_STATES", PDS_uShort, 4, "", NULL},
     {"NO_OF_NOPROC_STATES", PDS_uShort, 4, "", NULL},
     {"COMP_DARK_STATES", PDS_uShort, 4, "", NULL},
     {"INCOMP_DARK_STATES", PDS_uShort, 4, "", NULL},
     {"", PDS_Spare, 4, "", NULL}
};
     
/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
#include <_envi_pds_hdr.inc>

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_SPH
.PURPOSE     read Specific Product Header of Level 1b Product
.INPUT/OUTPUT
  call as   SCIA_LV1_RD_SPH( fd, mph, &sph );
     input:  
            FILE   *fd            : (open) stream pointer
	    struct mph_envi mph   : main product header
    output:  
            struct sph1_scia *sph : structure for the SPH

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_RD_SPH( FILE *fd, const struct mph_envi mph,
		      struct sph1_scia *sph )
       /*@globals sph1_items;@*/
{
     int    ibuff;

     unsigned int nr_byte;

     const unsigned int Length_SPH = mph.sph_size - mph.num_dsd * mph.dsd_size;
/*
 * always initialize pds_hdr.type to PDS_Optional for "INIT_VERSION"
 */
     sph1_items[10].type = PDS_Optional;
/*
 * always rewind the file, skipping the MPH
 */
     (void) fseek( fd, (long) PDS_MPH_LENGTH, SEEK_SET );
/*
 * read PDS header
 */
     nr_byte = NADC_RD_PDS_HDR( fd, NUM_SPH1_ITEMS, sph1_items );
     if ( nr_byte != Length_SPH )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_SIZE, "SPH size" );
/*
 * fill sph1_scia struct
 */
     (void) nadc_strlcpy( sph->descriptor, sph1_items[0].value+1, 
			  (size_t) sph1_items[0].length );
     (void) sscanf( sph1_items[1].value, "%hd", &sph->stripline );
     (void) sscanf( sph1_items[2].value, "%hd", &sph->slice_pos );
     (void) sscanf( sph1_items[3].value, "%hu", &sph->no_slice );
     (void) nadc_strlcpy( sph->start_time, sph1_items[4].value+1, 
			  (size_t) sph1_items[4].length );
     (void) nadc_strlcpy( sph->stop_time, sph1_items[5].value+1, 
			  (size_t) sph1_items[5].length );
     (void) sscanf( sph1_items[6].value, "%d", &ibuff );
     sph->start_lat = ibuff / 1e6;
     (void) sscanf( sph1_items[7].value, "%d", &ibuff );
     sph->start_lon = ibuff / 1e6;
     (void) sscanf( sph1_items[8].value, "%d", &ibuff );
     sph->stop_lat = ibuff / 1e6;
     (void) sscanf( sph1_items[9].value, "%d", &ibuff );
     sph->stop_lon = ibuff / 1e6;
     (void) nadc_strlcpy( sph->init_version, sph1_items[10].value, 
			  (size_t) sph1_items[10].length+1 );
     (void) nadc_strlcpy( sph->key_data, sph1_items[11].value+1, 
			  (size_t) sph1_items[11].length );
     (void) nadc_strlcpy( sph->m_factor, sph1_items[12].value+1, 
			  (size_t) sph1_items[12].length );
     (void) nadc_strlcpy( sph->spec_cal, sph1_items[13].value+1, 
			  (size_t) sph1_items[13].length );
     (void) nadc_strlcpy( sph->saturate, sph1_items[14].value+1, 
			  (size_t) sph1_items[14].length );
     (void) nadc_strlcpy( sph->dead_pixel, sph1_items[15].value+1, 
			  (size_t) sph1_items[15].length );
     (void) nadc_strlcpy( sph->dark_check, sph1_items[16].value+1, 
			  (size_t) sph1_items[16].length );
     (void) sscanf( sph1_items[17].value, "%hu", &sph->no_nadir );
     (void) sscanf( sph1_items[18].value, "%hu", &sph->no_limb );
     (void) sscanf( sph1_items[19].value, "%hu", &sph->no_occult );
     (void) sscanf( sph1_items[20].value, "%hu", &sph->no_monitor );
     (void) sscanf( sph1_items[21].value, "%hu", &sph->no_noproc );
     (void) sscanf( sph1_items[22].value, "%hu", &sph->comp_dark );
     (void) sscanf( sph1_items[23].value, "%hu", &sph->incomp_dark );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_SPH
.PURPOSE     write Specific Product Header of Level 1b Product
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_SPH( fd, mph, sph );
     input:  
            FILE   *fd            : (open) stream pointer
	    struct mph_envi  mph  : main product header
            struct sph1_scia sph  : structure for the SPH

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_SPH( FILE *fd, const struct mph_envi mph,
		      const struct sph1_scia sph )
       /*@globals sph1_items;@*/
{
     int ibuff;

     const unsigned int Length_SPH = mph.sph_size - mph.num_dsd * mph.dsd_size;
/*
 * fill sph1_scia struct
 */
     NADC_WR_PDS_ITEM( sph1_items+0, sph.descriptor );
     NADC_WR_PDS_ITEM( sph1_items+1, &sph.stripline );
     NADC_WR_PDS_ITEM( sph1_items+2, &sph.slice_pos );
     NADC_WR_PDS_ITEM( sph1_items+3, &sph.no_slice );
     NADC_WR_PDS_ITEM( sph1_items+4, sph.start_time );
     NADC_WR_PDS_ITEM( sph1_items+5, sph.stop_time );
     ibuff = NINT(1e6 * sph.start_lat);
     NADC_WR_PDS_ITEM( sph1_items+6, &ibuff );
     ibuff = NINT(1e6 * sph.start_lon);
     NADC_WR_PDS_ITEM( sph1_items+7, &ibuff );
     ibuff = NINT(1e6 * sph.stop_lat);
     NADC_WR_PDS_ITEM( sph1_items+8, &ibuff );
     ibuff = NINT(1e6 * sph.stop_lon);
     NADC_WR_PDS_ITEM( sph1_items+9, &ibuff );
     NADC_WR_PDS_ITEM( sph1_items+10, sph.init_version );
     NADC_WR_PDS_ITEM( sph1_items+11, sph.key_data );
     NADC_WR_PDS_ITEM( sph1_items+12, sph.m_factor );
     NADC_WR_PDS_ITEM( sph1_items+13, sph.spec_cal );
     NADC_WR_PDS_ITEM( sph1_items+14, sph.saturate );
     NADC_WR_PDS_ITEM( sph1_items+15, sph.dead_pixel );
     NADC_WR_PDS_ITEM( sph1_items+16, sph.dark_check );
     NADC_WR_PDS_ITEM( sph1_items+17, &sph.no_nadir );
     NADC_WR_PDS_ITEM( sph1_items+18, &sph.no_limb );
     NADC_WR_PDS_ITEM( sph1_items+19, &sph.no_occult );
     NADC_WR_PDS_ITEM( sph1_items+20, &sph.no_monitor );
     NADC_WR_PDS_ITEM( sph1_items+21, &sph.no_noproc );
     NADC_WR_PDS_ITEM( sph1_items+22, &sph.comp_dark );
     NADC_WR_PDS_ITEM( sph1_items+23, &sph.incomp_dark );
/*
 * write PDS header
 */
     if ( NADC_WR_PDS_HDR( NUM_SPH1_ITEMS, sph1_items, fd ) != Length_SPH )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_SIZE, "SPH size" );
}
