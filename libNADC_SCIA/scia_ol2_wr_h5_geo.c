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

.IDENTifer   SCIA_OL2_WR_H5_GEO
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 Offline - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 2 Geolocation Data Set(s)
.INPUT/OUTPUT
  call as    SCIA_OL2_WR_H5_NGEO( param, nr_geo, geo );
             SCIA_OL2_WR_H5_LGEO( param, nr_geo, geo );
     input:  
             struct param_record param  : struct holding user-defined settings
	     unsigned int nr_geo        : number of Geolocations data sets
	     struct ngeo_scia *geo      : Nadir Geolocation Data Set(s)
	     struct lgeo_scia *geo      : Limb Geolocation Data Set(s)

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   26-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              1.2   22-Jan-2003	bugfix, errors in the product definition, RvH
              1.1   14-May-2002	added scia_ol2_wr_h5_lgeo, RvH
              1.0   29-Apr-2002	Created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

#define NFIELDS    11

static const size_t ngeo_size = sizeof( struct ngeo_scia );
static const size_t ngeo_offs[NFIELDS] = {
     HOFFSET( struct ngeo_scia, mjd ),
     HOFFSET( struct ngeo_scia, flag_mds ),
     HOFFSET( struct ngeo_scia, intg_time ),
     HOFFSET( struct ngeo_scia, sun_zen_ang ),
     HOFFSET( struct ngeo_scia, los_zen_ang ),
     HOFFSET( struct ngeo_scia, rel_azi_ang ),
     HOFFSET( struct ngeo_scia, sat_h ),
     HOFFSET( struct ngeo_scia, radius ),
     HOFFSET( struct ngeo_scia, subsat ),
     HOFFSET( struct ngeo_scia, corner ),
     HOFFSET( struct ngeo_scia, center )
};

