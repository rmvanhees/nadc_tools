/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2015 SRON (R.M.van.Hees@sron.nl)

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
.PURPOSE     read SCIAMACHY level 0 Measurement Data Sets of one State execution
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
.VERSION      5.5   18-Mar-2015	bugfixes and code improvements, RvH
              5.4   29-Sep-2011	check on likelihood of "start" corruption, RvH
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
void SET_NO_CLUSTER_CORRECTION( void )
{
     char *env_str = getenv( "NO_CLUSTER_CORRECTION" );

     if (  env_str != NULL && *env_str == '1' ) {
	  ClusterCorrectionFlag = FALSE;
     } else if ( ! CLUSDEF_DB_EXISTS() ) {
	  ClusterCorrectionFlag = FALSE;
     } else {
	  ClusterCorrectionFlag = TRUE;
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

#define FMT_MSG_CHANID "corrected (%-hhu,%2hhu,%5hu,%4hu) ID of channel to %hhu"
#define FMT_MSG_CLUSID "corrected (%-hhu,%2hhu,%5hu,%4hu) ID of cluster to %hhu"
#define FMT_MSG_START  "corrected (%-hhu,%2hhu,%5hu,%4hu) start of cluster to %hu"
#define FMT_MSG_LENGTH "corrected (%-hhu,%2hhu,%5hu,%4hu) length of cluster to %hu"
#define FMT_MSG_FAIL   "failed correcting cluster (%-hhu,%2hhu,%5hu,%4hu)"

static inline
unsigned char _CHECK_CLUSTERDEF( unsigned short ncl, 
				struct det_src *det_src )
       /*@globals  nadc_stat, nadc_err_stack, numClusDef, clusDef;@*/
       /*@modifies nadc_stat, nadc_err_stack, det_src@*/
{
     const char prognm[] = "_CHECK_CLUSTERDEF";

     register unsigned short nc;

     char msg[SHORT_STRING_LENGTH];

     unsigned char stat = DET_SRC_MODIFIED_NONE;

     const unsigned char mask_id = (unsigned char) 0xFU;
     const unsigned short mask_start = (unsigned short) 0x1FFFU;
     const unsigned short mask_length = (unsigned short) 0x7FFU;

     const unsigned char chanID  = (det_src->hdr.channel.field.id & mask_id);
     const unsigned char clusID  = (det_src->pixel[ncl].cluster_id & mask_id);
     const unsigned short start  = (det_src->pixel[ncl].start & mask_start);
     const unsigned short length = (det_src->pixel[ncl].length & mask_length);

     /* check for precense of an out-of-bound value due to a bitflip */
     if ( chanID != det_src->hdr.channel.field.id ) {
	  det_src->hdr.channel.field.id &= mask_id;
	  stat = DET_SRC_MODIFIED_CHANID;
     }
     if ( clusID != det_src->pixel[ncl].cluster_id ) {
	  det_src->pixel[ncl].cluster_id &= mask_id;
	  stat &= DET_SRC_MODIFIED_CLUSID;
     }
     if ( start  != det_src->pixel[ncl].start ) {
	  det_src->pixel[ncl].start &= mask_start;
	  stat &= DET_SRC_MODIFIED_START;
     }
     if ( length != det_src->pixel[ncl].length ) {
	  det_src->pixel[ncl].length &= mask_length;
	  stat &= DET_SRC_MODIFIED_LENGTH;
     }

     /* check if all fields in header are oke */
     nc = 0;
     do {
	  if ( clusDef[nc].chanID == chanID 
	       && clusDef[nc].clusID == clusID
	       && clusDef[nc].start == start
	       && clusDef[nc].length == length ) break;
     } while( ++nc < numClusDef );
     if ( nc < numClusDef ) return stat;

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
			   chanID, clusID, start, length, clusDef[nc].clusID );
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
			   chanID, clusID, start, length, clusDef[nc].chanID );
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
			   chanID, clusID, start, length, clusDef[nc].start );
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
			   chanID, clusID, start, length, clusDef[nc].length );
	  NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	  return DET_SRC_MODIFIED_LENGTH;
     }

     /* Oeps, can not correct inconsistent header fields */
     (void) snprintf( msg, SHORT_STRING_LENGTH, FMT_MSG_FAIL,
		      chanID, clusID, start, length );
     NADC_ERROR( prognm, NADC_ERR_NONE, msg );
     return DET_SRC_READ_FAILED;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_MDS_ANNOTATION
