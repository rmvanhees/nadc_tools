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

.IDENTifer   ENVI_PDS_MPH
.AUTHOR      R.M. van Hees
.KEYWORDS    Envisat PDS headers
.LANGUAGE    ANSI C
.PURPOSE     read/write Main Product Header of the Envisat PDS product
.COMMENTS    contains ENVI_RD_MPH and ENVI_WR_MPH
.ENVIRONment None
.EXTERNALs   ENVI_RD_PDS_INFO
.VERSION      8.0   18-Sep-2008	renamed to geneal Envisat module, RvH
              7.1   11-Oct-2005	do not use lseek when writing(!); 
                                add usage of SCIA_LV1_ADD_DSD, RvH
              7.0   20-Apr-2005	complete rewrite & added write routine, RvH
              6.1   12-Dec-2001	updated documentation, RvH
              6.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              5.0   29-Oct-2001	moved to new Error handling routines, RvH 
              4.0   24-Oct-2001 removed bytes_read parameter, RvH
              3.1   10-Oct-2001 added list of external (NADC) function, RvH
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
#include <nadc_common.h>

#define NUM_MPH_ITEMS 41

static NADC_pds_hdr_t mph_items[NUM_MPH_ITEMS] = {
     {"PRODUCT", PDS_String, ENVI_FILENAME_SIZE, "", NULL},
     {"PROC_STAGE", PDS_Plain, 1, "", NULL},
     {"REF_DOC", PDS_String, 24, "", NULL},
     {"", PDS_Spare, 40, "", NULL},
     {"ACQUISITION_STATION", PDS_String, 21, "", NULL},
     {"PROC_CENTER", PDS_String, 7, "", NULL},
     {"PROC_TIME", PDS_String, UTC_STRING_LENGTH, "", NULL},
     {"SOFTWARE_VER", PDS_String, 15, "", NULL},
     {"", PDS_Spare, 40, "", NULL},
     {"SENSING_START", PDS_String, UTC_STRING_LENGTH, "", NULL},
     {"SENSING_STOP", PDS_String, UTC_STRING_LENGTH, "", NULL},
     {"", PDS_Spare, 40, "", NULL},
     {"PHASE", PDS_Plain, 1, "", NULL},
     {"CYCLE", PDS_Short, 4, "", NULL},
     {"REL_ORBIT", PDS_Long, 6, "", NULL},
     {"ABS_ORBIT", PDS_Long, 6, "", NULL},
     {"STATE_VECTOR_TIME", PDS_String, UTC_STRING_LENGTH, "", NULL},
     {"DELTA_UT1", PDS_Ado06, 8, "", "<s>"},
     {"X_POSITION", PDS_Ado73, 12, "", "<m>"},
     {"Y_POSITION", PDS_Ado73, 12, "", "<m>"},
     {"Z_POSITION", PDS_Ado73, 12, "", "<m>"},
     {"X_VELOCITY", PDS_Ado46, 12, "", "<m/s>"},
     {"Y_VELOCITY", PDS_Ado46, 12, "", "<m/s>"},
     {"Z_VELOCITY", PDS_Ado46, 12, "", "<m/s>"},
     {"VECTOR_SOURCE", PDS_String, 3, "", NULL},
     {"", PDS_Spare, 40, "", NULL},
     {"UTC_SBT_TIME", PDS_String, UTC_STRING_LENGTH, "", NULL},
     {"SAT_BINARY_TIME", PDS_uLong, 11, "", NULL},
     {"CLOCK_STEP", PDS_uLong, 11, "", "<ps>"},
     {"", PDS_Spare, 32, "", NULL},
     {"LEAP_UTC", PDS_String, UTC_STRING_LENGTH, "", NULL},
     {"LEAP_SIGN", PDS_Short, 4, "", NULL},
     {"LEAP_ERR", PDS_Plain, 1, "", NULL},
     {"", PDS_Spare, 40, "", NULL},
     {"PRODUCT_ERR", PDS_Plain, 1, "", NULL},
     {"TOT_SIZE", PDS_uLong, 21, "", "<bytes>"},
     {"SPH_SIZE", PDS_uLong, 11, "", "<bytes>"},
     {"NUM_DSD", PDS_uLong, 11, "", NULL},
     {"DSD_SIZE", PDS_uLong, 11, "", "<bytes>"},
     {"NUM_DATA_SETS", PDS_uLong, 11, "", NULL},
     {"", PDS_Spare, 40, "", NULL}
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
#include <_envi_pds_hdr.inc>

/*+++++++++++++++++++++++++
.IDENTifer   ENVI_RD_MPH
.PURPOSE     read Main Product Header of the Envisat PDS product
.INPUT/OUTPUT
  call as   ENVI_RD_MPH( fd, &mph );
     input:  
            FILE             *fd  : (open) stream pointer
    output:  
            struct mph_envi  *mph : structure for the MPH

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void ENVI_RD_MPH( FILE *fd, struct mph_envi *mph )
       /*@globals mph_items;@*/
{
/*
 * always rewind the file
 */
     if ( fseek( fd, 0L, SEEK_SET ) != 0 ) perror( __func__ );
/*
 * read PDS header
 */
     if ( NADC_RD_PDS_HDR( fd, NUM_MPH_ITEMS, mph_items ) != PDS_MPH_LENGTH )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_SIZE, "MPH size" );
/*
 * fill mph_envi struct
 */
     (void) nadc_strlcpy( mph->product, mph_items[0].value+1, 
			  (size_t) mph_items[0].length );
     (void) nadc_strlcpy( mph->proc_stage, mph_items[1].value, 
			  (size_t) mph_items[1].length+1 );
     (void) nadc_strlcpy( mph->ref_doc, mph_items[2].value+1, 
			  (size_t) mph_items[2].length );
/* spare */
     (void) nadc_strlcpy( mph->acquis, mph_items[4].value+1, 
			  (size_t) mph_items[4].length );
     (void) nadc_strlcpy( mph->proc_center, mph_items[5].value+1, 
			  (size_t) mph_items[5].length );
     (void) nadc_strlcpy( mph->proc_time, mph_items[6].value+1, 
			  (size_t) mph_items[6].length );
     (void) nadc_strlcpy( mph->soft_version, mph_items[7].value+1, 
			  (size_t) mph_items[7].length );
/* spare */
     (void) nadc_strlcpy( mph->sensing_start, mph_items[9].value+1, 
			  (size_t) mph_items[9].length );
     (void) nadc_strlcpy( mph->sensing_stop, mph_items[10].value+1, 
			  (size_t) mph_items[10].length );
/* spare */
     (void) nadc_strlcpy( mph->phase, mph_items[12].value, 
			  (size_t) mph_items[12].length+1 );
     (void) sscanf( mph_items[13].value, "%hd", &mph->cycle );
     (void) sscanf( mph_items[14].value, "%d", &mph->rel_orbit );
     (void) sscanf( mph_items[15].value, "%d", &mph->abs_orbit );
     (void) nadc_strlcpy( mph->state_vector, mph_items[16].value+1, 
			  (size_t) mph_items[16].length );
     (void) sscanf( mph_items[17].value, "%lf", &mph->delta_ut );
     (void) sscanf( mph_items[18].value, "%lf", &mph->x_position );
     (void) sscanf( mph_items[19].value, "%lf", &mph->y_position );
     (void) sscanf( mph_items[20].value, "%lf", &mph->z_position );
     (void) sscanf( mph_items[21].value, "%lf", &mph->x_velocity );
     (void) sscanf( mph_items[22].value, "%lf", &mph->y_velocity );
     (void) sscanf( mph_items[23].value, "%lf", &mph->z_velocity );
     (void) nadc_strlcpy( mph->vector_source, mph_items[24].value+1, 
			  (size_t) mph_items[24].length );
/* spare */
     (void) nadc_strlcpy( mph->utc_sbt_time, mph_items[26].value+1, 
			  (size_t) mph_items[26].length );
     (void) sscanf( mph_items[27].value, "%u", &mph->sat_binary_time );
     (void) sscanf( mph_items[28].value, "%u", &mph->clock_step );
/* spare */
     (void) nadc_strlcpy( mph->leap_utc, mph_items[30].value+1, 
			  (size_t) mph_items[30].length );
     (void) sscanf( mph_items[31].value, "%hd", &mph->leap_sign );
     (void) nadc_strlcpy( mph->leap_err, mph_items[32].value, 
			  (size_t) mph_items[32].length+1 );
/* spare */
     (void) nadc_strlcpy( mph->product_err, mph_items[34].value, 
			  (size_t) mph_items[34].length+1 );
     (void) sscanf( mph_items[35].value, "%u", &mph->tot_size );
     (void) sscanf( mph_items[36].value, "%u", &mph->sph_size );
     (void) sscanf( mph_items[37].value, "%u", &mph->num_dsd );
     (void) sscanf( mph_items[38].value, "%u", &mph->dsd_size );
     (void) sscanf( mph_items[39].value, "%u", &mph->num_data_sets );
/* spare */
}

