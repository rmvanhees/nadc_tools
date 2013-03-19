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

.IDENTifer   GOME_LV1_RD_FSR
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1 data
.LANGUAGE    ANSI C
.PURPOSE     Read the File Structure Record
.INPUT/OUTPUT
  call as   GOME_LV1_RD_FSR( infl, &fsr );

     input:  
            FILE   *infl           : (open) file descriptor 
    output:  
            struct fsr1_gome *fsr  : structure for the FSR record

.RETURNS     nothing: modifies error status
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   11-Nov-2001	moved to the new Error handling routines, RvH
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

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_FSR( struct fsr1_gome *fsr )
{
     register short nr;

     fsr->nr_sph  = byte_swap_16( fsr->nr_sph );
     fsr->sz_sph  = byte_swap_32( fsr->sz_sph );
     fsr->nr_fcd  = byte_swap_16( fsr->nr_fcd );
     fsr->sz_fcd  = byte_swap_32( fsr->sz_fcd );
     fsr->nr_pcd  = byte_swap_16( fsr->nr_pcd );
     fsr->sz_pcd  = byte_swap_32( fsr->sz_pcd );
     fsr->nr_scd  = byte_swap_16( fsr->nr_scd );
     fsr->sz_scd  = byte_swap_32( fsr->sz_scd );
     fsr->nr_mcd  = byte_swap_16( fsr->nr_mcd );
     fsr->sz_mcd  = byte_swap_32( fsr->sz_mcd );
     for ( nr = 0; nr < NUM_SPEC_BANDS; nr++ ) {
	  fsr->nr_band[nr] = byte_swap_16( fsr->nr_band[nr] );
	  fsr->sz_band[nr] = byte_swap_32( fsr->sz_band[nr] );
     }
}
#endif

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_LV1_RD_FSR( FILE *infl, struct fsr1_gome *fsr )
{
     const char prognm[] = "GOME_LV1_RD_FSR";

     register char *fsr_pntr;
     register short ni, nr;

     char  fsr_char[LVL1_FSR_LENGTH];
     short *fsr_short = NULL;
     int   *fsr_int   = NULL;

     const long FSR_BYTE_OFFS = LVL1_PIR_LENGTH;
/*
 * allocate memory to temporary store data for output structure
 */
     fsr_short = (short *) malloc( (LVL1_FSR_LENGTH/6) * sizeof(short) );
     if ( fsr_short == NULL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "fsr_short" );
     fsr_int   = (int *) malloc( (LVL1_FSR_LENGTH/6) * sizeof(int) );
     if ( fsr_int == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "fsr_int" );
/*
 * rewind/read input data file
 */
     (void) fseek( infl, FSR_BYTE_OFFS, SEEK_SET );
     if ( fread( fsr_char, LVL1_FSR_LENGTH, 1, infl ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );

     ni = 0;
     fsr_pntr = fsr_char;
     do { 
	  (void) memcpy( &fsr_short[ni], fsr_pntr, GOME_SHORT );
	  fsr_pntr += GOME_SHORT;
	  (void) memcpy( &fsr_int[ni], fsr_pntr, GOME_INT );
	  fsr_pntr += GOME_INT;
     } while ( ++ni < (short)(LVL1_FSR_LENGTH/6) );

     fsr->nr_sph  = fsr_short[0];
     fsr->sz_sph  = fsr_int[0];
     fsr->nr_fcd  = fsr_short[1];
     fsr->sz_fcd  = fsr_int[1];
     fsr->nr_pcd  = fsr_short[2];
     fsr->sz_pcd  = fsr_int[2];
     fsr->nr_scd  = fsr_short[3];
     fsr->sz_scd  = fsr_int[3];
     fsr->nr_mcd  = fsr_short[4];
     fsr->sz_mcd  = fsr_int[4];
     for ( nr = 0; nr < NUM_SPEC_BANDS; nr++ ) {
	  fsr->nr_band[nr] = fsr_short[6 + nr];
	  fsr->sz_band[nr] = fsr_int[6 + nr];
     }
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_FSR( fsr );
#endif
 done:
     if ( fsr_short != NULL ) free( fsr_short );
     if ( fsr_int   != NULL ) free( fsr_int );
     return;
}
