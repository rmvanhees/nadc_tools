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

.IDENTifer   SCIA_LV2_RD_BIAS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 data
.LANGUAGE    ANSI C
.PURPOSE     read BIAS Fitting Window Application Data set
.INPUT/OUTPUT
  call as   nbyte = SCIA_LV2_RD_BIAS( fd, bias_name, num_dsd, dsd, bias );
     input:
            FILE *fd              :  stream pointer
	    char bias_name[]      :  PDS name for BIAS data set
	    unsigned int num_dsd  :  number of DSDs
	    struct dsd_envi *dsd  :  structure for the DSDs
    output:
            struct bias_scia **bias : BIAS Fitting Window Application Data set

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
void Sun2Intel_BIAS( struct bias_scia *bias )
{
     register unsigned short nc;

     unsigned int n_cross;

     bias->mjd.days = byte_swap_32( bias->mjd.days );
     bias->mjd.secnd = byte_swap_u32( bias->mjd.secnd );
     bias->mjd.musec = byte_swap_u32( bias->mjd.musec );
     bias->dsrlen = byte_swap_u32( bias->dsrlen );
     bias->intg_time = byte_swap_u16( bias->intg_time );
     bias->numsegm = byte_swap_u16( bias->numsegm );
     IEEE_Swap__FLT( &bias->hght );
     IEEE_Swap__FLT( &bias->errhght );
     IEEE_Swap__FLT( &bias->vcd );
     IEEE_Swap__FLT( &bias->errvcd );
     IEEE_Swap__FLT( &bias->closure );
     IEEE_Swap__FLT( &bias->errclosure );
     IEEE_Swap__FLT( &bias->rms );
     IEEE_Swap__FLT( &bias->chi2 );
     IEEE_Swap__FLT( &bias->goodness );
     IEEE_Swap__FLT( &bias->cutoff );

     bias->numiter = byte_swap_u16( bias->numiter );
     n_cross = (bias->numfitp * (bias->numfitp - 1)) / 2;
     for ( nc = 0; nc < n_cross; nc++ ) IEEE_Swap__FLT( &bias->corrpar[nc] );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned 
int SCIA_LV2_RD_BIAS( FILE *fd, const char bias_name[],
		      unsigned int num_dsd, const struct dsd_envi *dsd, 
		      struct bias_scia **bias_out )
{
     char         *bias_pntr, *bias_char = NULL;
     size_t       dsd_size, nr_byte;
     unsigned int indx_dsd;
     unsigned int n_cross;

     unsigned int nr_dsr = 0;
     struct bias_scia *bias;
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, bias_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, bias_name );
     if ( dsd[indx_dsd].size == 0u ) return 0;
     if ( ! Use_Extern_Alloc ) {
	  bias_out[0] = (struct bias_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct bias_scia));
     }
     if ( (bias = bias_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "bias" );
/*
 * allocate memory to temporary store data for output structure
 */
     dsd_size = (size_t) dsd[indx_dsd].size;
     if ( (bias_char = (char *) malloc( dsd_size )) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "bias_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
     if ( fread( bias_char, dsd_size, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to BIAS structure
 */
     bias_pntr = bias_char;
     do {
	  (void) memcpy( &bias->mjd.days, bias_pntr, ENVI_INT );
	  bias_pntr += ENVI_INT;
	  (void) memcpy( &bias->mjd.secnd, bias_pntr, ENVI_UINT );
	  bias_pntr += ENVI_UINT;
	  (void) memcpy( &bias->mjd.musec, bias_pntr, ENVI_UINT );
	  bias_pntr += ENVI_UINT;
	  (void) memcpy( &bias->dsrlen, bias_pntr, ENVI_UINT );
	  bias_pntr += ENVI_UINT;
	  (void) memcpy( &bias->quality, bias_pntr, ENVI_CHAR );
	  bias_pntr += ENVI_CHAR;
	  (void) memcpy( &bias->intg_time, bias_pntr, ENVI_USHRT );
	  bias_pntr += ENVI_USHRT;
	  (void) memcpy( &bias->numfitp, bias_pntr, ENVI_USHRT );
	  bias_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  bias->numfitp = byte_swap_u16( bias->numfitp );
#endif
	  (void) memcpy( &bias->numsegm, bias_pntr, ENVI_USHRT );
	  bias_pntr += ENVI_USHRT;
	  (void) memcpy( &bias->hght, bias_pntr, ENVI_FLOAT );
	  bias_pntr += ENVI_FLOAT;
	  (void) memcpy( &bias->errhght, bias_pntr, ENVI_FLOAT );
	  bias_pntr += ENVI_FLOAT;
	  (void) memcpy( &bias->hghtflag, bias_pntr, ENVI_USHRT );
	  bias_pntr += ENVI_USHRT;
	  (void) memcpy( &bias->vcd, bias_pntr, ENVI_FLOAT );
	  bias_pntr += ENVI_FLOAT;
	  (void) memcpy( &bias->errvcd, bias_pntr, ENVI_FLOAT );
	  bias_pntr += ENVI_FLOAT;
	  (void) memcpy( &bias->closure, bias_pntr, ENVI_FLOAT );
	  bias_pntr += ENVI_FLOAT;
	  (void) memcpy( &bias->errclosure, bias_pntr, ENVI_FLOAT );
	  bias_pntr += ENVI_FLOAT;
	  (void) memcpy( &bias->rms, bias_pntr, ENVI_FLOAT );
	  bias_pntr += ENVI_FLOAT;
	  (void) memcpy( &bias->chi2, bias_pntr, ENVI_FLOAT );
	  bias_pntr += ENVI_FLOAT;
	  (void) memcpy( &bias->goodness, bias_pntr, ENVI_FLOAT );
	  bias_pntr += ENVI_FLOAT;
	  (void) memcpy( &bias->cutoff, bias_pntr, ENVI_FLOAT );
	  bias_pntr += ENVI_FLOAT;
	  (void) memcpy( &bias->numiter, bias_pntr, ENVI_USHRT );
	  bias_pntr += ENVI_USHRT;
	  n_cross = (bias->numfitp * (bias->numfitp - 1u)) / 2u;
	  nr_byte = (size_t) n_cross * sizeof( float );
	  bias->corrpar = (float *) malloc( nr_byte );
	  if ( bias->corrpar == NULL ) 
	       NADC_GOTO_ERROR( NADC_ERR_ALLOC, "corrpar" );
	  (void) memcpy( bias->corrpar, bias_pntr, nr_byte ); 
	  bias_pntr += nr_byte;
	  (void) memcpy( &bias->vcdflag, bias_pntr, ENVI_USHRT );
	  bias_pntr += ENVI_USHRT;

#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_BIAS( bias );
#endif
	  bias++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * check if we read the whole DSR
 */
     if ( (unsigned int)(bias_pntr - bias_char) != dsd[indx_dsd].size )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, bias_name );
     bias_pntr = NULL;
/*
 * set return values
 */
 done:
     if ( bias_char != NULL ) free( bias_char );

     return nr_dsr;
}
