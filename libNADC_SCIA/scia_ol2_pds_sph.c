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

.IDENTifer   SCIA_OL2_PDS_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Specific Product Header of Offline Level 2 Product
.COMMENTS    contains SCIA_OL2_RD_SPH and SCIA_OL2_WR_SPH
.ENVIRONment None
.VERSION      2.0   17-Oct-2006	complete rewrite & added write routine, RvH
              1.3   07-Dec-2005	store lat/lon values as doubles, RvH
	      1.2   12-Apr-2005	obtain SPH size from MPH, RvH
              1.1   16-Aug-2002	updated to ENV-ID-DLR-SCI-2200-4, RvH
              1.0   26-Apr-2002	created by R. M. van Hees 
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
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

#define NUM_SPH2_ITEMS 61

static NADC_pds_hdr_t sph2_items[NUM_SPH2_ITEMS] = {
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
     {"DECONT", PDS_Optional, 43, "", NULL},
     {"DB_SERVER_VER", PDS_String, 6, "", NULL},
     {"FITTING_ERROR_SUM", PDS_String, 5, "", NULL},
     {"NO_OF_NADIR_FITTING_WINDOWS", PDS_uShort, 4, "", NULL},
     {"NAD_FIT_WINDOW_UV0", PDS_String, 31, "", NULL},
     {"NAD_FIT_WINDOW_UV1", PDS_String, 31, "", NULL},
     {"NAD_FIT_WINDOW_UV2", PDS_String, 31, "", NULL},
     {"NAD_FIT_WINDOW_UV3", PDS_String, 31, "", NULL},
     {"NAD_FIT_WINDOW_UV4", PDS_String, 31, "", NULL},
     {"NAD_FIT_WINDOW_UV5", PDS_String, 31, "", NULL},
     {"NAD_FIT_WINDOW_UV6", PDS_String, 31, "", NULL},
     {"NAD_FIT_WINDOW_UV7", PDS_String, 31, "", NULL},
     {"NAD_FIT_WINDOW_UV8", PDS_Optional, 31, "", NULL},
     {"NAD_FIT_WINDOW_UV9", PDS_Optional, 31, "", NULL},
     {"NAD_FIT_WINDOW_IR0", PDS_String, 31, "", NULL},
     {"NAD_FIT_WINDOW_IR1", PDS_String, 31, "", NULL},
     {"NAD_FIT_WINDOW_IR2", PDS_String, 31, "", NULL},
     {"NAD_FIT_WINDOW_IR3", PDS_String, 31, "", NULL},
     {"NAD_FIT_WINDOW_IR4", PDS_String, 31, "", NULL},
     {"NAD_FIT_WINDOW_IR5", PDS_String, 31, "", NULL},
     {"NO_OF_LIMB_FITTING_WINDOWS", PDS_uShort, 4, "", NULL},
     {"LIM_FIT_WINDOW_PTH", PDS_String, 31, "", NULL},
     {"LIM_FIT_WINDOW_UV0", PDS_String, 31, "", NULL},
     {"LIM_FIT_WINDOW_UV1", PDS_String, 31, "", NULL},
     {"LIM_FIT_WINDOW_UV2", PDS_String, 31, "", NULL},
     {"LIM_FIT_WINDOW_UV3", PDS_String, 31, "", NULL},
     {"LIM_FIT_WINDOW_UV4", PDS_String, 31, "", NULL},
     {"LIM_FIT_WINDOW_UV5", PDS_String, 31, "", NULL},
     {"LIM_FIT_WINDOW_UV6", PDS_String, 31, "", NULL},
     {"LIM_FIT_WINDOW_UV7", PDS_String, 31, "", NULL},
     {"LIM_FIT_WINDOW_IR0", PDS_String, 31, "", NULL},
     {"LIM_FIT_WINDOW_IR1", PDS_String, 31, "", NULL},
     {"LIM_FIT_WINDOW_IR2", PDS_String, 31, "", NULL},
     {"LIM_FIT_WINDOW_IR3", PDS_String, 31, "", NULL},
     {"LIM_FIT_WINDOW_IR4", PDS_String, 31, "", NULL},
     {"NO_OF_OCCL_FITTING_WINDOWS", PDS_uShort, 4, "", NULL},
     {"OCC_FIT_WINDOW_PTH", PDS_String, 31, "", NULL},
     {"OCC_FIT_WINDOW_UV0", PDS_String, 31, "", NULL},
     {"OCC_FIT_WINDOW_UV1", PDS_String, 31, "", NULL},
     {"OCC_FIT_WINDOW_UV2", PDS_String, 31, "", NULL},
     {"OCC_FIT_WINDOW_UV3", PDS_String, 31, "", NULL},
     {"OCC_FIT_WINDOW_UV4", PDS_String, 31, "", NULL},
     {"OCC_FIT_WINDOW_UV5", PDS_String, 31, "", NULL},
     {"OCC_FIT_WINDOW_UV6", PDS_String, 31, "", NULL},
     {"OCC_FIT_WINDOW_UV7", PDS_String, 31, "", NULL},
     {"OCC_FIT_WINDOW_IR0", PDS_String, 31, "", NULL},
     {"OCC_FIT_WINDOW_IR1", PDS_String, 31, "", NULL},
     {"OCC_FIT_WINDOW_IR2", PDS_String, 31, "", NULL},
     {"OCC_FIT_WINDOW_IR3", PDS_String, 31, "", NULL},
     {"OCC_FIT_WINDOW_IR4", PDS_String, 31, "", NULL},
     {"", PDS_Spare, 64, "", NULL}
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
#include <_envi_pds_hdr.inc>

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_OL2_RD_SPH
.PURPOSE     read Specific Product Header of Level 2 Offline Product
.INPUT/OUTPUT
  call as   SCIA_OL2_RD_SPH( fd, mph, &sph );
     input:
            FILE   *fd             : (open) stream pointer
            struct mph_envi mph    : main product header
    output:
            struct sph_sci_ol *sph : structure for the SPH

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_OL2_RD_SPH( FILE *fd, const struct mph_envi mph,
		      struct sph_sci_ol *sph )
       /*@globals sph2_items;@*/
{
     const char prognm[] = "SCIA_OL2_RD_SPH";

     int    ibuff;

     unsigned int nr_byte;

     const unsigned int Length_SPH = mph.sph_size - mph.num_dsd * mph.dsd_size;
/*
 * always initialize pds_hdr.type to PDS_Optional for "DECONT"
 */
     sph2_items[10].type = PDS_Optional;
/*
 * always rewind the file, skipping the MPH
 */
     (void) fseek( fd, (long) PDS_MPH_LENGTH, SEEK_SET );
/*
 * read PDS header
 */
     nr_byte = NADC_RD_PDS_HDR( fd, NUM_SPH2_ITEMS, sph2_items );
     if ( nr_byte != Length_SPH )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_SIZE, "SPH size" );
/*
 * fill sph2_scia struct
 */
     (void) nadc_strlcpy( sph->descriptor, sph2_items[0].value+1, 
			  (size_t) sph2_items[0].length );
     (void) sscanf( sph2_items[1].value, "%hd", &sph->stripline );
     (void) sscanf( sph2_items[2].value, "%hd", &sph->slice_pos );
     (void) sscanf( sph2_items[3].value, "%hu", &sph->no_slice );
     (void) nadc_strlcpy( sph->start_time, sph2_items[4].value+1, 
			  (size_t) sph2_items[4].length );
     (void) nadc_strlcpy( sph->stop_time, sph2_items[5].value+1, 
			  (size_t) sph2_items[5].length );
     (void) sscanf( sph2_items[6].value, "%d", &ibuff );
     sph->start_lat = ibuff / 1e6;
     (void) sscanf( sph2_items[7].value, "%d", &ibuff );
     sph->start_lon = ibuff / 1e6;
     (void) sscanf( sph2_items[8].value, "%d", &ibuff );
     sph->stop_lat = ibuff / 1e6;
     (void) sscanf( sph2_items[9].value, "%d", &ibuff );
     sph->stop_lon = ibuff / 1e6;
     (void) nadc_strlcpy( sph->decont, sph2_items[10].value+1, 
			  (size_t) sph2_items[10].length );
     (void) nadc_strlcpy( sph->dbserver, sph2_items[11].value+1, 
			  (size_t) sph2_items[11].length );
     (void) nadc_strlcpy( sph->errorsum, sph2_items[12].value+1, 
			  (size_t) sph2_items[12].length );
     (void) sscanf( sph2_items[13].value, "%hu", &sph->no_nadir_win );
     (void) nadc_strlcpy( sph->nadir_win_uv0, sph2_items[14].value+1, 
			  (size_t) sph2_items[14].length );
     (void) nadc_strlcpy( sph->nadir_win_uv1, sph2_items[15].value+1, 
			  (size_t) sph2_items[15].length );
     (void) nadc_strlcpy( sph->nadir_win_uv2, sph2_items[16].value+1, 
			  (size_t) sph2_items[16].length );
     (void) nadc_strlcpy( sph->nadir_win_uv3, sph2_items[17].value+1, 
			  (size_t) sph2_items[17].length );
     (void) nadc_strlcpy( sph->nadir_win_uv4, sph2_items[18].value+1, 
			  (size_t) sph2_items[18].length );
     (void) nadc_strlcpy( sph->nadir_win_uv5, sph2_items[19].value+1, 
			  (size_t) sph2_items[19].length );
     (void) nadc_strlcpy( sph->nadir_win_uv6, sph2_items[20].value+1, 
			  (size_t) sph2_items[20].length );
     (void) nadc_strlcpy( sph->nadir_win_uv7, sph2_items[21].value+1, 
			  (size_t) sph2_items[21].length );
     (void) nadc_strlcpy( sph->nadir_win_uv8, sph2_items[22].value+1, 
			  (size_t) sph2_items[22].length );
     (void) nadc_strlcpy( sph->nadir_win_uv9, sph2_items[23].value+1, 
			  (size_t) sph2_items[23].length );
     (void) nadc_strlcpy( sph->nadir_win_ir0, sph2_items[24].value+1, 
			  (size_t) sph2_items[24].length );
     (void) nadc_strlcpy( sph->nadir_win_ir1, sph2_items[25].value+1, 
			  (size_t) sph2_items[25].length );
     (void) nadc_strlcpy( sph->nadir_win_ir2, sph2_items[26].value+1, 
			  (size_t) sph2_items[26].length );
     (void) nadc_strlcpy( sph->nadir_win_ir3, sph2_items[27].value+1, 
			  (size_t) sph2_items[27].length );
     (void) nadc_strlcpy( sph->nadir_win_ir4, sph2_items[28].value+1, 
			  (size_t) sph2_items[28].length );
     (void) nadc_strlcpy( sph->nadir_win_ir5, sph2_items[29].value+1, 
			  (size_t) sph2_items[29].length );
     (void) sscanf( sph2_items[30].value, "%hu", &sph->no_limb_win );
     (void) nadc_strlcpy( sph->limb_win_pth, sph2_items[31].value+1, 
			  (size_t) sph2_items[31].length );
     (void) nadc_strlcpy( sph->limb_win_uv0, sph2_items[32].value+1, 
			  (size_t) sph2_items[32].length );
     (void) nadc_strlcpy( sph->limb_win_uv1, sph2_items[33].value+1, 
			  (size_t) sph2_items[33].length );
     (void) nadc_strlcpy( sph->limb_win_uv2, sph2_items[34].value+1, 
			  (size_t) sph2_items[34].length );
     (void) nadc_strlcpy( sph->limb_win_uv3, sph2_items[35].value+1, 
			  (size_t) sph2_items[35].length );
     (void) nadc_strlcpy( sph->limb_win_uv4, sph2_items[36].value+1, 
			  (size_t) sph2_items[36].length );
     (void) nadc_strlcpy( sph->limb_win_uv5, sph2_items[37].value+1, 
			  (size_t) sph2_items[37].length );
     (void) nadc_strlcpy( sph->limb_win_uv6, sph2_items[38].value+1, 
			  (size_t) sph2_items[38].length );
     (void) nadc_strlcpy( sph->limb_win_uv7, sph2_items[39].value+1, 
			  (size_t) sph2_items[39].length );
     (void) nadc_strlcpy( sph->limb_win_ir0, sph2_items[40].value+1, 
			  (size_t) sph2_items[40].length );
     (void) nadc_strlcpy( sph->limb_win_ir1, sph2_items[41].value+1, 
			  (size_t) sph2_items[41].length );
     (void) nadc_strlcpy( sph->limb_win_ir2, sph2_items[42].value+1, 
			  (size_t) sph2_items[42].length );
     (void) nadc_strlcpy( sph->limb_win_ir3, sph2_items[43].value+1, 
			  (size_t) sph2_items[43].length );
     (void) nadc_strlcpy( sph->limb_win_ir4, sph2_items[44].value+1, 
			  (size_t) sph2_items[44].length );
     (void) sscanf( sph2_items[45].value, "%hu", &sph->no_occl_win );
     (void) nadc_strlcpy( sph->occl_win_pth, sph2_items[46].value+1, 
			  (size_t) sph2_items[46].length );
     (void) nadc_strlcpy( sph->occl_win_uv0, sph2_items[47].value+1, 
			  (size_t) sph2_items[47].length );
     (void) nadc_strlcpy( sph->occl_win_uv1, sph2_items[48].value+1, 
			  (size_t) sph2_items[48].length );
     (void) nadc_strlcpy( sph->occl_win_uv2, sph2_items[49].value+1, 
			  (size_t) sph2_items[49].length );
     (void) nadc_strlcpy( sph->occl_win_uv3, sph2_items[50].value+1, 
			  (size_t) sph2_items[50].length );
     (void) nadc_strlcpy( sph->occl_win_uv4, sph2_items[51].value+1, 
			  (size_t) sph2_items[51].length );
     (void) nadc_strlcpy( sph->occl_win_uv5, sph2_items[52].value+1, 
			  (size_t) sph2_items[52].length );
     (void) nadc_strlcpy( sph->occl_win_uv6, sph2_items[53].value+1, 
			  (size_t) sph2_items[53].length );
     (void) nadc_strlcpy( sph->occl_win_uv7, sph2_items[54].value+1, 
			  (size_t) sph2_items[54].length );
     (void) nadc_strlcpy( sph->occl_win_ir2, sph2_items[55].value+1, 
			  (size_t) sph2_items[55].length );
     (void) nadc_strlcpy( sph->occl_win_ir3, sph2_items[56].value+1, 
			  (size_t) sph2_items[56].length );
     (void) nadc_strlcpy( sph->occl_win_ir4, sph2_items[57].value+1, 
			  (size_t) sph2_items[57].length );
     (void) nadc_strlcpy( sph->occl_win_ir0, sph2_items[58].value+1, 
			  (size_t) sph2_items[58].length );
     (void) nadc_strlcpy( sph->occl_win_ir1, sph2_items[59].value+1, 
			  (size_t) sph2_items[59].length );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_OL2_WR_SPH
.PURPOSE     write Specific Product Header of Level 2 Offline Product
.INPUT/OUTPUT
  call as   SCIA_OL2_WR_SPH( fd, mph, sph );
     input:  
            FILE   *fd            : (open) stream pointer
	    struct mph_envi  mph  : main product header
            struct sph_sci_ol sph : structure for the SPH

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_OL2_WR_SPH( FILE *fd, const struct mph_envi mph,
		      const struct sph_sci_ol sph )
       /*@globals sph2_items;@*/
{
     const char prognm[] = "SCIA_OL2_WR_SPH";

     int ibuff;

     const unsigned int Length_SPH = mph.sph_size - mph.num_dsd * mph.dsd_size;
/*
 * fill sph2_scia struct
 */
     NADC_WR_PDS_ITEM( sph2_items+0, sph.descriptor );
     NADC_WR_PDS_ITEM( sph2_items+1, &sph.stripline );
     NADC_WR_PDS_ITEM( sph2_items+2, &sph.slice_pos );
     NADC_WR_PDS_ITEM( sph2_items+3, &sph.no_slice );
     NADC_WR_PDS_ITEM( sph2_items+4, sph.start_time );
     NADC_WR_PDS_ITEM( sph2_items+5, sph.stop_time );
     ibuff = NINT(1e6 * sph.start_lat);
     NADC_WR_PDS_ITEM( sph2_items+6, &ibuff );
     ibuff = NINT(1e6 * sph.start_lon);
     NADC_WR_PDS_ITEM( sph2_items+7, &ibuff );
     ibuff = NINT(1e6 * sph.stop_lat);
     NADC_WR_PDS_ITEM( sph2_items+8, &ibuff );
     ibuff = NINT(1e6 * sph.stop_lon);
     NADC_WR_PDS_ITEM( sph2_items+9, &ibuff );
     NADC_WR_PDS_ITEM( sph2_items+10, sph.decont );
     NADC_WR_PDS_ITEM( sph2_items+11, sph.dbserver );
     NADC_WR_PDS_ITEM( sph2_items+12, sph.errorsum );
     NADC_WR_PDS_ITEM( sph2_items+13, &sph.no_nadir_win );
     NADC_WR_PDS_ITEM( sph2_items+14, sph.nadir_win_uv0 );
     NADC_WR_PDS_ITEM( sph2_items+15, sph.nadir_win_uv1 );
     NADC_WR_PDS_ITEM( sph2_items+16, sph.nadir_win_uv2 );
     NADC_WR_PDS_ITEM( sph2_items+17, sph.nadir_win_uv3 );
     NADC_WR_PDS_ITEM( sph2_items+18, sph.nadir_win_uv4 );
     NADC_WR_PDS_ITEM( sph2_items+19, sph.nadir_win_uv5 );
     NADC_WR_PDS_ITEM( sph2_items+20, sph.nadir_win_uv6 );
     NADC_WR_PDS_ITEM( sph2_items+21, sph.nadir_win_uv7 );
     NADC_WR_PDS_ITEM( sph2_items+22, sph.nadir_win_uv8 );
     NADC_WR_PDS_ITEM( sph2_items+23, sph.nadir_win_uv9 );
     NADC_WR_PDS_ITEM( sph2_items+24, sph.nadir_win_ir0 );
     NADC_WR_PDS_ITEM( sph2_items+25, sph.nadir_win_ir1 );
     NADC_WR_PDS_ITEM( sph2_items+26, sph.nadir_win_ir2 );
     NADC_WR_PDS_ITEM( sph2_items+27, sph.nadir_win_ir3 );
     NADC_WR_PDS_ITEM( sph2_items+28, sph.nadir_win_ir4 );
     NADC_WR_PDS_ITEM( sph2_items+29, sph.nadir_win_ir5 );
     NADC_WR_PDS_ITEM( sph2_items+30, &sph.no_limb_win );
     NADC_WR_PDS_ITEM( sph2_items+31, sph.limb_win_pth );
     NADC_WR_PDS_ITEM( sph2_items+32, sph.limb_win_uv0 );
     NADC_WR_PDS_ITEM( sph2_items+33, sph.limb_win_uv1 );
     NADC_WR_PDS_ITEM( sph2_items+34, sph.limb_win_uv2 );
     NADC_WR_PDS_ITEM( sph2_items+35, sph.limb_win_uv3 );
     NADC_WR_PDS_ITEM( sph2_items+36, sph.limb_win_uv4 );
     NADC_WR_PDS_ITEM( sph2_items+37, sph.limb_win_uv5 );
     NADC_WR_PDS_ITEM( sph2_items+38, sph.limb_win_uv6 );
     NADC_WR_PDS_ITEM( sph2_items+39, sph.limb_win_uv7 );
     NADC_WR_PDS_ITEM( sph2_items+40, sph.limb_win_ir0 );
     NADC_WR_PDS_ITEM( sph2_items+41, sph.limb_win_ir1 );
     NADC_WR_PDS_ITEM( sph2_items+42, sph.limb_win_ir2 );
     NADC_WR_PDS_ITEM( sph2_items+43, sph.limb_win_ir3 );
     NADC_WR_PDS_ITEM( sph2_items+44, sph.limb_win_ir4 );
     NADC_WR_PDS_ITEM( sph2_items+45, &sph.no_occl_win );
     NADC_WR_PDS_ITEM( sph2_items+46, sph.occl_win_pth );
     NADC_WR_PDS_ITEM( sph2_items+47, sph.occl_win_uv0 );
     NADC_WR_PDS_ITEM( sph2_items+48, sph.occl_win_uv1 );
     NADC_WR_PDS_ITEM( sph2_items+49, sph.occl_win_uv2 );
     NADC_WR_PDS_ITEM( sph2_items+50, sph.occl_win_uv3 );
     NADC_WR_PDS_ITEM( sph2_items+51, sph.occl_win_uv4 );
     NADC_WR_PDS_ITEM( sph2_items+52, sph.occl_win_uv5 );
     NADC_WR_PDS_ITEM( sph2_items+53, sph.occl_win_uv6 );
     NADC_WR_PDS_ITEM( sph2_items+54, sph.occl_win_uv7 );
     NADC_WR_PDS_ITEM( sph2_items+55, sph.occl_win_ir0 );
     NADC_WR_PDS_ITEM( sph2_items+56, sph.occl_win_ir1 );
     NADC_WR_PDS_ITEM( sph2_items+57, sph.occl_win_ir2 );
     NADC_WR_PDS_ITEM( sph2_items+58, sph.occl_win_ir3 );
     NADC_WR_PDS_ITEM( sph2_items+59, sph.occl_win_ir4 );
/*
 * write PDS header
 */
     if ( NADC_WR_PDS_HDR( NUM_SPH2_ITEMS, sph2_items, fd ) != Length_SPH )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_SIZE, "SPH size" );
}
