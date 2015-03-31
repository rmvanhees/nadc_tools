/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2009 - 2015 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GET_SCIA_LV0_MDS_INFO
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0
.LANGUAGE    ANSI C
.PURPOSE     query the contents a SCIAMACHY level 0 product
.INPUT/OUTPUT
  call as   nr_info = GET_SCIA_LV0_MDS_INFO( fd, mph, dsd, &info );
     input:  
            FILE   *fd             : (open) stream pointer
	    struct mph_envi mph    : Envisat main product header
            struct dsd_envi *dsd   : structure for the DSD records
    output:  
            struct mds0_info *info : structure holding info about MDS records

.RETURNS     number of MDS's read (unsigned int), 
             error status passed by global variable ``nadc_stat'
.COMMENTS    None
.ENVIRONment None
.EXTERNALs   ENVI_GET_DSD_INDEX 
.VERSION     4.0     23-Mar-2015   new info-record implementation, RvH
             3.1     18-Nov-2013   added corrupted DSR detection, RvH
             3.0     28-Oct-2013   re-write, no cluster-info, RvH
             2.0     13-Nov-2012   added cluster checking, RvH
             1.1     16-Feb-2010   added stricter validity checking, RvH
             1.0     28-Oct-2009   initial release by R. M. van Hees
                                   rewrite of GET_LV0_MDS_INFO
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
#define _SCIA_LEVEL_0
#include <nadc_scia.h>
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>
#endif

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int GET_SCIA_LV0_MDS_INFO( FILE *fd, const struct dsd_envi *dsd, 
				    struct mds0_info *info )
{
     register unsigned int num_info = 0;

     register char *cpntr;

     register unsigned short ii, jj;

     register struct mds0_info *info_pntr = info;

     char           *mds_char;
     size_t         offs;
     unsigned short pdh_length, isp_length;
     unsigned short aux_sync, det_sync, pmd_sync;

