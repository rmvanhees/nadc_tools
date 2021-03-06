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

.IDENTifer   MERIS_RR2_RD_MDS_15
.AUTHOR      R.M. van Hees
.KEYWORDS    MERIS level 2 product (RR)
.LANGUAGE    ANSI C
.PURPOSE     read measurement data set (15)
.INPUT/OUTPUT
  call as   nr_tie = MERIS_RR2_RD_MDS_15( fd, num_dsd, dsd, &mds_15 );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct mds_rr2_15_meris **mds_15 :  MDS type 15

.RETURNS     number of data set records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0   10-Oct-2008 created by R. M. van Hees
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
void Sun2Intel_MDS_15( struct mds_rr2_15_meris *mds_15 )
{
     mds_15->mjd.days = byte_swap_32( mds_15->mjd.days );
     mds_15->mjd.secnd = byte_swap_u32( mds_15->mjd.secnd );
     mds_15->mjd.musec = byte_swap_u32( mds_15->mjd.musec );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int MERIS_RR2_RD_MDS_15( FILE *fd, unsigned int num_dsd, 
				  const struct dsd_envi *dsd, 
				  struct mds_rr2_15_meris **mds_15_out )
{
     char         *mds_15_char, *mds_15_pntr;
     size_t       dsr_size;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct mds_rr2_15_meris *mds_15;

     const size_t nr_byte = sizeof(char) * 1121;
     const char   dsd_name[] = "Chl_1, TOAVI   - MDS(15)";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_ABSENT || IS_ERR_STAT_FATAL ) {
          NADC_ERROR( NADC_ERR_PDS_RD, dsd_name );
	  return 0;
     }
     if ( dsd[indx_dsd].num_dsr == 0 ) return 0;
     if ( ! Use_Extern_Alloc ) {
	  mds_15_out[0] = (struct mds_rr2_15_meris *) 
	       malloc(dsd[indx_dsd].num_dsr * sizeof(struct mds_rr2_15_meris));
     }
     if ( (mds_15 = mds_15_out[0]) == NULL ) {
	  NADC_ERROR( NADC_ERR_ALLOC, "mds_15" );
	  return 0;
     }
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (mds_15_char = (char *) malloc( dsr_size )) == NULL ) {
	  NADC_ERROR( NADC_ERR_ALLOC, "mds_15_char" );
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
	  if ( fread( mds_15_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to MDS_15 structure
 */
	  (void) memcpy( &mds_15->mjd.days, mds_15_char, ENVI_INT );
	  mds_15_pntr = mds_15_char + ENVI_INT;
	  (void) memcpy( &mds_15->mjd.secnd, mds_15_pntr, ENVI_UINT );
	  mds_15_pntr += ENVI_UINT;
	  (void) memcpy( &mds_15->mjd.musec, mds_15_pntr, ENVI_UINT );
	  mds_15_pntr += ENVI_UINT;
	  (void) memcpy( &mds_15->quality_flag, mds_15_pntr, ENVI_UCHAR );
	  mds_15_pntr += ENVI_UCHAR;
	  (void) memcpy( &mds_15->algal_toavi_cld, mds_15_pntr, nr_byte );
	  mds_15_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(mds_15_pntr - mds_15_char) != dsr_size )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, dsd_name );

#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_MDS_15( mds_15 );
#endif
	  ++mds_15;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * deallocate memory
 */
 done:
     mds_15_pntr = NULL;
     free( mds_15_char );
/*
 * set return values
 */
     return nr_dsr;
}
