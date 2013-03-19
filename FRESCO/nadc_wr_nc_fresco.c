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

.IDENTifer   NADC_WR_NC_FRESCO
.AUTHOR      R.M. van Hees
.KEYWORDS    FRESCO GOME SCIA
.LANGUAGE    ANSI C
.PURPOSE     write KNMI Fresco product in ADAGUC format
.COMMENTS    contains NADC_FRESCO_WR_NC_META and NADC_FRESCO_WR_NC_REC
.ENVIRONment None
.VERSION     1.0     20-Oct-2008   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>
#include <netcdf.h>

/*+++++ Local Headers +++++*/
#define __NEED_ISO_ENTRIES
#include <nadc_fresco.h>

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
.IDENTifer   NADC_FRESCO_WR_NC_META
.PURPOSE     add header to Fresco product in netCDF-4 format (ADAGUC standard)
.INPUT/OUTPUT
  call as   NADC_FRESCO_WR_NC_META( ncid, instr, hdr );
     input:
            int ncid               :  netCDF file ID
            struct fresco_hdr *hdr :  Fresco header

.RETURNS     Nothing
.COMMENTS    None
-------------------------*/
void NADC_FRESCO_WR_NC_META( int ncid, const struct fresco_hdr *hdr )
{
     const char prognm[] = "NADC_FRESCO_WR_NC_META";

     register unsigned short ni;

     int  retval;
     int  var_id;
/*
 * Product root-data
 */
     for ( ni = 0; ni < numRootKeys; ni++ ) {
	  if ( strcmp(meta_root_list[ni].attr_name, "source") == 0 )
	       meta_root_list[ni].attr_value = hdr->source;
          retval = nc_put_att_text( ncid, NC_GLOBAL,
                                    meta_root_list[ni].attr_name,
                                    strlen(meta_root_list[ni].attr_value)+1,
                                    meta_root_list[ni].attr_value );
          if ( retval != NC_NOERR )
               NADC_RETURN_ERROR(prognm, NADC_ERR_FATAL, nc_strerror(retval));
     }
/*
 * Product meta-data
 */
     retval = nc_def_var( ncid, "product", NC_CHAR, 0, NULL, &var_id );
     if ( retval != NC_NOERR )
          NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
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
	       NADC_RETURN_ERROR(prognm, NADC_ERR_FATAL, nc_strerror(retval));
     }
/*
 * Custom meta-data
 */
     retval = nc_def_var( ncid, "custom", NC_CHAR, 0, NULL, &var_id );
     if ( retval != NC_NOERR )
          NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_put_att_ushort( ncid, var_id, "number_input_products",
                                 NC_USHORT, 1, &hdr->numProd );
     if ( retval != NC_NOERR )
          NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
/*
 * Projection meta-data
 */
     retval = nc_def_var( ncid, "projection", NC_CHAR, 0, NULL, &var_id );
     if ( retval != NC_NOERR )
          NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     for ( ni = 0; ni < numGeoKeys; ni++ ) {
	  retval = nc_put_att_text( ncid, var_id,
				    geo_prod_list[ni].attr_name,
				    strlen(geo_prod_list[ni].attr_value)+1,
				    geo_prod_list[ni].attr_value );
	  if ( retval != NC_NOERR )
	       NADC_RETURN_ERROR(prognm, NADC_ERR_FATAL, nc_strerror(retval));
     }
