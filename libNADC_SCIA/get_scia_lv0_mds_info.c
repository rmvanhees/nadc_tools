/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2009 - 2013 SRON (R.M.van.Hees@sron.nl)

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
.VERSION     2.0     13-NOV-2012   added cluster checking, RvH
             1.1     16-FEB-2010   added stricter validity checking, RvH
             1.0     28-OCT-2009   initial release by R. M. van Hees
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
#include <limits.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_0
#include <nadc_scia.h>
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>
#endif

/*+++++ Macros +++++*/
#define CMP_MJD(a,b) memcmp( &(a), &(b), sizeof(struct mjd_envi))

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
#define OFFS_AUX_SYNC   68
#define OFFS_DET_SYNC   104
#define OFFS_PMD_SYNC   52

#define FMT_DET_CHAN_SYNC \
     "DET package (%s bcps=%-hu) Channel Sync corrupted - %hu"
#define FMT_DET_CLUS_SYNC \
     "DET package (%s bcps=%-hu) Cluster Sync corrupted - %hhu"
#define FMT_DET_TOO_LARGE \
     "DET package (%s bcps=%-hu) too large - %hhu"

static const unsigned short AUX_SYNC = 0xDDDD;
static const unsigned short CHANNEL_SYNC = 0xAAAA;
static const unsigned short CLUSTER_SYNC = 0xBBBB;
static const unsigned short PMD_SYNC = 0xEEEE;


static unsigned short numClus = 0;
static bool ClusterCorrectionFlag = TRUE;
static /*@null@*/ const struct clusdef_rec *ClusterDef = NULL;

#define NCLUSDEF_ONE     40
static
const struct clusdef_rec ClusDefOne[NCLUSDEF_ONE] = {
     {1, 0, 0, 5}, {1, 1, 5, 192}, {1, 2, 197, 355}, {1, 3, 552, 290},
     {1, 4, 842, 177}, {1, 5, 1019, 5}, {2, 0, 1024, 5}, {2, 1, 1029, 71},
     {2, 2, 1100, 778}, {2, 3, 1878, 94}, {2, 4, 1972, 71}, {2, 5, 2043, 5},
     {3, 0, 2048, 10}, {3, 1, 2058, 23}, {3, 2, 2081, 897}, {3, 3, 2978, 89},
     {3, 4, 3067, 5}, {4, 0, 3072, 5}, {4, 1, 3077, 5}, {4, 2, 3082, 909},
     {4, 3, 3991, 100}, {4, 4, 4091, 5}, {5, 0, 4096, 5}, {5, 1, 4101, 5},
     {5, 2, 4106, 991}, {5, 3, 5097, 18}, {5, 4, 5115, 5}, {6, 0, 5120, 10},
     {6, 1, 5130, 14}, {6, 2, 5144, 973}, {6, 3, 6117, 17}, {6, 4, 6134, 10},
     {7, 0, 6144, 10}, {7, 1, 6154, 38}, {7, 2, 6192, 940}, {7, 3, 7132,26 },
     {7, 4, 7158, 10}, {8, 0, 7168, 10}, {8, 1, 7178, 1004}, {8, 2, 8182, 10}
};

#define NCLUSDEF_THREE   56
static
const struct clusdef_rec ClusDefThree[NCLUSDEF_THREE] = {
     {1, 0, 0, 5}, {1, 1, 5, 192}, {1, 2, 197, 355}, {1, 3, 552, 196},
     {1, 4, 748, 94}, {1, 5, 1019, 5}, {2, 0, 1024, 5}, {2, 1, 1100, 114},
     {2, 2, 1214, 664}, {2, 3, 1878, 94}, {2, 4, 2043, 5}, {3, 0, 2048, 10},
     {3, 1, 2081, 50}, {3, 2, 2131, 80}, {3, 3, 2211, 436}, {3, 4, 2647, 75},
     {3, 5, 2722, 87}, {3, 6, 2809, 135}, {3, 7, 2944, 34}, {3, 8, 3067, 5},
     {4, 0, 3072, 5}, {4, 1, 3082, 36}, {4, 2, 3118, 32}, {4, 3, 3150, 535},
     {4, 4, 3685, 134}, {4, 5, 3819, 106}, {4, 6, 3925, 66}, {4, 7, 4091, 5},
     {5, 0, 4096, 5}, {5, 1, 4106, 46}, {5, 2, 4152, 28}, {5, 3, 4180, 525},
     {5, 4, 4705, 158}, {5, 5, 4863, 234}, {5, 6, 5115, 5}, {6, 0, 5120, 10},
     {6, 1, 5144, 83}, {6, 2, 5227, 228}, {6, 3, 5455, 26}, {6, 4, 5481, 178},
     {6, 5, 5659, 28}, {6, 6, 5687, 179}, {6, 7, 5866, 154}, {6, 8, 6020, 31},
     {6, 9, 6051, 14}, {6, 10, 6065, 52}, {6, 11, 6134, 10}, {7, 0, 6144, 10},
     {7, 1, 6192, 245}, {7, 2, 6437, 148}, {7, 3, 6585, 442}, {7, 4, 7027,105},
     {7, 5, 7158, 10}, {8, 0, 7168, 10}, {8, 1, 7178, 1004}, {8, 2, 8182, 10}
};


