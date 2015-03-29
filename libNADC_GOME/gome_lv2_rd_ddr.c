/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 1999 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GOME_LV2_RD_DDR
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 2 data
.LANGUAGE    ANSI C
.COMMENTS    contains GOME_LV2_RD_DDR
.ENVIRONment None
.VERSION      3.0   17-Jul-2008	added read routines for file format v2.0,RvH
              2.0   11-Nov-2001	moved to the new Error handling routines, RvH
              1.0   18-Mar-1999	Created by R. M. van Hees 
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
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_IRR1( const struct sph2_gome *sph, struct irr1_gome *irr )
{
     register int ni, nm, nw;

     irr->indx_vcd = byte_swap_16( irr->indx_vcd );
     irr->indx_doas = byte_swap_16( irr->indx_doas );
     irr->indx_amf = byte_swap_16( irr->indx_amf );
     irr->indx_icfa = byte_swap_16( irr->indx_icfa );
     irr->indx_stats = byte_swap_16( irr->indx_stats );
     IEEE_Swap__FLT( &irr->cloud_frac );
     IEEE_Swap__FLT( &irr->cloud_pres );
     IEEE_Swap__FLT( &irr->err_cloud_frac );
     IEEE_Swap__FLT( &irr->err_cloud_pres );
     IEEE_Swap__FLT( &irr->surface_pres );
     IEEE_Swap__FLT( &irr->cca_cloud_frac );
     IEEE_Swap__FLT( &irr->pixel_gradient );
     for ( ni = 0; ni < 3; ni++ ) {
	  IEEE_Swap__FLT( &irr->pmd_avg[ni] );
	  IEEE_Swap__FLT( &irr->pmd_sdev[ni] );
     }
     for ( ni = 0; ni < 16; ni++ ) {
	  IEEE_Swap__FLT( &irr->pixel_color[ni] );
     }
     for ( nw = 0; nw < sph->nwin; nw++ ) {
	  IEEE_Swap__FLT( &irr->rms_doas[nw] );
	  IEEE_Swap__FLT( &irr->chi_doas[nw] );
	  IEEE_Swap__FLT( &irr->fit_doas[nw] );
	  IEEE_Swap__FLT( &irr->iter_doas[nw] );
	  IEEE_Swap__FLT( &irr->intensity_ground[nw] );
	  IEEE_Swap__FLT( &irr->intensity_cloud[nw] );
	  IEEE_Swap__FLT( &irr->intensity_measured[nw] );
     }
     for ( nm = 0; nm < sph->nmol; nm++ ) {
	  IEEE_Swap__FLT( &irr->total_vcd[nm] );
	  IEEE_Swap__FLT( &irr->error_vcd[nm] );
	  IEEE_Swap__FLT( &irr->slant_doas[nm] );
	  IEEE_Swap__FLT( &irr->error_doas[nm] );
	  IEEE_Swap__FLT( &irr->ground_amf[nm] );
	  IEEE_Swap__FLT( &irr->cloud_amf[nm] );
     }
}

