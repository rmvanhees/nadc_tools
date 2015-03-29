/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2008 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_WR_NC_TOGOMI
.AUTHOR      R.M. van Hees
.KEYWORDS    TOGOMI GOME
.LANGUAGE    ANSI C
.PURPOSE     write TOGOMI product in ADAGUC format
.COMMENTS    contains NADC_TOGOMI_WR_NC_META and NADC_TOGOMI_WR_NC_REC
.ENVIRONment None
.VERSION     1.1     30-Oct-2008   removed check on longitudes, RvH
             1.0     08-Oct-2008   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

#ifdef _WITH_NC4
#include <hdf5.h>
#include <netcdf.h>

/*+++++ Local Headers +++++*/
#define __NEED_ISO_ENTRIES
#include <nadc_togomi.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include <_adaguc_def_var.inc>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   NADC_TOGOMI_WR_NC_META
.PURPOSE     add header to TOGOMI product in netCDF-4 format (ADAGUC standard)
.INPUT/OUTPUT
  call as   NADC_TOGOMI_WR_NC_META( ncid, hdr );
     input:
            int ncid               :  netCDF file ID
            struct togomi_hdr *hdr :  TOGOMI header

.RETURNS     Nothing
.COMMENTS    None
-------------------------*/
void NADC_TOGOMI_WR_NC_META( int ncid, const struct togomi_hdr *hdr )
{
     register unsigned short ni;

     int  retval;
     int  var_id;
/*
 * Product root-data
 */
     for ( ni = 0; ni < numRootKeys; ni++ ) {
          retval = nc_put_att_text( ncid, NC_GLOBAL,
                                    meta_root_list[ni].attr_name,
                                    strlen(meta_root_list[ni].attr_value)+1,
                                    meta_root_list[ni].attr_value );
          if ( retval != NC_NOERR )
               NADC_RETURN_ERROR(NADC_ERR_FATAL, nc_strerror(retval));
     }
/*
 * Product meta-data
 */
     retval = nc_def_var( ncid, "product", NC_CHAR, 0, NULL, &var_id );
     if ( retval != NC_NOERR )
          NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     for ( ni = 0; ni < numProdKeys; ni++ ) {
	  if ( strcmp(meta_prod_list[ni].attr_name, "input_products")  == 0 )
	       meta_prod_list[ni].attr_value = hdr->l1b_product;
	  else if ( strcmp(meta_prod_list[ni].attr_name, "creation_date")  == 0 )
	       meta_prod_list[ni].attr_value = hdr->creation_date;
	  else if ( strcmp(meta_prod_list[ni].attr_name, "software_version")  == 0 )
	       meta_prod_list[ni].attr_value = hdr->software_version;
	  else if ( strcmp(meta_prod_list[ni].attr_name, "validity_start")  == 0 )
	       meta_prod_list[ni].attr_value = hdr->validity_start;
	  else if ( strcmp(meta_prod_list[ni].attr_name, "validity_stop")  == 0 )
	       meta_prod_list[ni].attr_value = hdr->validity_stop;

	  retval = nc_put_att_text( ncid, var_id,
				    meta_prod_list[ni].attr_name,
				    strlen(meta_prod_list[ni].attr_value)+1,
				    meta_prod_list[ni].attr_value );
	  if ( retval != NC_NOERR )
	       NADC_RETURN_ERROR(NADC_ERR_FATAL, nc_strerror(retval));
     }
/*
 * Custom meta-data
 */
     retval = nc_def_var( ncid, "custom", NC_CHAR, 0, NULL, &var_id );
     if ( retval != NC_NOERR )
          NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_put_att_ushort( ncid, var_id, "number_input_products", 
				 NC_USHORT, 1, &hdr->numProd );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
/*
 * Projection meta-data
 */
     retval = nc_def_var( ncid, "projection", NC_CHAR, 0, NULL, &var_id );
     if ( retval != NC_NOERR )
          NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     for ( ni = 0; ni < numGeoKeys; ni++ ) {
	  retval = nc_put_att_text( ncid, var_id,
				    geo_prod_list[ni].attr_name,
				    strlen(geo_prod_list[ni].attr_value)+1,
				    geo_prod_list[ni].attr_value );
	  if ( retval != NC_NOERR )
	       NADC_RETURN_ERROR(NADC_ERR_FATAL, nc_strerror(retval));
     }
/*
 * ISO meta-data
 */
     retval = nc_def_var( ncid, "iso_dataset", NC_CHAR, 0, NULL, &var_id );
     if ( retval != NC_NOERR )
          NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     for ( ni = 0; ni < numIsoKeys; ni++ ) {
	  if ( strcmp(iso_prod_list[ni].attr_name, "max-x")  == 0 || 
	       strcmp(iso_prod_list[ni].attr_name, "min-x")  == 0 ||
	       strcmp(iso_prod_list[ni].attr_name, "max-y")  == 0 ||
	       strcmp(iso_prod_list[ni].attr_name, "min-y")  == 0 ) {
	       /* these 4 vars should we written out as NC floats */
	       float fValue = atof(iso_prod_list[ni].attr_value);
	       retval = nc_put_att_float( ncid, var_id,
					  iso_prod_list[ni].attr_name, 
					  NC_FLOAT, 1, &fValue );
	  } else {
	       retval = nc_put_att_text( ncid, var_id,
					 iso_prod_list[ni].attr_name,
					 strlen(iso_prod_list[ni].attr_value)+1,
					 iso_prod_list[ni].attr_value );
	  }
	  if ( retval != NC_NOERR )
	       NADC_RETURN_ERROR(NADC_ERR_FATAL, nc_strerror(retval));
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_TOGOMI_WR_NC_REC
.PURPOSE     write records to TOGOMI product in netCDF-4 format 
             (ADAGUC standard)
.INPUT/OUTPUT
  call as   NADC_TOGOMI_WR_NC_REC( ncid, numRec, rec );
     input:
            int ncid               :   netCDF file ID
            unsigned int numRec    :   number of TOGOMI records
            struct togomi_rec *rec :   TOGOMI records

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
void NADC_TOGOMI_WR_NC_REC( int ncid, unsigned int numRec,
			    const struct togomi_rec *rec )
{
     register unsigned int ni, nr;

     int    retval;
     int    time_id, nv_id, dimids[2];
     int    meta_id, var_id;

     float  *rbuff = NULL;
     double *dbuff = NULL;

     struct togomi_meta_rec *mbuff;

     size_t chunk_size[2] = {numRec, NUM_CORNERS};
     
     const size_t nr_byte = NUM_CORNERS * sizeof(float);

     if ( numRec == 0 ) return;
/*
 * write dimension scale "time"
 */
     dbuff = (double *) malloc( numRec * sizeof(double) );
     if ( dbuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "dbuff" );

     retval = nc_def_dim( ncid, "time", (size_t) numRec, &time_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     (void) nc_def_var( ncid, "time", NC_DOUBLE, 1, &time_id, &var_id );
     (void) nc_put_att_text( ncid, var_id, "long_name", 4, "time" );
     (void) nc_put_att_text( ncid, var_id, "units", 34, 
			     "days since 2000-01-01 00:00:00 UTC" );
     (void) nc_put_att_text( ncid, var_id, "calendar", 4, "none" );
     for ( nr = 0; nr < numRec; nr++ ) dbuff[nr] = rec[nr].jday;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_double( ncid, var_id, dbuff );

     retval = nc_def_dim( ncid, "nv", NUM_CORNERS, &nv_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
/*
 * write longitude and latitude of measurements
 */
     rbuff = (float *) malloc( numRec * sizeof(float) );
     if ( rbuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );

     retval = nc_def_var( ncid, "lon", NC_FLOAT, 1, &time_id, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_att_text( ncid, var_id, "long_name", 9, "longitude" );
     (void) nc_put_att_text( ncid, var_id, "units", 12, "degrees_east" );
     (void) nc_put_att_text( ncid, var_id, "standard_name", 9, "longitude" );
     (void) nc_put_att_text( ncid, var_id, "bounds", 8, "lon_bnds" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].lon_center;
     (void) nc_put_var_float( ncid, var_id, rbuff );

     retval = nc_def_var( ncid, "lat", NC_FLOAT, 1, &time_id, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_att_text( ncid, var_id, "long_name", 8, "latitude" );
     (void) nc_put_att_text( ncid, var_id, "units", 13, "degrees_north" );
     (void) nc_put_att_text( ncid, var_id, "standard_name", 8, "latitude" );
     (void) nc_put_att_text( ncid, var_id, "bounds", 8, "lat_bnds" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].lat_center;
     (void) nc_put_var_float( ncid, var_id, rbuff );
/*
 * write datasets
 */
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "vcd", "Dobson unit", 
			     "vertical ozone column density",
			     "total_vertical_ozone_column",
			     "vcdError" );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "vcd" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].vcd;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_float( ncid, var_id, rbuff );
     /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "vcdError", "Dobson unit",
			     "uncertainty in vertical ozone column",
			     "total_vertical_ozone_column standard_error",
			     NULL );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "vcdError" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].vcdError;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_float( ncid, var_id, rbuff );
      /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "scd", "Dobson unit", 
			     "slant ozone column density",
			     "total_slant_ozone_column",
			     "scdError" );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "scd" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].scd;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_float( ncid, var_id, rbuff );
      /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "scdError", "Dobson unit",
			     "uncertainty in slant ozone column",
			     "total_slant_ozone_column standard_error",
			     NULL );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "scdError" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].scdError;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_float( ncid, var_id, rbuff );