static const size_t lgeo_size = sizeof( struct lgeo_scia );
static const size_t lgeo_offs[NFIELDS] = {
     HOFFSET( struct lgeo_scia, mjd ),
     HOFFSET( struct lgeo_scia, flag_mds ),
     HOFFSET( struct lgeo_scia, intg_time ),
     HOFFSET( struct lgeo_scia, sun_zen_ang ),
     HOFFSET( struct lgeo_scia, los_zen_ang ),
     HOFFSET( struct lgeo_scia, rel_azi_ang ),
     HOFFSET( struct lgeo_scia, sat_h ),
     HOFFSET( struct lgeo_scia, radius ),
     HOFFSET( struct lgeo_scia, tan_h ),
     HOFFSET( struct lgeo_scia, subsat ),
     HOFFSET( struct lgeo_scia, tang )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_OL2_WR_H5_NGEO( struct param_record param, unsigned int nr_ngeo,
			  const struct ngeo_scia *ngeo )
{
     hid_t   ads_id;
     hid_t   type_id;
     hbool_t compress;
     hsize_t adim;
     hid_t   ngeo_type[NFIELDS];

     const char *ngeo_names[NFIELDS] = {
          "dsr_time", "attach_flag", "integr_time", "sol_zen_anlge_toa", 
	  "los_zen_angle_toa", "rel_azi_angle_toa", "sat_geod_ht", 
	  "earth_rad", "sub_sat_point", "cor_coor_nad", "cen_coor_nad"
     };
/*
 * check number of NGEO records
 */
     if ( nr_ngeo == 0 ) return;
/*
 * set HDF5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
          compress = TRUE;
     else
          compress = FALSE;
/*
 * create group /ADS
 */
     ads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/ADS" );
     if ( ads_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/ADS" );
/*
 * define user-defined data types of the Table-fields
 */
     ngeo_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     ngeo_type[1] = H5Tcopy( H5T_NATIVE_UCHAR );
     ngeo_type[2] = H5Tcopy( H5T_NATIVE_USHORT );
     adim = 3;
     ngeo_type[3] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     ngeo_type[4] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     ngeo_type[5] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     ngeo_type[6] = H5Tcopy( H5T_NATIVE_FLOAT );
     ngeo_type[7] = H5Tcopy( H5T_NATIVE_FLOAT );
     ngeo_type[8] = H5Topen( param.hdf_file_id, "coord", H5P_DEFAULT );
     adim = NUM_CORNERS;
     type_id = H5Topen( param.hdf_file_id, "coord", H5P_DEFAULT );
     ngeo_type[9] = H5Tarray_create( type_id, 1, &adim );
     (void) H5Tclose( type_id );
     ngeo_type[10] = H5Topen( param.hdf_file_id, "coord", H5P_DEFAULT );
/*
 * create table
 */
     (void) H5TBmake_table( "Geolocation ADS (Nadir)", ads_id, "ngeo", NFIELDS,
			    nr_ngeo, ngeo_size, ngeo_names, ngeo_offs, 
			    ngeo_type, nr_ngeo, NULL, compress, ngeo );
/*
 * close interface
 */
     (void) H5Tclose( ngeo_type[0] );
     (void) H5Tclose( ngeo_type[1] );
     (void) H5Tclose( ngeo_type[2] );
     (void) H5Tclose( ngeo_type[3] );
     (void) H5Tclose( ngeo_type[4] );
     (void) H5Tclose( ngeo_type[5] );
     (void) H5Tclose( ngeo_type[6] );
     (void) H5Tclose( ngeo_type[7] );
     (void) H5Tclose( ngeo_type[8] );
     (void) H5Tclose( ngeo_type[9] );
     (void) H5Tclose( ngeo_type[10] );
     (void) H5Gclose( ads_id );
}

void SCIA_OL2_WR_H5_LGEO( struct param_record param, unsigned int nr_lgeo,
			  const struct lgeo_scia *lgeo )
{
     hid_t   ads_id;
     hid_t   type_id;
     hbool_t compress;
     hsize_t adim;
     hid_t   lgeo_type[NFIELDS];

     const char *lgeo_names[NFIELDS] = {
          "dsr_time", "attach_flag", "integr_time", "sol_zen_anlge_toa", 
	  "los_zen_angle_toa", "rel_azi_angle_toa", "sat_geod_ht", 
	  "earth_rad", "tangent_height", "sub_sat_point", "tangent_coord"
     };
/*
 * check number of GEO records
 */
     if ( nr_lgeo == 0 ) return;
/*
 * set HDF5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
          compress = TRUE;
     else
          compress = FALSE;
/*
 * create group /ADS
 */
     ads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/ADS" );
     if ( ads_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/ADS" );
/*
 * define user-defined data types of the Table-fields
 */
     lgeo_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     lgeo_type[1] = H5Tcopy( H5T_NATIVE_UCHAR );
     lgeo_type[2] = H5Tcopy( H5T_NATIVE_USHORT );
     adim = 3;
     lgeo_type[3] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     lgeo_type[4] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     lgeo_type[5] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     lgeo_type[6] = H5Tcopy( H5T_NATIVE_FLOAT );
     lgeo_type[7] = H5Tcopy( H5T_NATIVE_FLOAT );
     adim = 3;
     lgeo_type[8] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     lgeo_type[9] = H5Topen( param.hdf_file_id, "coord", H5P_DEFAULT );
     type_id = H5Topen( param.hdf_file_id, "coord", H5P_DEFAULT );
     lgeo_type[10] = H5Tarray_create( type_id, 1, &adim );
     (void) H5Tclose( type_id );
/*
 * create table
 */
     (void) H5TBmake_table( "Geolocation ADS (Limb)", ads_id, "lgeo", NFIELDS,
			    nr_lgeo, lgeo_size, lgeo_names, lgeo_offs, 
			    lgeo_type, nr_lgeo, NULL, compress, lgeo );
/*
 * close interface
 */
     (void) H5Tclose( lgeo_type[0] );
     (void) H5Tclose( lgeo_type[1] );
     (void) H5Tclose( lgeo_type[2] );
     (void) H5Tclose( lgeo_type[3] );
     (void) H5Tclose( lgeo_type[4] );
     (void) H5Tclose( lgeo_type[5] );
     (void) H5Tclose( lgeo_type[6] );
     (void) H5Tclose( lgeo_type[7] );
     (void) H5Tclose( lgeo_type[8] );
     (void) H5Tclose( lgeo_type[9] );
     (void) H5Tclose( lgeo_type[10] );
     (void) H5Gclose( ads_id );
}
