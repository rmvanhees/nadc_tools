/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2002 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GET_SCIA_LV1C_GEO
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 1c - geolocation
.LANGUAGE    ANSI C
.PURPOSE     convert/copy gelocation records level 1b to level 1c format
.INPUT/OUTPUT
  call as    GET_SCIA_LV1C_GEON( nr_geo, geoN, nr_geo1c, geoN_1c );

     input:
             unsigned int nr_geo       : number of level 1b geo-records
	     struct geoN_scia *geoN    : Nadir gelocation records (level 1b)
	     unsigned int nr_geo1c     : number of level 1c geo-records
    output:
             struct geoN_scia *geoN_1c : Nadir gelocation records (level 1c)

  call as    GET_SCIA_LV1C_GEOL( nr_geo, geoL, nr_geo1c, geoL_1c )

     input:
             unsigned int nr_geo       : number of level 1b geo-records
	     struct geoL_scia *geoL    : Limb gelocation records (level 1b)
	     unsigned int nr_geo1c     : number of level 1c geo-records
    output:
             struct geoL_scia *geoL_1c : Limb gelocation records (level 1c)

  call as    GET_SCIA_LV1C_GEOC( nr_geo, geoC, nr_geo1c, geoC_1c )

     input:
             unsigned int nr_geo       : number of level 1b geo-records
	     struct geoC_scia *geoC    : Monitor gelocation records (level 1b)
	     unsigned int nr_geo1c     : number of level 1c geo-records
    output:
             struct geoC_scia *geoC_1c : Monitor gelocation records (level 1c)

.RETURNS     nothing
.COMMENTS    None
.ENVIRONment None
.VERSION     1.6     07-Dec-2005   use modf (floating point exceptions), RvH
             1.5     01-Sep-2004   more minor clean-ups and speed-ups, RvH
             1.4     03-Aug-2004   minor bug-fixes and speed-ups, RvH
             1.3     16-Mar-2004   minor bug-fixes, RvH
             1.2     15-Mar-2004   fixed some ToDo interpolation, RvH
             1.1     08-Apr-2003   applied some optimalisations, RvH
             1.0     07-Nov-2002   Created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++ Macros +++++*/
#define START_AT_ZERO  ((unsigned char) 0)
#define START_AT_ONE   ((unsigned char) 1)
#define START_AT_CNTR  ((unsigned char) 2)

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
static const double LonOffs = PI / 4.;
static const double deg2rad = DEG2RAD;

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

     *Y_out = (double) Y_left;
     if ( flag == START_AT_CNTR )
	  *Y_out += Y_delta / 2;
     else if ( flag == START_AT_ONE )
	  *Y_out += Y_delta;

     while ( ++ny < num_Y ) Y_out[ny] = Y_out[ny-1] + Y_delta;
}

/*+++++++++++++++++++++++++
.IDENTifer  Quick_LinInterPol_d
.PURPOSE    quick en dirty interpolation routine (no underfow checking!)
.INPUT/OUTPUT
  call as   Quick_LinInterPol_d( flag, Y_left, Y_right, num_Y, Y_out );
     input:
            unsigned char flag   : defines starting value (=Y_out[0])
	                             START_AT_ZERO : Y_left
				     START_AT_ONE  : Y_left + dY/num_Y
				     START_AT_CNTR : Y_left + 0.5 * dY/num_Y
            double Y_left        : start value (see flag)
	    double Y_right       : end value
	    unsigned short num_Y : number of interpolated values
    output:
            double *Y_out        : interpolated values

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void Quick_LinInterPol_d( unsigned char flag, double Y_left, double Y_right,
			  unsigned short num_Y, /*@out@*/ double *Y_out )
{
     register unsigned short ny = 0;

     const double Y_delta = (Y_right - Y_left) / num_Y;

     if ( flag == START_AT_ZERO )
	  *Y_out = Y_left;
     else if ( flag == START_AT_ONE )
	  *Y_out = Y_left + Y_delta;
     else /* START_AT_CNTR */
	  *Y_out = Y_left + Y_delta / 2;

     while ( ++ny < num_Y ) Y_out[ny] = Y_out[ny-1] + Y_delta;
}