static
void Sun2Intel_IRR2( const struct sph2_gome *sph, struct irr2_gome *irr )
{
     register int nm, nw;

     irr->indx_vcd = byte_swap_16( irr->indx_vcd );
     irr->indx_doas = byte_swap_16( irr->indx_doas );
     irr->indx_amf = byte_swap_16( irr->indx_amf );
     IEEE_Swap__FLT( &irr->ozone_temperature );
     IEEE_Swap__FLT( &irr->ozone_ring_corr );
     IEEE_Swap__FLT( &irr->ghost_column );
     IEEE_Swap__FLT( &irr->cld_frac );
     IEEE_Swap__FLT( &irr->error_cld_frac );
     IEEE_Swap__FLT( &irr->cld_height );
     IEEE_Swap__FLT( &irr->error_cld_height );
     IEEE_Swap__FLT( &irr->cld_press );
     IEEE_Swap__FLT( &irr->error_cld_press );
     IEEE_Swap__FLT( &irr->cld_albedo );
     IEEE_Swap__FLT( &irr->error_cld_albedo );
     IEEE_Swap__FLT( &irr->surface_height );
     IEEE_Swap__FLT( &irr->surface_press );
     IEEE_Swap__FLT( &irr->surface_albedo );
     for ( nw = 0; nw < sph->nwin; nw++ ) {
	  IEEE_Swap__FLT( &irr->rms_doas[nw] );
	  IEEE_Swap__FLT( &irr->chi_doas[nw] );
	  IEEE_Swap__FLT( &irr->fit_doas[nw] );
	  IEEE_Swap__FLT( &irr->iter_doas[nw] );
     }
     for ( nm = 0; nm < sph->nmol; nm++ ) {
	  IEEE_Swap__FLT( &irr->total_vcd[nm] );
	  IEEE_Swap__FLT( &irr->error_vcd[nm] );
	  IEEE_Swap__FLT( &irr->slant_doas[nm] );
	  IEEE_Swap__FLT( &irr->error_doas[nm] );
	  IEEE_Swap__FLT( &irr->ground_amf[nm] );
	  IEEE_Swap__FLT( &irr->error_ground_amf[nm] );
	  IEEE_Swap__FLT( &irr->cloud_amf[nm] );
	  IEEE_Swap__FLT( &irr->error_cloud_amf[nm] );
     }
}

static
void Sun2Intel_DDR( struct ddr_gome *ddr )
{
     register int ni;

     ddr->glr.pixel_nr = byte_swap_32( ddr->glr.pixel_nr );
     ddr->glr.subsetcounter = byte_swap_32( ddr->glr.subsetcounter );
     ddr->glr.utc_date = byte_swap_u32( ddr->glr.utc_date );
     ddr->glr.utc_time = byte_swap_u32( ddr->glr.utc_time );
     IEEE_Swap__FLT( &ddr->glr.sat_geo_height );
     IEEE_Swap__FLT( &ddr->glr.earth_radius );
     IEEE_Swap__FLT( &ddr->ozone );
     IEEE_Swap__FLT( &ddr->error );
     for ( ni = 0; ni < 3; ni++ ) {
	  IEEE_Swap__FLT( &ddr->glr.sat_zenith[ni] );
	  IEEE_Swap__FLT( &ddr->glr.sat_sight[ni] );
	  IEEE_Swap__FLT( &ddr->glr.sat_azim[ni] );
	  IEEE_Swap__FLT( &ddr->glr.toa_zenith[ni] );
	  IEEE_Swap__FLT( &ddr->glr.toa_sight[ni] );
	  IEEE_Swap__FLT( &ddr->glr.toa_azim[ni] );
     }
     for ( ni = 0; ni < 5; ni++ ) {
	  IEEE_Swap__FLT( &ddr->glr.lat[ni] );
	  IEEE_Swap__FLT( &ddr->glr.lon[ni] );
     }
}
#endif

