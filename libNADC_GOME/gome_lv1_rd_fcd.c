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

.IDENTifer   GOME_LV1_RD_FCD
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1 data
.LANGUAGE    ANSI C
.PURPOSE     Read the Fixed Calibration Record
.INPUT/OUTPUT
  call as   num_fcd = GOME_LV1_RD_FCD( infl, fsr, &fcd );

     input:  
            FILE   *infl          : (open) file descriptor 
	    struct fsr1_gome *fsr : structure for the FSR record
    output:  
            struct fcd_gome *fcd  : structure for the FCD record

.RETURNS     number of FCD records found (modifies error status)
.COMMENTS    None
.ENVIRONment None
.VERSION      2.1   07-Sep-2005 return number of FCD records found, RvH
              2.0   11-Nov-2001	moved to the new Error handling routines, RvH
              1.1   17-Feb-1999	conform to ANSI C3.159-1989, RvH 
              1.0   10-Feb-1999 created by R. M. van Hees
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
#define _GOME_COMMON
#include <nadc_gome.h>

#define LVL1_FCD_NLEAK  17252
#define LVL1_FCD_NANG   49188

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_FCD( struct fcd_gome *fcd )
{
     register short ng;
     register int   ni, nj, nr;

     ni = 0;
     do {
	  fcd->bcr[ni].array_nr = byte_swap_16( fcd->bcr[ni].array_nr );
	  fcd->bcr[ni].start = byte_swap_16( fcd->bcr[ni].start );
	  fcd->bcr[ni].end = byte_swap_16( fcd->bcr[ni].end );
     } while ( ++ni < NUM_SPEC_BANDS );

     ni = 0;
     do {
	  IEEE_Swap__FLT( &fcd->stray_level[ni] );

	  IEEE_Swap__FLT( &fcd->kde.bsdf_1[ni] );
	  IEEE_Swap__FLT( &fcd->kde.bsdf_2[ni] );
	  IEEE_Swap__FLT( &fcd->kde.resp_1[ni] );
	  IEEE_Swap__FLT( &fcd->kde.resp_2[ni] );
	  IEEE_Swap__FLT( &fcd->kde.f2_1[ni] );
	  IEEE_Swap__FLT( &fcd->kde.f2_2[ni] );
	  IEEE_Swap__FLT( &fcd->kde.smdep_1[ni] );
	  IEEE_Swap__FLT( &fcd->kde.smdep_2[ni] );
	  IEEE_Swap__FLT( &fcd->kde.chi_1[ni] );
	  IEEE_Swap__FLT( &fcd->kde.chi_2[ni] );
	  IEEE_Swap__FLT( &fcd->kde.eta_1[ni] );
	  IEEE_Swap__FLT( &fcd->kde.eta_2[ni] );
	  IEEE_Swap__FLT( &fcd->kde.ksi_1[ni] );
	  IEEE_Swap__FLT( &fcd->kde.ksi_2[ni] );

	  for ( ng = 0; ng < NUM_STRAY_GHOSTS; ng++ ) {
	       fcd->ghost.symmetry[ni][ng] = 
		    byte_swap_16( fcd->ghost.symmetry[ni][ng] );
	       fcd->ghost.center[ni][ng] = 
		    byte_swap_16( fcd->ghost.center[ni][ng] );
	       IEEE_Swap__FLT( &fcd->ghost.defocus[ni][ng] );
	       IEEE_Swap__FLT( &fcd->ghost.energy[ni][ng] );
	  }
     } while ( ++ni < SCIENCE_CHANNELS );
     ni = 0;
     do {
	  IEEE_Swap__FLT( &fcd->kde.rfs[ni] );
     } while( ++ni < SCIENCE_PIXELS );

     IEEE_Swap__FLT( &fcd->bsdf_0 );
     IEEE_Swap__FLT( &fcd->elevation );
     IEEE_Swap__FLT( &fcd->azimuth );

     ni = 0;
     do {
	  IEEE_Swap__FLT( &fcd->coeffs[ni] );
     } while ( ++ni < 8 );
     fcd->width_conv = byte_swap_16( fcd->width_conv );
     ni = 0;
     do {
	  IEEE_Swap__FLT( &fcd->scale_peltier[ni] );
     } while ( ++ni < NUM_FPA_SCALE );
     fcd->npeltier = byte_swap_16( fcd->npeltier );
     ni = 0;
     do {
	  IEEE_Swap__FLT( &fcd->filter_peltier[ni] );
     } while ( ++ni < NUM_FPA_COEFFS );
/*
 * Nleak is already byte-swapped
 */
     for ( ni = 0; ni < fcd->nleak; ni++ ) {
	  IEEE_Swap__FLT( &fcd->leak[ni].noise );
	  nj = 0;
	  do {
	       IEEE_Swap__FLT( &fcd->leak[ni].pmd_offs[nj] );
	  } while ( ++nj < PMD_NUMBER );
	  IEEE_Swap__FLT( &fcd->leak[ni].pmd_noise );
	  nj = 0;
	  do {
	       IEEE_Swap__FLT( &fcd->leak[ni].dark[nj] );
	  } while ( ++nj < SCIENCE_PIXELS );
     }
/*
 * Nhot is already byte-swapped. Note fcd->nhot can be zero
 */
     for ( ni = 0; ni < fcd->nhot; ni++ ) {
	  fcd->hot[ni].record = byte_swap_16( fcd->hot[ni].record );
	  fcd->hot[ni].array  = byte_swap_16( fcd->hot[ni].array );
	  fcd->hot[ni].pixel  = byte_swap_16( fcd->hot[ni].pixel );
     }
/*
 * Nspec is already byte-swapped
 */
     for ( nr = 0; nr < fcd->nspec; nr++ ) {
	  for ( ni = 0; ni < SCIENCE_CHANNELS; ni++ ) {
	       for ( nj = 0; nj < NUM_SPEC_COEFFS; nj++ )
		    IEEE_Swap__DBL( &fcd->spec[nr].coeffs[ni][nj] );
	       IEEE_Swap__DBL( &fcd->spec[nr].error[ni] );
	  }
     }
     fcd->indx_spec = byte_swap_16( fcd->indx_spec );
     ni = 0;
     do { 
	  IEEE_Swap__FLT( &fcd->pixel_gain[ni] );
	  IEEE_Swap__FLT( &fcd->intensity[ni] );
	  IEEE_Swap__FLT( &fcd->sun_ref[ni] );
	  IEEE_Swap__FLT( &fcd->sun_precision[ni] );
     } while ( ++ni < SCIENCE_PIXELS );
     ni = 0;
     do {
	  IEEE_Swap__FLT( &fcd->sun_pmd[ni] );
	  IEEE_Swap__FLT( &fcd->sun_pmd_wv[ni] );
     } while ( ++ni < PMD_NUMBER );
     fcd->sun_date = byte_swap_u32( fcd->sun_date );
     fcd->sun_time = byte_swap_u32( fcd->sun_time );
/*
 * Nang is already byte-swapped
 */
     for ( nr = 0; nr < fcd->nang; nr++ ) {
	  for ( ni = 0; ni < CHANNEL_SIZE; ni++ ) {
	       IEEE_Swap__FLT( &fcd->calib[nr].eta_omega[ni] );
	       IEEE_Swap__FLT( &fcd->calib[nr].response[ni] );
	  }
     }
}
#endif

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
short GOME_LV1_RD_FCD( FILE *infl, const struct fsr1_gome *fsr, 
		       struct fcd_gome *fcd )
{
     const char prognm[] = "GOME_LV1_RD_FCD";