/*+++++++++++++++++++++++++
.IDENTifer  Cntr_LinInterPol
.PURPOSE    quick en dirty interpolation routine (no underfow checking!)
.INPUT/OUTPUT
  call as   Cntr_LinInterPol_d( Y_left, Y_cntr, Y_right, num_Y, Y_out );
     input:
            float Y_left         : Y-value on the left (or NAN)
            float Y_cntr         : Y-value in the center
            float Y_righ t       : Y-right on the left (or NAN)
	    unsigned short num_Y : dimension of the output array
    output:
            double *Y_out        : interpolated values

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void Cntr_LinInterPol( float Y_left, float Y_cntr, float Y_right, 
		       unsigned short num_Y, /*@out@*/ double *Y_out )
{
     register unsigned short ny;

     double Y_start, dY_left, dY_right;

     const unsigned short half_Y = (unsigned short) (num_Y / 2);

     if ( isnan( Y_left ) ) {
	  dY_left = dY_right = ((double) Y_right - Y_cntr) / num_Y;
     } else if ( isnan( Y_right ) ) {
	  dY_right = dY_left = ((double) Y_cntr - Y_left) / num_Y;
     } else {
	  dY_left = ((double) Y_right - Y_cntr) / num_Y;
	  dY_right = ((double) Y_cntr - Y_left) / num_Y;
     }

     if ( num_Y % 2 == 1 ) {
	  Y_start = Y_cntr - half_Y * dY_left;
	  ny = 0;
	  do {
	       *Y_out++ = Y_start + ny * dY_left;
	  } while ( ++ny < half_Y );
	  *Y_out++ = Y_cntr;
	  ny = 0;
	  do {
	       *Y_out++ = Y_cntr + (ny+1) * dY_right;
	  } while ( ++ny < half_Y );
     } else {
	  Y_start = Y_cntr - half_Y * dY_left + (dY_left / 2);
	  ny = 0;
	  do {
	       *Y_out++ = Y_start + ny * dY_left;
	  } while ( ++ny < half_Y );

	  Y_start = Y_cntr + dY_right / 2;
	  ny = 0;
	  do {
	       *Y_out++ = Y_start + ny * dY_right;
	  } while ( ++ny < half_Y );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer  Cntr_LinInterPol_d
.PURPOSE    quick en dirty interpolation routine (no underfow checking!)
.INPUT/OUTPUT
  call as   Cntr_LinInterPol_d( Y_left, Y_cntr, Y_right, num_Y, Y_out );
     input:
            double Y_left        : Y-value on the left (or NAN)
            double Y_cntr        : Y-value in the center
            double Y_right       : Y-right on the left (or NAN)
	    unsigned short num_Y : dimension of the output array
    output:
            double *Y_out        : interpolated values

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void Cntr_LinInterPol_d( double Y_left, double Y_cntr, double Y_right, 
			 unsigned short num_Y, /*@out@*/ double *Y_out )
{
     register unsigned short ny;

     double Y_start, dY_left, dY_right;

     const unsigned short half_Y = (unsigned short) (num_Y / 2);

     if ( isnan( Y_left ) ) {
	  dY_left = dY_right = (Y_right - Y_cntr) / num_Y;
     } else if ( isnan( Y_right ) ) {
	  dY_right = dY_left = (Y_cntr - Y_left) / num_Y;
     } else {
	  dY_left = (Y_right - Y_cntr) / num_Y;
	  dY_right = (Y_cntr - Y_left) / num_Y;
     }

     if ( num_Y % 2 == 1 ) {
	  Y_start = Y_cntr - half_Y * dY_left;
	  ny = 0;
	  do {
	       *Y_out++ = Y_start + ny * dY_left;
	  } while ( ++ny < half_Y );
	  *Y_out++ = Y_cntr;
	  ny = 0;
	  do {
	       *Y_out++ = Y_cntr + (ny+1) * dY_right;
	  } while ( ++ny < half_Y );
     } else {
	  Y_start = Y_cntr - half_Y * dY_left + (dY_left / 2);
	  ny = 0;
	  do {
	       *Y_out++ = Y_start + ny * dY_left;
	  } while ( ++ny < half_Y );

	  Y_start = Y_cntr + dY_right / 2;
	  ny = 0;
	  do {
	       *Y_out++ = Y_start + ny * dY_right;
	  } while ( ++ny < half_Y );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer  Coord2XYZ
.PURPOSE    transform (lat,lon) coordinates to (x,y,z) coordinated 
.INPUT/OUTPUT
  call as   Coord2XYZ( DoLonOffs, coord, xx, yy, zz );

     input:
            int  DoLonOffs : transpose longitutes to omit roundoff errors
            struct coord_envi coord : lat/lon coordinates
    output:
            double xx               :   x-coordinates
            double yy               :   y-coordinates
            double zz               :   z-coordinates

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void Coord2XYZ( int DoLonOffs, const struct coord_envi coord, 
		/*@out@*/ double *xx, /*@out@*/ double *yy, 
		/*@out@*/ double *zz )
     /*@globals  errno@*/
{
     double coslat, sinlat, coslon, sinlon;

     const double mudeg2rad = deg2rad / 1e6;

/* do transformation */
     coslat = cos( mudeg2rad * coord.lat );
     sinlat = sin( mudeg2rad * coord.lat );
     if ( DoLonOffs ) {
	  coslon = cos( LonOffs + mudeg2rad * coord.lon );
	  sinlon = sin( LonOffs + mudeg2rad * coord.lon );
     } else {
	  coslon = cos( mudeg2rad * coord.lon );
	  sinlon = sin( mudeg2rad * coord.lon );
     }
     *xx = coslat * coslon;
     *yy = coslat * sinlon;
     *zz = sinlat;
}

/*+++++++++++++++++++++++++
.IDENTifer  XYZ2Coord
.PURPOSE    transform (x,y,z) coordinates to (lat,lon) coordinated 
.INPUT/OUTPUT
  call as   XYZ2Coord( DoLonOffs, xx, yy, zz, coord );

     input:
            int  DoLonOffs : transpose longitutes to omit roundoff errors
            double xx      : x-coordinates
            double yy      : y-coordinates
            double zz      : z-coordinates
     output:
            struct coord_envi *coord : lat/lon coordinates

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void XYZ2Coord( int DoLonOffs, double xx, double yy, double zz,
		/*@out@*/ struct coord_envi *coord ) 
     /*@globals  errno@*/
{
     double dintegral;

     const double mudeg2rad = deg2rad / 1e6;

     const double dlat = 
	  atan( zz / sqrt( xx * xx + yy * yy ));
     const double dlon = 
	  (DoLonOffs) ? atan2( yy, xx ) - LonOffs : atan2( yy, xx );

     (void) modf( (dlat / mudeg2rad), &dintegral );
     coord->lat = (int) dintegral;
     (void) modf( (dlon / mudeg2rad), &dintegral );
     coord->lon = (int) dintegral;
}

/*+++++++++++++++++++++++++
.IDENTifer  GET_CNTR_COORD 
.PURPOSE    calculate central coordinate (lat,lon) between 2 positions 
.INPUT/OUTPUT
  call as   GET_CNTR_COORD( coord_A, coord_B, center );

     input:
            struct coord_envi coord_A : geographical location A (ISO 6709)
            struct coord_envi coord_B : geographical location B (ISO 6709)
    output:
            struct coord_envi *center : geographical location between A and B

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void GET_CNTR_COORD( struct coord_envi coord_A,
		     struct coord_envi coord_B,
		     /*@out@*/ struct coord_envi *center )
     /*@globals  errno@*/
{
     bool   do_lon_offs = FALSE;

     double xx_A, yy_A, zz_A, xx_B, yy_B, zz_B;
     double xx_cntr, yy_cntr, zz_cntr;
/*
 * do we need to shift the transformation for the longitude calculation?
 */
     if ( (coord_A.lon > -95000000 && coord_A.lon < -85000000) 
	  || (coord_A.lon > 85000000 && coord_A.lon < 95000000) )
          do_lon_offs = TRUE;
     else if ( (coord_B.lon > -95000000 && coord_B.lon < -85000000) 
	       || (coord_B.lon > 85000000 && coord_B.lon < 95000000) )
          do_lon_offs = TRUE;
/*
 * transform to Earth centered coordinated
 */
     Coord2XYZ( do_lon_offs, coord_A, &xx_A, &yy_A, &zz_A );
     Coord2XYZ( do_lon_offs, coord_B, &xx_B, &yy_B, &zz_B );

     xx_cntr = (xx_A + xx_B) / 2;
     yy_cntr = (yy_A + yy_B) / 2;
     zz_cntr = (zz_A + zz_B) / 2;
/*
 * transform back to (lat,lon) coordinates
 */
     XYZ2Coord( do_lon_offs, xx_cntr, yy_cntr, zz_cntr, center );
}

/*+++++++++++++++++++++++++
.IDENTifer  GET_CNTR_CORNER 
.PURPOSE    calculate central coordinate (lat,lon) from a corner_coord
.INPUT/OUTPUT
  call as   GET_CNTR_CORNER( corner, center );

     input:
            struct coord_envi corner[4] : geographical location A (ISO 6709)
    output:
            struct coord_envi *center : geographical location between A and B

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void GET_CNTR_CORNER( struct coord_envi corner[],
		     /*@out@*/ struct coord_envi *center )
     /*@globals  errno@*/
{
     struct coord_envi center_A, center_B;

     GET_CNTR_COORD( corner[0], corner[1], &center_A );
     GET_CNTR_COORD( corner[2], corner[3], &center_B );
     GET_CNTR_COORD( center_A, center_B, center );
}


/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GET_SCIA_LV1C_GEON( unsigned int nr_geo, const struct geoN_scia *geoN,
			 unsigned int nr_geo1c, struct geoN_scia *geoN_1c )
{
     const char prognm[] = "GET_SCIA_LV1C_GEON";

     register unsigned int   nr;
     register unsigned short ns;

     unsigned short icntr;

     if ( nr_geo == nr_geo1c ) {
	  (void) memcpy( geoN_1c, geoN, nr_geo * sizeof( struct geoN_scia ));
     } else if ( nr_geo > nr_geo1c ) {
	  const unsigned short nscale = (unsigned short)(nr_geo / nr_geo1c);

	  for ( nr = 0; nr < nr_geo1c; nr++ ) {
	       bool homo_pixel_type = TRUE;
	       
	       geoN_1c->glint_flag = UCHAR_ZERO;
	       geoN_1c->pos_esm = 0.f;
	       geoN_1c->sat_h = 0.f;
	       geoN_1c->earth_rad = 0.f;
	       for ( ns = 0; ns < nscale; ns++ ) {
		    if ( geoN[0].pixel_type != geoN[ns].pixel_type )
			 homo_pixel_type = FALSE;
		    geoN_1c->glint_flag = 
			 (geoN_1c->glint_flag | geoN[ns].glint_flag);
		    geoN_1c->pos_esm += geoN[ns].pos_esm;
		    geoN_1c->sat_h += geoN[ns].sat_h;
		    geoN_1c->earth_rad += geoN[ns].earth_rad;
	       }
	       geoN_1c->pos_esm /= nscale;
	       geoN_1c->sat_h /= nscale;
	       geoN_1c->earth_rad /= nscale;

	       if ( homo_pixel_type ) {
		    icntr = (unsigned short)(nscale / 2);
/* pixel type */
		    geoN_1c->pixel_type = geoN[0].pixel_type;
/* viewing geometry */
		    geoN_1c->sun_zen_ang[0] = geoN[0].sun_zen_ang[0];
		    geoN_1c->sun_azi_ang[0] = geoN[0].sun_azi_ang[0];
		    geoN_1c->los_zen_ang[0] = geoN[0].los_zen_ang[0];
		    geoN_1c->los_azi_ang[0] = geoN[0].los_azi_ang[0];
		    geoN_1c->sun_zen_ang[1] = geoN[icntr].sun_zen_ang[0];
		    geoN_1c->sun_azi_ang[1] = geoN[icntr].sun_azi_ang[0];
		    geoN_1c->los_zen_ang[1] = geoN[icntr].los_zen_ang[0];
		    geoN_1c->los_azi_ang[1] = geoN[icntr].los_azi_ang[0];
		    geoN_1c->sun_zen_ang[2] = geoN[nscale-1].sun_zen_ang[2];
		    geoN_1c->sun_azi_ang[2] = geoN[nscale-1].sun_azi_ang[2];
		    geoN_1c->los_zen_ang[2] = geoN[nscale-1].los_zen_ang[2];
		    geoN_1c->los_azi_ang[2] = geoN[nscale-1].los_azi_ang[2];
/* corner coords */
		    (void) memcpy( &geoN_1c->corner[0], &geoN->corner[0],
				   sizeof(struct coord_envi));
		    (void) memcpy( &geoN_1c->corner[1], &geoN->corner[1],
				   sizeof(struct coord_envi));
		    (void) memcpy( &geoN_1c->corner[2], 
				   &geoN[nscale-1].corner[2],
				   sizeof(struct coord_envi));
		    (void) memcpy( &geoN_1c->corner[3],
				   &geoN[nscale-1].corner[3],
				   sizeof(struct coord_envi));
/* center coordinate */
		    GET_CNTR_CORNER( geoN_1c->corner, &geoN_1c->center );
/* sub-satellite point */
		    if ( nscale % 2 == 0 ) {   /* even */
			 GET_CNTR_COORD( geoN[icntr-1].sub_sat_point, 
					 geoN[icntr].sub_sat_point,
					 &geoN_1c->sub_sat_point );
		    } else {                   /* odd */
			 (void) memcpy( &geoN_1c->sub_sat_point,
					&geoN[icntr].sub_sat_point,
					sizeof(struct coord_envi));
		    }
	       } else { /* ! home_pixel_type */
		    register unsigned short last_fscan = nscale;

		    while ( --last_fscan > 0 )
			 if ( (int) geoN[last_fscan].pixel_type == 1 ) break;
		    icntr = (unsigned short)(last_fscan / 2);
/* pixel type */
		    geoN_1c->pixel_type = (unsigned char) NO_PIXEL_TYPE;
/* viewing geometry */
		    geoN_1c->sun_zen_ang[0] = geoN[0].sun_zen_ang[0];
		    geoN_1c->sun_azi_ang[0] = geoN[0].sun_azi_ang[0];
		    geoN_1c->los_zen_ang[0] = geoN[0].los_zen_ang[0];
		    geoN_1c->los_azi_ang[0] = geoN[0].los_azi_ang[0];
		    geoN_1c->sun_zen_ang[1] = 0.f;
		    geoN_1c->sun_azi_ang[1] = 0.f;
		    geoN_1c->los_zen_ang[1] = 0.f;
		    geoN_1c->los_azi_ang[1] = 0.f;
		    for ( ns = 0; ns < nscale; ns++ ) {
			 geoN_1c->sun_zen_ang[1] += geoN[ns].sun_zen_ang[0];
			 geoN_1c->sun_azi_ang[1] += geoN[ns].sun_azi_ang[0];
			 geoN_1c->los_zen_ang[1] += geoN[ns].los_zen_ang[0];
			 geoN_1c->los_azi_ang[1] += geoN[ns].los_azi_ang[0];
		    }
		    geoN_1c->sun_zen_ang[1] /= nscale;
		    geoN_1c->sun_azi_ang[1] /= nscale;
		    geoN_1c->los_zen_ang[1] /= nscale;
		    geoN_1c->los_azi_ang[1] /= nscale;
		    /* BUG: backscan in neglected */
		    geoN_1c->sun_zen_ang[2] = geoN[last_fscan].sun_zen_ang[2];
		    geoN_1c->sun_azi_ang[2] = geoN[last_fscan].sun_azi_ang[2];
		    geoN_1c->los_zen_ang[2] = geoN[last_fscan].los_zen_ang[2];
		    geoN_1c->los_azi_ang[2] = geoN[last_fscan].los_azi_ang[2];
/* Corner coords */
		    (void) memcpy( &geoN_1c->corner[0], &geoN->corner[0],
				   sizeof(struct coord_envi));
		    (void) memcpy( &geoN_1c->corner[1], &geoN->corner[1],
				   sizeof(struct coord_envi));
		    (void) memcpy( &geoN_1c->corner[2],
				   &geoN[last_fscan].corner[2],
				   sizeof(struct coord_envi));
		    (void) memcpy( &geoN_1c->corner[3],
				   &geoN[last_fscan].corner[3],
				   sizeof(struct coord_envi));
/* center coordinate */
		    /* BUG: backscan in neglected */
		    GET_CNTR_CORNER( geoN_1c->corner, &geoN_1c->center );
/* sub-satellite point */
		    /* BUG: backscan in neglected */
		    if ( nscale % 2 == 0 ) {   /* even */
			 GET_CNTR_COORD( geoN[icntr-1].sub_sat_point, 
					 geoN[icntr].sub_sat_point,
					 &geoN_1c->sub_sat_point );
		    } else {                   /* odd */
			 (void) memcpy( &geoN_1c->sub_sat_point,
					&geoN[icntr].sub_sat_point,
					sizeof(struct coord_envi));
		    }
	       }
	       geoN_1c++;
	       geoN += nscale;
	  }
     } else { /* nr_geo < nr_geo1c */
	  register bool DoLonOffs;

	  const unsigned short nscale = (unsigned short)(nr_geo1c / nr_geo);

	  double xx[4], yy[4], zz[4];
	  double *xval, *yval, *zval;

	  xval = (double *) malloc( nscale * sizeof(double) );
	  if ( xval == NULL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "xval" );
	  yval = (double *) malloc( nscale * sizeof(double) );
	  if ( yval == NULL ) {
	       free( xval );
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "yval" );
	  }
	  zval = (double *) malloc( nscale * sizeof(double) );
	  if ( zval == NULL ) {
	       free( xval ); free( yval );
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "zval" );
	  }
	  for ( nr = 0; nr < nr_geo; nr++ ) {
/* pixel_type and Sun glint/Rainbow flag */
	       for ( ns = 0; ns < nscale; ns++ ) {
		    geoN_1c[ns].pixel_type = geoN->pixel_type;
		    geoN_1c[ns].glint_flag = geoN->glint_flag;
	       }
/* pos_esm */
	       if ( nr == 0 )
		    Cntr_LinInterPol( NAN, geoN[0].pos_esm, 
				      geoN[1].pos_esm, nscale, yval );
	       else if ( nr == nr_geo-1 )
		    Cntr_LinInterPol( geoN[-1].pos_esm, geoN[0].pos_esm,
				      NAN, nscale, yval );
	       else
		    Cntr_LinInterPol( geoN[-1].pos_esm, geoN[0].pos_esm,
				      geoN[1].pos_esm, nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoN_1c[ns].pos_esm = (float) yval[ns];
/* sat_h */
	       if ( nr == 0 )
		    Cntr_LinInterPol( NAN, geoN[0].sat_h, 
				      geoN[1].sat_h, nscale, yval );
	       else if ( nr == nr_geo-1 )
		    Cntr_LinInterPol( geoN[-1].sat_h, geoN[0].sat_h,
				      NAN, nscale, yval );
	       else
		    Cntr_LinInterPol( geoN[-1].sat_h, geoN[0].sat_h,
				      geoN[1].sat_h, nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoN_1c[ns].sat_h = (float) yval[ns];
/* earth_rad */
	       if ( nr == 0 )
		    Cntr_LinInterPol( NAN, geoN[0].earth_rad, 
				      geoN[1].earth_rad, nscale, yval );
	       else if ( nr == nr_geo-1 )
		    Cntr_LinInterPol( geoN[-1].earth_rad, geoN[0].earth_rad,
				      NAN, nscale, yval );
	       else
		    Cntr_LinInterPol( geoN[-1].earth_rad, geoN[0].earth_rad,
				      geoN[1].earth_rad, nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoN_1c[ns].earth_rad = (float) yval[ns];
/* sun_zen_ang */
	       Quick_LinInterPol( START_AT_ZERO, geoN->sun_zen_ang[0],
				  geoN->sun_zen_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoN_1c[ns].sun_zen_ang[0] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_CNTR, geoN->sun_zen_ang[0],
				  geoN->sun_zen_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoN_1c[ns].sun_zen_ang[1] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_ONE, geoN->sun_zen_ang[0],
				  geoN->sun_zen_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoN_1c[ns].sun_zen_ang[2] = (float) yval[ns];
/* sun_azi_ang */
	       Quick_LinInterPol( START_AT_ZERO, geoN->sun_azi_ang[0],
				  geoN->sun_azi_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoN_1c[ns].sun_azi_ang[0] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_CNTR, geoN->sun_azi_ang[0],
				  geoN->sun_azi_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoN_1c[ns].sun_azi_ang[1] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_ONE, geoN->sun_azi_ang[0],
				  geoN->sun_azi_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoN_1c[ns].sun_azi_ang[2] = (float) yval[ns];
/* los_zen_ang */
	       Quick_LinInterPol( START_AT_ZERO, geoN->los_zen_ang[0],
				  geoN->los_zen_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoN_1c[ns].los_zen_ang[0] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_CNTR, geoN->los_zen_ang[0],
				  geoN->los_zen_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoN_1c[ns].los_zen_ang[1] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_ONE, geoN->los_zen_ang[0],
				  geoN->los_zen_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoN_1c[ns].los_zen_ang[2] = (float) yval[ns];
/* los_azi_ang */
	       Quick_LinInterPol( START_AT_ZERO, geoN->los_azi_ang[0],
				  geoN->los_azi_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoN_1c[ns].los_azi_ang[0] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_CNTR, geoN->los_azi_ang[0],
				  geoN->los_azi_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoN_1c[ns].los_azi_ang[1] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_ONE, geoN->los_azi_ang[0],
				  geoN->los_azi_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoN_1c[ns].los_azi_ang[2] = (float) yval[ns];
/* Corner coords */
	       /* do we need to transpose the longitudes? */
               /* Note pixel [1] & [3] are neglected... */
	       DoLonOffs = TRUE;
	       if ( geoN->corner[2].lon < -95000000 
		    || geoN->corner[0].lon > 95000000
		    || (geoN->corner[0].lon > -85000000
			&& geoN->corner[2].lon < 85000000) )
		    DoLonOffs = FALSE;
	       Coord2XYZ( DoLonOffs, geoN->corner[0], xx, yy, zz );
	       Coord2XYZ( DoLonOffs, geoN->corner[1], xx+1, yy+1, zz+1 );
	       Coord2XYZ( DoLonOffs, geoN->corner[2], xx+2, yy+2, zz+2 );
	       Coord2XYZ( DoLonOffs, geoN->corner[3], xx+3, yy+3, zz+3 );
/* ----- corner upper left */
	       Quick_LinInterPol_d( START_AT_ZERO, xx[1], xx[3], nscale, xval);
	       Quick_LinInterPol_d( START_AT_ZERO, yy[1], yy[3], nscale, yval);
	       Quick_LinInterPol_d( START_AT_ZERO, zz[1], zz[3], nscale, zval);
	       for ( ns = 0; ns < nscale; ns++ ) {
		    XYZ2Coord( DoLonOffs, xval[ns], yval[ns], zval[ns],
			       &geoN_1c[ns].corner[1] );
	       }
/* ----- corner upper right */
	       Quick_LinInterPol_d( START_AT_ONE, xx[1], xx[3], nscale, xval );
	       Quick_LinInterPol_d( START_AT_ONE, yy[1], yy[3], nscale, yval );
	       Quick_LinInterPol_d( START_AT_ONE, zz[1], zz[3], nscale, zval );
	       for ( ns = 0; ns < nscale; ns++ ) {
		    XYZ2Coord( DoLonOffs, xval[ns], yval[ns], zval[ns],
			       &geoN_1c[ns].corner[3] );
	       }
/* ----- corner lower left */
	       Quick_LinInterPol_d( START_AT_ZERO, xx[0], xx[2], nscale, xval);
	       Quick_LinInterPol_d( START_AT_ZERO, yy[0], yy[2], nscale, yval);
	       Quick_LinInterPol_d( START_AT_ZERO, zz[0], zz[2], nscale, zval);
	       for ( ns = 0; ns < nscale; ns++ ) {
		    XYZ2Coord( DoLonOffs, xval[ns], yval[ns], zval[ns],
			       &geoN_1c[ns].corner[0] );
	       }
/* ----- corner lower right */
	       Quick_LinInterPol_d( START_AT_ONE, xx[0], xx[2], nscale, xval );
	       Quick_LinInterPol_d( START_AT_ONE, yy[0], yy[2], nscale, yval );
	       Quick_LinInterPol_d( START_AT_ONE, zz[0], zz[2], nscale, zval );
	       for ( ns = 0; ns < nscale; ns++ ) {
		    XYZ2Coord( DoLonOffs, xval[ns], yval[ns], zval[ns],
			       &geoN_1c[ns].corner[2] );
	       }
/* center_coord */
	       for ( ns = 0; ns < nscale; ns++ ) {
		    GET_CNTR_CORNER( geoN_1c[ns].corner, &geoN_1c[ns].center );
	       }
/* sub_sat_point */
	       DoLonOffs = TRUE;
	       if ( geoN->sub_sat_point.lon < -95000000
		    || geoN->sub_sat_point.lon > 95000000
		    || (geoN->sub_sat_point.lon > -85000000
			&& geoN->sub_sat_point.lon < 85000000) )
		    DoLonOffs = FALSE;
	       if ( nr == 0 ) {
		    Coord2XYZ( DoLonOffs, geoN[0].sub_sat_point, xx, yy, zz );
		    Coord2XYZ( DoLonOffs, geoN[1].sub_sat_point, 
			       xx+1, yy+1, zz+1 );

		    Cntr_LinInterPol_d( NAN, xx[0], xx[1], nscale, xval );
		    Cntr_LinInterPol_d( NAN, yy[0], yy[1], nscale, yval );
		    Cntr_LinInterPol_d( NAN, zz[0], zz[1], nscale, zval );
	       } else if ( nr == nr_geo-1 ) {
		    Coord2XYZ( DoLonOffs, geoN[-1].sub_sat_point, 
			       xx, yy, zz );
		    Coord2XYZ( DoLonOffs, geoN[0].sub_sat_point, 
			       xx+1, yy+1, zz+1 );

		    Cntr_LinInterPol_d( xx[0], xx[1], NAN, nscale, xval );
		    Cntr_LinInterPol_d( yy[0], yy[1], NAN, nscale, yval );
		    Cntr_LinInterPol_d( zz[0], zz[1], NAN, nscale, zval );
	       } else {
		    Coord2XYZ( DoLonOffs, geoN[-1].sub_sat_point, 
			       xx, yy, zz );
		    Coord2XYZ( DoLonOffs, geoN[0].sub_sat_point, 
			       xx+1, yy+1, zz+1 );
		    Coord2XYZ( DoLonOffs, geoN[1].sub_sat_point, 
			       xx+2, yy+2, zz+2 );

		    Cntr_LinInterPol_d( xx[0], xx[1], xx[2], nscale, xval );
		    Cntr_LinInterPol_d( yy[0], yy[1], yy[2], nscale, yval );
		    Cntr_LinInterPol_d( zz[0], zz[1], zz[2], nscale, zval );
	       }
	       for ( ns = 0; ns < nscale; ns++ ) {
		    XYZ2Coord( DoLonOffs, xval[ns], yval[ns], zval[ns],
			       &geoN_1c[ns].sub_sat_point );
	       }
	       geoN++;
	       geoN_1c += nscale;	       
	  }
	  free( xval );
	  free( yval );
	  free( zval );
     }
}
		      
/*+++++++++++++++++++++++++*/
void GET_SCIA_LV1C_GEOL( unsigned int nr_geo, const struct geoL_scia *geoL,
			 unsigned int nr_geo1c, struct geoL_scia *geoL_1c )
{
     const char prognm[] = "GET_SCIA_LV1C_GEOL";

     register unsigned int   nr;
     register unsigned short ns;

     unsigned short icntr;

     if ( nr_geo == nr_geo1c ) {
	  (void) memcpy( geoL_1c, geoL, nr_geo * sizeof( struct geoL_scia ));
     } else if ( nr_geo > nr_geo1c ) {
	  const unsigned short nscale = (unsigned short)(nr_geo / nr_geo1c);

	  icntr = (unsigned short)(nscale / 2);
	  for ( nr = 0; nr < nr_geo1c; nr++ ) {
	       geoL_1c->pixel_type = geoL[0].pixel_type;
	       geoL_1c->glint_flag = UCHAR_ZERO;
	       geoL_1c->pos_asm = 0.f;
	       geoL_1c->pos_esm = 0.f;
	       geoL_1c->sat_h = 0.f;
	       geoL_1c->earth_rad = 0.f;
	       geoL_1c->dopp_shift = 0.f;
	       for ( ns = 0; ns < nscale; ns++ ) {
		    if ( (geoL[ns].pixel_type & DEEP_SPACE) != 0 )
			 geoL_1c->pixel_type |= DEEP_SPACE;
		    geoL_1c->glint_flag = 
                         (geoL_1c->glint_flag | geoL[ns].glint_flag);
		    geoL_1c->pos_asm += geoL[ns].pos_asm;
		    geoL_1c->pos_esm += geoL[ns].pos_esm;
		    geoL_1c->sat_h += geoL[ns].sat_h;
		    geoL_1c->earth_rad += geoL[ns].earth_rad;
		    geoL_1c->dopp_shift += geoL[ns].dopp_shift;
	       }
	       geoL_1c->pos_asm /= nscale;
	       geoL_1c->pos_esm /= nscale;
	       geoL_1c->sat_h /= nscale;
	       geoL_1c->earth_rad /= nscale;
	       geoL_1c->dopp_shift /= nscale;

	       geoL_1c->sun_zen_ang[0] = geoL[0].sun_zen_ang[0];
	       geoL_1c->sun_zen_ang[1] = geoL[icntr].sun_zen_ang[0];
	       geoL_1c->sun_zen_ang[2] = geoL[nscale-1].sun_zen_ang[2];
	       geoL_1c->sun_azi_ang[0] = geoL[0].sun_azi_ang[0];
	       geoL_1c->sun_azi_ang[1] = geoL[icntr].sun_azi_ang[0];
	       geoL_1c->sun_azi_ang[2] = geoL[nscale-1].sun_azi_ang[2];
	       geoL_1c->los_zen_ang[0] = geoL[0].los_zen_ang[0];
	       geoL_1c->los_zen_ang[1] = geoL[icntr].los_zen_ang[0];
	       geoL_1c->los_zen_ang[2] = geoL[nscale-1].los_zen_ang[2];
	       geoL_1c->los_azi_ang[0] = geoL[0].los_azi_ang[0];
	       geoL_1c->los_azi_ang[1] = geoL[icntr].los_azi_ang[0];
	       geoL_1c->los_azi_ang[2] = geoL[nscale-1].los_azi_ang[2];
	       geoL_1c->tan_h[0] = geoL[0].tan_h[0];
	       geoL_1c->tan_h[1] = geoL[icntr].tan_h[0];
	       geoL_1c->tan_h[2] = geoL[nscale-1].tan_h[2];

	       (void) memcpy( &geoL_1c->tang_ground_point[0],
			      &geoL->tang_ground_point[0],
			      sizeof(struct coord_envi));
	       (void) memcpy( &geoL_1c->tang_ground_point[1],
			      &geoL[icntr].tang_ground_point[0],
			      sizeof(struct coord_envi));
	       (void) memcpy( &geoL_1c->tang_ground_point[2],
			      &geoL[nscale-1].tang_ground_point[2],
			      sizeof(struct coord_envi));
	       if ( nscale % 2 == 0 ) {   /* even */
		    GET_CNTR_COORD( geoL[icntr-1].sub_sat_point,
				    geoL[icntr].sub_sat_point,
				    &geoL_1c->sub_sat_point );
	       } else {                   /* odd */
		    (void) memcpy( &geoL_1c->sub_sat_point,
				   &geoL[icntr].sub_sat_point,
				   sizeof(struct coord_envi));
	       }
	       geoL_1c++;
	       geoL += nscale;
	  }
     } else {
	  const unsigned short nscale = (unsigned short)(nr_geo1c / nr_geo);

	  int DoLonOffs;
	  double xx[3], yy[3], zz[3];
	  double *xval, *yval, *zval;

          xval = (double *) malloc( nscale * sizeof(double) );
          if ( xval == NULL ) 
               NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "xval" );
	  yval = (double *) malloc( nscale * sizeof(double) );
	  if ( yval == NULL ) {
	       free( xval );
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "yval" );
	  }
          zval = (double *) malloc( nscale * sizeof(double) );
          if ( zval == NULL ) {
               free( xval ); free( yval );
               NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "zval" );
          }
	  for ( nr = 0; nr < nr_geo; nr++ ) {
/* pixel_type and glint_flag */
	       geoL_1c[0].pixel_type = UCHAR_ZERO;
	       for ( ns = 0; ns < nscale; ns++ ) {
		    geoL_1c[ns].pixel_type |= geoL->pixel_type;
		    geoL_1c[ns].glint_flag = geoL->glint_flag;
	       }
/* pos_asm */
	       if ( nr == 0 )
		    Cntr_LinInterPol( NAN, geoL[0].pos_asm, 
				      geoL[1].pos_asm, nscale, yval );
	       else if ( nr == nr_geo-1 )
		    Cntr_LinInterPol( geoL[-1].pos_asm, geoL[0].pos_asm,
				      NAN, nscale, yval );
	       else
		    Cntr_LinInterPol( geoL[-1].pos_asm, geoL[0].pos_asm,
				      geoL[1].pos_asm, nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].pos_asm = (float) yval[ns];
/* pos_esm */
	       if ( nr == 0 )
		    Cntr_LinInterPol( NAN, geoL[0].pos_esm, 
				      geoL[1].pos_esm, nscale, yval );
	       else if ( nr == nr_geo-1 )
		    Cntr_LinInterPol( geoL[-1].pos_esm, geoL[0].pos_esm,
				      NAN, nscale, yval );
	       else
		    Cntr_LinInterPol( geoL[-1].pos_esm, geoL[0].pos_esm,
				      geoL[1].pos_esm, nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].pos_esm = (float) yval[ns];
/* sat_h */
	       if ( nr == 0 )
		    Cntr_LinInterPol( NAN, geoL[0].sat_h, 
				      geoL[1].sat_h, nscale, yval );
	       else if ( nr == nr_geo-1 )
		    Cntr_LinInterPol( geoL[-1].sat_h, geoL[0].sat_h,
				      NAN, nscale, yval );
	       else
		    Cntr_LinInterPol( geoL[-1].sat_h, geoL[0].sat_h,
				      geoL[1].sat_h, nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].sat_h = (float) yval[ns];
/* earth_rad */
	       if ( nr == 0 )
		    Cntr_LinInterPol( NAN, geoL[0].earth_rad, 
				      geoL[1].earth_rad, nscale, yval );
	       else if ( nr == nr_geo-1 )
		    Cntr_LinInterPol( geoL[-1].earth_rad, geoL[0].earth_rad,
				      NAN, nscale, yval );
	       else
		    Cntr_LinInterPol( geoL[-1].earth_rad, geoL[0].earth_rad,
				      geoL[1].earth_rad, nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].earth_rad = (float) yval[ns];
/* dopp_shift */
	       if ( nr == 0 )
		    Cntr_LinInterPol( NAN, geoL[0].dopp_shift, 
				      geoL[1].dopp_shift, nscale, yval );
	       else if ( nr == nr_geo-1 )
		    Cntr_LinInterPol( geoL[-1].dopp_shift, geoL[0].dopp_shift,
				      NAN, nscale, yval );
	       else
		    Cntr_LinInterPol( geoL[-1].dopp_shift, geoL[0].dopp_shift,
				      geoL[1].dopp_shift, nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].dopp_shift = (float) yval[ns];
/* sun_zen_ang */
	       Quick_LinInterPol( START_AT_ZERO, geoL->sun_zen_ang[0],
				  geoL->sun_zen_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].sun_zen_ang[0] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_CNTR, geoL->sun_zen_ang[0],
				  geoL->sun_zen_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].sun_zen_ang[1] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_ONE, geoL->sun_zen_ang[0],
				  geoL->sun_zen_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].sun_zen_ang[2] = (float) yval[ns];
/* sun_azi_ang */
	       Quick_LinInterPol( START_AT_ZERO, geoL->sun_azi_ang[0],
				  geoL->sun_azi_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].sun_azi_ang[0] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_CNTR, geoL->sun_azi_ang[0],
				  geoL->sun_azi_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].sun_azi_ang[1] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_ONE, geoL->sun_azi_ang[0],
				  geoL->sun_azi_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].sun_azi_ang[2] = (float) yval[ns];
/* los_zen_ang */
	       Quick_LinInterPol( START_AT_ZERO, geoL->los_zen_ang[0],
				  geoL->los_zen_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].los_zen_ang[0] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_CNTR, geoL->los_zen_ang[0],
				  geoL->los_zen_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].los_zen_ang[1] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_ONE, geoL->los_zen_ang[0],
				  geoL->los_zen_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].los_zen_ang[2] = (float) yval[ns];
