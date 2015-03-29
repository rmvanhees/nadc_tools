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

.IDENTifer   NADC_RD_NC_FRESCO
.AUTHOR      R.M. van Hees
.KEYWORDS    FRESCO GOME SCIA
.LANGUAGE    ANSI C
.PURPOSE     read KNMI Fresco product in ADAGUC format
.COMMENTS    contains NADC_FRESCO_rd_NC_META and NADC_FRESCO_rd_NC_REC
.ENVIRONment None
.VERSION     1.0     28-Oct-2008   initial release by R. M. van Hees
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
#include <nadc_fresco.h>

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
.IDENTifer   NADC_FRESCO_RD_NC_META
.PURPOSE     read header from Fresco product in ADAGUC format (netCDF-4)
.INPUT/OUTPUT
  call as   NADC_FRESCO_RD_NC_META( ncid, &hdr );
     input:
            int ncid               :  netCDF file ID
    output:
            struct fresco_hdr *hdr :  Fresco header

.RETURNS     Nothing
.COMMENTS    None
-------------------------*/
void NADC_FRESCO_RD_NC_META( int ncid, struct fresco_hdr *hdr )
{
     int  retval;
     int  var_id;

     retval = nc_get_att_text( ncid, NC_GLOBAL, "source", hdr->source );
     if ( retval != NC_NOERR )
          NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
/*
 * Product meta-data
 */
     if ( (retval = nc_inq_varid( ncid, "product", &var_id )) != NC_NOERR )
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
     if ( retval != NC_NOERR )
          NADC_RETURN_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_FRESCO_RD_NC_REC
.PURPOSE     read records from Fresco product in ADAGUC format (netCDF-4) 
.INPUT/OUTPUT
  call as   numRec = NADC_FRESCO_RD_NC_REC( ncid, &rec );
     input:
            int ncid                :  netCDF file ID
    output:
            struct fresco_rec **rec :  Fresco records

.RETURNS     Number of records read from product (unsigned int)
.COMMENTS    None
-------------------------*/
unsigned int NADC_FRESCO_RD_NC_REC( int ncid, struct fresco_rec **rec_out )
{
     register unsigned int ni, nr;

     unsigned int numRec = 0u;

     int    retval;
     int    time_id;
     int    var_id;
     size_t indx, length;

     float  *rbuff = NULL;
     double *dbuff = NULL;

     struct fresco_rec *rec = NULL;

     const size_t nr_byte = NUM_CORNERS * sizeof(float);

     rec_out[0] = NULL;
/*
 * read dimension scale "time"
 */
     if ( (retval = nc_inq_dimid( ncid, "time", &time_id )) != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     if ( (retval = nc_inq_dimlen( ncid, time_id, &length )) != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     numRec = (unsigned int) length;

     rec = (struct fresco_rec *) malloc( numRec * sizeof(struct fresco_rec) );
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
 * read datasets (cloudFraction,cloudFractionError,cloudTopHeight,cloudTopPress,
 *    cloudTopPressError,cloudAlbedo,cloudAlbedoError,surfaceAlbedo,
 *    surfaceHeight,groundPress)
 */
     if ( (retval = nc_inq_varid( ncid, "cloudFraction", &var_id )) != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].cloudFraction = rbuff[nr];
     if ( (retval = nc_inq_varid( ncid, "cloudFractionError", &var_id )) != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].cloudFractionError = rbuff[nr];
     if ( (retval = nc_inq_varid( ncid, "cloudTopHeight", &var_id )) != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].cloudTopHeight = rbuff[nr];
     if ( (retval = nc_inq_varid( ncid, "cloudTopPress", &var_id )) != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].cloudTopPress = rbuff[nr];
     if ( (retval = nc_inq_varid( ncid, "cloudTopPressError", &var_id )) != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].cloudTopPressError = rbuff[nr];
     if ( (retval = nc_inq_varid( ncid, "cloudAlbedo", &var_id )) != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].cloudAlbedo = rbuff[nr];
     if ( (retval = nc_inq_varid( ncid, "cloudAlbedoError", &var_id )) != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].cloudAlbedoError = rbuff[nr];
     if ( (retval = nc_inq_varid( ncid, "surfaceAlbedo", &var_id )) != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].surfaceAlbedo = rbuff[nr];
     if ( (retval = nc_inq_varid( ncid, "surfaceHeight", &var_id )) != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].surfaceHeight = rbuff[nr];
     if ( (retval = nc_inq_varid( ncid, "groundPress", &var_id )) != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].groundPress = rbuff[nr];
/*
 * read longitude and latitude of tile-corners
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
     if ( (retval = nc_inq_varid( ncid, "tile", &var_id )) != NC_NOERR )
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
