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

.IDENTifer   NADC_RD_TOSOMI
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA TOSOMI KNMI
.LANGUAGE    ANSI C
.PURPOSE     read TOSOMI products
.INPUT/OUTPUT
  call as   nrec = NADC_RD_TOSOMI( flname, &hdr, &rec );
     input:  
             char   tosomi_name[]    : filename of TOSOMI product
    output:  
             struct tosomi_hdr *hdr  : header of TOSOMI product
             struct tosomi_rec **rec : TOSOMI tile information

.RETURNS     number of records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     30-Sep-2008   initial release by R. M. van Hees
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
#include <ctype.h>
#include <zlib.h>

/*+++++ Local Headers +++++*/
#include <nadc_tosomi.h>

/*+++++ Macros +++++*/
#define NINT(a) ((a) >= 0.f ? (int)((a)+0.5) : (int)((a)-0.5))

#define MAX_LINE_LENGTH 256
#define NUM_COLUMNS      24
#define FORMAT_RECORD \
"%8s %10s %9d %9d %9d %9d %9d %9d %9d %9d %9d %9d \
 %3hhu %5hu %5hu %5hu %6hu %5d %5d %4hhu %4hu %4hhu %7f %7f"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static
void NADC_STRIP_ALL( const char *str_in, char *str_out )
{
     size_t ni = 0, nj = 0;

     while ( str_in[ni] != '\0' && str_in[ni] != '\n' ) {
          if ( ! isspace(str_in[ni]) && str_in[ni] != ',' ) {
               str_out[nj++] = str_in[ni];
          }
          ni++;
     }
     str_out[nj] = '\0';
}

static inline
double DateTime2SciaJDAY( const char *date, const char *time )
{
     unsigned int imin, ihour, iday, imon, iyear;
     double       day, jday, sec;

     const double jday_01012000 = 2451544.5;
/*
 * decomposition of date and time string into numbers into the tm struct
 */
     (void) sscanf( date, "%4u %2u %2u", &iyear, &imon, &iday );
     (void) sscanf( time, "%2u %2u %6lf", &ihour, &imin, &sec );
     day = iday + (ihour + (imin + (sec / 60.)) / 60.) / 24.;
/*
 * calculate output values
 */
     MJD_2_Julian( day, imon, iyear, &jday );

     return jday - jday_01012000;
}

/*+++++++++++++++++++++++++
.IDENTifer   Fill_TOSOMI_Rec_Meta
.PURPOSE     fill meta-record in TOSOMI record
.INPUT/OUTPUT
  call as   Fill_TOSOMI_Rec_Meta( const struct tosomi_hdr *hdr, 
			   struct tosomi_rec *rec )
     input:
            struct tosomi_hdr *hdr :  name of the TOSOMI product
 in/output:
            struct tosomi_rec *rec :  data records from the TOSOMI product

.RETURNS     nothing, error status is passed by global nadc_stat
.COMMENTS    static function
-------------------------*/
static
void Fill_TOSOMI_Rec_Meta( const struct tosomi_hdr *hdr, 
			   struct tosomi_rec *rec )
{
     register unsigned int   nr = 0;
     register unsigned short ns = 0;

     do {
	  register unsigned short numObs = 0;

	  do {
	       if ( nr >= hdr->numRec ) break;
	       rec[nr].meta.stateID = hdr->state_list[ns].stateID;
	       rec[nr].meta.intg_time = hdr->state_list[ns].intg_time;
	  } while ( ++nr, ++numObs < hdr->state_list[ns].numObs );
     } while ( ++ns, nr < hdr->numRec );
}