/* los_azi_ang */
	       Quick_LinInterPol( START_AT_ZERO, geoL->los_azi_ang[0],
				  geoL->los_azi_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].los_azi_ang[0] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_CNTR, geoL->los_azi_ang[0],
				  geoL->los_azi_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].los_azi_ang[1] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_ONE, geoL->los_azi_ang[0],
				  geoL->los_azi_ang[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].los_azi_ang[2] = (float) yval[ns];
/* tan_h */
	       Quick_LinInterPol( START_AT_ZERO, geoL->tan_h[0],
				  geoL->tan_h[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].tan_h[0] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_CNTR, geoL->tan_h[0],
				  geoL->tan_h[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].tan_h[1] = (float) yval[ns];
	       Quick_LinInterPol( START_AT_ONE, geoL->tan_h[0],
				  geoL->tan_h[2], nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoL_1c[ns].tan_h[2] = (float) yval[ns];
/* sub-satellite point */
	       DoLonOffs = TRUE;
	       if ( geoL->sub_sat_point.lon < -95000000
		    || geoL->sub_sat_point.lon > 95000000
		    || (geoL->sub_sat_point.lon > -85000000
			&& geoL->sub_sat_point.lon < 85000000) )
		    DoLonOffs = FALSE;
	       if ( nr == 0 ) {
		    Coord2XYZ( DoLonOffs, geoL[0].sub_sat_point, xx, yy, zz );
		    Coord2XYZ( DoLonOffs, geoL[1].sub_sat_point, 
			       xx+1, yy+1, zz+1 );

		    Cntr_LinInterPol_d( NAN, xx[0], xx[1], nscale, xval );
		    Cntr_LinInterPol_d( NAN, yy[0], yy[1], nscale, yval );
		    Cntr_LinInterPol_d( NAN, zz[0], zz[1], nscale, zval );
	       } else if ( nr == nr_geo-1 ) {
		    Coord2XYZ( DoLonOffs, geoL[-1].sub_sat_point, 
			       xx, yy, zz );
		    Coord2XYZ( DoLonOffs, geoL[0].sub_sat_point, 
			       xx+1, yy+1, zz+1 );

		    Cntr_LinInterPol_d( xx[0], xx[1], NAN, nscale, xval );
		    Cntr_LinInterPol_d( yy[0], yy[1], NAN, nscale, yval );
		    Cntr_LinInterPol_d( zz[0], zz[1], NAN, nscale, zval );
	       } else {
		    Coord2XYZ( DoLonOffs, geoL[-1].sub_sat_point, 
			       xx, yy, zz );
		    Coord2XYZ( DoLonOffs, geoL[0].sub_sat_point, 
			       xx+1, yy+1, zz+1 );
		    Coord2XYZ( DoLonOffs, geoL[1].sub_sat_point, 
			       xx+2, yy+2, zz+2 );

		    Cntr_LinInterPol_d( xx[0], xx[1], xx[2], nscale, xval );
		    Cntr_LinInterPol_d( yy[0], yy[1], yy[2], nscale, yval );
		    Cntr_LinInterPol_d( zz[0], zz[1], zz[2], nscale, zval );
	       }
	       for ( ns = 0; ns < nscale; ns++ ) {
		    XYZ2Coord( DoLonOffs, xval[ns], yval[ns], zval[ns],
			       &geoL_1c[ns].sub_sat_point );
	       }
/* coordinates of tangent ground point */
	       DoLonOffs = TRUE;
	       if ( geoL->tang_ground_point[2].lon < -95000000
		    || geoL->tang_ground_point[0].lon > 95000000
		    || (geoL->tang_ground_point[0].lon > -85000000
			&& geoL->tang_ground_point[2].lon < 85000000) )
		    DoLonOffs = FALSE;
	       Coord2XYZ( DoLonOffs, geoL->tang_ground_point[0], xx, yy, zz );
	       Coord2XYZ( DoLonOffs, geoL->tang_ground_point[2], 
			  xx+1, yy+1, zz+1 );
               Quick_LinInterPol_d( START_AT_ZERO, xx[0], xx[1], nscale, xval);
               Quick_LinInterPol_d( START_AT_ZERO, yy[0], yy[1], nscale, yval);
               Quick_LinInterPol_d( START_AT_ZERO, zz[0], zz[1], nscale, zval);
               for ( ns = 0; ns < nscale; ns++ ) {
                    XYZ2Coord( DoLonOffs, xval[ns], yval[ns], zval[ns],
                               &geoL_1c[ns].tang_ground_point[0] );
               }
               Quick_LinInterPol_d( START_AT_ONE, xx[0], xx[1], nscale, xval );
               Quick_LinInterPol_d( START_AT_ONE, yy[0], yy[1], nscale, yval );
               Quick_LinInterPol_d( START_AT_ONE, zz[0], zz[1], nscale, zval );
               for ( ns = 0; ns < nscale; ns++ ) {
                    XYZ2Coord( DoLonOffs, xval[ns], yval[ns], zval[ns],
			       &geoL_1c[ns].tang_ground_point[2] );
	       }
	       for ( ns = 0; ns < nscale; ns++ ) {
		    GET_CNTR_COORD( geoL_1c[ns].tang_ground_point[0],
				    geoL_1c[ns].tang_ground_point[2],
				    &geoL_1c[ns].tang_ground_point[1] );
	       }
	       geoL++;
	       geoL_1c += nscale;
	  }
          free( xval );
          free( yval );
          free( zval );
     }
}
		      
