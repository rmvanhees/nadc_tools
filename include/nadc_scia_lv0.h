/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2003 - 2019 SRON (R.M.van.Hees@sron.nl)

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
.VERSION      1.5   31-Oct-2013	new mds0_info record, RvH
              1.4   31-Mar-2003	modified include for C++ code
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
#define AUX_DATA_SRC_LENGTH     326
#define PMD_DATA_SRC_LENGTH     6802
#define NUM_LV0_AUX_BCP         ((unsigned short) 16)
#define NUM_LV0_AUX_PMTC_FRAME  ((unsigned short) 5)
#define NUM_LV0_PMD_PACKET      ((unsigned short) 200)

enum scia_packet { SCIA_DET_PACKET = 1, SCIA_AUX_PACKET, SCIA_PMD_PACKET };

/*
 * Auxiliary Data Packet
 * - ISP sensing time: struct mjd_envi                               [12 bytes]
 * - Front End Processor (FEP): struct fep_hdr                       [20 bytes]
 * - packet header: struct packet_hdr             [LV0_PACKET_HDR_LENGTH bytes]
 * - data field header: struct data_hdr             [LV0_DATA_HDR_LENGTH bytes]
 * - PMTC field header: struct pmtc_hdr             [LV0_PMTC_HDR_LENGTH bytes]
 * - PMTC auxiliary frames         [NUM_LV0_AUX_PMTC_FRAME * struct pmtc_frame]
 * |--- struct pmtc_frame                           [AUX_DATA_SRC_LENGTH bytes]
 *
 *
 *
 * Detector Data Packet
 * - ISP sensing time: struct mjd_envi                               [12 bytes]
 * - Front End Processor (FEP): struct fep_hdr                       [20 bytes]
 * - packet header: struct packet_hdr             [LV0_PACKET_HDR_LENGTH bytes]
 * - data field header: struct data_hdr             [LV0_DATA_HDR_LENGTH bytes]
 * - channel data blocks                                             [variable]
 * |--- channel data header: struct chan_hdr                         [16 bytes]
 * |--- pixel data blocks: struct chan_src                           [variable]
 *    |--- channel field header:                                     [20 bytes]
 *    |--- pixel data                                                [variable]
 *
 *
 *
 * PMD Data Packet
 * - ISP sensing time: struct mjd_envi                               [12 bytes]
 * - Front End Processor (FEP): struct fep_hdr                       [20 bytes]
 * - packet header: struct packet_hdr             [LV0_PACKET_HDR_LENGTH bytes]
 * - data field header: struct data_hdr             [LV0_DATA_HDR_LENGTH bytes]
 * - PMD data block                                 [PMD_DATA_SRC_LENGTH bytes]
 * |--- unsigned short temp                                           [2 bytes]
 * |--- PMD data packets                 [NUM_LV0_PMD_PACKET * struct pmd_data]
 *      |--- struct pmd_data                                         [34 bytes]
 */

struct fep_hdr
{
     struct mjd_envi  gsrt;
     unsigned short   isp_length;
     unsigned short   crc_errs;
     unsigned short   rs_errs;
};

struct packet_hdr
{
     union {
	  struct packet_bitfield {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char op_mode:5, vcid:5, :6;
#else
	       unsigned char :6, vcid:5, op_mode:5;
#endif
	  } field;
	  unsigned short two_byte;
     } api;
     unsigned short seq_cntrl;
     unsigned short length;     
};

struct data_hdr
{
     unsigned char  category;
     unsigned char  state_id;
     unsigned short length;
     union {
	  struct rdv_bitfield {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char atc_id:6, hsm:2, config_id:8;
#else
	       unsigned char hsm:2, atc_id:6, config_id:8;
#endif
	  } field;
	  unsigned short two_byte;
     } rdv;
     union {
	  struct id_bitfield {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char :4, packet:4, overflow:4, :4;
#else
	       unsigned char packet:4, :4, :4, overflow:4;
#endif
	  } field;
	  unsigned short two_byte;
     } id;
     unsigned int on_board_time;
};

