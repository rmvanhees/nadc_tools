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

.IDENTifer   SCIA_LV0_WR_ASCII_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    PDS SCIAMACHY headers
.LANGUAGE    ANSI C
.PURPOSE     Dump Specific Product Header of the level 0 product
.INPUT/OUTPUT
  call as   SCIA_LV0_WR_ASCII_SPH( param, sph );
     input: 
            struct param_record param : struct holding user-defined settings
	    struct sph0_scia *sph     : pointer to structure with SPH record

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      6.0   08-Nov-2001	moved to the new Error handling routines, RvH
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
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV0_WR_ASCII_SPH( struct param_record param, 
			    const struct sph0_scia *sph )
{
     register unsigned int  nr = 0;

     FILE *outfl = CRE_ASCII_File( param.outfile, "sph" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of SPH record
 */
     nadc_write_header( outfl, nr, param.infile, 
			 "Specific Product Header of Level 0 Product" );
     nadc_write_text( outfl, ++nr, "SPH_DESCRIPTOR", sph->descriptor );
     nadc_write_double( outfl, ++nr, "START_LAT", 6, sph->start_lat );
     nadc_write_double( outfl, ++nr, "START_LON", 6, sph->start_lon );
     nadc_write_double( outfl, ++nr, "STOP_LAT", 6, sph->stop_lat );
     nadc_write_double( outfl, ++nr, "STOP_LON", 6, sph->stop_lon );
     nadc_write_double( outfl, ++nr, "SAT_TRACK", 6, sph->sat_track );
     nr++;    /* spare_1 */
     nadc_write_ushort( outfl, ++nr, "ISP_ERRORS_SIGNIFICANT", 
			sph->isp_errors );
     nadc_write_ushort( outfl, ++nr, "MISSING_ISPS_SIGNIFICANT", 
		       sph->missing_isps );
     nadc_write_ushort( outfl, ++nr, "ISP_DISCARDED_SIGNIFICANT", 
		       sph->isp_discard );
     nadc_write_ushort( outfl, ++nr, "RS_SIGNIFICANT", sph->rs_sign );
     nr++;    /* spare_2 */
     nadc_write_int( outfl, ++nr, "NUM_ERROR_ISPS", sph->num_error_isps );
     nadc_write_double( outfl, ++nr, "ERROR_ISPS_THRESH", 8,
		       sph->error_isps_thres );
     nadc_write_int( outfl, ++nr, "NUM_MISSING_ISPS", sph->num_miss_isps );
     nadc_write_double( outfl, ++nr, "MISSING_ISPS_THRESH", 8,
		       sph->miss_isps_thres );
     nadc_write_int( outfl, ++nr, "NUM_DISCARDED_ISPS", 
		       sph->num_discard_isps );
     nadc_write_double( outfl, ++nr, "DISCARDED_ISPS_THRESH", 8,
		       sph->discard_isps_thres );
     nadc_write_int( outfl, ++nr, "NUM_RS_ISPS", sph->num_rs_isps );
     nadc_write_double( outfl, ++nr, "RS_THRESH", 8,sph->rs_thres );
     nr++;    /* spare_3 */
     nadc_write_text( outfl, ++nr, "TX_TX_POLAR", sph->tx_rx_polar );
     nadc_write_text( outfl, ++nr, "SWATH", sph->swath );
     nr++;    /* spare_4 */

     (void) fclose( outfl );
}