/*+++++++++++++++++++++++++*/
void GET_SCIA_LV1C_GEOC( unsigned int nr_geo, const struct geoC_scia *geoC,
			 unsigned int nr_geo1c, struct geoC_scia *geoC_1c )
{
     const char prognm[] = "GET_SCIA_LV1C_GEOC";

     register unsigned int   nr;
     register unsigned short ns;

     unsigned short icntr;

     if ( nr_geo == nr_geo1c ) {
	  (void) memcpy( geoC_1c, geoC, nr_geo * sizeof( struct geoC_scia ));
     } else if ( nr_geo > nr_geo1c ) {
	  const unsigned short nscale = (unsigned short)(nr_geo / nr_geo1c);

	  icntr = (unsigned short)(nscale / 2);
	  for ( nr = 0; nr < nr_geo1c; nr++ ) {
	       geoC_1c->pos_asm = 0.f;
	       geoC_1c->pos_esm = 0.f;
	       for ( ns = 0; ns < nscale; ns++ ) {
		    geoC_1c->pos_asm += geoC[ns].pos_asm;
		    geoC_1c->pos_esm += geoC[ns].pos_esm;
	       }
	       geoC_1c->pos_asm /= nscale;
	       geoC_1c->pos_esm /= nscale;

	       geoC_1c->sun_zen_ang = geoC[icntr].sun_zen_ang;
	       if ( nscale % 2 == 0 ) {   /* even */
		    GET_CNTR_COORD( geoC[icntr-1].sub_sat_point,
				    geoC[icntr].sub_sat_point,
				    &geoC_1c->sub_sat_point );
	       } else {                   /* odd */
		    (void) memcpy( &geoC_1c->sub_sat_point,
				   &geoC[icntr].sub_sat_point,
				   sizeof(struct coord_envi));
	       }
	       geoC_1c++;
	       geoC += nscale;
	  }
     } else {
	  const unsigned short nscale = (unsigned short)(nr_geo1c / nr_geo);

	  int DoLonOffs;
	  double xx[3], yy[3], zz[3];
	  double *xval, *yval, *zval;

          xval = (double *) malloc( nscale * sizeof(double) );
          if ( xval == NULL ) 
               NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "xval" );
	  yval = (double *) malloc( nscale * sizeof(double) );
	  if ( yval == NULL ) {
	       free( xval );
	       NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "yval" );
	  }
          zval = (double *) malloc( nscale * sizeof(double) );
          if ( zval == NULL ) {
               free( xval ); free( yval );
               NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "zval" );
          }
	  for ( nr = 0; nr < nr_geo; nr++ ) {
/* pos_asm */
	       if ( nr == 0 )
		    Cntr_LinInterPol( NAN, geoC[0].pos_asm, 
				      geoC[1].pos_asm, nscale, yval );
	       else if ( nr == nr_geo-1 )
		    Cntr_LinInterPol( geoC[-1].pos_asm, geoC[0].pos_asm,
				      NAN, nscale, yval );
	       else
		    Cntr_LinInterPol( geoC[-1].pos_asm, geoC[0].pos_asm,
				      geoC[1].pos_asm, nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoC_1c[ns].pos_asm = (float) yval[ns];
/* pos_esm */
	       if ( nr == 0 )
		    Cntr_LinInterPol( NAN, geoC[0].pos_esm, 
				      geoC[1].pos_esm, nscale, yval );
	       else if ( nr == nr_geo-1 )
		    Cntr_LinInterPol( geoC[-1].pos_esm, geoC[0].pos_esm,
				      NAN, nscale, yval );
	       else
		    Cntr_LinInterPol( geoC[-1].pos_esm, geoC[0].pos_esm,
				      geoC[1].pos_esm, nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoC_1c[ns].pos_esm = (float) yval[ns];
/* sun_zen_ang */
	       if ( nr == 0 )
		    Cntr_LinInterPol( NAN, geoC[0].sun_zen_ang, 
				      geoC[1].sun_zen_ang, nscale, yval );
	       else if ( nr == nr_geo-1 )
		    Cntr_LinInterPol( geoC[-1].sun_zen_ang, 
				      geoC[0].sun_zen_ang,
				      NAN, nscale, yval );
	       else
		    Cntr_LinInterPol( geoC[-1].sun_zen_ang, 
				      geoC[0].sun_zen_ang,
				      geoC[1].sun_zen_ang, nscale, yval );
	       for ( ns = 0; ns < nscale; ns++ )
		    geoC_1c[ns].sun_zen_ang = (float) yval[ns];
/* sub-satellite point */
	       DoLonOffs = TRUE;
	       if ( geoC->sub_sat_point.lon < -95000000
		    || geoC->sub_sat_point.lon > 95000000
		    || (geoC->sub_sat_point.lon > -85000000
			&& geoC->sub_sat_point.lon < 85000000) )
		    DoLonOffs = FALSE;
	       if ( nr == 0 ) {
		    Coord2XYZ( DoLonOffs, geoC[0].sub_sat_point, xx, yy, zz );
		    Coord2XYZ( DoLonOffs, geoC[1].sub_sat_point, 
			       xx+1, yy+1, zz+1 );

		    Cntr_LinInterPol_d( NAN, xx[0], xx[1], nscale, xval );
		    Cntr_LinInterPol_d( NAN, yy[0], yy[1], nscale, yval );
		    Cntr_LinInterPol_d( NAN, zz[0], zz[1], nscale, zval );
	       } else if ( nr == nr_geo-1 ) {
		    Coord2XYZ( DoLonOffs, geoC[-1].sub_sat_point, 
			       xx, yy, zz );
		    Coord2XYZ( DoLonOffs, geoC[0].sub_sat_point, 
			       xx+1, yy+1, zz+1 );

		    Cntr_LinInterPol_d( xx[0], xx[1], NAN, nscale, xval );
		    Cntr_LinInterPol_d( yy[0], yy[1], NAN, nscale, yval );
		    Cntr_LinInterPol_d( zz[0], zz[1], NAN, nscale, zval );
	       } else {
		    Coord2XYZ( DoLonOffs, geoC[-1].sub_sat_point, 
			       xx, yy, zz );
		    Coord2XYZ( DoLonOffs, geoC[0].sub_sat_point, 
			       xx+1, yy+1, zz+1 );
		    Coord2XYZ( DoLonOffs, geoC[1].sub_sat_point, 
			       xx+2, yy+2, zz+2 );

		    Cntr_LinInterPol_d( xx[0], xx[1], xx[2], nscale, xval );
		    Cntr_LinInterPol_d( yy[0], yy[1], yy[2], nscale, yval );
		    Cntr_LinInterPol_d( zz[0], zz[1], zz[2], nscale, zval );
	       }
	       for ( ns = 0; ns < nscale; ns++ ) {
		    XYZ2Coord( DoLonOffs, xval[ns], yval[ns], zval[ns],
			       &geoC_1c[ns].sub_sat_point );
	       }
	       geoC++;
	       geoC_1c += nscale;
	  }
          free( xval );
          free( yval );
          free( zval );
     }
}
