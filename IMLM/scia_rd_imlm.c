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

.IDENTifer   SCIA_RD_IMLM
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA SRON/IMLM
.LANGUAGE    ANSI C
.PURPOSE     read SRON IMLM products
.INPUT/OUTPUT
  call as   nrec = SCIA_RD_IMLM( flname, &hdr, &rec );
     input:  
             char   flname[]       : filename of IMLM product
    output:  
             struct imlm_hdr *hdr  : header of IMLM product
             struct imlm_rec **rec : structure holding IMLM tile information

.RETURNS     number of records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION     1.3     12-May-2011   allow long lines in v8.2, RvH
             1.2     12-Apr-2011   moved selection to calling routine, RvH
             1.1     20-Jan-2010   fixed the chi-square threhold, RvH
             1.0     02-Jul-2008   initial release by R. M. van Hees
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
#include <nadc_imlm.h>

/*+++++ Macros +++++*/
#define MAX_LINE_LENGTH  512
#define NUM_COLUMNS      35
#define FORMAT_RECORD \
"%f %f %f %f %f %f %f %f %f %f %lf %f %f %f %f %f %f \
 %f %f %f %f %f %f %f %f %hu %hu %f %f %f %hhu %hu %hhu \
 %*c %*f %f %f %*d %*d %*d %*d"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static
