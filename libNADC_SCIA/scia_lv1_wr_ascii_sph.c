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

.IDENTifer   SCIA_LV1_WR_ASCII_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    PDS SCIAMACHY headers
.LANGUAGE    ANSI C
.PURPOSE     Dump Specific Product Header of the level 1b product
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_ASCII_SPH( param, sph );
     input:  
             struct param_record param  : struct holding user-defined settings
	     struct sph1_scia *sph      : pointer to SPH structure

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     4.0     23-Aug-2001   Move to seperate module, RvH
             3.0     03-Jan-2001   Split the module "write_ascii", RvH
             2.2     21-Dec-2000   Added SCIA_LV1_WR_ASCII_NADIR, RvH
             2.1     20-Dec-2000   Use output filename given by the user, RvH
             2.0     17-Aug-2000   Major rewrite and standardization, RvH
             1.1     14-Jul-2000   Renamed: DEBUG -> SCIA_LV1_WR_ASCII, RvH
             1.0     02-Mar-1999   Created by R. M. van Hees
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
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_ASCII_SPH( struct param_record param, 
			    const struct sph1_scia *sph )
{
     register unsigned int  nr = 0;

     FILE *outfl = CRE_ASCII_File( param.outfile, "sph" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of SPH record
 */
     nadc_write_header( outfl, nr, param.infile, 
			 "Specific Product Header of Level 1b Product" );
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
     nr++;    /* spare_1 */
     nadc_write_text( outfl, ++nr, "KEY_DATA_VERSION", sph->key_data );
     nadc_write_text( outfl, ++nr, "M_FACTOR_VERSION", sph->m_factor );
     nadc_write_text( outfl, ++nr, "SPECTRAL_CAL_CHECK_SUM", sph->spec_cal );
     nadc_write_text( outfl, ++nr, "SATURATED_PIXEL", sph->saturate );
     nadc_write_text( outfl, ++nr, "DEAD_PIXEL", sph->dead_pixel );
     nadc_write_text( outfl, ++nr, "DARK_CHECK_SUM", sph->dark_check );
     nadc_write_ushort( outfl, ++nr, "NO_OF_NADIR_STATES", sph->no_nadir );
     nadc_write_ushort( outfl, ++nr, "NO_OF_LIMB_STATES", sph->no_limb );
     nadc_write_ushort( outfl, ++nr, "NO_OF_OCCULTATION_STATES", 
			sph->no_occult );
     nadc_write_ushort( outfl, ++nr, "NO_OF_MONI_STATES", sph->no_monitor );
     nadc_write_ushort( outfl, ++nr, "NO_OF_NOPROC_STATES", sph->no_noproc );
     nadc_write_ushort( outfl, ++nr, "COMP_DARK_STATES", sph->comp_dark );
     nadc_write_ushort( outfl, ++nr, "INCOMP_DARK_STATES", sph->incomp_dark );
     nr++;    /* spare_2 */
     (void) fclose( outfl );
}

