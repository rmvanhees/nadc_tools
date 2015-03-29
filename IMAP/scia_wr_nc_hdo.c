/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2011 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_WR_NC_HDO
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA IMAP HDO
.LANGUAGE    ANSI C
.PURPOSE     write IMAP-HDO product in ADAGUC format
.COMMENTS    contains SCIA_WR_NC_HDO_META and SCIA_WR_NC_HDO_REC
.ENVIRONment None
.VERSION     1.0     28-Apr-2011   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>
#include <netcdf.h>

/*+++++ Local Headers +++++*/
#define __IMAP_HDO_PRODUCT
#define __NEED_ISO_ENTRIES
#include <nadc_imap.h>

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
.IDENTifer   SCIA_WR_NC_HDO_META
.PURPOSE     add header to IMAP-HDO product in netCDF-4 (ADAGUC standard)
.INPUT/OUTPUT
  call as   SCIA_WR_NC_HDO_META( ncid, hdr );
     input:
            int ncid             :  netCDF file ID
	    struct imap_hdr *hdr :  IMAP header

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
void SCIA_WR_NC_HDO_META( int ncid, const struct imap_hdr *hdr )
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
               NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
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
	  else if ( strcmp(meta_prod_list[ni].attr_name, "validity_start")  == 0 )
	       meta_prod_list[ni].attr_value = hdr->validity_start;
	  else if ( strcmp(meta_prod_list[ni].attr_name, "validity_stop")  == 0 )
	       meta_prod_list[ni].attr_value = hdr->validity_stop;
	  else if ( strcmp(meta_prod_list[ni].attr_name, "software_version")  == 0 )
	       meta_prod_list[ni].attr_value = hdr->software_version;

	  retval = nc_put_att_text( ncid, var_id,
				    meta_prod_list[ni].attr_name,
				    strlen(meta_prod_list[ni].attr_value)+1,
				    meta_prod_list[ni].attr_value );
	  if ( retval != NC_NOERR )
	       NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     }
/*
 * Custom meta-data
 */
     retval = nc_def_var( ncid, "custom", NC_CHAR, 0, NULL, &var_id );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_put_att_ushort( ncid, var_id, "number_input_products", 
				 NC_INT, 1, &hdr->numProd );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_put_att_ushort( ncid, var_id, "file_counter", 
				 NC_USHORT, hdr->numProd, hdr->counter );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_put_att_uint( ncid, var_id, "abs_orbit", 
			       NC_UINT, hdr->numProd, hdr->orbit );
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
	       NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
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
	       NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_WR_NC_HDO_REC
