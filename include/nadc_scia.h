/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 1999 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_SCIA
.AUTHOR      R.M. van Hees 
.KEYWORDS    header file
.LANGUAGE    ANSI-C
.PURPOSE     macros and structures for SCIAMACHY data modules
.ENVIRONment none
.VERSION      4.2   06-Oct-2004	added coadd-factor to struct mds1c_scia, RvH
              4.1   31-Mar-2003	modified include for C++ code
              4.0   13-Sep-2001	separate 0/1b/2 specific definitions, RvH 
              3.0   29-Aug-2001 combines level 1b and 2 definitions, RvH
              2.0   04-Jan-2001 Updated to format: DDT v2.3+3 Ev 2.0, RvH
              1.0   12-Aug-1999 Creation by R.M. van Hees
------------------------------------------------------------*/
#ifndef  __NADC_SCIA                            /* Avoid redefinitions */
#define  __NADC_SCIA

#include <nadc_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/*+++++ Macros +++++*/
#define NUM_CORNERS         4

/* maximum number of cluster definitions (?) */
#define MAX_CLUSTER    ((unsigned short) 64)

/* value for mds_flag when a MDS is attached */
#define MDS_ATTACHED   ((unsigned char) 0)

/* general Sciamachy Instrument definitions */
#define SCIENCE_CHANNELS    8
#define VIS_CHANNELS        5
#define IR_CHANNELS         3
#define FirstInfraChan ((unsigned char) (SCIENCE_CHANNELS-IR_CHANNELS+1))

#define PMD_NUMBER          7
#define IR_PMD_NUMBER       2

#define CHANNEL_SIZE        1024
#define ALL_CHANNELS        (SCIENCE_CHANNELS + PMD_NUMBER)
#define SCIENCE_PIXELS      (SCIENCE_CHANNELS * CHANNEL_SIZE)
#define VIS_SCIENCE_PIXELS  (VIS_CHANNELS * CHANNEL_SIZE)
#define IR_SCIENCE_PIXELS   (IR_CHANNELS * CHANNEL_SIZE)

#define NUM_FRAC_POLV       12
#define NUM_SPEC_COEFFS     5

/* flags for SCIAMACHY spectral band selection */
#define BAND_NONE     ((unsigned char) 0x0U)
#define BAND_ONE      ((unsigned char) 0x1U)
#define BAND_TWO      ((unsigned char) 0x2U)
#define BAND_THREE    ((unsigned char) 0x4U)
#define BAND_FOUR     ((unsigned char) 0x8U)
#define BAND_FIVE     ((unsigned char) 0x10U)
#define BAND_SIX      ((unsigned char) 0x20U)
#define BAND_SEVEN    ((unsigned char) 0x40U)
#define BAND_EIGHT    ((unsigned char) 0x80U)
#define BAND_ALL      ((unsigned char) ~0x0U)

/* Scia Lv0 calibration options (avoid conflicts with Lv1b options!) */
#define DO_CORR_COADDF   (0x10000000U)
#define DO_CORR_NORM     (0x20000000U)

/* Scia Lv1b calibration options */
#define CALIB_NONE       (0x0U)
#define DO_CORR_VIS_MEM  (0x1U)
#define DO_SRON_MEM_NLIN (0x2U)
#define DO_CORR_IR_NLIN  (0x4U)
#define DO_SRON_DARK     (0x8U)
#define DO_CORR_AO       (0x10U)
#define DO_CORR_DARK     (0x20U)
#define DO_CORR_VDARK    (0x40U)
#define DO_CORR_VSTRAY   (0x80U)
#define DO_CORR_ADARK    (0x100U)
#define DO_CORR_LDARK    (0x200U)
#define DO_SRON_PPG      (0x400U)
#define DO_CORR_PPG      (0x800U)
#define DO_FIXED_PPG     (0x20000U)
#define DO_CORR_ETALON   (0x1000U)
#define DO_SRON_STRAY    (0x2000U)
#define DO_CORR_STRAY    (0x4000U)
#define DO_SRON_WAVE     (0x8000U)
#define DO_CALIB_WAVE    (0x10000U)
#define DO_CORR_POL      (0x40000U)
#define DO_SRON_RAD      (0x80000U)
#define DO_CORR_RAD      (0x100000U)
#define DO_SRON_BDPM     (0x200000U)
#define DO_MASK_BDPM     (0x400000U)
#define DO_DIVIDE_SUN    (0x800000U)
#define DO_SRON_SUN      (0x1000000U)
#define DO_CALC_ERROR    (0x2000000U)
#define DO_SRON_NOISE    (0x4000000U)
#define DO_SRON_TRANS    (0x8000000U)
#define DO_PATCH_L1C     (0x10000000U)
  /* KB: third possibility to DO_SRON_RAD/DO_CORR_RAD */
