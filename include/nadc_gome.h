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

.IDENTifer   NADC_GOME
.AUTHOR      R.M. van Hees 
.KEYWORDS    header file
.LANGUAGE    ANSI-C
.PURPOSE     macros and structures for GOME level 1b and 2 data modules
.ENVIRONment none
.VERSION      3.0   08-Jun-2009 update to product version 2, RvH
              2.2   31-Mar-2003	modified include for C++ code
              2.1   06-Mar-2003	added global "Use_Extern_Alloc", RvH 
              2.0   21-Aug-2001 combines level 1b and 2 definitions, RvH
              1.1   12-Jul-2001 added a few handy enumerations, RvH
              1.0   02-Feb-1999 created by R.M. van Hees
------------------------------------------------------------*/

#ifndef  __NADC_GOME                            /* Avoid redefinitions */
#define  __NADC_GOME

#include <nadc_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/*+++++ Macros +++++*/
#define GOME_CHAR   ((size_t) 1)
#define GOME_SHORT  ((size_t) 2)
#define GOME_USHRT  ((size_t) 2)
#define GOME_INT    ((size_t) 4)
#define GOME_UINT   ((size_t) 4)
#define GOME_FLOAT  ((size_t) 4)
#define GOME_DBLE   ((size_t) 8)

#define LVL1_PIR_LENGTH           38
#define LVL2_FSR_LENGTH           12
#define LVL1_FSR_LENGTH           96
#define LVL2_SPH_LENGTH          127
#define LVL2_GLR_LENGTH          136
#define LVL2_IRR_STATIC_LENGTH   142
#define LVL2_DDR_STATIC_LENGTH     8
#define LVL2_DDR_NWIN_LENGTH      28
#define LVL2_DDR_NMOL_LENGTH      24

#define LVL2_MAX_NWIN              5
#define LVL2_MAX_NMOL             10

#define LVL2_V2_NWIN               2
#define LVL2_V2_NMOL               2

#define MPH_SIZE                  34
#define SPH_SIZE                  22
#define IHR_SIZE                 198

#define PRE_DISP_TEMP_ENTRY      148       /* prism temp entry point in IHR */

#define NUM_COORDS                 5     /* 4 corner and center coordinates */

#define NUM_STRAY_GHOSTS           2
#define NUM_SPEC_COEFFS            5
#define NUM_POLAR_COEFFS  ((short) 8)
#define NUM_FPA_SCALE              5
#define NUM_FPA_COEFFS           100

#define NUM_SPEC_BANDS            10 /* incl. blind and stray */
#define NUM_BAND_IN_CHANNEL        2
#define PMD_NUMBER                 3
#define PMD_IN_GRID               16
#define SCIENCE_CHANNELS           4
#define CHANNEL_SIZE            1024
#define SCIENCE_PIXELS      (SCIENCE_CHANNELS * CHANNEL_SIZE)

#define FLAG_UNSET          ((unsigned char) 0)
#define FLAG_EARTH          ((unsigned char) 1)
#define FLAG_MOON           ((unsigned char) 2)
#define FLAG_SUN            ((unsigned char) 3)

/* flags for GOME spectral band selection */
#define BAND_NONE           ((unsigned char) 0x0U)

#define BAND_ONE_A          ((unsigned char) 0x1U)
#define BAND_ONE_B          ((unsigned char) 0x1U)
#define BAND_TWO_A          ((unsigned char) 0x2U)
#define BAND_TWO_B          ((unsigned char) 0x1U)
#define BAND_THREE          ((unsigned char) 0x4U)
#define BAND_FOUR           ((unsigned char) 0x8U)
#define BAND_ALL            ((unsigned char) ~0x0U)

/* flags for ground pixel selection */
#define SUBSET_NONE         ((unsigned char) 0x0U)
#define SUBSET_EAST         ((unsigned char) 0x1U)
#define SUBSET_CENTER       ((unsigned char) 0x2U)
#define SUBSET_WEST         ((unsigned char) 0x4U)
#define SUBSET_BACK         ((unsigned char) 0x8U)
#define SUBSET_ALL          (SUBSET_EAST|SUBSET_CENTER|SUBSET_WEST|SUBSET_BACK)

