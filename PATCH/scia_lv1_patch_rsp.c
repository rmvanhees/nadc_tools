/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2010 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_PATCH_RSP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY
.LANGUAGE    ANSI C
.PURPOSE     copy/patch PDS annotation datasets between files

.INPUT/OUTPUT
  call as   SCIA_LV1_PATCH_RSP( keydata, num_dsd, dsd, fp_in, fp_out );
     input:
            struct keydata_rec keydata :  SRON rsp, psp, srs keydata
            unsigned int num_dsd       :  number of DSDs
            struct dsd_envi *dsd       :  structure for the DSDs
	    FILE            *fp_in     :  file-pointer to input file
	    FILE            *fp_out    :  file-pointer to output file

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     2.0     22-Nov-2010   rewrite, RvH
             1.1     07-Dec-2005   fixed memory leakage, RvH        
             1.0     24-Apr-2005   initial release by R. M. van Hees
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

#include <hdf5.h>

/*+++++ Local Headers +++++*/
#define _SCIA_PATCH_1
#include <defs_nadc.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_PATCH_RSPN( const struct keydata_rec *keydata,
			  unsigned int num_dsd, const struct dsd_envi *dsd,
			  FILE *fp_in, FILE *fp_out )
{
     const char prognm[] = "SCIA_LV1_PATCH_RSPN";

     int num_rspn = 0;

     struct rspn_scia  *rspn;
/*
 * read (G)ADS
 */
     num_rspn = SCIA_LV1_RD_RSPN( fp_in, num_dsd, dsd, &rspn );
     if ( IS_ERR_STAT_FATAL )
          NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "RSPN" );
/*
 * patch keydata
 */
     if ( keydata != NULL && num_rspn == NUM_KEY_RSPN ) {
	  (void) memcpy( rspn, keydata->rspn, 
			 NUM_KEY_RSPN * sizeof( struct rspn_scia ));
     }
/*
 * write (G)ADS
 */
     (void) SCIA_LV1_WR_RSPN( fp_out, num_rspn, rspn );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "RSPN" );
     if ( num_rspn > 0 ) free( rspn );
}

void SCIA_LV1_PATCH_RSPL( const struct keydata_rec *keydata,
			  unsigned int num_dsd, const struct dsd_envi *dsd,
			  FILE *fp_in, FILE *fp_out )
{
     const char prognm[] = "SCIA_LV1_PATCH_RSPL";

     int num_rspl = 0;

     struct rsplo_scia  *rspl;
/*
 * read (G)ADS
 */
     num_rspl = SCIA_LV1_RD_RSPL( fp_in, num_dsd, dsd, &rspl );
     if ( IS_ERR_STAT_FATAL )
          NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "RSPL" );
/*
 * patch keydata
 */
     if ( keydata != NULL && num_rspl == NUM_KEY_RSPL ) {
	  (void) memcpy( rspl, keydata->rspl, 
			 NUM_KEY_RSPL * sizeof( struct rsplo_scia ));
     }
/*
 * write (G)ADS
 */
     (void) SCIA_LV1_WR_RSPL( fp_out, num_rspl, rspl );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "RSPL" );
     if ( num_rspl > 0 ) free( rspl );
}
