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

.IDENTifer   SCIA_OL2_RD_SQADS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 offline product
.LANGUAGE    ANSI C
.PURPOSE     read Summary of Quality Flags per State
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_OL2_RD_SQADS( fd, num_dsd, dsd, &sqads );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct sqads_sci_ol **sqads :  summary of quality flags per state

.RETURNS     number of data set records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0   29-Apr-2002 created by R. M. van Hees
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
void Sun2Intel_SQADS( struct sqads_sci_ol *sqads )
{
     sqads->mjd.days = byte_swap_32( sqads->mjd.days );
     sqads->mjd.secnd = byte_swap_u32( sqads->mjd.secnd );
     sqads->mjd.musec = byte_swap_u32( sqads->mjd.musec );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned 
int SCIA_OL2_RD_SQADS( FILE *fd, unsigned int num_dsd, 
		       const struct dsd_envi *dsd, 
		       struct sqads_sci_ol **sqads_out )
{
     const char prognm[] = "SCIA_OL2_RD_SQADS";

     char         *sqads_pntr, *sqads_char = NULL;
     size_t       dsr_size;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct sqads_sci_ol *sqads;

     const char dsd_name[] = "SUMMARY_QUALITY";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( ! Use_Extern_Alloc ) {
	  sqads_out[0] = (struct sqads_sci_ol *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct sqads_sci_ol));
     }
     if ( (sqads = sqads_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "sqads" );
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (sqads_char = (char *) malloc( dsr_size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "sqads_char" );
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
	  (void) memcpy( &sqads->flag_pqf, sqads_pntr, 
			   OL2_SQADS_PQF_FLAGS * ENVI_UCHAR );
	  sqads_pntr += OL2_SQADS_PQF_FLAGS * ENVI_UCHAR;
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
     sqads_pntr = NULL;
/*
 * set return values
 */
 done:
     if ( sqads_char != NULL ) free( sqads_char );

     return nr_dsr;
}
