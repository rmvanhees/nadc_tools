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

.IDENTifer   GOME_LV1_RD_SMCD
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read a Sun/Moon Specific Calibration record
.INPUT/OUTPUT
  call as   nr_smcd = GOME_LV1_RD_SMCD( infl, fsr, sph, &smcd );

     input:  
            FILE   *infl            : (open) file descriptor 
	    struct fsr1_gome *fsr   : structure for the FSR record
	    struct sph1_gome *sph   : structure for the SPH record
    output:  
            struct smcd_gome **smcd : structure for the SMCD record(s)

.RETURNS     number of SMCD records found (modifies error status)
.COMMENTS    None
.ENVIRONment None
.VERSION      3.3   06-Mar-2003 use global "Use_Extern_Alloc", RvH
              3.2   22-Nov-2001	mixing sz_scd and sz_mcd :-(, RvH
              3.1   20-Nov-2001	check number of records to be written, RvH
              3.0   11-Nov-2001	moved to the new Error handling routines, RvH 
              2.1   19-Jul-2001	pass structures using pointers, RvH 
              2.0   07-Sep-2000 major rewrite and standardization, RvH
              1.3   17-Feb-1999 conform to ANSI C3.159-1989, RvH
              1.2   20-Aug-1999 read prism-temp from IHR, RvH
              1.1   08-Apr-1999 read Instrument Header Record, RvH
              1.0   05-Feb-1999 created by R. M. van Hees
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

#define AVERAGE_MODE_MASK   (0x0040)

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_SMCD( struct smcd_gome *smcd )
{
     register int ni;

     smcd->indx_spec = byte_swap_16( smcd->indx_spec );
     smcd->indx_leak = byte_swap_16( smcd->indx_leak );
     ni = 0;
     do {
	  smcd->indx_bands[ni] = byte_swap_16( smcd->indx_bands[ni] );
     } while ( ++ni < NUM_SPEC_BANDS );
     smcd->utc_date = byte_swap_u32( smcd->utc_date );
     smcd->utc_time = byte_swap_u32( smcd->utc_time );
     IEEE_Swap__FLT( &smcd->north_sun_zen );
     IEEE_Swap__FLT( &smcd->north_sun_azim );
     IEEE_Swap__FLT( &smcd->north_sm_zen );
     IEEE_Swap__FLT( &smcd->north_sm_azim );
     IEEE_Swap__FLT( &smcd->sun_or_moon );
     IEEE_Swap__FLT( &smcd->dark_current );
     IEEE_Swap__FLT( &smcd->noise_factor );
     smcd->ihr.subsetcounter = byte_swap_u16( smcd->ihr.subsetcounter );
     smcd->ihr.prism_temp    = byte_swap_u16( smcd->ihr.prism_temp );
     smcd->ihr.averagemode   = byte_swap_u16( smcd->ihr.averagemode );
     smcd->ihr.intg.two_byte = byte_swap_u16( smcd->ihr.intg.two_byte );
     ni = 0;
     do { 
	  smcd->ihr.pmd[0][ni] = byte_swap_u16( smcd->ihr.pmd[0][ni] );
	  smcd->ihr.pmd[1][ni] = byte_swap_u16( smcd->ihr.pmd[1][ni] );
	  smcd->ihr.pmd[2][ni] = byte_swap_u16( smcd->ihr.pmd[2][ni] );
     } while ( ++ni < PMD_IN_GRID );
     ni = 0;
     do { 
	  smcd->ihr.peltier[ni] = byte_swap_u16( smcd->ihr.peltier[ni] );
     } while ( ++ni < SCIENCE_CHANNELS );
}
#endif

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
short GOME_LV1_RD_SMCD( unsigned char source, FILE *infl, 
			const struct fsr1_gome *fsr, 
			const struct sph1_gome *sph,
			struct smcd_gome **smcd_out )
{
     register int nr;

     char  *smcd_char = NULL;
     char  *smcd_pntr;
     char  mph_buff[MPH_SIZE], sph_buff[SPH_SIZE];

     unsigned short ihr_buff[IHR_SIZE];

     short  nr_smcd = 0;                   /* initialize return value */
     short  num_smcd;
     int    sz_smcd;
     long   byte_offs;

     struct smcd_gome *smcd;
/*
 * initialize some constants
 */
     if ( source == FLAG_SUN ) {
	  num_smcd = fsr->nr_scd;
	  sz_smcd  = fsr->sz_scd;
	  byte_offs = (long) (LVL1_PIR_LENGTH + LVL1_FSR_LENGTH 
			      + fsr->sz_sph + fsr->sz_fcd 
			      + fsr->nr_pcd * fsr->sz_pcd);
     } else {
	  num_smcd = fsr->nr_mcd;
	  sz_smcd  = fsr->sz_mcd;
	  byte_offs = (long) (LVL1_PIR_LENGTH + LVL1_FSR_LENGTH 
			      + fsr->sz_sph + fsr->sz_fcd 
			      + fsr->nr_pcd * fsr->sz_pcd
			      + fsr->nr_scd * fsr->sz_scd);
     }
/*
 * check number of SMCD records
 */
     if ( num_smcd == 0 ) return 0;
/*
 * allocate memory for the SMCD records
 */
     if ( ! Use_Extern_Alloc ) {
	  smcd_out[0] = (struct smcd_gome *)
	       malloc( num_smcd * sizeof( struct smcd_gome ) );
     }
     if ( (smcd = smcd_out[0]) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "smcd" );
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (smcd_char = (char *) malloc( (size_t) sz_smcd )) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "smcd_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( infl, byte_offs, SEEK_SET );
/*
 * read data buffer to SMCD structure
 */
     do {
	  if ( fread( smcd_char, sz_smcd, 1, infl ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
	  (void) memcpy( &smcd->utc_date, smcd_char, GOME_UINT );
	  smcd_pntr = smcd_char + GOME_UINT;
	  (void) memcpy( &smcd->utc_time, smcd_pntr, GOME_UINT );
	  smcd_pntr += GOME_UINT;
	  (void) memcpy( &smcd->north_sun_zen, smcd_pntr, GOME_FLOAT );
	  smcd_pntr += GOME_FLOAT;
	  (void) memcpy( &smcd->north_sun_azim, smcd_pntr, GOME_FLOAT);
	  smcd_pntr += GOME_FLOAT;
	  (void) memcpy( &smcd->north_sm_zen, smcd_pntr, GOME_FLOAT );
	  smcd_pntr += GOME_FLOAT;
	  (void) memcpy( &smcd->north_sm_azim, smcd_pntr, GOME_FLOAT );
	  smcd_pntr += GOME_FLOAT;
	  (void) memcpy( &smcd->sun_or_moon, smcd_pntr, GOME_FLOAT );
	  smcd_pntr += GOME_FLOAT;
	  (void) memcpy( &smcd->dark_current, smcd_pntr, GOME_FLOAT );
	  smcd_pntr += GOME_FLOAT;
	  (void) memcpy( &smcd->noise_factor, smcd_pntr, GOME_FLOAT );
	  smcd_pntr += GOME_FLOAT;
	  (void) memcpy( &smcd->indx_spec, smcd_pntr, GOME_SHORT );
	  smcd_pntr += GOME_SHORT;
	  (void) memcpy( &smcd->indx_leak, smcd_pntr, GOME_SHORT );
	  smcd_pntr += GOME_SHORT;
	  (void) memcpy( mph_buff, smcd_pntr, MPH_SIZE );
	  smcd_pntr += MPH_SIZE;
	  (void) memcpy( sph_buff, smcd_pntr, SPH_SIZE );
	  smcd_pntr += SPH_SIZE;
	  (void) memcpy( ihr_buff, smcd_pntr, IHR_SIZE * GOME_USHRT );
	  smcd_pntr += IHR_SIZE * GOME_USHRT;
	  (void) memcpy( smcd->indx_bands, smcd_pntr, 
			   NUM_SPEC_BANDS * GOME_SHORT );
	  smcd_pntr += NUM_SPEC_BANDS * GOME_SHORT;
/*
 * read Level 0 data MPH
 */
	  (void) nadc_strlcpy( smcd->mph0.ProductConfidenceData, mph_buff, 3 );
	  (void) nadc_strlcpy( smcd->mph0.UTC_MPH_Generation, mph_buff+2, 25 );
	  (void) nadc_strlcpy( smcd->mph0.ProcessorSoftwareVersion, 
			       mph_buff+26, 9);
/*
 * read Level 0 data SPH
 */
	  (void) nadc_strlcpy( smcd->sph0.sph_5, sph_buff, 2 );
	  (void) nadc_strlcpy( smcd->sph0.sph_6, sph_buff+1, 21 );
/*
 * Get subset counter, PMD and Peltier values
 */
	  smcd->ihr.subsetcounter = ihr_buff[sph->subset_entry];
	  smcd->ihr.prism_temp = ihr_buff[PRE_DISP_TEMP_ENTRY];
	  smcd->ihr.intg.two_byte = ihr_buff[sph->intgstat_entry];
	  for ( nr = 0; nr < PMD_IN_GRID; nr++ ) {
	       smcd->ihr.pmd[0][nr] = ihr_buff[sph->pmd_entry + nr * 4];
	       smcd->ihr.pmd[1][nr] = ihr_buff[sph->pmd_entry + nr * 4 + 1];
	       smcd->ihr.pmd[2][nr] = ihr_buff[sph->pmd_entry + nr * 4 + 2];
	  }
	  for ( nr = 0; nr < SCIENCE_CHANNELS; nr++ ) {
	       smcd->ihr.peltier[nr] = 
		    (short) ihr_buff[sph->peltier_entry + nr];
	  }
	  if ((AVERAGE_MODE_MASK & ihr_buff[sph->status2_entry]) == 0 )
	       smcd->ihr.averagemode = FALSE;
	  else
	       smcd->ihr.averagemode = TRUE;
/*
 * check if we read the whole DSR
 */
	  if ( smcd_pntr - smcd_char != sz_smcd )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, "SMCD size" );

#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_SMCD( smcd );
#endif
	  smcd++;
     } while ( ++nr_smcd < num_smcd );
 done:
     if ( smcd_char != NULL ) free( smcd_char );
/*
 * set return value
 */
     return nr_smcd;
}