/* flags for calibration selection */
#define CALIB_NONE       (0x0U)

#define GOME_CAL_LEAK    ((unsigned short) 0x1U)
#define GOME_CAL_FPA     ((unsigned short) 0x2U)/* only Earth data (band 1a) */
#define GOME_CAL_BSDF    ((unsigned short) 0x2U)          /* only Solar data */
#define GOME_CAL_FIXED   ((unsigned short) 0x4U)
#define GOME_CAL_STRAY   ((unsigned short) 0x8U)
#define GOME_CAL_NORM    ((unsigned short) 0x10U)
#define GOME_CAL_ALBEDO  ((unsigned short) 0x20U)
#define GOME_CAL_POLAR   ((unsigned short) 0x40U)
#define GOME_CAL_INTENS  ((unsigned short) 0x80U)
#define GOME_CAL_JUMPS   ((unsigned short) 0x100U)
#define GOME_CAL_AGING   ((unsigned short) 0x200U)
#define GOME_CAL_UNIT    ((unsigned short) 0x400U)

#define GOME_CAL_PMD     (GOME_CAL_LEAK|GOME_CAL_POLAR|GOME_CAL_INTENS)
#define GOME_CAL_EARTH   (GOME_CAL_LEAK|GOME_CAL_FPA|GOME_CAL_FIXED|\
                          GOME_CAL_STRAY|GOME_CAL_NORM|GOME_CAL_POLAR|\
                          GOME_CAL_INTENS)
#define GOME_CAL_MOON    (GOME_CAL_LEAK|GOME_CAL_FPA|GOME_CAL_FIXED|\
                          GOME_CAL_STRAY|GOME_CAL_NORM|GOME_CAL_INTENS)
#define GOME_CAL_SUN     (GOME_CAL_LEAK|GOME_CAL_FIXED|GOME_CAL_STRAY|\
                          GOME_CAL_NORM|GOME_CAL_BSDF|GOME_CAL_INTENS)

/*+++++ Global Variables +++++*/
/* 
 * Most routines to read SCIAMACHY data can allocate memory internally
 * However IDL requires the use of their own memory allocation routines
 */
extern bool Use_Extern_Alloc;

/*+++++ Enumarations +++++*/
enum polar_coeffs { PC_PMD_1 = 0,
		    PC_PMD_2,
		    PC_PMD_3,
		    PC_OVERLAP_1,
		    PC_OVERLAP_2,
		    PC_OVERLAP_3,
		    PC_P7,
		    PC_P7_BAND_1A 
};

enum gome_bands { BAND_1a = 0, 
		  BAND_1b, 
		  BAND_2a, 
		  BAND_2b, 
		  BAND_3, 
		  BAND_4,
		  BLIND_1a, 
		  STRAY_1a, 
		  STRAY_1b, 
		  STRAY_2a 
};

/*+++++ Structures & Unions +++++*/
struct mjd_gome
{
     unsigned int days;   /* UTC days since 1.1.1950 */
     unsigned int msec;   /* UTC ms since midnight */
};

struct pir_gome
{
     char mission[3];
     char sensor[4];
     char orbit[6];
     char nr_orbit[5];
     char acquis[3];
     char product[6];
     char blank[2];
     char proc_id[3];
     char proc_date[9];
     char proc_time[7];
};

struct fsr1_gome
{
     short nr_sph;
     int   sz_sph;
     short nr_fcd;
     int   sz_fcd;
     short nr_pcd;
     int   sz_pcd;
     short nr_scd;
     int   sz_scd;
     short nr_mcd;
     int   sz_mcd;
     short nr_band[NUM_SPEC_BANDS];
     int   sz_band[NUM_SPEC_BANDS];
};

struct fsr2_gome
{
     short nr_sph;
     int   sz_sph;
     short nr_ddr;
     int   sz_ddr;
};