#define DO_KEYDATA_RAD   (0x20000000U)
  /* KB: Apply m-factors in applicator */
#define DO_MFACTOR_RAD   (0x40000000U)
#define DO_MFAC_H5_RAD   (0x80000000U)

#define SCIA_ATBD_CALIB  (DO_CORR_VIS_MEM|DO_CORR_IR_NLIN|\
                          DO_CORR_AO|DO_CORR_DARK|DO_CORR_VDARK|\
                          DO_CORR_PPG|DO_CORR_ETALON|DO_CORR_STRAY|\
                          DO_CALIB_WAVE|DO_CORR_POL|DO_CORR_RAD|DO_MASK_BDPM)

#define SCIA_SRON_CALIB  (DO_CORR_VIS_MEM|DO_CORR_IR_NLIN|DO_SRON_MEM_NLIN|\
                          DO_CORR_AO|DO_CORR_DARK|DO_CORR_VDARK|DO_SRON_DARK|\
                          DO_CORR_STRAY|DO_CORR_PPG|DO_SRON_PPG|\
                          DO_CORR_ETALON|\
                          DO_CALIB_WAVE|DO_SRON_WAVE|\
                          DO_CORR_POL|DO_CORR_RAD|\
                          DO_MASK_BDPM|DO_SRON_BDPM)

/* Scia Lv1 patch calibration options */
#define SCIA_PATCH_ID    "B"
#define SCIA_PATCH_NONE  ((unsigned short) 0x0U)
#define SCIA_PATCH_MEM   ((unsigned short) 0x1U)
#define SCIA_PATCH_NLIN  ((unsigned short) 0x2U)
#define SCIA_PATCH_DARK  ((unsigned short) 0x4U)
#define SCIA_PATCH_PPG   ((unsigned short) 0x8U)
#define SCIA_PATCH_STRAY ((unsigned short) 0x10U)
#define SCIA_PATCH_BASE  ((unsigned short) 0x20U)
#define SCIA_PATCH_SUN   ((unsigned short) 0x40U)
#define SCIA_PATCH_POL   ((unsigned short) 0x80U)
#define SCIA_PATCH_RAD   ((unsigned short) 0x100U)
#define SCIA_PATCH_BDPM  ((unsigned short) 0x200U)
#define SCIA_PATCH_ALL   (SCIA_PATCH_MEM|SCIA_PATCH_NLIN|SCIA_PATCH_DARK|\
                          SCIA_PATCH_STRAY|SCIA_PATCH_BASE|SCIA_PATCH_POL|\
                          SCIA_PATCH_BDPM)

/*+++++ Structures & Unions +++++*/
enum scia_state { SCIA_LVL0 = 0, 
		  SCIA_NADIR, SCIA_LIMB, SCIA_OCCULT, SCIA_MONITOR 
};

enum scia_polV { PMD_THEORY = 0,
		 PMD_1, PMD_2, PMD_3, PMD_4, PMD_5, PMD_6, 
		 PMD_OVL1, PMD_OVL2, PMD_OVL3, PMD_OVL4, PMD_OVL5 
};

enum pixel_type_nadir { BACK_SCAN = 0, FORWARD_SCAN, NO_PIXEL_TYPE = 255 
};

enum scia_q_sost { SCIA_Q_OK = 0,
		   SCIA_Q_DECON, SCIA_Q_RECOVER, SCIA_Q_UNAVAIL, 
		   SCIA_Q_UNKOWN = 255
};

#define ALONG_TANG_HGHT ((unsigned char) 0x0U)
#define NEW_TANG_HGHT   ((unsigned char) 0x1U)
#define DEEP_SPACE      ((unsigned char) 0x2U)

/*+++++ Global Variables +++++*/
/* 
 * Most routines to read SCIAMACHY data can allocate memory internally
 * However IDL requires the use of their own memory allocation routines
 */
extern bool Use_Extern_Alloc;

/*+++++ Structures & Unions +++++*/
struct scale_rec {
     float offs;
     float scale;
};

struct scia_memcorr {
     size_t dims[2];
     float **matrix;
};

struct scia_nlincorr {
     size_t dims[2];
     char  *curve;
     float **matrix;
};

struct scia_straycorr {
     size_t dims[2];
     float *grid_in;
     float *grid_out;
     float **matrix;
};

/*
 * basic SCIAMACHY data structures
 */
struct geoL_scia
{
     unsigned char pixel_type;
     unsigned char glint_flag;
     float pos_esm;
     float pos_asm;
     float sat_h;
     float earth_rad;
     float dopp_shift;
     float sun_zen_ang[3];
     float sun_azi_ang[3];
     float los_zen_ang[3];
     float los_azi_ang[3];
     float tan_h[3];
     struct coord_envi sub_sat_point;
     struct coord_envi tang_ground_point[3];
};

