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

.IDENTifer   SCIA_OL2_WR_ASCII_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    PDS SCIAMACHY headers
.LANGUAGE    ANSI C
.PURPOSE     Dump Specific Product Header of the Offline level 2 product
.INPUT/OUTPUT
  call as    SCIA_OL2_WR_ASCII_SPH( param, sph );
     input:  
             struct param_record param  : struct holding user-defined settings
	     struct sph_sci_ol *sph     : pointer to SPH structure

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     1.2     17-Oct-2006  update to latest format changes, RvH
             1.1     07-Dec-2005  write lat/lon values as doubles, RvH
             1.0     26-Apr-2002  created by R. M. van Hees
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

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_OL2_WR_ASCII_SPH( struct param_record param, 
			    const struct sph_sci_ol *sph )
{
     const char prognm[] = "SCIA_OL2_WR_ASCII_SPH";

     register unsigned int  nr = 0;

     FILE *outfl = CRE_ASCII_File( param.outfile, "sph" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of SPH record
 */
     nadc_write_header( outfl, nr, param.infile, 
			 "Specific Product Header of Level 2 Product" );
     nadc_write_text( outfl, ++nr, "SPH_DESCRIPTOR", sph->descriptor );
     nadc_write_short( outfl, ++nr, "STRIPLINE_CONTINUITY_INDICATOR", 
		       sph->stripline );
     nadc_write_short( outfl, ++nr, "SLICE_POSITION", sph->slice_pos );
     nadc_write_ushort( outfl, ++nr, "NUM_SLICES", sph->no_slice );
     nadc_write_text( outfl, ++nr, "START_TIME", sph->start_time );
     nadc_write_text( outfl, ++nr, "STOP_TIME", sph->stop_time );
     nadc_write_double( outfl, ++nr, "START_LAT", 6, sph->start_lat );
     nadc_write_double( outfl, ++nr, "START_LON", 6, sph->start_lon );
     nadc_write_double( outfl, ++nr, "STOP_LAT", 6, sph->stop_lat );
     nadc_write_double( outfl, ++nr, "STOP_LON", 6, sph->stop_lon );
     if ( strlen( sph->decont ) == 0 )
	  nadc_write_text( outfl, ++nr, "DECONT", sph->decont );
     else
	  ++nr;    /* spare_1 */
     nadc_write_text( outfl, ++nr, "DB_SERVER_VER", sph->dbserver );
     nadc_write_text( outfl, ++nr, "FITTING_ERROR_SUM", sph->errorsum );
/*
 * Nadir
 */
     nadc_write_ushort( outfl, ++nr, "NO_OF_NADIR_FITTING_WINDOWS", 
			 sph->no_nadir_win );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_UV0", sph->nadir_win_uv0 );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_UV1", sph->nadir_win_uv1 );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_UV2", sph->nadir_win_uv2 );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_UV3", sph->nadir_win_uv3 );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_UV4", sph->nadir_win_uv4 );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_UV5", sph->nadir_win_uv5 );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_UV6", sph->nadir_win_uv6 );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_UV7", sph->nadir_win_uv7 );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_UV8", sph->nadir_win_uv8 );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_UV9", sph->nadir_win_uv9 );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_IR0", sph->nadir_win_ir0 );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_IR1", sph->nadir_win_ir1 );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_IR2", sph->nadir_win_ir2 );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_IR3", sph->nadir_win_ir3 );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_IR4", sph->nadir_win_ir4 );
     nadc_write_text( outfl, ++nr, "NAD_FIT_WINDOW_IR5", sph->nadir_win_ir5 );
/*
 * Limb
 */
     nadc_write_ushort( outfl, ++nr, "NO_OF_LIMB_FITTING_WINDOWS", 
			 sph->no_limb_win );
     nadc_write_text( outfl, ++nr, "LIM_FIT_WINDOW_PTH", sph->limb_win_pth );
     nadc_write_text( outfl, ++nr, "LIM_FIT_WINDOW_UV0", sph->limb_win_uv0 );
     nadc_write_text( outfl, ++nr, "LIM_FIT_WINDOW_UV1", sph->limb_win_uv1 );
     nadc_write_text( outfl, ++nr, "LIM_FIT_WINDOW_UV2", sph->limb_win_uv2 );
     nadc_write_text( outfl, ++nr, "LIM_FIT_WINDOW_UV3", sph->limb_win_uv3 );
     nadc_write_text( outfl, ++nr, "LIM_FIT_WINDOW_UV4", sph->limb_win_uv4 );
     nadc_write_text( outfl, ++nr, "LIM_FIT_WINDOW_UV5", sph->limb_win_uv5 );
     nadc_write_text( outfl, ++nr, "LIM_FIT_WINDOW_UV6", sph->limb_win_uv6 );
     nadc_write_text( outfl, ++nr, "LIM_FIT_WINDOW_UV7", sph->limb_win_uv7 );
     nadc_write_text( outfl, ++nr, "LIM_FIT_WINDOW_IR0", sph->limb_win_ir0 );
     nadc_write_text( outfl, ++nr, "LIM_FIT_WINDOW_IR1", sph->limb_win_ir1 );
     nadc_write_text( outfl, ++nr, "LIM_FIT_WINDOW_IR2", sph->limb_win_ir2 );
     nadc_write_text( outfl, ++nr, "LIM_FIT_WINDOW_IR3", sph->limb_win_ir3 );
     nadc_write_text( outfl, ++nr, "LIM_FIT_WINDOW_IR4", sph->limb_win_ir4 );
/*
 * Occultation
 */
     nadc_write_ushort( outfl, ++nr, "NO_OF_OCCL_FITTING_WINDOWS", 
			 sph->no_occl_win );
     nadc_write_text( outfl, ++nr, "OCC_FIT_WINDOW_PTH", sph->occl_win_pth );
     nadc_write_text( outfl, ++nr, "OCC_FIT_WINDOW_UV0", sph->occl_win_uv0 );
     nadc_write_text( outfl, ++nr, "OCC_FIT_WINDOW_UV1", sph->occl_win_uv1 );
     nadc_write_text( outfl, ++nr, "OCC_FIT_WINDOW_UV2", sph->occl_win_uv2 );
     nadc_write_text( outfl, ++nr, "OCC_FIT_WINDOW_UV3", sph->occl_win_uv3 );
     nadc_write_text( outfl, ++nr, "OCC_FIT_WINDOW_UV4", sph->occl_win_uv4 );
     nadc_write_text( outfl, ++nr, "OCC_FIT_WINDOW_UV5", sph->occl_win_uv5 );
     nadc_write_text( outfl, ++nr, "OCC_FIT_WINDOW_UV6", sph->occl_win_uv6 );
     nadc_write_text( outfl, ++nr, "OCC_FIT_WINDOW_UV7", sph->occl_win_uv7 );
     nadc_write_text( outfl, ++nr, "OCC_FIT_WINDOW_IR0", sph->occl_win_ir0 );
     nadc_write_text( outfl, ++nr, "OCC_FIT_WINDOW_IR1", sph->occl_win_ir1 );
     nadc_write_text( outfl, ++nr, "OCC_FIT_WINDOW_IR2", sph->occl_win_ir2 );
     nadc_write_text( outfl, ++nr, "OCC_FIT_WINDOW_IR3", sph->occl_win_ir3 );
     nadc_write_text( outfl, ++nr, "OCC_FIT_WINDOW_IR4", sph->occl_win_ir4 );
     (void) fclose( outfl );
}
