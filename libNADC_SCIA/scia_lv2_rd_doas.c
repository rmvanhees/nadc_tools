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

.IDENTifer   SCIA_LV2_RD_DOAS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 data
.LANGUAGE    ANSI C
.PURPOSE     read DOAS Fitting Window Application data sets
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV2_RD_DOAS( fd, doas_name, num_dsd, dsd, doas );
     input:
            FILE *fd              :  stream pointer
            char doas_name[]      :  PDS name for DOAS data set
	    unsigned int num_dsd  :  number of DSDs
	    struct dsd_envi *dsd  :  structure for the DSDs
    output:
            struct doas_scia **doas:  DOAS Fitting Window Application data set

.RETURNS     number of data set records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      2.1   21-Jan-2002	use of global Use_Extern_Alloc, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.1   09-Oct-2001	changed input/output, RvH 
              1.0   14-Sep-2001 created by R. M. van Hees
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
void Sun2Intel_DOAS( struct doas_scia *doas )
{
     register unsigned short nc;

     unsigned int n_cross;

     doas->mjd.days = byte_swap_32( doas->mjd.days );
     doas->mjd.secnd = byte_swap_u32( doas->mjd.secnd );
     doas->mjd.musec = byte_swap_u32( doas->mjd.musec );
     doas->dsrlen = byte_swap_u32( doas->dsrlen );
     doas->intg_time = byte_swap_u16( doas->intg_time );
     IEEE_Swap__FLT( &doas->vcd );
     IEEE_Swap__FLT( &doas->errvcd );
     IEEE_Swap__FLT( &doas->esc );
     IEEE_Swap__FLT( &doas->erresc );
     IEEE_Swap__FLT( &doas->rms );
     IEEE_Swap__FLT( &doas->chi2 );
     IEEE_Swap__FLT( &doas->goodness );
     IEEE_Swap__FLT( &doas->amfgnd );
     IEEE_Swap__FLT( &doas->amfcld );
     IEEE_Swap__FLT( &doas->reflgnd );
     IEEE_Swap__FLT( &doas->reflcld );
     IEEE_Swap__FLT( &doas->refl );

     doas->numiter = byte_swap_u16( doas->numiter );
     n_cross = (doas->numfitp * (doas->numfitp - 1)) / 2;
     for ( nc = 0; nc < n_cross; nc++ ) IEEE_Swap__FLT( &doas->corrpar[nc] );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int SCIA_LV2_RD_DOAS( FILE *fd, const char doas_name[],
			       unsigned int num_dsd, 
			       const struct dsd_envi *dsd, 
			       struct doas_scia **doas_out )
{
     const char prognm[] = "SCIA_LV2_RD_DOAS";

     char         *doas_pntr, *doas_char = NULL;
     size_t       dsd_size, nr_byte;
     unsigned int indx_dsd;
     unsigned int n_cross;

     unsigned int nr_dsr = 0;

     struct doas_scia *doas;
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, doas_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, doas_name );
     if ( dsd[indx_dsd].size == 0u ) return 0u;
     if ( ! Use_Extern_Alloc ) {
	  doas_out[0] = (struct doas_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct doas_scia));
     }
     if ( (doas = doas_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "doas" );
/*
 * allocate memory to temporary store data for output structure
 */
     dsd_size = (size_t) dsd[indx_dsd].size;
     if ( (doas_char = (char *) malloc( dsd_size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "doas_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
     if ( fread( doas_char, dsd_size, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to DOAS structure
 */
     doas_pntr = doas_char;
     do {
	  (void) memcpy( &doas->mjd.days, doas_pntr, ENVI_INT );
	  doas_pntr += ENVI_INT;
	  (void) memcpy( &doas->mjd.secnd, doas_pntr, ENVI_UINT );
	  doas_pntr += ENVI_UINT;
	  (void) memcpy( &doas->mjd.musec, doas_pntr, ENVI_UINT );
	  doas_pntr += ENVI_UINT;
	  (void) memcpy( &doas->dsrlen, doas_pntr, ENVI_UINT );
	  doas_pntr += ENVI_UINT;
	  (void) memcpy( &doas->quality, doas_pntr, ENVI_CHAR );
	  doas_pntr += ENVI_CHAR;
	  (void) memcpy( &doas->intg_time, doas_pntr, ENVI_USHRT );
	  doas_pntr += ENVI_USHRT;
	  (void) memcpy( &doas->numfitp, doas_pntr, ENVI_USHRT );
	  doas_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  doas->numfitp = byte_swap_u16( doas->numfitp );
#endif
	  (void) memcpy( &doas->vcd, doas_pntr, ENVI_FLOAT );
	  doas_pntr += ENVI_FLOAT;
	  (void) memcpy( &doas->errvcd, doas_pntr, ENVI_FLOAT );
	  doas_pntr += ENVI_FLOAT;
	  (void) memcpy( &doas->vcdflag, doas_pntr, ENVI_USHRT );
	  doas_pntr += ENVI_USHRT;
	  (void) memcpy( &doas->esc, doas_pntr, ENVI_FLOAT );
	  doas_pntr += ENVI_FLOAT;
	  (void) memcpy( &doas->erresc, doas_pntr, ENVI_FLOAT );
	  doas_pntr += ENVI_FLOAT;
	  (void) memcpy( &doas->rms, doas_pntr, ENVI_FLOAT );
	  doas_pntr += ENVI_FLOAT;
	  (void) memcpy( &doas->chi2, doas_pntr, ENVI_FLOAT );
	  doas_pntr += ENVI_FLOAT;
	  (void) memcpy( &doas->goodness, doas_pntr, ENVI_FLOAT );
	  doas_pntr += ENVI_FLOAT;
	  (void) memcpy( &doas->numiter, doas_pntr, ENVI_USHRT );
	  doas_pntr += ENVI_USHRT;
	  n_cross = (doas->numfitp * (doas->numfitp - 1u)) / 2u;
	  nr_byte = (size_t) n_cross * sizeof( float );
	  doas->corrpar = (float *) malloc( nr_byte );
	  if ( doas->corrpar == NULL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "corrpar" );
	  (void) memcpy( doas->corrpar, doas_pntr, nr_byte ); 
	  doas_pntr += nr_byte;
	  (void) memcpy( &doas->escflag, doas_pntr, ENVI_USHRT );
	  doas_pntr += ENVI_USHRT;
	  (void) memcpy( &doas->amfgnd, doas_pntr, ENVI_FLOAT );
	  doas_pntr += ENVI_FLOAT;
	  (void) memcpy( &doas->amfcld, doas_pntr, ENVI_FLOAT );
	  doas_pntr += ENVI_FLOAT;
	  (void) memcpy( &doas->reflgnd, doas_pntr, ENVI_FLOAT );
	  doas_pntr += ENVI_FLOAT;
	  (void) memcpy( &doas->reflcld, doas_pntr, ENVI_FLOAT );
	  doas_pntr += ENVI_FLOAT;
	  (void) memcpy( &doas->refl, doas_pntr, ENVI_FLOAT );
	  doas_pntr += ENVI_FLOAT;
	  (void) memcpy( &doas->amfflag, doas_pntr, ENVI_USHRT );
	  doas_pntr += ENVI_USHRT;

#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_DOAS( doas );
#endif
	  doas++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * check if we read the whole DSR
 */
     if ( (unsigned int) (doas_pntr - doas_char) != dsd[indx_dsd].size )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, doas_name );
     doas_pntr = NULL;
/*
 * set return values
 */
 done:
     if ( doas_char != NULL ) free( doas_char );

     return nr_dsr;
}