struct geoN_scia
{
     unsigned char pixel_type;
     unsigned char glint_flag;
     float pos_esm;
     float sat_h;
     float earth_rad;
     float sun_zen_ang[3];
     float sun_azi_ang[3];
     float los_zen_ang[3];
     float los_azi_ang[3];
     struct coord_envi sub_sat_point;
     struct coord_envi corner[NUM_CORNERS];
     struct coord_envi center;
};

struct geoC_scia
{
     float pos_esm;
     float pos_asm;
     float sun_zen_ang;
     struct coord_envi sub_sat_point;
};

struct gdf_para {
     float p_bar;
     float beta;
     float w0;
};

struct polV_scia
{
     float Q[NUM_FRAC_POLV];
     float error_Q[NUM_FRAC_POLV];
     float U[NUM_FRAC_POLV];
     float error_U[NUM_FRAC_POLV];
     float rep_wv[NUM_FRAC_POLV+1];
     struct gdf_para gdf;
     unsigned short intg_time;
};

/* common SCIA PDS data structures */
struct lads_scia
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     struct coord_envi corner[NUM_CORNERS];
};

struct mds1c_scia
{
     struct mjd_envi  mjd;
     signed char      rad_units_flag;
     signed char      quality_flag;
     unsigned char    type_mds;
     unsigned char    coaddf;
     unsigned char    category;
     unsigned char    state_id;
     unsigned char    state_index;
     unsigned char    chan_id;
     unsigned char    clus_id;
     unsigned short   dur_scan;
     unsigned short   num_obs;
     unsigned short   num_pixels;
     unsigned int     dsr_length;
     float            orbit_phase;
     float            pet;
     unsigned short   *pixel_ids;     /* num_pixels */
     float            *pixel_wv;      /* num_pixels */
     float            *pixel_wv_err;  /* num_pixels */
     float            *pixel_val;     /* num_obs * num_pixels */
     float            *pixel_err;     /* num_obs * num_pixels */
     struct geoC_scia *geoC;          /* num_obs, Monitoring MDS only  */
     struct geoL_scia *geoL;          /* num_obs, Limb/Occultation MDS only  */
     struct geoN_scia *geoN;          /* num_obs, Nadir MDS only  */
};

struct mds1c_pmd
{
     struct mjd_envi  mjd;
     signed char      quality_flag;
     unsigned char    type_mds;
     unsigned char    category;
     unsigned char    state_id;
     unsigned char    state_index;
     unsigned short   dur_scan;
     unsigned short   num_pmd;
     unsigned short   num_geo;
     unsigned int     dsr_length;
     float            orbit_phase;
     float            *int_pmd;       /* num_pmd */
     struct geoL_scia *geoL;          /* num_obs, Limb/Occultation MDS only  */
     struct geoN_scia *geoN;          /* num_obs, Nadir MDS only  */
};

struct mds1c_polV
{
     struct mjd_envi  mjd;
     signed char      quality_flag;
     unsigned char    type_mds;
     unsigned char    category;
     unsigned char    state_id;
     unsigned char    state_index;
     unsigned short   dur_scan;
     unsigned short   total_polV;
     unsigned short   num_diff_intg;
     unsigned short   num_geo;
     unsigned int     dsr_length;
     float            orbit_phase;
     unsigned short   intg_times[MAX_CLUSTER];
     unsigned short   num_polar[MAX_CLUSTER];
     struct polV_scia *polV;          /* total_num_polV */
     struct geoL_scia *geoL;          /* num_obs, Limb/Occultation MDS only  */
     struct geoN_scia *geoN;          /* num_obs, Nadir MDS only  */
};

struct dmop_rec {
     char timeLine[8];
     char dateTimeStart[24];
     char dateTimeStop[24];
     unsigned int   muSecStart;
     unsigned int   muSecStop;
     float          orbitPhase;
     unsigned char  stateID;
     unsigned short absOrbit;
};

#ifdef _WITH_SQL
struct stateinfo_rec {
     char           softVersion[4];
     unsigned char  stateID;
     unsigned short indxState;
     unsigned int   indxDMOP;
     double         jday;
     double         dtMatch;
};

struct mds0_sql {
     struct mjd_envi mjd;
     unsigned char   stateID;
     unsigned short  nrAux;
     unsigned short  nrDet;
     unsigned short  nrPMD;
     float           obmTemp;
     float           pmdTemp;
     float           chanTemp[SCIENCE_CHANNELS];
};
#endif

/*
 * define scale/offset values for memory/non-linearity correction values
 */
