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

.IDENTifer   ADAGUC_TOSOMI
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA TOSOMI ADAGUC
.LANGUAGE    ANSI C
.PURPOSE     convert/combine TOSOMI product to ADAGUC standard
.INPUT/OUTPUT
  call as   
            nadc_tosomi [options] <input-files>

.RETURNS     non-negative on success, negative on failure
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     23-Oct-2008   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>
#include <netcdf.h>

/*+++++ Local Headers +++++*/
#include <nadc_tosomi.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   HPSORT
.PURPOSE     sort an array into assending order using the Heapsort algorithm
.INPUT/OUTPUT
  call as   HPSORT( dim, jday, rec );
     input:
            unsigned int dim       :    dimension of the array to be sorted
    output:
            double       *jday     :    array to be sorted
	    struct tosomi_rec *rec :    array of records to be sorted as jday

.RETURNS     nothing
.COMMENTS    static function
             Heapsort algorithm in a N log_2 N routine
-------------------------*/
static
void HPSORT( unsigned int dim, double *jday, struct tosomi_rec *rec )
{
     register unsigned int ii,  ir, jj, ll;

     double jbuff;
     struct tosomi_rec buff;

     const size_t sizeof_tosomi_rec = sizeof(struct tosomi_rec);
/*
 * initialization
 */
     jday--; rec--;
     ll = (dim >> 1) + 1;
     ir = dim;

     while ( TRUE ) {
	  if ( ll > 1 ) {                          /* still in hirring phase */
	       jbuff = jday[--ll];
	       (void) memcpy( &buff, rec+ll, sizeof_tosomi_rec );
	  } else {                      /* in retirement-and-promotion phase */
	       jbuff = jday[ir];              /* clear space at end of array */
	       (void) memcpy( &buff, rec+ir, sizeof_tosomi_rec );
	       jday[ir] = jday[1];     /* retire the top of the heap into it */
	       (void) memcpy( rec+ir, rec+1, sizeof_tosomi_rec );
	       if ( --ir == 1 ) {            /* done with the last promotion */
		    jday[1] = jbuff;    /* the least competent worker of all */
		    (void) memcpy( rec+1, &buff, sizeof_tosomi_rec );
		    break;
	       }
	  }
/*
 * whether in the hiring phase or promotion phase, we here set up to shift
 * down element jbuff to its proper level
 */
	  ii = ll;
	  jj = ll + ll;
	  while ( jj <= ir ) {
	       if ( jj < ir && jday[jj] < jday[jj+1] ) jj++;
	       if ( jbuff < jday[jj] ) {                     /* demote jbuff */
		    jday[ii] = jday[jj];
		    (void) memcpy( rec+ii, rec+jj, sizeof_tosomi_rec );
		    ii = jj;
		    jj <<= 1;
	       } else
		    jj = ir + 1;       /* set jj to terminate the shift down */
	  }
	  jday[ii] = jbuff;                       /* put jbuff into its slot */
	  (void) memcpy( rec+ii, &buff, sizeof_tosomi_rec );
     }
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
int main ( int argc, char *argv[] )
{
     const char prognm[] = "adaguc_tosomi";

     register int na = 0;

     char flname[MAX_STRING_LENGTH];

     register unsigned int nr;

     int    ncid;
     int    retval;
     double *jday = NULL;

     struct param_adaguc param;

     struct tosomi_hdr hdr;
     struct tosomi_rec *rec = NULL;
/*
 * check command-line parameters
 */
     ADAGUC_INIT_PARAM( argc, argv, &param );
     if ( IS_ERR_STAT_FATAL ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, "" );
/*
 * check if we have to display version and exit
 */
     if ( param.flag_version == PARAM_SET ) {
	  (void) fprintf( stdout, "\nFile format:\n" );
	  (void) fprintf( stdout, "\tconforms to \"%s\" version (%s)\n\n", 
			  "ADAGUC Data Product Standard", ADAGUC_REF_VERSION );
	  NADC_SHOW_VERSION( stdout, prognm );
	  exit( EXIT_SUCCESS );
     }
/*
 * process input files
 */
     hdr.numProd = 0;
     do {
	  struct tosomi_hdr hdr_tmp;
	  struct tosomi_rec *rec_tmp = NULL;
	  
	  (void) nadc_strlcpy( flname, 
			       param.name_infiles[na], MAX_STRING_LENGTH );
	  if ( H5Fis_hdf5( flname ) ) {
	       if ( (retval = nc_open(flname, NC_NOWRITE, &ncid)) != NC_NOERR )
		    NADC_GOTO_ERROR(prognm,NADC_ERR_FATAL,nc_strerror(retval));
	       NADC_TOSOMI_RD_NC_META( ncid, &hdr_tmp );
	       hdr_tmp.numRec = NADC_TOSOMI_RD_NC_REC( ncid, &rec_tmp );
	       if ( nc_close( ncid ) != NC_NOERR )
		    NADC_GOTO_ERROR(prognm,NADC_ERR_FATAL,nc_strerror(retval));
	  } else {
	       (void) NADC_RD_TOSOMI( flname, &hdr_tmp, &rec_tmp );
	       if ( IS_ERR_STAT_WARN ) { 
		    nadc_stat &= ~NADC_STAT_WARN;
		    continue;
	       }
#ifdef DEBUG
	       (void) printf( "%s %s %s %s %s %s %s %s %hu %hu %hu %u\n", 
			      hdr_tmp.product, hdr_tmp.creation_date, 
			      hdr_tmp.receive_date, hdr_tmp.l1b_product, 
			      hdr_tmp.validity_start, hdr_tmp.validity_stop, 
			      hdr_tmp.software_version, hdr_tmp.lv1c_version, 
			      hdr_tmp.numProd, hdr_tmp.numRec, 
			      hdr_tmp.numState, hdr_tmp.file_size );
#endif
	  }
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR(prognm,NADC_ERR_FATAL,"failed to read product");

	  if ( hdr.numProd == 0 ) {
	       (void) memcpy( &hdr, &hdr_tmp, sizeof(struct tosomi_hdr) ); 
	       if ( hdr_tmp.numRec > 0 ) {
		    rec = (struct tosomi_rec *)
			 malloc( hdr_tmp.numRec * sizeof( struct tosomi_rec ));
		    if ( rec == NULL ) 
			 NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rec" );
		    (void) memcpy( rec, rec_tmp, 
				   hdr_tmp.numRec * sizeof(struct tosomi_rec) );
	       }
	  } else {
	       if ( strcmp( hdr.software_version, hdr_tmp.software_version ) != 0 )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL,
				     "inconsistent product versions" );

	       /* update header struct */
	       hdr.numProd += hdr_tmp.numProd;
	       hdr.numRec  += hdr_tmp.numRec;
	       hdr.file_size = hdr_tmp.file_size;
	       (void) strcat( hdr.l1b_product, "," );
	       (void) strcat( hdr.l1b_product, hdr_tmp.l1b_product );

	       /* append new records */
	       if ( hdr.numRec > 0 ) {
		    rec = (struct tosomi_rec *)
			 realloc( rec, hdr.numRec * sizeof(struct tosomi_rec));
		    if ( rec == NULL ) 
			 NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rec" );
		    (void) memcpy( rec + (hdr.numRec - hdr_tmp.numRec), 
				   rec_tmp, 
				   hdr_tmp.numRec * sizeof(struct tosomi_rec) );
	       }
	  }
	  if ( rec_tmp != NULL ) free( rec_tmp );
     } while ( ++na < param.num_infiles );
/*
 * check number of records read from IMAP product
 */
     if ( hdr.numRec == 0u || rec == NULL ) {
          NADC_GOTO_ERROR( prognm, NADC_ERR_NONE, 
                           "No valid retrievals found in product" );
     }
/*
 * sort the TOSOMI records in time
 */
     if ( hdr.numRec > 1 ) {
	  jday = (double *) malloc( hdr.numRec * sizeof(double) );
	  if ( jday == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "jday" );
	  for ( nr = 0; nr < hdr.numRec; nr++ ) jday[nr] = rec[nr].jday;
	  HPSORT( hdr.numRec, jday, rec );

	  if ( param.flag_clip == PARAM_SET ) {
	       register unsigned int ilow, ihigh;

	       double jdayStart, jdayStop;

	       jdayStart = Adaguc2sciaJDAY( param.clipStart );
	       jdayStop  = Adaguc2sciaJDAY( param.clipStop );
	       (void) nadc_strlcpy( hdr.validity_start, param.clipStart, 16 );
	       (void) nadc_strlcpy( hdr.validity_stop, param.clipStop, 16 );

	       for ( ilow = 0; ilow < hdr.numRec; ilow++ ) 
		    if ( jday[ilow] >= jdayStart ) break;
	       for ( ihigh = hdr.numRec-1; ihigh > ilow; ihigh-- ) 
		    if ( jday[ihigh] <= jdayStop ) break;

	       if ( ihigh <= ilow ) {
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
				     "all records outside clipRange" );
	       } else if ( ilow > 0 || ihigh < (hdr.numRec-1) ) {
		    hdr.numRec = ihigh - ilow + 1;
		    (void) memmove( rec, rec+ilow, 
				    hdr.numRec * sizeof(struct tosomi_rec) );
	       }
	  }
	  free( jday );
     }
