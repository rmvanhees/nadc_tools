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

.IDENTifer   SCIA_LV2_WR_H5_GEO
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 2 Geolocation Data Set(s)
.INPUT/OUTPUT
  call as    SCIA_LV2_WR_H5_GEO( param, nr_geo, geo );
     input:  
             struct param_record param  : struct holding user-defined settings
	     unsigned int nr_geo        : number of Geolocations data sets
	     struct geo_scia *geo       : Geolocation Data Set(s)

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   26-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.1   03-Sep-2002	moved DSD-data to subgroup ADS, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.0   13-Sep-2001	created by R. M. van Hees 
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

#define NFIELDS    10

static const size_t geo_size = sizeof( struct geo_scia );
static const size_t geo_offs[NFIELDS] = {
     HOFFSET( struct geo_scia, mjd ),
     HOFFSET( struct geo_scia, flag_mds ),
     HOFFSET( struct geo_scia, intg_time ),
     HOFFSET( struct geo_scia, sun_zen_ang ),
     HOFFSET( struct geo_scia, los_zen_ang ),
     HOFFSET( struct geo_scia, rel_azi_ang ),
     HOFFSET( struct geo_scia, sat_h ),
     HOFFSET( struct geo_scia, earth_rad ),
     HOFFSET( struct geo_scia, corner ),
     HOFFSET( struct geo_scia, center )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV2_WR_H5_GEO( struct param_record param, unsigned int nr_geo,
			 const struct geo_scia *geo )
{
     hid_t   ads_id;
     hid_t   type_id;
     hbool_t compress;
     hsize_t adim;
     hid_t   geo_type[NFIELDS];

     const char *geo_names[NFIELDS] = {
          "dsr_time", "attach_flag", "integr_time", "sol_zen_anlge_toa", 
	  "los_zen_angle_toa", "rel_azi_angle_toa", "sat_geod_ht", 
	  "earth_rad", "cor_coor_nad", "cen_coor_nad"
     };
/*
 * check number of GEO records
 */
     if ( nr_geo == 0 ) return;
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
     geo_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     geo_type[1] = H5Tcopy( H5T_NATIVE_UCHAR );
     geo_type[2] = H5Tcopy( H5T_NATIVE_USHORT );
     adim = 3;
     geo_type[3] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     geo_type[4] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     geo_type[5] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     geo_type[6] = H5Tcopy( H5T_NATIVE_FLOAT );
     geo_type[7] = H5Tcopy( H5T_NATIVE_FLOAT );
     adim = NUM_CORNERS;
     type_id = H5Topen( param.hdf_file_id, "coord", H5P_DEFAULT );
     geo_type[8] = H5Tarray_create( type_id, 1, &adim );
     (void) H5Tclose( type_id );
     geo_type[9] = H5Topen( param.hdf_file_id, "coord", H5P_DEFAULT );
/*
 * create table
 */
     (void) H5TBmake_table( "Geolocation ADS", ads_id, "geo", NFIELDS, 
			    nr_geo, geo_size, geo_names, geo_offs, 
			    geo_type, nr_geo, NULL, compress, geo );
/*
 * close interface
 */
     (void) H5Tclose( geo_type[0] );
     (void) H5Tclose( geo_type[1] );
     (void) H5Tclose( geo_type[2] );
     (void) H5Tclose( geo_type[3] );
     (void) H5Tclose( geo_type[4] );
     (void) H5Tclose( geo_type[5] );
     (void) H5Tclose( geo_type[6] );
     (void) H5Tclose( geo_type[7] );
     (void) H5Tclose( geo_type[8] );
     (void) H5Tclose( geo_type[9] );
     (void) H5Gclose( ads_id );
}
