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

.IDENTifer   SCIA_WR_NC_H2O
.AUTHOR      R.M. van Hees
.KEYWORDS    SRON IMLM Sciamachy
.LANGUAGE    ANSI C
.PURPOSE     write IMLM-H2O product in ADAGUC format
.COMMENTS    contains SCIA_WR_NC_H2O_META and SCIA_WR_NC_H2O_REC
.ENVIRONment None
.VERSION     1.0     12-Apr-2011   initial release by R. M. van Hees
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
#define __IMLM_H2O_PRODUCT
#define __NEED_ISO_ENTRIES
#include <nadc_imlm.h>

/*+++++ Macros +++++*/
#define H2O_COMMENT "TBD"

#define H2O_ERR_COMMENT "The H2O error is an instrument-noise related error"\
     " and does not include a fit error"

#define CH4_COMMENT "CH4 columns are only given for sea pixels and denotes"\
     " a partial column above the cloud. This column may be used as a cloud"\
     " top height estimator as described by Gloudemans et al., Atmos. Chem."\
     " Phys., 9, 3799-3813, 2009]. The quality of the CH4 columns is"\
     " however not good enough for investigating sources and sinks of CH4"

#define CH4_ERR_COMMENT "The CH4 error is an instrument-noise related error"\
     " and does not include a fit error"

#define ELEV_COMMENT "A mean Elevation less than 1.e-3 denotes a sea pixel."\
     " In that case only the H2O column above the cloud is given. The"\
     " corresponding CH4 column can be used to estimate the cloud top"\
     " height [cf. Gloudemans et al., Atmos. Chem. Phys., 9, 3799-3813, 2009]"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include <_adaguc_def_var.inc>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_WR_NC_H2O_META
.PURPOSE     add header to IMLM-H2O product in netCDF-4 (ADAGUC standard)
.INPUT/OUTPUT
  call as   SCIA_WR_NC_H2O_META( ncid, hdr );
     input:
            int ncid             :  netCDF file ID
	    struct imlm_hdr *hdr :  IMLM header

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
void SCIA_WR_NC_H2O_META( int ncid, const struct imlm_hdr *hdr )
{
     const char prognm[] = "SCIA_WR_NC_H2O_META";

     register unsigned short ni;

     char cbuff[SHORT_STRING_LENGTH];
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
	       NADC_RETURN_ERROR(prognm, NADC_ERR_FATAL, nc_strerror(retval));
     }