void DateTime2ADAGUC( const char *date, const char *time, char *timeStamp )
{
     const char *mnth_lst[] = {
          "FAIL", "Jan", "Feb", "Mar", "Apr", "May",
          "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

     char         cbuff[4];
     unsigned int imin, ihour, iday, imon, iyear;
     double       sec;
/*
 * decomposition of date and time string into numbers
 */
     (void) sscanf( date, "%2u-%3s-%4u", &iday, cbuff, &iyear );
     (void) sscanf( time, "%2u:%2u:%6lf", &ihour, &imin, &sec );

     for ( imon = 1; imon <= 12; imon++ )
          if ( strcmp( cbuff, mnth_lst[imon] ) == 0 ) break;

     (void) sprintf( timeStamp, "%04u%02u%02uT%02u%02u%02.0f",
                     iyear, imon, iday, ihour, imin, sec );
}

/*+++++++++++++++++++++++++
.IDENTifer   Read_IMLM_Header
.PURPOSE     read header of (ascii) SRON IMLM product
.INPUT/OUTPUT
  call as   Read_IMLM_Header( flname, hdr );
     input:
            char flname[]        :  name of the IMLM product
    output:
            struct imlm_hdr *hdr :  header info from IMLM product

.RETURNS     nothing, error status is passed by global nadc_stat
.COMMENTS    static function
-------------------------*/
static
void Read_IMLM_Header( const char flname[], /*@out@*/ struct imlm_hdr *hdr )
   /*@globals  errno, nadc_stat, nadc_err_stack;@*/
   /*@modifies errno, nadc_stat, nadc_err_stack, hdr@*/
{
     const char prognm[] = "Read_IMLM_Header";

     char   line[MAX_LINE_LENGTH];
     char   sciaDate[12], sciaTime[16];

     unsigned int nline = 0u;

     gzFile fp;
/*
 * open the IMLM product
 */
     if ( (fp = gzopen( flname, "r" )) == NULL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, flname );

     do {
          if ( gzeof ( fp ) == 1 ) break;
          if ( gzgets( fp, line, MAX_LINE_LENGTH ) == NULL ) break;
	  if ( strncmp( line, "ENDFILE", 7 ) == 0 ) break;

	  if ( *line == '#' ) {
	       if ( strncmp( line+2, "Generated", 9 ) == 0 ) {
		    char *cpntr = strrchr( line, '>' );
		    if ( cpntr == NULL ) continue;
		    (void) sscanf(cpntr+4, "%*s %s %s", sciaDate, sciaTime);
		    (void) snprintf( hdr->creation_date, 28,
                                     "%s %s", sciaDate, sciaTime );
		    cpntr[1] = '\0';
		    (void) nadc_strlcpy( hdr->contact, line+15, 64 );
	       } else if ( strncmp( line+2, "Level-2", 7 ) == 0 ) {
		    (void) sscanf( line, "# Level-2 format version: %s",
				   hdr->product_format );
	       } else if ( strncmp( line+2, "Level-1b", 8 ) == 0 ) {
		    (void) sscanf( line, "# Level-1b product: %s", 
				   hdr->l1b_product );
	       } else if ( strncmp( line+2, "Sensing start", 13 ) == 0 ) {
		    (void) sscanf( line, "# Sensing start: %s %s",
				   sciaDate, sciaTime );
		    DateTime2ADAGUC( sciaDate, sciaTime, hdr->validity_start );
	       } else if ( strncmp( line+2, "Sensing stop", 12 ) == 0 ) {
		    (void) sscanf( line, "# Sensing stop: %s %s",
				   sciaDate, sciaTime );
		    DateTime2ADAGUC( sciaDate, sciaTime, hdr->validity_stop );
	       } else if ( strncmp( line+2, "Orbit", 5 ) == 0 ) {
		    (void) sscanf( line, "# Orbit: %u", hdr->orbit );
	       } else if ( strncmp( line+2, "IMLM", 4 ) == 0 ) {
		    (void) sscanf( line, "# IMLM version: %s %*c", 
				   hdr->software_version );
	       } else if ( strncmp( line+2, "SCIAMACHY", 9 ) == 0 ) {
		    (void) sscanf( line, "# SCIAMACHY channel: %hhu", 
				   &hdr->scia_channel );
	       } else if ( strncmp( line+2, "Window (pixels)", 15 ) == 0 ) {
		    (void) sscanf( line, "# Window (pixels): %hu %*s %hu", 
				   hdr->window_pixel, hdr->window_pixel+1 );
	       } else if ( strncmp( line+2, "Window (nm)", 11 ) == 0 ) {
		    (void) sscanf( line, "# Window (nm): %f %*s %f", 
				   hdr->window_wave, hdr->window_wave+1 );
	       } else if ( strncmp( line+2, "Pixel mask", 10 ) == 0 ) {
		    (void) sscanf( line, "# Pixel mask version: %s (%*c", 
				   hdr->pixelmask_version );
	       } else if ( strncmp( line+2, "Cloud mask", 10 ) == 0 ) {
		    (void) sscanf( line, "# Cloud mask version: %s (%*c", 
				   hdr->cloudmask_version );
	       }
	  } else
	       ++nline;
     } while( TRUE );

     hdr->numProd = 1;
     hdr->numRec  = nline;
     (void) gzclose( fp );
}

/*+++++++++++++++++++++++++
.IDENTifer   Read_IMLM_Record
.PURPOSE     read data records from (ascii) SRON IMLM products
.INPUT/OUTPUT
  call as   nline = Read_IMLM_Record( fp, rec );
     input:
            char flname[]        :  name of the IMLM product
    output:
            struct imlm_rec *rec :  data records from the IMLM product

.RETURNS     number of lines erad from product (unsigned int)
.COMMENTS    static function
-------------------------*/
static 
unsigned int Read_IMLM_Record( const char flname[], 
			       /*@null@*/ /*@out@*/ struct imlm_rec *rec )
   /*@globals  errno, nadc_stat, nadc_err_stack;@*/
   /*@modifies errno, nadc_stat, nadc_err_stack, rec@*/
{
     const char prognm[] = "Read_IMLM_Record";

     char   line[MAX_LINE_LENGTH];
     int    numItems;

     gzFile fp;

     register unsigned int   numRec = 0u;
     register unsigned short nr;
/*
 * open the IMLM product
 */
     if ( (fp = gzopen( flname, "r" )) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, flname );

     do {
	  char *cpntr;

          if ( gzeof ( fp ) == 1 ) goto done;
          if ( gzgets( fp, line, MAX_LINE_LENGTH ) == Z_NULL ) goto done;


	  if ( *line == '#' ) continue;
	  if ( (cpntr = strrchr( line, '#' )) != NULL ) *cpntr = '\0';

	  numItems = sscanf( line, FORMAT_RECORD, 
			     rec->lon_corner+1, rec->lat_corner+1,
			     rec->lon_corner  , rec->lat_corner  , 
			     rec->lon_corner+2, rec->lat_corner+2, 
			     rec->lon_corner+3, rec->lat_corner+3, 
			     &rec->lon_center, &rec->lat_center, 
			     &rec->dsr_time, &rec->meta.intg_time, 
			     &rec->meta.sza, &rec->meta.lza, 
			     &rec->H2O, &rec->HDO, &rec->N2O, &rec->CO, 
			     &rec->CH4, &rec->H2O_err, &rec->HDO_err, 
			     &rec->N2O_err, &rec->CO_err, &rec->CH4_err, 
			     &rec->meta.chisq, &rec->meta.dof, 
			     &rec->meta.eflag, &rec->albedo, &rec->cl_fr, 
			     &rec->mean_elev, &rec->meta.st, &rec->meta.px, 
			     &rec->meta.bs, &rec->signal, &rec->pressure );

	  if ( numItems != NUM_COLUMNS ) {
	       char msg[80];
	       (void) snprintf( msg, 80, "incomplete record[%-u]\n", numRec );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, msg );
	  }
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
void SCIA_RD_IMLM( const char *flname, struct imlm_hdr *hdr,
		   struct imlm_rec **imlm_out )
{
     const char prognm[] = "SCIA_RD_IMLM";

     char   *cpntr, ctemp[SHORT_STRING_LENGTH];

     unsigned int numRec = 0u;

     struct imlm_rec *rec = NULL;
/*
 * strip path of file-name & remove extension ".gz"
 */
     if ( (cpntr = strrchr( flname, '/' )) != NULL ) {
          (void) nadc_strlcpy( ctemp, ++cpntr, SHORT_STRING_LENGTH );
     } else {
          (void) nadc_strlcpy( ctemp, flname, SHORT_STRING_LENGTH );
     }
     if ( (cpntr = strstr( ctemp, ".gz" )) != NULL ) *cpntr = '\0';
/*
 * initialize IMLM header structure
 */
     (void) nadc_strlcpy( hdr->product, ctemp, 42 );
     NADC_RECEIVEDATE( flname, hdr->receive_date );
     (void) strcpy( hdr->creation_date, "" );
     (void) strcpy( hdr->software_version, "" );
     hdr->file_size = nadc_file_size( flname );
     hdr->numRec    = 0u;
     imlm_out[0] = NULL;
/*
 * read header of the IMLM product
 */
     Read_IMLM_Header( flname, hdr );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "Read_IMLM_Header" );
     if ( hdr->numRec == 0u ) return;
/*
 * allocate enough space to store all records
 */
     rec = (struct imlm_rec *) 
	  malloc( hdr->numRec * sizeof( struct imlm_rec ));
     if ( rec == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rec" );
/*
 * read records from the IMLM product
 */
     if ( (numRec = Read_IMLM_Record( flname, rec )) != hdr->numRec ) {
	  char msg[SHORT_STRING_LENGTH];

	  free( rec );
	  (void) snprintf( msg, SHORT_STRING_LENGTH,
			   "failed to read all records %u out of %u\n", 
			   numRec, hdr->numRec );
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, msg );
     }
     imlm_out[0] = rec;
}
