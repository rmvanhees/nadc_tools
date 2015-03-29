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

.IDENTifer   ENVI_WR_ASCII_MPH
.AUTHOR      R.M. van Hees
.KEYWORDS    Envisat PDS headers
.LANGUAGE    ANSI C
.PURPOSE     Dump Main Product Header
.INPUT/OUTPUT
  call as   ENVI_WR_ASCII_MPH( param, mph );
     input:  
             struct param_record param : struct holding user-defined settings
	     struct mph_envi *mph :      structure for the MPH

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      7.0   22-Sep-2008	renamed module to envi_wr_ascii_mph, RvH
              6.0   08-Nov-2001	moved to the new Error handling routines, RvH
              5.0   01-Nov-2001	moved to new Error handling routines, RvH 
              4.0   23-Aug-2001 move to seperate module, RvH
              3.0   03-Jan-2001 split the module "write_ascii", RvH
              2.2   21-Dec-2000 added SCIA_LV1_WR_ASCII_NADIR, RvH
              2.1   20-Dec-2000 use output filename given by the user, RvH
              2.0   17-Aug-2000 major rewrite and standardization, RvH
              1.1   14-Jul-2000 renamed: DEBUG -> SCIA_LV1_WR_ASCII, RvH
              1.0   02-Mar-1999 created by R. M. van Hees
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
#include <nadc_common.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void ENVI_WR_ASCII_MPH( struct param_record param, 
			const struct mph_envi *mph )
{
     register unsigned int  nr = 0;

     FILE *outfl = CRE_ASCII_File( param.outfile, "mph" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of MPH record
 */
     nadc_write_header( outfl, nr, param.infile, "Main Product Header" );
     nadc_write_text( outfl, ++nr, "PRODUCT", mph->product );
     nadc_write_text( outfl, ++nr, "PROC_STAGE", mph->proc_stage );
     nadc_write_text( outfl, ++nr, "REF_DOC", mph->ref_doc );
     nr++;    /* spare_1 */
     nadc_write_text( outfl, ++nr, "ACQUISITION_STATION", mph->acquis );
     nadc_write_text( outfl, ++nr, "PROC_CENTER", mph->proc_center );
     nadc_write_text( outfl, ++nr, "PROC_TIME", mph->proc_time );
     nadc_write_text( outfl, ++nr, "SOFTWARE_VER", mph->soft_version );
     nr++;    /* spare_2 */
     nadc_write_text( outfl, ++nr, "SENSING_START", mph->sensing_start );
     nadc_write_text( outfl, ++nr, "SENSING_STOP", mph->sensing_stop );
     nr++;    /* spare_3 */
     nadc_write_text( outfl, ++nr, "PHASE", mph->phase );
     nadc_write_short( outfl, ++nr, "CYCLE", mph->cycle );
     nadc_write_int( outfl, ++nr, "REL_ORBIT", mph->rel_orbit );
     nadc_write_int( outfl, ++nr, "ABS_ORBIT", mph->abs_orbit );
     nadc_write_text( outfl, ++nr, "STATE_VECTOR_TIME", mph->state_vector );
     nadc_write_double( outfl, ++nr, "DELTA_UT1", 3, mph->delta_ut );
     nadc_write_double( outfl, ++nr, "X_POSITION", 3, mph->x_position );
     nadc_write_double( outfl, ++nr, "Y_POSITION", 3, mph->y_position );
     nadc_write_double( outfl, ++nr, "Z_POSITION", 3, mph->z_position );
     nadc_write_double( outfl, ++nr, "X_VELOCITY", 6, mph->x_velocity );
     nadc_write_double( outfl, ++nr, "Y_VELOCITY", 6, mph->y_velocity );
     nadc_write_double( outfl, ++nr, "Z_VELOCITY", 6, mph->z_velocity );
     nr++;    /* spare_4 */
     nadc_write_text( outfl, ++nr, "VECTOR_SOURCE", mph->vector_source );
     nadc_write_text( outfl, ++nr, "UTC_SBT_TIME", mph->utc_sbt_time );
     nadc_write_uint( outfl, ++nr, "SAT_BINARY_TIME", mph->sat_binary_time );
     nadc_write_uint( outfl, ++nr, "CLOCK_STEP", mph->clock_step );
     nr++;    /* spare_5 */
     nadc_write_text( outfl, ++nr, "LEAP_UTC", mph->leap_utc );
     nadc_write_short( outfl, ++nr, "LEAP_SIGN", mph->leap_sign );
     nadc_write_text( outfl, ++nr, "LEAP_ERR", mph->leap_err );
     nr++;    /* spare_6 */
     nadc_write_text( outfl, ++nr, "PRODUCT_ERR", mph->product_err );
     nadc_write_uint( outfl, ++nr, "TOT_SIZE", mph->tot_size );
     nadc_write_uint( outfl, ++nr, "SPH_SIZE", mph->sph_size );
     nadc_write_uint( outfl, ++nr, "NUM_DSD", mph->num_dsd );
     nadc_write_uint( outfl, ++nr, "DSD_SIZE", mph->dsd_size );
     nadc_write_uint( outfl, ++nr, "NUM_DATA_SETS", mph->num_data_sets );
     nr++;    /* spare_7 */
     (void) fclose( outfl );
}
