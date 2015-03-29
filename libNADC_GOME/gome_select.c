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

.IDENTifer   GOME_SELECT
.AUTHOR      R.M. van Hees
.KEYWORDS    Gome level 1b and 2
.LANGUAGE    ANSI C
.PURPOSE     select data according to selection criteria
.RETURNS     one if setected, else zero
.COMMENTS    holds SELECT_BAND, SELECT_PCD, SELECT_SMCD, SELECT_DDR
.ENVIRONment None
.VERSION      3.0   11-Nov-2001	moved to the new Error handling routines, RvH
              2.0   21-Aug-2001	combined various modules, RvH 
              1.0   05-Aug-1999 translated from GOME_lv1/select_pcd.c, RvH
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include "eval_poly.inc"

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   BandChannel
.PURPOSE     returns GOME spectral band for given channel, subchannel
.INPUT/OUTPUT
  call as    nband = BandChannel( channel, short subchannel );

     input:
             short channel    : GOME channel [1,2,3,4]
	     short subchannel : GOME sub-channel [1=a,2=b]
            
.RETURNS     GOME spectral band index (starts at zero)
.COMMENTS    none
-------------------------*/
inline 
short BandChannel( short channel, short subchannel )
{
     const short Band2Channel[SCIENCE_CHANNELS][NUM_BAND_IN_CHANNEL] = {
	  { BAND_1a, BAND_1b },
	  { BAND_2a, BAND_2b },
	  { BAND_3, -1 },
	  { BAND_4, -1 }
     };

     if ( channel < SCIENCE_CHANNELS && subchannel < NUM_BAND_IN_CHANNEL )
	  return Band2Channel[channel][subchannel];
     else
	  return -1;
}

/*+++++++++++++++++++++++++
.IDENTifer   SELECT_BAND 
.PURPOSE     check(s) if a spectral band is selected
.INPUT/OUTPUT
  call as    if ( SELECT_BAND( nband, param, fcd ) ) ...

     input:
             short nband               : number of spectral band [1a=0,1b,2a..]
             struct param_record param : command-line parameters
	     struct fcd_gome   *fcd  : Fixed Calibration Record
            
.RETURNS     true, is selected
.COMMENTS    none
-------------------------*/
int SELECT_BAND( short nband, struct param_record param, 
		 const struct fcd_gome *fcd )
{
     register short ns;

     short channel = fcd->bcr[nband].array_nr - 1;

     switch ( nband ) {
     case BAND_1a:
	  if ( (param.chan_mask & BAND_ONE_A) == UCHAR_ZERO ) return FALSE;
	  break;
     case BAND_1b:
	  if ( (param.chan_mask & BAND_ONE_B) == UCHAR_ZERO ) return FALSE;
	  break;
     case BAND_2a:
	  if ( (param.chan_mask & BAND_TWO_A) == UCHAR_ZERO ) return FALSE;
	  break;
     case BAND_2b:
	  if ( (param.chan_mask & BAND_TWO_B) == UCHAR_ZERO ) return FALSE;
	  break;
     case BAND_3:
	  if ( (param.chan_mask & BAND_THREE) == UCHAR_ZERO ) return FALSE;
	  break;
     case BAND_4:
	  if ( (param.chan_mask & BAND_FOUR) == UCHAR_ZERO ) return FALSE;
	  break;
     case BLIND_1a:
	  if ( param.write_blind == PARAM_UNSET ) return FALSE;
	  break;
     case STRAY_1a:
     case STRAY_1b:
     case STRAY_2a:
	  if ( param.write_stray == PARAM_UNSET ) return FALSE;
	  break;
     default:
	  NADC_ERROR( NADC_ERR_FATAL, "unknown detector band" );
	  return FALSE;
     }
/*
 * this band is selcted by the user, check wavelength interval
 */
     if ( param.flag_wave == PARAM_SET ) {
	  register float wv, wave_mn, wave_mx;
/*
 * calculate wavelength overlap
 */
	  wave_mn = 2000.f;
	  wave_mx = 0.f;
	  for ( ns = 0; ns < fcd->nspec; ns++ ) {
	       wv = (float) Eval_Poly( fcd->bcr[nband].start, 
				       fcd->spec[ns].coeffs[channel] );
	       if ( wv < wave_mn ) wave_mn = wv;
	       wv = (float) Eval_Poly( fcd->bcr[nband].end, 
				       fcd->spec[ns].coeffs[channel]);
	       if ( wv > wave_mx ) wave_mx = wv;
	  }
	  if ( wave_mn > param.wave[1] || wave_mx < param.wave[0] )
	       return FALSE;
     }
     return TRUE;
}