struct sph1_gome
{
     short nr_inref;
     char  inref[2][39];
     char  soft_version[6];
     char  calib_version[6];
     short prod_version;
     unsigned int time_orbit;
     unsigned int time_utc_day;
     unsigned int time_utc_ms;
     unsigned int time_counter;
     unsigned int time_period;
     short pmd_entry;
     short subset_entry;
     short intgstat_entry;
     short peltier_entry;
     short status2_entry;
     float pmd_conv[2 * PMD_NUMBER];
     unsigned int state_utc_day;
     unsigned int state_utc_ms;
     unsigned int state_orbit;
     float  state_x;
     float  state_y;
     float  state_z;
     float  state_dx;
     float  state_dy;
     float  state_dz;
     double att_yaw;
     double att_pitch;
     double att_roll;
     double att_dyaw;
     double att_dpitch;
     double att_droll;
     int    att_flag;
     int    att_stat;
     double julian;
     double semi_major;
     double excen;
     double incl;
     double right_asc;
     double perigee;
     double mn_anom;
     struct mjd_gome start_time;
     struct mjd_gome stop_time;
};

struct sph2_gome
{
     char  inref[39];
     char  soft_version[6];
     char  param_version[6];
     char  format_version[6];
     short nwin;
     short nmol;
     float height;
     float window[LVL2_MAX_NWIN];
     short mol_win[LVL2_MAX_NMOL];
     char  mol_name[LVL2_MAX_NMOL][6];
     struct mjd_gome start_time;
     struct mjd_gome stop_time;
};

struct glr1_gome
{
     char  sun_glint;
     unsigned char subsetcounter;
     unsigned int utc_date;
     unsigned int utc_time;
     float sat_geo_height;
     float earth_radius;
     float sun_zen_sat_north[3];
     float sun_azim_sat_north[3];
     float los_zen_sat_north[3];
     float los_azim_sat_north[3];
     float sun_zen_sat_ers[3];
     float sun_azim_sat_ers[3];
     float los_zen_sat_ers[3];
     float los_azim_sat_ers[3];
     float sun_zen_surf_north[3];
     float sun_azim_surf_north[3];
     float los_zen_surf_north[3];
     float los_azim_surf_north[3];
     float lon[NUM_COORDS];
     float lat[NUM_COORDS];
};

struct glr2_gome
{
     int   pixel_nr;
     int   subsetcounter;
     unsigned int utc_date;
     unsigned int utc_time;
     float sat_geo_height;
     float earth_radius;
     float sat_zenith[3];
     float sat_sight[3];
     float sat_azim[3];
     float toa_zenith[3];
     float toa_sight[3];
     float toa_azim[3];
     float lon[NUM_COORDS];
     float lat[NUM_COORDS];
};

struct cr1_gome
{
     short  mode;
     short  type;
     float  surfaceHeight;
     float  fraction;
     float  fractionError;
     float  albedo;
     float  albedoError;
     float  height;
     float  heightError;
     float  thickness;
     float  thicknessError;
     float  topPress;
     float  topPressError;
};

union quality_fcd
{
     struct fcd_breakout
     {
	  unsigned int array_1:2;
	  unsigned int array_2:2;
	  unsigned int array_3:2;
	  unsigned int array_4:2;
	  unsigned int pmd_1:1;
	  unsigned int pmd_2:1;
	  unsigned int pmd_3:1;
	  unsigned int spare:5;
     } flag_fields;

     short flags;
};

struct lv1_bcr
{
     short array_nr;
     short start;
     short end;
};

struct lv1_kde
{
     float bsdf_1[SCIENCE_CHANNELS];
     float bsdf_2[SCIENCE_CHANNELS];
     float resp_1[SCIENCE_CHANNELS];
     float resp_2[SCIENCE_CHANNELS];
     float f2_1[SCIENCE_CHANNELS];
     float f2_2[SCIENCE_CHANNELS];
     float smdep_1[SCIENCE_CHANNELS];
     float smdep_2[SCIENCE_CHANNELS];
     float chi_1[SCIENCE_CHANNELS];
     float chi_2[SCIENCE_CHANNELS];
     float eta_1[SCIENCE_CHANNELS];
     float eta_2[SCIENCE_CHANNELS];
     float ksi_1[SCIENCE_CHANNELS];
     float ksi_2[SCIENCE_CHANNELS];
     float rfs[SCIENCE_PIXELS];
};

