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

.IDENTifer   SCIA_LV0_WR_ASCII_INFO
.AUTHOR      R.M. van Hees
.KEYWORDS    PDS SCIAMACHY headers
.LANGUAGE    ANSI C
.PURPOSE     Dump collected Info-records of an SCIA level 0 file
.INPUT/OUTPUT
  call as   SCIA_LV0_WR_ASCII_INFO( param, num_info, info );
     input: 
            struct param_record param : struct holding user-defined settings
	    unsigned int num_info     : number of info-records
	    struct mds0_info *info    : pointer to structure with info-records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0     06-Jan-2003   Created by R. M. van Hees
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
void SCIA_LV0_WR_ASCII_INFO( struct param_record param, unsigned int num_info,
			     const struct mds0_info *info )
{
     const char prognm[] = "SCIA_LV0_WR_ASCII_INFO";

     register unsigned int ni;

     char  date_str[UTC_STRING_LENGTH];

     FILE *outfl = CRE_ASCII_File( param.outfile, "info" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of INFO records
 */
     nadc_write_header( outfl, 0, param.infile, 
			"Info records of a Level 0 Product" );
     for ( ni = 0; ni < num_info; ni++ ) {
	  (void) MJD_2_ASCII( info[ni].mjd.days, info[ni].mjd.secnd, 
			      info[ni].mjd.musec, date_str );
	  nadc_write_text( outfl, ni, "ISP (Date Time)", date_str );
	  nadc_write_double( outfl, ni, "ISP (Julian Day)", 16, 
			     (double) info[ni].mjd.days + 
			     ((info[ni].mjd.secnd + info[ni].mjd.musec / 1e6)
			      / (24. * 60 * 60)) );
	  nadc_write_uchar( outfl, ni, "Packet ID", info[ni].packet_type );
	  nadc_write_uchar( outfl, ni, "Category", info[ni].category );
	  nadc_write_uchar( outfl, ni, "State ID", info[ni].state_id );
	  nadc_write_uchar( outfl, ni, "Quality", info[ni].quality );
	  nadc_write_ushort( outfl, ni, "CRC errors", info[ni].crc_errors );
	  nadc_write_ushort( outfl, ni, "Reed-Solomon errors", 
	       info[ni].rs_errors );
	  nadc_write_ushort( outfl, ni, "BCPS", info[ni].bcps );
	  nadc_write_ushort( outfl, ni, "State counter", info[ni].state_index );
	  nadc_write_ushort( outfl, ni, "Data packet length", 
			     info[ni].packet_length );
	  nadc_write_uint( outfl, ni, "ICU on-board-time", 
			   info[ni].on_board_time );
	  nadc_write_uint( outfl, ni, "Offset in file", info[ni].offset );
     }
     (void) fclose( outfl );
}
