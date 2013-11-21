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

.IDENTifer   NADC_RD_TOGOMI
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME TOGOMI KNMI
.LANGUAGE    ANSI C
.PURPOSE     read TOGOMI products
.INPUT/OUTPUT
  call as   nrec = NADC_RD_TOGOMI( flname, &hdr, &rec );
     input:  
             char   togomi_name[]    : filename of TOGOMI product
    output:  
             struct togomi_hdr *hdr  : header of TOGOMI product
             struct togomi_rec **rec : TOGOMI tile information

.RETURNS     number of records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION     1.1     30-Oct-2008   added check on longitudes, RvH
             1.0     06-Oct-2008   initial release by R. M. van Hees
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
#include <math.h>
#include <zlib.h>

/*+++++ Local Headers +++++*/
#include <nadc_togomi.h>

/*+++++ Macros +++++*/
#define NINT(a) ((a) >= 0.f ? (int)((a)+0.5) : (int)((a)-0.5))

#define MAX_LINE_LENGTH 256
#define NUM_COLUMNS      25
#define FORMAT_RECORD \
"%s %s %hhu %hhu %f %f %f %f %f %f %f %f %f %f \
 %f %f %f %f %f %f %f %f %f %f %f"

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

/*+++++++++++++++++++++++++
.IDENTifer   Fill_TOGOMI_Rec_Meta
.PURPOSE     fill meta-record in TOGOMI record
.INPUT/OUTPUT
  call as   Fill_TOGOMI_Rec_Meta( const struct togomi_hdr *hdr, 
			   struct togomi_rec *rec )
     input:
            struct togomi_hdr *hdr :  name of the TOGOMI product
 in/output:
            struct togomi_rec *rec :  data records from the TOGOMI product

.RETURNS     nothing, error status is passed by global nadc_stat
.COMMENTS    static function
-------------------------*/
static
void Fill_TOGOMI_Rec_Meta( const struct togomi_hdr *hdr, 
			   struct togomi_rec *rec )
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
.IDENTifer   Read_TOGOMI_Header
.PURPOSE     read header of (ascii) KNMI TOGOMI product
.INPUT/OUTPUT
  call as   Read_TOGOMI_Header( flname, hdr );
     input:
            char flname[]          :  name of the TOGOMI product
    output:
            struct togomi_hdr *hdr :  header info from TOGOMI product

.RETURNS     nothing, error status is passed by global nadc_stat
.COMMENTS    static function
-------------------------*/
static
void Read_TOGOMI_Header( const char *flname, struct togomi_hdr *hdr )
   /*@globals  errno, nadc_stat, nadc_err_stack;@*/
   /*@modifies errno, nadc_stat, nadc_err_stack, hdr@*/
{
     const char prognm[] = "Read_TOGOMI_Header";

     char  *cpntr, line[MAX_LINE_LENGTH];

     gzFile fp;
     bool   gzDirect;
/*
 * open/read TOGOMI product
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
	  } else if ( strncmp( line+2, "TOGOMI", 6 ) == 0 ) {
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
	       if ( hdr->numState == MAX_NUM_TOGOMI_STATES ) {
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
				  "increase value for max_num_togomi_states" );
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
.IDENTifer   Read_TOGOMI_Record
.PURPOSE     read data records from (ascii) KNMI TOGOMI products
.INPUT/OUTPUT
  call as   nline = Read_TOGOMI_Record( fp, rec );
     input:
            char flname[]          :  name of the TOGOMI product
    output:
            struct togomi_rec *rec :  data records from the TOGOMI product

.RETURNS     number of lines erad from product (unsigned short)
.COMMENTS    static function
-------------------------*/
static
unsigned int Read_TOGOMI_Records( const char *flname, 
				  struct togomi_rec *rec )
   /*@globals  errno, nadc_stat, nadc_err_stack;@*/
   /*@modifies errno, nadc_stat, nadc_err_stack, rec@*/
{
     const char prognm[] = "Read_TOGOMI_Records";

     char tmp_date[12], tmp_time[13], dateTime[DATE_STRING_LENGTH], 
	  line[MAX_LINE_LENGTH];

     int          numItems;
     unsigned int utc_day, utc_msec;

     gzFile fp;

     register unsigned int nr, numRec = 0;

     const double mSecPerDay = 1e3 * 24. * 60. * 60.;
/*
 * open/read TOGOMI product
 */
     if ( (fp = gzopen( flname, "r" )) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_FILE, flname );
	  return 0;
     }
/*
 * read TOGOMI records from product
 */
     do {
	  if ( gzeof ( fp ) == 1 ) break;
	  if ( gzgets( fp, line, MAX_LINE_LENGTH ) == Z_NULL ) break;
	  if ( *line == '#' ) continue;

	  numItems = sscanf( line, FORMAT_RECORD, 
			     tmp_date, tmp_time,
			     &rec->meta.pixelType, &rec->meta.swathType, 
			     &rec->lat_corner[2], &rec->lat_corner[3], 
			     &rec->lat_corner[1], &rec->lat_corner[0], 
			     &rec->lat_center, 
			     &rec->lon_corner[2], &rec->lon_corner[3], 
			     &rec->lon_corner[1], &rec->lon_corner[0], 
			     &rec->lon_center,
			     &rec->meta.sza, 
			     &rec->scd, &rec->scdError, 
			     &rec->meta.effTemp,
			     &rec->vcd, &rec->vcdError, 
			     &rec->meta.cloudFraction, &rec->meta.cloudHeight, 
			     &rec->meta.amfSky, &rec->meta.amfCloud,
			     &rec->meta.ghostColumn );
	  if ( numItems != NUM_COLUMNS ) {
	       char msg[80];
	       (void) snprintf( msg, 80, "incomplete record[%-u]\n", numRec );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, msg );
	  }
	  (void) snprintf( dateTime, DATE_STRING_LENGTH,
			   "%s %s", tmp_date, tmp_time );
	  ASCII_2_UTC( dateTime, &utc_day, &utc_msec );
	  rec->jday = utc_day + utc_msec / mSecPerDay;

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
     } while ( TRUE );
done:
     gzclose( fp );
     return numRec;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int NADC_RD_TOGOMI( const char *flname, struct togomi_hdr *hdr,
			     struct togomi_rec **togomi_out )
{
     const char prognm[] = "NADC_RD_TOGOMI";

     register unsigned short num;

     char  *cpntr, ctemp[SHORT_STRING_LENGTH];

     struct togomi_rec *togomi;
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
 * initialize TOGOMI header structure
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
     for ( num = 0; num < MAX_NUM_TOGOMI_STATES; num++ )
	  hdr->state_list[num].numObs = 0;
/*
 * read header from TOGOMI product
 */
     Read_TOGOMI_Header( flname, hdr );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, "corrupted header" );
     if ( hdr->numRec == 0 ) goto done;
/*
 * allocate enough space to store all records
 */
     togomi = (struct togomi_rec *) 
	  malloc( hdr->numRec * sizeof( struct togomi_rec ));
     if ( togomi == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "togomi" );
/*
 * read data records
 */
     if ( Read_TOGOMI_Records( flname, togomi ) != hdr->numRec ) {
	  free ( togomi );
	  togomi_out[0] = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, 
			   "failed to read all records" );
     }
     Fill_TOGOMI_Rec_Meta( hdr, togomi );
     GomeJDAY2adaguc( togomi->jday, hdr->validity_start );
     GomeJDAY2adaguc( togomi[hdr->numRec-1].jday, hdr->validity_stop );

     togomi_out[0] = togomi;
 done:
     return hdr->numRec;
}