struct lv1_ghost
{
     short symmetry[SCIENCE_CHANNELS][NUM_STRAY_GHOSTS];
     short center[SCIENCE_CHANNELS][NUM_STRAY_GHOSTS];
     float defocus[SCIENCE_CHANNELS][NUM_STRAY_GHOSTS];
     float energy[SCIENCE_CHANNELS][NUM_STRAY_GHOSTS];
};

struct lv1_leak
{
     float noise;
     float pmd_offs[PMD_NUMBER];
     float pmd_noise;
     float dark[SCIENCE_PIXELS];
};

struct lv1_hot
{
     short record;
     short array;
     short pixel;
};

struct lv1_spec
{
     double coeffs[SCIENCE_CHANNELS][NUM_SPEC_COEFFS];
     double error[SCIENCE_CHANNELS];
};

struct lv1_calib
{
     float eta_omega[CHANNEL_SIZE];
     float response[CHANNEL_SIZE];
};

struct fcd_gome
{
     union quality_fcd detector_flags;
     short  npeltier;
     short  nleak;
     short  nhot;
     short  nspec;
     short  nang;
     short  width_conv;
     short  indx_spec;
     unsigned int sun_date;
     unsigned int sun_time;
     float  bsdf_0;
     float  elevation;
     float  azimuth;
     float  sun_pmd[PMD_NUMBER];
     float  sun_pmd_wv[PMD_NUMBER];
     float  stray_level[4];
     float  scale_peltier[NUM_FPA_SCALE];
     float  coeffs[8];
     float  filter_peltier[NUM_FPA_COEFFS];
     float  pixel_gain[SCIENCE_PIXELS];
     float  intensity[SCIENCE_PIXELS];
     float  sun_ref[SCIENCE_PIXELS];
     float  sun_precision[SCIENCE_PIXELS];
     struct lv1_ghost ghost;
     struct lv1_kde kde;
     struct lv1_bcr bcr[NUM_SPEC_BANDS];
     struct lv1_leak *leak;
     struct lv1_hot  *hot;
     struct lv1_spec *spec;
     struct lv1_calib *calib;
};

struct polar_gome
{
     float wv[NUM_POLAR_COEFFS];
     float coeff[NUM_POLAR_COEFFS];
     float error[NUM_POLAR_COEFFS];
     float chi;
};

struct mph0_gome
{
     char ProductConfidenceData[3];
     char UTC_MPH_Generation[25];
     char ProcessorSoftwareVersion[9];
};

struct sph0_gome
{
     char sph_5[2];
     char sph_6[21];
};


struct ihr_gome
{
     unsigned short subsetcounter;
     unsigned short prism_temp;
     unsigned short averagemode;
     union {
	  struct {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       unsigned char ch4:2;
	       unsigned char ch3:2;
	       unsigned char ch2b:2;
	       unsigned char ch2a:2;
	       unsigned char ch1b:2;
	       unsigned char ch1a:2;
	       unsigned char fpa2:1;
	       unsigned char fpa3:2;
	       unsigned char fpa4:1;
#else
	       unsigned char fpa4:1;
	       unsigned char fpa3:2;
	       unsigned char fpa2:1;
	       unsigned char ch1a:2;
	       unsigned char ch1b:2;
	       unsigned char ch2a:2;
	       unsigned char ch2b:2;
	       unsigned char ch3:2;
	       unsigned char ch4:2;
#endif	       
	  } stat;
	  unsigned short two_byte;
     } intg;
     unsigned short pmd[PMD_NUMBER][PMD_IN_GRID];
     short peltier[SCIENCE_CHANNELS];
};

struct pmd_gome
{
     struct glr1_gome glr;
     float value[PMD_NUMBER];
};

struct pcd_gome
{
     short selected;
     short indx_spec;
     short indx_leak;
     short indx_bands[NUM_SPEC_BANDS];
     float dark_current;
     float noise_factor;
     struct glr1_gome glr;
     struct cr1_gome  cld;
     struct polar_gome polar;
     struct mph0_gome mph0;
     struct sph0_gome sph0;
     struct ihr_gome  ihr;
     struct pmd_gome  pmd[PMD_IN_GRID];
};

