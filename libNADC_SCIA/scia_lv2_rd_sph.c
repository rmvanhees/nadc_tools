/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2000 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV2_RD_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 data
.LANGUAGE    ANSI C
.PURPOSE     read Specific Product Header of Level 2 Product
.INPUT/OUTPUT
  call as   SCIA_LV2_RD_SPH( fd, mph, &sph );
     input:  
            FILE   *fd               : (open) stream pointer
	    struct mph_envi *mph     : main product header
    output:  
            struct sph2_scia *sph    : structure for the SPH

.RETURNS     nothing (check global error status)
.COMMENTS    None
.ENVIRONment None
.VERSION      5.2   07-Dec-2005	store lat/lon values as doubles, RvH
              5.1   12-Apr-2005	obtain SPH size from MPH, RvH
              5.0   08-Nov-2001	moved to the new Error handling routines, RvH
              4.0   01-Nov-2001	moved to new Error handling routines, RvH 
              3.0   24-Oct-2001 removed bytes_read parameter, RvH
              2.0   23-Aug-2001 moved to separate module, RvH
              1.0   13-Jan-2001 created by R. M. van Hees
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

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

#define MX_SZ_CBUFF       25

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV2_RD_SPH( FILE *fd, const struct mph_envi mph,
		      struct sph2_scia *sph )
{
     register unsigned short nm, nr;

     char   keyword[PDS_KEYWORD_LENGTH], keyvalue[PDS_KEYVAL_LENGTH];
     char   cbuff[MX_SZ_CBUFF];
     int    ibuff;
     unsigned int nbyte;

     const char prognm[] = "SCIA_LV2_RD_SPH";
     const char err_rd_pds[] = "error reading PDS header";

