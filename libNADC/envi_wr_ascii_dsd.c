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

.IDENTifer   ENVI_WR_ASCII_DSD
.AUTHOR      R.M. van Hees
.KEYWORDS    Envisat PDS headers
.LANGUAGE    ANSI C
.PURPOSE     Dump Data Set Descriptor Records
.INPUT/OUTPUT
  call as   ENVI_WR_ASCII_DSD( param, num_dsd, dsd );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned int num_dsd :      number of DSD records
	     struct dsd_envi *dsd :      structure for the DSD records

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION     5.0     22-Sep-2008   renamed module to envi_wr_ascii_dsd, RvH
             4.1     10-Oct-2001   One call to dump all DSDs, RvH
             4.0     23-Aug-2001   Move to seperate module, RvH
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
#include <nadc_common.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void ENVI_WR_ASCII_DSD( struct param_record param, 
			unsigned int num_dsd,
			const struct dsd_envi *dsd )
{
     register unsigned int nd = 0, nr = 0;

     const char prognm[] = "ENVI_WR_ASCII_DSD";

     FILE *outfl = CRE_ASCII_File( param.outfile, "dsd" );

     if ( outfl == NULL || IS_ERR_STAT_FATAL )
          NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile );
/*
 * write ASCII dump of DSD record
 */
     nadc_write_header( outfl, nr, param.infile, 
			 "Data Set Descriptor Records" );
     do {
	  nadc_write_text( outfl, ++nr, "DS_NAME", dsd->name );
	  nadc_write_text( outfl, nr, "DS_TYPE", dsd->type );
	  nadc_write_text( outfl, nr, "FILENAME", dsd->flname );
	  nadc_write_uint( outfl, nr, "DS_OFFSET", dsd->offset );
	  nadc_write_uint( outfl, nr, "DS_SIZE", dsd->size );
	  nadc_write_uint( outfl, nr, "NUM_DSR", dsd->num_dsr );
	  nadc_write_int( outfl, nr, "DSR_SIZE", dsd->dsr_size );
	  dsd++;
     } while ( ++nd < num_dsd );

     (void) fclose( outfl );
}
