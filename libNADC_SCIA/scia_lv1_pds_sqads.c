/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2000 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_PDS_SQADS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Summary of Quality flags per state records
.COMMENTS    contains SCIA_LV1_RD_SQADS and SCIA_LV1_WR_SQADS
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   18-Apr-2005 added routine to write SQADS-struct to file,RvH
              2.1   16-Jan-2002	use of global Use_Extern_Alloc, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.1   04-Oct-2001	changed input/output, RvH 
              1.0   04-Nov-1999 created by R. M. van Hees
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
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

#define NumSpareFlags    10

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_SQADS( struct sqads1_scia *sqads )
{
     register int nr;

     sqads->mjd.days = byte_swap_32( sqads->mjd.days );
     sqads->mjd.secnd = byte_swap_u32( sqads->mjd.secnd );
     sqads->mjd.musec = byte_swap_u32( sqads->mjd.musec );
     for ( nr = 0; nr < SCIENCE_CHANNELS; nr++ ) {
	  IEEE_Swap__FLT( &sqads->mean_wv_diff[nr] );
	  IEEE_Swap__FLT( &sqads->sdev_wv_diff[nr] );
     }
     for ( nr = 0; nr < ALL_CHANNELS; nr++ ) {
	  sqads->hotpixel[nr] = byte_swap_u16( sqads->hotpixel[nr] );
	  IEEE_Swap__FLT( &sqads->mean_diff_leak[nr] );
     }
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_SQADS
.PURPOSE     read Summary of Quality flags per state records
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_SQADS( fd, num_dsd, dsd, &sqads );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi dsd   :   structure for the DSDs
    output:
            struct sqads1_scia **sqads :  summary of quality flags per state

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_SQADS( FILE *fd, unsigned int num_dsd, 
				const struct dsd_envi *dsd,
				struct sqads1_scia **sqads_out )
{
     const char prognm[] = "SCIA_LV1_RD_SQADS";

     char         *sqads_pntr, *sqads_char = NULL;
     size_t       dsr_size;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct sqads1_scia *sqads;

     const char dsd_name[] = "SUMMARY_QUALITY";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( dsd[indx_dsd].num_dsr == 0 ) {
          sqads_out[0] = NULL;
          return 0u;
     }
     if ( ! Use_Extern_Alloc ) {
	  sqads_out[0] = (struct sqads1_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct sqads1_scia));
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
	  sqads_pntr = sqads_char;
/*
 * read data buffer to SQADS structure
 */
	  (void) memcpy( &sqads->mjd.days, sqads_pntr, ENVI_INT );
	  sqads_pntr += ENVI_INT;
	  (void) memcpy( &sqads->mjd.secnd, sqads_pntr, ENVI_UINT );
	  sqads_pntr += ENVI_UINT;
	  (void) memcpy( &sqads->mjd.musec, sqads_pntr, ENVI_UINT );
	  sqads_pntr += ENVI_UINT;
	  (void) memcpy( &sqads->flag_mds, sqads_pntr, ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( sqads->mean_wv_diff, sqads_pntr, 
			   SCIENCE_CHANNELS * ENVI_FLOAT );
	  sqads_pntr += SCIENCE_CHANNELS * ENVI_FLOAT;
	  (void) memcpy( sqads->sdev_wv_diff, sqads_pntr, 
			   SCIENCE_CHANNELS * ENVI_FLOAT );
	  sqads_pntr += SCIENCE_CHANNELS * ENVI_FLOAT;
	  (void) memcpy( &sqads->missing_readouts, sqads_pntr, 
			   ENVI_USHRT );
	  sqads_pntr += ENVI_USHRT;
	  (void) memcpy( sqads->mean_diff_leak, sqads_pntr, 
			   ALL_CHANNELS * ENVI_FLOAT );
	  sqads_pntr += ALL_CHANNELS * ENVI_FLOAT;
	  (void) memcpy( &sqads->flag_glint, sqads_pntr, 
			   ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->flag_rainbow, sqads_pntr, 
			   ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( &sqads->flag_saa_region, sqads_pntr, 
			   ENVI_UCHAR );
	  sqads_pntr += ENVI_UCHAR;
	  (void) memcpy( sqads->hotpixel, sqads_pntr, 
			   (ALL_CHANNELS * ENVI_USHRT) );
	  sqads_pntr += (ALL_CHANNELS * ENVI_USHRT);
/*
 * spare for additional flags
 */
	  sqads_pntr += NumSpareFlags;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(sqads_pntr - sqads_char) != dsr_size )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_SQADS( sqads );
#endif
	  sqads++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * set return values
 */
     sqads_pntr = NULL;
 done:
     if ( sqads_char != NULL ) free( sqads_char );
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_SQADS
.PURPOSE     write Summary of Quality flags per state records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_SQADS( fd, num_sqads, sqads );
     input:
            FILE *fd                  :  stream pointer
	    unsigned int num_sqads    :  number of SQADS records
            struct sqads1_scia *sqads :  summary of quality flags per state

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_SQADS( FILE *fd, unsigned int num_sqads, 
			const struct sqads1_scia *sqads_in )
{
     const char prognm[] = "SCIA_LV1_WR_SQADS";

     size_t nr_byte;

     struct sqads1_scia sqads;

     const char StringSpareFlags[NumSpareFlags+1] = "          ";

     struct dsd_envi dsd = {
	  "SUMMARY_QUALITY", "A",
	  "                                                              ",
	  0u, 0u, 0u, 0
     };

     if ( num_sqads == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     do {
	  (void) memcpy( &sqads, sqads_in, sizeof( struct sqads1_scia ));
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_SQADS( &sqads );
#endif
/*
 * write SQADS structure
 */
	  if ( fwrite( &sqads.mjd.days, ENVI_INT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_INT;
	  if ( fwrite( &sqads.mjd.secnd, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &sqads.mjd.musec, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &sqads.flag_mds, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;
	  nr_byte = (size_t) ENVI_FLOAT * SCIENCE_CHANNELS;
	  if ( fwrite( sqads.mean_wv_diff, nr_byte, 1, fd) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	       if ( fwrite( sqads.sdev_wv_diff, nr_byte, 1, fd) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( &sqads.missing_readouts, ENVI_USHRT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_USHRT;
	  nr_byte = (size_t) ENVI_FLOAT * ALL_CHANNELS;
	  if ( fwrite( sqads.mean_diff_leak, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( &sqads.flag_glint, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;
	  if ( fwrite( &sqads.flag_rainbow, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;
	  if ( fwrite( &sqads.flag_saa_region, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;
	  nr_byte = (size_t) ENVI_USHRT * ALL_CHANNELS;
	  if ( fwrite( sqads.hotpixel, nr_byte,1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( StringSpareFlags, NumSpareFlags, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += NumSpareFlags;

	  sqads_in++;
     } while ( ++dsd.num_dsr < num_sqads );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}