/*
 * ISO meta-data
 */
     retval = nc_def_var( ncid, "iso_dataset", NC_CHAR, 0, NULL, &var_id );
     if ( retval != NC_NOERR )
          NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     for ( ni = 0; ni < numIsoKeys; ni++ ) {
	  if ( strcmp(iso_prod_list[ni].attr_name, "uid")  == 0 ) {
	       if ( strcmp( hdr->source, "GOME" ) == 0 )
		    retval = nc_put_att_text( ncid, var_id,
					      iso_prod_list[ni].attr_name,
					      strlen(GOME_UUID)+1, GOME_UUID );
	       else
		    retval = nc_put_att_text( ncid, var_id,
					      iso_prod_list[ni].attr_name,
					      strlen(SCIA_UUID)+1, SCIA_UUID );
	  } else if ( strcmp(iso_prod_list[ni].attr_name, "metadata-id")  == 0 ) {
	       if ( strcmp( hdr->source, "GOME" ) == 0 )
		    retval = nc_put_att_text( ncid, var_id,
					      iso_prod_list[ni].attr_name,
					      strlen(GOME_METAID)+1, 
					      GOME_METAID );
	       else
		    retval = nc_put_att_text( ncid, var_id,
					      iso_prod_list[ni].attr_name,
					      strlen(SCIA_METAID)+1, 
					      SCIA_METAID );
	  } else if ( strcmp(iso_prod_list[ni].attr_name, "abstract")  == 0 ) {
	       if ( strcmp( hdr->source, "GOME" ) == 0 )
		    retval = nc_put_att_text( ncid, var_id,
					      iso_prod_list[ni].attr_name,
					      strlen(GOME_Abstract)+1, 
					      GOME_Abstract );
	       else
		    retval = nc_put_att_text( ncid, var_id,
					      iso_prod_list[ni].attr_name,
					      strlen(SCIA_Abstract)+1, 
					      SCIA_Abstract );
	  } else if ( strcmp(iso_prod_list[ni].attr_name, "max-x")  == 0 || 
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
	       NADC_RETURN_ERROR(prognm, NADC_ERR_FATAL, nc_strerror(retval));
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_FRESCO_WR_NC_REC
.PURPOSE     write records to Fresco product in netCDF-4 format 
             (ADAGUC standard)
.INPUT/OUTPUT
  call as   NADC_FRESCO_WR_NC_REC( ncid, instr, numRec, rec );
     input:
            int ncid               :  netCDF file ID
	    char *instr            :  instrument (GOME/SCIA)
            unsigned int numRec    :  number of Fresco records
            struct fresco_rec *rec :  Fresco records

.RETURNS     Nothing
.COMMENTS    None
-------------------------*/
void NADC_FRESCO_WR_NC_REC( int ncid, const char *instr, unsigned int numRec,
			    const struct fresco_rec *rec )
{
     const char prognm[] = "NADC_FRESCO_WR_NC_REC";

     register unsigned int ni, nr;

     int    retval;
     int    time_id, nv_id, dimids[2];
     int    meta_id, var_id;

     float  *rbuff = NULL;
     double *dbuff = NULL;

     struct fresco_meta_rec *mbuff;

     size_t chunk_size[2] = {numRec, NUM_CORNERS};
     
     const size_t nr_byte = NUM_CORNERS * sizeof(float);

     if ( numRec == 0u ) return;
/*
 * write dimension scale "time"
 */
     dbuff = (double *) malloc( numRec * sizeof(double) );
     if ( dbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "dbuff" );

     retval = nc_def_dim( ncid, "time", (size_t) numRec, &time_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     (void) nc_def_var( ncid, "time", NC_DOUBLE, 1, &time_id, &var_id );
     (void) nc_put_att_text( ncid, var_id, "long_name", 4, "time" );
     if ( strncmp( instr, "SCIA", 4 ) == 0 )
	  (void) nc_put_att_text( ncid, var_id, "units", 34, 
				    "days since 2000-01-01 00:00:00 UTC" );
     else if ( strncmp( instr, "GOME", 4 ) == 0 )
	  (void) nc_put_att_text( ncid, var_id, "units", 34, 
				    "days since 1950-01-01 00:00:00 UTC" );
     else
	  (void) nc_put_att_text( ncid, var_id, "units", 7, "UNKNOWN" );
     (void) nc_put_att_text( ncid, var_id, "calendar", 4, "none" );
     for ( nr = 0; nr < numRec; nr++ ) dbuff[nr] = rec[nr].jday;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_double( ncid, var_id, dbuff );

     retval = nc_def_dim( ncid, "nv", NUM_CORNERS, &nv_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
/*
 * write longitude and latitude of measurements
 */
     rbuff = (float *) malloc( numRec * sizeof(float) );
     if ( rbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );

     retval = nc_def_var( ncid, "lon", NC_FLOAT, 1, &time_id, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
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
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_att_text( ncid, var_id, "long_name", 8, "latitude" );
     (void) nc_put_att_text( ncid, var_id, "units", 13, "degrees_north" );
     (void) nc_put_att_text( ncid, var_id, "standard_name", 8, "latitude" );
     (void) nc_put_att_text( ncid, var_id, "bounds", 8, "lat_bnds" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].lat_center;
     (void) nc_put_var_float( ncid, var_id, rbuff );
/*
 * write pixel meta-data as compound dataset
 */
     retval = nc_def_compound( ncid, sizeof(struct fresco_meta_rec),
                               "meta_rec", &meta_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     nc_insert_compound( ncid, meta_id, "integration_time",
                         HOFFSET( struct fresco_meta_rec, intg_time ), 
			 NC_UBYTE );
     nc_insert_compound( ncid, meta_id, "state_id",
                         HOFFSET( struct fresco_meta_rec, stateID ), 
			 NC_UBYTE );
     nc_insert_compound( ncid, meta_id, "pixel_type",
                         HOFFSET( struct fresco_meta_rec, pixelType ),
			 NC_UBYTE );
     nc_insert_compound( ncid, meta_id, "errorFlag",
                         HOFFSET( struct fresco_meta_rec, errorFlag ), 
			 NC_UBYTE );
     nc_insert_compound( ncid, meta_id, "line_of_sight_zenith_angle",
                         HOFFSET( struct fresco_meta_rec, lza ), NC_FLOAT );
     nc_insert_compound( ncid, meta_id, "satellite_zenith_angle",
                         HOFFSET( struct fresco_meta_rec, sza ), NC_FLOAT );
     nc_insert_compound( ncid, meta_id, "relative_azimuth_angle",
                         HOFFSET( struct fresco_meta_rec, raa ), NC_FLOAT );
     nc_insert_compound( ncid, meta_id, "chi-square_of_fit",
                         HOFFSET( struct fresco_meta_rec, chisq ), NC_FLOAT );
     nc_def_var( ncid, "tile_properties", meta_id, 1, &time_id, &var_id );
     (void) nc_put_att_text( ncid, var_id, "long_name", 36,
                               "pixel_properties_and_retrieval_flags" );
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     mbuff = (struct fresco_meta_rec *) 
	  malloc( numRec * sizeof(struct fresco_meta_rec) );
     if ( mbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "mbuff" );
     for ( nr = 0; nr < numRec; nr++ )
	  (void) memcpy( mbuff+nr, &rec[nr].meta, 
			 sizeof(struct fresco_meta_rec) );
     (void) nc_put_var( ncid, var_id, mbuff );
     free( mbuff );
/*
 * write datasets
 */
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "cloudFraction", 
			      "none", "effective cloud fraction", 
			      "cloud_area_fraction", "cloudFractionError" );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "cloudFraction" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].cloudFraction;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_float( ncid, var_id, rbuff );
     /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "cloudFractionError", 
			      "none", "error of effective cloud fraction", 
			      "cloud_area_fraction standard_error", NULL );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "cloudFractionError" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].cloudFractionError;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_float( ncid, var_id, rbuff );
     /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "cloudTopHeight", 
			      "m", "cloud height", "cloud_top_altitude", NULL );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "cloudTopHeight" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].cloudTopHeight;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_float( ncid, var_id, rbuff );
    /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "cloudTopPress", 
			      "hPa", "cloud pressure", 
			      "air_pressure_at_cloud_top", 
			      "cloudTopPressError" );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "cloudTopPress" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].cloudTopPress;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_float( ncid, var_id, rbuff );
     /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "cloudTopPressError", 
			      "hPa", "error of cloud pressure", 
			      "air_pressure_at_cloud_top standard_error",
			      NULL );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "cloudTopPressError" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].cloudTopPressError;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_float( ncid, var_id, rbuff );
      /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "cloudAlbedo", 
			      "none", "cloud albedo", 
			      "cloud_albedo", "cloudAlbedoError" );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "cloudAlbedo" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].cloudAlbedo;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_float( ncid, var_id, rbuff );
      /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "cloudAlbedoError", 
			      "none", "error of cloud albedo", 
			      "cloud_albedo standard_error", NULL );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "cloudAlbedoError" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].cloudAlbedoError;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_float( ncid, var_id, rbuff );
     /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "surfaceAlbedo", 
			      "none", "wavelength averaged surface albedo", 
			      "surface_albedo", NULL );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "surfaceAlbedo" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].surfaceAlbedo;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_float( ncid, var_id, rbuff );
     /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "surfaceHeight", 
			      "m", "surface heigth", "surface_altitude", NULL );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "surfaceHeight" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].surfaceHeight;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_float( ncid, var_id, rbuff );
     /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "groundPress", 
			      "Pa", "assumed surface pressure", 
			      "surface_air_pressure", NULL );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "groundPress" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].groundPress;
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     (void) nc_put_var_float( ncid, var_id, rbuff );
/*
 * write longitude and latitude of tile-corners
 */
     rbuff = (float *) realloc( rbuff, NUM_CORNERS * numRec * sizeof(float) );
     if ( rbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );

     dimids[0] = time_id;
     dimids[1] = nv_id;
     retval = nc_def_var( ncid, "lon_bnds", NC_FLOAT, 2, dimids, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     for ( ni = nr = 0; nr < numRec; nr++, ni += NUM_CORNERS )
          (void) memcpy( rbuff+ni, rec[nr].lon_corner, nr_byte );
     (void) nc_put_var_float( ncid, var_id, rbuff );

     retval = nc_def_var( ncid, "lat_bnds", NC_FLOAT, 2, dimids, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     (void) nc_def_var_chunking( ncid, var_id, 0, chunk_size );
     (void) nc_def_var_deflate( ncid, var_id, 0, 1, 6 );
     for ( ni = nr = 0; nr < numRec; nr++, ni += NUM_CORNERS )
          (void) memcpy( rbuff+ni, rec[nr].lat_corner, nr_byte );
     (void) nc_put_var_float( ncid, var_id, rbuff );

 done:
     if ( rbuff != NULL ) free( rbuff );
     if ( dbuff != NULL ) free( dbuff );
}
