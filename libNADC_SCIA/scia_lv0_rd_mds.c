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

.IDENTifer   SCIA_LV0_RD_MDS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0 data
.LANGUAGE    ANSI C
.PURPOSE     read SCIAMACHY level 0 Measurement Data Sets 
.COMMENTS    contains SCIA_LV0_RD_AUX, SCIA_LV0_RD_DET, SCIA_LV0_RD_PMD,
		      SCIA_LV0_RD_LV1_AUX, SCIA_LV0_FREE_MDS_DET, 
		      SCIA_LV0_RD_LV1_PMD
             Documentation:
	      - Envisat-1 Product Specifications
	        Volume 6: Level 0 Product Specification
		Ref: PO-RS-MDA-GS-2009
	      - Measurement Data Definition and Format description
	        for SCIAMACHY
		Ref: PO-ID-DOR-SY-0032

.ENVIRONment none
.EXTERNALs   ENVI_GET_DSD_INDEX 
.VERSION      5.4   29-Sep-2011	check on likelihood of "start" corruption, RvH
              5.3   22-Nov-2009	more fixes in clusDef correction algorithm, RvH
              5.2   28-Oct-2009	move GET_LV0_MDS_INFO to seperate module, RvH
              5.1   10-Apr-2009	fixed memory corruption bug (numClusters=0), RvH
              5.0   08-May-2007	improved DET_SRC correction, RvH
              4.2   04-May-2007	add routine to correct corrupted clusters, RvH
              4.15  20-Feb-2006	add flag "flag_silent" to GET_LV0_MDS_INFO, RvH
              4.13  22-Jan-2004	implemented skipping of corrupted MDS, RvH
              4.12  13-Oct-2003	changes to due modification of mds0_info, RvH
              4.11  06-Dec-2002	bug: swapped indices of PMD data, RvH
              4.10  04-Dec-2002	added bcps to info-struct for mds_det, RvH
              4.9   04-Oct-2002	give better error-mesg when reading fails, RvH
              4.8   19-Mar-2002	replaced confusing bench_obm 
                                with bench_rad, RvH
              4.7   19-Feb-2002	no longer uses indx_info, RvH 
              4.6   01-Feb-2002	added selection in detector channel(s), RvH 
              4.5   01-Feb-2002	adapted to new MDS structures, RvH 
              4.4   04-Jan-2002	fixed reading AUX_SRC, RvH 
              4.3   20-Dec-2001	added SCIA_LV0_ALLOC_MDS_DET, RvH 
              4.2   12-Dec-2001	updated documentation, RvH 
              4.1   07-Dec-2001	added check for data integrity, RvH 
              4.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              3.0   01-Nov-2001	moved to new Error handling routines, RvH 
              2.0   12-Oct-2001 complete new implementation, RvH
                                  separate modules for Auxiliary, Detector,
				  and PMD data
              1.2   10-Oct-2001 added list of external (NADC) function, RvH
              1.1   09-Oct-2001 changed input/output, RvH
              1.0   11-Nov-2001 created by R. M. van Hees
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
#include <limits.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

static bool ClusterCorrectionFlag = TRUE;
static unsigned short numClusDef  = 0;
static struct clusdef_rec clusDef[MAX_NUM_STATE];

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include "selected_channel.inc"
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include "swap_lv0_mds.inc"
#endif /* _SWAP_TO_LITTLE_ENDIAN */

static inline
void SET_NO_CLUSTER_CORRECTION( FILE *fd )
{
     char *env_str = getenv( "NO_CLUSTER_CORRECTION" );

     if (  env_str == NULL ) {
	  struct mph_envi  mph;

	  ENVI_RD_MPH( fd, &mph );
	  if ( mph.abs_orbit > 4151 ) 
	       ClusterCorrectionFlag = TRUE;
	  else
	       ClusterCorrectionFlag = FALSE;
     } else if ( *env_str == '0' )  {
	  ClusterCorrectionFlag = TRUE;
     } else {
	  ClusterCorrectionFlag = FALSE;
     }
}


