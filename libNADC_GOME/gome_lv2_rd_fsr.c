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

.IDENTifer   GOME_LV2_RD_FSR
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 2 data
.LANGUAGE    ANSI C
.PURPOSE     Read the File Structure Record
.INPUT/OUTPUT
  call as   GOME_LV2_RD_FSR( infl, &fsr );

     input:  
            FILE   *infl          : (open) file descriptor 
    output:  
            struct fsr2_gome *fsr : structure for the FSR record

..RETURNS    nothing: modifies error status
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   11-Nov-2001	moved to the new Error handling routines, RvH
              1.0   18-Mar-1999	Created by R. M. van Hees 
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
void Sun2Intel_FSR( struct fsr2_gome *fsr )
{
     fsr->nr_sph  = byte_swap_16( fsr->nr_sph );
     fsr->sz_sph  = byte_swap_32( fsr->sz_sph );
     fsr->nr_ddr  = byte_swap_16( fsr->nr_ddr );
     fsr->sz_ddr  = byte_swap_32( fsr->sz_ddr );
}
#endif

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_LV2_RD_FSR( FILE *infl, struct fsr2_gome *fsr )
{
     register char *fsr_pntr;

     char  *fsr_char;

     const long FSR_BYTE_OFFS = LVL1_PIR_LENGTH;
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (fsr_char = (char *) malloc( LVL2_FSR_LENGTH )) == NULL ) 
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "fsr_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( infl, FSR_BYTE_OFFS, SEEK_SET );
     if ( fread( fsr_char, LVL2_FSR_LENGTH, 1, infl ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );

     (void) memcpy( &fsr->nr_sph, fsr_char, GOME_SHORT );
     fsr_pntr = fsr_char + GOME_SHORT;
     (void) memcpy( &fsr->sz_sph, fsr_pntr, GOME_INT );
     fsr_pntr += GOME_INT;
     (void) memcpy( &fsr->nr_ddr, fsr_pntr, GOME_SHORT );
     fsr_pntr += GOME_SHORT;
     (void) memcpy( &fsr->sz_ddr, fsr_pntr, GOME_INT );
     fsr_pntr += GOME_INT;
/*
 * check size of FSR read
 */
     if ( fsr_pntr - fsr_char != LVL2_FSR_LENGTH )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, "FSR size" );

#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_FSR( fsr );
#endif
 done:
     free( fsr_char );
     return;
}
