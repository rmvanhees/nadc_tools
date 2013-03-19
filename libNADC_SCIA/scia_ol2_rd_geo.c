/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_OL2_RD_GEO
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 offline product
.LANGUAGE    ANSI C
.PURPOSE     read Geolocation Data Sets
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_OL2_RD_NGEO( fd, num_dsd, dsd, &geo );
            nr_dsr = SCIA_OL2_RD_LGEO( fd, num_dsd, dsd, &geo );

     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct ngeo_scia *geo :   Nadir Geolocation Data sets
            struct lgeo_scia *geo :   Limb/Occultation Geolocation Data sets

.RETURNS     number of data set records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      1.2   22-Jan-2003	bugfix, errors in the product definition, RvH
              1.1   21-Jan-2003 added check for presence of this dataset, RvH
              1.0   29-Apr-2002 created by R. M. van Hees
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
#include <math.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static inline
void Sun2Intel_NGEO( struct ngeo_scia *geo )
{
     register unsigned short ni;

     geo->mjd.days = byte_swap_32( geo->mjd.days );
     geo->mjd.secnd = byte_swap_u32( geo->mjd.secnd );
     geo->mjd.musec = byte_swap_u32( geo->mjd.musec );
     geo->intg_time = byte_swap_u16( geo->intg_time );
     for ( ni = 0; ni < 3; ni++ ) {
	  IEEE_Swap__FLT( &geo->sun_zen_ang[ni] );
	  IEEE_Swap__FLT( &geo->los_zen_ang[ni] );
	  IEEE_Swap__FLT( &geo->rel_azi_ang[ni] );
     }
     IEEE_Swap__FLT( &geo->sat_h );
     IEEE_Swap__FLT( &geo->radius );
     geo->subsat.lon = byte_swap_32( geo->subsat.lon );
     geo->subsat.lat = byte_swap_32( geo->subsat.lat );
     for ( ni = 0; ni < NUM_CORNERS; ni++ ) {
	  geo->corner[ni].lon = byte_swap_32( geo->corner[ni].lon );
	  geo->corner[ni].lat = byte_swap_32( geo->corner[ni].lat );
     }
     geo->center.lon = byte_swap_32( geo->center.lon );
     geo->center.lat = byte_swap_32( geo->center.lat );
}