/*
 * Custom meta-data
 */
     retval = nc_def_var( ncid, "custom", NC_CHAR, 0, NULL, &var_id );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_put_att_ushort( ncid, var_id, "number_input_products", 
				 NC_INT, 1, &hdr->numProd );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_put_att_uint( ncid, var_id, "abs_orbit", 
			       NC_UINT, hdr->numProd, hdr->orbit );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_put_att_text( ncid, var_id, "pixelmask_version", 
			       strlen(hdr->pixelmask_version)+1, 
			       hdr->pixelmask_version );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_put_att_text( ncid, var_id, "cloudmask_version", 
			       strlen(hdr->cloudmask_version)+1, 
			       hdr->cloudmask_version );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_put_att_uchar( ncid, var_id, "Sciamachy_channel", 
				NC_INT, 1, &hdr->scia_channel );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     (void) snprintf( cbuff, SHORT_STRING_LENGTH, "%hu - %-hu", 
		      hdr->window_pixel[0], hdr->window_pixel[1] );
     retval = nc_put_att_text( ncid, var_id, "window_pixels", 
			       strlen(cbuff)+1, cbuff );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     (void) snprintf( cbuff, SHORT_STRING_LENGTH, "%-.2f - %-.2f", 
		      hdr->window_wave[0], hdr->window_wave[1] );
     retval = nc_put_att_text( ncid, var_id, "window_wave", 
			       strlen(cbuff)+1, cbuff );
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
	       NADC_RETURN_ERROR(prognm, NADC_ERR_FATAL, nc_strerror(retval));
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_WR_NC_H2O_REC
.PURPOSE     write records to IMLM-H2O product in netCDF-4 (ADAGUC standard)
.INPUT/OUTPUT
  call as   SCIA_WR_NC_H2O_REC( ncid, numRec, rec );
     input:
            int ncid              :  netCDF file ID
	    unsigned int numRec   :  number of IMLM records
	    struct imlm_rec *rec  :  IMLM records

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
void SCIA_WR_NC_H2O_REC( int ncid, unsigned int numRec,
			  const struct imlm_rec *rec )
{
     const char prognm[] = "SCIA_WR_NC_H2O_REC";

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
     if ( dbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "dbuff" );

     retval = nc_def_dim( ncid, "time", (size_t) numRec, &time_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_def_var( ncid, "time", NC_DOUBLE, 1, &time_id, &var_id );
     retval = nc_put_att_text( ncid, var_id, "long_name", 4, "time" );
     retval = nc_put_att_text( ncid, var_id, "units", 34, 
			       "days since 2000-01-01 00:00:00 UTC" );
     retval = nc_put_att_text( ncid, var_id, "calendar", 4, "none" );
     for ( nr = 0; nr < numRec; nr++ ) dbuff[nr] = rec[nr].dsr_time;
     retval = nc_put_var_double( ncid, var_id, dbuff );

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
     retval = nc_put_att_text( ncid, var_id, "long_name", 9, "longitude" );
     retval = nc_put_att_text( ncid, var_id, "standard_name", 9, "longitude" );
     retval = nc_put_att_text( ncid, var_id, "units", 12, "degrees_east" );
     (void) nc_put_att_text( ncid, var_id, "bounds", 8, "lon_bnds" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].lon_center;
     retval = nc_put_var_float( ncid, var_id, rbuff );

     retval = nc_def_var( ncid, "lat", NC_FLOAT, 1, &time_id, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_put_att_text( ncid, var_id, "long_name", 8, "latitude" );
     retval = nc_put_att_text( ncid, var_id, "standard_name", 8, "latitude" );
     retval = nc_put_att_text( ncid, var_id, "units", 13, "degrees_north" );
     (void) nc_put_att_text( ncid, var_id, "bounds", 8, "lat_bnds" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].lat_center;
     retval = nc_put_var_float( ncid, var_id, rbuff );
/*
 * write pixel meta-data as compound dataset
 */
     retval = nc_def_compound( ncid, sizeof(struct imlm_meta_rec), 
			       "meta_rec", &meta_id );
     nc_insert_compound( ncid, meta_id, "integration_time",
			 HOFFSET(struct imlm_meta_rec, intg_time ), NC_FLOAT );
     nc_insert_compound( ncid, meta_id, "satellite_zenith_angle",
			 HOFFSET( struct imlm_meta_rec, sza ), NC_FLOAT );
     nc_insert_compound( ncid, meta_id, "line_of_sight_zenith_angle",
			 HOFFSET( struct imlm_meta_rec, lza ), NC_FLOAT );
     nc_insert_compound( ncid, meta_id, "chi-square_of_fit",
			 HOFFSET( struct imlm_meta_rec, chisq ), NC_FLOAT );
     nc_insert_compound( ncid, meta_id, "degrees_of_freedom",
			 HOFFSET( struct imlm_meta_rec, dof ), NC_USHORT );
     nc_insert_compound( ncid, meta_id, "retrieval_error_flag",
			 HOFFSET( struct imlm_meta_rec, eflag ), NC_USHORT );
     nc_insert_compound( ncid, meta_id, "ordinal_number_of_state",
			 HOFFSET( struct imlm_meta_rec, st ), NC_UBYTE );
     nc_insert_compound( ncid, meta_id, "backscan_flag",
			 HOFFSET( struct imlm_meta_rec, bs ), NC_UBYTE );
     nc_insert_compound( ncid, meta_id, "pixel_number",
			 HOFFSET( struct imlm_meta_rec, px ), NC_USHORT );
     nc_def_var( ncid, "tile_properties", meta_id, 1, &time_id, &var_id );
     retval = nc_put_att_text( ncid, var_id, "long_name", 36, 
			       "pixel_properties_and_retrieval_flags" );
     for ( indx = 0; indx < (size_t) numRec; indx++ ) {
	  retval = nc_put_var1( ncid, var_id, &indx, &rec[indx].meta );
     }
/*
 * write datasets (H2O, CH4)
 */
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "H2O", "molecules/cm2",
			     "vertical column density of H2O",
			     "atmosphere_number_content_of_water_in_air",
			     "H2O_error" );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "H2O" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].H2O;
     retval = nc_put_var_float( ncid, var_id, rbuff );
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "H2O_error", "molecules/cm2",
			     "error of vertical column density of H2O",
		    "atmosphere_number_content_of_water_in_air standard_error",
			     NULL );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "H2O_error" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].H2O_err;
     retval = nc_put_var_float( ncid, var_id, rbuff );

     /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "CH4", "molecules/cm2",
			     "vertical column density of CH4",
			     "atmosphere_number_content_of_methane_in_air",
			     "CH4_error" );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "CH4" );
     retval = nc_put_att_text( ncid, var_id, "comment",
			       strlen(CH4_COMMENT), CH4_COMMENT );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].CH4;
     retval = nc_put_var_float( ncid, var_id, rbuff );
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "CH4_error", 
			     "molecules/cm2",
			     "error of vertical column density of CH4",
		  "atmosphere_number_content_of_methane_in_air standard_error",
			     NULL );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "CH4_error" );
     retval = nc_put_att_text( ncid, var_id, "comment",
			       strlen(CH4_ERR_COMMENT), CH4_ERR_COMMENT );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].CH4_err;
     retval = nc_put_var_float( ncid, var_id, rbuff );

     /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "albedo", NULL,
			     "albedo", "surface_albedo", NULL );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "albedo" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].albedo;
     retval = nc_put_var_float( ncid, var_id, rbuff );

     /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "cloudFraction", NULL,
			     "cloud fraction", "cloud_area_fraction", NULL );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "cloudFraction" );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].cl_fr;
     retval = nc_put_var_float( ncid, var_id, rbuff );

     /*+++++++++++++++++++++++++*/
     var_id = ADAGUC_DEF_VAR( ncid, NC_FLOAT, time_id, "meanElevation", "m",
			     "mean elevation", "surface_altitude", NULL );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "meanElevation" );
     retval = nc_put_att_text( ncid, var_id, "comment",
			       strlen(ELEV_COMMENT), ELEV_COMMENT );
     for ( nr = 0; nr < numRec; nr++ ) rbuff[nr] = rec[nr].mean_elev;
     retval = nc_put_var_float( ncid, var_id, rbuff );
/*
 * write longitude and latitude bounding boxes
 */
     rbuff = (float *) realloc( rbuff, NUM_CORNERS * numRec * sizeof(float) );
     if ( rbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );

     dimids[0] = time_id;
     dimids[1] = nv_id;
     retval = nc_def_var( ncid, "lon_bnds", NC_FLOAT, 2, dimids, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     for ( ni = nr = 0; nr < numRec; nr++, ni += NUM_CORNERS )
	  (void) memcpy( rbuff+ni, rec[nr].lon_corner, nr_byte );
     retval = nc_put_var_float( ncid, var_id, rbuff );

     retval = nc_def_var( ncid, "lat_bnds", NC_FLOAT, 2, dimids, &var_id );
     if ( retval != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     for ( ni = nr = 0; nr < numRec; nr++, ni += NUM_CORNERS )
	  (void) memcpy( rbuff+ni, rec[nr].lat_corner, nr_byte );
     retval = nc_put_var_float( ncid, var_id, rbuff );

 done:
     if ( rbuff != NULL ) free( rbuff );
     if ( dbuff != NULL ) free( dbuff );
}
