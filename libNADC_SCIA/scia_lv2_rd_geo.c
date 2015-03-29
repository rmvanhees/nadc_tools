/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV2_RD_GEO
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 data
.LANGUAGE    ANSI C
.PURPOSE     read Geolocation Data Sets
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV2_RD_GEO( fd, num_dsd, dsd, &geo );

     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct geo_scia *geo  :  Geolocation Data sets

.RETURNS     number of data set records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      2.1   21-Jan-2002	use of global Use_Extern_Alloc, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.1   09-Oct-2001	changed input/output, RvH 
              1.0   13-Sep-2001 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_GEO( struct geo_scia *geo )
{
     register unsigned short ni;

     geo->mjd.days = byte_swap_32( geo->mjd.days );
     geo->mjd.secnd = byte_swap_u32( geo->mjd.secnd );
     geo->mjd.musec = byte_swap_u32( geo->mjd.musec );
     geo->intg_time = byte_swap_u16( geo->intg_time );
     IEEE_Swap__FLT( &geo->sat_h );
     IEEE_Swap__FLT( &geo->earth_rad );
     for ( ni = 0; ni < 3; ni++ ) {
	  IEEE_Swap__FLT( &geo->sun_zen_ang[ni] );
	  IEEE_Swap__FLT( &geo->los_zen_ang[ni] );
	  IEEE_Swap__FLT( &geo->rel_azi_ang[ni] );
     }
     for ( ni = 0; ni < 4; ni++ ) {
	  geo->corner[ni].lon = byte_swap_32( geo->corner[ni].lon );
	  geo->corner[ni].lat = byte_swap_32( geo->corner[ni].lat );
     }
     geo->center.lon = byte_swap_32( geo->center.lon );
     geo->center.lat = byte_swap_32( geo->center.lat );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned 
int SCIA_LV2_RD_GEO( FILE *fd, unsigned int num_dsd,
		     const struct dsd_envi *dsd, 
		     struct geo_scia **geo_out )
{
     char         *geo_pntr, *geo_char = NULL;
     size_t       dsr_size;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct geo_scia *geo;

     const char dsd_name[] = "GEOLOCATION";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, dsd_name );
     if ( ! Use_Extern_Alloc ) {
	  geo_out[0] = (struct geo_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct geo_scia));
     }
     if ( (geo = geo_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geo" );
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (geo_char = (char *) malloc( dsr_size )) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "geo_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( geo_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to GEO structure
 */
	  (void) memcpy( &geo->mjd.days, geo_char, ENVI_INT );
	  geo_pntr = geo_char + ENVI_INT;
	  (void) memcpy( &geo->mjd.secnd, geo_pntr, ENVI_UINT );
	  geo_pntr += ENVI_UINT;
	  (void) memcpy( &geo->mjd.musec, geo_pntr, ENVI_UINT );
	  geo_pntr += ENVI_UINT;
	  (void) memcpy( &geo->flag_mds, geo_pntr, ENVI_UCHAR );
	  geo_pntr += ENVI_UCHAR;

	  (void) memcpy( &geo->intg_time, geo_pntr, ENVI_USHRT );
	  geo_pntr += ENVI_USHRT;
	  (void) memcpy( geo->sun_zen_ang, geo_pntr, 3 * ENVI_FLOAT );
	  geo_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( geo->los_zen_ang, geo_pntr, 3 * ENVI_FLOAT );
	  geo_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( geo->rel_azi_ang, geo_pntr, 3 * ENVI_FLOAT );
	  geo_pntr += 3 * ENVI_FLOAT;
	  (void) memcpy( &geo->sat_h, geo_pntr, ENVI_FLOAT );
	  geo_pntr += ENVI_FLOAT;
	  (void) memcpy( &geo->earth_rad, geo_pntr, ENVI_FLOAT );
	  geo_pntr += ENVI_FLOAT;
	  (void) memcpy( geo->corner, geo_pntr, 
			   4 * sizeof(struct coord_envi) );
	  geo_pntr += 4 * sizeof(struct coord_envi);
	  (void) memcpy( &geo->center, geo_pntr, 
			   sizeof(struct coord_envi) );
	  geo_pntr += sizeof(struct coord_envi);
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(geo_pntr - geo_char) != dsr_size )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, dsd_name );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_GEO( geo );
#endif
	  geo++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
     geo_pntr = NULL;
/*
 * set return values
 */
 done:
     if ( geo_char != NULL ) free( geo_char );

     return nr_dsr;
}

