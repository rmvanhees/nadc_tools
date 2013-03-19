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

.IDENTifer   GOME_LV1_PMD_GEO
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1
.LANGUAGE    ANSI C
.PURPOSE     copy PMD records from IHR record for calibration
.INPUT/OUTPUT
  call as    GOME_LV1_PMD_GEO( write_pmd_geo, nr_pcd, indx_pcd, pcd, &pmd );
  input: 
             struct fcd_gome  fcd[]    : Fixed Calibration Data Record
	     int    nr_pcd             : number of selected PCD records
	     short  indx_pcd[]         : indices to selected PCD records
 in/output:
	     struct pcd_gome  pcd[]    : Pixel Specific Calibration Records

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      6.0   08-Jun-2009 update to product version 2, RvH
              5.0   10-Apr-2003	calculate Geolocation per PMD record, RvH
              4.1   02-Apr-2003	GET_LV1_PCD_PMD collects PMD data, RvH
              4.0   11-Nov-2001	moved to the new Error handling routines, RvH
              3.5   05-Sep-2001	compiles without HDF5 library, RvH 
              3.4   26-Jul-2001 combined PROCESS_PCD_PMD & PROCESS_SMCD_PMD
              3.3   19-Jul-2001 pass structures using pointers, RvH
              3.2   04-Nov-2000 let this module create its own group, RvH
              3.1   03-Nov-2000 found a few bugs in GET_PMD_COORDS, RvH
                                  (Thank you Olaf...)
              3.0   08-Feb-2000 read all PCD for calibration, write selected
              2.0   19-Jan-2000 added calibration of Sun/Moon PMD's, RvH
              1.0   16-Jun-1999 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <hdf5.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

#define START_AT_ZERO  ((unsigned char) 0)
#define START_AT_ONE   ((unsigned char) 1)
#define START_AT_CNTR  ((unsigned char) 2)

