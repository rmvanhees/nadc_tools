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

.IDENTifer   GOME_LV1_RD_PCD
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1 data
.LANGUAGE    ANSI C
.PURPOSE     Read a Pixel Specific Calibration record
.INPUT/OUTPUT
  call as   nr_pcd = GOME_LV1_RD_PCD( infl, fsr, sph, &pcd );

     input:  
            FILE   *infl           : (open) file descriptor 
	    struct fsr1_gome *fsr  : structure for the FSR record
	    struct sph1_gome *sph  : structure for the SPH record
    output:  
            struct pcd_gome **pcd  : structure for the PCD record

.RETURNS     number of PCD records found (modifies error status)
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   08-Jun-2009 update to product version 2, RvH
              2.2   06-Mar-2003 use global "Use_Extern_Alloc", RvH
              2.1   20-Nov-2001	check number of records to be written, RvH
              2.0   11-Nov-2001	moved to the new Error handling routines, RvH 
              1.4   19-Jul-2001	pass structures using pointers, RvH 
              1.3   17-Feb-1999 conform to ANSI C3.159-1989, RvH
              1.2   20-Aug-1999 read prism-temp from IHR, RvH
              1.1   08-Apr-1999 read Instrument Header Record, RvH
              1.0   05-Feb-1999 created by R. M. van Hees
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
#include <math.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

#ifndef M_LN2
#define M_LN2               0.69314718055994530942  /* log_e 2 */
#endif
#define AVERAGE_MODE_MASK   (0x0040)

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_PCD( struct pcd_gome *pcd )
{
     register int ni;

     pcd->glr.utc_date = byte_swap_u32( pcd->glr.utc_date );
     pcd->glr.utc_time = byte_swap_u32( pcd->glr.utc_time );
     ni = 0;
     do {
	  IEEE_Swap__FLT( &pcd->glr.sun_zen_sat_north[ni] );
	  IEEE_Swap__FLT( &pcd->glr.sun_azim_sat_north[ni] );
	  IEEE_Swap__FLT( &pcd->glr.los_zen_sat_north[ni] );
	  IEEE_Swap__FLT( &pcd->glr.los_azim_sat_north[ni] );
	  IEEE_Swap__FLT( &pcd->glr.sun_zen_sat_ers[ni] );
	  IEEE_Swap__FLT( &pcd->glr.sun_azim_sat_ers[ni] );
	  IEEE_Swap__FLT( &pcd->glr.los_zen_sat_ers[ni] );
	  IEEE_Swap__FLT( &pcd->glr.los_azim_sat_ers[ni] );
	  IEEE_Swap__FLT( &pcd->glr.sun_zen_surf_north[ni] );
	  IEEE_Swap__FLT( &pcd->glr.sun_azim_surf_north[ni] );
	  IEEE_Swap__FLT( &pcd->glr.los_zen_surf_north[ni] );
	  IEEE_Swap__FLT( &pcd->glr.los_azim_surf_north[ni] );
     } while ( ++ni < 3 );
     IEEE_Swap__FLT( &pcd->glr.sat_geo_height );
     IEEE_Swap__FLT( &pcd->glr.earth_radius );
     ni = 0;
     do {
	  IEEE_Swap__FLT( &pcd->glr.lon[ni] );
	  IEEE_Swap__FLT( &pcd->glr.lat[ni] );
     } while ( ++ni < NUM_COORDS );
     pcd->cld.mode = byte_swap_16( pcd->cld.mode );
     pcd->cld.type = byte_swap_16( pcd->cld.type );
     IEEE_Swap__FLT( &pcd->cld.surfaceHeight );
     IEEE_Swap__FLT( &pcd->cld.fraction );
     IEEE_Swap__FLT( &pcd->cld.fractionError );
     IEEE_Swap__FLT( &pcd->cld.albedo );
     IEEE_Swap__FLT( &pcd->cld.albedoError );
     IEEE_Swap__FLT( &pcd->cld.height );
     IEEE_Swap__FLT( &pcd->cld.heightError );
     IEEE_Swap__FLT( &pcd->cld.thickness );
     IEEE_Swap__FLT( &pcd->cld.thicknessError );
     IEEE_Swap__FLT( &pcd->cld.topPress );
     IEEE_Swap__FLT( &pcd->cld.topPressError );
     IEEE_Swap__FLT( &pcd->dark_current );
     IEEE_Swap__FLT( &pcd->noise_factor );
     pcd->indx_spec = byte_swap_16( pcd->indx_spec );
     pcd->indx_leak = byte_swap_16( pcd->indx_leak );
     ni = 0;
     do {
	  IEEE_Swap__FLT( &pcd->polar.wv[ni] );
	  IEEE_Swap__FLT( &pcd->polar.coeff[ni] );
	  IEEE_Swap__FLT( &pcd->polar.error[ni] );
     } while ( ++ni < NUM_POLAR_COEFFS );
     IEEE_Swap__FLT( &pcd->polar.chi );
     pcd->ihr.subsetcounter = byte_swap_u16( pcd->ihr.subsetcounter );
     pcd->ihr.prism_temp    = byte_swap_u16( pcd->ihr.prism_temp );
     pcd->ihr.intg.two_byte = byte_swap_u16( pcd->ihr.intg.two_byte );
     ni = 0;
     do { 
	  pcd->ihr.peltier[ni] = byte_swap_16( pcd->ihr.peltier[ni] );
     } while ( ++ni < SCIENCE_CHANNELS );
     ni = 0;
     do { 
	  pcd->ihr.pmd[0][ni] = byte_swap_u16( pcd->ihr.pmd[0][ni] );
	  pcd->ihr.pmd[1][ni] = byte_swap_u16( pcd->ihr.pmd[1][ni] );
	  pcd->ihr.pmd[2][ni] = byte_swap_u16( pcd->ihr.pmd[2][ni] );
     } while ( ++ni < PMD_IN_GRID );
     ni = 0;
     do {
	  pcd->indx_bands[ni] = byte_swap_16( pcd->indx_bands[ni] );
     } while ( ++ni < NUM_SPEC_BANDS );
}
#endif

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
short GOME_LV1_RD_PCD( FILE *infl, const struct fsr1_gome *fsr, 
		       const struct sph1_gome *sph,
		       struct pcd_gome **pcd_out )
{
     const char prognm[] = "GOME_LV1_RD_PCD";