static const size_t InfoClusSize = MAX_CLUSTER * sizeof( struct info_clus );

static struct mjd_envi mjd_first, mjd_last;

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static inline
bool MJD_BEFORE_SENSING_START( const struct mjd_envi mjd )
{
     if ( mjd.days < mjd_first.days ) return TRUE;
     if ( mjd.days == mjd_first.days ) {
	  if ( mjd.secnd < mjd_first.secnd ) return TRUE;
	  if ( mjd.secnd == mjd_first.secnd 
	       && mjd.musec < mjd_first.musec ) return TRUE;
     }
     return FALSE;
}
 
static inline
bool MJD_AFTER_SENSING_STOP( const struct mjd_envi mjd )
{
     if ( mjd.days > mjd_last.days ) return TRUE;
     if ( mjd.days == mjd_last.days ) {
	  if ( mjd.secnd > mjd_last.secnd ) return TRUE;
	  if ( mjd.secnd == mjd_last.secnd 
	       && mjd.musec > mjd_last.musec ) return TRUE;
     }
     return FALSE;
}
 
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

static inline
unsigned short GET_CLUSTERDEF( unsigned char stateID )
       /*@globals  ClusterDef;@*/
       /*@modifies ClusterDef@*/
{
     const char prognm[] = "GET_CLUSTERDEF";

     const unsigned short ClusDefIndx[MAX_NUM_STATE] = {
          3, 3, 3, 3, 3, 3, 3, 1, 3, 3,
          3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
          1, 1, 3, 3, 3, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 3, 1, 1,
          1, 3, 3, 3, 3, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1
     };

     if ( stateID == UCHAR_ZERO || stateID > (unsigned char) MAX_NUM_STATE ) {
          NADC_ERROR( prognm, NADC_ERR_FATAL, "state ID out-of-range" );
          ClusterDef = NULL;
          return USHRT_ZERO;
     }
     switch ( ClusDefIndx[stateID-1] ) {
     case ((unsigned short) 1 ): {
          ClusterDef = ClusDefOne;
          return NCLUSDEF_ONE;
     }
     case ((unsigned short) 3 ): {
          ClusterDef = ClusDefThree;
          return NCLUSDEF_THREE;
     }
     default: {
          ClusterDef = NULL;
          return USHRT_ZERO;
     }}
}

