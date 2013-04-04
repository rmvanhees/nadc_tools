/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2007 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_RD_FRESCO
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME/SCIA Fresco KNMI
.LANGUAGE    ANSI C
.PURPOSE     read Fresco products
.INPUT/OUTPUT
  call as   nrec = NADC_RD_FRESCO( flname, &hdr, &rec );
     input:  
             char   fresco_name[]    : filename of Fresco product
    output:  
             struct fresco_hdr *hdr  : header of Fresco product
             struct fresco_rec **rec : Fresco tile information

.RETURNS     number of records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION     1.2     17-Sep-2012   fixed code for reading the data, RvH
             1.1     25-Jun-2008   update to Fresco+, 
                                   added more sanity checks, RvH
             1.0     14-Feb-2007   initial release by R. M. van Hees
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
#include <nadc_fresco.h>

/*+++++ Macros +++++*/
#define NINT(a) ((a) >= 0.f ? (int)((a)+0.5) : (int)((a)-0.5))

#define MAX_LINE_LENGTH 256
#define NUM_COLUMNS      28
#define FORMAT_RECORD \
"%8s %11s %2hhu %8f %8f %8f %8f %9f %9f %9f %9f %9f %10f \
 %8f %8f %8f %8f %8f %8f %8f %8f %8f %8f %10f %2hhu %9f %9f %9f"

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
static const double jday_01011950 = 2433282.5;
static const double jday_01012000 = 2451544.5;

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
double DateTime2GomeJDAY( const char *date, const char *time )
{
     unsigned int imin, ihour, iday, imon, iyear;
     double       day, jday, sec;
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

     return jday - jday_01011950;
}

static inline
double DateTime2SciaJDAY( const char *date, const char *time )
{
     unsigned int imin, ihour, iday, imon, iyear;
     double       day, jday, sec;
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

static
void Read_Fresco_Header( const char *flname, struct fresco_hdr *hdr )
{
     const char prognm[] = "Read_Fresco_Header";

     char  *cpntr, line[MAX_LINE_LENGTH];

     gzFile fp;
/*
 * open/read Fresco product
 */
     if ( (fp = gzopen( flname, "r" )) == NULL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, flname );

     do {
	  *line = '\0';
	  if ( gzgets( fp, line, MAX_LINE_LENGTH ) == Z_NULL ) {
	       if ( gzeof ( fp ) == 1 ) break;
	       if ( strlen( line ) )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, flname );
	  }
	  hdr->file_size += strlen( line );

          if ( *line != '#' ) {
	       hdr->numRec++;
	  } else if ( strncmp( line+2, "FRESCO", 6 ) == 0 ) {
	       if ( (cpntr = strchr( line, ':' )) != NULL ) {
		    NADC_STRIP_ALL( cpntr+1, hdr->software_version );
		    if ( strncmp( hdr->software_version, "GO", 2 ) == 0 )
			 (void) strlcpy( hdr->source, "GOME", 5 );
		    else
			 (void) strlcpy( hdr->source, "SCIA", 5 );
	       } else
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
				     "corrupted header at FRESCO+ version" );
	  } else if ( strncmp( line+2, "Level 2 date/time", 17 ) == 0 ) {
	       if ( (cpntr = strchr( line, ':' )) != NULL ) {
		    NADC_STRIP_ALL( cpntr+1, hdr->creation_date );
		    (void) memmove( hdr->creation_date+9,
				    hdr->creation_date+8, 7 );
		    hdr->creation_date[8] = 'T';
	       } else
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
				     "corrupted header at Level 2 date/time" );
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
	       } else
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
				     "corrupted header at Level 1 filename" );
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
	       if ( hdr->numState == MAX_NUM_FRESCO_STATES ) {
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, 
				  "increase value for max_num_fresco_states" );
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

static
unsigned int Read_Fresco_Records( const char *flname, 
				  const struct fresco_hdr *hdr,
				  struct fresco_rec *fresco )
{
     const char prognm[] = "Read_Fresco_Records";

     char  tmp_date[9], tmp_time[11];
     char  line[MAX_LINE_LENGTH];
     int   numItems;
     float *lon;

     register unsigned short numState = 0;
     register unsigned int   nr, numRec = 0;

     gzFile fp;
/*
 * open/read Fresco product
 */
     if ( (fp = gzopen( flname, "r" )) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_FILE, flname );
	  return 0;
     }
