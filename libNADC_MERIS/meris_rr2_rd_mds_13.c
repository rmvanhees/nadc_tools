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

.IDENTifer   MERIS_RR2_RD_MDS_13
.AUTHOR      R.M. van Hees
.KEYWORDS    MERIS level 2 product (RR)
.LANGUAGE    ANSI C
.PURPOSE     read measurement data set (1-13)
.INPUT/OUTPUT
  call as   nr_tie = MERIS_RR2_RD_MDS_13( fd, num_dsd, dsd, &mds_13 );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct mds_rr2_13_meris **mds_13 :  MDS type 1-13

.RETURNS     number of data set records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0   10-Oct-2008 created by R. M. van Hees
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
#define _MERIS_COMMON
#include <nadc_meris.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_MDS_13( struct mds_rr2_13_meris *mds_13 )
{
     register unsigned short ni = 0;

     mds_13->mjd.days = byte_swap_32( mds_13->mjd.days );
     mds_13->mjd.secnd = byte_swap_u32( mds_13->mjd.secnd );
     mds_13->mjd.musec = byte_swap_u32( mds_13->mjd.musec );
     do {
	  mds_13->norm_surf_refl[ni] = 
	       byte_swap_u32( mds_13->norm_surf_refl[ni] );
     } while ( ++ni < 1121 );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int MERIS_RR2_RD_MDS_13( int mds_number, FILE *fd, 
				  unsigned int num_dsd, 
				  const struct dsd_envi *dsd, 
				  struct mds_rr2_13_meris **mds_13_out )
{
     const char prognm[] = "MERIS_RR2_RD_MDS_13";

     char         dsd_name[25];
     char         *mds_13_char, *mds_13_pntr;
     size_t       dsr_size;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct mds_rr2_13_meris *mds_13;

     const size_t nr_byte = sizeof(short) * 1121;
/*
 * get index to data set descriptor
 */
     (void) snprintf( dsd_name, 25, "Norm. rho_surf - MDS(%-d)", mds_number );
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_ABSENT || IS_ERR_STAT_FATAL ) {
          NADC_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
	  return 0;
     }
     if ( dsd[indx_dsd].num_dsr == 0 ) return 0;
     if ( ! Use_Extern_Alloc ) {
	  mds_13_out[0] = (struct mds_rr2_13_meris *) 
	       malloc(dsd[indx_dsd].num_dsr * sizeof(struct mds_rr2_13_meris));
     }
     if ( (mds_13 = mds_13_out[0]) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_ALLOC, "mds_13" );
	  return 0;
     }
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (mds_13_char = (char *) malloc( dsr_size )) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_ALLOC, "mds_13_char" );
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
	  if ( fread( mds_13_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to MDS_13 structure
 */
	  (void) memcpy( &mds_13->mjd.days, mds_13_char, ENVI_INT );
	  mds_13_pntr = mds_13_char + ENVI_INT;
	  (void) memcpy( &mds_13->mjd.secnd, mds_13_pntr, ENVI_UINT );
	  mds_13_pntr += ENVI_UINT;
	  (void) memcpy( &mds_13->mjd.musec, mds_13_pntr, ENVI_UINT );
	  mds_13_pntr += ENVI_UINT;
	  (void) memcpy( &mds_13->quality_flag, mds_13_pntr, ENVI_UCHAR );
	  mds_13_pntr += ENVI_UCHAR;
	  (void) memcpy( &mds_13->norm_surf_refl, mds_13_pntr, nr_byte );
	  mds_13_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(mds_13_pntr - mds_13_char) != dsr_size )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );

#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_MDS_13( mds_13 );
#endif
	  ++mds_13;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * deallocate memory
 */
 done:
     mds_13_pntr = NULL;
     free( mds_13_char );
/*
 * set return values
 */
     return nr_dsr;
}
