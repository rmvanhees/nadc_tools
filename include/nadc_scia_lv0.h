/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2003 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_SCIA_LV0
.AUTHOR      R.M. van Hees 
.KEYWORDS    header file
.LANGUAGE    ANSI-C
.PURPOSE     macros and structures for SCIAMACHY level 0 modules
.ENVIRONment none
.VERSION      1.4   31-Mar-2003	modified include for C++ code
              1.3   20-Dec-2002	added bcps to mds0_info 
                                and align the structure, RvH
              1.2   19-Mar-2002	replaced confusing bench_obm with bench_rad, 
                                RvH
              1.1   07-Dec-2001	added state_id and category to mds0_info, RvH 
              1.0   13-Aug-2001 creation by R.M. van Hees
------------------------------------------------------------*/
#ifndef  __NADC_SCIA_LV0                        /* Avoid redefinitions */
#define  __NADC_SCIA_LV0

#ifdef __cplusplus
extern "C" {
#endif

#define LV0_MX_CLUSTERS         16

#define LV0_ANNOTATION_LENGTH   32
#define LV0_PACKET_HDR_LENGTH   6
#define LV0_DATA_HDR_LENGTH     12
#define LV0_PMTC_HDR_LENGTH     18

#define DET_DATA_HDR_LENGTH     66
#define PMD_DATA_HDR_LENGTH     12
#define AUX_DATA_SRC_LENGTH     1630
#define PMD_DATA_SRC_LENGTH     6802
#define NUM_LV0_AUX_BCP         ((unsigned short) 16)
#define NUM_LV0_AUX_PMTC_FRAME  ((unsigned short) 5)
#define NUM_LV0_PMD_PACKET      ((unsigned short) 200)

enum scia_packet { SCIA_DET_PACKET = 1, SCIA_AUX_PACKET, SCIA_PMD_PACKET };

/*
 * cluster definitions (hard-coded!), valid after orbit 4150 (??)
 */
struct clusdef_rec {
     unsigned char  chanID;
     unsigned char  clusID;
     unsigned short start;
     unsigned short length;
};

/*
 * used in "mds0_aux", "mds0_det" and "mds0_pmd"
 */
struct fep_hdr
{
     struct mjd_envi  gsrt;
     unsigned short   isp_length;
     unsigned short   crc_errs;
     unsigned short   rs_errs;
};

/*
 * used in "mds0_aux", "mds0_det" and "mds0_pmd"
 */
struct packet_hdr
{
     union 
     {
	  struct packet_id_breakout
	  {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned short op_mode:5;
	       unsigned short vcid:5;
	       unsigned short :6;
#else
	       unsigned short :6;
	       unsigned short vcid:5;
	       unsigned short op_mode:5;
#endif
	  } field;

	  unsigned short two_byte;
     } api;

     unsigned short seq_cntrl;
     unsigned short length;     
};

/*
 * used in "mds0_aux", "mds0_det" and "mds0_pmd"
 */
struct data_hdr
{
     unsigned char  category;
     unsigned char  state_id;
     unsigned short length;
     union
     {
	  struct vector_breakout
	  {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char atc_id:6;
	       unsigned char hsm:2;
	       unsigned char config_id:8;
#else
	       unsigned char hsm:2;
	       unsigned char atc_id:6;
	       unsigned char config_id:8;
#endif
	  } field;

	  unsigned short two_byte;
     } rdv;
     union
     {
	  struct id_breakout
	  {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char :4;
	       unsigned char  packet:4;
	       unsigned char  overflow:4;
	       unsigned char :4;
#else
	       unsigned char  packet:4;
	       unsigned char :4;
	       unsigned char :4;
	       unsigned char  overflow:4;
#endif
	  } field;
	  unsigned short two_byte;
     } id;
     unsigned int on_board_time;
};

/*
 * used in "mds0_aux" and "mds0_det"
 */
struct pmtc_hdr
{
     union 
     {
	  struct pmtc_1_breakout
	  {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char ndfm:2;
	       unsigned char :2;
	       unsigned char phase:4;
	       unsigned char sls:2;
	       unsigned char wls:2;
	       unsigned char apsm:2;
	       unsigned char ncwm:2;
#else
	       unsigned char phase:4;
	       unsigned char :2;
	       unsigned char ndfm:2;
	       unsigned char ncwm:2;
	       unsigned char apsm:2;
	       unsigned char wls:2;
	       unsigned char sls:2;
#endif
	  } field;

	  unsigned short two_byte;
     } pmtc_1;
     unsigned short scanner_mode;

     union 
     {
	  struct az_breakout
	  {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned short repeat:12;
	       unsigned char  basic:4;
	       unsigned char  h_w:4;
	       unsigned char  rel:4;
	       unsigned char  corr:4;
	       unsigned char  invert:1;
	       unsigned char  filter:1;
	       unsigned char  centre:1;
	       unsigned char  type:1;
#else
	       unsigned char  type:1;
	       unsigned char  centre:1;
	       unsigned char  filter:1;
	       unsigned char  invert:1;
	       unsigned char  corr:4;
	       unsigned char  rel:4;
	       unsigned char  h_w:4;
	       unsigned char  basic:4;
	       unsigned short repeat:12;
#endif
	  } field;

	  unsigned int four_byte;
     } az_param;

     union 
     {
	  struct elv_breakout
	  {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned short repeat:12;
	       unsigned char  basic:4;
	       unsigned char  :4;
	       unsigned char  rel:4;
	       unsigned char  corr:4;
	       unsigned char  invert:1;
	       unsigned char  filter:1;
	       unsigned char  centre:1;
	       unsigned char  :1;
#else
	       unsigned char  :1;
	       unsigned char  centre:1;
	       unsigned char  filter:1;
	       unsigned char  invert:1;
	       unsigned char  corr:4;
	       unsigned char  rel:4;
	       unsigned char  :4;
	       unsigned char  basic:4;
	       unsigned short repeat:12;
#endif
	  } field;

	  unsigned int four_byte;
     } elv_param;
     unsigned char factor[6];
};

/*
 * -------------------------
 * Auxiliary: [ISP] Packet Data Field (Source Data)
 */
union bench_cntrl
{
     struct lv_bench_breakout
     {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  unsigned char  stat:1;
	  unsigned short temp:15;
#else
	  unsigned short temp:15;
	  unsigned char  stat:1;
#endif
     } field;
     unsigned short two_byte;
};

struct aux_bcp
{
     unsigned short sync;
     unsigned short bcps;
     union
     {
	  struct flag_breakout
	  {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char phase:4;
	       unsigned char m:1;
	       unsigned char d:1;
	       unsigned char eu:1;
	       unsigned char au:1;
	       unsigned char pointing:6;
	       unsigned char :2;
#else
	       unsigned char au:1;
	       unsigned char eu:1;
	       unsigned char d:1;
	       unsigned char m:1;
	       unsigned char phase:4;
	       unsigned char :2;
	       unsigned char pointing:6;
#endif
	  } field;
	  unsigned short two_byte;
     } flags;
     unsigned int azi_encode_cntr;
     unsigned int ele_encode_cntr;
     unsigned short azi_cntr_error;
     unsigned short ele_cntr_error;
     unsigned short azi_scan_error;
     unsigned short ele_scan_error;
};

struct pmtc_frame
{
     struct aux_bcp bcp[NUM_LV0_AUX_BCP];
     union bench_cntrl bench_rad;
     union bench_cntrl bench_elv;
     union bench_cntrl bench_az;
};

struct aux_src
{
     struct pmtc_frame pmtc[NUM_LV0_AUX_PMTC_FRAME];
};

/*
 * -------------------------
 * Detector: [ISP] Packet Data Field (Source Data)
 *
 * Channel Data Header
 */
struct chan_hdr
{
     unsigned short sync;
     unsigned short bcps;
     unsigned short bias;
     unsigned short temp;
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
     union
     {
	  struct
	  {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char status:3;
	       unsigned char ratio:5;
	       unsigned char frame:8;
#else
	       unsigned char ratio:5;
	       unsigned char status:3;
	       unsigned char frame:8;
#endif
	  } field;
	  unsigned short two_byte;
     } ratio_hdr;
     union
     {
	  struct
	  {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char  cntrl:2;
	       unsigned char  ratio:5;
	       unsigned short sec:9;
	       unsigned char  mode:2;
	       unsigned short etf:14;
#else
	       unsigned short etf:14;
	       unsigned char  mode:2;
	       unsigned short sec:9;
	       unsigned char  ratio:5;
	       unsigned char  cntrl:2;
#endif
	  } field;
	  unsigned int four_byte;
     } command_vis;
     union
     {
	  struct
	  {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char  cntrl:2;
	       unsigned char  pet:4;
	       unsigned char  :2;
	       unsigned char  bias:3;
	       unsigned char  :3;
	       unsigned char  comp:2;
	       unsigned char  mode:2;
	       unsigned short etf:14;
#else
	       unsigned short etf:14;
	       unsigned char  mode:2;
	       unsigned char  comp:2;
	       unsigned char  :3;
	       unsigned char  bias:3;
	       unsigned char  :2;
	       unsigned char  pet:4;
	       unsigned char  cntrl:2;
#endif
	  } field;
	  unsigned int four_byte;
     } command_ir;
};

/*
 * Pixel Data Block
 */
struct chan_src
{
     unsigned char  cluster_id;
     unsigned char  co_adding;
     unsigned short sync;
     unsigned short block_nr;
     unsigned short start;
     unsigned short length;
     unsigned char  *data;
};

#ifdef _HDF5_H
struct h5_chan_src 
{
     unsigned char   cluster_id;
     unsigned char   co_adding;
     unsigned short  sync;
     unsigned short  block_nr;
     unsigned short  start;
     unsigned short  length;
     unsigned short  indx_data;
};
#endif

struct det_src
{
     struct chan_hdr hdr;
     struct chan_src *pixel;
};

/*
 * -------------------------
 * PMD: [ISP] Packet Data Field (Source Data)
 */
struct pmd_data
{
     unsigned short sync;
     unsigned short data[PMD_NUMBER][2];   /* [2,PMD_NUMBER] */
     unsigned short bcps;
     union 
     {
	  struct pmd_time_breakout
	  {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned short delta:15;
	       unsigned short is:1;
#else
	       unsigned short is:1;
	       unsigned short delta:15;
#endif
	  } field;
	  
	  unsigned short two_byte;
     } time;
};

struct pmd_src
{
     unsigned short temp;
     struct pmd_data packet[NUM_LV0_PMD_PACKET];
};

/* +++++ SCIA level 0 PDS data structures +++++ */
struct sph0_scia
{
     char  descriptor[29];
     double start_lat;
     double start_lon;
     double stop_lat;
     double stop_lon;
     double sat_track;

     unsigned short  isp_errors;
     unsigned short  missing_isps;
     unsigned short  isp_discard;
     unsigned short  rs_sign;

     int num_error_isps;
     double error_isps_thres;
     int num_miss_isps;
     double miss_isps_thres;
     int num_discard_isps;
     double discard_isps_thres;
     int num_rs_isps;
     double rs_thres;

     char  tx_rx_polar[6];
     char  swath[4];
};

/*
 * -------------------------
 * structure used to store info about level 0 MDSs
 */
struct info_clus
{
     unsigned char  chanID;
     unsigned char  clusID;
     unsigned char  coAdding;
     unsigned short start;
     unsigned short length;
};

struct mds0_info
{
     unsigned char   packetID;
     unsigned char   category;
     unsigned char   stateID;
     unsigned char   numClusters;
     unsigned short  length;
     unsigned short  bcps;
     unsigned short  stateIndex;
     unsigned int    offset;
     struct mjd_envi mjd;
     struct info_clus cluster[MAX_CLUSTER];
};

struct mds0_aux
{
     struct mjd_envi   isp;
     struct fep_hdr    fep_hdr;
     struct packet_hdr packet_hdr;
     struct data_hdr   data_hdr;
     struct pmtc_hdr   pmtc_hdr;
     struct aux_src    data_src;
};

struct mds0_det
{
     unsigned short    bcps;
     unsigned short    num_chan;
     int               orbit_vector[8];
     struct mjd_envi   isp;
     struct fep_hdr    fep_hdr;
     struct packet_hdr packet_hdr;
     struct data_hdr   data_hdr;
     struct pmtc_hdr   pmtc_hdr;
     struct det_src    *data_src;
};

struct mds0_pmd
{
     struct mjd_envi   isp;
     struct fep_hdr    fep_hdr;
     struct packet_hdr packet_hdr;
     struct data_hdr   data_hdr;
     struct pmd_src    data_src;
};

#define SCIA_INFO_DB_NAME  "scia_lv0_info.h5"
struct offs_size_rec
{
     unsigned int   offset;
     unsigned int   length;
};

struct h5_mds0_info
{
     unsigned char   packetID;
     unsigned char   category;
     unsigned char   stateID;
     unsigned char   numClusters;
     unsigned short  length;
     unsigned short  bcps;
     unsigned short  stateIndex;
     unsigned int    offset;
     struct mjd_envi mjd;
};

/*
 * prototype declarations of Sciamachy level 0 functions
 */
extern unsigned short GET_SCIA_CLUSDEF( unsigned short, 
					unsigned int, 
					const struct mds0_info *,
					/*@NULL@*/ /*@out@*/
					struct clusdef_rec *clusDef )
       /*@globals clusDef;@*/
       /*@modifies clusDef@*/;


#if defined _STDIO_INCLUDED || defined _STDIO_H || defined __STDIO_H__
extern void SCIA_LV0_RD_SPH( FILE *fd, const struct mph_envi,
			     /*@out@*/ struct sph0_scia *sph )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *sph@*/;
extern void SCIA_LV0_WR_SPH( FILE *fd, const struct mph_envi,
			     const struct sph0_scia sph )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/;

extern unsigned int SCIA_LV0_RD_MDS_INFO( FILE *fd, unsigned int, 
					  const struct dsd_envi *,
					  /*@out@*/ struct mds0_info **info )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *info@*/;

extern unsigned int GET_SCIA_LV0_MDS_INFO( FILE *fd, struct mph_envi,
					   const struct dsd_envi *, 
					   struct mds0_info *info )
       /*@globals  errno, stderr, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, stderr, nadc_stat, nadc_err_stack, fd, info@*/;

extern unsigned int SCIA_LV0_RD_AUX( FILE *fd, const struct mds0_info *,
                                     unsigned int,
                                     /*@out@*/ struct mds0_aux **aux_out )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *aux_out@*/;
extern unsigned int SCIA_LV0_WR_AUX( FILE *fd, const struct mds0_info *,
                                     unsigned int, const struct mds0_aux * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/;
extern unsigned int GET_SCIA_LV0_STATE_AUX( FILE *, const struct mds0_info *,
				     unsigned int, struct mds0_aux **aux_out )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *aux_out@*/;

extern unsigned int SCIA_LV0_RD_DET( FILE *fd, const struct mds0_info *,
                                     unsigned int, unsigned char,
                                     /*@out@*/ struct mds0_det **det_out )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *det_out@*/;

extern unsigned int SCIA_LV0_WR_DET( FILE *fd, const struct mds0_info *,
                                     unsigned int, const struct mds0_det * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/;
extern unsigned int GET_SCIA_LV0_STATE_DET( unsigned char, FILE *, 
					    const struct mds0_info *,
					    unsigned int, 
					    struct mds0_det **det_out )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, det_out@*/;

extern unsigned int SCIA_LV0_RD_PMD( FILE *fd, const struct mds0_info *,
                                     unsigned int,
                                     /*@out@*/ struct mds0_pmd **pmd_out )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *pmd_out@*/;
extern unsigned int SCIA_LV0_WR_PMD( FILE *fd, const struct mds0_info *,
                                     unsigned int, const struct mds0_pmd * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/;
extern unsigned int GET_SCIA_LV0_STATE_PMD( FILE *, const struct mds0_info *,
				     unsigned int, struct mds0_pmd **pmd_out )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, pmd_out@*/;

extern void SCIA_LV0_RD_LV1_AUX( FILE *fd, /*@out@*/ struct mds0_aux *aux )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *aux@*/;
extern unsigned int SCIA_LV0_WR_LV1_AUX( FILE *fd, const struct mds0_aux )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/;

extern void SCIA_LV0_RD_LV1_PMD( FILE *fd, /*@out@*/ struct mds0_pmd *pmd )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *pmd@*/;
extern unsigned int SCIA_LV0_WR_LV1_PMD( FILE *fd, const struct mds0_pmd )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/;

extern void SCIA_WR_ASCII_LV0_AUX( FILE *fd, unsigned int,
				   const struct mds0_aux * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, fd@*/;
extern void SCIA_WR_ASCII_LV0_PMD( FILE *fd, unsigned int,
				   const struct mds0_pmd * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, fd@*/;
#endif   /* ---- defined _STDIO_INCLUDED || defined _STDIO_H ----- */

extern double GET_SCIA_LV0_MDS_TIME( int, const void * );
extern void GET_SCIA_LV0_MDS_TIME_ARR(int, const void *, /*@out@*/ double *);
extern void GET_SCIA_LV0_MDS_ANGLES( const struct mds0_aux *, 
				       /*@out@*/ float *, /*@out@*/ float * );
extern void GET_SCIA_LV0_STATE_ANGLE( unsigned short, 
					const struct mds0_aux *, 
					/*@out@*/ float *, /*@out@*/ float * );
extern void GET_SCIA_LV0_STATE_OBMtemp( bool, unsigned short, 
					  const struct mds0_aux *, 
					  /*@out@*/ float * );
extern void GET_SCIA_LV0_STATE_DETtemp( unsigned short, 
					const struct mds0_det *,
					/*@out@*/ float * );
extern void GET_SCIA_LV0_STATE_PMDtemp( unsigned short, 
					const struct mds0_pmd *, 
					/*@out@*/ float * );
extern void GET_SCIA_LV0_DET_PET( struct chan_hdr, 
				  /*@out@*/ float *, 
				  /*@out@*/ unsigned short * );
extern unsigned short GET_SCIA_LV0C_MDS( unsigned int, 
					 const struct mds0_info *,
					 const struct mds0_det *, 
					 /*@out@*/ struct mds1c_scia **mds )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, *mds@*/;

extern void SCIA_LV0_WR_ASCII_SPH( struct param_record, 
				   const struct sph0_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;

extern void SCIA_LV0_WR_ASCII_INFO( struct param_record, unsigned int,
				    const struct mds0_info * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;

extern void SCIA_LV0_FREE_MDS_DET( unsigned int, 
                                   /*@only@*/ struct mds0_det * );

extern unsigned int SCIA_LV0_SELECT_MDS( int, const struct param_record,
					 unsigned int, 
					 const struct mds0_info *,
			  /*@null@*/ /*@out@*/ struct mds0_info **info )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, *info@*/;

extern void SCIA_LV0_WR_ASCII_AUX( struct param_record, unsigned int,
				   unsigned int, const struct mds0_aux * )
      /*@globals  errno, nadc_stat, nadc_err_stack;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack@*/;
 
extern void SCIA_LV0_WR_ASCII_DET( struct param_record, unsigned int,
				   unsigned int, const struct mds0_det * )
      /*@globals  nadc_stat, nadc_err_stack;@*/
      /*@modifies nadc_stat, nadc_err_stack@*/;
 
extern void SCIA_LV0_WR_ASCII_PMD( struct param_record, unsigned int,
				   unsigned int, const struct mds0_pmd * )
      /*@globals  nadc_stat, nadc_err_stack;@*/
      /*@modifies nadc_stat, nadc_err_stack@*/;
 
#ifdef _HDF5_H
extern void SCIA_LV0_WR_H5_SPH( struct param_record, 
				const struct sph0_scia * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;
extern void SCIA_LV0_WR_H5_AUX( struct param_record, unsigned int, 
				unsigned int, const struct mds0_aux * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;
extern void SCIA_LV0_WR_H5_DET( struct param_record, const struct mds0_info *, 
				unsigned int, const struct mds0_det * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;
extern void SCIA_LV0_WR_H5_PMD( struct param_record, unsigned int, 
				unsigned int, const struct mds0_pmd * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;
extern unsigned int SCIA_LV0_RD_H5_INFO_DB( const char *,
					    struct mds0_info **info )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, *info@*/;
#endif /* _HDF5_H */

#ifdef LIBPQ_FE_H
extern void SCIA_LV0_WR_SQL_META( PGconn *conn, bool, const char *, 
                                  const struct mph_envi * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void SCIA_LV0_MATCH_STATE( PGconn *conn, bool, const struct mph_envi *,
                                  unsigned short, const struct mds0_sql * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void SCIA_LV0_DEL_ENTRY( PGconn *conn, bool, const char * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
#endif /* LIBPQ_FE_H */

#ifdef __cplusplus
  }
#endif
#endif /* __NADC_SCIA_LV0 */
