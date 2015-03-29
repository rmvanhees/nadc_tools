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

.IDENTifer   GOME_LV2_RD_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 2 data
.LANGUAGE    ANSI C
.PURPOSE     Read the Specific Product Header
.INPUT/OUTPUT
  call as   nr_byte = GOME_LV2_RD_SPH( fid, fsr, &sph );

     input:  
            int    *infl          : (open) file descriptor 
	    struct fsr2_gome *fsr : structure for the FSR record
    output:  
            struct sph2_gome *sph : structure for the SPH

.RETURNS     number of bytes read (unsigned int)
             modifies global error status
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   11-Nov-2001	moved to the new Error handling routines, RvH
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

#define LVL2_SPH_NWIN  55

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_SPH( struct sph2_gome *sph )
{
     register int ni;

     for ( ni = 0; ni < sph->nwin; ni++ ) IEEE_Swap__FLT( &sph->window[ni] );
     IEEE_Swap__FLT( &sph->height );
}
#endif

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_LV2_RD_SPH( FILE *infl, const struct fsr2_gome *fsr,
		      struct sph2_gome *sph )
{
     register int nr;

     char   *sph_char; 
     char   *sph_pntr;

     const long SPH_BYTE_OFFS = LVL1_PIR_LENGTH + LVL2_FSR_LENGTH;
/*
 * allocate memory to store data from SPH-record
 */
     if ( (sph_char = (char *) malloc( fsr->sz_sph )) == NULL ) 
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "sph_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( infl, SPH_BYTE_OFFS, SEEK_SET );
     if ( fread( sph_char, (size_t) fsr->sz_sph, 1, infl ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to SPH structure
 */
     sph_pntr = sph_char;
     (void) memcpy( sph->inref, sph_pntr, LVL1_PIR_LENGTH  );
     sph->inref[LVL1_PIR_LENGTH ] = '\0';
     sph_pntr += LVL1_PIR_LENGTH ;
     (void) memcpy( sph->soft_version, sph_pntr, 5 );
     sph->soft_version[5] = '\0';
     sph_pntr += 5;
     (void) memcpy( sph->param_version, sph_pntr, 5 );
     sph->param_version[5] = '\0';
     sph_pntr += 5;
     (void) memcpy( sph->format_version, sph_pntr, 5 );
     sph->format_version[5] = '\0';
     sph_pntr += 5;
     (void) memcpy( &sph->nwin, sph_pntr, GOME_SHORT );
     sph_pntr += GOME_SHORT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     sph->nwin = byte_swap_16( sph->nwin );
#endif
/*
 * now we have to read the variable portions
 */
     if ( sph->nwin > LVL2_MAX_NWIN )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, "array window too small");
     if ( sph->nwin > 0 ) {
	  (void) memcpy( sph->window, sph_pntr, 
			   (size_t) (2 * sph->nwin * GOME_FLOAT) );
	  sph_pntr += 2 * sph->nwin * GOME_FLOAT;
     }
     (void) memcpy( &sph->nmol, sph_pntr, GOME_SHORT );
     sph_pntr += GOME_SHORT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     sph->nmol = byte_swap_16( sph->nmol );
#endif
     if ( sph->nmol > LVL2_MAX_NMOL )
	  NADC_RETURN_ERROR( NADC_ERR_FATAL, "array nmol too small");
     if ( sph->nmol > 0 ) {
	  char buff[2], mol_string[6];

	  nr = 0;
	  do {
	       (void) memcpy( mol_string, sph_pntr, 6 );
	       sph_pntr += 6;
               (void) nadc_strlcpy( buff, mol_string, 2 );
               sph->mol_win[nr] = (short) atoi( buff );
               (void) nadc_strlcpy( sph->mol_name[nr], mol_string+1, 6 );
	  } while ( ++nr < sph->nmol );
     }
     (void) memcpy( &sph->height, sph_pntr, GOME_FLOAT );
     sph_pntr += GOME_FLOAT;
/*
 * check if we read the whole DSR
 */
     if ( sph_pntr - sph_char != fsr->sz_sph )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, "SPH size" );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_SPH( sph );
#endif
 done:
     free( sph_char );
}
