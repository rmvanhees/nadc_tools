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

.IDENTifer   MERIS_RR2_RD_SFGI
.AUTHOR      R.M. van Hees
.KEYWORDS    MERIS level 2 product
.LANGUAGE    ANSI C
.PURPOSE     read scaling factors and general info GADS
.INPUT/OUTPUT
  call as   nr_dsr = MERIS_RR2_RD_SFGI( fd, num_dsd, dsd, &sfgi );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct sfgi_meris *sfgi :  scaling factors and general info GADS

.RETURNS     number of data set records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0   22-Sep-2008 created by R. M. van Hees
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
void Sun2Intel_SFGI( struct sfgi_meris *sfgi )
{
     register unsigned short ni;

     IEEE_Swap__FLT( &sfgi->sf_altitude );
     IEEE_Swap__FLT( &sfgi->sf_roughness );
     IEEE_Swap__FLT( &sfgi->sf_zonal_wind );
     IEEE_Swap__FLT( &sfgi->sf_merid_wind );
     IEEE_Swap__FLT( &sfgi->sf_atm_press );
     IEEE_Swap__FLT( &sfgi->sf_ozone );
     IEEE_Swap__FLT( &sfgi->sf_humidity );
     for ( ni = 0; ni < 13; ni++ )
	  IEEE_Swap__FLT( &sfgi->sf_reflectance[ni] );
     IEEE_Swap__FLT( &sfgi->sf_pigment_index );
     IEEE_Swap__FLT( &sfgi->sf_yellow_sub );
     IEEE_Swap__FLT( &sfgi->sf_sediment );
     IEEE_Swap__FLT( &sfgi->sf_aer_opt_thick );
     IEEE_Swap__FLT( &sfgi->sf_cld_opt_thick );
     IEEE_Swap__FLT( &sfgi->sf_surf_press );
     IEEE_Swap__FLT( &sfgi->sf_water_vapour );
     IEEE_Swap__FLT( &sfgi->sf_photo_rad );
     IEEE_Swap__FLT( &sfgi->sf_toa_veg_index );
     IEEE_Swap__FLT( &sfgi->sf_boa_veg_index );
     IEEE_Swap__FLT( &sfgi->sf_cld_albedo );
     IEEE_Swap__FLT( &sfgi->sf_cld_press );
     for ( ni = 0; ni < 13; ni++ )
	  IEEE_Swap__FLT( &sfgi->off_reflectance[ni] );
     IEEE_Swap__FLT( &sfgi->off_pigment_index );
     IEEE_Swap__FLT( &sfgi->off_yellow_sub );
     IEEE_Swap__FLT( &sfgi->off_sediment );
     IEEE_Swap__FLT( &sfgi->off_aer_opt_thick );
     IEEE_Swap__FLT( &sfgi->off_cld_opt_thick );
     IEEE_Swap__FLT( &sfgi->off_surf_press );
     IEEE_Swap__FLT( &sfgi->off_water_vapour );
     IEEE_Swap__FLT( &sfgi->off_photo_rad );
     IEEE_Swap__FLT( &sfgi->off_toa_veg_index );
     IEEE_Swap__FLT( &sfgi->off_boa_veg_index );
     IEEE_Swap__FLT( &sfgi->off_cld_albedo );
     IEEE_Swap__FLT( &sfgi->off_cld_press );
     sfgi->sampling_rate = byte_swap_32( sfgi->sampling_rate );
     for ( ni = 0; ni < 15; ni++ )
	  IEEE_Swap__FLT( &sfgi->sun_flux[ni] );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int MERIS_RR2_RD_SFGI( FILE *fd, unsigned int num_dsd, 
				const struct dsd_envi *dsd, 
				struct sfgi_meris *sfgi )
{
     char         *sfgi_char, *sfgi_pntr;
     size_t       dsr_size;
     unsigned int indx_dsd;

     const char dsd_name[] = "Scaling Factor GADS";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_ABSENT || IS_ERR_STAT_FATAL ) {
          NADC_ERROR( NADC_ERR_PDS_RD, dsd_name );
	  return 0;
     }
     if ( dsd[indx_dsd].num_dsr == 0 ) return 0;
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (sfgi_char = (char *) malloc( dsr_size )) == NULL ) {
	  NADC_ERROR( NADC_ERR_ALLOC, "sfgi_char" );
	  return 0;
     }
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
     if ( fread( sfgi_char, dsr_size, 1, fd ) != 1 ) {
	  free( sfgi_char );
	  NADC_ERROR( NADC_ERR_PDS_RD, "" );
	  return 0;
     }
/*
 * read data buffer to SFGI structure
 */
     (void) memcpy( &sfgi->sf_altitude, sfgi_char, ENVI_FLOAT );
     sfgi_pntr = sfgi_char + ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_roughness, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_zonal_wind, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_merid_wind, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_atm_press, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_ozone, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_humidity, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_reflectance, sfgi_pntr, 13 * ENVI_FLOAT );
     sfgi_pntr += 13 * ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_pigment_index, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_yellow_sub, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_sediment, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_aer_epsilon, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_aer_opt_thick, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_cld_opt_thick, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_surf_press, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_water_vapour, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_photo_rad, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_toa_veg_index, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_boa_veg_index, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_cld_albedo, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_cld_press, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->off_reflectance, sfgi_pntr, 13 * ENVI_FLOAT );
     sfgi_pntr += 13 * ENVI_FLOAT;
     (void) memcpy( &sfgi->off_pigment_index, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->off_yellow_sub, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->off_sediment, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->off_aer_epsilon, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->off_aer_opt_thick, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->off_cld_opt_thick, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->off_surf_press, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->off_water_vapour, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->off_photo_rad, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->off_toa_veg_index, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->off_boa_veg_index, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->off_cld_albedo, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->off_cld_press, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->gains, sfgi_pntr, 5 * 16 * ENVI_UCHAR );
     sfgi_pntr += 5 * 16 * ENVI_UCHAR;
     (void) memcpy( &sfgi->sampling_rate, sfgi_pntr, ENVI_UINT );
     sfgi_pntr += ENVI_UINT;
     (void) memcpy( &sfgi->sun_flux, sfgi_pntr, 15 * ENVI_FLOAT );
     sfgi_pntr += 15 * ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_nir_refl, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->off_nir_refl, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->sf_red_refl, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     (void) memcpy( &sfgi->off_red_refl, sfgi_pntr, ENVI_FLOAT );
     sfgi_pntr += ENVI_FLOAT;
     sfgi_pntr += 44 * ENVI_UCHAR;  /* spare */
/*
 * check if we read the whole DSR
 */
     if ( (size_t)(sfgi_pntr - sfgi_char) != dsr_size ) {
	  (void) fprintf( stderr, "%d %zd\n", (int) (sfgi_pntr - sfgi_char), 
			  dsr_size );
	  free( sfgi_char );
	  NADC_ERROR( NADC_ERR_PDS_SIZE, dsd_name );
	  return 0;
     }
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_SFGI( sfgi );
#endif
/*
 * deallocate memory
 */
     sfgi_pntr = NULL;
     free( sfgi_char );
/*
 * set return values
 */
     return 1;
}