struct smcd_gome
{
     short selected;
     short indx_spec;
     short indx_leak;
     short indx_bands[NUM_SPEC_BANDS];
     unsigned int utc_date;
     unsigned int utc_time;
     float north_sun_zen;
     float north_sun_azim;
     float north_sm_zen;
     float north_sm_azim;
     float sun_or_moon;
     float dark_current;
     float noise_factor;
     struct mph0_gome mph0;
     struct sph0_gome sph0;
     struct ihr_gome ihr;
     struct pmd_gome  pmd[PMD_IN_GRID];
};

union quality_rec
{
     struct rec_breakout
     {
	  unsigned int dead:2;
	  unsigned int hot:2;
	  unsigned int saturate:2;
	  unsigned int spectral:2;
     } flag_fields;

     short flags;
};

struct rec_gome
{
     union quality_rec pixel_flags;
     short indx_psp;
     short indx_pcd;
     float integration[2];
     float wave[CHANNEL_SIZE];
     float data[CHANNEL_SIZE];
};

struct irr1_gome
{
     short indx_vcd;
     short indx_doas;
     short indx_amf;
     short indx_icfa;
     short indx_stats;

     float cloud_frac;
     float cloud_pres;
     float err_cloud_frac;
     float err_cloud_pres;
     float surface_pres;
     float cca_cloud_frac;
     signed char cca_subpixel[16];

     float pmd_avg[3];
     float pmd_sdev[3];
     float pixel_color[16];
     float pixel_gradient;

     float total_vcd[LVL2_MAX_NMOL];
     float error_vcd[LVL2_MAX_NMOL];
     float slant_doas[LVL2_MAX_NMOL];
     float error_doas[LVL2_MAX_NMOL];
     float rms_doas[LVL2_MAX_NWIN];
     float chi_doas[LVL2_MAX_NWIN];
     float fit_doas[LVL2_MAX_NWIN];
     float iter_doas[LVL2_MAX_NWIN];
     float ground_amf[LVL2_MAX_NMOL];
     float cloud_amf[LVL2_MAX_NMOL];
     float intensity_ground[LVL2_MAX_NWIN];
     float intensity_cloud[LVL2_MAX_NWIN];
     float intensity_measured[LVL2_MAX_NWIN];
};

struct irr2_gome
{
     short indx_vcd;
     short indx_doas;
     short indx_amf;

     float ozone_temperature;
     float ozone_ring_corr;
     float ghost_column;
     float cld_frac;
     float error_cld_frac;
     float cld_height;
     float error_cld_height;
     float cld_press;
     float error_cld_press;
     float cld_albedo;
     float error_cld_albedo;
     float surface_height;
     float surface_press;
     float surface_albedo;

     float total_vcd[LVL2_V2_NMOL];
     float error_vcd[LVL2_V2_NMOL];
     float slant_doas[LVL2_V2_NMOL];
     float error_doas[LVL2_V2_NMOL];
     float rms_doas[LVL2_V2_NWIN];
     float chi_doas[LVL2_V2_NWIN];
     float fit_doas[LVL2_V2_NWIN];
     float iter_doas[LVL2_V2_NWIN];
     float ground_amf[LVL2_V2_NMOL];
     float error_ground_amf[LVL2_V2_NMOL];
     float cloud_amf[LVL2_V2_NMOL];
     float error_cloud_amf[LVL2_V2_NMOL];
};

struct ddr_gome
{
     struct glr2_gome glr;
     struct irr1_gome *irr1;
     struct irr2_gome *irr2;
     float ozone;
     float error;
};

/*
 * prototype declarations of GOME functions
 */
extern void GOME_SET_PARAM( int, char **, int, struct param_record *param )
     /*@modifies param@*/;
extern void GOME_SHOW_PARAM( int, const struct param_record );

extern void GOME_GET_VERSION( /*@out@*/ unsigned int *, 
			      /*@out@*/ unsigned int *, 
			      /*@out@*/ unsigned int *, 
			      /*@out@*/ char * );

