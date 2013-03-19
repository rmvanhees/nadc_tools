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

.IDENTifer   SCIA_LV1_PATCH_SRS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY
.LANGUAGE    ANSI C
.PURPOSE     copy/patch PDS annotation datasets between files

.INPUT/OUTPUT
  call as   SCIA_LV1_PATCH_SRS( keydata, num_dsd, dsd, fp_in, fp_out );
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
.VERSION     1.0     22-Nov-2010   initial release by R. M. van Hees
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
void SCIA_LV1_PATCH_SRS( const struct keydata_rec *keydata,
			 unsigned int num_dsd, const struct dsd_envi *dsd,
			 FILE *fp_in, FILE *fp_out )
{
     const char prognm[] = "SCIA_LV1_PATCH_SRS";

     int num_srs = 0;

     struct srs_scia  *srs;
/*
 * read (G)ADS
 */
     num_srs = SCIA_LV1_RD_SRS( fp_in, num_dsd, dsd, &srs );
     if ( IS_ERR_STAT_FATAL )
          NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "SRS" );
/*
 * patch keydata
 */
     if ( keydata != NULL && num_srs == NUM_KEY_SRS ) {
	  (void) memcpy( srs[0].mean_sun, keydata->srs[0].mean_sun, 
			 SCIENCE_PIXELS * sizeof(float) );
     }
/*
 * write (G)ADS
 */
     (void) SCIA_LV1_WR_SRS( fp_out, num_srs, srs );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "SRS" );
     if ( num_srs > 0 ) free( srs );
}