/*+++++++++++++++++++++++++
.IDENTifer   SELECT_PCD
.PURPOSE     checks if a PCD record is selected
.INPUT/OUTPUT
  call as   nr_select += SELECT_PCD( param, &pcd );

     input:  
             struct param_record param : command-line parameters
	     struct pcd_gome   *pcd  : Pixel Specific Calibration Record

.RETURNS     one if pixel is selected, else zero
.COMMENTS    none
-------------------------*/
short SELECT_PCD( struct param_record param, const struct pcd_gome *pcd )
{
     register short corner;

     double  bgn_jdate, end_jdate, utc_jdate;

     unsigned int bgn_date, end_date;
     unsigned int bgn_time, end_time;

     const double Msec2DecimalDay = 1000 * 24. * 60. * 60.;
/*
 * select GOME pixels according to the given geographical range
 */
     if ( param.flag_geoloc == PARAM_SET ) {
	  register bool within_region = FALSE;

	  corner = 0;
	  if ( param.flag_geomnmx == PARAM_SET ) {
/*
 * at least one corner has to be within latitude/longitude range
 */
	       do {
		    if ( (pcd->glr.lat[corner] >= param.geo_lat[0]
			  && pcd->glr.lat[corner] <= param.geo_lat[1])
			 && (pcd->glr.lon[corner] >= param.geo_lon[0]
			     && pcd->glr.lon[corner] <= param.geo_lon[1]) ) {
			 within_region = TRUE;
			 break;
		    }
	       } while ( ++corner < NUM_COORDS );
	  } else {
/*
 * at least one corner has to be within the latitude range,
 *                         and outside the longitude range
 */
	       do {
		    if ( (pcd->glr.lat[corner] >= param.geo_lat[0]
			  && pcd->glr.lat[corner] <= param.geo_lat[1])
			 && (pcd->glr.lon[corner] <= param.geo_lon[0]
			     || pcd->glr.lon[corner] >= param.geo_lon[1]) ) {
			 within_region = TRUE;
			 break;
		    }
	       } while ( ++corner < NUM_COORDS );
	  }
/*
 * not one corner within the region... EXIT
 */
	  if ( within_region ) return (short) 0;    /* NOT Selected */
     }
/*
 * select specific ground pixels
 */
     if ( param.flag_subset == PARAM_SET ) {
	  switch ( pcd->ihr.subsetcounter ) {
	  case 0:
	       if ( (param.write_subset & SUBSET_EAST) == UCHAR_ZERO )
		    return (short) 0;
	       break;
	  case 1:
	       if ( (param.write_subset & SUBSET_CENTER) == UCHAR_ZERO )
		    return (short) 0;
	       break;
	  case 2:
	       if ( (param.write_subset & SUBSET_WEST) == UCHAR_ZERO )
		    return (short) 0;
	       break;
	  case 3:
	       if ( (param.write_subset & SUBSET_BACK) == UCHAR_ZERO )
		    return (short) 0;
	       break;
	  default:
	       NADC_ERROR( NADC_ERR_FATAL, "Invalid subsetcounter" );
	       return (short) 0;
	  }
     }
/*
 * select PCD records according to the given time window
 */
     if ( param.flag_period == PARAM_SET ) {
	  ASCII_2_UTC( param.bgn_date, &bgn_date, &bgn_time );
	  bgn_jdate = (double) bgn_date + bgn_time / Msec2DecimalDay;
	  ASCII_2_UTC( param.end_date, &end_date, &end_time );
	  end_jdate = (double) end_date + end_time / Msec2DecimalDay;

	  utc_jdate = (double) pcd->glr.utc_date 
	       + pcd->glr.utc_time / Msec2DecimalDay;
	  if ( (bgn_jdate - utc_jdate) >= FLT_EPSILON 
	       || (utc_jdate - end_jdate) >= FLT_EPSILON )
	       return (short) 0;                            /* NOT Selected */
     }
/*
 * select PCD records according to the given Solar zenith angle range
 */
     if ( param.flag_sunz == PARAM_SET )
	  (void) fprintf( stderr, "Sorry, not implemented yet...\n" );
/*
 * select PCD records according to the given cloud cover range
 */
     if ( param.flag_cloud == PARAM_SET )
	  (void) fprintf( stderr, "Sorry, only GOME level 2...\n" );

     return (short) 1;                                 /* Select this pixel */
}