/*
 * read Fresco records from product
 */
     do {
	  *line = '\0';
	  if ( gzgets( fp, line, MAX_LINE_LENGTH ) == Z_NULL ) {
	       if ( gzeof ( fp ) == 1 ) break;
	       if ( strlen( line ) )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, flname );
	  }
	  if ( *line == '#' ) continue;

	  lon = fresco[numRec].lon_corner;
	  numItems = sscanf( line, FORMAT_RECORD, tmp_date, tmp_time,
			     &fresco[numRec].meta.pixelType, 
			     &fresco[numRec].lat_corner[2],
			     &fresco[numRec].lat_corner[3],
			     &fresco[numRec].lat_corner[1],
			     &fresco[numRec].lat_corner[0],
			     &fresco[numRec].lat_center,
			     &lon[2], &lon[3], &lon[1], &lon[0],
			     &fresco[numRec].lon_center, 
			     &fresco[numRec].meta.lza, 
			     &fresco[numRec].meta.sza,
			     &fresco[numRec].meta.raa,
			     &fresco[numRec].cloudFraction, 
			     &fresco[numRec].cloudFractionError, 
			     &fresco[numRec].cloudTopHeight, 
			     &fresco[numRec].cloudAlbedo, 
			     &fresco[numRec].cloudAlbedoError, 
			     &fresco[numRec].surfaceAlbedo, 
			     &fresco[numRec].surfaceHeight, 
			     &fresco[numRec].meta.chisq, 
			     &fresco[numRec].meta.errorFlag, 
			     &fresco[numRec].cloudTopPress, 
			     &fresco[numRec].cloudTopPressError,
			     &fresco[numRec].groundPress );

	  if ( numItems != NUM_COLUMNS ) {
	       char msg[80];
	       (void) snprintf( msg, 80, "incomplete record[%-u]\n", numRec );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, msg );
	  }
	  if ( strncmp(hdr->source, "GOME", 4) == 0 )
	       fresco[numRec].jday = DateTime2GomeJDAY( tmp_date, tmp_time );
	  else
	       fresco[numRec].jday = DateTime2SciaJDAY( tmp_date, tmp_time );

          fresco[numRec].lon_center = LON_IN_RANGE(fresco[numRec].lon_center);
          for ( nr = 0; nr < NUM_CORNERS; nr++ ) {
               lon[nr] = LON_IN_RANGE( lon[nr] );
               if ( fabsf(fresco[numRec].lon_center - lon[nr]) > 180.f ) {
                    if ( fresco[numRec].lon_center > 0.f )
                         lon[nr] += 360.f;
                    else
                         lon[nr] -= 360.f;
               }
          }
	  /* convert heigth to meters, instead of km */
	  fresco[numRec].cloudTopHeight *= 1e3;
	  fresco[numRec].surfaceHeight *= 1e3;

	  ++numRec;
     } while ( TRUE );

     nr = 0;
     do {
	  register unsigned short numObs = 0;

	  do {
	       if ( nr >= numRec ) break;
	       fresco[nr].meta.stateID = hdr->state_list[numState].stateID;
	       fresco[nr].meta.intg_time = hdr->state_list[numState].intg_time;
	  } while ( ++nr, ++numObs < hdr->state_list[numState].numObs );
	  numState++;
     } while ( nr < numRec );
done:
     gzclose( fp );
     return numRec;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int NADC_RD_FRESCO( const char *flname, struct fresco_hdr *hdr,
			     struct fresco_rec **fresco_out )
{
     const char prognm[] = "NADC_RD_FRESCO";

     register unsigned short num;

     char  *cpntr, ctemp[SHORT_STRING_LENGTH];

     struct fresco_rec *fresco;
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
 * initialize Fresco header structure
 */
     NADC_RECEIVEDATE( flname, hdr->receive_date );
     (void) strlcpy( hdr->product, ctemp, 26 );
     (void) strcpy( hdr->creation_date, "" );
     (void) strcpy( hdr->software_version, "" );
     (void) strcpy( hdr->lv1c_version, "" );
     (void) strcpy( hdr->validity_start, "00000000T000000" );
     (void) strcpy( hdr->validity_stop , "99999999T999999" );
     hdr->numState  = 0;
     hdr->numRec    = 0u;
     hdr->file_size = 0u;
     for ( num = 0; num < MAX_NUM_FRESCO_STATES; num++ )
	  hdr->state_list[num].numObs = 0;
/*
 * read header from Fresco product
 */
     Read_Fresco_Header( flname, hdr );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, "corrupted header" );
     if ( hdr->numRec == 0u ) goto done;
/*
 * allocate enough space to store all records
 */
     fresco = (struct fresco_rec *) 
	  malloc( hdr->numRec * sizeof( struct fresco_rec ));
     if ( fresco == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "fresco" );
/*
 * read data records
 */
     if ( Read_Fresco_Records( flname, hdr, fresco ) != hdr->numRec ) {
	  free ( fresco );
	  fresco_out[0] = NULL;
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE_RD, 
			   "failed to read all records" );
     }
     if ( strncmp(hdr->source, "GOME", 4) == 0 ) {
	  GomeJDAY2adaguc( fresco->jday, hdr->validity_start );
	  GomeJDAY2adaguc( fresco[hdr->numRec-1].jday, hdr->validity_stop );
     } else {
	  SciaJDAY2adaguc( fresco->jday, hdr->validity_start );
	  SciaJDAY2adaguc( fresco[hdr->numRec-1].jday, hdr->validity_stop );
     }
     fresco_out[0] = fresco;
 done:
     return hdr->numRec;
}