     register short ng;
     register int   nr;

     char   *fcd_char, *fcd_pntr;
     short  num_fcd = 0;
     int    nrpix;
     long   byte_offs;
     size_t nbytes, nr_read, num;
     float  rbuff[2 * SCIENCE_CHANNELS];
/*
 * allocate memory to store data from FCD-record up to Nleak
 */
     if ( (fcd_char = (char *) malloc( LVL1_FCD_NLEAK )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "fcd_char" );
/*
 * rewind/read input data file
 */
     byte_offs = (long) (LVL1_PIR_LENGTH + LVL1_FSR_LENGTH + fsr->sz_sph);
     (void) fseek( infl, byte_offs, SEEK_SET );
     if ( fread( fcd_char, LVL1_FCD_NLEAK, 1, infl ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to FCD structure
 */
     (void) memcpy( &fcd->detector_flags.flags, fcd_char, GOME_SHORT );
     fcd_pntr = fcd_char + GOME_SHORT;
     nr = 0;
     do {
	  (void) memcpy( &fcd->bcr[nr].array_nr,fcd_pntr,GOME_SHORT );
	  fcd_pntr += GOME_SHORT;
	  (void) memcpy( &fcd->bcr[nr].start, fcd_pntr, GOME_SHORT );
	  fcd_pntr += GOME_SHORT;
	  (void) memcpy( &fcd->bcr[nr].end, fcd_pntr, GOME_SHORT );
	  fcd_pntr += GOME_SHORT;
     } while ( ++nr < NUM_SPEC_BANDS );

     nrpix = 2 * SCIENCE_CHANNELS;
     (void) memcpy( rbuff, fcd_pntr, nrpix * GOME_FLOAT );
     fcd_pntr += nrpix * GOME_FLOAT;
     fcd->kde.bsdf_1[0] = rbuff[0]; fcd->kde.bsdf_2[0] = rbuff[1];
     fcd->kde.bsdf_1[1] = rbuff[2]; fcd->kde.bsdf_2[1] = rbuff[3];
     fcd->kde.bsdf_1[2] = rbuff[4]; fcd->kde.bsdf_2[2] = rbuff[5];
     fcd->kde.bsdf_1[3] = rbuff[6]; fcd->kde.bsdf_2[3] = rbuff[7];
     (void) memcpy( rbuff, fcd_pntr, nrpix * GOME_FLOAT );
     fcd_pntr += nrpix * GOME_FLOAT;
     fcd->kde.resp_1[0] = rbuff[0]; fcd->kde.resp_2[0] = rbuff[1];
     fcd->kde.resp_1[1] = rbuff[2]; fcd->kde.resp_2[1] = rbuff[3];
     fcd->kde.resp_1[2] = rbuff[4]; fcd->kde.resp_2[2] = rbuff[5];
     fcd->kde.resp_1[3] = rbuff[6]; fcd->kde.resp_2[3] = rbuff[7];
     (void) memcpy( rbuff, fcd_pntr, nrpix * GOME_FLOAT );
     fcd_pntr += nrpix * GOME_FLOAT;
     fcd->kde.f2_1[0] = rbuff[0]; fcd->kde.f2_2[0] = rbuff[1];
     fcd->kde.f2_1[1] = rbuff[2]; fcd->kde.f2_2[1] = rbuff[3];
     fcd->kde.f2_1[2] = rbuff[4]; fcd->kde.f2_2[2] = rbuff[5];
     fcd->kde.f2_1[3] = rbuff[6]; fcd->kde.f2_2[3] = rbuff[7];
     (void) memcpy( rbuff, fcd_pntr, nrpix * GOME_FLOAT );
     fcd_pntr += nrpix * GOME_FLOAT;
     fcd->kde.smdep_1[0] = rbuff[0]; fcd->kde.smdep_2[0] = rbuff[1];
     fcd->kde.smdep_1[1] = rbuff[2]; fcd->kde.smdep_2[1] = rbuff[3];
     fcd->kde.smdep_1[2] = rbuff[4]; fcd->kde.smdep_2[2] = rbuff[5];
     fcd->kde.smdep_1[3] = rbuff[6]; fcd->kde.smdep_2[3] = rbuff[7];
     (void) memcpy( rbuff, fcd_pntr, nrpix * GOME_FLOAT );
     fcd_pntr += nrpix * GOME_FLOAT;
     fcd->kde.chi_1[0] = rbuff[0]; fcd->kde.chi_2[0] = rbuff[1];
     fcd->kde.chi_1[1] = rbuff[2]; fcd->kde.chi_2[1] = rbuff[3];
     fcd->kde.chi_1[2] = rbuff[4]; fcd->kde.chi_2[2] = rbuff[5];
     fcd->kde.chi_1[3] = rbuff[6]; fcd->kde.chi_2[3] = rbuff[7];
     (void) memcpy( rbuff, fcd_pntr, nrpix * GOME_FLOAT );
     fcd_pntr += nrpix * GOME_FLOAT;
     fcd->kde.eta_1[0] = rbuff[0]; fcd->kde.eta_2[0] = rbuff[1];
     fcd->kde.eta_1[1] = rbuff[2]; fcd->kde.eta_2[1] = rbuff[3];
     fcd->kde.eta_1[2] = rbuff[4]; fcd->kde.eta_2[2] = rbuff[5];
     fcd->kde.eta_1[3] = rbuff[6]; fcd->kde.eta_2[3] = rbuff[7];
     (void) memcpy( rbuff, fcd_pntr, nrpix * GOME_FLOAT );
     fcd_pntr += nrpix * GOME_FLOAT;
     fcd->kde.ksi_1[0] = rbuff[0]; fcd->kde.ksi_2[0] = rbuff[1];
     fcd->kde.ksi_1[1] = rbuff[2]; fcd->kde.ksi_2[1] = rbuff[3];
     fcd->kde.ksi_1[2] = rbuff[4]; fcd->kde.ksi_2[2] = rbuff[5];
     fcd->kde.ksi_1[3] = rbuff[6]; fcd->kde.ksi_2[3] = rbuff[7];
     (void) memcpy( fcd->kde.rfs, fcd_pntr, SCIENCE_PIXELS * GOME_FLOAT );
     fcd_pntr += SCIENCE_PIXELS * GOME_FLOAT;

     (void) memcpy( &fcd->bsdf_0, fcd_pntr, GOME_FLOAT );
     fcd_pntr += GOME_FLOAT;
     (void) memcpy( &fcd->elevation, fcd_pntr, GOME_FLOAT );
     fcd_pntr += GOME_FLOAT;
     (void) memcpy( &fcd->azimuth, fcd_pntr, GOME_FLOAT );
     fcd_pntr += GOME_FLOAT;
     (void) memcpy( fcd->coeffs, fcd_pntr, 8 * GOME_FLOAT );
     fcd_pntr += 8 * GOME_FLOAT;
     (void) memcpy( fcd->stray_level, fcd_pntr, 4 * GOME_FLOAT );
     fcd_pntr += 4 * GOME_FLOAT;

     nr = 0;
     do {
	  for ( ng = 0; ng < NUM_STRAY_GHOSTS; ng++ ) {
	       (void) memcpy( &fcd->ghost.symmetry[nr][ng], 
				fcd_pntr, GOME_SHORT );
	       fcd_pntr += GOME_SHORT;
	       (void) memcpy( &fcd->ghost.center[nr][ng], 
				fcd_pntr, GOME_SHORT );
	       fcd_pntr += GOME_SHORT;
	       (void) memcpy( &fcd->ghost.defocus[nr][ng], 
				fcd_pntr, GOME_FLOAT );
	       fcd_pntr += GOME_FLOAT;
	       (void) memcpy( &fcd->ghost.energy[nr][ng], 
				fcd_pntr, GOME_FLOAT );
	       fcd_pntr += GOME_FLOAT;
	  }
     } while ( ++nr < SCIENCE_CHANNELS );
     (void) memcpy( &fcd->width_conv, fcd_pntr, GOME_SHORT );
     fcd_pntr += GOME_SHORT;
     (void) memcpy( fcd->scale_peltier, fcd_pntr, 
		      NUM_FPA_SCALE * GOME_FLOAT );
     fcd_pntr += NUM_FPA_SCALE * GOME_FLOAT;
     (void) memcpy( &fcd->npeltier, fcd_pntr, GOME_SHORT );
     fcd_pntr += GOME_SHORT;
     (void) memcpy( fcd->filter_peltier, fcd_pntr, 
		      NUM_FPA_COEFFS * GOME_FLOAT );
     fcd_pntr += NUM_FPA_COEFFS * GOME_FLOAT;
     (void) memcpy( &fcd->nleak, fcd_pntr, GOME_SHORT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     fcd->nleak = byte_swap_16( fcd->nleak );
#endif
     fcd_pntr += GOME_SHORT;
     nbytes = (size_t) (fcd_pntr - fcd_char);
/*
 * deallocate memory
 */
     fcd_pntr = NULL;
     free( fcd_char );
/*
 * now we have to read the variable portions
 */
     fcd->leak = NULL;
     if ( fcd->nleak > 0 ) {
	  nr_read = (size_t) (fcd->nleak) * sizeof( struct lv1_leak );
	  if ( (fcd->leak = (struct lv1_leak *) malloc( nr_read )) == NULL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "fcd->leak" );
	  nr = 0;
	  do {
	       if ( fread( &fcd->leak[nr].noise, GOME_FLOAT, 1, infl ) != 1 )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	       nbytes += GOME_FLOAT;
	       num = fread( fcd->leak[nr].pmd_offs, GOME_FLOAT,
			    PMD_NUMBER, infl );
	       if ( num != PMD_NUMBER ) 
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	       nbytes += GOME_FLOAT * PMD_NUMBER;
	       if ( fread(&fcd->leak[nr].pmd_noise, GOME_FLOAT, 1, infl) != 1 )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	       nbytes += GOME_FLOAT;
	       num = fread( fcd->leak[nr].dark, GOME_FLOAT,
			    SCIENCE_PIXELS, infl );
	       if ( num != SCIENCE_PIXELS ) 
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	       nbytes += GOME_FLOAT * SCIENCE_PIXELS;
	  } while ( ++nr < fcd->nleak );
     }
     num = fread( fcd->pixel_gain, GOME_FLOAT, SCIENCE_PIXELS, infl );
     if ( num != SCIENCE_PIXELS ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
     nbytes += GOME_FLOAT * SCIENCE_PIXELS;
     if ( fread( &fcd->nhot, GOME_SHORT, 1, infl ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
     nbytes += GOME_SHORT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     fcd->nhot = byte_swap_16( fcd->nhot );
#endif
     fcd->hot = NULL;
     if ( fcd->nhot > 0 ) {
	  nr_read = (size_t) (fcd->nhot) * sizeof( struct lv1_hot );
	  if ( (fcd->hot = (struct lv1_hot *) malloc( nr_read )) == NULL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "fcd->hot" );
	  nr = 0;
	  do {
	       if ( fread( &fcd->hot[nr].record, GOME_SHORT, 1, infl ) != 1 )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	       nbytes += GOME_SHORT;
	       if ( fread( &fcd->hot[nr].array, GOME_SHORT, 1, infl ) != 1 )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	       nbytes += GOME_SHORT;
	       if ( fread( &fcd->hot[nr].pixel, GOME_SHORT, 1, infl ) != 1 )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	       nbytes += GOME_SHORT;
	  } while ( ++nr < fcd->nhot );
     }
     if ( fread( &fcd->nspec, GOME_SHORT, 1, infl ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
     nbytes += GOME_SHORT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     fcd->nspec = byte_swap_16( fcd->nspec );
#endif
     fcd->spec = NULL;
     if ( fcd->nspec > 0 ) {
	  nr_read = (size_t) (fcd->nspec * sizeof( struct lv1_spec ));
	  if ( (fcd->spec = (struct lv1_spec *) malloc( nr_read )) == NULL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "fcd->spec" );
	  nr = 0;
	  do {
	       num = fread( fcd->spec[nr].coeffs, GOME_DBLE,
			    NUM_SPEC_COEFFS * SCIENCE_CHANNELS, infl );
	       if ( num != NUM_SPEC_COEFFS * SCIENCE_CHANNELS ) 
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	       nbytes += GOME_DBLE * NUM_SPEC_COEFFS * SCIENCE_CHANNELS;
	       num = fread( fcd->spec[nr].error, GOME_DBLE,
			    SCIENCE_CHANNELS, infl );
	       if ( num != SCIENCE_CHANNELS ) 
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	       nbytes += GOME_DBLE * SCIENCE_CHANNELS;
	  } while ( ++nr < fcd->nspec );
     }
/*
 * allocate memory to store data from FCD-record up to Nleak
 */
     if ( (fcd_char = (char *) malloc( LVL1_FCD_NANG )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "fcd_char" );
/*
 * read a next chunck of data until the next variable portion
 */
     if ( fread( fcd_char, LVL1_FCD_NANG, 1, infl ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
     (void) memcpy( &fcd->indx_spec, fcd_char, GOME_SHORT );
     fcd_pntr = fcd_char + GOME_SHORT;
     (void) memcpy( fcd->intensity, fcd_pntr, SCIENCE_PIXELS * GOME_FLOAT );
     fcd_pntr += SCIENCE_PIXELS * GOME_FLOAT;
     (void) memcpy( fcd->sun_ref, fcd_pntr, SCIENCE_PIXELS * GOME_FLOAT );
     fcd_pntr += SCIENCE_PIXELS * GOME_FLOAT;
     (void) memcpy( fcd->sun_precision, fcd_pntr, 
		      SCIENCE_PIXELS * GOME_FLOAT );
     fcd_pntr += SCIENCE_PIXELS * GOME_FLOAT;
     (void) memcpy( fcd->sun_pmd, fcd_pntr, PMD_NUMBER * GOME_FLOAT );
     fcd_pntr += PMD_NUMBER * GOME_FLOAT;
     (void) memcpy( fcd->sun_pmd_wv, fcd_pntr, PMD_NUMBER * GOME_FLOAT );
     fcd_pntr += PMD_NUMBER * GOME_FLOAT;
     (void) memcpy( &fcd->sun_date, fcd_pntr, GOME_UINT );
     fcd_pntr += GOME_UINT;
     (void) memcpy( &fcd->sun_time, fcd_pntr, GOME_UINT );
     fcd_pntr += GOME_UINT;
     (void) memcpy( &fcd->nang, fcd_pntr, GOME_SHORT );
     fcd_pntr += GOME_SHORT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     fcd->nang = byte_swap_16( fcd->nang );
#endif
     nbytes += (size_t) (fcd_pntr - fcd_char);
/*
 * deallocate memory
 */
     fcd_pntr = NULL;
     free( fcd_char );
/*
 * now we have to read the variable portions
 */
     fcd->calib = NULL;
     if ( fcd->nang > 0 ) {
	  nr_read = (size_t) (fcd->nang * sizeof( struct lv1_calib ));
	  if ( (fcd->calib = (struct lv1_calib *) malloc( nr_read )) == NULL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "fcd->calib" );
	  nr = 0;
	  do {
	       num = fread( fcd->calib[nr].eta_omega, GOME_FLOAT,
			     CHANNEL_SIZE, infl );
	       if ( num != CHANNEL_SIZE ) 
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	       nbytes += GOME_FLOAT * CHANNEL_SIZE;
	       num = fread( fcd->calib[nr].response, GOME_FLOAT,
			    CHANNEL_SIZE, infl );
	       if ( num != CHANNEL_SIZE ) 
		    NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	       nbytes += GOME_FLOAT * CHANNEL_SIZE;
	  } while ( ++nr < fcd->nang );
     }
/*
 * check if we read the whole DSR
 */
     if ( nbytes != (size_t) fsr->sz_fcd ) {
	  (void) printf( "%d %d\n", (int) nbytes, fsr->sz_fcd );
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, "FCD size" );
     } else
	  num_fcd = 1;

#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_FCD( fcd );
#endif
/*
 * set return values
 */
 done:
     return num_fcd;
}
