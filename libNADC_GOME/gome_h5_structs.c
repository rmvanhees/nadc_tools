/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2003 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GOME_LV0_H5_STRUCTS
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1b, HDF5
.LANGUAGE    ANSI C
.PURPOSE     create data structures to store GOME compound data types

.INPUT/OUTPUT
  call as   CRE_GOME_LV1_H5_STRUCTS( param );

     input:  
            struct param_record param : struct holding user-defined settings

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   08-Jun-2009 update to product version 2, RvH
              1.0   09-Apr-2003 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <hdf5.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++++++++++++++++++++++
.IDENTifer   CRE_GOME_LV1_H5_STRUCTS
.PURPOSE     create data structures to store GOME compound data types
.INPUT/OUTPUT
  call as   CRE_GOME_LV1_H5_STRUCTS( param );
     input:  
            struct param_record param : struct holding user-defined settings

.RETURNS     Nothing
.COMMENT     None
-------------------------*/
void CRE_GOME_LV1_H5_STRUCTS( struct param_record param )
{
     hid_t   arr_id, type_id;
     hsize_t adim;
/*
 * create the "glr1" structure
 */
     type_id = H5Tcreate( H5T_COMPOUND, sizeof( struct glr1_gome ));
     (void) H5Tinsert( type_id, "sun_glint_flag", 
		       HOFFSET(struct glr1_gome, sun_glint), 
		       H5T_NATIVE_UCHAR);
     (void) H5Tinsert( type_id, "pixel_type", 
		       HOFFSET(struct glr1_gome, subsetcounter), 
		       H5T_NATIVE_UCHAR);
     (void) H5Tinsert( type_id, "UTC_Date", 
		       HOFFSET(struct glr1_gome, utc_date), 
		       H5T_NATIVE_UINT );
     (void) H5Tinsert( type_id, "UTC_Time", 
		       HOFFSET(struct glr1_gome, utc_time), 
		       H5T_NATIVE_UINT );
     (void) H5Tinsert( type_id, "sat_h", 
		       HOFFSET(struct glr1_gome, sat_geo_height), 
		       H5T_NATIVE_FLOAT );
     (void) H5Tinsert( type_id, "earth_rad", 
		       HOFFSET(struct glr1_gome, earth_radius), 
		       H5T_NATIVE_FLOAT );
     adim = 3;
     arr_id = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     (void) H5Tinsert( type_id, "sun_zen_sat_north_ang", 
		       HOFFSET(struct glr1_gome, sun_zen_sat_north), arr_id );
     (void) H5Tinsert( type_id, "sun_azi_sat_north_ang", 
		       HOFFSET(struct glr1_gome, sun_azim_sat_north), arr_id );
     (void) H5Tinsert( type_id, "los_zen_sat_north_ang", 
		       HOFFSET(struct glr1_gome, los_zen_sat_north), arr_id );
     (void) H5Tinsert( type_id, "los_azi_sat_north_ang", 
		       HOFFSET(struct glr1_gome, los_azim_sat_north), arr_id );
     (void) H5Tinsert( type_id, "sun_zen_sat_ers_ang", 
		       HOFFSET(struct glr1_gome, sun_zen_sat_ers), arr_id );
     (void) H5Tinsert( type_id, "sun_azi_sat_ers_ang", 
		       HOFFSET(struct glr1_gome, sun_azim_sat_ers), arr_id );
     (void) H5Tinsert( type_id, "los_zen_sat_ers_ang", 
		       HOFFSET(struct glr1_gome, los_zen_sat_ers), arr_id );
     (void) H5Tinsert( type_id, "los_azi_sat_ers_ang", 
		       HOFFSET(struct glr1_gome, los_azim_sat_ers), arr_id );
     (void) H5Tinsert( type_id, "sun_zen_surf_north_ang", 
		       HOFFSET(struct glr1_gome, sun_zen_surf_north), arr_id );
     (void) H5Tinsert( type_id, "sun_azi_surf_north_ang", 
		       HOFFSET(struct glr1_gome, sun_azim_surf_north), arr_id );
     (void) H5Tinsert( type_id, "los_zen_surf_north_ang", 
		       HOFFSET(struct glr1_gome, los_zen_surf_north), arr_id );
     (void) H5Tinsert( type_id, "los_azi_surf_north_ang", 
		       HOFFSET(struct glr1_gome, los_azim_surf_north), arr_id );
      (void) H5Tclose( arr_id );

     adim = NUM_COORDS;
     arr_id = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     (void) H5Tinsert( type_id, "lon", 
		       HOFFSET(struct glr1_gome, lon), arr_id );
     (void) H5Tinsert( type_id, "lat", 
		       HOFFSET(struct glr1_gome, lat), arr_id );
     (void) H5Tclose( arr_id );
     (void) H5Tcommit( param.hdf_file_id, "glr", type_id, 
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     (void) H5Tclose( type_id );
/*
 * create the "cr1" structure
 */
      type_id = H5Tcreate( H5T_COMPOUND, sizeof( struct cr1_gome ));
     (void) H5Tinsert( type_id, "mode", 
		       HOFFSET(struct cr1_gome, mode), 
		       H5T_NATIVE_SHORT);
     (void) H5Tinsert( type_id, "type", 
		       HOFFSET(struct cr1_gome, type), 
		       H5T_NATIVE_SHORT);
     (void) H5Tinsert( type_id, "surfaceHeight", 
		       HOFFSET(struct cr1_gome, surfaceHeight), 
		       H5T_NATIVE_FLOAT);
     (void) H5Tinsert( type_id, "fraction", 
		       HOFFSET(struct cr1_gome, fraction), 
		       H5T_NATIVE_FLOAT);
     (void) H5Tinsert( type_id, "fractionError", 
		       HOFFSET(struct cr1_gome, fractionError), 
		       H5T_NATIVE_FLOAT);
     (void) H5Tinsert( type_id, "albedo", 
		       HOFFSET(struct cr1_gome, albedo), 
		       H5T_NATIVE_FLOAT);
     (void) H5Tinsert( type_id, "albedoError", 
		       HOFFSET(struct cr1_gome, albedoError), 
		       H5T_NATIVE_FLOAT);
     (void) H5Tinsert( type_id, "height", 
		       HOFFSET(struct cr1_gome, height), 
		       H5T_NATIVE_FLOAT);
     (void) H5Tinsert( type_id, "heightError", 
		       HOFFSET(struct cr1_gome, heightError), 
		       H5T_NATIVE_FLOAT);
     (void) H5Tinsert( type_id, "thickness", 
		       HOFFSET(struct cr1_gome, thickness), 
		       H5T_NATIVE_FLOAT);
     (void) H5Tinsert( type_id, "thicknessError", 
		       HOFFSET(struct cr1_gome, thicknessError), 
		       H5T_NATIVE_FLOAT);
     (void) H5Tinsert( type_id, "topPress", 
		       HOFFSET(struct cr1_gome, topPress), 
		       H5T_NATIVE_FLOAT);
     (void) H5Tinsert( type_id, "topPressError", 
		       HOFFSET(struct cr1_gome, topPressError), 
		       H5T_NATIVE_FLOAT);
     (void) H5Tcommit( param.hdf_file_id, "cld", type_id, 
		       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     (void) H5Tclose( type_id );
}
