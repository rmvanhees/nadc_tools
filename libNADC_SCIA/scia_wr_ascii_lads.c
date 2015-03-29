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

.IDENTifer   SCIA_WR_ASCII_LADS
.AUTHOR      R.M. van Hees
.KEYWORDS    PDS SCIAMACHY level 1b/2
.LANGUAGE    ANSI C
.PURPOSE     Dump Geolocation of State
.INPUT/OUTPUT
  call as   SCIA_WR_ASCII_LADS( param, num_dsr, lads );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned int num_dsr :      number of LADS records
	     struct lads_scia *lads :    structure for the LADS records

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION     1.1     07-Dec-2005   write lat/lon values as doubles, RvH
             1.0     13-Sep-2001   Moved from scia_lv1_wr_ascii_ads.c, RvH
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
#define _SCIA_COMMON
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_WR_ASCII_LADS( struct param_record param, unsigned int num_dsr,
			 const struct lads_scia *lads )
{
     register int nc;
     register unsigned int nd, nr;

     char  date_str[UTC_STRING_LENGTH];
     unsigned int  count[2];
     double latlon[2 * NUM_CORNERS];

     FILE  *outfl = CRE_ASCII_File( param.outfile, "lads" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
          NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of LADS record
 */
     nadc_write_header( outfl, 0, param.infile, "Geolocation of the States" );
     for ( nd = 0; nd < num_dsr; nd++ ) {
	  nr = 1;
	  (void) MJD_2_ASCII( lads[nd].mjd.days, lads[nd].mjd.secnd,
			      lads[nd].mjd.musec, date_str );
	  nadc_write_text( outfl, nr++, "Date", date_str );
	  nadc_write_uchar( outfl, nr++, "MDS DSR attached", 
			    lads[nd].flag_mds );
	  count[0] = NUM_CORNERS;
	  count[1] = 2;
	  for ( nc = 0; nc < NUM_CORNERS; nc++ ) {
	       latlon[nc] = lads[nd].corner[nc].lat / 1e6;
	       latlon[NUM_CORNERS + nc] = lads[nd].corner[nc].lon / 1e6;
	  }
	  nadc_write_arr_double( outfl, nr,"Pixel corner coordinates",
				 2, count, 6, latlon );
     }
     (void) fclose( outfl );
}