static
char *GOME_LV2_RD_IRR1( const struct sph2_gome *sph,
			char *ddr_char, struct irr1_gome *irr )
{
     register int nw;

     char   *ddr_pntr = ddr_char;
     size_t nr_byte;
     
     nr_byte = sph->nmol * GOME_FLOAT;
     (void) memcpy( irr->total_vcd, ddr_pntr, nr_byte );
     ddr_pntr += nr_byte;
     (void) memcpy( irr->error_vcd, ddr_pntr, nr_byte );
     ddr_pntr += nr_byte;
     (void) memcpy( &irr->indx_vcd, ddr_pntr, GOME_SHORT );
     ddr_pntr += GOME_SHORT;
     (void) memcpy( irr->slant_doas, ddr_pntr, nr_byte );
     ddr_pntr += nr_byte;
     (void) memcpy( irr->error_doas, ddr_pntr, nr_byte );
     ddr_pntr += nr_byte;

     for ( nw = 0; nw < sph->nwin; nw++ ) {
	  (void) memcpy( &irr->rms_doas[nw], ddr_pntr, GOME_FLOAT );
	  ddr_pntr += GOME_FLOAT;
	  (void) memcpy( &irr->chi_doas[nw], ddr_pntr, GOME_FLOAT );
	  ddr_pntr += GOME_FLOAT;
	  (void) memcpy( &irr->fit_doas[nw], ddr_pntr, GOME_FLOAT );
	  ddr_pntr += GOME_FLOAT;
	  (void) memcpy( &irr->iter_doas[nw], ddr_pntr, GOME_FLOAT );
	  ddr_pntr += GOME_FLOAT;
     }
     (void) memcpy( &irr->indx_doas, ddr_pntr, GOME_SHORT );
     ddr_pntr += GOME_SHORT;

     nr_byte = sph->nmol * GOME_FLOAT;
     (void) memcpy( irr->ground_amf, ddr_char, nr_byte );
     ddr_pntr += nr_byte;
     (void) memcpy( irr->cloud_amf, ddr_pntr, nr_byte );
     ddr_pntr += nr_byte;

     nr_byte = sph->nwin * GOME_FLOAT;
     (void) memcpy( irr->intensity_ground, ddr_pntr, nr_byte );
     ddr_pntr += nr_byte;
     (void) memcpy( irr->intensity_cloud, ddr_pntr, nr_byte );
     ddr_pntr += nr_byte;
     (void) memcpy( irr->intensity_measured, ddr_pntr, nr_byte );
     ddr_pntr += nr_byte;

     (void) memcpy( &irr->indx_amf, ddr_pntr, GOME_SHORT );
     ddr_pntr += GOME_SHORT;
     (void) memcpy( &irr->cloud_frac, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->cloud_pres, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->err_cloud_frac, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->err_cloud_pres, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->surface_pres, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->cca_cloud_frac, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->cca_subpixel, ddr_pntr, 16 );
     ddr_pntr += 16;
     (void) memcpy( &irr->indx_icfa, ddr_pntr, GOME_SHORT );
     ddr_pntr += GOME_SHORT;
     (void) memcpy( irr->pmd_avg, ddr_pntr, 3 * GOME_FLOAT );
     ddr_pntr += 3 * GOME_FLOAT;
     (void) memcpy( irr->pmd_sdev, ddr_pntr, 3 * GOME_FLOAT );
     ddr_pntr += 3 * GOME_FLOAT;
     (void) memcpy( irr->pixel_color, ddr_pntr, 16 * GOME_FLOAT );
     ddr_pntr += 16 * GOME_FLOAT;
     (void) memcpy( &irr->pixel_gradient, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->indx_stats, ddr_pntr, GOME_SHORT );
     return (ddr_pntr + GOME_SHORT);
}