     const unsigned int Size_Spare_11 = 51u;
     const unsigned int Length_SPH = mph.sph_size - mph.num_dsd * mph.dsd_size;
/*
 * always rewind the file, skipping the MPH
 */
     (void) fseek( fd, (long) PDS_MPH_LENGTH, SEEK_SET );
/*
 * read keywords and values line by line
 */
     nbyte = ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
     if ( strcmp( keyword, "SPH_DESCRIPTOR" ) != 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
     (void) strlcpy( sph->descriptor, keyvalue+1, 29 );
/*
 * field 2
 */
     nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
     if ( strncmp( keyword, "STRIPLINE", 9 ) != 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
     (void) sscanf( keyvalue, "%hd", &sph->stripline );
/*
 * field 3
 */
     nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
     if ( strcmp( keyword, "SLICE_POSITION" ) != 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
     (void) sscanf( keyvalue, "%hd", &sph->slice_pos );
/*
 * field 4
 */
     nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
     if ( strcmp( keyword, "NUM_SLICES" ) != 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
     (void) sscanf( keyvalue, "%hu", &sph->no_slice );
/*
 * field 5
 */
     nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
     if ( strcmp( keyword, "START_TIME" ) != 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
     (void) strlcpy( sph->start_time, keyvalue+1, UTC_STRING_LENGTH );
/*
 * field 6
 */
     nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
     if ( strcmp( keyword, "STOP_TIME" ) != 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
     (void) strlcpy( sph->stop_time, keyvalue+1, UTC_STRING_LENGTH );
/*
 * field 7
 */
     nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
     if ( strcmp( keyword, "START_LAT" ) != 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
     (void) sscanf( keyvalue, "%d", &ibuff );
     sph->start_lat = ibuff / 1e6;
/*
 * field 8
 */
     nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
     if ( strcmp( keyword, "START_LONG" ) != 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
     (void) sscanf( keyvalue, "%d", &ibuff );
     sph->start_lon = ibuff / 1e6;
/*
 * field 9
 */
     nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
     if ( strcmp( keyword, "STOP_LAT" ) != 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
     (void) sscanf( keyvalue, "%d", &ibuff );
     sph->stop_lat = ibuff / 1e6;
/*
 * field 10
 */
     nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
     if ( strcmp( keyword, "STOP_LONG" ) != 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
     (void) sscanf( keyvalue, "%d", &ibuff );
     sph->stop_lon = ibuff / 1e6;
/*
 * field 11 (empty) + 12
 */
     nbyte += Size_Spare_11;
     nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
     if ( strcmp( keyword, "FITTING_ERROR_SUM" ) != 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
     (void) strlcpy( sph->fit_error, keyvalue+1, 5 );
/*
 * field 13
 */
     nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
     if ( strncmp( keyword, "NO_OF_DOAS_FITTING_WIN", 22 ) != 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
     (void) sscanf( keyvalue, "%hu", &sph->no_doas_win );
/*
 * field 14 -- 20
 */
     for ( nr = 0; nr < MAX_DOAS_FITTING_WIN; nr++ ) {
	  (void) snprintf(cbuff, MX_SZ_CBUFF, "DOAS_FITTING_WINDOW_%-hu", nr);
	  nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
	  if ( strcmp( keyword, cbuff ) != 0 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
	  if ( nr < sph->no_doas_win ) {
	       (void) sscanf( keyvalue+1, "%4hu", &sph->doas_win[nr].wv_min );
	       (void) sscanf( keyvalue+6, "%4hu", &sph->doas_win[nr].wv_max );
	  } else {
	       sph->doas_win[nr].wv_min = 0;
	       sph->doas_win[nr].wv_max = 0;
	  }
     }
/*
 * field 21
 */
     nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
     if ( strncmp( keyword, "NO_OF_BIAS_FITTING_WIN", 22 ) != 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
     (void) sscanf( keyvalue, "%hu", &sph->no_bias_win );
/*
 * field 22 -- 25
 */
     for ( nr = 0; nr < MAX_BIAS_FITTING_WIN; nr++ ) {
	  (void) snprintf(cbuff, MX_SZ_CBUFF, "BIAS_FITTING_WINDOW_%-hd", nr);
	  nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
	  if ( strcmp( keyword, cbuff ) != 0 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
	  if ( nr < sph->no_bias_win ) {
	       unsigned int offs = 1;

	       (void) sscanf( keyvalue+offs, "%4hu", 
			      &sph->bias_win[nr].wv_min );
	       offs += 5;
	       (void) sscanf( keyvalue+offs, "%4hu", 
			      &sph->bias_win[nr].wv_max );
	       offs += 5;
	       (void) sscanf( keyvalue+11, "%4hu", 
			      &sph->bias_win[nr].nr_micro );
	       offs += 5;
	       for ( nm = 0; nm < MAX_BIAS_MICRO_WIN; nm++ ) {
		    if ( nm < sph->bias_win[nr].nr_micro ) {
			 (void) sscanf( keyvalue+offs, "%4hu", 
					&sph->bias_win[nr].micro_min[nm] );
			 offs += 5;
			 (void) sscanf( keyvalue+offs, "%4hu", 
					&sph->bias_win[nr].micro_max[nm] );
			 offs += 5;
		    } else {
			 sph->bias_win[nr].micro_min[nm] = 0u;
			 sph->bias_win[nr].micro_max[nm] = 0u;
		    }
	       }
		    
	  } else {
	       sph->bias_win[nr].wv_min = 0;
	       sph->bias_win[nr].wv_max = 0;
	       sph->bias_win[nr].nr_micro = 0;
	  }
     }
/*
 * field 26
 */
     nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
     if ( strncmp( keyword, "NO_OF_DOAS_MOL", 14 ) != 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
     (void) sscanf( keyvalue, "%hu", &sph->no_doas_mol );
/*
 * field 27 -- 48
 */
     for ( nr = 0; nr < MAX_DOAS_SPECIES; nr++ ) {
	  (void) snprintf(cbuff, MX_SZ_CBUFF, "DOAS_MOLECULE_%02hd", nr);
	  nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
	  if ( strcmp( keyword, cbuff ) != 0 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
	  (void) strlcpy( sph->doas_mol[nr], keyvalue+1, 9 );
     }
/*
 * field 49
 */
     nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
     if ( strncmp( keyword, "NO_OF_BIAS_MOL", 14 ) != 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
     (void) sscanf( keyvalue, "%hu", &sph->no_bias_mol );
/*
 * field 50 -- 59
 */
     for ( nr = 0; nr < MAX_BIAS_SPECIES; nr++ ) {
	  (void) snprintf(cbuff, MX_SZ_CBUFF, "BIAS_MOLECULE_%-hd", nr);
	  nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE, err_rd_pds );
	  if ( strcmp( keyword, cbuff ) != 0 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_KEY, keyword );
	  (void) strlcpy( sph->bias_mol[nr], keyvalue+1, 9 );
     }
/*
 * check number of bytes read
 */
     if ( nbyte != Length_SPH )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_SIZE, "SPH size" );
     return;
}