/*
 * write longitude and latitude of tile-corners
 */
     rbuff = (float *) realloc( rbuff, NUM_CORNERS * numRec * sizeof(float) );
     if ( rbuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );

     dimids[0] = time_id;
     dimids[1] = nv_id;
     retval = nc_def_var( ncid, "lon_bnds", NC_FLOAT, 2, dimids, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     for ( ni = nr = 0; nr < numRec; nr++, ni += NUM_CORNERS )
          (void) memcpy( rbuff+ni, rec[nr].lon_corner, nr_byte );
     (void) nc_put_var_float( ncid, var_id, rbuff );

     retval = nc_def_var( ncid, "lat_bnds", NC_FLOAT, 2, dimids, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     for ( ni = nr = 0; nr < numRec; nr++, ni += NUM_CORNERS )
          (void) memcpy( rbuff+ni, rec[nr].lat_corner, nr_byte );
     (void) nc_put_var_float( ncid, var_id, rbuff );
/*
 * write pixel meta-data as compound dataset
 */
     retval = nc_def_compound( ncid, sizeof(struct togomi_meta_rec),
                               "meta_rec", &meta_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     (void) nc_insert_compound( ncid, meta_id, "integration_time",
				HOFFSET( struct togomi_meta_rec, intg_time ), 
				NC_UBYTE );
     (void) nc_insert_compound( ncid, meta_id, "state_id",
				HOFFSET( struct togomi_meta_rec, stateID ), 
				NC_UBYTE );
     (void) nc_insert_compound( ncid, meta_id, "pixel_type",
				HOFFSET( struct togomi_meta_rec, pixelType ),
				NC_UBYTE );
     (void) nc_insert_compound( ncid, meta_id, "swath_type",
				HOFFSET( struct togomi_meta_rec, swathType ), 
				NC_UBYTE );
     (void) nc_insert_compound( ncid, meta_id, "solar_zenith_angle",
				HOFFSET( struct togomi_meta_rec, sza ), 
				NC_FLOAT );
     (void) nc_insert_compound( ncid, meta_id, "effective_ozone_temp",
				HOFFSET( struct togomi_meta_rec, effTemp ), 
				NC_FLOAT );
     (void) nc_insert_compound( ncid, meta_id, "ghost_ozone_column",
				HOFFSET( struct togomi_meta_rec, ghostColumn ),
				NC_FLOAT );
     (void) nc_insert_compound( ncid, meta_id, "cloud_fraction",
				HOFFSET(struct togomi_meta_rec, cloudFraction),
				NC_FLOAT );
     (void) nc_insert_compound( ncid, meta_id, "cloud_top_height",
				HOFFSET( struct togomi_meta_rec, cloudHeight ),
				NC_FLOAT );
     (void) nc_insert_compound( ncid, meta_id, "clear_sky_air_mass_factor",
				HOFFSET( struct togomi_meta_rec, amfSky ), 
				NC_FLOAT );
     (void) nc_insert_compound( ncid, meta_id, "cloud_top_air_mass_factor",
				HOFFSET( struct togomi_meta_rec, amfCloud ),
				NC_FLOAT );
     retval = nc_def_var( ncid, "tile_properties", 
			  meta_id, 1, &time_id, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     (void) nc_put_att_text( ncid, var_id, "long_name", 36,
			     "pixel properties and retrieval flags" );
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     mbuff = (struct togomi_meta_rec *) 
	  malloc( numRec * sizeof(struct togomi_meta_rec) );
     if ( mbuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "mbuff" );
     for ( nr = 0; nr < numRec; nr++ )
	  (void) memcpy( mbuff+nr, &rec[nr].meta, 
			 sizeof(struct togomi_meta_rec) );
     retval = nc_put_var( ncid, var_id, mbuff );
     free( mbuff );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
 done:
     if ( rbuff != NULL ) free( rbuff );
     if ( dbuff != NULL ) free( dbuff );
}
#endif   /* _WITH_NC4 */