#ifdef _MEMNLIN_CORR_OLD
static
const struct scale_rec nadc_scale[SCIENCE_CHANNELS] = {
     {-37.f, 1.25f}, {-37.f, 1.25f}, {-37.f, 1.25f}, {-37.f, 1.25f},
     {-37.f, 1.25f}, {-102.f, 1.25f}, {126.f, 1.5f}, {126.f, 1.25f}
};

static
const struct scale_rec atbd_scale[SCIENCE_CHANNELS] = {
     {0.f, 2.f}, {0.f, 2.f}, {0.f, 2.f}, {0.f, 2.f}, {0.f, 2.f},
     {-102.f, 1.25f}, {126.f, 1.5f}, {126.f, 1.25f}
};
#endif

/*
 * prototype declarations of Sciamachy functions 
 */
extern unsigned int GET_SCIA_MAGIC_ID( const char * )
       /*@globals errno;@*/
       /*@modifies errno@*/;

extern unsigned short GET_SCIA_QUALITY( int, /*@null@*/ /*@out@*/ int *period );

extern unsigned char GET_SCIA_MDS_TYPE( unsigned char stateID );

extern void GET_SCIA_ROE_INFO( bool, const double, /*@out@*/ int *, 
			       /*@out@*/ bool *, /*@out@*/ float * );
extern double GET_SCIA_ROE_JDAY( unsigned short );

extern void GET_SCIA_MEAN_STATE( bool, 
				 unsigned short, const struct mds1c_scia *,
				 /*@out@*/ unsigned short *, 
				 /*@out@*/ unsigned char *,  /*@out@*/ float *,
				 /*@out@*/ float *, /*@out@*/ float * );

extern void SCIA_LV1C_FREE_MDS( int, unsigned int, 
				/*@only@*/ struct mds1c_scia * );
extern void SCIA_LV1C_FREE_MDS_PMD( int, /*@only@*/ struct mds1c_pmd * );
extern void SCIA_LV1C_FREE_MDS_POLV( int, /*@only@*/ struct mds1c_polV * );

extern void SCIA_LV1C_CAL( int, unsigned int, unsigned short, 
			   struct mds1c_scia *mds_out )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, mds_out->pixel_val@*/;

extern void SCIA_SET_CALIB( /*@notnull@*/ const char *, 
                            /*@out@*/ unsigned int * );
extern void SCIA_GET_CALIB( unsigned int, /*@out@*/ char * );

extern void SCIA_SET_PATCH( /*@notnull@*/ const char *, 
                            /*@out@*/ unsigned short * );
extern void SCIA_GET_PATCH( unsigned short, /*@out@*/ char * );

extern void SCIA_SET_PARAM( int, char **, int,
                             /*@out@*/ struct param_record *param )
     /*@globals  errno, stderr, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, stderr, nadc_stat, nadc_err_stack, param@*/;
extern void SCIA_SHOW_PARAM( int, struct param_record );

#if defined _STDIO_INCLUDED || defined _STDIO_H || defined __STDIO_H__
extern void SCIA_SHOW_VERSION( FILE *stream, const char * )
     /*@modifies stream@*/;
extern void SCIA_SHOW_CALIB( FILE *stream )
     /*@modifies stream@*/;
extern void SCIA_SHOW_PATCH( FILE *stream )
     /*@modifies stream@*/;

extern unsigned int SCIA_RD_LADS( FILE *fp, unsigned int, 
				  const struct dsd_envi *,
				  /*@out@*/ struct lads_scia **lads )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *lads@*/;
#endif   /* ---- defined _STDIO_INCLUDED || defined _STDIO_H ----- */

extern void SCIA_WR_ASCII_LADS( struct param_record, unsigned int, 
				const struct lads_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;

#ifdef _HDF5_H
extern void SCIA_WR_H5_VERSION( hid_t )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern hid_t SCIA_CRE_H5_FILE( int instrument, const struct param_record * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern void SCIA_WR_H5_MPH( struct param_record, const struct mph_envi * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern unsigned int SCIA_RD_H5_LADS( struct param_record, struct lads_scia * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern void SCIA_WR_H5_LADS( struct param_record, unsigned int,
			     const struct lads_scia * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
#endif /* _HDF5_H */

#ifdef _SCIA_LEVEL_0
#include <nadc_scia_lv0.h>
#endif
#if (defined _SCIA_LEVEL_1) || (defined _SCIA_PATCH_1)
#include <nadc_scia_lv0.h>
#include <nadc_scia_lv1.h>
#endif
#ifdef _SCIA_LEVEL_2
#include <nadc_scia_lv2.h>
#endif

#ifdef __cplusplus
  }
#endif
#endif /* __NADC_SCIA */