/*+++++++++++++++++++++++++
.IDENTifier CHECK_CLUSTERDEF
.PURPOSE    Check and (if necessary) correct cluster parameters
.INPUT/OUTPUT
  call as   stat = CHECK_CLUSTERDEF( ncl, det_src );
 in/output:  
	    struct det_src *data_src  : Detector Source Packets

.RETURNS     flag to indicate if and what has been modified (unsigned char)
             error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
#define DET_SRC_MODIFIED_NONE   ((unsigned char) 0x0U)
#define DET_SRC_MODIFIED_CHANID ((unsigned char) 0x1U)
#define DET_SRC_MODIFIED_CLUSID ((unsigned char) 0x2U)
#define DET_SRC_MODIFIED_START  ((unsigned char) 0x4U)
#define DET_SRC_MODIFIED_LENGTH ((unsigned char) 0x8U)
#define DET_SRC_READ_FAILED     ((unsigned char) 0x10U)

#define FMT_MSG_CHANID   "corrected ID of channel from %-hhu to %-hhu"
#define FMT_MSG_CLUSID   "corrected ID of cluster from %-hhu to %-hhu"
#define FMT_MSG_START    "corrected start of cluster from %-hu to %-hu"
#define FMT_MSG_LENGTH   "corrected length of cluster from %-hu to %-hu"
#define FMT_MSG_FAIL     "can not correct corrupted cluster"

static inline
unsigned char CHECK_CLUSTERDEF( unsigned short ncl, 
				struct det_src *det_src )
       /*@globals  nadc_stat, nadc_err_stack, numClusDef, clusDef;@*/
       /*@modifies nadc_stat, nadc_err_stack, det_src@*/
{
     const char prognm[] = "CHECK_CLUSTERDEF";

     register unsigned short nc;

     char msg[SHORT_STRING_LENGTH];

     const unsigned char chanID  = det_src->hdr.channel.field.id;
     const unsigned char clusID  = (det_src->pixel[ncl].cluster_id &= 0x0F);
     const unsigned short start  = det_src->pixel[ncl].start;
     const unsigned short length = det_src->pixel[ncl].length;

     /* perform check on request or when a memeory corruption is obvious */
     if ( ! ClusterCorrectionFlag && length <= CHANNEL_SIZE ) 
	  return DET_SRC_MODIFIED_NONE;

     /* check if all fields in header are oke */
     nc = 0;
     do {
	  if ( clusDef[nc].chanID == chanID 
	       && clusDef[nc].clusID == clusID
	       && clusDef[nc].start == start
	       && clusDef[nc].length == length ) break;
     } while( ++nc < numClusDef );
     if ( nc < numClusDef ) return DET_SRC_MODIFIED_NONE;

     /* check if 3 fields in header are oke, except "clusID" */
     nc = 0;
     do {
	  if ( clusDef[nc].chanID == chanID 
	       && clusDef[nc].start == start
	       && clusDef[nc].length == length ) break;
     } while( ++nc < numClusDef );
     if ( nc < numClusDef ) {
	  det_src->pixel[ncl].cluster_id = clusDef[nc].clusID;
	  (void) snprintf( msg, SHORT_STRING_LENGTH, FMT_MSG_CLUSID, 
			   clusID, clusDef[nc].clusID );
	  NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	  return DET_SRC_MODIFIED_CLUSID;
     }

     /* check if 3 fields in header are oke, except "chanID" */
     nc = 0;
     do {
	  if ( clusDef[nc].clusID == clusID
	       && clusDef[nc].start == start
	       && clusDef[nc].length == length ) break;
     } while( ++nc < numClusDef );
     if ( nc < numClusDef ) {
	  det_src->hdr.channel.field.id = clusDef[nc].chanID;
	  (void) snprintf( msg, SHORT_STRING_LENGTH, FMT_MSG_CHANID, 
			   chanID, clusDef[nc].chanID );
	  NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	  return DET_SRC_MODIFIED_CHANID;
     }

     /* check if 3 fields in header are oke, except "start" */
     nc = 0;
     do {
	  if ( clusDef[nc].chanID == chanID 
	       && clusDef[nc].clusID == clusID
	       && clusDef[nc].length == length ) break;
     } while( ++nc < numClusDef );
     if ( nc < numClusDef ) {
	  det_src->pixel[ncl].start = clusDef[nc].start;
	  (void) snprintf( msg, SHORT_STRING_LENGTH, FMT_MSG_START, 
			   start, clusDef[nc].start );
	  NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	  return DET_SRC_MODIFIED_START;
     }

     /* check if 3 fields in header are oke, except "length" */
     nc = 0;
     do {
	  if ( clusDef[nc].chanID == chanID 
	       && clusDef[nc].clusID == clusID
	       && clusDef[nc].start == start ) break;
     } while( ++nc < numClusDef );
     if ( nc < numClusDef ) {
	  det_src->pixel[ncl].length = clusDef[nc].length;
	  (void) snprintf( msg, SHORT_STRING_LENGTH, FMT_MSG_LENGTH,
			   length, clusDef[nc].length );
	  NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	  return DET_SRC_MODIFIED_LENGTH;
     }

     /* Oeps, can not correct inconsistent header fields */
     (void) snprintf( msg, SHORT_STRING_LENGTH, FMT_MSG_FAIL );
     NADC_ERROR( prognm, NADC_ERR_NONE, msg );
     return DET_SRC_READ_FAILED;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_MDS_ANNOTATION
.PURPOSE     read level 0 MDS  annotations
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_MDS_ANNOTATION( fd, *isp, *fep_hdr );
     input:  
            FILE *fd                 : stream pointer
    output:  
	    struct mjd_envi *isp     : ISP sensing time
	    struct fep_hdr  *fep_hdr : FEP annotations

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void SCIA_LV0_RD_MDS_ANNOTATION( FILE *fd, 
                                 /*@out@*/ struct mjd_envi *isp,
				 /*@out@*/ struct fep_hdr  *fep_hdr )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, isp, fep_hdr@*/
{
     const char prognm[] = "SCIA_LV0_RD_MDS_ANNOTATION";

     register char *hdr_pntr;

     char hdr_char[LV0_ANNOTATION_LENGTH];
/*
 * read header data
 */
     if ( fread( hdr_char, LV0_ANNOTATION_LENGTH, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );
/*
 * read ISP sensing time
 */
     hdr_pntr = hdr_char;
     (void) memcpy( &isp->days, hdr_pntr, ENVI_INT );
     hdr_pntr += ENVI_INT;
     (void) memcpy( &isp->secnd, hdr_pntr, ENVI_UINT );
     hdr_pntr += ENVI_UINT;
     (void) memcpy( &isp->musec, hdr_pntr, ENVI_UINT );
     hdr_pntr += ENVI_UINT;
/*
 * read Annotation (FEP)
 */
     (void) memcpy( &fep_hdr->gsrt.days, hdr_pntr, ENVI_INT );
     hdr_pntr += ENVI_INT;
     (void) memcpy( &fep_hdr->gsrt.secnd, hdr_pntr, ENVI_UINT );
     hdr_pntr += ENVI_UINT;
     (void) memcpy( &fep_hdr->gsrt.musec, hdr_pntr, ENVI_UINT );
     hdr_pntr += ENVI_UINT;
     (void) memcpy( &fep_hdr->isp_length, hdr_pntr, ENVI_USHRT );
     hdr_pntr += ENVI_USHRT;
     (void) memcpy( &fep_hdr->crc_errs, hdr_pntr, ENVI_USHRT );
     hdr_pntr += ENVI_USHRT;
     (void) memcpy( &fep_hdr->rs_errs, hdr_pntr, ENVI_USHRT );

#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_ANNOTATION( isp, fep_hdr );
#endif
     return;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_MDS_PACKET_HDR
.PURPOSE     read level 0 MDS ISP Packet header
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_MDS_PACKET_HDR( fd, *packet_hdr );
     input:  
            FILE *fd                      : stream pointer
    output:  
	    struct packet_hdr *packet_hdr : Packet header

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SCIA_LV0_RD_MDS_PACKET_HDR( FILE *fd, 
                                 /*@out@*/ struct packet_hdr *packet_hdr )
       /*@globals  errno;@*/
       /*@modifies errno, fd, packet_hdr@*/
{
     const char prognm[] = "SCIA_LV0_RD_MDS_PACKET_HDR";

     if ( fread( &packet_hdr->api.two_byte, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );
     if ( fread( &packet_hdr->seq_cntrl, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );
     if ( fread( &packet_hdr->length, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );

#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_PACKET_HDR( packet_hdr );
#endif
     return;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_MDS_DATA_HDR
.PURPOSE     read Data Field Header of the Packet Data Field [ISP]
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_MDS_DATA_HDR( fd, data_hdr );
     input:  
            FILE *fd                  : stream pointer
    output:  
            struct data_hdr *data_hdr : Data Field Header

.RETURNS     nothing
.COMMENTS    Static function
             The Data Field Header for each of the three packets (i.e.
	     Detector, Auxiliary and PMD) contains standard information.
-------------------------*/
static
void SCIA_LV0_RD_MDS_DATA_HDR( FILE *fd, 
                               /*@out@*/ struct data_hdr *data_hdr )
       /*@globals  errno;@*/
       /*@modifies errno, fd, *data_hdr@*/
{
     const char prognm[] = "SCIA_LV0_RD_MDS_DATA_HDR";

     if ( fread( &data_hdr->length, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );
     if ( fread( &data_hdr->category, ENVI_UCHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );
     if ( fread( &data_hdr->state_id, ENVI_UCHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );
     if ( fread( &data_hdr->on_board_time, ENVI_UINT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );
     if ( fread( &data_hdr->rdv.two_byte, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );
     if ( fread( &data_hdr->id.two_byte, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );

#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_DATA_HDR( data_hdr );
#endif
     return;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_MDS_PMTC_HDR
.PURPOSE     read PMTC settings from the ISP Data Field Header
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_MDS_PMTC_HDR( fd, pmtc_hdr );
     input:  
            FILE *fd                  : stream pointer
    output:  
            struct pmtc_hdr *pmtc_hdr : PMTC settings

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void SCIA_LV0_RD_MDS_PMTC_HDR( FILE *fd, 
                               /*@out@*/ struct pmtc_hdr *pmtc_hdr )
       /*@globals  errno;@*/
       /*@modifies errno, fd, *pmtc_hdr@*/

{
     const char prognm[] = "SCIA_LV0_RD_MDS_PMTC_HDR";

     if ( fread( &pmtc_hdr->pmtc_1.two_byte, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );
     if ( fread( &pmtc_hdr->scanner_mode, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );
     if ( fread( &pmtc_hdr->az_param.four_byte, ENVI_UINT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );
     if ( fread( &pmtc_hdr->elv_param.four_byte, ENVI_UINT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );
     if ( fread( &pmtc_hdr->factor, 6 * ENVI_UCHAR, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );

#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_PMTC_HDR( pmtc_hdr );
#endif
     return;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_PMTC_FRAME
.PURPOSE     read source data of Auxilirary MDS
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_PMTC_FRAME( fd, pmtc_frame );
     input:  
            FILE *fd                      : stream pointer
    output:  
            struct pmtc_frame *pmtc_frame : PMTC frame

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static inline
void SCIA_LV0_RD_MDS_PMTC_FRAME( FILE *fd, 
				 /*@out@*/ struct pmtc_frame *pmtc_frame )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *pmtc_frame@*/
{
     const char prognm[] = "SCIA_LV0_RD_PMTC_FRAME";
     
     register unsigned short nb;

     char *src_pntr;
     char src_char[AUX_DATA_SRC_LENGTH];

     unsigned int ubuff;

     const unsigned short AUX_SYNC = 0xDDDD;
/*
 * initialize output structure to zero
 */
     (void) memset( pmtc_frame, 0, sizeof( struct pmtc_frame ));
/*
 * read packet data header data
 */
     if ( fread( src_char, AUX_DATA_SRC_LENGTH, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "" );

     src_pntr = src_char;
     nb = 0;
     do {
	  (void) memcpy( &pmtc_frame->bcp[nb].sync, src_pntr, ENVI_USHRT );
/*
 * check data integrity
 */
	  if ( pmtc_frame->bcp[nb].sync != USHRT_ZERO
	       && pmtc_frame->bcp[nb].sync != AUX_SYNC ) {
	       NADC_RETURN_ERROR( prognm, NADC_WARN_PDS_RD, 
				  "incorrect AUX Sync value(s) found" );
	  }
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( &pmtc_frame->bcp[nb].bcps, src_pntr, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( &pmtc_frame->bcp[nb].flags.two_byte, 
			 src_pntr, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( &ubuff, src_pntr, ENVI_UINT );
	  pmtc_frame->bcp[nb].azi_encode_cntr = 
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       (byte_swap_u32( ubuff ) >> 4) & (~0U >> 12);
#else
	       (ubuff >> 4) & (~0U >> 12);
#endif
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( &ubuff, src_pntr, ENVI_UINT );
	  pmtc_frame->bcp[nb].ele_encode_cntr = 
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       byte_swap_u32( ubuff ) & (~0U >> 12);
#else
	       ubuff & (~0U >> 12);
#endif
	  src_pntr += ENVI_UINT;
	       
	  (void) memcpy( &pmtc_frame->bcp[nb].azi_cntr_error, 
			 src_pntr, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( &pmtc_frame->bcp[nb].ele_cntr_error, 
			 src_pntr, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( &pmtc_frame->bcp[nb].azi_scan_error, 
			 src_pntr, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( &pmtc_frame->bcp[nb].ele_scan_error, 
			 src_pntr, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
     } while ( ++nb < NUM_LV0_AUX_BCP );
	  
     (void) memcpy( &pmtc_frame->bench_rad.two_byte, src_pntr, ENVI_USHRT );
     src_pntr += ENVI_USHRT;
     (void) memcpy( &pmtc_frame->bench_elv.two_byte, src_pntr, ENVI_USHRT );
     src_pntr += ENVI_USHRT;
     (void) memcpy( &pmtc_frame->bench_az.two_byte, src_pntr, ENVI_USHRT );
     src_pntr += ENVI_USHRT;

     if ( (src_pntr - src_char) != AUX_DATA_SRC_LENGTH ) {
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_SIZE, 
			     "Auxiliary Source Data" );
     }
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_PMTC_FRAME( pmtc_frame );
#endif
}

/*+++++++++++++++++++++++++
.IDENTifier   SCIA_LV0_RD_MDS_DET_SRC
.PURPOSE     read Source Data of Detector Data Packet
.INPUT/OUTPUT
  call as   nr_chan = SCIA_LV0_RD_MDS_DET_SRC( fd, det_length,
                                               chan_mask, num_chan, data_src );
     input:  
            FILE *fd                  : stream pointer
            size_t det_length         : size of the Packet data field 
                                          (in bytes)
	    unsigned char chan_mask   : mask for channel selection
            unsigned short num_chan   : Number of channels
    output:  
            struct det_src *data_src  : Detector Source Packets

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
unsigned short SCIA_LV0_RD_MDS_DET_SRC( FILE *fd, size_t det_length, 
					unsigned char chan_mask, 
					unsigned short num_chan,
					/*@out@*/ struct det_src *data_src )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, data_src@*/
{
     const char prognm[] = "SCIA_LV0_RD_MDS_DET_SRC";

     register unsigned short n_ch, n_cl;

     char *src_pntr;
     char *src_char;

     unsigned char  stat = DET_SRC_MODIFIED_NONE;
     unsigned short nr_chan = 0;

     bool           Band_Is_Selected;
     size_t         num_byte;
     unsigned short numClusters = 0;

     struct chan_src dummy_pixel[LV0_MX_CLUSTERS];

     const unsigned short CHANNEL_SYNC = 0xAAAA;
     const unsigned short CLUSTER_SYNC = 0xBBBB;
/*
 * read packet data header data
 */
     if ( (src_char = (char *) malloc( det_length )) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_ALLOC, "src_char" );
	  return (unsigned short) 0;
     }
     if ( fread( src_char, det_length, 1, fd ) != 1 )  {
	  free( src_char );
	  NADC_ERROR( prognm, NADC_ERR_FILE_RD, "src_char" );
	  return (unsigned short) 0;
     }
/*
 * read data of the different channels
 */
     src_pntr = src_char;
     for ( n_ch = 0; n_ch < num_chan; n_ch++ ) {
	  (void) memcpy( &data_src->hdr.sync, src_pntr, ENVI_USHRT );
/*
 * check data integrity
 */
	  if ( data_src->hdr.sync != CHANNEL_SYNC )
	       NADC_GOTO_ERROR( prognm, NADC_WARN_PDS_RD, 
				"incorrect Channel Sync value(s) found" );
	  src_pntr += ENVI_USHRT;

	  (void) memcpy( &data_src->hdr.channel.two_byte, src_pntr, 
			 ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( &data_src->hdr.bcps, src_pntr, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( &data_src->hdr.command_vis.four_byte, src_pntr, 
			 ENVI_UINT );
	  (void) memcpy( &data_src->hdr.command_ir.four_byte, src_pntr, 
			 ENVI_UINT );
	  src_pntr += ENVI_UINT;
	  (void) memcpy( &data_src->hdr.ratio_hdr.two_byte, src_pntr, 
			 ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( &data_src->hdr.bias, src_pntr, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( &data_src->hdr.temp, src_pntr, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_MDS_CHAN_HDR( &data_src->hdr );
#endif
	  numClusters = data_src->hdr.channel.field.clusters;
	  Band_Is_Selected = 
	       SELECTED_CHANNEL( chan_mask, data_src->hdr.channel.field.id );
	  if ( Band_Is_Selected && numClusters > 0 ) {
	       nr_chan++;
	       data_src->pixel = (struct chan_src *)
		    malloc ( numClusters * sizeof( struct chan_src ) );
	       if ( data_src->pixel == NULL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "pixel" );
	  } else 
	       data_src->pixel = dummy_pixel;
/*
 * read data of the clusters
 */
	  n_cl = (unsigned short) 0;
	  do {
	       (void) memcpy( &data_src->pixel[n_cl].sync, src_pntr, 
			      ENVI_USHRT );
/* check data integrity */
	       if ( data_src->pixel[n_cl].sync != CLUSTER_SYNC ) {
		    if ( Band_Is_Selected && numClusters > 0 ) {
			 while ( n_cl > 0 )
			      free( data_src->pixel[--n_cl].data );
			 free( data_src->pixel );
			 data_src->hdr.channel.field.clusters = 0;
		    }
		    NADC_GOTO_ERROR( prognm, NADC_WARN_PDS_RD, 
				 "corrupted cluster Sync, remainder skipped" );
	       }
	       src_pntr += ENVI_USHRT;
/* cluster block identifier */
	       (void) memcpy( &data_src->pixel[n_cl].block_nr, src_pntr, 
			      ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       data_src->pixel[n_cl].block_nr = 
		    byte_swap_u16( data_src->pixel[n_cl].block_nr );
#endif
	       src_pntr += ENVI_USHRT;
/* cluster identifier */
	       (void) memcpy( &data_src->pixel[n_cl].cluster_id, src_pntr,
			      ENVI_UCHAR );
	       src_pntr += ENVI_UCHAR;
/* co-adding indicator */
	       (void) memcpy( &data_src->pixel[n_cl].co_adding, src_pntr, 
			      ENVI_UCHAR );
	       src_pntr += ENVI_UCHAR;
/* start pixel indicator */
	       (void) memcpy( &data_src->pixel[n_cl].start, src_pntr, 
			      ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       data_src->pixel[n_cl].start = 
		    byte_swap_u16( data_src->pixel[n_cl].start );
#endif
	       src_pntr += ENVI_USHRT;
/* cluster block length */
	       (void) memcpy( &data_src->pixel[n_cl].length, src_pntr, 
			      ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       data_src->pixel[n_cl].length = 
		    byte_swap_u16( data_src->pixel[n_cl].length );
#endif
	       src_pntr += ENVI_USHRT;
/* check for corrupted cluster, and try to correct them */
	       stat = CHECK_CLUSTERDEF( n_cl, data_src );
	       if ( stat == DET_SRC_READ_FAILED ) {
		    if ( Band_Is_Selected && numClusters > 0 ) {
			 while ( n_cl > 0 )
			      free( data_src->pixel[--n_cl].data );
			 free( data_src->pixel );
			 data_src->hdr.channel.field.clusters = 0;
		    }
		    NADC_GOTO_ERROR( prognm, NADC_WARN_PDS_RD,
				"corrupted cluster block, remainder skipped" );
	       }
/* determine size of cluster pixel data block */
	       if ( data_src->pixel[n_cl].co_adding == UCHAR_ONE )
		    num_byte = (size_t) 
			 data_src->pixel[n_cl].length * ENVI_USHRT;
	       else {
		    num_byte = (size_t) 
			 data_src->pixel[n_cl].length * 3 * ENVI_UCHAR;
		    if ( (data_src->pixel[n_cl].length % 2) == 1 ) 
			 num_byte += ENVI_UCHAR;
	       }
/* pixel data */
	       if ( Band_Is_Selected ) {
		    data_src->pixel[n_cl].data = 
			 (unsigned char *) malloc( num_byte );
		    if ( data_src->pixel[n_cl].data == NULL )
			 NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, 
					  "pixel->data" );
		    (void) memcpy( data_src->pixel[n_cl].data, 
				   src_pntr, num_byte );
	       }
	       src_pntr += num_byte;
	       
	  } while( ++n_cl < numClusters );

	  if ( Band_Is_Selected ) data_src++;
     }   /* for-loop over num_chan */
/*
 * check number of bytes read
 */
     if ( (size_t)(src_pntr - src_char) != det_length ) {
	  char msg[80];
	  (void) snprintf( msg, 80, 
			   "Detector[%-hu] MDS size: %zd instead of %zd", 
			   numClusters, (size_t)(src_pntr - src_char), 
			   det_length );
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, msg );
     }
/*
 * release allocated memory
 */
 done:
     free( src_char );
     return nr_chan;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_PMD_SRC
.PURPOSE     read source data of PMD MDS
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_PMD_SRC( fd, pmd_src );
     input:  
            FILE *fd                : stream pointer
    output:  
            struct pmd_src *pmd_src : structure for PMD Source data

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void SCIA_LV0_RD_MDS_PMD_SRC( FILE *fd, /*@out@*/ struct pmd_src *pmd_src )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *pmd_src@*/
{
     const char prognm[]  = "SCIA_LV0_RD_PMD_SRC";

     register unsigned short np;

     char *src_pntr;
     char src_char[PMD_DATA_SRC_LENGTH];

     const size_t nr_byte = 2 * PMD_NUMBER * ENVI_USHRT;

     const unsigned short PMD_SYNC = 0xEEEE;
/*
 * initialize output structure to zero
 */
     (void) memset( pmd_src, 0, sizeof( struct pmd_src ));
/*
 * read packet data header data
 */
     if ( fread( src_char, PMD_DATA_SRC_LENGTH, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "" );

     src_pntr = src_char;
     (void) memcpy( &pmd_src->temp, src_pntr, ENVI_USHRT );
     src_pntr += ENVI_USHRT;

     np = 0;
     do {
	  (void) memcpy( &pmd_src->packet[np].sync, src_pntr, ENVI_USHRT );
/*
 * check data integrity
 */
	  if ( pmd_src->packet[np].sync != USHRT_ZERO
	       && pmd_src->packet[np].sync != PMD_SYNC ) {
	       NADC_RETURN_ERROR( prognm, NADC_WARN_PDS_RD, 
				  "incorrect PMD Sync value(s) found" );
	  }
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( pmd_src->packet[np].data, src_pntr, nr_byte );
	  src_pntr += nr_byte;
	  (void) memcpy( &pmd_src->packet[np].bcps, src_pntr, ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
	  (void) memcpy( &pmd_src->packet[np].time.two_byte, src_pntr, 
			   ENVI_USHRT );
	  src_pntr += ENVI_USHRT;
     } while ( ++np < NUM_LV0_PMD_PACKET );

     if ( (src_pntr - src_char) != PMD_DATA_SRC_LENGTH ) {
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_SIZE, "PMD Source Data" );
     }
#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_PMD_SRC( pmd_src );
#endif
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_ONE_AUX
.PURPOSE     read one SCIAMACHY level 0 Auxiliary MDS
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_ONE_AUX( fd, info, aux );
     input:  
            FILE   *fd             : (open) stream pointer
	    struct mds0_info *info : structure holding info about MDS records
    output:  
            struct mds0_aux *aux   : Auxiliary MDS records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void SCIA_LV0_RD_ONE_AUX( FILE *fd, const struct mds0_info *info,
			  struct mds0_aux *aux )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, aux@*/
{
     const char prognm[] = "SCIA_LV0_RD_ONE_AUX";

     register unsigned short nf;
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) info->offset, SEEK_SET );
/*
 * read Annotation (ISP, FEP)
 */
     SCIA_LV0_RD_MDS_ANNOTATION( fd, &aux->isp, &aux->fep_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "Annotation" );
/*
 * read ISP Packet Header
 */
     SCIA_LV0_RD_MDS_PACKET_HDR( fd, &aux->packet_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "PACKET_HDR" );
/*
 * read ISP Data Field Header
 */
     SCIA_LV0_RD_MDS_DATA_HDR( fd, &aux->data_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_DATA_HDR" );
/*
 * check packet ID
 */
     if ( (int) aux->data_hdr.id.field.packet != SCIA_AUX_PACKET )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, 
			     "not an Auxiliary data packet" );
/*
 * read PMTC settings
 */
     SCIA_LV0_RD_MDS_PMTC_HDR( fd, &aux->pmtc_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_PMTC_HDR" );
/*
 * read ISP Auxiliary data source packet
 */
     nf = 0;
     do {
	  SCIA_LV0_RD_MDS_PMTC_FRAME( fd, &aux->data_src[nf] );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_PMTC_FRAME" );
     } while ( ++nf < NUM_LV0_AUX_PMTC_FRAME );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_ONE_DET
.PURPOSE     read one SCIAMACHY level 0 Detector MDS
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_ONE_DET( fd, info, det );
     input:  
            FILE   *fd              : (open) stream pointer
	    struct mds0_info *info  : structure holding info about MDS records
	    unsigned char chan_mask : mask for channel selection
    output:  
            struct mds0_det *det    : Detector MDS records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void SCIA_LV0_RD_ONE_DET( FILE *fd, unsigned char chan_mask,
			  const struct mds0_info *info,
			  struct mds0_det *det )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, det@*/
{
     const char prognm[] = "SCIA_LV0_RD_ONE_DET";

     size_t nr_byte;
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) info->offset, SEEK_SET );
/*
 * read Annotation (ISP, FEP)
 */
     SCIA_LV0_RD_MDS_ANNOTATION( fd, &det->isp, &det->fep_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "Annotation" );
/*
 * read Packet Header
 */
     SCIA_LV0_RD_MDS_PACKET_HDR( fd, &det->packet_hdr);
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "PACKET_HDR" );
/*
 * read ISP Data Field Header
 */
     SCIA_LV0_RD_MDS_DATA_HDR( fd, &det->data_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_DATA_HDR" );
/*
 * check packet ID
 */
     if ( (int) det->data_hdr.id.field.packet != SCIA_DET_PACKET )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, 
			     "not a detector data packet" );
/*
 * read Broadcast counter (MDI)
 */
     if ( fread( &det->bcps, ENVI_SHORT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "" );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     det->bcps = byte_swap_u16( det->bcps );
#endif
/*
 * read PMTC settings
 */
     SCIA_LV0_RD_MDS_PMTC_HDR( fd, &det->pmtc_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_PMTC_HDR" );
/*
 * read remaining variables from Data Field Header for the Detector Data Packet
 */
     if ( fread( det->orbit_vector, ENVI_INT, 8, fd ) != 8 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "" );
     if ( fread( &det->num_chan, ENVI_USHRT, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "" );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     {
	  register unsigned short nr = 0;

	  do {
	       det->orbit_vector[nr] = byte_swap_32( det->orbit_vector[nr] );
	  } while ( ++nr < 8 );

	  det->num_chan = byte_swap_u16( det->num_chan );
     }
#endif
     /* obtain cluster definition (not implemented for absOrbit < 4151) */
     numClusDef = GET_SCIA_CLUSDEF( info->state_id, clusDef );
/*
 * read ISP Detector Data Source Packet
 */
     if ( ! Use_Extern_Alloc ) {
	  det->data_src = (struct det_src *) 
	       malloc( det->num_chan * sizeof( struct det_src ) );
     }
     if ( det->data_src == NULL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "det_src" );

     nr_byte = (size_t) (det->packet_hdr.length - DET_DATA_HDR_LENGTH + 1);
     det->num_chan = SCIA_LV0_RD_MDS_DET_SRC( fd, nr_byte, chan_mask, 
					      det->num_chan, det->data_src );
     if ( IS_ERR_STAT_FATAL ) {
	  if ( ! Use_Extern_Alloc ) free( det->data_src );
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_DATA_SRC" );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_ONE_PMD
.PURPOSE     read one SCIAMACHY level 0 PMD MDS
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_ONE_PMD( fd, info, pmd );
     input:  
            FILE   *fd             : (open) stream pointer
	    struct mds0_info *info : structure holding info about MDS records
    output:  
            struct mds0_pmd *pmd   : PMD MDS records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static inline
void SCIA_LV0_RD_ONE_PMD( FILE *fd, const struct mds0_info *info,
			  struct mds0_pmd *pmd )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, pmd@*/
{
     const char prognm[] = "SCIA_LV0_RD_ONE_PMD";
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) info->offset, SEEK_SET );
/*
 * read Annotation (ISP, FEP)
 */
     SCIA_LV0_RD_MDS_ANNOTATION( fd, &pmd->isp, &pmd->fep_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "Annotation" );
/*
 * read Packet Header
 */
     SCIA_LV0_RD_MDS_PACKET_HDR( fd, &pmd->packet_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "PACKET_HDR" );
/*
 * read ISP Data Field Header
 */
     SCIA_LV0_RD_MDS_DATA_HDR( fd, &pmd->data_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_DATA_HDR" );
/*
 * check packet ID
 */
     if ( (int) pmd->data_hdr.id.field.packet != SCIA_PMD_PACKET )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, 
			     "not a PMD data packet" );
/*
 * read ISP PMD data source packet
 */
     SCIA_LV0_RD_MDS_PMD_SRC( fd, &pmd->data_src );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_PMD_SRC" );
}

/*+++++++++++++++++++++++++ Main Program or Functions +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_AUX
.PURPOSE     read selected SCIAMACHY level 0 Auxiliary MDS
.INPUT/OUTPUT
  call as   nr_aux = SCIA_LV0_RD_AUX( fd, info, num_info, &aux );
     input:  
            FILE   *fd             : (open) stream pointer
	    struct mds0_info *info : structure holding info about MDS records
	    unsigned int num_info  : number of indices to struct info
    output:  
            struct mds0_aux **aux  : Auxiliary MDS records

.RETURNS     number of Auxiliary MDS read (unsigned int), 
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV0_RD_AUX( FILE *fd, const struct mds0_info *info,
			      unsigned int num_info, 
			      struct mds0_aux **aux_out )
{
     const char prognm[] = "SCIA_LV0_RD_AUX";

     register unsigned int nr_aux = 0;

     struct mds0_aux *aux;

     if ( num_info == 0 ) {
	  *aux_out = NULL;
	  return 0u;
     }
/*
 * allocate memory to store output records
 */
     if ( ! Use_Extern_Alloc ) {
	  *aux_out = (struct mds0_aux *) 
	       malloc( num_info * sizeof(struct mds0_aux));
     }
     if ( (aux = *aux_out) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mds0_aux" );
/*
 * read data buffer to MDS structure
 */
     do {
	  SCIA_LV0_RD_ONE_AUX( fd, info, aux );
	  if ( IS_ERR_STAT_FATAL ) {
	       char msg[32];

	       free( *aux_out );
	       (void) snprintf( msg, 32, "MDS_AUX[%-u]", nr_aux );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, msg );
	  }
/*
 * go to next MDS info data set
 */
	  info++;
	  aux++;
     } while ( ++nr_aux < num_info );
/*
 * set return values
 */
     if ( ClusterCorrectionFlag ) {
	  register unsigned int nr = 0;

	  aux -= nr_aux;
	  do {
	       if ( aux[nr].isp.days == 0 ) {
		    if ( nr > 0 )
			 (void) memcpy( &aux[nr].isp, &aux[nr-1].isp, 
					sizeof( struct mjd_envi ) );
		    else
			 (void) memcpy( &aux[nr].isp, &aux[nr+1].isp, 
					sizeof( struct mjd_envi ) );
	       }
	  } while ( ++nr < nr_aux );
     }
 done:
     return nr_aux;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_DET
.PURPOSE     read selected SCIAMACHY level 0 Detector MDS
.INPUT/OUTPUT
  call as   nr_det = SCIA_LV0_RD_DET( fd, info, num_info, &det );
     input:  
            FILE   *fd              : (open) stream pointer
	    struct mds0_info *info  : structure holding info about MDS records
	    unsigned int num_info   : number of indices to struct info
	    unsigned char chan_mask : mask for channel selection
    output:  
            struct mds0_det **det   : Detector MDS records

.RETURNS     number of Detector MDS read (unsigned int), 
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV0_RD_DET( FILE *fd, const struct mds0_info *info,
			      unsigned int num_info, 
			      unsigned char chan_mask,
			      struct mds0_det **det_out )
{
     const char prognm[] = "SCIA_LV0_RD_DET";

     register unsigned int   nr_det = 0;

     struct mds0_det *det;

     if ( num_info == 0 ) {
	  *det_out = NULL;
	  return 0u;
     }
/*
 * set flag which indicates that Detector MDS records have to be checked
 */
     SET_NO_CLUSTER_CORRECTION( fd );
/*
 * allocate memory to store output records
 */
     if ( ! Use_Extern_Alloc ) {
	  *det_out = (struct mds0_det *) 
	       malloc( num_info * sizeof(struct mds0_det));
     }
     if ( (det = *det_out) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mds0_det" );
/*
 * read data buffer to MDS structure
 */
     do {
	  SCIA_LV0_RD_ONE_DET( fd, chan_mask, info, det );
	  if ( IS_ERR_STAT_FATAL ) {
	       char msg[32];

	       free( *det_out );
	       (void) snprintf( msg, 32, "MDS_DET[%-u]", nr_det );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, msg );
	  }
/*
 * go to next MDS info data set
 */
	  info++;
	  det++;
     } while ( ++nr_det < num_info );
/*
 * set return values
 */
     if ( ClusterCorrectionFlag ) {
	  register unsigned int nr = 0;

	  det -= nr_det;
	  do {
	       if ( det[nr].isp.days == 0 ) {
		    if ( nr > 0 )
			 (void) memcpy( &det[nr].isp, &det[nr-1].isp, 
					sizeof( struct mjd_envi ) );
		    else
			 (void) memcpy( &det[nr].isp, &det[nr+1].isp, 
					sizeof( struct mjd_envi ) );
	       }
	  } while ( ++nr < nr_det );
     }
 done:
     return nr_det;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_PMD
.PURPOSE     read selected SCIAMACHY level 0 PMD MDS
.INPUT/OUTPUT
  call as   nr_pmd = SCIA_LV0_RD_PMD( fd, info, num_info, &pmd );
     input:  
            FILE   *fd             : (open) stream pointer
	    struct mds0_info *info : structure holding info about MDS records
	    unsigned int num_info  : number of indices to struct info
    output:  
            struct mds0_pmd **pmd  : PMD MDS records

.RETURNS     number of PMD MDS read (unsigned int), 
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV0_RD_PMD( FILE *fd, const struct mds0_info *info,
			      unsigned int num_info, 
			      struct mds0_pmd **pmd_out )
{
     const char prognm[] = "SCIA_LV0_RD_PMD";

     register unsigned int nr_pmd = 0;

     struct mds0_pmd *pmd;

     if ( num_info == 0 ) {
	  *pmd_out = NULL;
	  return 0u;
     }
/*
 * allocate memory to store output records
 */
     if ( ! Use_Extern_Alloc ) {
	  *pmd_out = (struct mds0_pmd *) 
	       malloc( num_info * sizeof(struct mds0_pmd));
     }
     if ( (pmd = *pmd_out) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mds0_pmd" );
/*
 * read data buffer to MDS structure
 */
     do {
	  SCIA_LV0_RD_ONE_PMD( fd, info, pmd );
	  if ( IS_ERR_STAT_FATAL ) {
	       char msg[32];

	       free( *pmd_out );
	       (void) snprintf( msg, 32, "MDS_PMD[%-u]", nr_pmd );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, msg );
	  }
/*
 * go to next MDS info data set
 */
	  info++;
	  pmd++;
     } while ( ++nr_pmd < num_info );
/*
 * set return values
 */
     if ( ClusterCorrectionFlag ) {
	  register unsigned int nr = 0;

	  pmd -= nr_pmd;
	  do {
	       if ( pmd[nr].isp.days == 0 ) {
		    if ( nr > 0 )
			 (void) memcpy( &pmd[nr].isp, &pmd[nr-1].isp, 
					sizeof( struct mjd_envi ) );
		    else
			 (void) memcpy( &pmd[nr].isp, &pmd[nr+1].isp, 
					sizeof( struct mjd_envi ) );
	       }
	  } while ( ++nr < nr_pmd );
     }
 done:
     return nr_pmd;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_FREE_MDS_DET
.PURPOSE     free memory allocated by SCIA_LV0_RD_DET
.INPUT/OUTPUT
  call as   SCIA_LV0_FREE_MDS_DET( num_det, det );
     input:  
            unsigned short num_det  : number of Detector MDS
    output:  
            struct mds0_det *det    : Detector MDS records

.RETURNS     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV0_FREE_MDS_DET( unsigned int num_det, struct mds0_det *det )
{
     register unsigned int nd = 0;

     if ( num_det == 0 ) return;

     do {
	  register unsigned short n_ch, n_cl;

	  for ( n_ch = 0; n_ch < det[nd].num_chan; n_ch++ ) {
	       unsigned short numClusters = (unsigned short) 
		    det[nd].data_src[n_ch].hdr.channel.field.clusters;

	       for ( n_cl = 0; n_cl < numClusters; n_cl++ )
		    free( det[nd].data_src[n_ch].pixel[n_cl].data );

	       if ( numClusters > 0 ) free( det[nd].data_src[n_ch].pixel );
	  }
	  free( det[nd].data_src );
     } while( ++nd < num_det );

     free( det );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_LV1_AUX
.PURPOSE     read SCIAMACHY level 0 Auxiliary MDS as stored in 
             a level 1b product
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_LV1_AUX( fd, aux );
     input:  
            FILE   *fd             : (open) stream pointer
    output:  
            struct mds1_aux *aux   : Auxiliary MDS records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV0_RD_LV1_AUX( FILE *fd, struct mds1_aux *aux )
{
     const char prognm[] = "SCIA_LV0_RD_LV1_AUX";

     register unsigned short nf;
/*
 * read Packet Header
 */
     SCIA_LV0_RD_MDS_PACKET_HDR( fd, &aux->packet_hdr);
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "PACKET_HDR" );
/*
 * read ISP data field header
 */
     SCIA_LV0_RD_MDS_DATA_HDR( fd, &aux->data_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_DATA_HDR" );
/*
 * check packet ID
 */
     if ( (int) aux->data_hdr.id.field.packet != SCIA_AUX_PACKET )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, 
			     "not an Auxiliary data packet" );
/*
 * read PMTC settings
 */
     SCIA_LV0_RD_MDS_PMTC_HDR( fd, &aux->pmtc_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_PMTC_HDR" );
/*
 * read ISP AUX data source packet
 */
     nf = 0;
     do {
	  SCIA_LV0_RD_MDS_PMTC_FRAME( fd, &aux->data_src[nf] );
	  if ( IS_ERR_STAT_FATAL ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_PMTC_FRAME" );
     } while ( ++nf < NUM_LV0_AUX_PMTC_FRAME );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_LV1_PMD
.PURPOSE     read SCIAMACHY level 0 PMD MDS as stored in a level 1b product
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_LV1_PMD( fd, pmd );
     input:  
            FILE   *fd             : (open) stream pointer
    output:  
            struct mds1_pmd *pmd   : PMD MDS records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV0_RD_LV1_PMD( FILE *fd, struct mds1_pmd *pmd )
{
     const char prognm[] = "SCIA_LV0_RD_LV1_PMD";
/*
 * read Packet Header
 */
     SCIA_LV0_RD_MDS_PACKET_HDR( fd, &pmd->packet_hdr);
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "PACKET_HDR" );
/*
 * read ISP data field header
 */
     SCIA_LV0_RD_MDS_DATA_HDR( fd, &pmd->data_hdr );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_DATA_HDR" );
/*
 * check packet ID
 */
     if ( (int) pmd->data_hdr.id.field.packet != SCIA_PMD_PACKET )
 	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "not a PMD data packet" );
/*
 * read ISP PMD data source packet
 */
     SCIA_LV0_RD_MDS_PMD_SRC( fd, &pmd->data_src );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "MDS_PMD_SRC" );
}
