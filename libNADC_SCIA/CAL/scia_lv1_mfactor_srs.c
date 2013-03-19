/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2007 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_MFACTOR_SRS
.AUTHOR      Klaus Bramstedt (ife Bremen)
.KEYWORDS    SCIA Calibration
.LANGUAGE    ANSI C
.PURPOSE     apply mfactor M_CAL to SMR (ESM Diffuser: first spectrum)
.INPUT/OUTPUT
  call as    SCIA_LV1_MFACTOR_SRS ( sensing_start, calibFlag, num_dsr, srs );
     input:
            char *sensing_start     :  taken from MPH
	    unsigned int calibFlag  :  bit-flag which defines how to calibrate
	    unsigned int num_dsr    :  number of Sun reference spactra records
 in/output:
	     struct srs_scia *srs   :  Sun reference spectrum

.RETURNS     nothing
.COMMENTS    nothing
.ENVIRONment None
.VERSION      1.1   30-Jul-2007 Made this a seperate module, RvH
              1.0   07-Jun-2007	Initial release, Klaus Bramstedt (ife Bremen)
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_MFACTOR_SRS( const char *sensing_start, unsigned int calibFlag,
			   unsigned int num_dsr, struct srs_scia *srs_out )
{
     const char prognm[] = "SCIA_LV1_MFACTOR_SRS";

     float mfactor[SCIENCE_PIXELS];

     register unsigned int n_pix = 0;

/* nothing to do */
     if ( (calibFlag & DO_MFACTOR_RAD) == UINT_ZERO ) return;
     if ( num_dsr == 0 ) return;

/* obtain m-factor values */
    SCIA_RD_MFACTOR( M_CAL, sensing_start, calibFlag, mfactor );
    if ( IS_ERR_STAT_FATAL )
	 NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "SCIA_LV1_MFACTOR" );
/* 
 * Apply mfactor to first SMR, which is the ESM-diffuser spectrum
 *  for all known products.
 */
    do {
	 srs_out[0].mean_sun[n_pix] *= mfactor[n_pix];
    } while ( ++n_pix < SCIENCE_PIXELS);
}