/*+++++++++++++++++++++++++
.IDENTifer   CHECK_CLUSTERDEF
.PURPOSE     Check and (if necessary) correct cluster parameters
.INPUT/OUTPUT
  call as   stat = CHECK_CLUSTERDEF( det_src );
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
unsigned char CHECK_CLUSTERDEF( struct info_clus *info_clus )
       /*@globals  nadc_stat, nadc_err_stack, numClus, ClusterDef;@*/
       /*@modifies nadc_stat, nadc_err_stack, info_clus@*/
{
     const char prognm[] = "CHECK_CLUSTERDEF";

     register unsigned short nc;

     char msg[SHORT_STRING_LENGTH];

     /* perform check on request or when a memeory corruption is obvious */
     if ( ! ClusterCorrectionFlag && info_clus->length <= CHANNEL_SIZE ) 
	  return DET_SRC_MODIFIED_NONE;

     /* check if all fields in header are oke */
     nc = 0;
     do {
	  if ( ClusterDef[nc].chanID == info_clus->chanID 
	       && ClusterDef[nc].clusID == info_clus->clusID
	       && ClusterDef[nc].start == info_clus->start
	       && ClusterDef[nc].length == info_clus->length ) break;
     } while( ++nc < numClus );
     if ( nc < numClus ) return DET_SRC_MODIFIED_NONE;

     /* check if 3 fields in header are oke, except "clusID" */
     nc = 0;
     do {
	  if ( ClusterDef[nc].chanID == info_clus->chanID 
	       && ClusterDef[nc].start == info_clus->start
	       && ClusterDef[nc].length == info_clus->length ) break;
     } while( ++nc < numClus );
     if ( nc < numClus ) {
	  (void) snprintf( msg, SHORT_STRING_LENGTH, FMT_MSG_CLUSID, 
			   info_clus->clusID, ClusterDef[nc].clusID );
	  NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	  info_clus->clusID = ClusterDef[nc].clusID;
	  return DET_SRC_MODIFIED_CLUSID;
     }

     /* check if 3 fields in header are oke, except "chanID" */
     nc = 0;
     do {
	  if ( ClusterDef[nc].clusID == info_clus->clusID
	       && ClusterDef[nc].start == info_clus->start
	       && ClusterDef[nc].length == info_clus->length ) break;
     } while( ++nc < numClus );
     if ( nc < numClus ) {
	  (void) snprintf( msg, SHORT_STRING_LENGTH, FMT_MSG_CHANID, 
			   info_clus->chanID, ClusterDef[nc].chanID );
	  NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	  info_clus->chanID = ClusterDef[nc].chanID;
	  return DET_SRC_MODIFIED_CHANID;
     }

     /* check if 3 fields in header are oke, except "start" */
     nc = 0;
     do {
	  if ( ClusterDef[nc].chanID == info_clus->chanID 
	       && ClusterDef[nc].clusID == info_clus->clusID
	       && ClusterDef[nc].length == info_clus->length ) break;
     } while( ++nc < numClus );
     if ( nc < numClus ) {
	  (void) snprintf( msg, SHORT_STRING_LENGTH, FMT_MSG_START, 
			   info_clus->start, ClusterDef[nc].start );
	  NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	  info_clus->start = ClusterDef[nc].start;
	  return DET_SRC_MODIFIED_START;
     }

     /* check if 3 fields in header are oke, except "length" */
     nc = 0;
     do {
	  if ( ClusterDef[nc].chanID == info_clus->chanID 
	       && ClusterDef[nc].clusID == info_clus->clusID
	       && ClusterDef[nc].start == info_clus->start ) break;
     } while( ++nc < numClus );
     if ( nc < numClus ) {
	  (void) snprintf( msg, SHORT_STRING_LENGTH, FMT_MSG_LENGTH,
			   info_clus->length, ClusterDef[nc].length );
	  NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	  info_clus->length = ClusterDef[nc].length;
	  return DET_SRC_MODIFIED_LENGTH;
     }

     /* Oeps, can not correct inconsistent header fields */
     (void) snprintf( msg, SHORT_STRING_LENGTH, FMT_MSG_FAIL );
     NADC_ERROR( prognm, NADC_ERR_NONE, msg );
     return DET_SRC_READ_FAILED;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_INFO_READ_AUX
.PURPOSE     obtain MDS info of a SCIA level 0 Auxiliary package
.INPUT/OUTPUT
  call as   nr_bytes = SCIA_LV0_INFO_READ_AUX( mds_pntr, info );
     input:  
            char *mds_pntr         : source package buffer
 in/output:  
	    struct mds0_info *info : structure holding info about MDS records

.RETURNS     number of bytes read from buffer, 
             error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
size_t SCIA_LV0_INFO_READ_AUX( const char *mds_pntr, struct mds0_info *info )
{
     const char prognm[] = "SCIA_LV0_INFO_READ_AUX";

     register const char     *cpntr  = mds_pntr;
     register unsigned short num_aux = 0;
     register unsigned short num_sync = 0;

     unsigned short aux_sync;

     /* initialisation */
     info->bcps = 0;
     info->stateIndex = 0;
     info->numClusters = UCHAR_ZERO;
     (void) memset( info->cluster, 0, InfoClusSize );

     /* ISP */
     (void) memcpy( &info->mjd, cpntr, sizeof(struct mjd_envi) );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     info->mjd.days = byte_swap_32( info->mjd.days );
     info->mjd.secnd = byte_swap_u32( info->mjd.secnd );
     info->mjd.musec = byte_swap_u32( info->mjd.musec );
#endif
     cpntr += LV0_ANNOTATION_LENGTH;                /* move to Packet Header */

     /* Packet Header */
     cpntr += 2 * ENVI_USHRT;      /* skip Identification & Sequence Control */
     (void) memcpy( &info->length, cpntr, ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     info->length = byte_swap_u16( info->length );
#endif
     cpntr += ENVI_USHRT;                       /* move to Data Field Header */

     /* Data Header */
     cpntr += ENVI_USHRT;                   /* skip Data Field Header Length */
     (void) memcpy( &info->category, cpntr, ENVI_UCHAR );
     cpntr += ENVI_UCHAR;
     (void) memcpy( &info->stateID, cpntr, ENVI_UCHAR );
     cpntr += ENVI_UCHAR;
     cpntr += 3 * ENVI_USHRT;     /* skip ICU & Redundancy Definition Vector */
     (void) memcpy( &info->packetID, cpntr, ENVI_UCHAR );
     info->packetID = info->packetID >> 4;
     cpntr += ENVI_USHRT;
     cpntr += LV0_PMTC_HDR_LENGTH;                     /* skip PMTC settings */
#ifdef DEBUG
     (void) fprintf( stderr, "AUX: %3hhu %3hhu %5d %5u %6u\n", info->stateID, 
		     info->packetID, info->mjd.days,
		     info->mjd.secnd, info->mjd.musec );
#endif

     /* validity check */
     if ( info->packetID != SCIA_AUX_PACKET 
	  || info->stateID == 0 || info->stateID > 70
	  || MJD_BEFORE_SENSING_START( info->mjd ) 
	  || MJD_AFTER_SENSING_STOP( info->mjd ) ) {
 	  NADC_ERROR( prognm, NADC_ERR_NONE, "MDS_AUX validity check Failure" );
	  return 0;
     }

     /* read source packages */
     (void) memcpy( &info->bcps, cpntr+ENVI_USHRT, ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     info->bcps = byte_swap_u16( info->bcps );
#endif
     do {
	  register unsigned short np = 0;

	  do {
	       (void) memcpy( &aux_sync, cpntr, ENVI_USHRT );
	       if ( aux_sync == AUX_SYNC || aux_sync == USHRT_ZERO ) num_sync++;

	       cpntr += 10 * ENVI_USHRT;           /* skip PMTC frame */
	  } while ( ++np < NUM_LV0_AUX_BCP );
	  cpntr += 3 * ENVI_USHRT;                 /* skip Bench temperatures */
     } while ( ++num_aux < NUM_LV0_AUX_PMTC_FRAME ); 

     if ( num_sync < (NUM_LV0_AUX_BCP * NUM_LV0_AUX_PMTC_FRAME) ){
	  char msg[SHORT_STRING_LENGTH];
	  char  date_str[UTC_STRING_LENGTH];

	  (void) MJD_2_ASCII( info->mjd.days, info->mjd.secnd, 
			      info->mjd.musec, date_str );
	  (void) snprintf( msg, SHORT_STRING_LENGTH,
			   "AUX package (%s bcps=%-hu) incomplete - %hu", 
			   date_str, info->bcps, num_sync );
	  NADC_ERROR( prognm, NADC_ERR_NONE, msg );
     }
     return (cpntr - mds_pntr);
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_INFO_READ_DET
.PURPOSE     obtain MDS info of a SCIA level 0 Detector package
.INPUT/OUTPUT
  call as   nr_bytes = SCIA_LV0_INFO_READ_DET( mds_pntr, info );
     input:  
            char *mds_pntr         : source package buffer
 in/output:  
	    struct mds0_info *info : structure holding info about MDS records

.RETURNS     number of bytes read from buffer, 
             error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
size_t SCIA_LV0_INFO_READ_DET( const char *mds_pntr, struct mds0_info *info )
{
     const char prognm[] = "SCIA_LV0_INFO_READ_DET";

     register const char     *cpntr  = mds_pntr;
     register unsigned char  nc;
     register unsigned short nch, ncl;

     unsigned short num_chan;
     unsigned short chan_sync, clus_sync;

     union
     {
	  struct
	  {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char lu:2;
	       unsigned char is:2;
	       unsigned char id:4;
	       unsigned char clusters:8;
#else
	       unsigned char id:4;
	       unsigned char is:2;
	       unsigned char lu:2;
	       unsigned char clusters:8;
#endif
	  } field;
	  unsigned short two_byte;
     } channel;

     /* initialisation */
     info->bcps = 0;
     info->stateIndex = 0;
     info->numClusters = UCHAR_ZERO;
     (void) memset( info->cluster, 0, InfoClusSize );

     /* ISP */
     (void) memcpy( &info->mjd, cpntr, sizeof(struct mjd_envi) );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     info->mjd.days = byte_swap_32( info->mjd.days );
     info->mjd.secnd = byte_swap_u32( info->mjd.secnd );
     info->mjd.musec = byte_swap_u32( info->mjd.musec );
#endif
     cpntr += LV0_ANNOTATION_LENGTH;                /* move to Packet Header */

     /* Packet Header */
     cpntr += 2 * ENVI_USHRT;      /* skip Identification & Sequence Control */
     (void) memcpy( &info->length, cpntr, ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     info->length = byte_swap_u16( info->length );
#endif
     cpntr += ENVI_USHRT;                       /* move to Data Field Header */

     /* Data Header */
     cpntr += ENVI_USHRT;                   /* skip Data Field Header Length */
     (void) memcpy( &info->category, cpntr, ENVI_UCHAR );
     cpntr += ENVI_UCHAR;
     (void) memcpy( &info->stateID, cpntr, ENVI_UCHAR );
     cpntr += ENVI_UCHAR;
     cpntr += 3 * ENVI_USHRT;     /* skip ICU & Redundancy Definition Vector */
     (void) memcpy( &info->packetID, cpntr, ENVI_UCHAR );
     info->packetID = info->packetID >> 4;
     cpntr += ENVI_USHRT;

     /* skip Detector Data Field Header */
     cpntr += (DET_DATA_HDR_LENGTH - LV0_DATA_HDR_LENGTH);
     (void) memcpy( &num_chan, cpntr-ENVI_USHRT, ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     num_chan = byte_swap_u16( num_chan );
#endif
#ifdef DEBUG
     (void) fprintf( stdout, "DET: %3hhu %3hhu %5d %5u %6u %2hu\n", 
		     info->stateID, info->packetID, info->mjd.days,
		     info->mjd.secnd, info->mjd.musec, num_chan);
#endif
     /* validity check */
     if ( info->packetID != SCIA_DET_PACKET
	  || info->stateID == 0 || info->stateID > 70
	  || MJD_BEFORE_SENSING_START( info->mjd ) 
	  || MJD_AFTER_SENSING_STOP( info->mjd ) ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_NONE, 
			   "MDS_DET validity check Failure" );

     /* set global varible ClusterDef (not implemented for absOrbit < 4151) */
     numClus = GET_CLUSTERDEF( info->stateID );

     /* read source packages */
     (void) memcpy( &info->bcps, cpntr+(2 * ENVI_USHRT), ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     info->bcps = byte_swap_u16( info->bcps );
#endif
     for ( ncl = nch = 0; nch < num_chan; nch++ ) {
	  (void) memcpy( &chan_sync, cpntr, ENVI_USHRT );
	  cpntr += ENVI_USHRT;
	  if ( chan_sync != CHANNEL_SYNC ) {
	       char msg[SHORT_STRING_LENGTH];
	       char date_str[UTC_STRING_LENGTH];

	       (void) MJD_2_ASCII( info->mjd.days, info->mjd.secnd, 
				   info->mjd.musec, date_str );
	       (void) snprintf( msg, SHORT_STRING_LENGTH,
				FMT_DET_CHAN_SYNC, 
				date_str, info->bcps, nch );
	       NADC_GOTO_ERROR( prognm, NADC_ERR_NONE, msg );
	  }
	  (void) memcpy( &channel.two_byte, cpntr, ENVI_USHRT );
	  info->numClusters += channel.field.clusters;
	  cpntr += 7 * ENVI_USHRT;

	  for ( nc = 0; nc < channel.field.clusters; nc++ ) {
	       (void) memcpy( &clus_sync, cpntr, ENVI_USHRT );
	       cpntr += ENVI_USHRT;
	       if ( clus_sync != CLUSTER_SYNC ) {
		    char msg[SHORT_STRING_LENGTH];
		    char date_str[UTC_STRING_LENGTH];

		    (void) MJD_2_ASCII( info->mjd.days, info->mjd.secnd, 
					info->mjd.musec, date_str );
		    (void) snprintf( msg, SHORT_STRING_LENGTH, 
				     FMT_DET_CLUS_SYNC,
				     date_str, info->bcps, nc );
		    NADC_GOTO_ERROR( prognm, NADC_ERR_NONE, msg );
	       }
	       info->cluster[ncl].chanID = channel.field.id;
	       cpntr += ENVI_USHRT;
	       (void) memcpy(&info->cluster[ncl].clusID, cpntr, ENVI_UCHAR );
	       (void) memcpy(&info->cluster[ncl].coAdding, cpntr+1, ENVI_UCHAR);
	       cpntr += ENVI_USHRT;
	       (void) memcpy(&info->cluster[ncl].start, cpntr, ENVI_USHRT );
	       cpntr += ENVI_USHRT;
	       (void) memcpy(&info->cluster[ncl].length, cpntr, ENVI_USHRT );
	       cpntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       info->cluster[ncl].start = 
		    byte_swap_u16( info->cluster[ncl].start );
	       info->cluster[ncl].length = 
		    byte_swap_u16( info->cluster[ncl].length );
#endif
	       if ( CHECK_CLUSTERDEF(info->cluster+ncl) == DET_SRC_READ_FAILED )
		    NADC_GOTO_ERROR( prognm, NADC_WARN_PDS_RD,
				"corrupted cluster block, remainder skipped" );

	       if ( info->cluster[ncl].coAdding == UCHAR_ONE ) {
		    cpntr += (size_t) info->cluster[ncl].length * ENVI_USHRT;
	       } else {
		    cpntr += (size_t) info->cluster[ncl].length * 3;
		    if ( (info->cluster[ncl].length % 2) == 1 ) 
			 cpntr += ENVI_UCHAR;
	       }
	       /* LV0_ANNOTATION_LENGTH + LV0_PACKET_HDR_LENGTH + 1 = 39 */
	       if ( (cpntr - mds_pntr) > (info->length + 39) ) {
		    char msg[SHORT_STRING_LENGTH];
		    char  date_str[UTC_STRING_LENGTH];

		    (void) MJD_2_ASCII( info->mjd.days, info->mjd.secnd, 
					info->mjd.musec, date_str );
		    (void) snprintf( msg, SHORT_STRING_LENGTH, 
				     FMT_DET_TOO_LARGE,
				     date_str, info->bcps, nc );
		    NADC_ERROR( prognm, NADC_ERR_NONE, msg );
		    return (info->length + 39);
	       }
	       ncl++;
	  }
     }
     return (cpntr - mds_pntr);
done:
     return 0;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_INFO_READ_PMD
.PURPOSE     obtain MDS info of a SCIA level 0 PMD package
.INPUT/OUTPUT
  call as   nr_bytes = SCIA_LV0_INFO_READ_PMD( mds_pntr, info );
     input:  
            char *mds_pntr         : source package buffer
 in/output:  
	    struct mds0_info *info : structure holding info about MDS records

.RETURNS     number of bytes read from buffer, 
             error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
size_t SCIA_LV0_INFO_READ_PMD( const char *mds_pntr, struct mds0_info *info )
{
     const char prognm[] = "SCIA_LV0_INFO_READ_PMD";

     register const char     *cpntr  = mds_pntr;
     register unsigned short num_pmd = 0;
     register unsigned short num_sync = 0; 

     unsigned short pmd_sync;

     /* initialisation */
     info->bcps = 0;
     info->stateIndex = 0;
     info->numClusters = UCHAR_ZERO;
     (void) memset( info->cluster, 0, InfoClusSize );

     /* ISP */
     (void) memcpy( &info->mjd, cpntr, sizeof(struct mjd_envi) );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     info->mjd.days = byte_swap_32( info->mjd.days );
     info->mjd.secnd = byte_swap_u32( info->mjd.secnd );
     info->mjd.musec = byte_swap_u32( info->mjd.musec );
#endif
     cpntr += LV0_ANNOTATION_LENGTH;                /* move to Packet Header */

     /* Packet Header */
     cpntr += 2 * ENVI_USHRT;      /* skip Identification & Sequence Control */
     (void) memcpy( &info->length, cpntr, ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     info->length = byte_swap_u16( info->length );
#endif
     cpntr += ENVI_USHRT;                       /* move to Data Field Header */

     /* Data Header */
     cpntr += ENVI_USHRT;                   /* skip Data Field Header Length */
     (void) memcpy( &info->category, cpntr, ENVI_UCHAR );
     cpntr += ENVI_UCHAR;
     (void) memcpy( &info->stateID, cpntr, ENVI_UCHAR );
     cpntr += ENVI_UCHAR;
     cpntr += 3 * ENVI_USHRT;     /* skip ICU & Redundancy Definition Vector */
     (void) memcpy( &info->packetID, cpntr, ENVI_UCHAR );
     info->packetID = info->packetID >> 4;
     cpntr += ENVI_USHRT;
#ifdef DEBUG
     (void) fprintf( stderr, "PMD: %3hhu %3hhu %5d %5u %6u\n", info->stateID, 
		     info->packetID, info->mjd.days, 
		     info->mjd.secnd, info->mjd.musec );
#endif

     /* validity check */
     if ( info->packetID != SCIA_PMD_PACKET
	  || info->stateID == 0 || info->stateID > 70
	  || MJD_BEFORE_SENSING_START( info->mjd ) 
	  || MJD_AFTER_SENSING_STOP( info->mjd ) ) {
 	  NADC_ERROR( prognm, NADC_ERR_NONE, "MDS_PMD validity check Failure" );
	  return 0;
     }

     /* read source packages */
     cpntr += ENVI_USHRT;                         /* skip PMD Temperature HK */
     (void) memcpy( &info->bcps, cpntr+(PMD_NUMBER * 2 + 1) * ENVI_USHRT, 
		    ENVI_USHRT );
#ifdef _SWAP_TO_LITTLE_ENDIAN
     info->bcps = byte_swap_u16( info->bcps );
#endif
     do {
	  (void) memcpy( &pmd_sync, cpntr, ENVI_USHRT );
	  if ( pmd_sync == PMD_SYNC || pmd_sync == USHRT_ZERO ) num_sync++;
	  cpntr += (PMD_NUMBER * 2 + 3) * ENVI_USHRT;
     } while ( ++num_pmd < NUM_LV0_PMD_PACKET );

     if ( num_sync < NUM_LV0_PMD_PACKET ){
	  char msg[SHORT_STRING_LENGTH];
	  char  date_str[UTC_STRING_LENGTH];

	  (void) MJD_2_ASCII( info->mjd.days, info->mjd.secnd, 
			      info->mjd.musec, date_str );
	  (void) snprintf( msg, SHORT_STRING_LENGTH,
			   "PMD package (%s bcps=%-hu) incomplete - %hu", 
			   date_str, info->bcps, num_sync );
	  NADC_ERROR( prognm, NADC_ERR_NONE, msg );
     }
     return (cpntr - mds_pntr);
}

/*+++++++++++++++++++++++++
.IDENTifer   GET_INFO_STATEINDEX
.PURPOSE     identify MDS packages of the same state
.INPUT/OUTPUT
  call as   GET_INFO_STATEINDEX( num_info, info );
     input:  
            unsigned int num_info  : number of info records
 in/output:  
	    struct mds0_info *info : structure holding info about MDS records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static inline
void GET_INFO_STATEINDEX( unsigned int num_info, struct mds0_info *info )
{
     const char prognm[] = "GET_INFO_STATEINDEX";

     register unsigned int ni;
     register unsigned int ni_last = 0;

     unsigned short stateIndex = 1;

     struct mjd_envi mjd_current;
/*
 * handle special cases gracefully
 */
     if ( num_info == 0 ) return;
     if ( num_info == 1 ) {
	  info[0].stateIndex = 1;
	  return;
     }
/*
 * repair corrupted mjd-records
 */
     for ( ni = 0; ni < num_info; ni++ ) {
	  if ( info[ni].mjd.days == 0 ) {
	       char msg[SHORT_STRING_LENGTH];

	       if ( ni > 0 )
		    (void) memcpy( &info[ni].mjd, &info[ni-1].mjd,
				   sizeof( struct mjd_envi ) );
	       else
		    (void) memcpy( &info[ni].mjd, &info[ni+1].mjd,
				   sizeof( struct mjd_envi ) );

	       (void) snprintf( msg, SHORT_STRING_LENGTH,
				"correct MJD of info record: %-u", ni );
	  }
     }
/*
 * collect all readouts belonging to one state
 */
     do {
	  register unsigned int ni_first = ni_last;

	  (void) memcpy( &mjd_current, &info[ni_last].mjd, 
			 sizeof(struct mjd_envi));
	  for ( ni = ni_last; ni < num_info; ni++ ) {
	       if ( CMP_MJD( info[ni].mjd, mjd_current ) == 0 ) {
		    info[ni].stateIndex = stateIndex;
		    ni_last = ni;
	       }
	  }
	  if ( ni_last - ni_first == 1u )         /* corrupted time tag !*/
	       info[ni_first].stateIndex = USHRT_MAX;
	  else
	       stateIndex++;

	  /* find next record with stateIndex equal to zero */
	  if ( ++ni_last < num_info && info[ni_last].stateIndex == 0 )
	       continue;

	  ni_last = 0;
	  while ( ++ni_last < num_info && info[ni_last].stateIndex != 0 );
     } while ( ni_last < num_info );
/*
 * repair state IDs
 */
     for ( ni = 0; ni < num_info; ni++ ) {
	  if ( info[ni].stateIndex == USHRT_MAX ) {
	       char msg[SHORT_STRING_LENGTH];

	       (void) snprintf( msg, SHORT_STRING_LENGTH,
				"correct stateID of info record: %-u", ni );
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       if ( ni == 0 && info[ni].stateID == info[ni+1].stateID )
		    info[ni].stateIndex = info[ni+1].stateIndex;
	       else if ( ni == num_info-1 
			 && info[ni].stateID == info[ni-1].stateID )
		    info[ni].stateIndex = info[ni-1].stateIndex;
	       else if ( info[ni-1].stateID == info[ni+1].stateID )
		    info[ni].stateIndex = info[ni+1].stateIndex;
	  }
     }
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int GET_SCIA_LV0_MDS_INFO( FILE *fd, struct mph_envi mph,
				    const struct dsd_envi *dsd, 
				    struct mds0_info *info )
       /*@globals  errno, stderr, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, stderr, nadc_stat, nadc_err_stack, fd, info@*/
{
     const char prognm[] = "GET_SCIA_LV0_MDS_INFO";

     register char *cpntr;

     char           *mds_start, *mds_char = NULL;
     unsigned short ubuff;
     unsigned int   num_info = 0;

     size_t nr_byte;
/*
 * set flag which indicates that Detector MDS records have to be checked
 */
     SET_NO_CLUSTER_CORRECTION( fd );

     /* set range for the start time of the data packages */
     ASCII_2_MJD( mph.sensing_start, 
		  &mjd_first.days, &mjd_first.secnd, &mjd_first.musec );
     ASCII_2_MJD( mph.sensing_stop, 
		  &mjd_last.days, &mjd_last.secnd, &mjd_last.musec );

     /* allocate memory to buffer source packages */
     if ( (mds_char = (char *) malloc( (size_t) dsd->size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mds_char" );

     /* read all Sciamachy source packages in product */
     (void) fseek( fd, (long) dsd->offset, SEEK_SET );
     if ( fread( mds_char, (size_t) dsd->size, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd->name );

     /* examine whole source data section */
     cpntr = mds_start = mds_char;
     do {
	  (void) memcpy( &ubuff, cpntr, ENVI_USHRT );
	  if ( ubuff == CHANNEL_SYNC && 
		      (cpntr - mds_start) >= OFFS_DET_SYNC ) {
	       if ( (cpntr - mds_start) > OFFS_DET_SYNC )
		    mds_start = cpntr - OFFS_DET_SYNC;

	       /* store byte offset w.r.t. begin of file */
	       info[num_info].offset = (unsigned int) 
		    (dsd->offset + (mds_start - mds_char));

	       /* read Detector package */
	       nr_byte = SCIA_LV0_INFO_READ_DET( mds_start, info+num_info );
	       if ( nr_byte > 0 ) {
		    cpntr = (mds_start += nr_byte);
		    num_info++;
	       } else
		    cpntr += ENVI_USHRT;
	  } else if ( ubuff == AUX_SYNC 
		      && (cpntr - mds_start) >= OFFS_AUX_SYNC ) {
	       if ( (cpntr - mds_start) > OFFS_AUX_SYNC )
		    mds_start = cpntr - OFFS_AUX_SYNC;

	       /* store byte offset w.r.t. begin of file */
	       info[num_info].offset = (unsigned int) 
		    (dsd->offset + (mds_start - mds_char));

	       /* read Auxiliary package */
	       nr_byte = SCIA_LV0_INFO_READ_AUX( mds_start, info+num_info );
	       if ( nr_byte > 0 ) {
		    cpntr = (mds_start += nr_byte);
		    num_info++;
	       } else
		    cpntr += ENVI_USHRT;
	  } else if ( ubuff == PMD_SYNC 
		      && (cpntr - mds_start) >= OFFS_PMD_SYNC ) {
	       if ( (cpntr - mds_start) > OFFS_PMD_SYNC )
		    mds_start = cpntr - OFFS_PMD_SYNC;

	       /* store byte offset w.r.t. begin of file */
	       info[num_info].offset = (unsigned int) 
		    (dsd->offset + (mds_start - mds_char));

	       /* read PMD package */
	       nr_byte = SCIA_LV0_INFO_READ_PMD( mds_start, info+num_info );
	       if ( nr_byte > 0 ) {
		    cpntr = (mds_start += nr_byte);
		    num_info++;
	       } else
		    cpntr += ENVI_USHRT;
	  } else {
	       cpntr += ENVI_USHRT;
	  }
     } while ( (cpntr - mds_char) < (long) (dsd->size - 2) );

     GET_INFO_STATEINDEX( num_info, info );
done:
     if ( mds_char != NULL ) free( mds_char );
     return num_info;
}