struct pmtc_hdr
{
     union {
	  struct pmtc_1_bitfield {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char ndfm:2, :2, phase:4, 
		    sls:2, wls:2, apsm:2, ncwm:2;
#else
	       unsigned char phase:4, :2, ndfm:2,
		    ncwm:2, apsm:2, wls:2, sls:2;
#endif
	  } field;
	  unsigned short two_byte;
     } pmtc_1;
     unsigned short scanner_mode;
     union {
	  struct az_bitfield {
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
     union {
	  struct elv_bitfield {
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
struct aux_bcp
{
     unsigned short sync;
     unsigned short bcps;
     union {
	  struct flag_bitfield {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char phase:4, m:1, d:1, eu:1, au:1, pointing:6, :2;
#else
	       unsigned char au:1, eu:1, d:1, m:1, phase:4, :2, pointing:6;
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

union bench_cntrl {
     struct lv_bench_bitfield {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  unsigned short stat:1, temp:15;
#else
	  unsigned short temp:15, stat:1;
#endif
     } field;
     unsigned short two_byte;
};

struct pmtc_frame
{
     struct aux_bcp bcp[NUM_LV0_AUX_BCP];
     union bench_cntrl bench_rad;
     union bench_cntrl bench_elv;
     union bench_cntrl bench_az;
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
     union {
	  struct channel_bitfield {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char lu:2, is:2, id:4, clusters:8;
#else
	       unsigned char id:4, is:2, lu:2, clusters:8;
#endif
	  } field;
	  unsigned short two_byte;
     } channel;
     union {
	  struct ratio_hdr_bitfield {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char status:3, ratio:5, frame:8;
#else
	       unsigned char ratio:5, status:3, frame:8;
#endif
	  } field;
	  unsigned short two_byte;
     } ratio_hdr;
     union {
	  struct command_vis_bitfield {
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
     union {
	  struct command_ir_bitfield {
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
     union {
	  struct time_bitfield {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned short delta:15, is:1;
#else
	       unsigned short is:1, delta:15;
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

/*
 * cluster definitions
 */
struct clusdef_rec {
     unsigned char  chanID;
     unsigned char  clusID;
     unsigned short start;
     unsigned short length;
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
struct mds0_info
{
     struct mjd_envi mjd;
     unsigned int    offset;
     union {
	  struct qinfo_bitfield {                 // increasing severity
	       unsigned char packet_id : 1;
	       unsigned char state_id  : 1;
	       unsigned char on_board_time : 1;
	       unsigned char bcps      : 1;
	       unsigned char duplicate : 1;
	       unsigned char dsr_size  : 1;
	       unsigned char sync      : 1;
	       unsigned char dumy8     : 1;
	  } flag;
	  unsigned char value;
     } q;
     unsigned char   packet_id;
     unsigned char   category;
     unsigned char   state_id;
     unsigned short  crc_errors;
     unsigned short  rs_errors;
     unsigned short  bcps;
     unsigned short  packet_length;
     unsigned int    on_board_time;
};


union qstate_rec {
     struct qstate_bitfield {                     // increasing severity
	  unsigned char duplicates  : 1;
	  unsigned char sorted      : 1;
	  unsigned char too_short   : 1;
	  unsigned char dsr_missing : 1;
	  unsigned char sync        : 1;
	  unsigned char crc         : 1;
	  unsigned char dumy7 : 1;
	  unsigned char dumy8 : 1;
     } flag;
     unsigned char value;
};

struct mds0_states
{
     struct mjd_envi  mjd;
     union qstate_rec q_aux;
     union qstate_rec q_det;
     union qstate_rec q_pmd;
     unsigned char    category;
     unsigned char    state_id;
     unsigned short   num_aux;
     unsigned short   num_det;
     unsigned short   num_pmd;
     unsigned short   orbit;
     unsigned int     on_board_time;
     unsigned int     offset;
     struct mds0_info *info_aux;
     struct mds0_info *info_det;
     struct mds0_info *info_pmd;
};

struct mds0_aux
{
     struct mjd_envi   isp;
     struct fep_hdr    fep_hdr;
     struct packet_hdr packet_hdr;
     struct data_hdr   data_hdr;
     struct pmtc_hdr   pmtc_hdr;
     struct pmtc_frame data_src[NUM_LV0_AUX_PMTC_FRAME];
};

struct mds1_aux
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     struct packet_hdr packet_hdr;
     struct data_hdr   data_hdr;
     struct pmtc_hdr   pmtc_hdr;
     struct pmtc_frame data_src[NUM_LV0_AUX_PMTC_FRAME];
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

struct mds1_pmd
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     struct packet_hdr packet_hdr;
     struct data_hdr   data_hdr;
     struct pmd_src    data_src;
};

/*
 * prototype declarations of Sciamachy level 0 functions
 */
extern unsigned short GET_SCIA_CLUSDEF(unsigned char, 
					/*@out@*/ struct clusdef_rec *clusDef)
       /*@globals nadc_stat, nadc_err_stack;@*/
       /*@modifies clusDef, nadc_stat, nadc_err_stack@*/;

extern bool CLUSDEF_DB_EXISTS(void);
extern bool CLUSDEF_MTBL_VALID(unsigned char, unsigned short);
extern unsigned short CLUSDEF_DURATION(unsigned char, unsigned short);
extern unsigned short CLUSDEF_NUM_AUX(unsigned char, unsigned short);
extern unsigned short CLUSDEF_NUM_DET(unsigned char, unsigned short);
extern unsigned short CLUSDEF_NUM_PMD(unsigned char, unsigned short);
extern unsigned short CLUSDEF_NUM_DSR(unsigned char, unsigned short);
extern unsigned short CLUSDEF_INTG_MIN(unsigned char, unsigned short);
extern unsigned short CLUSDEF_DSR_SIZE(unsigned char, 
					unsigned short,
					unsigned short);
extern unsigned short CLUSDEF_CLCON(unsigned char, unsigned short,
				     /*@out@*/ struct clusdef_rec *);

#if defined _STDIO_H || defined _STDIO_H_
extern void SCIA_LV0_RD_SPH(FILE *fd, const struct mph_envi,
			     /*@out@*/ struct sph0_scia *sph)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *sph@*/;
extern void SCIA_LV0_WR_SPH(FILE *fd, const struct mph_envi,
			     const struct sph0_scia sph)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/;

extern size_t SCIA_LV0_RD_MDS_INFO(FILE *fd, unsigned int, 
				    const struct dsd_envi *,
				    /*@out@*/ struct mds0_states **states)
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *states@*/;

extern void SCIA_LV0_FREE_MDS_INFO(size_t, /*@only@*/ struct mds0_states *);

extern unsigned int GET_SCIA_LV0_MDS_INFO(FILE *fd, const struct dsd_envi *, 
					   struct mds0_info *info)
       /*@globals  errno, stderr, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, stderr, nadc_stat, nadc_err_stack, fd, info@*/;

extern unsigned short SCIA_LV0_RD_AUX(FILE *fd, const struct mds0_info *,
				       unsigned short,
				       /*@out@*/ struct mds0_aux **aux_out)
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *aux_out@*/;
extern unsigned short SCIA_LV0_WR_AUX(FILE *fd, const struct mds0_info *,
				       unsigned short, const struct mds0_aux *)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/;
extern unsigned int GET_SCIA_LV0_STATE_AUX(FILE *, const struct mds0_states *,
					   struct mds0_aux **aux_out)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *aux_out@*/;

extern unsigned short SCIA_LV0_RD_DET(FILE *fd, const struct mds0_info *,
				      unsigned short,
				      /*@out@*/ struct mds0_det **det_out)
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *det_out@*/;

extern unsigned short SCIA_LV0_WR_DET(FILE *fd, const struct mds0_info *,
				       unsigned short, const struct mds0_det *)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/;
extern unsigned int GET_SCIA_LV0_STATE_DET(FILE *, 
					   const struct mds0_states *,
					   struct mds0_det **det_out)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, det_out@*/;

extern unsigned short SCIA_LV0_RD_PMD(FILE *fd, const struct mds0_info *,
				       unsigned short,
				       /*@out@*/ struct mds0_pmd **pmd_out)
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *pmd_out@*/;
extern unsigned short SCIA_LV0_WR_PMD(FILE *fd, const struct mds0_info *,
				      unsigned short, const struct mds0_pmd *)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/;
extern unsigned int GET_SCIA_LV0_STATE_PMD(FILE *, const struct mds0_states *,
					   struct mds0_pmd **pmd_out)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, pmd_out@*/;

extern void SCIA_LV0_RD_LV1_AUX(FILE *fd, /*@out@*/ struct mds1_aux *aux)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *aux@*/;
extern unsigned int SCIA_LV0_WR_LV1_AUX(FILE *fd, const struct mds1_aux)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/;

extern void SCIA_LV0_RD_LV1_PMD(FILE *fd, /*@out@*/ struct mds1_pmd *pmd)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *pmd@*/;
extern unsigned int SCIA_LV0_WR_LV1_PMD(FILE *fd, const struct mds1_pmd)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/;

extern void SCIA_LV1_WR_ASCII_AUX(unsigned int, const struct mds1_aux *)
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, fd@*/;
extern void SCIA_LV1_WR_ASCII_PMD(unsigned int, const struct mds1_pmd *)
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, fd@*/;
#endif   /* ---- defined _STDIO_H || defined _STDIO_H_ ----- */

extern double GET_SCIA_LV0_MDS_TIME(int, const void *);
extern void GET_SCIA_LV0_MDS_TIME_ARR(int, const void *, /*@out@*/ double *);
extern void GET_SCIA_LV0_MDS_ANGLES(const struct mds0_aux *, 
				       /*@out@*/ float *, /*@out@*/ float *);
extern void GET_SCIA_LV0_STATE_ANGLE(unsigned short, 
					const struct mds0_aux *, 
					/*@out@*/ float *, /*@out@*/ float *);
extern void GET_SCIA_LV0_STATE_OBMtemp(bool, unsigned short, 
					  const struct mds0_aux *, 
					  /*@out@*/ float *);
extern void GET_SCIA_LV0_STATE_DETtemp(unsigned short, 
					const struct mds0_det *,
					/*@out@*/ float *);
extern void GET_SCIA_LV0_STATE_PMDtemp(unsigned short, 
					const struct mds0_pmd *, 
					/*@out@*/ float *);
extern void GET_SCIA_LV0_DET_PET(struct chan_hdr, 
				  /*@out@*/ float *, 
				  /*@out@*/ unsigned short *);
extern unsigned short GET_SCIA_LV0C_MDS(const unsigned int, 
					 const struct mds0_det *, 
					 /*@out@*/ struct mds1c_scia **mds)
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, *mds@*/;

extern void SCIA_LV0_WR_ASCII_SPH(const struct sph0_scia *)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;

extern void SCIA_LV0_WR_ASCII_INFO(size_t, const struct mds0_states *)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;

extern void SCIA_LV0_FREE_MDS_DET(unsigned short, 
                                   /*@only@*/ struct mds0_det *);

extern size_t SCIA_LV0_SELECT_MDS(size_t, const struct mds0_states *,
			  /*@null@*/ /*@out@*/ struct mds0_states **states)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, *states@*/;

extern void SCIA_LV0_WR_ASCII_AUX(unsigned int, unsigned short,
				  const struct mds0_aux *)
      /*@globals  errno, nadc_stat, nadc_err_stack;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack@*/;
 
extern void SCIA_LV0_WR_ASCII_DET(unsigned int, unsigned short,
				  const struct mds0_det *)
      /*@globals  nadc_stat, nadc_err_stack;@*/
      /*@modifies nadc_stat, nadc_err_stack@*/;
 
extern void SCIA_LV0_WR_ASCII_PMD(unsigned int, unsigned short,
				  const struct mds0_pmd *)
      /*@globals  nadc_stat, nadc_err_stack;@*/
      /*@modifies nadc_stat, nadc_err_stack@*/;
 
#ifdef _HDF5_H
extern void SCIA_LV0_WR_H5_SPH(const struct sph0_scia *)
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;
extern void SCIA_LV0_WR_H5_AUX(unsigned short, unsigned short,
			       const struct mds0_aux *)
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;
extern void SCIA_LV0_WR_H5_DET(unsigned short, unsigned short,
			       const struct mds0_det *)
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;
extern void SCIA_LV0_WR_H5_PMD(unsigned short, unsigned short,
			       const struct mds0_pmd *)
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;
#endif /* _HDF5_H */

#ifdef LIBPQ_FE_H
extern void SCIA_LV0_WR_SQL_META(PGconn *conn, const char *, 
                                  const struct mph_envi *)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void SCIA_LV0_MATCH_STATE(PGconn *conn, const struct mph_envi *,
				 unsigned short, const struct mds0_sql *)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void SCIA_LV0_DEL_ENTRY(PGconn *conn, const char *)
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
#endif /* LIBPQ_FE_H */

#ifdef __cplusplus
  }
#endif
#endif /* __NADC_SCIA_LV0 */
