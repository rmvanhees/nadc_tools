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

.IDENTifer   SCIA_RD_IMAP_HDO
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA IMAP HDO
.LANGUAGE    ANSI C
.PURPOSE     read SRON IMAP-HDO products
.INPUT/OUTPUT
  call as   nrec = SCIA_RD_IMAP_HDO( flname, &hdr, &rec );
     input:  
             char   flname[]       : filename of IMAP product
    output:  
             struct imap_hdr *hdr  : header of IMAP product
             struct imap_rec **rec : structure holding IMAP tile information

.RETURNS     number of records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     28-Apr-2011   initial release by R. M. van Hees
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
#include <math.h>
#include <zlib.h>

/*+++++ Local Headers +++++*/
#define __IMAP_HDO_PRODUCT
#include <nadc_imap.h>

/*+++++ Macros +++++*/
#define MAX_LINE_LENGTH  275
#define NUM_COLUMNS      33
#define FORMAT_RECORD \
"%d %d %d %f %lf \
 %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f \
 %hu %f %f %f %f %f %f %f %f"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   Read_IMAP_Record
.PURPOSE     read data records from (ascii) SRON IMAP products
.INPUT/OUTPUT
  call as   nline = Read_IMAP_Record( fp, rec );
     input:
            char flname[]        :  name of the IMAP product
    output:
            struct imap_rec *rec :  data records from the IMAP product

.RETURNS     number of lines erad from product (unsigned int)
.COMMENTS    static function
-------------------------*/
static 
unsigned int Read_IMAP_Record( const char flname[], 
			       /*@null@*/ /*@out@*/ struct imap_rec *rec )
   /*@globals  errno, nadc_stat, nadc_err_stack;@*/
   /*@modifies errno, nadc_stat, nadc_err_stack, rec@*/
{
     const char prognm[] = "Read_IMAP_Record";

     char   line[MAX_LINE_LENGTH];
     int    numItems;
     int    iday, imon, iyear;
     float  hour;

     gzFile fp;

     register unsigned int   numRec = 0u;
     register unsigned short nr;
/*
 * open the IMAP product
 */
     if ( (fp = gzopen( flname, "r" )) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, flname );

     do {
	  char *cpntr;

          if ( gzeof ( fp ) == 1 ) goto done;
          if ( gzgets( fp, line, MAX_LINE_LENGTH ) == Z_NULL ) goto done;


	  if ( *line == '#' ) continue;
	  if ( (cpntr = strrchr( line, '#' )) != NULL ) cpntr = '\0';

	  numItems = sscanf( line, FORMAT_RECORD,
			     &iday, &imon, &iyear, &hour, &rec->jday, 
			     &rec->lat_center, &rec->lon_center,
			     &rec->delta_d, &rec->delta_d_error, 
			     &rec->hdo_vcd, &rec->hdo_error, 
			     &rec->h2o_vcd, &rec->h2o_error, &rec->h2o_model,
			     &rec->meta.cl_frac, &rec->meta.cl_press,
			     &rec->meta.sza, &rec->meta.lza, 
			     &rec->meta.rms_res, &rec->meta.rms_res_wght,
			     &rec->meta.bu, &rec->meta.ocean_frac,
			     &rec->meta.elev, &rec->meta.intg_time,
			     &rec->meta.pixels_fresco,
			     rec->lat_corner+1, rec->lat_corner  , 
			     rec->lat_corner+2, rec->lat_corner+3, 
			     rec->lon_corner+1, rec->lon_corner  , 
			     rec->lon_corner+2, rec->lon_corner+3 );

	  if ( numItems != NUM_COLUMNS ) {
	       char msg[80];
	       (void) snprintf( msg, 80, "incomplete record[%-u]\n", numRec );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, msg );
	  }
	  rec->meta.stateID = UCHAR_ZERO;
	  rec->meta.bs = UCHAR_ZERO;
	  rec->lon_center = LON_IN_RANGE(rec->lon_center);
	  for ( nr = 0; nr < NUM_CORNERS; nr++ ) {
	       rec->lon_corner[nr] = LON_IN_RANGE( rec->lon_corner[nr] );
	       if ( fabsf(rec->lon_center - rec->lon_corner[nr]) > 180.f ) {
		    if ( rec->lon_center > 0.f )
			 rec->lon_corner[nr] += 360.f;
		    else
			 rec->lon_corner[nr] -= 360.f;
	       }
	  }
	  ++rec;
	  ++numRec;
     } while( TRUE );
 done:
     if ( fp != NULL ) (void) gzclose( fp );
     return numRec;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_RD_IMAP_HDO( const char *flname, struct imap_hdr *hdr,
		       struct imap_rec **imap_out )
{
     const char prognm[] = "SCIA_RD_IMAP_HDO";