/*+++++++++++++++++++++++++
.IDENTifer   Read_TOSOMI_Header
.PURPOSE     read header of (ascii) KNMI TOSOMI product
.INPUT/OUTPUT
  call as   Read_TOSOMI_Header( flname, hdr );
     input:
            char flname[]        :  name of the TOSOMI product
    output:
            struct tosomi_hdr *hdr :  header info from TOSOMI product

.RETURNS     nothing, error status is passed by global nadc_stat
.COMMENTS    static function
-------------------------*/
static
void Read_TOSOMI_Header( const char *flname, struct tosomi_hdr *hdr )
   /*@globals  errno, nadc_stat, nadc_err_stack;@*/
   /*@modifies errno, nadc_stat, nadc_err_stack, hdr@*/
{
     const char prognm[] = "Read_TOSOMI_Header";

     char  *cpntr, line[MAX_LINE_LENGTH];

     gzFile fp;
     bool   gzDirect;
/*
 * open/read TOSOMI product
 */
     if ( (fp = gzopen( flname, "r" )) == NULL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, flname );
     gzDirect = (gzdirect(fp) == 1) ? TRUE : FALSE;

     do {
	  if ( gzeof ( fp ) == 1 ) break;
	  if ( gzgets( fp, line, MAX_LINE_LENGTH ) == Z_NULL ) {
	       if ( gzDirect ) break;
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, flname );
	  }
	  hdr->file_size += strlen( line );

          if ( *line != '#' ) {
	       hdr->numRec++;
	  } else if ( strncmp( line+2, "TOSOMI", 6 ) == 0 ) {
	       if ( (cpntr = strchr( line, ':' )) != NULL ) {
		    NADC_STRIP_ALL( cpntr+1, hdr->software_version );
	       }
	  } else if ( strncmp( line+2, "Level 2 date/time", 17 ) == 0 ) {
	       if ( (cpntr = strchr( line, ':' )) != NULL ) {
		    NADC_STRIP_ALL( cpntr+1, hdr->creation_date );
		    (void) memmove( hdr->creation_date+9,
				    hdr->creation_date+8, 7 );
		    hdr->creation_date[8] = 'T';
	       }
	  } else if ( strncmp( line+2, "Level 1 version", 15 ) == 0 ) {
	       cpntr = line + strlen(line) - 1;
	        /* skip trailing spaces*/
	       while ( --cpntr > line && *cpntr == ' ' );
	       /* read software version */ 
	       while ( --cpntr > line && *cpntr != ' ' );
	       NADC_STRIP_ALL( cpntr, hdr->lv1c_version );
	  } else if ( strncmp( line+2, "Level 1 filename", 16 ) == 0 ) {
	       if ( (cpntr = strchr( line, ':' )) != NULL ) {
		    NADC_STRIP_ALL( cpntr+1, hdr->l1b_product );
	       }
	  } else if ( strncmp( line+2, "State id", 8 ) == 0 ) {
	       if ( (cpntr = strrchr( line, ':' )) != NULL ) {
		    hdr->state_list[hdr->numState].numObs = (unsigned short) 
			 strtoul(cpntr+1, (char **) NULL, 10);
		    *cpntr = '\0';
	       }
	       if ( (cpntr = strrchr( line, ':' )) != NULL ) {
		    hdr->state_list[hdr->numState].intg_time = (unsigned char) 
			 NINT( 16 * strtof( cpntr+1, (char **)NULL ) );
		    *cpntr = '\0';
	       }
	       if ( (cpntr = strrchr( line, ':' )) != NULL ) {
		    hdr->state_list[hdr->numState].stateID = (unsigned char) 
			 strtoul(cpntr+1, (char **) NULL, 10);
	       }
	       if ( hdr->numState == MAX_NUM_TOSOMI_STATES ) {
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
				  "increase value for max_num_tosomi_states" );
	       }
	       hdr->numState++;
	  } else {
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, line );
	  }
     } while( TRUE );
     hdr->numProd = 1;            /* read a complete products without errors */
 done:
     gzclose( fp );

     if ( hdr->numRec == 0u )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_WARN, "empty product" );
     if ( hdr->numState == 0u )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "outdated header" );
}

/*+++++++++++++++++++++++++
.IDENTifer   Read_TOSOMI_Record
.PURPOSE     read data records from (ascii) KNMI TOSOMI products
.INPUT/OUTPUT
  call as   nline = Read_TOSOMI_Record( fp, rec );
     input:
            char flname[]        :  name of the TOSOMI product
    output:
            struct tosomi_rec *rec :  data records from the TOSOMI product

.RETURNS     number of lines erad from product (unsigned short)
.COMMENTS    static function
-------------------------*/
static
unsigned int Read_TOSOMI_Records( const char *flname, 
				  struct tosomi_rec *rec )
   /*@globals  errno, nadc_stat, nadc_err_stack;@*/
   /*@modifies errno, nadc_stat, nadc_err_stack, rec@*/
{
     const char prognm[] = "Read_TOSOMI_Records";

     char   tmp_date[9], tmp_time[11], line[MAX_LINE_LENGTH];
     int    numItems, sza, vza, *lon;

     gzFile fp;

     register unsigned int   nr, numRec = 0u;
/*
 * open/read TOSOMI product
 */
     if ( (fp = gzopen( flname, "r" )) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_FILE, flname );
	  return 0;
     }