     register short ni;
     register int   nr;

     char  *pcd_char = NULL;
     char  *pcd_pntr;
     char  mph_buff[MPH_SIZE], sph_buff[SPH_SIZE];

     unsigned short status2;
     unsigned short ihr_buff[IHR_SIZE];
     size_t         nr_bytes;

     short  nr_pcd = 0;

     struct pcd_gome *pcd;

     const long byte_offs = (long) (LVL1_PIR_LENGTH + LVL1_FSR_LENGTH 
				    + fsr->sz_sph + fsr->sz_fcd);
/*
 * check number of PCD records
 */
     if ( fsr->nr_pcd == 0 ) return 0;
/*
 * allocate memory for the PCD records
 */
     if ( ! Use_Extern_Alloc ) {
	  pcd_out[0] = (struct pcd_gome *)
	       malloc( fsr->nr_pcd * sizeof( struct pcd_gome ) );
     }
     if ( (pcd = pcd_out[0]) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "pcd" );
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (pcd_char = (char *) malloc( (size_t) fsr->sz_pcd )) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "pcd_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( infl, (long) byte_offs, SEEK_SET );
/*
 * read data buffer to PCD structure
 */
     do { 
	  pcd_pntr = pcd_char;
	  if ( fread( pcd_pntr, (size_t) fsr->sz_pcd, 1, infl ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
/* copy data of GLR1 record */
	  (void) memcpy( &pcd->glr.utc_date, pcd_pntr, GOME_UINT );
	  pcd_pntr += GOME_UINT;
	  (void) memcpy( &pcd->glr.utc_time, pcd_pntr, GOME_UINT );
	  pcd_pntr += GOME_UINT;
	  for ( ni = 0; ni < 3; ni++ ) {
	       (void) memcpy( pcd->glr.sun_zen_sat_north+ni, pcd_pntr, 
				GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( pcd->glr.sun_azim_sat_north+ni, pcd_pntr, 
				GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	  }
	  for ( ni = 0; ni < 3; ni++ ) {
	       (void) memcpy( pcd->glr.los_zen_sat_north+ni, pcd_pntr, 
				GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( pcd->glr.los_azim_sat_north+ni, pcd_pntr, 
				GOME_FLOAT);
	       pcd_pntr += GOME_FLOAT;
	  }
	  for ( ni = 0; ni < 3; ni++ ) {
	       (void) memcpy( pcd->glr.sun_zen_sat_ers+ni, pcd_pntr, 
				GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( pcd->glr.sun_azim_sat_ers+ni, pcd_pntr, 
				GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	  }
	  for ( ni = 0; ni < 3; ni++ ) {
	       (void) memcpy( pcd->glr.los_zen_sat_ers+ni, pcd_pntr, 
				GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( pcd->glr.los_azim_sat_ers+ni, pcd_pntr, 
				GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	  }
	  if ( sph->prod_version > 1 ) {
	       for ( ni = 0; ni < 3; ni++ ) {
		    (void) memcpy( pcd->glr.sun_zen_surf_north+ni, pcd_pntr, 
				   GOME_FLOAT );
		    pcd_pntr += GOME_FLOAT;
		    (void) memcpy( pcd->glr.sun_azim_surf_north+ni, pcd_pntr, 
				   GOME_FLOAT );
		    pcd_pntr += GOME_FLOAT;
	       }
	       for ( ni = 0; ni < 3; ni++ ) {
		    (void) memcpy( pcd->glr.los_zen_surf_north+ni, pcd_pntr, 
				   GOME_FLOAT );
		    pcd_pntr += GOME_FLOAT;
		    (void) memcpy( pcd->glr.los_azim_surf_north+ni, pcd_pntr, 
				   GOME_FLOAT );
		    pcd_pntr += GOME_FLOAT;
	       }
	  } else {
	       pcd->glr.sun_zen_surf_north[0] = 
		    pcd->glr.sun_zen_surf_north[1] = 
		    pcd->glr.sun_zen_surf_north[2] = 0.f;
	       pcd->glr.sun_azim_surf_north[0] = 
		    pcd->glr.sun_azim_surf_north[1] = 
		    pcd->glr.sun_azim_surf_north[2] = 0.f;
	       pcd->glr.los_zen_surf_north[0] = 
		    pcd->glr.los_zen_surf_north[1] = 
		    pcd->glr.los_zen_surf_north[2] = 0.f;
	       pcd->glr.los_azim_surf_north[0] = 
		    pcd->glr.los_azim_surf_north[1] = 
		    pcd->glr.los_azim_surf_north[2] = 0.f;
	  }	  
	  (void) memcpy( &pcd->glr.sat_geo_height, pcd_pntr, GOME_FLOAT );
	  pcd_pntr += GOME_FLOAT;
	  (void) memcpy( &pcd->glr.earth_radius, pcd_pntr, GOME_FLOAT );
	  pcd_pntr += GOME_FLOAT;
	  (void) memcpy( &pcd->glr.sun_glint, pcd_pntr, GOME_CHAR );
	  pcd_pntr += GOME_CHAR;
	  for ( nr = 0; nr < NUM_COORDS; nr++ ) {
	       (void) memcpy( &pcd->glr.lat[nr], pcd_pntr, GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( &pcd->glr.lon[nr], pcd_pntr, GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	  }
/* copy Cloud record */
	  if ( sph->prod_version > 1 ) {
	       (void) memcpy( &pcd->cld.mode, pcd_pntr, GOME_SHORT );
	       pcd_pntr += GOME_SHORT;
	       (void) memcpy( &pcd->cld.surfaceHeight, pcd_pntr, GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( &pcd->cld.fraction, pcd_pntr, GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( &pcd->cld.fractionError, pcd_pntr, GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( &pcd->cld.albedo, pcd_pntr, GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( &pcd->cld.albedoError, pcd_pntr, GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( &pcd->cld.height, pcd_pntr, GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( &pcd->cld.heightError, pcd_pntr, GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( &pcd->cld.thickness, pcd_pntr, GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( &pcd->cld.thicknessError, pcd_pntr, GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( &pcd->cld.topPress, pcd_pntr, GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( &pcd->cld.topPressError, pcd_pntr, GOME_FLOAT );
	       pcd_pntr += GOME_FLOAT;
	       (void) memcpy( &pcd->cld.type, pcd_pntr, GOME_SHORT );
	       pcd_pntr += GOME_SHORT;
	  } else {
	       pcd->cld.mode = 0;
	       pcd->cld.type = 0;
	       pcd->cld.surfaceHeight = 0.f;
	       pcd->cld.fraction = 0.f;
	       pcd->cld.fractionError = 0.f;
	       pcd->cld.albedo = 0.f;
	       pcd->cld.albedoError = 0.f;
	       pcd->cld.height = 0.f;
	       pcd->cld.heightError = 0.f;
	       pcd->cld.thickness = 0.f;
	       pcd->cld.thicknessError = 0.f;
	       pcd->cld.topPress = 0.f;
	       pcd->cld.topPressError = 0.f;
	  }
/* copy Dark Current and Noise Factor */
	  (void) memcpy( &pcd->dark_current, pcd_pntr, GOME_FLOAT );
	  pcd_pntr += GOME_FLOAT;
	  (void) memcpy( &pcd->noise_factor, pcd_pntr, GOME_FLOAT );
	  pcd_pntr += GOME_FLOAT;
/* copy index to Spectral calibration parameters */
	  (void) memcpy( &pcd->indx_spec, pcd_pntr, GOME_SHORT );
	  pcd_pntr += GOME_SHORT;
/* copy index to Leakage correction parameters */
	  (void) memcpy( &pcd->indx_leak, pcd_pntr, GOME_SHORT );
	  pcd_pntr += GOME_SHORT;
/* copy Polarization parameters */
	  nr_bytes = NUM_POLAR_COEFFS * GOME_FLOAT;
	  (void) memcpy( pcd->polar.coeff, pcd_pntr, nr_bytes );
	  pcd_pntr += nr_bytes;
	  (void) memcpy( pcd->polar.wv, pcd_pntr, nr_bytes );
	  pcd_pntr += nr_bytes;
	  (void) memcpy( pcd->polar.error, pcd_pntr, nr_bytes );
	  pcd_pntr += nr_bytes;
	  (void) memcpy( &pcd->polar.chi, pcd_pntr, GOME_FLOAT );
	  pcd_pntr += GOME_FLOAT;
/* copy level 0 MPH */
	  (void) memcpy( mph_buff, pcd_pntr, MPH_SIZE );
	  pcd_pntr += MPH_SIZE;
/* copy level 0 SPH */
	  (void) memcpy( sph_buff, pcd_pntr, SPH_SIZE );
	  pcd_pntr += SPH_SIZE;
/* copy Instrument Header Record */
	  (void) memcpy( ihr_buff, pcd_pntr, IHR_SIZE * GOME_USHRT );
	  pcd_pntr += IHR_SIZE * GOME_USHRT;
/* copy index to the 10 spectral bands */
	  (void) memcpy( pcd->indx_bands, pcd_pntr, 
			   NUM_SPEC_BANDS * GOME_SHORT );
	  pcd_pntr += NUM_SPEC_BANDS * GOME_SHORT;
/*
 * read Level 0 data MPH
 */
	  (void) strlcpy( pcd->mph0.ProductConfidenceData, mph_buff, 3 );
	  (void) strlcpy( pcd->mph0.UTC_MPH_Generation, mph_buff+2, 25 );
	  (void) strlcpy( pcd->mph0.ProcessorSoftwareVersion, mph_buff+26, 9 );
/*
 * read Level 0 data SPH
 */
	  (void) strlcpy( pcd->sph0.sph_5, sph_buff, 2 );
	  (void) strlcpy( pcd->sph0.sph_6, sph_buff+1, 21 );
/*
 * Get subset counter, PMD and Peltier values
 */
	  pcd->ihr.subsetcounter = ihr_buff[sph->subset_entry];
	  pcd->ihr.prism_temp = ihr_buff[PRE_DISP_TEMP_ENTRY];
	  pcd->ihr.intg.two_byte = ihr_buff[sph->intgstat_entry];
	  for ( nr = 0; nr < PMD_IN_GRID; nr++ ) {
	       pcd->ihr.pmd[0][nr] = ihr_buff[sph->pmd_entry + nr * 4];
	       pcd->ihr.pmd[1][nr] = ihr_buff[sph->pmd_entry + nr * 4 + 1];
	       pcd->ihr.pmd[2][nr] = ihr_buff[sph->pmd_entry + nr * 4 + 2];
	  }
	  for ( nr = 0; nr < SCIENCE_CHANNELS; nr++ ) {
	       pcd->ihr.peltier[nr] = 
		    (short) ihr_buff[sph->peltier_entry + nr];
	  }
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  status2 = byte_swap_u16( ihr_buff[sph->status2_entry] );
#else
	  status2 = ihr_buff[sph->status2_entry];
#endif
	  if ((AVERAGE_MODE_MASK & status2) == 0 )
	       pcd->ihr.averagemode = FALSE;
	  else
	       pcd->ihr.averagemode = TRUE;
/*
 * check if we read the whole DSR
 */
	  if ( pcd_pntr - pcd_char != fsr->sz_pcd )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, "PCD size" );

#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_PCD( pcd );
#endif
	  pcd->glr.subsetcounter = 
	       (unsigned char) pcd->ihr.subsetcounter;
/* quality check on Ocra/Rocinn cloud information */
	  if ( pcd->cld.type > 9 || pcd->cld.type < 0 ) pcd->cld.type = 0;
	  if ( pcd->cld.mode > 1 || pcd->cld.mode < 0 ) pcd->cld.mode = 0;
	  if ( pcd->cld.albedo < 0.f || pcd->cld.albedoError < 0.f )
	       pcd->cld.thicknessError = -1.f;
	  else
	       pcd->cld.thicknessError = 
		    pcd->cld.albedoError / (0.2f + pcd->cld.albedo);	  
	  if ( pcd->cld.height < 0.f || pcd->cld.heightError < 0.f )
	       pcd->cld.topPressError = -1.f;
	  else
	       pcd->cld.topPressError = (float)
		    (pcd->cld.heightError * (pcd->cld.height * M_LN2 / 5.6));
/* move to next record */
	  pcd++;
     } while ( ++nr_pcd < fsr->nr_pcd );
 done:
     if ( pcd_char != NULL ) free( pcd_char );
/*
 * set return value
 */
     return nr_pcd;
}