     char   *cpntr, ctemp[SHORT_STRING_LENGTH], line[MAX_LINE_LENGTH];

     gzFile fp;

     unsigned int numRec = 0u;

     struct imap_rec *rec = NULL;
/*
 * strip path of file-name & remove extension ".gz"
 */
     if ( (cpntr = strrchr( flname, '/' )) != NULL ) {
          (void) strlcpy( ctemp, ++cpntr, SHORT_STRING_LENGTH );
     } else {
          (void) strlcpy( ctemp, flname, SHORT_STRING_LENGTH );
     }
     if ( (cpntr = strstr( ctemp, ".gz" )) != NULL ) *cpntr = '\0';
/*
 * initialize IMAP header structure
 */
     imap_out[0] = NULL;
     (void) strlcpy( hdr->product, ctemp, 42 );
     NADC_RECEIVEDATE( flname, hdr->receive_date );
     (void) strcpy( hdr->creation_date, hdr->receive_date );
     (void) strcpy( hdr->l1b_product, "" );
     (void) strcpy( hdr->validity_start, "" );
     (void) strcpy( hdr->validity_stop, "" );
     (void) strncpy( hdr->software_version, ctemp+13,3 );
     if ( (cpntr = strrchr( ctemp, '_' )) != NULL ) {
          char str_magic[5], str_orbit[6];

          (void) strlcpy( str_magic, cpntr+1, 5 );
          while ( --cpntr > ctemp ) if ( *cpntr == '_' ) break;
          (void) strlcpy( str_orbit, cpntr+1, 6 );
          hdr->counter[0] = 
               (unsigned short) strtoul( str_magic, (char **) NULL, 10 );
          hdr->orbit[0]   = 
               (unsigned int) strtoul( str_orbit, (char **) NULL, 10 );
     } else {
          hdr->counter[0] = 0u;
          hdr->orbit[0]   = 0u;
     }
     hdr->file_size = NADC_FILESIZE( flname );
     hdr->numProd   = 1u;
/*
 * obtain number of records
 */
     if ( (fp = gzopen( flname, "r" )) == NULL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, flname );
     hdr->numRec    = 0u;
     do {
          if ( gzeof ( fp ) == 1 ) break;
          if ( gzgets( fp, line, MAX_LINE_LENGTH ) == Z_NULL ) break;
	  if ( *line == '#' ) continue;
	  hdr->numRec++;
     } while( TRUE );
     (void) gzclose( fp );
     if ( hdr->numRec == 0u ) return;
/*
 * allocate enough space to store all records
 */
     rec = (struct imap_rec *) 
	  malloc( hdr->numRec * sizeof( struct imap_rec ));
     if ( rec == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rec" );
/*
 * read records from the IMAP product
 */
     if ( (numRec = Read_IMAP_Record( flname, rec )) != hdr->numRec ) {
	  char msg[SHORT_STRING_LENGTH];

	  free( rec );
	  (void) snprintf( msg, SHORT_STRING_LENGTH,
			   "failed to read all records %u out of %u\n", 
			   numRec, hdr->numRec );
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, msg );
     }
     SciaJDAY2adaguc( rec->jday, hdr->validity_start );
     SciaJDAY2adaguc( rec[hdr->numRec-1].jday, hdr->validity_stop );
     imap_out[0] = rec;
}
