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

.IDENTifer   SCIA_RD_NC_CH4
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA IMAP CH4
.LANGUAGE    ANSI C
.PURPOSE     read IMAP-CH4 product in ADAGUC format
.COMMENTS    contains SCIA_RD_NC_CH4_META and SCIA_RD_NC_CH4_REC
.ENVIRONment None
.VERSION     1.2     28-Apr-2011   differentiate between CH4 and HDO code, RvH
             1.0     20-Oct-2008   initial release by R. M. van Hees
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
#define __IMAP_CH4_PRODUCT
#include <nadc_imap.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_RD_NC_CH4_META
.PURPOSE     read header from IMAP-CH4 product in netCDF-4 (ADAGUC standard)
.INPUT/OUTPUT
  call as   SCIA_RD_NC_CH4_META( ncid, &hdr );
     input:
            int ncid             :  netCDF file ID
    output:
	    struct imap_hdr *hdr :  IMAP header

.RETURNS     Nothing
.COMMENTS    static function
-------------------------*/
void SCIA_RD_NC_CH4_META( int ncid, struct imap_hdr *hdr )
{
     int  retval;
     int  var_id;
/*
 * Product meta-data
 */
     if ( (retval  = nc_inq_varid( ncid, "product", &var_id )) != NC_NOERR )
          NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_att_text( ncid, var_id, "input_products", 
			       hdr->l1b_product );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_att_text( ncid, var_id, "creation_date", 
			       hdr->creation_date );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_att_text( ncid, var_id, "validity_start", 
			       hdr->validity_start );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_att_text( ncid, var_id, "validity_stop", 
			       hdr->validity_stop );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_att_text( ncid, var_id, "software_version", 
			       hdr->software_version );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
/*
 * Custom meta-data
 */
     if ( (retval = nc_inq_varid( ncid, "custom", &var_id )) != NC_NOERR )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_att_ushort( ncid, var_id, "number_input_products", 
				 &hdr->numProd );
     retval = nc_get_att_ushort( ncid, var_id, "file_counter", 
				 &hdr->counter[0] );
     retval = nc_get_att_uint( ncid, var_id, "abs_orbit", 
				 &hdr->orbit[0] );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_RD_NC_CH4_REC
.PURPOSE     read records from IMAP-CH4 product in netCDF-4 (ADAGUC standard)
.INPUT/OUTPUT
  call as   numRec = SCIA_RD_NC_CH4_REC( ncid, &rec );
     input:
            int ncid              :  netCDF file ID
    output:
	    struct imap_rec **rec :  IMAP records

.RETURNS     number of IMAP records (unsigned int)
.COMMENTS    static function
-------------------------*/
unsigned int SCIA_RD_NC_CH4_REC( int ncid, struct imap_rec **rec_out )
{
     register unsigned int ni, nr;

     unsigned int numRec = 0;

     int    retval;
     int    time_id;
     int    var_id;
     size_t indx, length;

     float  *rbuff = NULL;
     double *dbuff = NULL;

     struct imap_rec *rec = NULL;

     const size_t nr_byte = NUM_CORNERS * sizeof(float);

     rec_out[0] = NULL;
/*
 * get size of dimension scale "time"
 */
     if ( (retval = nc_inq_dimid( ncid, "time", &time_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     if ( (retval = nc_inq_dimlen( ncid, time_id, &length )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     numRec = (unsigned int) length;

     rec = (struct imap_rec *) malloc( numRec * sizeof(struct imap_rec) );
     if ( rec == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rec" );
/*
 * read Julian date of measurements
 */
     dbuff = (double *) malloc( numRec * sizeof(double) );
     if ( dbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "dbuff" );

     if ( (retval = nc_inq_varid( ncid, "time", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_double( ncid, var_id, dbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].jday = dbuff[nr];
     free( dbuff );
/*
 * read longitude and latitude of measurements
 */
     rbuff = (float *) malloc( numRec * sizeof(float) );
     if ( rbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );

     if ( (retval = nc_inq_varid( ncid, "lon", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].lon_center = rbuff[nr];

     if ( (retval = nc_inq_varid( ncid, "lat", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].lat_center = rbuff[nr];
/*
 * read datasets (CH4, CH4_error, CH4_model, CO2, CO2_error, xVMR_CH4)
 */
     /*+++++++++++++++++++++++++*/
     if ( (retval = nc_inq_varid( ncid, "CO2", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].co2_vcd = rbuff[nr];
     if ( (retval = nc_inq_varid( ncid, "CO2_error", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].co2_error = rbuff[nr];

     /*+++++++++++++++++++++++++*/
     if ( (retval = nc_inq_varid( ncid, "CH4", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].ch4_vcd = rbuff[nr];
     if ( (retval = nc_inq_varid( ncid, "CH4_error", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].ch4_error = rbuff[nr];
     if ( (retval = nc_inq_varid( ncid, "CH4_model", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].ch4_model = rbuff[nr];

     /*+++++++++++++++++++++++++*/
     if ( (retval = nc_inq_varid( ncid, "xVMR_CH4", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].ch4_vmr = rbuff[nr];
/*
 * read longitude and latitude bounding boxes
 */
     rbuff = (float *) realloc( rbuff, NUM_CORNERS * numRec * sizeof(float) );
     if ( rbuff == NULL ) NADC_GOTO_ERROR( NADC_ERR_ALLOC, "rbuff" );

     if ( (retval = nc_inq_varid( ncid, "lon_bnds", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( ni = nr = 0; nr < numRec; nr++, ni += NUM_CORNERS )
	  (void) memcpy( rec[nr].lon_corner, rbuff+ni, nr_byte );

     if ( (retval = nc_inq_varid( ncid, "lat_bnds", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( ni = nr = 0; nr < numRec; nr++, ni += NUM_CORNERS )
	  (void) memcpy( rec[nr].lat_corner, rbuff+ni, nr_byte );
/*
 * read pixel meta-data as compound dataset
 */
     if ( (retval = nc_inq_varid( ncid, "tile_properties", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     for ( indx = 0; indx < (size_t) numRec; indx++ ) {
	  retval = nc_get_var1( ncid, var_id, &indx, &rec[indx].meta );
     }

     rec_out[0] = rec;
     return numRec;
 done:
     if ( rec != NULL ) free( rec );
     if ( rbuff != NULL ) free( rbuff );
     if ( dbuff != NULL ) free( dbuff );
     return 0;
}