extern void GOME_LV1_CHK_SIZE( const struct fsr1_gome, const char * );
extern void GOME_LV2_CHK_SIZE( const struct fsr2_gome, const char * );

extern void GOME_SET_CALIB( /*@notnull@*/ const char *, 
			    /*@out@*/ unsigned short * );
extern void GOME_GET_CALIB( unsigned short, /*@out@*/ char * );

#if defined _STDIO_H || defined __STDIO_H__
extern void GOME_SHOW_VERSION( FILE *stream, const char * )
     /*@modifies stream@*/;
extern void GOME_SHOW_CALIB( FILE *stream )
     /*@modifies stream@*/;

extern void GOME_RD_PIR( FILE *fp, /*@out@*/ struct pir_gome *pir )
     /*@globals  errno;@*/
     /*@modifies errno, fp, pir@*/;
extern void GOME_LV1_RD_FSR( FILE *fp, /*@out@*/ struct fsr1_gome *fsr )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp, *fsr@*/;
extern void GOME_LV1_RD_SPH( FILE *fp, const struct fsr1_gome *,
			     /*@out@*/ struct sph1_gome *sph )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp, *sph@*/;
extern short GOME_LV1_RD_FCD( FILE *fp, const struct fsr1_gome *, 
			      /*@out@*/ struct fcd_gome *fcd )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp, *fcd@*/;
extern short GOME_LV1_RD_PCD( FILE *fp, const struct fsr1_gome *, 
			      const struct sph1_gome *,
			      /*@out@*/ struct pcd_gome **pcd )
     /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp, *pcd@*/;
extern short GOME_LV1_RD_SMCD( unsigned char, FILE *fp, 
			       const struct fsr1_gome *,
			       const struct sph1_gome *, 
			       /*@out@*/ struct smcd_gome **smcd )
     /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp, *smcd@*/;
extern short GOME_LV1_RD_BDR( FILE *fp, short, const struct fsr1_gome *,
			      const struct fcd_gome *, 
			      /*@out@*/ struct rec_gome **rec )
     /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp, *rec@*/;
extern void GOME_LV2_RD_FSR( FILE *fp, /*@out@*/ struct fsr2_gome *fsr )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp, *fsr@*/;
extern void GOME_LV2_RD_SPH( FILE *fp, const struct fsr2_gome *,
                             /*@out@*/ struct sph2_gome *sph )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp, *sph@*/;
extern short GOME_LV2_RD_DDR( FILE *fp, const struct fsr2_gome *,
			      const struct sph2_gome *, 
			      /*@out@*/ struct ddr_gome **ddr )
     /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp, *ddr@*/;
#endif   /* ---- defined _STDIO_H || defined __STDIO_H__ ----- */

extern void CALIB_PCD_PMD( unsigned char, unsigned short, 
			   const struct fcd_gome *,
			   short, const short *, struct pcd_gome * );
extern void CALIB_SMCD_PMD( unsigned short, const struct fcd_gome *,
			    short, const short *, struct smcd_gome *  );
extern void CALIB_PCD_BDR( unsigned short, short,
			   const struct fcd_gome *, 
			   short, const struct pcd_gome *, 
			   short, struct rec_gome *rec )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, rec@*/;
extern void CALIB_SMCD_BDR( unsigned short, short,
			    const struct fcd_gome *, 
			    const struct smcd_gome *, 
			    short, struct rec_gome *rec )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, rec@*/;

extern short BandChannel( short, short );
extern int   SELECT_BAND( short, struct param_record, 
			  const struct fcd_gome * )
     /*@globals  nadc_err_stack;@*/
     /*@modifies nadc_err_stack@*/;
extern short SELECT_PCD( struct param_record, const struct pcd_gome * )
     /*@globals  stderr, nadc_err_stack;@*/
     /*@modifies stderr, nadc_err_stack@*/;
extern short SELECT_SMCD( struct param_record, const struct smcd_gome * );
extern short SELECT_DDR( struct param_record, const struct glr2_gome * )
     /*@globals  nadc_err_stack;@*/
     /*@modifies nadc_err_stack@*/;

