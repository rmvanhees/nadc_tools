/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2006 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1C_CAL
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 1c product
.LANGUAGE    ANSI C
.PURPOSE     calibrate Science data
.INPUT/OUTPUT
  call as    SCIA_LV1C_CAL( calib_flag, num_mds, mds_1c );
     input:
	    unsigned int calib_flag   : bit-flag which defines how to calibrate
            unsigned int num_mds      : number of MDS records with L0 data
    output:
            struct mds1c_scia *mds_1c : level 1c MDS records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   30-May-2010 renamed to scia_lv1c_cal.c, RvH
              1.0   16-Nov-2006	Initial release, Richard van Hees (SRON)
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
#include <limits.h>
#include <math.h>
 
/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>
#include <nadc_scia_cal.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static
void SCIA_LV1C_CAL_COADD( unsigned short num_mds, struct mds1c_scia *mds_1c )
     /*@modifies mds_1c->pixel_val@*/
{
     register unsigned short num = 0u;     /* counter for number of clusters */

     do {
	  register unsigned short nobs = 0;
	  register float *signal = mds_1c->pixel_val;

	  do {
	       register unsigned short npix = 0;

	       do {
		    *signal++ /= mds_1c->coaddf;
	       } while ( ++npix < mds_1c->num_pixels );
	  } while( ++nobs < mds_1c->num_obs );
     } while ( mds_1c++, ++num < num_mds );
}

static
void SCIA_LV1C_CAL_NORM( unsigned short num_mds, struct mds1c_scia *mds_1c )
     /*@modifies mds_1c->pixel_val@*/
{
     register unsigned short num = 0u;     /* counter for number of clusters */

     do {
	  register unsigned short nobs = 0;
	  register float *signal = mds_1c->pixel_val;

	  do {
	       register unsigned short npix = 0;

	       do {
		    *signal++ /= mds_1c->pet;
	       } while ( ++npix < mds_1c->num_pixels );
	  } while( ++nobs < mds_1c->num_obs );
     } while ( mds_1c++, ++num < num_mds );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1C_CAL( int absOrbit, unsigned int calib_flag, 
		    unsigned short num_mds, struct mds1c_scia *mds_1c )
{
     const char prognm[] = "SCIA_LV1C_CAL";
/*
 * return directly if nothing has to be done
 */
     if ( calib_flag == CALIB_NONE ) return;
/*
 * apply memory correction (chan 1-5)
 */
     if ( (calib_flag & DO_CORR_VIS_MEM) != UINT_ZERO ) {
	  SCIA_SRON_CAL_MEM( num_mds, mds_1c );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "MEM" );
     }
/*
 * apply non-Linearity correction (chan 6-8)
 */
     if ( (calib_flag & DO_CORR_IR_NLIN) != UINT_ZERO ) {
	  SCIA_SRON_CAL_NLIN( num_mds, mds_1c );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "NLIN" );
     }
/*
 * apply Dark current correction
 */

/*
 * apply Bad Pixel Mask
 */
     if ( (calib_flag & DO_MASK_BDPM) != UINT_ZERO ) {
	  SCIA_LV1C_FLAG_BDPM( absOrbit, num_mds, mds_1c );
	  if ( IS_ERR_STAT_FATAL )
               NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "BDPM" );
     }
/*
 * apply correction for coadding
 */
     if ( (calib_flag & (DO_CORR_COADDF|DO_CORR_NORM)) != UINT_ZERO )
	  SCIA_LV1C_CAL_COADD( num_mds, mds_1c );
/*
 * apply normalisation to 1 second integration time
 */
     if ( (calib_flag & DO_CORR_NORM) != UINT_ZERO )
	  SCIA_LV1C_CAL_NORM( num_mds, mds_1c );
}