/*+++++++++++++++++++++++++
.IDENTifer   SELECT_SMCD
.PURPOSE     check(s) if a SMCD is selected
.INPUT/OUTPUT
  call as   nr_select += SELECT_SMCD( param, &smcd );

     input:  
             struct param_record param :  command-line parameters
	     struct smcd_gome  *smcd :  Sun/Moon Specific Calibration Record

.RETURNS     not selected: zero, selected one
.COMMENTS    none
-------------------------*/
short SELECT_SMCD( struct param_record param, const struct smcd_gome *smcd )
{
     double  bgn_jdate, end_jdate, utc_jdate;

     unsigned int bgn_date, end_date;
     unsigned int bgn_time, end_time;

     const double Msec2DecimalDay = 1000 * 24. * 60. * 60.;
/*
 * select GOME pixels according to the given time window
 */
     if ( param.flag_period == PARAM_SET ) {
	  ASCII_2_UTC( param.bgn_date, &bgn_date, &bgn_time );
	  bgn_jdate = (double) bgn_date + bgn_time / Msec2DecimalDay;
	  ASCII_2_UTC( param.end_date, &end_date, &end_time );
	  end_jdate = (double) end_date + end_time / Msec2DecimalDay;

	  utc_jdate = (double) 
	       smcd->utc_date + smcd->utc_time / Msec2DecimalDay;
	  if ( (bgn_jdate - utc_jdate) >= FLT_EPSILON 
	       || (utc_jdate - end_jdate) >= FLT_EPSILON )
	       return (short) 0;                            /* NOT Selected */
     }
     return (short) 1;                                 /* Select this pixel */
}

