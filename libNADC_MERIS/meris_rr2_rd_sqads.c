/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2008 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   MERIS_RR2_RD_SQADS
.AUTHOR      R.M. van Hees
.KEYWORDS    MERIS level 2 product
.LANGUAGE    ANSI C
.PURPOSE     read Summary of Quality Flags per tie frame
.INPUT/OUTPUT
  call as   nr_tie = MERIS_RR2_RD_SQADS( fd, num_dsd, dsd, &sqads );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct sqads2_meris **sqads :  summary of quality flags per tie

.RETURNS     number of data set records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0   22-Sep-2008 created by R. M. van Hees
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
#define _MERIS_COMMON
#include <nadc_meris.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_SQADS( struct sqads2_meris *sqads )
{
     sqads->mjd.days = byte_swap_32( sqads->mjd.days );
     sqads->mjd.secnd = byte_swap_u32( sqads->mjd.secnd );
     sqads->mjd.musec = byte_swap_u32( sqads->mjd.musec );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int MERIS_RR2_RD_SQADS( FILE *fd, unsigned int num_dsd, 
				 const struct dsd_envi *dsd, 
				 struct sqads2_meris **sqads_out )
{
     const char prognm[] = "MERIS_RR2_RD_SQADS";

     char         *sqads_char, *sqads_pntr;
     size_t       dsr_size;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct sqads2_meris *sqads;

     const char dsd_name[] = "Quality ADS";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_ABSENT || IS_ERR_STAT_FATAL ) {
          NADC_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
	  return 0;
     }
     if ( dsd[indx_dsd].num_dsr == 0 ) return 0;
     if ( ! Use_Extern_Alloc ) {
	  sqads_out[0] = (struct sqads2_meris *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct sqads2_meris));
     }
     if ( (sqads = sqads_out[0]) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_ALLOC, "sqads" );
	  return 0;
     }
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (sqads_char = (char *) malloc( dsr_size )) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_ALLOC, "sqads_char" );
	  return 0;
     }
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( sqads_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to SQADS structure
 */
	  (void) memcpy( &sqads->mjd.days, sqads_char, ENVI_INT );
	  sqads_pntr = sqads_char + ENVI_INT;
	  (void) memcpy( &sqads->mjd.secnd, sqads_pntr, ENVI_UINT );
	  sqads_pntr += ENVI_UINT;
	  (void) memcpy( &sqads->mjd.musec, sqads_pntr, ENVI_UINT );
	  sqads_pntr += ENVI_UINT;
	  (void) memcpy( &sqads->flag_mds, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->ocean_aerosols, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->ocean_climatology, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->ddv_land, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->turbit_climatology, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->consolidated, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  sqads_pntr += ENVI_UCHAR; /* spare_1 */
	  sqads_pntr += ENVI_UCHAR; /* spare_1 */
	  (void) memcpy( &sqads->failed_in_vapour, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->failed_out_vapour, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->failed_in_cloud, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->failed_out_cloud, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->failed_in_land, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->failed_out_land, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->failed_in_ocean, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->failed_out_ocean, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->failed_in_case1, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->failed_out_case1, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->failed_in_case2, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->failed_out_case2, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(sqads_pntr - sqads_char) != dsr_size )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );

#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_SQADS( sqads );
#endif
	  sqads++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * deallocate memory
 */
 done:
     sqads_pntr = NULL;
     free( sqads_char );
/*
 * set return values
 */
     return nr_dsr;
}
