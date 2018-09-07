/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 1999 - 2018 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_WR_ASCII
.AUTHOR      R.M. van Hees
.KEYWORDS    ascii dump
.LANGUAGE    ANSI C
.PURPOSE     display keyword and values uniformly
.COMMENTS    contains CRE_ASCII_FILE, CAT_ASCII_FILE, nadc_write_header,
             nadc_write_xxx, nadc_write_arr_xxx
.ENVIRONment uses the function snprintf (BSD 4.4) and ISO C99 extension
.VERSION     2.1     16-Mar-2004   renamed "nl_dc_" to "nadc_", RvH
             2.0     31-Oct-2001   moved to new Error handling routines, RvH
             1.2     14-Dec-2000   Add negative dimension to flip the axis of
                                   an array, RvH
             1.1     29-Aug-2000   More checks on errors, RvH
             1.0     16-Aug-2000   Created by R. M. van Hees
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
#include <time.h>
#include <math.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

#define FIELD_SEPARATOR   "|"

/*+++++++++++++++++++++++++ Exported Functions +++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   CRE_ASCII_FILE
.PURPOSE     create new file for writing (or truncate to zero length)
.INPUT/OUTPUT
  call as   outfile = CRE_ASCII_File( fl_name, extension );

     input:  
            char fl_name[]   :  name of output file (no extension!!!)
	    char extension[] :  extension to be concatenated (without ".")

.RETURNS     (open) stream pointer
.COMMENTS    none
-------------------------*/
FILE *CRE_ASCII_File( const char filebase[], const char ext[] )
{
     char   flname[MAX_STRING_LENGTH];
     FILE   *outfl = NULL;
/*
 * check size of output filename
 */
     if ( (strlen(filebase) + strlen(ext)) > MAX_STRING_LENGTH ) {
          (void) snprintf( flname, MAX_STRING_LENGTH,
			   "Filename too long (max: %d)\n", 
			   (int) MAX_STRING_LENGTH );
	  NADC_GOTO_ERROR( NADC_ERR_FILE, flname );
     }
     (void) snprintf( flname, MAX_STRING_LENGTH, "%s.%s", filebase, ext );
     if ( (outfl = fopen( flname, "w" )) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE_CRE, flname);
 done:
     return outfl;
}
/*+++++++++++++++++++++++++
.IDENTifer   CAT_ASCII_FILE
.PURPOSE     Open file for writing in append mode (create if non-existing)
.INPUT/OUTPUT
  call as   outfile = CAT_ASCII_File( fl_name, extension );

     input:  
            char fl_name[]   :  name of output file (no extension!!!)
	    char extension[] :  extension to be concatenated (without ".")

.RETURNS     (open) stream pointer
.COMMENTS    none
-------------------------*/
FILE *CAT_ASCII_File( const char filebase[], const char ext[] )
{
     char   flname[MAX_STRING_LENGTH];
     FILE   *outfl = NULL;
/*
 * check size of output filename
 */
     if ( (strlen( filebase) + strlen( ext )) > MAX_STRING_LENGTH ) {
          (void) snprintf( flname, MAX_STRING_LENGTH,
			   "Filename too long (max: %d)\n", 
			   (int) MAX_STRING_LENGTH );
	  NADC_GOTO_ERROR( NADC_ERR_FILE, flname );
     }
     (void) snprintf( flname, MAX_STRING_LENGTH, "%s.%s", filebase, ext );
     if ( (outfl = fopen( flname, "a")) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_FILE_CRE, flname);
 done:
     return outfl;
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_text
.PURPOSE     write keyword and value in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_text( fp, key_num, key_wrd, key_val );

     input:
            FILE *fp               :   file pointer
            unsigned int  key_num  :   (unique) number for this keyword
            char key_wrd           :   keyword description 
            char *key_val          :   keyword value

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_text( FILE *fp, unsigned int key_num, const char key_wrd[], 
		      const char key_val[] )
{
     int  nr_write;

     nr_write = fprintf( fp, "%3u %s %-33s %s %s\n", key_num,
			 FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, key_val );
     if ( nr_write < 0 ) NADC_RETURN_ERROR( NADC_ERR_FILE_WR, key_wrd );
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_bool
.PURPOSE     write keyword and value in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_bool( fp, key_num, key_wrd, key_val );

     input:
            FILE *fp               :   file pointer
            unsigned int  key_num  :   (unique) number for this keyword
            char key_wrd           :   keyword description 
            bool key_val           :   keyword value

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_bool( FILE *fp, unsigned int key_num, const char key_wrd[], 
		      bool key_val )
{
     int  nr_write;

     if ( key_val )
	  nr_write = fprintf( fp, "%3u %s %-33s %s TRUE\n", key_num,
			      FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR );
     else
	  nr_write = fprintf( fp, "%3u %s %-33s %s FALSE\n", key_num,
			      FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR );
     if ( nr_write < 0 ) NADC_RETURN_ERROR( NADC_ERR_FILE_WR, key_wrd );
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_schar
.PURPOSE     write keyword and value in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_schar( fp, key_num, key_wrd,  key_val );

     input:
            FILE *fp               :   file pointer
            unsigned int  key_num  :   (unique) number for this keyword
            char key_wrd           :   keyword description 
            signed char  key_val   :   keyword value

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_schar( FILE *fp, unsigned int key_num, const char key_wrd[], 
		       signed char key_val )
{
     int  nr_write;

     nr_write = fprintf( fp, "%3u %s %-33s %s % 3d\n", key_num,
			 FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			 (int) key_val );
     if ( nr_write < 0 ) NADC_RETURN_ERROR( NADC_ERR_FILE_WR, key_wrd );
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_uchar
.PURPOSE     write keyword and value in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_uchar( fp, key_num, key_wrd,  key_val );

     input:
            FILE *fp               :   file pointer
            unsigned int  key_num  :   (unique) number for this keyword
            char key_wrd           :   keyword description 
            unsigned char  key_val :   keyword value

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_uchar( FILE *fp, unsigned int key_num, const char key_wrd[], 
		       unsigned char key_val )
{
     int  nr_write;

     nr_write = fprintf( fp, "%3u %s %-33s %s %3u\n", key_num,
			 FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			 (unsigned int) key_val );
     if ( nr_write < 0 ) NADC_RETURN_ERROR( NADC_ERR_FILE_WR, key_wrd );
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_short
.PURPOSE     write keyword and value in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_short( fp, key_num, key_wrd,  key_val );

     input:
            FILE *fp               :   file pointer
            unsigned int  key_num  :   (unique) number for this keyword
            char key_wrd           :   keyword description 
            short  key_val         :   keyword value

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_short( FILE *fp, unsigned int key_num, const char key_wrd[], 
		       short key_val )
{
     int  nr_write;

     nr_write = fprintf( fp, "%3u %s %-33s %s %5hd\n", key_num,
			 FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, key_val );
     if ( nr_write < 0 ) NADC_RETURN_ERROR( NADC_ERR_FILE_WR, key_wrd );
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_ushort
.PURPOSE     write keyword and value in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_ushort( fp, key_num, key_wrd,  key_val );

     input:
            FILE *fp               :   file pointer
            unsigned int  key_num  :   (unique) number for this keyword
            char key_wrd           :   keyword description 
            unsigned short key_val :   keyword value

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_ushort( FILE *fp, unsigned int key_num, const char key_wrd[], 
			unsigned short key_val )
{
     int  nr_write;

     nr_write = fprintf( fp, "%3u %s %-33s %s %5hu\n", key_num,
			 FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, key_val );
     if ( nr_write < 0 ) NADC_RETURN_ERROR( NADC_ERR_FILE_WR, key_wrd );
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_int
.PURPOSE     write keyword and value in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_int( fp, key_num, key_wrd,  key_val );

     input:
            FILE *fp               :   file pointer
            unsigned int  key_num  :   (unique) number for this keyword
            char key_wrd           :   keyword description 
            int key_val            :   keyword value

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_int( FILE *fp, unsigned int key_num, const char key_wrd[], 
		     int key_val )
{
     int  nr_write;

     nr_write = fprintf( fp, "%3u %s %-33s %s %10d\n", key_num,
			 FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, key_val );
     if ( nr_write < 0 ) NADC_RETURN_ERROR( NADC_ERR_FILE_WR, key_wrd );
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_uint
.PURPOSE     write keyword and value in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_uint( fp, key_num, key_wrd,  key_val );

     input:
            FILE *fp               :   file pointer
            unsigned int  key_num  :   (unique) number for this keyword
            char key_wrd           :   keyword description 
            unsigned int key_val   :   keyword value

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_uint( FILE *fp, unsigned int key_num, const char key_wrd[], 
		      unsigned int key_val )
{
     int  nr_write;

     nr_write = fprintf( fp, "%3u %s %-33s %s %10u\n", key_num,
			 FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, key_val );
     if ( nr_write < 0 ) NADC_RETURN_ERROR( NADC_ERR_FILE_WR, key_wrd );
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_long
.PURPOSE     write keyword and value in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_long( fp, key_num, key_wrd,  key_val );

     input:
            FILE *fp               :   file pointer
            unsigned int  key_num  :   (unique) number for this keyword
            char key_wrd           :   keyword description 
            long key_val           :   keyword value

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_long( FILE *fp, unsigned int key_num, const char key_wrd[], 
		      long key_val )
{
     int  nr_write;

     nr_write = fprintf( fp, "%3u %s %-33s %s %10ld\n", key_num,
			 FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, key_val );
     if ( nr_write < 0 ) NADC_RETURN_ERROR( NADC_ERR_FILE_WR, key_wrd );
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_float
.PURPOSE     write keyword and value in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_float( fp, key_num, key_wrd, digits, key_val );

     input:
            FILE *fp               :   file pointer
            unsigned int  key_num  :   (unique) number for this keyword
            char key_wrd           :   keyword description 
            int  digits            :   number of digits (floating point only)
            float key_val          :   keyword value

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_float( FILE *fp, unsigned int key_num, const char key_wrd[], 
		       int digits, float key_val )
{
     char str_fmt[31];
     int  nr_write;

     (void) snprintf( str_fmt, 30, "%%3u %%s %%-33s %%s %%.%-dg\n", digits );

     nr_write = fprintf( fp, str_fmt, key_num, FIELD_SEPARATOR, key_wrd, 
			 FIELD_SEPARATOR, key_val );
     if ( nr_write < 0 ) NADC_RETURN_ERROR( NADC_ERR_FILE_WR, key_wrd );
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_double
.PURPOSE     write keyword and value in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_double( fp, key_num, key_wrd, digits, key_val );

     input:
            FILE *fp               :   file pointer
            unsigned int  key_num  :   (unique) number for this keyword
            char key_wrd           :   keyword description 
            int  digits            :   number of digits (floating point only)
            double key_val         :   keyword value

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_double( FILE *fp, unsigned int key_num, const char key_wrd[], 
			int digits, double key_val )
{
     char str_fmt[31];
     int  nr_write;

     (void) snprintf( str_fmt, 30, "%%3u %%s %%-33s %%s %%.%-dg\n", digits );

     nr_write = fprintf( fp, str_fmt, key_num, FIELD_SEPARATOR, key_wrd, 
			 FIELD_SEPARATOR, key_val );
     if ( nr_write < 0 ) NADC_RETURN_ERROR( NADC_ERR_FILE_WR, key_wrd );
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_arr_uchar
.PURPOSE     write keyword and its values in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_arr_uchar( fp, key_num, key_wrd, val_ndim, 
                                  val_count, key_val );

     input:
            FILE *fp                 :  file pointer
            unsigned int  key_num    :  (unique) number for this keyword
            char key_wrd[]           :  keyword description 
	    int  val_ndim            :  number  of dimensions
	                                Ps. a negative dimension flips the axis
	    unsigned int val_count[] :  size of the array dimensions
            unsigned char key_val[]  :  values for this keyword

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_arr_uchar( FILE *fp, unsigned int key_num, 
			   const char key_wrd[], int val_ndim, 
			   const unsigned int val_count[], 
			   const unsigned char key_val[] )
{
     register unsigned int nx, ny;

     if ( val_ndim == 1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  (void) fprintf( fp, "#" );
	  for ( nx = 0; nx < val_count[0]; nx++ )
	       (void) fprintf( fp, " %3u", (unsigned int) (*key_val++) );
	  (void) fprintf( fp, "\n" );
     } else if ( val_ndim == 2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0], val_count[1] );
	  for ( ny = 0; ny < val_count[0]; ny++ ) {
	       (void) fprintf( fp, "#" );
	       for ( nx = 0; nx < val_count[1]; nx++ )
		    (void) fprintf( fp, " %3u", 
				    (unsigned int) (*key_val++) );
	       (void) fprintf( fp, "\n" );
	  }
     } else if ( val_ndim == -1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[1][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  for ( nx = 0; nx < val_count[0]; nx++ )
	       (void) fprintf( fp, "# %3u\n", (unsigned int) (*key_val++) );
     } else if ( val_ndim == -2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[1], val_count[0] );
	  for ( nx = 0; nx < val_count[1]; nx++ ) {
	       (void) fprintf( fp, "#" );
	       for ( ny = 0; ny < val_count[0]; ny++ )
		    (void) fprintf( fp, " %3u", (unsigned int) 
				    key_val[nx + ny * val_count[0]] );
	       (void) fprintf( fp, "\n" );
	  }
     } else {
	  NADC_RETURN_ERROR( NADC_ERR_FILE_RD, key_wrd );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_arr_schar
.PURPOSE     write keyword and its values in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_arr_schar( fp, key_num, key_wrd, val_ndim, 
                                  val_count, key_val );

     input:
            FILE *fp                 :  file pointer
            unsigned int  key_num    :  (unique) number for this keyword
            char key_wrd[]           :  keyword description 
	    int  val_ndim            :  number  of dimensions
	                                Ps. a negative dimension flips the axis
	    unsigned int val_count[] :  size of the array dimensions
            signed char key_val[]    :  values for this keyword

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_arr_schar( FILE *fp, unsigned int key_num, 
			   const char key_wrd[], int val_ndim, 
			   const unsigned int val_count[], 
			   const signed char key_val[] )
{
     register unsigned int nx, ny;

     if ( val_ndim == 1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  (void) fprintf( fp, "#" );
	  for ( nx = 0; nx < val_count[0]; nx++ )
	       (void) fprintf( fp, " %3d", (int) (*key_val++) );
	  (void) fprintf( fp, "\n" );
     } else if ( val_ndim == 2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0], val_count[1] );
	  for ( ny = 0; ny < val_count[0]; ny++ ) {
	       (void) fprintf( fp, "#" );
	       for ( nx = 0; nx < val_count[1]; nx++ )
		    (void) fprintf( fp, " %3d", (int) (*key_val++) );
	       (void) fprintf( fp, "\n" );
	  }
     } else if ( val_ndim == -1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[1][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  for ( nx = 0; nx < val_count[0]; nx++ )
	       (void) fprintf( fp, "# %3d\n", (int) (*key_val++) );
     } else if ( val_ndim == -2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[1], val_count[0] );
	  for ( nx = 0; nx < val_count[1]; nx++ ) {
	       (void) fprintf( fp, "#" );
	       for ( ny = 0; ny < val_count[0]; ny++ )
		    (void) fprintf( fp, " %3d", 
				    (int) key_val[nx + ny * val_count[0]] );
	       (void) fprintf( fp, "\n" );
	  }
     } else {
	  NADC_RETURN_ERROR( NADC_ERR_FILE_RD, key_wrd );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_arr_short
.PURPOSE     write keyword and its values in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_arr_short( fp, key_num, key_wrd, val_ndim, 
                                  val_count, key_val );

     input:
            FILE *fp                 :  file pointer
            unsigned int  key_num    :  (unique) number for this keyword
            char key_wrd[]           :  keyword description 
	    int  val_ndim            :  number  of dimensions
	                                Ps. a negative dimension flips the axis
	    unsigned int val_count[] :  size of the array dimensions
            short key_val[]          :  values for this keyword

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_arr_short( FILE *fp, unsigned int key_num, 
			   const char key_wrd[], int val_ndim, 
			   const unsigned int val_count[], 
			   const short key_val[] )
{
     register unsigned int nx, ny;

     if ( val_ndim == 1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  (void) fprintf( fp, "#" );
	  for ( nx = 0; nx < val_count[0]; nx++ )
	       (void) fprintf( fp, " %+5hd", *key_val++ );
	  (void) fprintf( fp, "\n" );
     } else if ( val_ndim == 2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0], val_count[1] );
	  for ( ny = 0; ny < val_count[0]; ny++ ) {
	       (void) fprintf( fp, "#" );
	       for ( nx = 0; nx < val_count[1]; nx++ )
		    (void) fprintf( fp, " %+5hd", *key_val++ );
	       (void) fprintf( fp, "\n" );
	  }
     } else if ( val_ndim == -1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[1][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  for ( nx = 0; nx < val_count[0]; nx++ )
	       (void) fprintf( fp, "# %+5hd\n", *key_val++ );
     } else if ( val_ndim == -2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[1], val_count[0] );
	  for ( nx = 0; nx < val_count[1]; nx++ ) {
	       (void) fprintf( fp, "#" );
	       for ( ny = 0; ny < val_count[0]; ny++ )
		    (void) fprintf( fp, " %+5hd", 
				    key_val[nx + ny * val_count[0]] );
	       (void) fprintf( fp, "\n" );
	  }
     } else {
	  NADC_RETURN_ERROR( NADC_ERR_FILE_RD, key_wrd );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_arr_ushort
.PURPOSE     write keyword and its values in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_arr_ushort( fp, key_num, key_wrd, val_ndim, 
                                  val_count, key_val );

     input:
            FILE *fp                 :  file pointer
            unsigned int  key_num    :  (unique) number for this keyword
            char key_wrd[]           :  keyword description 
	    int  val_ndim            :  number  of dimensions
	                                Ps. a negative dimension flips the axis
	    unsigned int val_count[] :  size of the array dimensions
            unsigned short key_val[] :  values for this keyword

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_arr_ushort( FILE *fp, unsigned int key_num, 
			    const char key_wrd[], int val_ndim, 
			    const unsigned int val_count[], 
			    const unsigned short key_val[] )
{
     register unsigned int nx, ny;

     if ( val_ndim == 1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  (void) fprintf( fp, "#" );
	  for ( nx = 0; nx < val_count[0]; nx++ )
	       (void) fprintf( fp, " %5hu", *key_val++ );
	  (void) fprintf( fp, "\n" );
     } else if ( val_ndim == 2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0], val_count[1] );
	  for ( ny = 0; ny < val_count[0]; ny++ ) {
	       (void) fprintf( fp, "#" );
	       for ( nx = 0; nx < val_count[1]; nx++ )
		    (void) fprintf( fp, " %5hu", *key_val++ );
	       (void) fprintf( fp, "\n" );
	  }
     } else if ( val_ndim == -1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[1][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  for ( nx = 0; nx < val_count[0]; nx++ )
	       (void) fprintf( fp, "# %5hu\n", *key_val++ );
     } else if ( val_ndim == -2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[1], val_count[0] );
	  for ( nx = 0; nx < val_count[1]; nx++ ) {
	       (void) fprintf( fp, "#" );
	       for ( ny = 0; ny < val_count[0]; ny++ )
		    (void) fprintf( fp, " %5hu", 
				    key_val[nx + ny * val_count[0]] );
	       (void) fprintf( fp, "\n" );
	  }
     } else {
	  NADC_RETURN_ERROR( NADC_ERR_FILE_RD, key_wrd );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_arr_int
.PURPOSE     write keyword and its values in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_arr_int( fp, key_num, key_wrd, val_ndim, 
                                  val_count, key_val );

     input:
            FILE *fp                 :  file pointer
            unsigned int  key_num    :  (unique) number for this keyword
            char key_wrd[]           :  keyword description 
	    int  val_ndim            :  number  of dimensions
	                                Ps. a negative dimension flips the axis
	    unsigned int val_count[] :  size of the array dimensions
            int key_val[]            :  values for this keyword

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_arr_int( FILE *fp, unsigned int key_num, 
			 const char key_wrd[], int val_ndim, 
			 const unsigned int val_count[], const int key_val[] )
{
     register unsigned int nx, ny;

     if ( val_ndim == 1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  (void) fprintf( fp, "#" );
	  for ( nx = 0; nx < val_count[0]; nx++ )
	       (void) fprintf( fp, " %+10d", *key_val++ );
	  (void) fprintf( fp, "\n" );
     } else if ( val_ndim == 2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0], val_count[1] );
	  for ( ny = 0; ny < val_count[0]; ny++ ) {
	       (void) fprintf( fp, "#" );
	       for ( nx = 0; nx < val_count[1]; nx++ )
		    (void) fprintf( fp, " %+10d", *key_val++ );
	       (void) fprintf( fp, "\n" );
	  }
     } else if ( val_ndim == -1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[1][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  for ( nx = 0; nx < val_count[0]; nx++ )
	       (void) fprintf( fp, "# %+10d\n", *key_val++ );
     } else if ( val_ndim == -2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[1], val_count[0] );
	  for ( nx = 0; nx < val_count[1]; nx++ ) {
	       (void) fprintf( fp, "#" );
	       for ( ny = 0; ny < val_count[0]; ny++ )
		    (void) fprintf( fp, " %+10d", 
				    key_val[nx + ny * val_count[0]] );
	       (void) fprintf( fp, "\n" );
	  }
     } else {
	  NADC_RETURN_ERROR( NADC_ERR_FILE_RD, key_wrd );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_arr_uint
.PURPOSE     write keyword and its values in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_arr_uint( fp, key_num, key_wrd, val_ndim, 
                                  val_count, key_val );

     input:
            FILE *fp                 :  file pointer
            unsigned int  key_num    :  (unique) number for this keyword
            char key_wrd[]           :  keyword description 
	    int  val_ndim            :  number  of dimensions
	                                Ps. a negative dimension flips the axis
	    unsigned int val_count[] :  size of the array dimensions
            unsigned int key_val[]   :  values for this keyword

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_arr_uint( FILE *fp, unsigned int key_num, 
			  const char key_wrd[], int val_ndim, 
			  const unsigned int val_count[], 
			  const unsigned int key_val[] )
{
     register unsigned int nx, ny;

     if ( val_ndim == 1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  (void) fprintf( fp, "#" );
	  for ( nx = 0; nx < val_count[0]; nx++ )
	       (void) fprintf( fp, " %10u", *key_val++ );
	  (void) fprintf( fp, "\n" );
     } else if ( val_ndim == 2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0], val_count[1] );
	  for ( ny = 0; ny < val_count[0]; ny++ ) {
	       (void) fprintf( fp, "#" );
	       for ( nx = 0; nx < val_count[1]; nx++ )
		    (void) fprintf( fp, " %10u", *key_val++ );
	       (void) fprintf( fp, "\n" );
	  }
     } else if ( val_ndim == -1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[1][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  for ( nx = 0; nx < val_count[0]; nx++ )
	       (void) fprintf( fp, "# %10u\n", *key_val++ );
     } else if ( val_ndim == -2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[1], val_count[0] );
	  for ( nx = 0; nx < val_count[1]; nx++ ) {
	       (void) fprintf( fp, "#" );
	       for ( ny = 0; ny < val_count[0]; ny++ )
		    (void) fprintf( fp, " %10u", 
				    key_val[nx + ny * val_count[0]] );
	       (void) fprintf( fp, "\n" );
	  }
     } else {
	  NADC_RETURN_ERROR( NADC_ERR_FILE_RD, key_wrd );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_arr_float 
.PURPOSE     write keyword and its values in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_arr_float ( fp, key_num, key_wrd, val_ndim, 
                                  val_count, key_val );

     input:
            FILE *fp                 :  file pointer
            unsigned int  key_num    :  (unique) number for this keyword
            char key_wrd[]           :  keyword description 
            int  digits              :  number of digits (floating point only)
	    int  val_ndim            :  number  of dimensions
	                                Ps. a negative dimension flips the axis
	    unsigned int val_count[] :  size of the array dimensions
            float key_val[]          :  values for this keyword

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_arr_float( FILE *fp, unsigned int key_num, 
			   const char key_wrd[], int val_ndim, 
			   const unsigned int val_count[], 
			   int digits, const float key_val[] )
{
     register unsigned int nx, ny;

     char str_fmt[12];

     (void) snprintf( str_fmt, 12, " %%.%-dg", digits );
     if ( val_ndim == 1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  (void) fprintf( fp, "#" );
	  for ( nx = 0; nx < val_count[0]; nx++ )
	       (void) fprintf( fp, str_fmt, *key_val++ );
	  (void) fprintf( fp, "\n" );
     } else if ( val_ndim == 2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0], val_count[1] );

	  for ( ny = 0; ny < val_count[0]; ny++ ) {
	       (void) fprintf( fp, "#" );
	       for ( nx = 0; nx < val_count[1]; nx++ )
		    (void) fprintf( fp, str_fmt, *key_val++ );
	       (void) fprintf( fp, "\n" );
	  }
     } else if ( val_ndim == -1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[1][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  for ( nx = 0; nx < val_count[0]; nx++ ) {
	       (void) fprintf( fp, "#" );
	       (void) fprintf( fp, str_fmt, *key_val++ );
	       (void) fprintf( fp, "\n" );
	  }
     } else if ( val_ndim == -2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[1], val_count[0] );

	  for ( nx = 0; nx < val_count[1]; nx++ ) {
	       (void) fprintf( fp, "#" );
	       for ( ny = 0; ny < val_count[0]; ny++ )
		    (void) fprintf( fp, str_fmt,
				    key_val[nx + ny * val_count[0]] );
	       (void) fprintf( fp, "\n" );
	  }
     } else {
	  NADC_RETURN_ERROR( NADC_ERR_FILE_RD, key_wrd );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_arr_double
.PURPOSE     write keyword and its values in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_arr_double( fp, key_num, key_wrd, val_ndim, 
                                  val_count, key_val );

     input:
            FILE *fp                 :  file pointer
            unsigned int  key_num    :  (unique) number for this keyword
            char key_wrd[]           :  keyword description 
            int  digits              :  number of digits (floating point only)
	    int  val_ndim            :  number  of dimensions
	                                Ps. a negative dimension flips the axis
	    unsigned int val_count[] :  size of the array dimensions
            double key_val[]         :  values for this keyword

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_arr_double( FILE *fp, unsigned int key_num, 
			    const char key_wrd[], int val_ndim, 
			    const unsigned int val_count[], 
			    int digits, const double key_val[] )
{
     register unsigned int nx, ny;

     char str_fmt[12];

     (void) snprintf( str_fmt, 12, " %%.%-dg", digits );

     if ( val_ndim == 1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  (void) fprintf( fp, "#" );
	  for ( nx = 0; nx < val_count[0]; nx++ )
	       (void) fprintf( fp, str_fmt, *key_val++ );
	  (void) fprintf( fp, "\n" );
     } else if ( val_ndim == 2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0], val_count[1] );

	  for ( ny = 0; ny < val_count[0]; ny++ ) {
	       (void) fprintf( fp, "#" );
	       for ( nx = 0; nx < val_count[1]; nx++ )
		    (void) fprintf( fp, str_fmt, *key_val++ );
	       (void) fprintf( fp, "\n" );
	  }
     } else if ( val_ndim == -1 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[1][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[0] );
	  for ( nx = 0; nx < val_count[0]; nx++ ) {
	       (void) fprintf( fp, "#" );
	       (void) fprintf( fp, str_fmt, *key_val++ );
	       (void) fprintf( fp, "\n" );
	  }
     } else if ( val_ndim == -2 ) {
	  (void) fprintf( fp, "%3u %s %-33s %s array[%-u][%-u]\n", key_num,
			  FIELD_SEPARATOR, key_wrd, FIELD_SEPARATOR, 
			  val_count[1], val_count[0] );

	  for ( nx = 0; nx < val_count[1]; nx++ ) {
	       (void) fprintf( fp, "#" );
	       for ( ny = 0; ny < val_count[0]; ny++ )
		    (void) fprintf( fp, str_fmt, 
				    key_val[nx + ny * val_count[0]] );
	       (void) fprintf( fp, "\n" );
	  }
     } else {
	  NADC_RETURN_ERROR( NADC_ERR_FILE_RD, key_wrd );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_write_header
.PURPOSE     write keyword and value in uniform fashion
.INPUT/OUTPUT
  call as    nadc_write_header( fp, key_num, infl_name, component );

     input:
            FILE *fp               :   file pointer
            unsigned int key_num   :   (unique) number for this keyword
	    char infl_name[]       :   name of input file
	    char component         :   name of component

.RETURNS     nothing (check global error status)
.COMMENTS    none
-------------------------*/
void nadc_write_header( FILE *fp, unsigned int key_num, 
			const char infl_name[], const char component[] )
{
     char   string[26];
     time_t tp[1];

     nadc_write_text( fp, key_num, "Input filename", infl_name );
     nadc_write_text( fp, key_num, "Name of component", component );
     (void) time( tp );
     /* do not copy newline */
     (void) nadc_strlcpy( string, ctime( tp ), 25 );
     nadc_write_text( fp, key_num, "Creation date", string );
}