/*+++++++++++++++++++++++++
.IDENTifer   ENVI_WR_MPH
.PURPOSE     write Main Product Header of the Envisat PDS product
.INPUT/OUTPUT
  call as   ENVI_WR_MPH( fd, mph );
     input:  
            FILE             *fd  : (open) stream pointer
            struct mph_envi  mph  : structure for the MPH

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void ENVI_WR_MPH( FILE *fd, const struct mph_envi mph )
       /*@globals mph_items;@*/
{
/*
 * fill struct "mph_envi"
 */
     NADC_WR_PDS_ITEM( mph_items+0, mph.product );
     NADC_WR_PDS_ITEM( mph_items+1, mph.proc_stage );
     NADC_WR_PDS_ITEM( mph_items+2, mph.ref_doc );
/* spare */
     NADC_WR_PDS_ITEM( mph_items+4, mph.acquis );
     NADC_WR_PDS_ITEM( mph_items+5, mph.proc_center );
     NADC_WR_PDS_ITEM( mph_items+6, mph.proc_time );
     NADC_WR_PDS_ITEM( mph_items+7, mph.soft_version );
/* spare */
     NADC_WR_PDS_ITEM( mph_items+9, mph.sensing_start );
     NADC_WR_PDS_ITEM( mph_items+10, mph.sensing_stop );
/* spare */
     NADC_WR_PDS_ITEM( mph_items+12, mph.phase );
     NADC_WR_PDS_ITEM( mph_items+13, &mph.cycle );
     NADC_WR_PDS_ITEM( mph_items+14, &mph.rel_orbit );
     NADC_WR_PDS_ITEM( mph_items+15, &mph.abs_orbit );
     NADC_WR_PDS_ITEM( mph_items+16, mph.state_vector );
     NADC_WR_PDS_ITEM( mph_items+17, &mph.delta_ut );
     NADC_WR_PDS_ITEM( mph_items+18, &mph.x_position );
     NADC_WR_PDS_ITEM( mph_items+19, &mph.y_position );
     NADC_WR_PDS_ITEM( mph_items+20, &mph.z_position );
     NADC_WR_PDS_ITEM( mph_items+21, &mph.x_velocity );
     NADC_WR_PDS_ITEM( mph_items+22, &mph.y_velocity );
     NADC_WR_PDS_ITEM( mph_items+23, &mph.z_velocity );
     NADC_WR_PDS_ITEM( mph_items+24, mph.vector_source );
/* spare */
     NADC_WR_PDS_ITEM( mph_items+26, mph.utc_sbt_time );
     NADC_WR_PDS_ITEM( mph_items+27, &mph.sat_binary_time );
     NADC_WR_PDS_ITEM( mph_items+28, &mph.clock_step );
/* spare */
     NADC_WR_PDS_ITEM( mph_items+30, mph.leap_utc );
     NADC_WR_PDS_ITEM( mph_items+31, &mph.leap_sign );
     NADC_WR_PDS_ITEM( mph_items+32, mph.leap_err );
/* spare */
     NADC_WR_PDS_ITEM( mph_items+34, mph.product_err );
     NADC_WR_PDS_ITEM( mph_items+35, &mph.tot_size );
     NADC_WR_PDS_ITEM( mph_items+36, &mph.sph_size );
     NADC_WR_PDS_ITEM( mph_items+37, &mph.num_dsd );
     NADC_WR_PDS_ITEM( mph_items+38, &mph.dsd_size );
     NADC_WR_PDS_ITEM( mph_items+39, &mph.num_data_sets );
/* spare */
/*
 * write PDS header
 */
     if ( NADC_WR_PDS_HDR( NUM_MPH_ITEMS, mph_items, fd ) != PDS_MPH_LENGTH )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_SIZE, "MPH size" );
}
