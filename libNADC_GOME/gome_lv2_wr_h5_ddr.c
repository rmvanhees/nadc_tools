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

.IDENTifer   GOME_LV2_WR_H5_DDR
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 2 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write GOME level 2 DDR data

.INPUT/OUTPUT
  call as    GOME_LV2_WR_H5_DDR( param, nr_ddr, ddr );
  input: 
             struct param_record param : struct holding user-defined settings
	     int  nr_ddr               : number of DDR records
	     struct ddr_gome *ddr      : DOAS Data records

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   11-Nov-2001	moved to the new Error handling routines, RvH
              1.1   06-Nov-2000	let this module create its own group, RvH 
              1.0   03-Aug-1999 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdlib.h>

#include <hdf5.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_LV2_WR_H5_DDR( struct param_record param, short nr_ddr, 
			 const struct ddr_gome *ddr )
{
     register hsize_t nr, nx, ny;

     int          *ibuff;
     unsigned int *ubuff;
     float        *rbuff;

     hid_t    grp_id;
     hbool_t  compress;
     hsize_t  nrpix, dims[2];
/*
 * check number of DDR records
 */
     if ( nr_ddr == 0 ) return;
/*
 * set HDF5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
          compress = TRUE;
     else
          compress = FALSE;
/*
 * create group /DDR
 */
     grp_id = H5Gcreate( param.hdf_file_id, "/DDR", 
			 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     if ( grp_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/DDR" );
/*
 * +++++ create datasets in the /DDR group
 */
     dims[0] = nr_ddr;
/*
 * Ground Pixel Number and SubSet Counter
 */
     ibuff = (int *) malloc( dims[0] * sizeof(int));
     if ( ibuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "ibuff" );
     for ( ny = 0; ny < dims[0]; ny++ )
	  ibuff[ny] = ddr[ny].glr.pixel_nr;
     NADC_WR_HDF5_Dataset( compress, grp_id, "Ground Pixel Number", 
			  H5T_NATIVE_INT, 1, dims, ibuff );
     for ( ny = 0; ny < dims[0]; ny++ )
	  ibuff[ny] = ddr[ny].glr.subsetcounter;
     NADC_WR_HDF5_Dataset( compress, grp_id, "SubSetCounter", 
			  H5T_NATIVE_INT, 1, dims, ibuff );
     free( ibuff );
/*
 * UTC date and time
 */
     ubuff = (unsigned int *) malloc( dims[0] * sizeof( unsigned int));
     if ( ubuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "ubuff" );
     for ( ny = 0; ny < dims[0]; ny++ )
	  ubuff[ny] = ddr[ny].glr.utc_date;
     NADC_WR_HDF5_Dataset( compress, grp_id, "UTC date", 
			  H5T_NATIVE_UINT, 1, dims, ubuff );
     for ( ny = 0; ny < dims[0]; ny++ )
	  ubuff[ny] = ddr[ny].glr.utc_time;
     NADC_WR_HDF5_Dataset( compress, grp_id, "UTC time", 
			  H5T_NATIVE_UINT, 1, dims, ubuff );
     free( ubuff );
/*
 * Solar Zenith Angles at satellite
 */
     dims[0] = nr_ddr; dims[1] = 3;
     nrpix = dims[0] * dims[1];
     rbuff = (float *) malloc( nrpix * sizeof( float ));
     if ( rbuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );
     for ( nr = ny = 0; ny < dims[0]; ny++ )
	  for ( nx = 0; nx < dims[1]; nx++ )
	       rbuff[nr++] = ddr[ny].glr.sat_zenith[nx];
     NADC_WR_HDF5_Dataset( compress, grp_id, "Satellite Solar Zenith angle",
			  H5T_NATIVE_FLOAT, 2, dims, rbuff );
/*
 * Line of Sight angles at satellite
 */
     for ( nr = ny = 0; ny < dims[0]; ny++ )
	  for ( nx = 0; nx < dims[1]; nx++ )
	       rbuff[nr++] = ddr[ny].glr.sat_sight[nx];
     NADC_WR_HDF5_Dataset( compress, grp_id, "Satellite Line_of_Sight angle",
			  H5T_NATIVE_FLOAT, 2, dims, rbuff );
/*
 * Relative Azimuth angles at satellite
 */
     for ( nr = ny = 0; ny < dims[0]; ny++ )
	  for ( nx = 0; nx < dims[1]; nx++ )
	       rbuff[nr++] = ddr[ny].glr.sat_azim[nx];
     NADC_WR_HDF5_Dataset( compress, grp_id, 
			   "Satellite Relative Azimuth angle",
			   H5T_NATIVE_FLOAT, 2, dims, rbuff );
/*
 * Solar Zenith angles at TOA
 */
     for ( nr = ny = 0; ny < dims[0]; ny++ )
	  for ( nx = 0; nx < dims[1]; nx++ )
	       rbuff[nr++] = ddr[ny].glr.toa_zenith[nx];
     NADC_WR_HDF5_Dataset( compress, grp_id, "TOA Solar Zenith angle",
			   H5T_NATIVE_FLOAT, 2, dims, rbuff );
/*
 * Relative Azimuth angles at TOA
 */
     for ( nr = ny = 0; ny < dims[0]; ny++ )
	  for ( nx = 0; nx < dims[1]; nx++ )
	       rbuff[nr++] = ddr[ny].glr.toa_sight[nx];
     NADC_WR_HDF5_Dataset( compress, grp_id, "TOA Line_of_Sight angle",
			   H5T_NATIVE_FLOAT, 2, dims, rbuff );
/*
 * Relative Azimuth angles at TOA
 */
     for ( nr = ny = 0; ny < dims[0]; ny++ )
	  for ( nx = 0; nx < dims[1]; nx++ )
	       rbuff[nr++] = ddr[ny].glr.toa_azim[nx];
     NADC_WR_HDF5_Dataset( compress, grp_id, "TOA Relative Azimuth angle",
			   H5T_NATIVE_FLOAT, 2, dims, rbuff );
     free( rbuff );
/*
 * Satellite Geodetic Height
 */
     rbuff = (float *) malloc( dims[0] * sizeof( float ));
     if ( rbuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );
     for ( ny = 0; ny < dims[0]; ny++ )
          rbuff[ny] = ddr[ny].glr.sat_geo_height;
     NADC_WR_HDF5_Dataset( compress, grp_id, "Satellite Geodetic Height", 
			   H5T_NATIVE_FLOAT, 1, dims, rbuff );
/*
 * Earth Radius
 */
     for ( ny = 0; ny < dims[0]; ny++ )
          rbuff[ny] = ddr[ny].glr.earth_radius;
     NADC_WR_HDF5_Dataset( compress, grp_id, "Earth Radius", 
			   H5T_NATIVE_FLOAT, 1, dims, rbuff );
/*
 * Total Column of Ozone
 */
     for ( ny = 0; ny < dims[0]; ny++ )
          rbuff[ny] = ddr[ny].ozone;
     NADC_WR_HDF5_Dataset( compress, grp_id, "Total Ozone", 
			   H5T_NATIVE_FLOAT, 1, dims, rbuff );
/*
 * Relative error on Total Column
 */ 
     for ( ny = 0; ny < dims[0]; ny++ )
          rbuff[ny] = ddr[ny].error;
     NADC_WR_HDF5_Dataset( compress, grp_id, "Error Ozone", 
			   H5T_NATIVE_FLOAT, 1, dims, rbuff );
     free( rbuff );
/*
 * Latitutes/Longitutes
 */
     dims[0] = nr_ddr; dims[1] = NUM_COORDS; 
     nrpix = dims[0] * dims[1];
     rbuff = (float *) malloc( nrpix * sizeof( float ));
     if ( rbuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );
     for ( nr = ny = 0; ny < (hsize_t) nr_ddr; ny++)
	  for ( nx = 0; nx < NUM_COORDS; nx++ )
	       rbuff[nr++] = ddr[ny].glr.lat[nx];
     NADC_WR_HDF5_Dataset( compress, grp_id, "Latitude",
			   H5T_NATIVE_FLOAT, 2, dims, rbuff );

     for ( nr = ny = 0; ny < (hsize_t) nr_ddr; ny++)
	  for ( nx = 0; nx < NUM_COORDS; nx++ )
	       rbuff[nr++] = ddr[ny].glr.lon[nx];
     NADC_WR_HDF5_Dataset( compress, grp_id, "Longitude",
			   H5T_NATIVE_FLOAT, 2, dims, rbuff );
     free( rbuff );
/*
 * close interface
 */
     (void) H5Gclose( grp_id );
}