/*
 * construct ADAGUC compiant filename, 
 * first, recontruct validity period from TOSOMI records
 */
     if ( hdr.numProd > 1 && param.flag_clip == PARAM_UNSET ) {
	  SciaJDAY2adaguc( rec->jday, hdr.validity_start );
	  SciaJDAY2adaguc( rec[hdr.numRec-1].jday, hdr.validity_stop );
     }
     (void) snprintf( flname, MAX_STRING_LENGTH, "%s/"ADAGUC_PROD_TEMPLATE, 
		      param.outdir, param.prodClass, 
		      hdr.validity_start, hdr.validity_stop, 1 );

     if ( (retval = nc_create( flname, NC_NETCDF4, &ncid ))!= NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );

     NADC_TOSOMI_WR_NC_META( ncid, &hdr );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "TOSOMI header" );

     NADC_TOSOMI_WR_NC_REC( ncid, hdr.numRec, rec );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "TOSOMI records" );

     if ( nc_close( ncid ) != NC_NOERR )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, nc_strerror(retval) );
/*
 * free allocated memory
 */
 done:
     if ( rec != NULL ) free( rec );
/*
 * display error messages
 */
     if ( param.flag_silent == PARAM_UNSET ) NADC_Err_Trace( stderr );
     if ( IS_ERR_STAT_FATAL )
          return NADC_ERR_FATAL;
     else
          return NADC_ERR_NONE;
}