.PURPOSE     read level 0 MDS  annotations
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_MDS_ANNOTATION( cbuff, *isp, *fep_hdr );
     input:  
            char *cbuff              : pointer to annotation data
    output:  
	    struct mjd_envi *isp     : ISP sensing time
	    struct fep_hdr  *fep_hdr : FEP annotations

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
void SCIA_LV0_RD_MDS_ANNOTATION( const char *cbuff,
                                 /*@out@*/ struct mjd_envi *isp,
				 /*@out@*/ struct fep_hdr  *fep_hdr )
       /*@modifies isp, fep_hdr@*/
{
     register const char *hdr_pntr = cbuff;
/*
 * read ISP sensing time
 */
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
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_MDS_PACKET_HDR
.PURPOSE     read level 0 MDS ISP Packet header
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_MDS_PACKET_HDR( cbuff, *packet_hdr );
     input:  
            char *cbuff                   : pointer to packet header data
    output:  
	    struct packet_hdr *packet_hdr : Packet header

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
void SCIA_LV0_RD_MDS_PACKET_HDR( const char *cbuff,
                                 /*@out@*/ struct packet_hdr *packet_hdr )
       /*@modifies packet_hdr@*/
{
     register const char *cpntr = cbuff;
/*
 * copy data to output structure
 */
     (void) memcpy( &packet_hdr->api.two_byte, cpntr, ENVI_USHRT );
     cpntr += ENVI_USHRT;
     (void) memcpy( &packet_hdr->seq_cntrl, cpntr, ENVI_USHRT );
     cpntr += ENVI_USHRT;
     (void) memcpy( &packet_hdr->length, cpntr, ENVI_USHRT );

#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_PACKET_HDR( packet_hdr );
#endif
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_MDS_DATA_HDR
.PURPOSE     read Data Field Header of the Packet Data Field [ISP]
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_MDS_DATA_HDR( cbuff, data_hdr );
     input:  
            char *cbuff               : pointer to data header data
    output:  
            struct data_hdr *data_hdr : Data Field Header

.RETURNS     nothing
.COMMENTS    static function
             The Data Field Header for each of the three packets (i.e.
	     Detector, Auxiliary and PMD) contains standard information.
-------------------------*/
static inline
void SCIA_LV0_RD_MDS_DATA_HDR(  const char *cbuff,
                               /*@out@*/ struct data_hdr *data_hdr )
       /*@modifies *data_hdr@*/
{
     register const char *cpntr = cbuff;
/*
 * copy data to output structure
 */
     (void) memcpy( &data_hdr->length, cpntr, ENVI_USHRT );
     cpntr += ENVI_USHRT;
     (void) memcpy( &data_hdr->category, cpntr, ENVI_UCHAR );
     cpntr += ENVI_UCHAR;
     (void) memcpy( &data_hdr->state_id, cpntr, ENVI_UCHAR );
     cpntr += ENVI_UCHAR;
     (void) memcpy( &data_hdr->on_board_time, cpntr, ENVI_UINT );
     cpntr += ENVI_UINT;
     (void) memcpy( &data_hdr->rdv.two_byte, cpntr, ENVI_USHRT );
     cpntr += ENVI_USHRT;
     (void) memcpy( &data_hdr->id.two_byte, cpntr, ENVI_USHRT );

#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_DATA_HDR( data_hdr );
#endif
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_MDS_PMTC_HDR
.PURPOSE     read PMTC settings from the ISP Data Field Header
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_MDS_PMTC_HDR( cbuff, pmtc_hdr );
     input:  
            char *cbuff               : pointer to PMTC header data
    output:  
            struct pmtc_hdr *pmtc_hdr : PMTC settings

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void SCIA_LV0_RD_MDS_PMTC_HDR( const char *cbuff,
                               /*@out@*/ struct pmtc_hdr *pmtc_hdr )
       /*@modifies *pmtc_hdr@*/
{
     register const char *cpntr = cbuff;
/*
 * copy data to output structure
 */
     (void) memcpy( &pmtc_hdr->pmtc_1.two_byte, cpntr, ENVI_USHRT );
     cpntr += ENVI_USHRT;
     (void) memcpy( &pmtc_hdr->scanner_mode, cpntr, ENVI_USHRT );
     cpntr += ENVI_USHRT;
     (void) memcpy( &pmtc_hdr->az_param.four_byte, cpntr, ENVI_UINT );
     cpntr += ENVI_UINT;
     (void) memcpy( &pmtc_hdr->elv_param.four_byte, cpntr, ENVI_UINT );
     cpntr += ENVI_UINT;
     (void) memcpy( &pmtc_hdr->factor, cpntr, 6 * ENVI_UCHAR );

#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_PMTC_HDR( pmtc_hdr );
#endif
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_PMTC_FRAME
.PURPOSE     read source data of Auxilirary MDS
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_PMTC_FRAME( cbuff, pmtc_frame );
     input:  
            char *cbuff                   : pointer to data of PMTC frame
    output:  
            struct pmtc_frame *pmtc_frame : PMTC frame

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static inline
void SCIA_LV0_RD_MDS_PMTC_FRAME( const char *cbuff,
				 /*@out@*/ struct pmtc_frame *pmtc_frame )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, *pmtc_frame@*/
{
     const char prognm[] = "SCIA_LV0_RD_PMTC_FRAME";
     
     register unsigned short nb = 0;
     register const char *cpntr = cbuff;

     unsigned int ubuff;

     const unsigned short AUX_SYNC = 0xDDDD;
/*
 * initialize output structure to zero
 */
     (void) memset( pmtc_frame, 0, sizeof( struct pmtc_frame ));
/*
 * copy data to output structure
 */
     do {
	  (void) memcpy( &pmtc_frame->bcp[nb].sync, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;

          /* check data integrity */
	  if ( pmtc_frame->bcp[nb].sync != USHRT_ZERO
	       && pmtc_frame->bcp[nb].sync != AUX_SYNC ) {
	       NADC_RETURN_ERROR( prognm, NADC_WARN_PDS_RD, 
				  "incorrect AUX Sync value(s) found" );
	  }
	  (void) memcpy( &pmtc_frame->bcp[nb].bcps, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
	  (void) memcpy( &pmtc_frame->bcp[nb].flags.two_byte, 
			 cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
	  (void) memcpy( &ubuff, cpntr, ENVI_UINT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  ubuff = byte_swap_u32( ubuff );
#endif
	  pmtc_frame->bcp[nb].azi_encode_cntr = (ubuff >> 4) & 0xFFFFF;
	  cpntr += ENVI_USHRT;
	  (void) memcpy( &ubuff, cpntr, ENVI_UINT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  ubuff = byte_swap_u32( ubuff );
#endif
	  pmtc_frame->bcp[nb].ele_encode_cntr = (ubuff >> 4) & 0xFFFFF;
	  cpntr += ENVI_UINT;
	  (void) memcpy( &pmtc_frame->bcp[nb].azi_cntr_error, 
			 cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
	  (void) memcpy( &pmtc_frame->bcp[nb].ele_cntr_error, 
			 cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
	  (void) memcpy( &pmtc_frame->bcp[nb].azi_scan_error, 
			 cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
	  (void) memcpy( &pmtc_frame->bcp[nb].ele_scan_error, 
			 cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
     } while ( ++nb < NUM_LV0_AUX_BCP );

     (void) memcpy( &pmtc_frame->bench_rad.two_byte, cpntr, ENVI_USHRT );
     cpntr += ENVI_USHRT;
     (void) memcpy( &pmtc_frame->bench_elv.two_byte, cpntr, ENVI_USHRT );
     cpntr += ENVI_USHRT;
     (void) memcpy( &pmtc_frame->bench_az.two_byte, cpntr, ENVI_USHRT );
     cpntr += ENVI_USHRT;

#ifdef _SWAP_TO_LITTLE_ENDIAN
     Sun2Intel_MDS_PMTC_FRAME( pmtc_frame );
#endif
}

/*+++++++++++++++++++++++++
.IDENTifier   SCIA_LV0_RD_MDS_DET_SRC
.PURPOSE     read Source Data of Detector Data Packet
.INPUT/OUTPUT
  call as   nr_chan = SCIA_LV0_RD_MDS_DET_SRC( cbuff, det_length,
                                               chan_mask, num_chan, data_src );
     input:  
            char *cbuff               : 
            size_t det_length         : size of the detector data packet
                                          (in bytes)
	    unsigned char chan_mask   : mask for channel selection
            unsigned short num_chan   : number of channels
    output:  
            struct det_src *data_src  : Detector Source Packets

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
unsigned short SCIA_LV0_RD_MDS_DET_SRC( const char *cbuff, size_t det_length, 
					unsigned char chan_mask, 
					unsigned short num_chan,
					/*@out@*/ struct det_src *data_src )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, data_src@*/
{
     const char prognm[] = "SCIA_LV0_RD_MDS_DET_SRC";

     register unsigned short n_ch = 0;
     register unsigned short n_cl;

     register const char *cpntr = cbuff;

     unsigned short nr_chan = 0;

     bool           Band_Is_Selected;
     size_t         num_byte;
     unsigned short numClusters = 0;

     struct chan_src dummy_pixel[LV0_MX_CLUSTERS];

     const unsigned short CHANNEL_SYNC = 0xAAAA;
     const unsigned short CLUSTER_SYNC = 0xBBBB;
/*
 * copy data to output structure
 */
     do { 
	  (void) memcpy( &data_src->hdr.sync, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;

          /* check data integrity */
	  if ( data_src->hdr.sync != CHANNEL_SYNC )
	       NADC_GOTO_ERROR( prognm, NADC_WARN_PDS_RD, 
				"incorrect Channel Sync value found" );

	  (void) memcpy( &data_src->hdr.channel.two_byte, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
	  (void) memcpy( &data_src->hdr.bcps, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
	  (void) memcpy( &data_src->hdr.command_vis.four_byte, cpntr, 
			 ENVI_UINT );
	  (void) memcpy( &data_src->hdr.command_ir.four_byte, cpntr, 
			 ENVI_UINT );
	  cpntr += ENVI_UINT;
	  (void) memcpy( &data_src->hdr.ratio_hdr.two_byte, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
	  (void) memcpy( &data_src->hdr.bias, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
	  (void) memcpy( &data_src->hdr.temp, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_MDS_CHAN_HDR( &data_src->hdr );
#endif
	  numClusters = (unsigned short) data_src->hdr.channel.field.clusters;
	  Band_Is_Selected = 
	       SELECTED_CHANNEL( chan_mask, data_src->hdr.channel.field.id );
	  if ( Band_Is_Selected && numClusters > 0 ) {
	       nr_chan++;
	       data_src->pixel = (struct chan_src *)
		    malloc ( numClusters * sizeof( struct chan_src ) );
	       if ( data_src->pixel == NULL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "pixel" );
	  } else {
	       data_src->pixel = dummy_pixel;
	  }
/*
 * read data of the clusters
 */
	  n_cl = 0;
	  do {
	       (void) memcpy( &data_src->pixel[n_cl].sync, cpntr, 
			      ENVI_USHRT );
	       cpntr += ENVI_USHRT;

               /* check data integrity */
	       if ( data_src->pixel[n_cl].sync != CLUSTER_SYNC ) {
		    if ( Band_Is_Selected && numClusters > 0 ) {
			 while ( n_cl > 0 )
			      free( data_src->pixel[--n_cl].data );
			 free( data_src->pixel );
			 data_src->hdr.channel.field.clusters = 0;
		    } else 
			 data_src->pixel = NULL;
		    NADC_GOTO_ERROR( prognm, NADC_WARN_PDS_RD, 
				 "corrupted cluster Sync, remainder skipped" );
	       }
	       /* cluster block identifier */
	       (void) memcpy( &data_src->pixel[n_cl].block_nr, cpntr, 
			      ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       data_src->pixel[n_cl].block_nr = 
		    byte_swap_u16( data_src->pixel[n_cl].block_nr );
#endif
	       cpntr += ENVI_USHRT;
	       /* cluster identifier */
	       (void) memcpy( &data_src->pixel[n_cl].cluster_id, cpntr,
			      ENVI_UCHAR );
	       cpntr += ENVI_UCHAR;
	       /* co-adding indicator */
	       (void) memcpy( &data_src->pixel[n_cl].co_adding, cpntr, 
			      ENVI_UCHAR );
	       cpntr += ENVI_UCHAR;
	       /* start pixel indicator */
	       (void) memcpy( &data_src->pixel[n_cl].start, cpntr, ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       data_src->pixel[n_cl].start = 
		    byte_swap_u16( data_src->pixel[n_cl].start );
#endif
	       cpntr += ENVI_USHRT;
	       /* cluster block length */
	       (void) memcpy( &data_src->pixel[n_cl].length, cpntr, 
			      ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       data_src->pixel[n_cl].length = 
		    byte_swap_u16( data_src->pixel[n_cl].length );
#endif
	       cpntr += ENVI_USHRT;

               /* check for corrupted cluster, and try to correct them */
	       if ( ClusterCorrectionFlag && numClusDef != 0 ) {
		    unsigned char stat = _CHECK_CLUSTERDEF( n_cl, data_src );

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
				   cpntr, num_byte );
	       }
	       cpntr += num_byte;
	       
	  } while( ++n_cl < numClusters );

	  if ( Band_Is_Selected ) data_src++;
     } while ( ++n_ch < num_chan );
/*
 * check number of bytes read
 */
     if ( (size_t)(cpntr - cbuff) != det_length ) {
	  char msg[80];
	  (void) snprintf( msg, 80, 
			   "Detector[%-hu] MDS size: %zd instead of %zd", 
			   numClusters, (size_t)(cpntr - cbuff), 
			   det_length );
	  NADC_GOTO_ERROR( prognm, NADC_WARN_PDS_RD, msg );
     }
/*
 * release allocated memory
 */
 done:
     return nr_chan;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_RD_PMD_SRC
.PURPOSE     read source data of PMD MDS
.INPUT/OUTPUT
  call as   SCIA_LV0_RD_PMD_SRC( cbuff, pmd_src );
     input:  
            char *cbuff             : pointer to data of PMD data packet
    output:  
            struct pmd_src *pmd_src : structure for PMD Source data

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void SCIA_LV0_RD_MDS_PMD_SRC( const char *cbuff, 
			      /*@out@*/ struct pmd_src *pmd_src )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, *pmd_src@*/
{
     const char prognm[]  = "SCIA_LV0_RD_MDS_PMD_SRC";

     register unsigned short np;
     register const char *cpntr = cbuff;

     const size_t nr_byte = 2 * PMD_NUMBER * ENVI_USHRT;

     const unsigned short PMD_SYNC = 0xEEEE;
/*
 * initialize output structure to zero
 */
     (void) memset( pmd_src, 0, sizeof(struct pmd_src) );
/*
 * copy data to output structure
 */
     (void) memcpy( &pmd_src->temp, cpntr, ENVI_USHRT );
     cpntr += ENVI_USHRT;

     np = 0;
     do {
	  (void) memcpy( &pmd_src->packet[np].sync, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;

          /* check data integrity */
	  if ( pmd_src->packet[np].sync != USHRT_ZERO
	       && pmd_src->packet[np].sync != PMD_SYNC ) {
	       NADC_RETURN_ERROR( prognm, NADC_WARN_PDS_RD, 
				  "incorrect PMD Sync value(s) found" );
	  }
	  (void) memcpy( pmd_src->packet[np].data, cpntr, nr_byte );
	  cpntr += nr_byte;
	  (void) memcpy( &pmd_src->packet[np].bcps, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
	  (void) memcpy( &pmd_src->packet[np].time.two_byte, cpntr, 
			   ENVI_USHRT );
	  cpntr += ENVI_USHRT;
     } while ( ++np < NUM_LV0_PMD_PACKET );

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

     const size_t lv0_aux_dsr_size = LV0_ANNOTATION_LENGTH 
	  + LV0_PACKET_HDR_LENGTH + LV0_DATA_HDR_LENGTH + LV0_PMTC_HDR_LENGTH
	  + NUM_LV0_AUX_PMTC_FRAME * AUX_DATA_SRC_LENGTH;

     register unsigned short nf;
     register char *cpntr;

     char cbuff[lv0_aux_dsr_size];
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) info->offset, SEEK_SET );
/*
 * read Auxiliary DSR from file as a character array
 */
     if ( fread( cbuff, lv0_aux_dsr_size, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "LV0_AUX_DSR" );
     cpntr = cbuff;
/*
 * read Annotation (ISP, FEP)
 */
     SCIA_LV0_RD_MDS_ANNOTATION( cpntr, &aux->isp, &aux->fep_hdr );
     cpntr += LV0_ANNOTATION_LENGTH;
/*
 * read ISP Packet Header
 */
     SCIA_LV0_RD_MDS_PACKET_HDR( cpntr, &aux->packet_hdr );
     cpntr += LV0_PACKET_HDR_LENGTH;
/*
 * read ISP Data Field Header
 */
     SCIA_LV0_RD_MDS_DATA_HDR( cpntr, &aux->data_hdr );
     cpntr += LV0_DATA_HDR_LENGTH;
/*
 * check packet ID, state ID & on-board time
 */
     if ( info->q.flag.packet_id == 1 )
	  aux->data_hdr.id.field.packet = SCIA_AUX_PACKET;

     if ( info->q.flag.state_id == 1 )
	  aux->data_hdr.state_id = info->state_id;

     if ( info->q.flag.on_board_time == 1 )
	  aux->data_hdr.on_board_time = info->on_board_time;
/*
 * read PMTC settings
 */
     SCIA_LV0_RD_MDS_PMTC_HDR( cpntr, &aux->pmtc_hdr );
     cpntr += LV0_PMTC_HDR_LENGTH;
/*
 * read ISP Auxiliary data source packet
 */
     nf = 0;
     do {
	  SCIA_LV0_RD_MDS_PMTC_FRAME( cpntr, &aux->data_src[nf] );
	  cpntr += AUX_DATA_SRC_LENGTH;
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

     const size_t lv0_det_dsr_size = LV0_ANNOTATION_LENGTH 
	  + LV0_PACKET_HDR_LENGTH + LV0_DATA_HDR_LENGTH + LV0_PMTC_HDR_LENGTH
	  + ENVI_USHRT + 8 * ENVI_INT + ENVI_USHRT;

     register char *cpntr;

     char cbuff[lv0_det_dsr_size];
     char *cdet;
     size_t det_length;
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) info->offset, SEEK_SET );
/*
 * read PMD DSR from file as a character array
 */
     if ( fread( cbuff, lv0_det_dsr_size, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "LV0_DET_HDR" );
     cpntr = cbuff;
/*
 * read Annotation (ISP, FEP)
 */
     SCIA_LV0_RD_MDS_ANNOTATION( cpntr, &det->isp, &det->fep_hdr );
     cpntr += LV0_ANNOTATION_LENGTH;
/*
 * read Packet Header
 */
     SCIA_LV0_RD_MDS_PACKET_HDR( cpntr, &det->packet_hdr );
     cpntr += LV0_PACKET_HDR_LENGTH;
/*
 * read ISP Data Field Header
 */
     SCIA_LV0_RD_MDS_DATA_HDR( cpntr, &det->data_hdr );
     cpntr += LV0_DATA_HDR_LENGTH;
/*
 * check packet ID, state ID & on-board time
 */
     if ( info->q.flag.packet_id == 1 )
	  det->data_hdr.id.field.packet = SCIA_DET_PACKET;

     if ( info->q.flag.state_id == 1 )
	  det->data_hdr.state_id = info->state_id;

     if ( info->q.flag.on_board_time == 1 )
	  det->data_hdr.on_board_time = info->on_board_time;
/*
 * read Broadcast counter (MDI)
 */
     (void) memcpy( &det->bcps, cpntr, ENVI_USHRT );
     cpntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     det->bcps = byte_swap_u16( det->bcps );
#endif
/*
 * read PMTC settings
 */
     SCIA_LV0_RD_MDS_PMTC_HDR( cpntr, &det->pmtc_hdr );
     cpntr += LV0_PMTC_HDR_LENGTH;
/*
 * read remaining variables from Data Field Header for the Detector Data Packet
 */
     (void) memcpy( &det->orbit_vector, cpntr, 8 * ENVI_INT );
     cpntr += 8 * ENVI_INT;
     (void) memcpy( &det->num_chan, cpntr, ENVI_USHRT );
     cpntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
     det->orbit_vector[0] = byte_swap_32( det->orbit_vector[0] );
     det->orbit_vector[1] = byte_swap_32( det->orbit_vector[1] );
     det->orbit_vector[2] = byte_swap_32( det->orbit_vector[2] );
     det->orbit_vector[3] = byte_swap_32( det->orbit_vector[3] );
     det->orbit_vector[4] = byte_swap_32( det->orbit_vector[4] );
     det->orbit_vector[5] = byte_swap_32( det->orbit_vector[5] );
     det->orbit_vector[6] = byte_swap_32( det->orbit_vector[6] );
     det->orbit_vector[7] = byte_swap_32( det->orbit_vector[7] );
     det->num_chan = byte_swap_u16( det->num_chan );
#endif
/*
 * read ISP Detector Data Source Packet
 */
     det_length = (size_t) (det->packet_hdr.length - DET_DATA_HDR_LENGTH + 1);
     if ( (cdet = (char *) malloc( det_length )) == NULL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "cdet" );
     if ( fread( cdet, det_length, 1, fd ) != 1 ) {
	  free( cdet );
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_RD, "cdet" );
     }
     if ( ! Use_Extern_Alloc ) {
	  det->data_src = (struct det_src *) 
	       malloc( det->num_chan * sizeof( struct det_src ) );
     }
     if ( det->data_src == NULL ) {
	  free( cdet );
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "det_src" );
     }
     det->num_chan = SCIA_LV0_RD_MDS_DET_SRC( cdet, det_length, chan_mask, 
					      det->num_chan, det->data_src );
     free( cdet );
     if ( IS_ERR_STAT_WARN ) {
	  char msg[SHORT_STRING_LENGTH];
	  (void) snprintf( msg, SHORT_STRING_LENGTH,
			   "Read failed for on_board_time %-u state_id %hhu",
			   info->on_board_time, info->state_id );
	  nadc_stat &= ~(NADC_STAT_WARN);
	  NADC_RETURN_ERROR( prognm, NADC_ERR_NONE, msg );
     }
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
static
void SCIA_LV0_RD_ONE_PMD( FILE *fd, const struct mds0_info *info,
			  struct mds0_pmd *pmd )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, pmd@*/
{
     const char prognm[] = "SCIA_LV0_RD_ONE_PMD";

     const size_t lv0_pmd_dsr_size = LV0_ANNOTATION_LENGTH 
	  + LV0_PACKET_HDR_LENGTH + LV0_DATA_HDR_LENGTH + PMD_DATA_SRC_LENGTH;

     register char *cpntr;

     char cbuff[lv0_pmd_dsr_size];
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) info->offset, SEEK_SET );
/*
 * read PMD DSR from file as a character array
 */
     if ( fread( cbuff, lv0_pmd_dsr_size, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "LV0_PMD_DSR" );
     cpntr = cbuff;
/*
 * read Annotation (ISP, FEP)
 */
     SCIA_LV0_RD_MDS_ANNOTATION( cpntr, &pmd->isp, &pmd->fep_hdr );
     cpntr += LV0_ANNOTATION_LENGTH;
/*
 * read Packet Header
 */
     SCIA_LV0_RD_MDS_PACKET_HDR( cpntr, &pmd->packet_hdr );
     cpntr += LV0_PACKET_HDR_LENGTH;
/*
 * read ISP Data Field Header
 */
     SCIA_LV0_RD_MDS_DATA_HDR( cpntr, &pmd->data_hdr );
     cpntr += LV0_DATA_HDR_LENGTH;
/*
 * check packet ID, state ID, on-board time
 */
     if ( info->q.flag.packet_id == 1 )
	  pmd->data_hdr.id.field.packet = SCIA_PMD_PACKET;

     if ( info->q.flag.state_id == 1 )
	  pmd->data_hdr.state_id = info->state_id;

     if ( info->q.flag.on_board_time == 1 )
	  pmd->data_hdr.on_board_time = info->on_board_time;
/*
 * read ISP PMD data source packet
 */
     SCIA_LV0_RD_MDS_PMD_SRC( cpntr, &pmd->data_src );
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

     register unsigned int nr_det = 0;

     struct mds0_det *det;

     if ( num_info == 0 ) {
	  *det_out = NULL;
	  return 0u;
     }
/*
 * set flag which indicates that Detector MDS records have to be checked
 */
     SET_NO_CLUSTER_CORRECTION();

     /* obtain cluster definition */
     if ( ClusterCorrectionFlag ) {
//	  register unsigned short jj;
	  
	  struct mph_envi  mph;

          ENVI_RD_MPH( fd, &mph );
	  numClusDef = CLUSDEF_CLCON( info->state_id, mph.abs_orbit, clusDef );
//	  (void) fprintf( stderr, " %d %d %d %d\n", mph.abs_orbit,
//			  (int) info->state_id, (int) ClusterCorrectionFlag,
//			  (int) numClusDef );
//	  for ( jj = 0; jj < numClusDef; jj++ ) {
//	       (void) fprintf( stderr, "%2hhu %2hhu %4hu %4hu\n",
//			       clusDef[jj].chanID, clusDef[jj].clusID,
//			       clusDef[jj].start, clusDef[jj].length );
//	  }
     }
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

     const size_t lv0_aux_dsr_size = LV0_PACKET_HDR_LENGTH 
	  + LV0_DATA_HDR_LENGTH + LV0_PMTC_HDR_LENGTH
	  + NUM_LV0_AUX_PMTC_FRAME * AUX_DATA_SRC_LENGTH;

     register unsigned short nf;
     register char *cpntr;

     char cbuff[lv0_aux_dsr_size];
/*
 * read Auxiliary DSR from file as a character array
 */
     if ( fread( cbuff, lv0_aux_dsr_size, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "LV0_AUX_DSR" );
     cpntr = cbuff;
/*
 * read Packet Header
 */
     SCIA_LV0_RD_MDS_PACKET_HDR( cpntr, &aux->packet_hdr );
     cpntr += LV0_PACKET_HDR_LENGTH;
/*
 * read ISP data field header
 */
     SCIA_LV0_RD_MDS_DATA_HDR( cpntr, &aux->data_hdr );
     cpntr += LV0_DATA_HDR_LENGTH;
/*
 * check packet ID
 */
     if ( (int) aux->data_hdr.id.field.packet != SCIA_AUX_PACKET )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, 
			     "not an Auxiliary data packet" );
/*
 * read PMTC settings
 */
     SCIA_LV0_RD_MDS_PMTC_HDR( cpntr, &aux->pmtc_hdr );
     cpntr += LV0_PMTC_HDR_LENGTH;
/*
 * read ISP AUX data source packet
 */
     nf = 0;
     do {
	  SCIA_LV0_RD_MDS_PMTC_FRAME( cpntr, &aux->data_src[nf] );
	  cpntr += AUX_DATA_SRC_LENGTH;
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

     const size_t lv0_pmd_dsr_size = LV0_PACKET_HDR_LENGTH
	  + LV0_DATA_HDR_LENGTH + PMD_DATA_SRC_LENGTH;

     register char *cpntr;

     char cbuff[lv0_pmd_dsr_size];
/*
 * read PMD DSR from file as a character array
 */
     if ( fread( cbuff, lv0_pmd_dsr_size, 1, fd ) != 1 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "LV0_PMD_DSR" );
     cpntr = cbuff;
/*
 * read Packet Header
 */
     SCIA_LV0_RD_MDS_PACKET_HDR( cpntr, &pmd->packet_hdr);
     cpntr += LV0_PACKET_HDR_LENGTH;
/*
 * read ISP data field header
 */
     SCIA_LV0_RD_MDS_DATA_HDR( cpntr, &pmd->data_hdr );
     cpntr += LV0_DATA_HDR_LENGTH;
/*
 * check packet ID
 */
     if ( (int) pmd->data_hdr.id.field.packet != SCIA_PMD_PACKET )
 	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "not a PMD data packet" );
/*
 * read ISP PMD data source packet
 */
     SCIA_LV0_RD_MDS_PMD_SRC( cpntr, &pmd->data_src );
}