static inline
void Sun2Intel_LGEO( struct lgeo_scia *geo )
{
     register unsigned short ni;

     geo->mjd.days = byte_swap_32( geo->mjd.days );
     geo->mjd.secnd = byte_swap_u32( geo->mjd.secnd );
     geo->mjd.musec = byte_swap_u32( geo->mjd.musec );
     geo->intg_time = byte_swap_u16( geo->intg_time );
     IEEE_Swap__FLT( &geo->sat_h );
     IEEE_Swap__FLT( &geo->radius );
     geo->subsat.lon = byte_swap_32( geo->subsat.lon );
     geo->subsat.lat = byte_swap_32( geo->subsat.lat );
     for ( ni = 0; ni < 3; ni++ ) {
	  IEEE_Swap__FLT( &geo->sun_zen_ang[ni] );
	  IEEE_Swap__FLT( &geo->los_zen_ang[ni] );
	  IEEE_Swap__FLT( &geo->rel_azi_ang[ni] );
	  IEEE_Swap__FLT( &geo->tan_h[ni] );
	  geo->tang[ni].lon = byte_swap_32( geo->tang[ni].lon );
	  geo->tang[ni].lat = byte_swap_32( geo->tang[ni].lat );
     }     
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++
.IDENTifer  Coord2XYZ
.PURPOSE    transform (lat,lon) coordinates to (x,y,z) coordinated 
.INPUT/OUTPUT
  call as   Coord2XYZ( DoLonOffs, coord, xx, yy, zz );

     input:
            struct coord_envi coord : lat/lon coordinates
    output:
            double xx               :   x-coordinates
            double yy               :   y-coordinates
            double zz               :   z-coordinates

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void Coord2XYZ( bool DoLonOffs, const struct coord_envi coord, 
		/*@out@*/ double *xx, /*@out@*/ double *yy, 
		/*@out@*/ double *zz )
     /*@globals  errno@*/
{
     double coslat, sinlat, coslon, sinlon;

     const double LonOffs   = PI / 4.;
     const double mudeg2rad = DEG2RAD / 1e6;

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
.IDENTifer  getBackScanFlags
.PURPOSE    set pixel_type in structure ngeo_scia
.INPUT/OUTPUT
  call as  getBackScanFlags( unsigned int num_geo, struct ngeo_scia *geo )

     input:
            unsigned int num_geo  :   number of ngeo_scia records
 in/output:
            struct ngeo_scia *geo :   nadir geolocation records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
void getBackScanFlags( unsigned int num_geo, struct ngeo_scia *geo )
{
     register unsigned int num = 0u;
     register struct ngeo_scia *geo_pntr = geo;

     double threshold;
     double xx[2], yy[2], zz[2];
     double *dist = (double *) malloc( num_geo * sizeof(double) );
     
     do {
	  register bool  DoLonOffs = TRUE;

	  if ( geo_pntr->corner[2].lon < -95000000
	       || geo_pntr->corner[0].lon > 95000000
	       || (geo_pntr->corner[0].lon > -85000000
		   && geo_pntr->corner[2].lon < 85000000) )
	       DoLonOffs = FALSE;

	  Coord2XYZ( DoLonOffs, geo_pntr->corner[1], xx, yy, zz );
	  Coord2XYZ( DoLonOffs, geo_pntr->corner[2], xx+1, yy+1, zz+1 );
	  dist[num] = sqrt( (xx[1] - xx[0]) * (xx[1] - xx[0])
			    + (yy[1] - yy[0]) * (yy[1] - yy[0])
			    + (zz[1] - zz[0]) * (zz[1] - zz[0]) );
	  geo_pntr++;
     } while ( ++num < num_geo );

     threshold = 2 * SELECTd( (size_t) (num_geo/2), (size_t) num_geo, dist );
     for ( num = 0; num < num_geo; num++ )
	  geo[num].pixel_type = (unsigned char) 
	       ((dist[num] <= threshold) ? FORWARD_SCAN : BACK_SCAN);
     free( dist );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned 
int SCIA_OL2_RD_NGEO( FILE *fd, unsigned int num_dsd,
		      const struct dsd_envi *dsd, 
		      struct ngeo_scia **geo_out )
{
     const char prognm[] = "SCIA_OL2_RD_NGEO";

     char         *geo_pntr, *geo_char = NULL;
     size_t       dsr_size;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct ngeo_scia *geo;

     const char dsd_name[] = "GEOLOCATION_NADIR";
/*
 * get index to data set descriptor
 */
     NADC_ERR_SAVE();
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( IS_ERR_STAT_ABSENT || dsd[indx_dsd].num_dsr == 0 ) {
          NADC_ERR_RESTORE();
          geo_out[0] = NULL;
          return 0u;
     }
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
/*
 * allocate memory to store output structures
 */
     if ( ! Use_Extern_Alloc ) {
	  geo_out[0] = (struct ngeo_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct ngeo_scia));
     }
     if ( (geo = geo_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "geo" );
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (geo_char = (char *) malloc( dsr_size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "geo_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( geo_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to GEO structure
 */
	  (void) memcpy( &geo->mjd.days, geo_char, ENVI_INT );
	  geo_pntr = geo_char + ENVI_INT;
	  (void) memcpy( &geo->mjd.secnd, geo_pntr, ENVI_UINT );
	  geo_pntr += ENVI_UINT;
	  (void) memcpy( &geo->mjd.musec, geo_pntr, ENVI_UINT );
	  geo_pntr += ENVI_UINT;
	  (void) memcpy( &geo->flag_mds, geo_pntr, ENVI_UCHAR );
	  geo_pntr += ENVI_UCHAR;

	  (void) memcpy( &geo->intg_time, geo_pntr, ENVI_USHRT );
	  geo_pntr += ENVI_USHRT;
	  (void) memcpy( geo->sun_zen_ang, geo_pntr, 3 * ENVI_FLOAT );
	  geo_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( geo->los_zen_ang, geo_pntr, 3 * ENVI_FLOAT );
	  geo_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( geo->rel_azi_ang, geo_pntr, 3 * ENVI_FLOAT );
	  geo_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( &geo->sat_h, geo_pntr, ENVI_FLOAT );
	  geo_pntr += ENVI_FLOAT;
	  (void) memcpy( &geo->radius, geo_pntr, ENVI_FLOAT );
	  geo_pntr += ENVI_FLOAT;
	  (void) memcpy( &geo->subsat, geo_pntr, 
			   sizeof(struct coord_envi) );
	  geo_pntr += sizeof(struct coord_envi);
	  (void) memcpy( geo->corner, geo_pntr, 
			   NUM_CORNERS * sizeof(struct coord_envi) );
	  geo_pntr += NUM_CORNERS * sizeof(struct coord_envi);
	  (void) memcpy( &geo->center, geo_pntr, 
			   sizeof(struct coord_envi) );
	  geo_pntr += sizeof(struct coord_envi);
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(geo_pntr - geo_char) != dsr_size )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_NGEO( geo );
#endif
	  geo++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
     geo_pntr = NULL;
/*
 * set indicator for back-scan pixels
 */
     getBackScanFlags( dsd[indx_dsd].num_dsr, (geo = geo_out[0]) );
/*
 * set return values
 */
 done:
     if ( geo_char != NULL ) free( geo_char );

     return nr_dsr;
}

unsigned 
int SCIA_OL2_RD_LGEO( FILE *fd, unsigned int num_dsd,
		      const struct dsd_envi *dsd, 
		      struct lgeo_scia **geo_out )
{
     const char prognm[] = "SCIA_OL2_RD_LGEO";

     char         *geo_pntr, *geo_char = NULL;
     size_t       dsr_size;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct lgeo_scia *geo;

     const char dsd_name[] = "GEOLOCATION_LIMB";
/*
 * get index to data set descriptor
 */
     NADC_ERR_SAVE();
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( IS_ERR_STAT_ABSENT || dsd[indx_dsd].num_dsr == 0 ) {
          NADC_ERR_RESTORE();
          geo_out[0] = NULL;
          return 0u;
     }
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
/*
 * allocate memory to store output structures
 */
     if ( ! Use_Extern_Alloc ) {
	  geo_out[0] = (struct lgeo_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct lgeo_scia));
     }
     if ( (geo = geo_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "geo" );
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (geo_char = (char *) malloc( dsr_size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "geo_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( geo_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to GEO structure
 */
	  (void) memcpy( &geo->mjd.days, geo_char, ENVI_INT );
	  geo_pntr = geo_char + ENVI_INT;
	  (void) memcpy( &geo->mjd.secnd, geo_pntr, ENVI_UINT );
	  geo_pntr += ENVI_UINT;
	  (void) memcpy( &geo->mjd.musec, geo_pntr, ENVI_UINT );
	  geo_pntr += ENVI_UINT;
	  (void) memcpy( &geo->flag_mds, geo_pntr, ENVI_UCHAR );
	  geo_pntr += ENVI_UCHAR;

	  (void) memcpy( &geo->intg_time, geo_pntr, ENVI_USHRT );
	  geo_pntr += ENVI_USHRT;
	  (void) memcpy( geo->sun_zen_ang, geo_pntr, 3 * ENVI_FLOAT );
	  geo_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( geo->los_zen_ang, geo_pntr, 3 * ENVI_FLOAT );
	  geo_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( geo->rel_azi_ang, geo_pntr, 3 * ENVI_FLOAT );
	  geo_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( &geo->sat_h, geo_pntr, ENVI_FLOAT );
	  geo_pntr += ENVI_FLOAT;
	  (void) memcpy( &geo->radius, geo_pntr, ENVI_FLOAT );
	  geo_pntr += ENVI_FLOAT;
	  (void) memcpy( &geo->subsat, geo_pntr, 
			   sizeof(struct coord_envi) );
	  geo_pntr += sizeof(struct coord_envi);
	  (void) memcpy( geo->tang, geo_pntr, 
			   3 * sizeof(struct coord_envi) );
	  geo_pntr += 3 * sizeof(struct coord_envi);
	  (void) memcpy( &geo->tan_h, geo_pntr, 3 * ENVI_FLOAT );
	  geo_pntr += 3 * ENVI_FLOAT;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(geo_pntr - geo_char) != dsr_size )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_LGEO( geo );
#endif
	  geo++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
     geo_pntr = NULL;
/*
 * set return values
 */
 done:
     if ( geo_char != NULL ) free( geo_char );

     return nr_dsr;
}