static
char *GOME_LV2_RD_IRR2( const struct sph2_gome *sph,
			char *ddr_char, struct irr2_gome *irr )
{
     register int nw;

     char   *ddr_pntr = ddr_char;
     size_t nr_byte;
     
     nr_byte = sph->nmol * GOME_FLOAT;
     (void) memcpy( irr->total_vcd, ddr_pntr, nr_byte );
     ddr_pntr += nr_byte;
     (void) memcpy( irr->error_vcd, ddr_pntr, nr_byte );
     ddr_pntr += nr_byte;
     (void) memcpy( &irr->indx_vcd, ddr_pntr, GOME_SHORT );
     ddr_pntr += GOME_SHORT;

     (void) memcpy( irr->slant_doas, ddr_pntr, nr_byte );
     ddr_pntr += nr_byte;
     (void) memcpy( irr->error_doas, ddr_pntr, nr_byte );
     ddr_pntr += nr_byte;
     for ( nw = 0; nw < sph->nwin; nw++ ) {
	  (void) memcpy( &irr->rms_doas[nw], ddr_pntr, GOME_FLOAT );
	  ddr_pntr += GOME_FLOAT;
	  (void) memcpy( &irr->chi_doas[nw], ddr_pntr, GOME_FLOAT );
	  ddr_pntr += GOME_FLOAT;
	  (void) memcpy( &irr->fit_doas[nw], ddr_pntr, GOME_FLOAT );
	  ddr_pntr += GOME_FLOAT;
	  (void) memcpy( &irr->iter_doas[nw], ddr_pntr, GOME_FLOAT );
	  ddr_pntr += GOME_FLOAT;
     }
     (void) memcpy( &irr->ozone_temperature, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->ozone_ring_corr, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->indx_doas, ddr_pntr, GOME_SHORT );
     ddr_pntr += GOME_SHORT;

     nr_byte = sph->nmol * GOME_FLOAT;
     (void) memcpy( irr->ground_amf, ddr_char, nr_byte );
     ddr_pntr += nr_byte;
     (void) memcpy( irr->error_ground_amf, ddr_char, nr_byte );
     ddr_pntr += nr_byte;
     (void) memcpy( irr->cloud_amf, ddr_pntr, nr_byte );
     ddr_pntr += nr_byte;
     (void) memcpy( irr->error_cloud_amf, ddr_pntr, nr_byte );
     ddr_pntr += nr_byte;
     (void) memcpy( &irr->indx_amf, ddr_pntr, GOME_SHORT );
     ddr_pntr += GOME_SHORT;

     (void) memcpy( &irr->ghost_column, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;

     (void) memcpy( &irr->cld_frac, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->error_cld_frac, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;

     (void) memcpy( &irr->cld_height, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->error_cld_height, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->cld_press, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->error_cld_press, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->cld_albedo, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->error_cld_albedo, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;

     (void) memcpy( &irr->surface_height, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->surface_press, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;
     (void) memcpy( &irr->surface_albedo, ddr_pntr, GOME_FLOAT );
     ddr_pntr += GOME_FLOAT;

     return (ddr_pntr + (80 + 12 * sph->nwin - 8 * sph->nmol));
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   GOME_LV2_RD_DDR
.PURPOSE     Read a Spectral Band Record
.INPUT/OUTPUT
  call as   nr_ddr = GOME_LV2_RD_DDR( infl, fsr, sph, &ddr );
     input:  
            int    fid             : (open) file descriptor
	    struct fsr2_gome *fsr  : structure for the FSR record
	    struct sph2_gome *sph  : Specific Product Header
    output:  
            struct ddr_gome **ddr  : structure for a DOAS data record

.RETURNS     number of DDR record read
.COMMENTS    none
-------------------------*/
short GOME_LV2_RD_DDR( FILE *infl, 
		       const struct fsr2_gome *fsr,
		       const struct sph2_gome *sph,
		       struct ddr_gome **ddr_out )
{
     register int nc;

     char   *ddr_pntr, *ddr_char = NULL;
     short  nr_ddr = 0;                    /* initialize return value */
     struct ddr_gome *ddr;

     const long byte_offs = LVL1_PIR_LENGTH + LVL2_FSR_LENGTH + fsr->sz_sph;
/*
 * allocate memory for the DDR records
 */
     ddr = ddr_out[0] = (struct ddr_gome *)
	  malloc( fsr->nr_ddr * sizeof( struct ddr_gome ) );
     if ( ddr == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "ddr" );
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (ddr_char = (char *) malloc( (size_t) fsr->sz_ddr )) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "ddr_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( infl, byte_offs, SEEK_SET );
/*
 * read data buffer to DDR structure
 */
     do { 
	  ddr_pntr = ddr_char;
	  if ( fread( ddr_char, (size_t) fsr->sz_ddr, 1, infl ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
	  (void) memcpy( &ddr->glr.pixel_nr, ddr_pntr, GOME_INT );
	  ddr_pntr += GOME_INT;
	  (void) memcpy( &ddr->glr.subsetcounter, ddr_pntr, GOME_INT );
	  ddr_pntr += GOME_INT;
	  (void) memcpy( &ddr->glr.utc_date, ddr_pntr, GOME_UINT );
	  ddr_pntr += GOME_UINT;
	  (void) memcpy( &ddr->glr.utc_time, ddr_pntr, GOME_UINT );
	  ddr_pntr += GOME_UINT;
	  (void) memcpy( ddr->glr.sat_zenith, ddr_pntr, 3 * GOME_FLOAT );
	  ddr_pntr += 3 * GOME_FLOAT;
	  (void) memcpy( ddr->glr.sat_sight, ddr_pntr, 3 * GOME_FLOAT );
	  ddr_pntr += 3 * GOME_FLOAT;
	  (void) memcpy( ddr->glr.sat_azim, ddr_pntr, 3 * GOME_FLOAT );
	  ddr_pntr += 3 * GOME_FLOAT;
	  (void) memcpy( ddr->glr.toa_zenith, ddr_pntr, 3 * GOME_FLOAT );
	  ddr_pntr += 3 * GOME_FLOAT;
	  (void) memcpy( ddr->glr.toa_sight, ddr_pntr, 3 * GOME_FLOAT );
	  ddr_pntr += 3 * GOME_FLOAT;
	  (void) memcpy( ddr->glr.toa_azim, ddr_pntr, 3 * GOME_FLOAT );
	  ddr_pntr += 3 * GOME_FLOAT;
	  (void) memcpy( &ddr->glr.sat_geo_height, ddr_pntr, GOME_FLOAT );
	  ddr_pntr += GOME_FLOAT;
	  (void) memcpy( &ddr->glr.earth_radius, ddr_pntr, GOME_FLOAT );
	  ddr_pntr += GOME_FLOAT;
	  nc = 0;
	  do {
	       (void) memcpy( &ddr->glr.lat[nc], ddr_pntr, GOME_FLOAT );
	       ddr_pntr += GOME_FLOAT;
	       (void) memcpy( &ddr->glr.lon[nc], ddr_pntr, GOME_FLOAT );
	       ddr_pntr += GOME_FLOAT;
	  } while ( ++nc < NUM_COORDS );
	  (void) memcpy( &ddr->ozone, ddr_pntr, GOME_FLOAT );
	  ddr_pntr += GOME_FLOAT;
	  (void) memcpy( &ddr->error, ddr_pntr, GOME_FLOAT );
	  ddr_pntr += GOME_FLOAT;
/*
 * read Intermediate Results Record
 */
	  if ( strncmp( sph->format_version, "02.", 3 ) != 0 ) {
	       ddr->irr1 = (struct irr1_gome *) 
		    malloc( sizeof(struct irr1_gome) );
	       ddr->irr2 = NULL;
	       ddr_pntr = GOME_LV2_RD_IRR1( sph, ddr_pntr, ddr->irr1 );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       Sun2Intel_IRR1( sph, ddr->irr1 );
#endif
	  } else {
	       ddr->irr1 = NULL;
	       ddr->irr2 = (struct irr2_gome *) 
		    malloc( sizeof(struct irr2_gome) );
	       ddr_pntr = GOME_LV2_RD_IRR2( sph, ddr_pntr, ddr->irr2 );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       Sun2Intel_IRR2( sph, ddr->irr2 );
#endif
	  }
/*
 * check if we read the whole DSR
 */
	  if ( (int) (ddr_pntr - ddr_char) != fsr->sz_ddr ) {
	       char msg[64];
	       (void) snprintf( msg, 64, "DDR size (%-d != %-d)",
				(int) (ddr_pntr - ddr_char), fsr->sz_ddr );
	       NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, msg );
	  }

#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_DDR( ddr );
#endif
	  ddr++;
     } while ( ++nr_ddr < fsr->nr_ddr );
 done:
     if ( ddr_char != NULL ) free( ddr_char );
/*
 * set return value
 */
     return nr_ddr;
}
