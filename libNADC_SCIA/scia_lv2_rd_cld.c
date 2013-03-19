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

.IDENTifer   SCIA_LV2_RD_CLD
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 data
.LANGUAGE    ANSI C
.PURPOSE     read Cloud and Aerosol Data sets
.INPUT/OUTPUT
  call as   nbyte = SCIA_LV2_RD_CLD( fd, num_dsd, dsd, &cld );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct cld_scia **cld :   Cloud and Aerosol Data sets

.RETURNS     number of data set records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      2.2   05-Nov-2002	implemented a work-around for file with 
                        a corrupted CLOUDS_AEROSOL MDS, RvH
              2.1   21-Jan-2002	use of global Use_Extern_Alloc, RvH
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
void Sun2Intel_CLD( struct cld_scia *cld )
{
     register unsigned short np;

     cld->mjd.days = byte_swap_32( cld->mjd.days );
     cld->mjd.secnd = byte_swap_u32( cld->mjd.secnd );
     cld->mjd.musec = byte_swap_u32( cld->mjd.musec );
     cld->dsrlen = byte_swap_u32( cld->dsrlen );
     cld->intg_time = byte_swap_u16( cld->intg_time );
     for ( np = 0; np < cld->numpmd; np++ )
	  IEEE_Swap__FLT( &cld->pmdcloudfrac[np] );
     IEEE_Swap__FLT( &cld->cloudfrac );
     IEEE_Swap__FLT( &cld->toppress );
     IEEE_Swap__FLT( &cld->aai );
     IEEE_Swap__FLT( &cld->albedo );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned 
int SCIA_LV2_RD_CLD( FILE *fd, unsigned int num_dsd,
		     const struct dsd_envi *dsd, 
		     struct cld_scia **cld_out )
{
     const char prognm[]   = "SCIA_LV2_RD_CLD";

     char         *cld_pntr, *cld_char = NULL;
     size_t       dsd_size;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct cld_scia *cld;

     const char dsd_name[] = "CLOUDS_AEROSOL";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( ! Use_Extern_Alloc ) {
	  cld_out[0] = (struct cld_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct cld_scia));
     }
     if ( (cld = cld_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "cld" );
/*
 * allocate memory to temporary store data for output structure
 */
     dsd_size = (size_t) dsd[indx_dsd].size;
     if ( (cld_char = (char *) malloc( dsd_size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "cld_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
     if ( fread( cld_char, dsd_size, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to CLD structure
 */
     cld_pntr = cld_char;
     do { 
	  (void) memcpy( &cld->mjd.days, cld_pntr, ENVI_INT );
	  cld_pntr += ENVI_INT;
	  (void) memcpy( &cld->mjd.secnd, cld_pntr, ENVI_UINT );
	  cld_pntr += ENVI_UINT;
	  (void) memcpy( &cld->mjd.musec, cld_pntr, ENVI_UINT );
	  cld_pntr += ENVI_UINT;
	  (void) memcpy( &cld->dsrlen, cld_pntr, ENVI_UINT );
	  cld_pntr += ENVI_UINT;
	  (void) memcpy( &cld->quality, cld_pntr, ENVI_CHAR );
	  cld_pntr += ENVI_CHAR;
	  (void) memcpy( &cld->intg_time, cld_pntr, ENVI_USHRT );
	  cld_pntr += ENVI_USHRT;
	  (void) memcpy( &cld->numpmd, cld_pntr, ENVI_USHRT );
	  cld_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  cld->numpmd = byte_swap_u16( cld->numpmd );
#endif
	  cld->pmdcloudfrac = 
	       (float *) malloc((size_t) cld->numpmd * sizeof( float ));
	  if ( cld->pmdcloudfrac == NULL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "pmdcloudfrac" );
	  (void) memcpy( cld->pmdcloudfrac, cld_pntr, 
			   cld->numpmd * ENVI_FLOAT );
	  cld_pntr += cld->numpmd * ENVI_FLOAT;
	  (void) memcpy( &cld->cloudfrac, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->quality_cld, cld_pntr, ENVI_UCHAR );
	  cld_pntr += ENVI_UCHAR;
	  (void) memcpy( &cld->toppress, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->aai, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->albedo, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->outputflag, cld_pntr, ENVI_USHRT );
	  cld_pntr += ENVI_USHRT;

#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_CLD( cld );
#endif
	  cld++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr 
	       && (unsigned int)(cld_pntr - cld_char) < dsd[indx_dsd].size );
/*
 * check if we read the whole DSR
 */
     if ( (unsigned int)(cld_pntr - cld_char) != dsd[indx_dsd].size )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
     cld_pntr = NULL;
/*
 * set return values
 */
 done:
     if ( cld_char != NULL ) free( cld_char );

     return nr_dsr;
}

