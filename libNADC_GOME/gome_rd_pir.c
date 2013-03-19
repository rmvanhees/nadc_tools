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

.IDENTifer   GOME_RD_PIR
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1 data
.LANGUAGE    ANSI C
.PURPOSE     Read the Product Identifier Record
.INPUT/OUTPUT
  call as   GOME_RD_PIR( infl, &pir );

     input:  
            FILE   *infl          : (open) file descriptor 
    output:  
            struct pir_gome *pir  : structure for the PIR

.RETURNS     nothing
.COMMENTS    None
.ENVIRONment None
.VERSION     2.0     21-Aug-2001   same routine for level 1 and 2 data, RvH
             1.1     17-Feb-1999   conform to ANSI C3.159-1989, RvH
             1.0     10-Feb-1999   Created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_RD_PIR( FILE *infl, struct pir_gome *pir )
{
     const char prognm[] = "GOME_RD_PIR";

     char   pir_char[LVL1_PIR_LENGTH];
/*
 * rewind/read input data file
 */
     (void) fseek( infl, 0L, SEEK_SET );
     if ( fread( pir_char, LVL1_PIR_LENGTH, 1, infl ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "" );

     (void) memcpy( pir->mission, pir_char, 2 );
     pir->mission[2] = '\0';
     (void) memcpy( pir->sensor, pir_char+2, 3 );
     pir->sensor[3] = '\0';
     (void) memcpy( pir->orbit, pir_char+5, 5 );
     pir->orbit[5]  = '\0';
     (void) memcpy( pir->nr_orbit, pir_char+10, 4 );
     pir->nr_orbit[4] = '\0';
     (void) memcpy( pir->acquis, pir_char+14, 2 );
     pir->acquis[2] = '\0';
     (void) memcpy( pir->product, pir_char+16, 5 );
     pir->product[5] = '\0';
     (void) memcpy( pir->proc_id, pir_char+22, 2 );
     pir->proc_id[2] = '\0';
     (void) memcpy( pir->proc_date, pir_char+14, 8 );
     pir->proc_date[8] = '\0';
     (void) memcpy( pir->proc_time, pir_char+32, 6 );
     pir->proc_time[6] = '\0';
     return;
}
