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

.IDENTifer   SCIA_OL2_RD_CLD
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA offline level 2 data
.LANGUAGE    ANSI C
.PURPOSE     read Cloud and Aerosol Data sets
.INPUT/OUTPUT
  call as   nbyte = SCIA_OL2_RD_CLD( fd, num_dsd, dsd, &cld );
     input:
            FILE *fd                :   stream pointer
	    unsigned int num_dsd    :   number of DSDs
	    struct dsd_envi *dsd    :   structure for the DSDs
    output:
            struct cld_sci_ol **cld :   Cloud and Aerosol Data sets

.RETURNS     number of data set records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      1.2   21-Jan-2003 added check for presence of this dataset, RvH
              1.0   14-May-2002 created by R. M. van Hees
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
void Sun2Intel_CLD( struct cld_sci_ol *cld )
{
     register unsigned short np;

     cld->mjd.days = byte_swap_32( cld->mjd.days );
     cld->mjd.secnd = byte_swap_u32( cld->mjd.secnd );
     cld->mjd.musec = byte_swap_u32( cld->mjd.musec );
     cld->dsrlen = byte_swap_u32( cld->dsrlen );
     cld->intg_time = byte_swap_u16( cld->intg_time );

     IEEE_Swap__FLT( &cld->surfpress );
     IEEE_Swap__FLT( &cld->cloudfrac );
     IEEE_Swap__FLT( &cld->errcldfrac );
     cld->numpmdpix = byte_swap_u16( cld->numpmdpix );
     cld->fullfree[0] = byte_swap_u16( cld->fullfree[0] );
     cld->fullfree[1] = byte_swap_u16( cld->fullfree[1] );
     IEEE_Swap__FLT( &cld->toppress );
     IEEE_Swap__FLT( &cld->errtoppress );
     IEEE_Swap__FLT( &cld->cldoptdepth );
     IEEE_Swap__FLT( &cld->errcldoptdep );
     cld->cloudtype = byte_swap_u16( cld->cloudtype );
     IEEE_Swap__FLT( &cld->cloudbrdf );
     IEEE_Swap__FLT( &cld->errcldbrdf );
     IEEE_Swap__FLT( &cld->effsurfrefl );
     IEEE_Swap__FLT( &cld->erreffsrefl );
     cld->cloudflag = byte_swap_u16( cld->cloudflag );
     IEEE_Swap__FLT( &cld->aai );
     IEEE_Swap__FLT( &cld->aaidiag );
     cld->aaiflag = byte_swap_u16( cld->aaiflag );
     for ( np = 0; np < cld->numaeropars; np++ )
	  IEEE_Swap__FLT( &cld->aeropars[np] );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned 
int SCIA_OL2_RD_CLD( FILE *fd, unsigned int num_dsd,
		     const struct dsd_envi *dsd, 
		     struct cld_sci_ol **cld_out )
{
     const char prognm[]   = "SCIA_OL2_RD_CLD";

     char         *cld_pntr, *cld_char = NULL;
     size_t       dsd_size;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct cld_sci_ol *cld;

     const char dsd_name[] = "CLOUDS_AEROSOL";
/*
 * get index to data set descriptor
 */
     NADC_ERR_SAVE();
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( IS_ERR_STAT_ABSENT || dsd[indx_dsd].num_dsr == 0 ) {
          NADC_ERR_RESTORE();
          cld_out[0] = NULL;
          return 0u;
     }
     dsd_size = (size_t) dsd[indx_dsd].size;
/*
 * allocate memory to store output structures
 */
     if ( ! Use_Extern_Alloc ) {
	  cld_out[0] = (struct cld_sci_ol *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct cld_sci_ol));
     }
     if ( (cld = cld_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "cld" );
/*
 * allocate memory to temporary store data for output structure
 */
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
	  (void) memcpy( &cld->surfpress, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->cloudfrac, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->errcldfrac, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->numpmdpix, cld_pntr, ENVI_USHRT );
	  cld_pntr += ENVI_USHRT;
	  (void) memcpy( cld->fullfree, cld_pntr, 2 * ENVI_USHRT );
	  cld_pntr += 2 * ENVI_USHRT;
	  (void) memcpy( &cld->toppress, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->errtoppress, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->cldoptdepth, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->errcldoptdep, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->cloudtype, cld_pntr, ENVI_USHRT );
	  cld_pntr += ENVI_USHRT;
	  (void) memcpy( &cld->cloudbrdf, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->errcldbrdf, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->effsurfrefl, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->erreffsrefl, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->cloudflag, cld_pntr, ENVI_USHRT );
	  cld_pntr += ENVI_USHRT;
	  (void) memcpy( &cld->aai, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->aaidiag, cld_pntr, ENVI_FLOAT );
	  cld_pntr += ENVI_FLOAT;
	  (void) memcpy( &cld->aaiflag, cld_pntr, ENVI_USHRT );
	  cld_pntr += ENVI_USHRT;
	  (void) memcpy( &cld->numaeropars, cld_pntr, ENVI_USHRT );
	  cld_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  cld->numaeropars = byte_swap_u16( cld->numaeropars );
#endif
	  if ( cld->numaeropars > 0u ) {
	       cld->aeropars = (float *)
		    malloc((size_t) cld->numaeropars * sizeof( float ));
	       if ( cld->aeropars == NULL ) 
		    NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "aeropars" );
	       (void) memcpy( cld->aeropars, cld_pntr, 
			      cld->numaeropars * ENVI_FLOAT );
	       cld_pntr += cld->numaeropars * ENVI_FLOAT;
	  }
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_CLD( cld );
#endif
	  cld++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
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

