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

.IDENTifer   NADC_RD_NC_TOGOMI
.AUTHOR      R.M. van Hees
.KEYWORDS    TOGOMI GOME
.LANGUAGE    ANSI C
.PURPOSE     read TOGOMI product in ADAGUC format
.COMMENTS    contains NADC_TOGOMI_RD_NC_META and NADC_TOGOMI_RD_NC_REC
.ENVIRONment None
.VERSION     1.0     23-Oct-2008   initial release by R. M. van Hees
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
#include <nadc_togomi.h>

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
.IDENTifer   NADC_TOGOMI_RD_NC_META
.PURPOSE     read header from TOGOMI product in ADAGUC format (netCDF-4)
.INPUT/OUTPUT
  call as   NADC_TOGOMI_RD_NC_META( ncid, &hdr );
     input:
            int ncid               :  netCDF file ID
    output:
            struct togomi_hdr *hdr :  TOGOMI header

.RETURNS     Nothing
.COMMENTS    None
-------------------------*/
void NADC_TOGOMI_RD_NC_META( int ncid, struct togomi_hdr *hdr )
{
     const char prognm[] = "NADC_TOGOMI_RD_NC_META";

     int  retval;
     int  var_id;
/*
 * Product meta-data
 */
     if ( (retval = nc_inq_varid( ncid, "product", &var_id )) != NC_NOERR )
          NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_att_text( ncid, var_id, "input_products", 
			       hdr->l1b_product );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_att_text( ncid, var_id, "creation_date", 
			       hdr->creation_date );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_att_text( ncid, var_id, "validity_start", 
			       hdr->validity_start );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_att_text( ncid, var_id, "validity_stop", 
			       hdr->validity_stop );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_att_text( ncid, var_id, "software_version", 
			       hdr->software_version );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
/*
 * Custom meta-data
 */
     if ( (retval = nc_inq_varid( ncid, "custom", &var_id )) != NC_NOERR )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_att_ushort( ncid, var_id, "number_input_products", 
				 &hdr->numProd );
     if ( retval != NC_NOERR )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_TOGOMI_RD_NC_REC
.PURPOSE     read records from TOGOMI product in ADAGUC format (netCDF-4)
.INPUT/OUTPUT
  call as   numRec = NADC_TOGOMI_RD_NC_REC( ncid, &rec );
     input:
            int ncid               :   netCDF file ID
    output:
            struct togomi_rec *rec :   TOGOMI records

.RETURNS     number of TOGOMI records (unsigned short)
.COMMENTS    static function
-------------------------*/
unsigned int NADC_TOGOMI_RD_NC_REC( int ncid, struct togomi_rec **rec_out )
{
     const char prognm[] = "NADC_TOGOMI_RD_NC_REC";

     register unsigned int ni, nr;

     unsigned int numRec = 0;

     int    retval;
     int    time_id;
     int    var_id;
     size_t indx, length;

     float  *rbuff = NULL;
     double *dbuff = NULL;

     struct togomi_rec *rec = NULL;

     const size_t nr_byte = NUM_CORNERS * sizeof(float);

     rec_out[0] = NULL;
/*
 * read dimension scale "time"
 */
     if ( (retval = nc_inq_dimid( ncid, "time", &time_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     if ( (retval = nc_inq_dimlen( ncid, time_id, &length )) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     numRec = (unsigned int) length;

     rec = (struct togomi_rec *) malloc( numRec * sizeof(struct togomi_rec) );
     if ( rec == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rec" );
/*
 * read Julian date of measurements
 */
     dbuff = (double *) malloc( numRec * sizeof(double) );
     if ( dbuff == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "dbuff" );

     if ( (retval = nc_inq_varid( ncid, "time", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_double( ncid, var_id, dbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].jday = dbuff[nr];
     free( dbuff );
/*
 * read longitude and latitude of measurements
 */
     rbuff = (float *) malloc( numRec * sizeof(float) );
     if ( rbuff == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );

     if ( (retval = nc_inq_varid( ncid, "lon", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].lon_center = rbuff[nr];

     if ( (retval = nc_inq_varid( ncid, "lat", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].lat_center = rbuff[nr];
/*
 * read datasets (vcd, vcdError, scd, scdError)
 */
     if ( (retval = nc_inq_varid( ncid, "vcd", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].vcd = rbuff[nr];
     if ( (retval = nc_inq_varid( ncid, "vcdError", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].vcdError = rbuff[nr];
      /*+++++++++++++++++++++++++*/
     if ( (retval = nc_inq_varid( ncid, "scd", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].scd = rbuff[nr];
     if ( (retval = nc_inq_varid( ncid, "scdError", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( nr = 0; nr < numRec; nr++ ) rec[nr].scdError = rbuff[nr];
/*
 * read longitude and latitude of tile-corners
 */
     rbuff = (float *) realloc( rbuff, NUM_CORNERS * numRec * sizeof(float) );
     if ( rbuff == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );

     if ( (retval = nc_inq_varid( ncid, "lon_bnds", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( ni = nr = 0; nr < numRec; nr++, ni += NUM_CORNERS )
	  (void) memcpy( rec[nr].lon_corner, rbuff+ni, nr_byte );

     if ( (retval = nc_inq_varid( ncid, "lat_bnds", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_get_var_float( ncid, var_id, rbuff );
     for ( ni = nr = 0; nr < numRec; nr++, ni += NUM_CORNERS )
	  (void) memcpy( rec[nr].lat_corner, rbuff+ni, nr_byte );
/*
 * read pixel meta-data as compound dataset
 */
     if ( (retval = nc_inq_varid( ncid, "tile", &var_id )) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
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