.PURPOSE     write records to IMAP-HDO product in netCDF-4 (ADAGUC standard)
.INPUT/OUTPUT
  call as   SCIA_WR_NC_HDO_REC( ncid, numRec, rec );
     input:
            int ncid              :  netCDF file ID
	    unsigned int numRec   :  number of IMAP records
	    struct imap_rec *rec  :  IMAP records

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
void SCIA_WR_NC_HDO_REC( int ncid, unsigned int numRec,
			 const struct imap_rec *rec )
{
     register unsigned int ni, nr;

     int    retval;
     int    time_id, nv_id, dimids[2];
     int    meta_id, var_id;
     size_t indx;

     float  *rbuff = NULL;
     double *dbuff = NULL;
     
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
     if ( (retval = nc_put_var_double( ncid, var_id, dbuff )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );

     if ( (retval = nc_def_dim( ncid, "nv", NUM_CORNERS, &nv_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
/*
 * write longitude and latitude of measurements
 */
     rbuff = (float *) malloc( numRec * sizeof(float) );
     if ( rbuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );

     retval = nc_def_var( ncid, "lon", NC_FLOAT, 1, &time_id, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     (void) nc_put_att_text( ncid, var_id, "long_name", 9, "longitude" );
     (void) nc_put_att_text( ncid, var_id, "units", 12, "degrees_east" );
     (void) nc_put_att_text( ncid, var_id, "standard_name", 9, "longitude" );
     (void) nc_put_att_text( ncid, var_id, "bounds", 8, "lon_bnds" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].lon_center;
     retval = nc_put_var_float( ncid, var_id, rbuff );

     retval = nc_def_var( ncid, "lat", NC_FLOAT, 1, &time_id, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     (void) nc_put_att_text( ncid, var_id, "long_name", 8, "latitude" );
     (void) nc_put_att_text( ncid, var_id, "units", 13, "degrees_north" );
     (void) nc_put_att_text( ncid, var_id, "standard_name", 8, "latitude" );
     (void) nc_put_att_text( ncid, var_id, "bounds", 8, "lat_bnds" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].lat_center;
     if ( (retval = nc_put_var_float( ncid, var_id, rbuff )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
/*
 * write pixel meta-data as compound dataset
 */
     retval = nc_def_compound( ncid, sizeof(struct imap_meta_rec), 
			       "meta_rec", &meta_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     (void) nc_insert_compound( ncid, meta_id, "state_id",
			 HOFFSET(struct imap_meta_rec, stateID), NC_UBYTE );
     (void) nc_insert_compound( ncid, meta_id, "backscan_flag",
			 HOFFSET(struct imap_meta_rec, bs), NC_UBYTE );
     (void) nc_insert_compound( ncid, meta_id, "pixels_fresco",
			 HOFFSET(struct imap_meta_rec, pixels_fresco), 
				NC_USHORT);
     (void) nc_insert_compound( ncid, meta_id, "integration_time",
                         HOFFSET(struct imap_meta_rec, intg_time ), NC_FLOAT );
     (void) nc_insert_compound( ncid, meta_id, "solar_zenith_angle",
			 HOFFSET(struct imap_meta_rec, sza), NC_FLOAT );
     (void) nc_insert_compound( ncid, meta_id, "line_of_sight_zenith_angle",
			 HOFFSET(struct imap_meta_rec, lza), NC_FLOAT );
     (void) nc_insert_compound( ncid, meta_id, "surface_elevation",
			 HOFFSET(struct imap_meta_rec, elev), NC_FLOAT );
     (void) nc_insert_compound( ncid, meta_id, "rms_residual",
			 HOFFSET(struct imap_meta_rec, rms_res), NC_FLOAT );
     (void) nc_insert_compound( ncid, meta_id, "rms_residual_weighted",
			 HOFFSET(struct imap_meta_rec, rms_res_wght), 
				NC_FLOAT );
     (void) nc_insert_compound( ncid, meta_id, "pixel_one_readout",
			 HOFFSET(struct imap_meta_rec, bu), NC_FLOAT );
     (void) nc_insert_compound( ncid, meta_id, "cloud_fraction",
			 HOFFSET(struct imap_meta_rec, cl_frac), NC_FLOAT );
     (void) nc_insert_compound( ncid, meta_id, "cloud_top_pressure",
			 HOFFSET(struct imap_meta_rec, cl_press), NC_FLOAT );
     (void) nc_insert_compound( ncid, meta_id, "ocean_fraction",
			 HOFFSET(struct imap_meta_rec, ocean_frac), NC_FLOAT );
     retval = nc_def_var( ncid, "tile_properties", 
			  meta_id, 1, &time_id, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     (void) nc_put_att_text( ncid, var_id, "long_name", 36, 
			       "pixel_properties_and_retrieval_flags" );
     for ( indx = 0; indx < (size_t) numRec; indx++ ) {
	  retval = nc_put_var1( ncid, var_id, &indx, &rec[indx].meta );
	  if ( retval != NC_NOERR )
	       NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     }
/*
 * write datasets (HDO, CO2)
 */
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "HDO", "cm-2",
			     "vertical column number density of (HDO)",
			     "atmosphere_number_content_of_hdo_in_air",
			     "HDO_error" );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "HDO" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].hdo_vcd;
     if ( (retval = nc_put_var_float( ncid, var_id, rbuff )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "HDO_error", "cm-2",
			     "vertical column number density of HDO (Error)",
		    "atmosphere_number_content_of_hdo_in_air standard_error",
			     NULL );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "HDO_error" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].hdo_error;
     if ( (retval = nc_put_var_float( ncid, var_id, rbuff )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "H2O", "cm-2",
			     "vertical column number density of water", 
			   "atmosphere_number_content_of_water_in_air",
			     "H2O_error H2O_model" );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "H2O" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].h2o_vcd;
     if ( (retval = nc_put_var_float( ncid, var_id, rbuff )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "H2O_error", "cm-2",
			"vertical column number density of water (Error)", 
	       "atmosphere_number_content_of_water_in_air standard_error",
			     NULL );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "H2O_error" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].h2o_error;
     if ( (retval = nc_put_var_float( ncid, var_id, rbuff )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "H2O_model", "cm-2",
			"vertical column number density of water (ECMWF)", 
			"atmosphere_number_content_of_water_in_air model",
			     NULL );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "H2O_model" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].h2o_model;
     if ( (retval = nc_put_var_float( ncid, var_id, rbuff )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "delta_d", "per mil",
			     "delta D", "delta D", "delta_d_error" );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "delta_d" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].delta_d;
     if ( (retval = nc_put_var_float( ncid, var_id, rbuff )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "delta_d_error", 
			     "per mil", "delta D (error)", "delta D", NULL );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( NADC_ERR_HDF_WR, "delta_d_error" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].delta_d_error;
     if ( (retval = nc_put_var_float( ncid, var_id, rbuff )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
/*
 * write longitude and latitude bounding boxes
 */
     rbuff = (float *) realloc( rbuff, NUM_CORNERS * numRec * sizeof(float) );
     if ( rbuff == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "rbuff" );

     dimids[0] = time_id;
     dimids[1] = nv_id;
     retval = nc_def_var( ncid, "lon_bnds", NC_FLOAT, 2, dimids, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     for ( ni = nr = 0; nr < numRec; nr++, ni += NUM_CORNERS )
	  (void) memcpy( rbuff+ni, rec[nr].lon_corner, nr_byte );
     if ( (retval = nc_put_var_float( ncid, var_id, rbuff )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );

     retval = nc_def_var( ncid, "lat_bnds", NC_FLOAT, 2, dimids, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     for ( ni = nr = 0; nr < numRec; nr++, ni += NUM_CORNERS )
	  (void) memcpy( rbuff+ni, rec[nr].lat_corner, nr_byte );
     if ( (retval = nc_put_var_float( ncid, var_id, rbuff )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
 done:
     if ( rbuff != NULL ) free( rbuff );
     if ( dbuff != NULL ) free( dbuff );
}