/*+++++++++++++++++++++++++
.IDENTifer   SELECT_DDR
.PURPOSE     check(s) if a DDR is selected
.INPUT/OUTPUT
  call as   nr_select += SELECT_DDR( param, glr );

     input:  
             struct param_record param : struct holding user-defined settings
	     struct glr2_gome *glr     : DOAS Data Record (geolocation part)

.RETURNS     one if setected, else zero
.COMMENTS    none
-------------------------*/
short SELECT_DDR( struct param_record param, const struct glr2_gome *glr )
{
     register short corner;

     double  bgn_jdate, end_jdate, utc_jdate;

     unsigned int bgn_date, end_date;
     unsigned int bgn_time, end_time;

     const double Msec2DecimalDay = 1000 * 24. * 60. * 60.;
/*
 * reject GOME pixels with ground pixel numer less than one
 */
     if ( glr->pixel_nr == 0 ) return (short) 0;   /* NOT Selected */
/*
 * select GOME pixels according to the given geographical range
 */
     if ( param.flag_geoloc == PARAM_SET ) {
	  register bool within_region = FALSE;

	  corner = 0;
	  if ( param.flag_geomnmx == PARAM_SET ) {
/*
 * at least one corner has to be within latitude/longitude range
 */
	       do {
		    if ( (glr->lat[corner] >= param.geo_lat[0]
			  && glr->lat[corner] <= param.geo_lat[1])
			 && (glr->lon[corner] >= param.geo_lon[0]
			     && glr->lon[corner] <= param.geo_lon[1]) ) {
			 within_region = TRUE;
			 break;
		    }
	       } while ( ++corner < (short) (NUM_COORDS-1) );
	  } else {
/*
 * at least one corner has to be within the latitude range,
 *                         and outside the longitude range
 */
	       do {
		    if ( (glr->lat[corner] >= param.geo_lat[0]
			  && glr->lat[corner] <= param.geo_lat[1])
			 && (glr->lon[corner] <= param.geo_lon[0]
			     || glr->lon[corner] >= param.geo_lon[1]) ) {
			 within_region = TRUE;
			 break;
		    }
	       } while ( ++corner < (short) (NUM_COORDS-1) );
	  }
/*
 * not one corner within the region... EXIT
 */
	  if ( within_region ) return (short) 0;   /* NOT Selected */
     }
/*
 * select specific ground pixels
 */
     if ( param.flag_subset == PARAM_SET ) {
	  switch ( glr->subsetcounter ) {
	  case 0:
	       if ( (param.write_subset & SUBSET_EAST) == UCHAR_ZERO )
		    return (short) 0;
	       break;
	  case 1:
	       if ( (param.write_subset & SUBSET_CENTER) == UCHAR_ZERO )
		    return (short) 0;
	       break;
	  case 2:
	       if ( (param.write_subset & SUBSET_WEST) == UCHAR_ZERO )
		    return (short) 0;
	       break;
	  case 3:
	       if ( (param.write_subset & SUBSET_BACK) == UCHAR_ZERO )
		    return (short) 0;
	       break;
	  default:
	       NADC_ERROR( NADC_ERR_FATAL, "Invalid subsetcounter" );
	       return (short) 0;
	  }
     }
/*
 * select DDR records according to the given time window
 */
     if ( param.flag_period == PARAM_SET ) {
	  ASCII_2_UTC( param.bgn_date, &bgn_date, &bgn_time );
	  bgn_jdate = (double) bgn_date + bgn_time / Msec2DecimalDay;
#ifdef DEBUG
	  (void) printf( "%s: %d %u %12.8f\n", param.bgn_date, bgn_date, 
			 bgn_time, bgn_jdate );
#endif
	  ASCII_2_UTC( param.end_date, &end_date, &end_time );
	  end_jdate = (double) end_date + end_time / Msec2DecimalDay;
#ifdef DEBUG
	  (void) printf( "%s: %u %u %12.8f\n", param.end_date, end_date, 
			 end_time, end_jdate );
#endif
	  utc_jdate = (double) glr->utc_date 
	       + glr->utc_time / Msec2DecimalDay;
#ifdef DEBUG
	  (void) printf( "%u %u %12.8f\n", glr->utc_date, 
			 glr->utc_time, utc_jdate );
#endif
	  if ( (bgn_jdate - utc_jdate) >= FLT_EPSILON 
	       || (utc_jdate - end_jdate) >= FLT_EPSILON )
	       return (short) 0;                            /* NOT Selected */
     }
/*
 * select DDR records according to the given Solar zenith angle range
 */
     if ( param.flag_sunz == PARAM_SET ) {
	  register bool within_range = FALSE;

	  for ( corner = 0; corner < 3; corner++ ) {
	       if ( param.sunz[0] <= glr->toa_zenith[corner]
		    && param.sunz[1] >= glr->toa_zenith[corner] ) {
		    within_range = TRUE;
		    break;
	       }
	  }
	  if ( within_range ) return (short) 0;    /* NOT Selected */
     }
     return (short) 1;                                 /* Select this pixel */
}
