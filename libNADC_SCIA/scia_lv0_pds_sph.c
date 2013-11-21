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

.IDENTifer   SCIA_LV0_PDS_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Specific Product Header of Level 0 product
.COMMENTS    contains SCIA_LV0_RD_SPH
.ENVIRONment None
.EXTERNALs   ENVI_RD_PDS_INFO
.VERSION      6.2   17-Nov-2005	minor bugfix in SCIA_LV0_WR_SPH, RvH
              6.1   11-Oct-2005	do not use lseek when writing(!), RvH
              6.0   21-Apr-2005	complete rewrite & added write routine, RvH
              5.2   12-Apr-2005	obtain SPH size from MPH, RvH
              5.1   12-Dec-2001	updated documentation, RvH
              5.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              4.0   29-Oct-2001	moved to new Error handling routines, RvH 
              3.0   24-Oct-2001 removed bytes_read parameter, RvH
              2.1   10-Oct-2001 added list of external (NADC) function, RvH
              2.0   23-Aug-2001 moved to separate module, RvH
              1.0   27-Mar-2001 created by R. M. van Hees
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
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

#define NUM_SPH0_ITEMS 24

static NADC_pds_hdr_t sph0_items[NUM_SPH0_ITEMS] = {
     {"SPH_DESCRIPTOR", PDS_String, 29, "", NULL},
     {"START_LAT", PDS_Long, 11, "", "<10-6degN>"},
     {"START_LONG", PDS_Long, 11, "", "<10-6degN>"},
     {"STOP_LAT", PDS_Long, 11, "", "<10-6degN>"},
     {"STOP_LONG", PDS_Long, 11, "", "<10-6degN>"},
     {"SAT_TRACK", PDS_Ado18e, 15, "", "<deg>"},
     {"", PDS_Spare, 50, "", NULL},
     {"ISP_ERRORS_SIGNIFICANT", PDS_Plain, 1, "", NULL},
     {"MISSING_ISPS_SIGNIFICANT", PDS_Plain, 1, "", NULL},
     {"ISP_DISCARDED_SIGNIFICANT", PDS_Plain, 1, "", NULL},
     {"RS_SIGNIFICANT", PDS_Plain, 1, "", NULL},
     {"", PDS_Spare, 50, "", NULL},
     {"NUM_ERROR_ISPS", PDS_Long, 11, "", NULL},
     {"ERROR_ISPS_THRESH", PDS_Ado18e, 15, "", "<%>"},
     {"NUM_MISSING_ISPS", PDS_Long, 11, "", NULL},
     {"MISSING_ISPS_THRESH", PDS_Ado18e, 15, "", "<%>"},
     {"NUM_DISCARDED_ISPS", PDS_Long, 11, "", NULL},
     {"DISCARDED_ISPS_THRESH", PDS_Ado18e, 15, "", "<%>"},
     {"NUM_RS_ISPS", PDS_Long, 11, "", NULL},
     {"RS_THRESH", PDS_Ado18e, 15, "", "<%>"},
     {"", PDS_Spare, 100, "", NULL},
     {"TX_RX_POLAR", PDS_String, 6, "", NULL},
     {"SWATH", PDS_String, 4, "", NULL},
     {"", PDS_Spare, 41, "", NULL}
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
#include <_envi_pds_hdr.inc>

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_SPH
.PURPOSE     read Specific Product Header of Level 0 Product
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_SPH( fd, mph, &sph );
     input:  
            FILE   *fd            : (open) stream pointer
	    struct mph_envi mph   : main product header
    output:  
            struct sph0_scia *sph : structure for the SPH

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV0_RD_SPH( FILE *fd, const struct mph_envi mph,
		      struct sph0_scia *sph )
       /*@globals sph0_items;@*/
{
     const char prognm[] = "SCIA_LV0_RD_SPH";

     int            ibuff;

     const unsigned int Length_SPH = mph.sph_size - mph.num_dsd * mph.dsd_size;
/*
 * always rewind the file, skipping the MPH
 */
     (void) fseek( fd, (long) PDS_MPH_LENGTH, SEEK_SET );
/*
 * read PDS header
 */
     if ( NADC_RD_PDS_HDR( fd, NUM_SPH0_ITEMS, sph0_items ) != Length_SPH )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_SIZE, "SPH size" );
/*
 * fill sph0_scia struct
 */
     (void) nadc_strlcpy( sph->descriptor, sph0_items[0].value+1, 
			  (size_t) sph0_items[0].length );
     (void) sscanf( sph0_items[1].value, "%d", &ibuff );
     sph->start_lat = ibuff / 1e6;
     (void) sscanf( sph0_items[2].value, "%d", &ibuff );
     sph->start_lon = ibuff / 1e6;
     (void) sscanf( sph0_items[3].value, "%d", &ibuff );
     sph->stop_lat = ibuff / 1e6;
     (void) sscanf( sph0_items[4].value, "%d", &ibuff );
     sph->stop_lon = ibuff / 1e6;
     (void) sscanf( sph0_items[5].value, "%lf", &sph->sat_track );
/* spare */
     (void) sscanf( sph0_items[7].value, "%hu", &sph->isp_errors );
     (void) sscanf( sph0_items[8].value, "%hu", &sph->missing_isps );
     (void) sscanf( sph0_items[9].value, "%hu", &sph->isp_discard );
     (void) sscanf( sph0_items[10].value, "%hu", &sph->rs_sign );
/* spare */
     (void) sscanf( sph0_items[12].value, "%d", &sph->num_error_isps );
     (void) sscanf( sph0_items[13].value, "%lf", &sph->error_isps_thres );
     (void) sscanf( sph0_items[14].value, "%d", &sph->num_miss_isps );
     (void) sscanf( sph0_items[15].value, "%lf", &sph->miss_isps_thres );
     (void) sscanf( sph0_items[16].value, "%d", &sph->num_discard_isps );
     (void) sscanf( sph0_items[17].value, "%lf", &sph->discard_isps_thres );
     (void) sscanf( sph0_items[18].value, "%d", &sph->num_rs_isps );
     (void) sscanf( sph0_items[19].value, "%lf", &sph->rs_thres );
/* spare */
     (void) nadc_strlcpy( sph->tx_rx_polar, sph0_items[21].value+1,
			  (size_t) sph0_items[21].length );
     (void) nadc_strlcpy( sph->swath, sph0_items[22].value+1, 
			  (size_t) sph0_items[22].length );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_WR_SPH
.PURPOSE     write Specific Product Header of Level 0 Product
.INPUT/OUTPUT
  call as   SCIA_LV0_WR_SPH( fd, mph, sph );
     input:  
            FILE   *fd            : (open) stream pointer
	    struct mph_envi mph   : main product header
            struct sph0_scia sph  : structure for the SPH

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV0_WR_SPH( FILE *fd, const struct mph_envi mph,
		      const struct sph0_scia sph )
       /*@globals sph0_items;@*/
{
     const char prognm[] = "SCIA_LV0_WR_SPH";

     int ibuff;

     const unsigned int Length_SPH = mph.sph_size - mph.num_dsd * mph.dsd_size;
/*
 * fill sph0_scia struct
 */
     NADC_WR_PDS_ITEM( sph0_items+0, sph.descriptor );
     ibuff = NINT(1e6 * sph.start_lat);
     NADC_WR_PDS_ITEM( sph0_items+1, &ibuff );
     ibuff = NINT(1e6 * sph.start_lon);
     NADC_WR_PDS_ITEM( sph0_items+2, &ibuff );
     ibuff = NINT(1e6 * sph.stop_lat);
     NADC_WR_PDS_ITEM( sph0_items+3, &ibuff );
     ibuff = NINT(1e6 * sph.stop_lon);
     NADC_WR_PDS_ITEM( sph0_items+4, &ibuff );
     NADC_WR_PDS_ITEM( sph0_items+5, &sph.sat_track );
     NADC_WR_PDS_ITEM( sph0_items+7, &sph.isp_errors );
     NADC_WR_PDS_ITEM( sph0_items+8, &sph.missing_isps );
     NADC_WR_PDS_ITEM( sph0_items+9, &sph.isp_discard );
     NADC_WR_PDS_ITEM( sph0_items+10, &sph.rs_sign );
     NADC_WR_PDS_ITEM( sph0_items+12, &sph.num_error_isps );
     NADC_WR_PDS_ITEM( sph0_items+13, &sph.error_isps_thres );
     NADC_WR_PDS_ITEM( sph0_items+14, &sph.num_miss_isps );
     NADC_WR_PDS_ITEM( sph0_items+15, &sph.miss_isps_thres );
     NADC_WR_PDS_ITEM( sph0_items+16, &sph.num_discard_isps );
     NADC_WR_PDS_ITEM( sph0_items+17, &sph.discard_isps_thres );
     NADC_WR_PDS_ITEM( sph0_items+18, &sph.num_rs_isps );
     NADC_WR_PDS_ITEM( sph0_items+19, &sph.rs_thres );
     NADC_WR_PDS_ITEM( sph0_items+21, &sph.tx_rx_polar );
     NADC_WR_PDS_ITEM( sph0_items+22, &sph.swath );
/*
 * write PDS header
 */
     if ( NADC_WR_PDS_HDR( NUM_SPH0_ITEMS, sph0_items, fd ) != Length_SPH )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_SIZE, "SPH size" );
}