     /* allocate memory to buffer source packages */
     if ( (mds_char = (char *) malloc( (size_t) dsd->size )) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mds_char" );

     /* read all Sciamachy source packages in product */
     (void) fseek( fd, (long) dsd->offset, SEEK_SET );
     if ( fread( mds_char, (size_t) dsd->size, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, dsd->name );

     /* examine whole source data section */
     cpntr = mds_char;
     do {
	  /* store byte offset w.r.t. begin of file */
	  info_pntr->offset = (unsigned int) 
	       (dsd->offset + (cpntr - mds_char));
	  info_pntr->q.value = 0;

	  (void) memcpy( &info_pntr->mjd, cpntr, sizeof(struct mjd_envi) );
	  cpntr += (3 * ENVI_UINT);
	  cpntr += (3 * ENVI_UINT);  /* skip FEP mjd */
	  (void) memcpy( &isp_length, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;       /* skip DSR size */
	  (void) memcpy( &info_pntr->crc_errors, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
	  (void) memcpy( &info_pntr->rs_errors, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
	  cpntr += ENVI_USHRT;       /* skip spare */
	  cpntr += ENVI_USHRT;       /* skip Packet identifier */
	  cpntr += ENVI_USHRT;       /* skip Packet sequence control */
	  (void) memcpy( &info_pntr->packet_length, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
	  (void) memcpy( &pdh_length, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;       /* skip Packet data header length */
	  (void) memcpy( &info_pntr->category, cpntr, ENVI_UCHAR );
	  cpntr += ENVI_UCHAR;
	  (void) memcpy( &info_pntr->state_id, cpntr, ENVI_UCHAR );
	  cpntr += ENVI_UCHAR;
	  (void) memcpy( &info_pntr->on_board_time, cpntr, ENVI_UINT );
	  cpntr += ENVI_UINT;
	  cpntr += ENVI_USHRT;       /* skip Redundancy definition vector */
	  (void) memcpy( &info_pntr->packet_id, cpntr, ENVI_UCHAR );
	  info_pntr->packet_id = info_pntr->packet_id >> 4;
	  cpntr += ENVI_UCHAR;
	  cpntr += ENVI_UCHAR;       /* skip overfow flag */

	  /* check package corruption */
	  if ( info_pntr->packet_length != isp_length )
	       break;

	  /* check correctness of packet_id */
	  if ( info_pntr->packet_id == 0 || info_pntr->packet_id > 3 ) {
	       pdh_length = byte_swap_u16( pdh_length );
	       isp_length = byte_swap_u16( isp_length );
	       
	       if ( pdh_length == 30 || isp_length == 1659 )
		    info_pntr->packet_id = SCIA_AUX_PACKET;
	       else if ( pdh_length == 12 || isp_length == 6813 ) {
		    info_pntr->packet_id = SCIA_PMD_PACKET;
	       } else {
		    info_pntr->packet_id = SCIA_DET_PACKET;
	       }
	       info_pntr->q.flag.packet_id = 1;
	  }
	  
	  /* read BCPS, different for AUX, DET and PMD */
	  info_pntr->bcps = 0;
          if ( info_pntr->packet_id == SCIA_DET_PACKET ) {
	       offs = DET_DATA_HDR_LENGTH - LV0_DATA_HDR_LENGTH;
	       (void) memcpy( &det_sync, cpntr+offs, sizeof(short) );
	       if ( det_sync != 0xAAAA && det_sync != 0 )
		    info_pntr->q.flag.sync = 1;
	       offs += 2 * ENVI_USHRT;
	       (void) memcpy( &info_pntr->bcps, cpntr+offs, sizeof(short) );
	  } else if ( info_pntr->packet_id == SCIA_AUX_PACKET ) {
	       offs = LV0_PMTC_HDR_LENGTH + ENVI_USHRT;
	       (void) memcpy( &info_pntr->bcps, cpntr+offs, sizeof(short) );

	       offs = LV0_PMTC_HDR_LENGTH;
	       for ( ii = 0; ii < NUM_LV0_AUX_PMTC_FRAME; ii++ ) {
		    for ( jj = 0; jj < NUM_LV0_AUX_BCP; jj++ ) {
			 (void) memcpy( &aux_sync, cpntr+offs, sizeof(short) );
			 if ( aux_sync != 0xDDDD && aux_sync != 0 )
			      info_pntr->q.flag.sync = 1;
			 offs += 20;
		    }
		    offs += 6;
	       }
	  } else {
	       offs = 2 * (PMD_NUMBER + 1) * ENVI_USHRT;
	       (void) memcpy( &info_pntr->bcps, cpntr+offs, sizeof(short) );
	       offs = ENVI_USHRT;  // skip PMD temperature
	       for ( ii = 0; ii < NUM_LV0_PMD_PACKET; ii++ ) {
		    (void) memcpy( &pmd_sync, cpntr+offs, sizeof(short) );
		    if ( pmd_sync != 0xEEEE && pmd_sync != 0 ) {
			 info_pntr->q.flag.sync = 1;
		    }
		    offs += 34;
	       }
          }

#ifdef _SWAP_TO_LITTLE_ENDIAN
	  info_pntr->mjd.days      = byte_swap_32(  info_pntr->mjd.days );
	  info_pntr->mjd.secnd     = byte_swap_u32( info_pntr->mjd.secnd );
	  info_pntr->mjd.musec     = byte_swap_u32( info_pntr->mjd.musec );
	  info_pntr->crc_errors    = byte_swap_u16( info_pntr->crc_errors );
	  info_pntr->rs_errors     = byte_swap_u16( info_pntr->rs_errors );
	  info_pntr->on_board_time = byte_swap_u32( info_pntr->on_board_time );
	  info_pntr->bcps          = byte_swap_u16( info_pntr->bcps );
	  info_pntr->packet_length = byte_swap_u16( info_pntr->packet_length );
#endif
	  /* move to the next MDS */
	  cpntr += info_pntr->packet_length - 11;
	  if ( (cpntr - mds_char) > dsd->size ) break;
     } while ( info_pntr++, ++num_info < dsd->num_dsr );
done:
     if ( mds_char != NULL ) free( mds_char );
     return num_info;
}