#define NUM_CORNER     (NUM_COORDS - 1)
#define PMD_UNDEFINED  (-999.f)

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer  Quick_LinInterPol
.PURPOSE    quick en dirty interpolation routine (no underfow checking!)
.INPUT/OUTPUT
  call as   Quick_LinInterPol( flag, Y_left, Y_right, num_Y, Y_out );
     input:
            unsigned char flag   : defines starting value (=Y_out[0])
	                             START_AT_ZERO : Y_left
				     START_AT_ONE  : Y_left + dY/num_Y
				     START_AT_CNTR : Y_left + 0.5 * dY/num_Y
            float Y_left         : start value (see flag)
	    float Y_right        : end value
	    unsigned short num_Y : number of interpolated values
    output:
            double *Y_out        : interpolated values

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void Quick_LinInterPol( unsigned char flag, float Y_left, float Y_right,
			unsigned short num_Y, /*@out@*/ double *Y_out )
{
     register unsigned short ny = 0;

     const double Y_delta = ((double) Y_right - Y_left) / num_Y;

     if ( flag == START_AT_ONE ) {
	  do {
	       *Y_out++ = (double) Y_left + (ny * Y_delta);
	  } while ( ++ny < num_Y );
     } else if ( flag == START_AT_ZERO ) {
	  
	  while ( ++ny <= num_Y ) {
	       *Y_out++ = (double) Y_left + (ny * Y_delta);
	  }
     } else { /* START_AT_CNTR */
	  const double Y_start = Y_left + Y_delta / 2;

	  do {
	       *Y_out++ = Y_start + (ny * Y_delta);
	  } while ( ++ny < num_Y );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   CALC_PMD_GEO
.PURPOSE     return corner and central coordinates of PMD pixels
.INPUT/OUTPUT
  call as    CALC_PMD_GEO( lat, lon, pmd_lat, pmd_lon );

     input:
            float lat[]     :   array with GOME pixel latitudes
	    float lon[]     :   array with GOME pixel longitudes
    output:
            float *pmd_lat  :   array with PMD pixel latitudes
            float *pmd_lon  :   array with PMD pixel longitudes
            
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void CALC_PMD_GEO( const float *lat, const float *lon,
		  /*@out@*/ float *pmd_lat, /*@out@*/ float *pmd_lon )
     /*@globals errno@*/
{
     register int ii, jj;

     bool set_lon_offs = FALSE;
     double aa, coslat, coslon, dlat, dlon, sinlat, sinlon;
     double xx_str_bottom, yy_str_bottom, zz_str_bottom;
     double xx_diff_bottom, yy_diff_bottom, zz_diff_bottom;
     double xx_str_top, yy_str_top, zz_str_top;
     double xx_diff_top, yy_diff_top, zz_diff_top;
     double xx_orig[NUM_CORNER], yy_orig[NUM_CORNER], zz_orig[NUM_CORNER];
     double xx_pmd[PMD_IN_GRID][NUM_COORDS], 
	  yy_pmd[PMD_IN_GRID][NUM_COORDS], zz_pmd[PMD_IN_GRID][NUM_COORDS];

     const double LonOffs = PI / 4.;
     const double deg2rad = DEG2RAD;
     const double Epsilon = 1.0e-6;
/*
 * transform to Earth centered coordinated
 */  
     for ( ii = NUM_COORDS-1; ii >= 0; ii-- ) {
	  if ( (lon[ii] > 85.f && lon[ii] < 95.f) 
	       || (lon[ii] > 265.f && lon[ii] < 275.f) ) {
	       set_lon_offs = TRUE;
	       break;
	  }
     }
     ii = 0;
     do {
	  coslat = cos( deg2rad * lat[ii] );
	  sinlat = sin( deg2rad * lat[ii] );
	  if ( set_lon_offs ) {
	       coslon = cos( LonOffs + deg2rad * lon[ii] );
	       sinlon = sin( LonOffs + deg2rad * lon[ii] );
	  } else {
	       coslon = cos( deg2rad * lon[ii] );
	       sinlon = sin( deg2rad * lon[ii] );
	  }
	  xx_orig[ii] = coslat * coslon;
	  yy_orig[ii] = coslat * sinlon;
	  zz_orig[ii] = sinlat;
     } while ( ++ii < NUM_CORNER );
/*
 * calculate corners of PMD gridboxes
 */
     xx_str_bottom  = xx_orig[0];
     xx_diff_bottom = (xx_orig[2] - xx_orig[0]) / PMD_IN_GRID;
     xx_str_top     = xx_orig[1];
     xx_diff_top    = (xx_orig[3] - xx_orig[1]) / PMD_IN_GRID;
     yy_str_bottom  = yy_orig[0];
     yy_diff_bottom = (yy_orig[2] - yy_orig[0]) / PMD_IN_GRID;
     yy_str_top     = yy_orig[1];
     yy_diff_top    = (yy_orig[3] - yy_orig[1]) / PMD_IN_GRID;
     zz_str_bottom  = zz_orig[0];
     zz_diff_bottom = (zz_orig[2] - zz_orig[0]) / PMD_IN_GRID;
     zz_str_top     = zz_orig[1];
     zz_diff_top    = (zz_orig[3] - zz_orig[1]) / PMD_IN_GRID;

     ii = 0;
     do {
	  xx_pmd[ii][0] = xx_str_bottom;
	  xx_pmd[ii][1] = xx_str_top;
	  xx_pmd[ii][2] = (xx_str_bottom += xx_diff_bottom);
	  xx_pmd[ii][3] = (xx_str_top += xx_diff_top);
	  xx_pmd[ii][4] = (xx_pmd[ii][0] + xx_pmd[ii][1] 
			   + xx_pmd[ii][2] + xx_pmd[ii][3]) / NUM_CORNER;

	  yy_pmd[ii][0] = yy_str_bottom;
	  yy_pmd[ii][1] = yy_str_top;
	  yy_pmd[ii][2] = (yy_str_bottom += yy_diff_bottom);
	  yy_pmd[ii][3] = (yy_str_top += yy_diff_top);
	  yy_pmd[ii][4] = (yy_pmd[ii][0] + yy_pmd[ii][1] 
			   + yy_pmd[ii][2] + yy_pmd[ii][3]) / NUM_CORNER;

	  zz_pmd[ii][0] = zz_str_bottom;
	  zz_pmd[ii][1] = zz_str_top;
	  zz_pmd[ii][2] = (zz_str_bottom += zz_diff_bottom);
	  zz_pmd[ii][3] = (zz_str_top += zz_diff_top);
	  zz_pmd[ii][4] = (zz_pmd[ii][0] + zz_pmd[ii][1] 
			   + zz_pmd[ii][2] + zz_pmd[ii][3]) / NUM_CORNER;
     } while ( ++ii < PMD_IN_GRID );
/*
 * transform back to (lat,lon) coordinates
 */
     ii = 0;
     do {
	  jj = 0;
	  do {
	       aa = xx_pmd[ii][jj] * xx_pmd[ii][jj] 
		    + yy_pmd[ii][jj] * yy_pmd[ii][jj];
	       dlat = atan( zz_pmd[ii][jj] / sqrt( aa ));

	       coslat = cos( dlat );
	       sinlon = yy_pmd[ii][jj] / coslat;
	       coslon = xx_pmd[ii][jj] / coslat;
	       if ( sinlon < -Epsilon ) {
		    if ( coslon > Epsilon )
			 dlon = asin( sinlon ) + 2 * PI;
		    else if ( coslon < -Epsilon )
			 dlon = PI - asin( sinlon );
		    else
			 dlon = 3 * PI / 2;
	       } else if ( sinlon > Epsilon ) {
		    if ( coslon > Epsilon ) 
			 dlon = asin( sinlon );
		    else if ( coslon < -Epsilon )
			 dlon = PI - asin( sinlon );
		    else
			 dlon = PI / 2;
	       } else {
		    if ( coslon > Epsilon )
			 dlon = 0.;
		    else
			 dlon = PI;
	       }
	       if ( set_lon_offs ) dlon -= LonOffs;
	       *pmd_lat++ = (float) (dlat / deg2rad);
	       *pmd_lon++ = (float) (dlon / deg2rad);
	  } while ( ++jj < NUM_COORDS );
     } while ( ++ii < PMD_IN_GRID );
}

/*+++++++++++++++++++++++++
.IDENTifer  GET_LV1_PMD_GEO
.PURPOSE    interpolation PCD geolocation data to PMD geolocation data, 
.INPUT/OUTPUT
  call as   GET_LV1_PMD_GEO( pcd_glr, pmd_glr );
     input:
            struct glr1_gome *pcd_glr
    output:
            struct glr1_gome *pmd_glr

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void GET_LV1_PMD_GEO( const struct glr1_gome *pcd_glr, 
		      /*@out@*/ struct glr1_gome *pmd_glr )
     /*@globals  errno;@*/
     /*@modifies errno, pmd_glr@*/
{
     register int nc, nr;

     float  lat[PMD_IN_GRID * NUM_COORDS], lon[PMD_IN_GRID * NUM_COORDS];
     double dbuff[PMD_IN_GRID];

     const unsigned int msecperday = 24 * 60 * 60 * 1000;
     const unsigned int pmd_toffs[] = {
	  1406, 1313, 1219, 1125, 1031, 938, 844, 
	  750, 656, 563, 469, 375, 281, 188, 94, 0 };

     nr = 0;
     do {
/* date */
	  pmd_glr[nr].utc_date = pcd_glr->utc_date;
	  if ( pcd_glr->utc_time >= pmd_toffs[nr] ) {
	       pmd_glr[nr].utc_time = pcd_glr->utc_time - pmd_toffs[nr];
	  } else {
	       pmd_glr[nr].utc_date--;
	       pmd_glr[nr].utc_time = 
		    pcd_glr->utc_time + (msecperday - pmd_toffs[nr]);
	  }
/* pixel_type */
	  pmd_glr[nr].subsetcounter = pcd_glr->subsetcounter;
/* sun_glint */
	  pmd_glr[nr].sun_glint = pcd_glr->sun_glint;
/* sat_geo_height */
	  pmd_glr[nr].sat_geo_height = pcd_glr->sat_geo_height;
/* earth_radius */
	  pmd_glr[nr].earth_radius = pcd_glr->earth_radius;
     } while( ++nr < PMD_IN_GRID );

/* sun_zen_sat_north */
     Quick_LinInterPol( START_AT_ONE, pcd_glr->sun_zen_sat_north[0],
			pcd_glr->sun_zen_sat_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_zen_sat_north[0] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_CNTR, pcd_glr->sun_zen_sat_north[0],
			pcd_glr->sun_zen_sat_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_zen_sat_north[1] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_ZERO, pcd_glr->sun_zen_sat_north[0],
			pcd_glr->sun_zen_sat_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_zen_sat_north[2] = (float) dbuff[nr];
/* sun_azim_sat_north */
     Quick_LinInterPol( START_AT_ONE, pcd_glr->sun_azim_sat_north[0],
			pcd_glr->sun_azim_sat_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_azim_sat_north[0] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_CNTR, pcd_glr->sun_azim_sat_north[0],
			pcd_glr->sun_azim_sat_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_azim_sat_north[1] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_ZERO, pcd_glr->sun_azim_sat_north[0],
			pcd_glr->sun_azim_sat_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_azim_sat_north[2] = (float) dbuff[nr];
/* los_zen_sat_north */
     Quick_LinInterPol( START_AT_ONE, pcd_glr->los_zen_sat_north[0],
			pcd_glr->los_zen_sat_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_zen_sat_north[0] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_CNTR, pcd_glr->los_zen_sat_north[0],
			pcd_glr->los_zen_sat_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_zen_sat_north[1] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_ZERO, pcd_glr->los_zen_sat_north[0],
			pcd_glr->los_zen_sat_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_zen_sat_north[2] = (float) dbuff[nr];
/* los_azim_sat_north */
     Quick_LinInterPol( START_AT_ONE, pcd_glr->los_azim_sat_north[0],
			pcd_glr->los_azim_sat_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_azim_sat_north[0] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_CNTR, pcd_glr->los_azim_sat_north[0],
			pcd_glr->los_azim_sat_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_azim_sat_north[1] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_ZERO, pcd_glr->los_azim_sat_north[0],
			pcd_glr->los_azim_sat_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_azim_sat_north[2] = (float) dbuff[nr];
/* sun_zen_sat_ers */
     Quick_LinInterPol( START_AT_ONE, pcd_glr->sun_zen_sat_ers[0],
			pcd_glr->sun_zen_sat_ers[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_zen_sat_ers[0] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_CNTR, pcd_glr->sun_zen_sat_ers[0],
			pcd_glr->sun_zen_sat_ers[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_zen_sat_ers[1] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_ZERO, pcd_glr->sun_zen_sat_ers[0],
			pcd_glr->sun_zen_sat_ers[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_zen_sat_ers[2] = (float) dbuff[nr];
/* sun_azim_sat_ers */
     Quick_LinInterPol( START_AT_ONE, pcd_glr->sun_azim_sat_ers[0],
			pcd_glr->sun_azim_sat_ers[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_azim_sat_ers[0] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_CNTR, pcd_glr->sun_azim_sat_ers[0],
			pcd_glr->sun_azim_sat_ers[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_azim_sat_ers[1] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_ZERO, pcd_glr->sun_azim_sat_ers[0],
			pcd_glr->sun_azim_sat_ers[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_azim_sat_ers[2] = (float) dbuff[nr];
/* los_zen_sat_ers */
     Quick_LinInterPol( START_AT_ONE, pcd_glr->los_zen_sat_ers[0],
			pcd_glr->los_zen_sat_ers[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_zen_sat_ers[0] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_CNTR, pcd_glr->los_zen_sat_ers[0],
			pcd_glr->los_zen_sat_ers[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_zen_sat_ers[1] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_ZERO, pcd_glr->los_zen_sat_ers[0],
			pcd_glr->los_zen_sat_ers[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_zen_sat_ers[2] = (float) dbuff[nr];
/* los_azim_sat_ers */
     Quick_LinInterPol( START_AT_ONE, pcd_glr->los_azim_sat_ers[0],
			pcd_glr->los_azim_sat_ers[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_azim_sat_ers[0] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_CNTR, pcd_glr->los_azim_sat_ers[0],
			pcd_glr->los_azim_sat_ers[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_azim_sat_ers[1] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_ZERO, pcd_glr->los_azim_sat_ers[0],
			pcd_glr->los_azim_sat_ers[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_azim_sat_ers[2] = (float) dbuff[nr];
/* sun_zen_surf_north */
     Quick_LinInterPol( START_AT_ONE, pcd_glr->sun_zen_surf_north[0],
			pcd_glr->sun_zen_surf_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_zen_surf_north[0] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_CNTR, pcd_glr->sun_zen_surf_north[0],
			pcd_glr->sun_zen_surf_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_zen_surf_north[1] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_ZERO, pcd_glr->sun_zen_surf_north[0],
			pcd_glr->sun_zen_surf_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_zen_surf_north[2] = (float) dbuff[nr];
/* sun_azim_surf_north */
     Quick_LinInterPol( START_AT_ONE, pcd_glr->sun_azim_surf_north[0],
			pcd_glr->sun_azim_surf_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_azim_surf_north[0] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_CNTR, pcd_glr->sun_azim_surf_north[0],
			pcd_glr->sun_azim_surf_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_azim_surf_north[1] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_ZERO, pcd_glr->sun_azim_surf_north[0],
			pcd_glr->sun_azim_surf_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].sun_azim_surf_north[2] = (float) dbuff[nr];
/* los_zen_surf_north */
     Quick_LinInterPol( START_AT_ONE, pcd_glr->los_zen_surf_north[0],
			pcd_glr->los_zen_surf_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_zen_surf_north[0] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_CNTR, pcd_glr->los_zen_surf_north[0],
			pcd_glr->los_zen_surf_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_zen_surf_north[1] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_ZERO, pcd_glr->los_zen_surf_north[0],
			pcd_glr->los_zen_surf_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_zen_surf_north[2] = (float) dbuff[nr];
/* los_azim_surf_north */
     Quick_LinInterPol( START_AT_ONE, pcd_glr->los_azim_surf_north[0],
			pcd_glr->los_azim_surf_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_azim_surf_north[0] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_CNTR, pcd_glr->los_azim_surf_north[0],
			pcd_glr->los_azim_surf_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_azim_surf_north[1] = (float) dbuff[nr];
     Quick_LinInterPol( START_AT_ZERO, pcd_glr->los_azim_surf_north[0],
			pcd_glr->los_azim_surf_north[2], PMD_IN_GRID, dbuff );
     for ( nr = 0; nr < PMD_IN_GRID; nr++ )
	  pmd_glr[nr].los_azim_surf_north[2] = (float) dbuff[nr];
/* lon & lat */
     CALC_PMD_GEO( pcd_glr->lat, pcd_glr->lon, lat, lon );
     for ( nc = 0, nr = 0; nr < PMD_IN_GRID; nr++, nc += NUM_COORDS ) {
	  (void) memcpy( pmd_glr[nr].lat, lat+nc, NUM_COORDS * sizeof(float) );
	  (void) memcpy( pmd_glr[nr].lon, lon+nc, NUM_COORDS * sizeof(float) );
     }
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_LV1_PMD_GEO( unsigned char write_pmd_geo,
		       short nr_pcd, const short *indx_pcd, 
		       struct pcd_gome *pcd )
{
     register short  nr, pg, pn;
/*
 * calculate geolocation of each PMD read-out
 */
     if ( write_pmd_geo == PARAM_SET ) {
	  struct glr1_gome pmd_glr[PMD_IN_GRID];

	  nr = 0;
	  do {
	       GET_LV1_PMD_GEO( &pcd[indx_pcd[nr]].glr, pmd_glr );
	       pg = 0;
	       do {
		    (void) memcpy( &pcd[indx_pcd[nr]].pmd[pg].glr, 
				   &pmd_glr[pg], sizeof( struct glr1_gome ) );
	       } while ( ++pg < PMD_IN_GRID );
	  } while ( ++nr < nr_pcd );
     }
/*
 * copy PMD read-out values
 */
     nr = 0;
     do {
	  pg = 0;
	  do {
	       pn = 0;
	       do {
		    pcd[indx_pcd[nr]].pmd[pg].value[pn] = 
			 (float) pcd[indx_pcd[nr]].ihr.pmd[pn][pg];
	       } while ( ++pn < PMD_NUMBER );
	  } while ( ++pg < PMD_IN_GRID );
     } while ( ++nr < nr_pcd );
}
