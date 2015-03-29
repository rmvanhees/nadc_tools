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

.IDENTifer   MERIS_RD_TIE
.AUTHOR      R.M. van Hees
.KEYWORDS    MERIS 
.LANGUAGE    ANSI C
.PURPOSE     read MERIS tie point annotation (LADS)
.INPUT/OUTPUT
  call as   nr_dsr = MERIS_RD_TIE( fd, num_dsd, dsd, &tie );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct tie_meris *tie :   tie point annotation info LADS

.RETURNS     number of data set records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0   09-Oct-2008 created by R. M. van Hees
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
#define _MERIS_COMMON
#include <nadc_meris.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_TIE( struct tie_meris *tie )
{
     register unsigned short ni;

     tie->mjd.days  = byte_swap_32( tie->mjd.days );
     tie->mjd.secnd = byte_swap_u32( tie->mjd.secnd );
     tie->mjd.musec = byte_swap_u32( tie->mjd.musec );
     for ( ni = 0; ni < MERIS_NUM_TIE_POINT; ni++ ) {
	  tie->coord[ni].lat      = byte_swap_32( tie->coord[ni].lat );
	  tie->coord[ni].lon      = byte_swap_32( tie->coord[ni].lon );
	  tie->dem_altitude[ni]   = byte_swap_32( tie->dem_altitude[ni] );
	  tie->dem_roughness[ni]  = byte_swap_u32( tie->dem_roughness[ni] );
	  tie->dem_lat_corr[ni]   = byte_swap_32( tie->dem_lat_corr[ni] );
	  tie->dem_lon_corr[ni]   = byte_swap_32( tie->dem_lon_corr[ni] );
	  tie->sun_zen_angle[ni]  = byte_swap_u32( tie->sun_zen_angle[ni] );
	  tie->sun_azi_angle[ni]  = byte_swap_32( tie->sun_azi_angle[ni] );
	  tie->view_zen_angle[ni] = byte_swap_u32( tie->view_zen_angle[ni] );
	  tie->view_azi_angle[ni] = byte_swap_32( tie->view_azi_angle[ni] );
	  tie->zonal_wind[ni]     = byte_swap_16( tie->zonal_wind[ni] );
	  tie->merid_wind[ni]     = byte_swap_16( tie->merid_wind[ni] );
	  tie->atm_press[ni]      = byte_swap_u16( tie->atm_press[ni] );
	  tie->ozone[ni]          = byte_swap_u16( tie->ozone[ni] );
	  tie->humidity[ni]       = byte_swap_u16( tie->humidity[ni] );
     }
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int MERIS_RD_TIE( FILE *fd, unsigned int num_dsd, 
			   const struct dsd_envi *dsd, 
			   struct tie_meris **tie_out )
{
     register unsigned short ni;

     char         *tie_char, *tie_pntr;
     size_t       dsr_size;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct tie_meris *tie;

     const char dsd_name[] = "Tie points ADS";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_ABSENT || IS_ERR_STAT_FATAL ) {
          NADC_ERROR( NADC_ERR_PDS_RD, dsd_name );
	  return 0;
     }
     if ( dsd[indx_dsd].num_dsr == 0 ) return 0;
     if ( ! Use_Extern_Alloc ) {
          tie_out[0] = (struct tie_meris *)
               malloc( dsd[indx_dsd].num_dsr * sizeof(struct tie_meris));
     }
     if ( (tie = tie_out[0]) == NULL ) {
          NADC_ERROR( NADC_ERR_ALLOC, "tie" );
	  return 0;
     }
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (tie_char = (char *) malloc( dsr_size )) == NULL ) {
	  NADC_ERROR( NADC_ERR_ALLOC, "tie_char" );
	  return 0;
     }
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
     do {
	  if ( fread( tie_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to Tie point structure
 */
	  (void) memcpy( &tie->mjd.days, tie_char, ENVI_INT );
	  tie_pntr = tie_char + ENVI_INT;
	  (void) memcpy( &tie->mjd.secnd, tie_pntr, ENVI_UINT );
	  tie_pntr += ENVI_UINT;
	  (void) memcpy( &tie->mjd.musec, tie_pntr, ENVI_UINT );
	  tie_pntr += ENVI_UINT;
	  (void) memcpy( &tie->flag_mds, tie_pntr, ENVI_UCHAR );
	  tie_pntr += ENVI_UCHAR;
	  for ( ni = 0; ni < MERIS_NUM_TIE_POINT; ni++ ) {
	       (void) memcpy( &tie->coord[ni].lat, tie_pntr, ENVI_INT );
	       tie_pntr += ENVI_INT;
	  }
	  for ( ni = 0; ni < MERIS_NUM_TIE_POINT; ni++ ) {
	       (void) memcpy( &tie->coord[ni].lon, tie_pntr, ENVI_INT );
	       tie_pntr += ENVI_INT;
	  }
	  (void) memcpy( &tie->dem_altitude, tie_pntr, 
			 MERIS_NUM_TIE_POINT * ENVI_INT );
	  tie_pntr += MERIS_NUM_TIE_POINT * ENVI_INT;
	  (void) memcpy( &tie->dem_roughness, tie_pntr, 
			 MERIS_NUM_TIE_POINT * ENVI_INT );
	  tie_pntr += MERIS_NUM_TIE_POINT * ENVI_INT;
	  (void) memcpy( &tie->dem_lat_corr, tie_pntr, 
			 MERIS_NUM_TIE_POINT * ENVI_INT );
	  tie_pntr += MERIS_NUM_TIE_POINT * ENVI_INT;
	  (void) memcpy( &tie->dem_lon_corr, tie_pntr, 
			 MERIS_NUM_TIE_POINT * ENVI_INT );
	  tie_pntr += MERIS_NUM_TIE_POINT * ENVI_INT;
	  (void) memcpy( &tie->sun_zen_angle, tie_pntr, 
			 MERIS_NUM_TIE_POINT * ENVI_UINT );
	  tie_pntr += MERIS_NUM_TIE_POINT * ENVI_UINT;
	  (void) memcpy( &tie->sun_azi_angle, tie_pntr, 
			 MERIS_NUM_TIE_POINT * ENVI_INT );
	  tie_pntr += MERIS_NUM_TIE_POINT * ENVI_INT;
	  (void) memcpy( &tie->view_zen_angle, tie_pntr, 
			 MERIS_NUM_TIE_POINT * ENVI_UINT );
	  tie_pntr += MERIS_NUM_TIE_POINT * ENVI_UINT;
	  (void) memcpy( &tie->view_azi_angle, tie_pntr, 
			 MERIS_NUM_TIE_POINT * ENVI_INT );
	  tie_pntr += MERIS_NUM_TIE_POINT * ENVI_INT;
	  (void) memcpy( &tie->zonal_wind, tie_pntr, 
			 MERIS_NUM_TIE_POINT * ENVI_SHORT );
	  tie_pntr += MERIS_NUM_TIE_POINT * ENVI_SHORT;
	  (void) memcpy( &tie->merid_wind, tie_pntr, 
			 MERIS_NUM_TIE_POINT * ENVI_SHORT );
	  tie_pntr += MERIS_NUM_TIE_POINT * ENVI_SHORT;
	  (void) memcpy( &tie->atm_press, tie_pntr, 
			 MERIS_NUM_TIE_POINT * ENVI_USHRT );
	  tie_pntr += MERIS_NUM_TIE_POINT * ENVI_USHRT;
	  (void) memcpy( &tie->ozone, tie_pntr,
			 MERIS_NUM_TIE_POINT * ENVI_USHRT );
	  tie_pntr += MERIS_NUM_TIE_POINT * ENVI_USHRT;
	  (void) memcpy( &tie->humidity, tie_pntr, 
			 MERIS_NUM_TIE_POINT * ENVI_USHRT );
	  tie_pntr += MERIS_NUM_TIE_POINT * ENVI_USHRT;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(tie_pntr - tie_char) != dsr_size ) {
	       (void) fprintf( stderr, "%d %zd\n", 
			       (int) (tie_pntr - tie_char), 
			       dsr_size );
	       NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, dsd_name );
	  }
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_TIE( tie );
#endif
	  tie++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * deallocate memory
 */
 done:
     tie_pntr = NULL;
     free( tie_char );
/*
 * set return values
 */
     return nr_dsr;
}