/*
 * read TOSOMI records from product
 */
     do {
	  if ( gzeof ( fp ) == 1 ) break;
	  if ( gzgets( fp, line, MAX_LINE_LENGTH ) == Z_NULL ) break;
	  if ( *line == '#' ) continue;

	  lon = rec->lon_corner;
	  numItems = sscanf( line, FORMAT_RECORD, 
			     tmp_date, tmp_time, 
			     lon+1, rec->lat_corner+1, 
			     lon  , rec->lat_corner, 
			     lon+2, rec->lat_corner+2, 
			     lon+3, rec->lat_corner+3, 
			     &rec->lon_center, &rec->lat_center, 
			     &rec->meta.pixelType, 
			     &rec->vcd, &rec->vcdError,
			     &rec->vcdRaw, &rec->scd, &sza, &vza,
			     &rec->meta.cloudFraction, 
			     &rec->meta.cloudTopPress, 
			     &rec->meta.radWeight, 
			     &rec->meta.amfSky, &rec->meta.amfCloud );

	  if ( numItems != NUM_COLUMNS ) {
	       char msg[80];
	       (void) snprintf( msg, 80, "incomplete record[%-u]\n", numRec );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, msg );
	  }
	  rec->jday = DateTime2SciaJDAY( tmp_date, tmp_time );
	  rec->meta.sza = sza / 100.f;
	  rec->meta.vza = vza / 100.f;
	  rec->lon_center = ILON_IN_RANGE(rec->lon_center);
	  for ( nr = 0; nr < NUM_CORNERS; nr++ ) {
	       lon[nr] = ILON_IN_RANGE( lon[nr] );
	       if ( abs(rec->lon_center - lon[nr]) > 18000 ) {
		    if ( rec->lon_center > 0 )
			 lon[nr] += 36000;
		    else
			 lon[nr] -= 36000;
	       }
	  }
	  ++rec;
	  ++numRec;
     } while ( TRUE );
done:
     gzclose( fp );
     return numRec;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int NADC_RD_TOSOMI( const char *flname, struct tosomi_hdr *hdr,
			     struct tosomi_rec **tosomi_out )
{
     const char prognm[] = "NADC_RD_TOSOMI";

     register unsigned short num;

     char  *cpntr, ctemp[SHORT_STRING_LENGTH];

     struct tosomi_rec *tosomi;
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
 * initialize TOSOMI header structure
 */
     NADC_RECEIVEDATE( flname, hdr->receive_date );
     (void) nadc_strlcpy( hdr->product, ctemp, 26 );
     (void) strcpy( hdr->creation_date, "" );
     (void) strcpy( hdr->software_version, "" );
     (void) strcpy( hdr->lv1c_version, "" );
     (void) strcpy( hdr->validity_start, "00000000T000000" );
     (void) strcpy( hdr->validity_stop , "99999999T999999" );
     hdr->numState  = 0;
     hdr->numRec    = 0u;
     hdr->file_size = 0u;
     for ( num = 0; num < MAX_NUM_TOSOMI_STATES; num++ )
	  hdr->state_list[num].numObs = 0;
/*
 * read header from TOSOMI product
 */
     Read_TOSOMI_Header( flname, hdr );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, "corrupted header" );
     if ( hdr->numRec == 0u ) goto done;
/*
 * allocate enough space to store all records
 */
     tosomi = (struct tosomi_rec *) 
	  malloc( hdr->numRec * sizeof( struct tosomi_rec ));
     if ( tosomi == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "tosomi" );
/*
 * read data records
 */
     if ( Read_TOSOMI_Records( flname, tosomi ) != hdr->numRec ) {
	  free ( tosomi );
	  tosomi_out[0] = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, 
			   "failed to read all records" );
     }
     Fill_TOSOMI_Rec_Meta( hdr, tosomi );
     SciaJDAY2adaguc( tosomi->jday, hdr->validity_start );
     SciaJDAY2adaguc( tosomi[hdr->numRec-1].jday, hdr->validity_stop );
     tosomi_out[0] = tosomi;
 done:
     return hdr->numRec;
}