extern void GOME_LV1_PMD_GEO( unsigned char, short, const short *, 
			     struct pcd_gome *pcd )
     /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, pcd@*/;

extern void PROCESS_PCD_BDR( short, struct param_record, 
			     const struct fsr1_gome *, 
			     const struct fcd_gome *, 
			     short, /*@null@*/ const short *, 
                             /*@null@*/ const struct pcd_gome *, 
		             const struct rec_gome * )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void PROCESS_SMCD_BDR( unsigned char, short, struct param_record, 
			      const struct fcd_gome *, short, const short *,
			      /*@null@*/ const struct smcd_gome *, 
			      const struct rec_gome * )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack@*/;

extern void GOME_LV1_WR_ASCII_FSR( struct param_record, 
				   const struct fsr1_gome * )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void GOME_LV1_WR_ASCII_SPH( struct param_record, 
				   const struct sph1_gome * )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void GOME_LV1_WR_ASCII_FCD( struct param_record, 
				   const struct fcd_gome * )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void GOME_LV1_WR_ASCII_PCD( struct param_record, short, const short *,
				   const struct pcd_gome * )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void GOME_LV1_WR_ASCII_SMCD( unsigned char, struct param_record, short, 
                              const short *, const struct smcd_gome * )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void GOME_LV1_WR_ASCII_REC( unsigned char, short, struct param_record, 
			     short, short, short, const struct rec_gome * )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack@*/;

extern void GOME_LV2_WR_ASCII_FSR( struct param_record, 
				   const struct fsr2_gome * )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void GOME_LV2_WR_ASCII_SPH( struct param_record, 
				   const struct sph2_gome * )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void GOME_LV2_WR_ASCII_DDR( struct param_record, struct sph2_gome,
				   short, const struct ddr_gome * )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack@*/;

#ifdef _HDF5_H
extern void GOME_WR_H5_VERSION( hid_t )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern hid_t GOME_CRE_H5_FILE( int instrument, const struct param_record * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern void CRE_GOME_LV1_H5_STRUCTS( struct param_record );
extern void GOME_WR_H5_PIR( struct param_record, const struct pir_gome * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern void GOME_LV1_WR_H5_SPH( struct param_record, 
				const struct sph1_gome * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern void GOME_LV1_WR_H5_FCD( struct param_record, 
				const struct fcd_gome * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern void GOME_LV1_WR_H5_PCD( struct param_record, 
				short, /*@null@*/ const short *,
				/*@null@*/ const struct pcd_gome * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern void GOME_LV1_WR_H5_SMCD( unsigned char, struct param_record, short,
				 /*@null@*/ const short *, 
				 /*@null@*/ const struct smcd_gome * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern void GOME_LV1_WR_H5_REC( unsigned char, short, struct param_record,
				short, short, short, const struct rec_gome * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;

extern void GOME_LV2_WR_H5_SPH( struct param_record, 
				const struct sph2_gome * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern void GOME_LV2_WR_H5_DDR( struct param_record, 
				short, const struct ddr_gome * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern void GOME_LV2_WR_H5_IRR( struct param_record,
				short, const struct ddr_gome * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
#endif /* _HDF5_H */

#if defined(LIBPQ_FE_H)
extern int GOME_LV1_WR_SQL_META( PGconn *conn, bool, const char *, 
				 const struct sph1_gome *,
				 const struct fsr1_gome * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void GOME_LV1_WR_SQL_TILE( PGconn *conn, bool, int, const char *, short,
				  const short *, const struct pcd_gome * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void GOME_LV1_DEL_ENTRY( PGconn *conn, const char *, const char * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern int GOME_LV2_WR_SQL_META( PGconn *conn, bool, const char *, 
				 const struct sph2_gome *,
				 const struct fsr2_gome * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void GOME_LV2_WR_SQL_TILE( PGconn *conn, bool, int, const char *,
			          short, const struct ddr_gome * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void GOME_LV2_DEL_ENTRY( PGconn *conn, const char *, const char * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
#endif

#ifdef __cplusplus
  }
#endif
#endif /* __NADC_GOME */
