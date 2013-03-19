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

.IDENTifer   nadc_scia_lv1
.AUTHOR      R.M. van Hees 
.KEYWORDS    header file
.LANGUAGE    ANSI-C
.PURPOSE     macros and structures for SCIAMACHY level 1b modules
.ENVIRONment none
.VERSION      1.3   31-Mar-2003	modified include for C++ code
              1.2   07-Mar-2002	added level 0 packet and data header to 
                                mds1_scia, RvH
              1.1   07-Mar-2002	gave MDS structs more logical names, RvH 
              1.0   13-Aug-2001	Creation by R.M. van Hees 
------------------------------------------------------------*/
#ifndef  __NADC_SCIA_LV1                        /* Avoid redefinitions */
#define  __NADC_SCIA_LV1

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DiffOrbitNumber 21

#define NUM_LV0_PMD       6820
#define NUM_LV0_AUX       1666

#define LV1_Clcon_LENGTH    17

#define MaxBoundariesSIP    13

/*+++++ Structures & Unions +++++*/
enum cluster_type { RSIG = 1, RSIGC, ESIG, ESIGC };

/*
 * compound data types
 */
struct Clcon_scia
{
     unsigned char id;
     unsigned char channel;
     unsigned char type;
/*      unsigned char dummy; */
     unsigned short pixel_nr;
     unsigned short length;
     unsigned short intg_time;
     unsigned short coaddf;
     unsigned short n_read;
     float pet;
};

union det_signal
{
     struct signal_breakout
     {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  unsigned int  sign :24;
	  signed char   corr :8;
#else
	  signed char   corr :8;
	  unsigned int  sign :24;
#endif
     } field;
     
     unsigned int four_byte;
};

struct Sig_scia
{
     unsigned char  stray;
     signed char    corr;
     unsigned short sign;
};

struct Sigc_scia
{
     unsigned char    stray;
     union det_signal det;
};

struct Clus_scia
{
     unsigned short    n_sig;
     unsigned short    n_sigc;
     struct Sig_scia   *sig;
     struct Sigc_scia  *sigc;
};

/* SCIA level 1b PDS data structures */
struct sph1_scia
{
     char   spec_cal[5];
     char   saturate[5];
     char   dark_check[5];
     char   dead_pixel[5];
     char   key_data[6];
     char   m_factor[6];
     char   descriptor[29];
     char   init_version[38];
     char   start_time[UTC_STRING_LENGTH];
     char   stop_time[UTC_STRING_LENGTH];
     short  stripline;
     short  slice_pos;
     unsigned short  no_slice;
     unsigned short  no_nadir;
     unsigned short  no_limb;
     unsigned short  no_occult;
     unsigned short  no_monitor;
     unsigned short  no_noproc;
     unsigned short  comp_dark;
     unsigned short  incomp_dark;
     double start_lat;
     double start_lon;
     double stop_lat;
     double stop_lon;
};

struct sqads1_scia
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     unsigned char flag_glint;
     unsigned char flag_rainbow;
     unsigned char flag_saa_region;
     unsigned short missing_readouts;
     unsigned short hotpixel[ALL_CHANNELS];
     float mean_wv_diff[SCIENCE_CHANNELS];
     float sdev_wv_diff[SCIENCE_CHANNELS];
     float mean_diff_leak[ALL_CHANNELS];
};

struct sip_scia
{
     char do_use_limb_dark[2];
     char do_pixelwise[SCIENCE_CHANNELS+1];
     char do_ib_oc_etn[PMD_NUMBER+1];
     char do_ib_sd_etn[PMD_NUMBER+1];
     char do_fraunhofer[5 * SCIENCE_CHANNELS+1];
     char do_etalon[3 * SCIENCE_CHANNELS+1];
     char do_var_lc_cha[4 * IR_CHANNELS+1];
     char do_stray_lc_cha[4 * SCIENCE_CHANNELS+1];
     char do_var_lc_pmd[4 * IR_PMD_NUMBER+1];
     char do_stray_lc_pmd[4 * PMD_NUMBER+1];
     char do_pol_point[NUM_FRAC_POLV+1];
     unsigned char n_lc_min;
     unsigned char ds_n_phases;
     unsigned char sp_n_phases;
     unsigned char ds_poly_order;
     unsigned char lc_harm_order;
     unsigned char level_2_smr[SCIENCE_CHANNELS];
     unsigned short sat_level[SCIENCE_CHANNELS];
     unsigned short pmd_sat_limit;
     short startpix_6;
     short startpix_8;
     float alpha0_asm;
     float alpha0_esm;
     float ppg_error;
     float stray_error;
     float h_toa;
     float lambda_end_gdf;
     float ds_phase_boundaries[MaxBoundariesSIP];
     float sp_phase_boundaries[MaxBoundariesSIP];
     float lc_stray_indx[2];
     float electrons_bu[SCIENCE_CHANNELS];
};

struct clcp_scia
{
     float fpn[SCIENCE_PIXELS];
     float fpn_error[SCIENCE_PIXELS];
     float lc[SCIENCE_PIXELS];
     float lc_error[SCIENCE_PIXELS];
     float pmd_dark[2 * PMD_NUMBER];
     float pmd_dark_error[2 * PMD_NUMBER];
     float mean_noise[SCIENCE_PIXELS];
};

struct vlcp_scia
{
     float orbit_phase;
     float obm_pmd[IR_CHANNELS + PMD_NUMBER];
     float var_lc[IR_CHANNELS * CHANNEL_SIZE];
     float var_lc_error[IR_CHANNELS * CHANNEL_SIZE];
     float solar_stray[SCIENCE_PIXELS];
     float solar_stray_error[SCIENCE_PIXELS];
     float pmd_stray[PMD_NUMBER];
     float pmd_stray_error[PMD_NUMBER];
     float pmd_dark[IR_PMD_NUMBER];
     float pmd_dark_error[IR_PMD_NUMBER];
};

struct ppg_scia
{
     float ppg_fact[SCIENCE_PIXELS];
     float etalon_fact[SCIENCE_PIXELS];
     float etalon_resid[SCIENCE_PIXELS];
     float wls_deg_fact[SCIENCE_PIXELS];
     unsigned char bad_pixel[SCIENCE_PIXELS];
};

struct base_scia
{
     float wvlen_det_pix[SCIENCE_PIXELS];
};

struct scp_scia
{
     float orbit_phase;
     double coeffs[NUM_SPEC_COEFFS * SCIENCE_CHANNELS];
     unsigned short num_lines[SCIENCE_CHANNELS];
     float wv_error_calib[SCIENCE_CHANNELS];
};

struct srs_scia
{
     char sun_spec_id[3];
     float avg_asm;
     float avg_esm;
     float avg_elev_sun;
     float dopp_shift;
     float wvlen_sun[SCIENCE_PIXELS];
     float mean_sun[SCIENCE_PIXELS];
     float precision_sun[SCIENCE_PIXELS];
     float accuracy_sun[SCIENCE_PIXELS];
     float etalon[SCIENCE_PIXELS];
     float pmd_mean[PMD_NUMBER];
     float pmd_out_nd_out[PMD_NUMBER];
     float pmd_out_nd_in[PMD_NUMBER];
};

struct pspn_scia
{
     float  ang_esm;
     double mu2[SCIENCE_PIXELS];
     double mu3[SCIENCE_PIXELS];
};

struct psplo_scia
{
     float  ang_esm;
     float  ang_asm;
     double mu2[SCIENCE_PIXELS];
     double mu3[SCIENCE_PIXELS];
};

struct rspn_scia
{
     float  ang_esm;
     double sensitivity[SCIENCE_PIXELS];
};

struct rsplo_scia
{
     float  ang_esm;
     float  ang_asm;
     double sensitivity[SCIENCE_PIXELS];
};

struct ekd_scia
{
     float mu2_nadir[SCIENCE_PIXELS];
     float mu3_nadir[SCIENCE_PIXELS];
     float mu2_limb[SCIENCE_PIXELS];
     float mu3_limb[SCIENCE_PIXELS];
     float radiance_vis[SCIENCE_PIXELS];
     float radiance_nadir[SCIENCE_PIXELS];
     float radiance_limb[SCIENCE_PIXELS];
     float radiance_sun[SCIENCE_PIXELS];
     float bsdf[SCIENCE_PIXELS];
};

struct sfp_scia
{
     unsigned short pix_pos_slit_fun;
     unsigned char  type_slit_fun;
     float          fwhm_slit_fun;
     float          f_voi_fwhm_loren;
};

struct asfp_scia
{
     unsigned short pix_pos_slit_fun;
     unsigned char  type_slit_fun;
     float          fwhm_slit_fun;
     float          f_voi_fwhm_gauss;
};

struct state1_scia
{
     struct mjd_envi mjd;
     struct Clcon_scia Clcon[MAX_CLUSTER];
     unsigned char  flag_mds;
     unsigned char  flag_reason;
     unsigned char  type_mds;
/*      unsigned char  dummy; */
     unsigned short category;
     unsigned short state_id;
     unsigned short dur_scan;
     unsigned short longest_intg_time;
     unsigned short num_clus;
     unsigned short num_aux;
     unsigned short num_pmd;
     unsigned short num_intg;
     unsigned short intg_times[MAX_CLUSTER];
     unsigned short num_polar[MAX_CLUSTER];
     unsigned short total_polar;
     unsigned short num_dsr;
     unsigned int   indx;
     unsigned int   length_dsr;
     unsigned int   offset;
     unsigned int   offs_pmd;
     unsigned int   offs_polV;
     float          orbit_phase;
};

struct pmd_scia
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     struct mds0_pmd mds0;
};

struct aux_scia
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     struct mds0_aux mds0;
};

struct lcpn_scia
{
     struct mjd_envi mjd;
     struct mjd_envi mjd_last;
     unsigned char flag_mds;
     float orbit_phase;
     float obm_pmd[IR_CHANNELS + PMD_NUMBER];
     float fpn[SCIENCE_PIXELS];
     float fpn_error[SCIENCE_PIXELS];
     float lc[SCIENCE_PIXELS];
     float lc_error[SCIENCE_PIXELS];
     float mean_noise[SCIENCE_PIXELS];
     float pmd_off[2 * PMD_NUMBER];
     float pmd_off_error[2 * PMD_NUMBER];
};

struct dark_scia
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     float dark_spec[SCIENCE_PIXELS];
     float sdev_dark_spec[SCIENCE_PIXELS];
     float pmd_off[2 * PMD_NUMBER];
     float pmd_off_error[2 * PMD_NUMBER];
     float sol_stray[SCIENCE_PIXELS]; 
     float sol_stray_error[SCIENCE_PIXELS]; 
     float pmd_stray[PMD_NUMBER];
     float pmd_stray_error[PMD_NUMBER];
};

struct ppgn_scia
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     float gain_fact[SCIENCE_PIXELS];
     float etalon_fact[SCIENCE_PIXELS];
     float etalon_resid[SCIENCE_PIXELS]; 
     float avg_wls_spec[SCIENCE_PIXELS];
     float sd_wls_spec[SCIENCE_PIXELS];
     unsigned char bad_pixel[SCIENCE_PIXELS];
};

struct scpn_scia
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     float orbit_phase;
     unsigned char srs_param[SCIENCE_CHANNELS];
     unsigned short num_lines[SCIENCE_CHANNELS];
     float wv_error_calib[SCIENCE_CHANNELS];
     float sol_spec[SCIENCE_PIXELS];
     float line_pos[3 * SCIENCE_CHANNELS];
     double coeffs[NUM_SPEC_COEFFS * SCIENCE_CHANNELS];
};

struct srsn_scia
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     unsigned char flag_neu;
     char sun_spec_id[3];
     float avg_asm;
     float avg_esm;
     float avg_elev_sun;
     float dopp_shift;
     float wvlen_sun[SCIENCE_PIXELS];
     float mean_sun[SCIENCE_PIXELS];
     float precision_sun[SCIENCE_PIXELS];
     float accuracy_sun[SCIENCE_PIXELS];
     float etalon[SCIENCE_PIXELS];
     float pmd_mean[PMD_NUMBER];
     float pmd_out[PMD_NUMBER];
};

struct lv0_hdr
{
     unsigned short    bcps;
     unsigned short    num_chan;
     int               orbit_vector[8];
     struct packet_hdr packet_hdr;
     struct data_hdr   data_hdr;
     struct pmtc_hdr   pmtc_hdr;
};

struct mds1_scia
{
     struct mjd_envi  mjd;
     signed char      quality_flag;
     unsigned char    type_mds;
     unsigned char    state_id;
     unsigned char    state_index;
     unsigned short   n_clus;
     unsigned short   n_aux;
     unsigned short   n_pmd;
     unsigned short   n_pol;
     unsigned int     dsr_length;
     unsigned char    scale_factor[SCIENCE_CHANNELS];
     unsigned char    *sat_flags;
     unsigned char    *red_grass;
     struct lv0_hdr   *lv0;
     struct geoC_scia *geoC;
     struct geoL_scia *geoL;
     struct geoN_scia *geoN;
     float            *int_pmd;
     struct polV_scia *polV;
     struct Clus_scia clus[MAX_CLUSTER];
};

/*
 * level 1c specific structures
 */
struct cal_options
{
     signed char geo_filter;
     signed char time_filter;
     signed char category_filter;
     signed char nadir_mds;
     signed char limb_mds;
     signed char occ_mds;
     signed char moni_mds;
     signed char pmd_mds;
     signed char frac_pol_mds;
     signed char slit_function;
     signed char sun_mean_ref;
     signed char leakage_current;
     signed char spectral_cal;
     signed char pol_sens;
     signed char rad_sens;
     signed char ppg_etalon;
     signed char mem_effect_cal;
     signed char leakage_cal;
     signed char straylight_cal;
     signed char ppg_cal;
     signed char etalon_cal;
     signed char wave_cal;
     signed char polarisation_cal;
     signed char radiance_cal;
     unsigned short num_nadir;
     unsigned short num_limb;
     unsigned short num_occ;
     unsigned short num_moni;
     int start_lat;
     int start_lon;
     int end_lat;
     int end_lon;
     struct mjd_envi start_time;
     struct mjd_envi stop_time;
     unsigned short category[5];
     signed char nadir_cluster[MAX_CLUSTER];
     signed char limb_cluster[MAX_CLUSTER];
     signed char occ_cluster[MAX_CLUSTER];
     signed char moni_cluster[MAX_CLUSTER];
     char l1b_prod_name[ENVI_FILENAME_SIZE];
};

#ifdef _SCIA_PATCH_1
#define NUM_KEY_PSPN  17
#define NUM_KEY_RSPN  17
#define NUM_KEY_PSPL  115
#define NUM_KEY_RSPL  115
#define NUM_KEY_SRS   12

struct keydata_rec {
     struct pspn_scia pspn[NUM_KEY_PSPN];
     struct rspn_scia rspn[NUM_KEY_RSPN];
     struct psplo_scia pspl[NUM_KEY_PSPL];
     struct rsplo_scia rspl[NUM_KEY_RSPL];
     struct srs_scia srs[NUM_KEY_SRS];
};
#endif /* _SCIA_PATCH_1 */

/* 
 * KB: Mfactor types (calibration, limb, nadir light path)  
 */
enum mf_type { M_CAL, M_DL, M_DN, MTYPE_MAX };

/* added kb 2006/08/11 */
/* Structure holding keydata for Radiance Sensitivity calculations */
/*  without angle dependencie */
struct rspd_key_fix_scia
{    
    float wl[SCIENCE_PIXELS];
    float OBM_s_p[SCIENCE_PIXELS];
    float ABS_RAD[SCIENCE_PIXELS];
    float NDF[SCIENCE_PIXELS];
    float NDF_s_p[SCIENCE_PIXELS];
    float PPG0[SCIENCE_PIXELS];
};

/* number of wavelength where the BRDF is characterized */
/*  Fixed Maximum number: dirty but easy */
/* BRDF : 32 */
/* ELEV : 18 */
/* ELAZ : 28 */

#define BRDF_WAVELENGTH SCIENCE_PIXELS

/* structure for ELEV_s and ELEV_p  */
struct rspd_ELEV_scia
{
    int n_wl;
    float elevat_angle;
    float wl[BRDF_WAVELENGTH];
    float sensitivity[BRDF_WAVELENGTH];
};

/* structure for EL_AZ_s and EL_AZ_p  */
struct rspd_EL_AZ_scia
{
    int n_wl;
    float elevat_angle;
    float azimuth_angle;
    float wl[BRDF_WAVELENGTH];
    float sensitivity[BRDF_WAVELENGTH];
};

/* structure for BRDF_s and BRDF_p */
struct rspd_BRDF_scia
{
    int n_wl;
    float elevat_angle;
    float asm_angle;
    float wl[BRDF_WAVELENGTH];
    float sensitivity[BRDF_WAVELENGTH];
};

/* structure holding complete set of above entities */
struct rspd_key
{
    struct rspd_key_fix_scia key_fix;
    struct rspd_ELEV_scia  *elev_p;
    struct rspd_ELEV_scia  *elev_s;
    struct rspd_EL_AZ_scia *el_az_p;
    struct rspd_EL_AZ_scia *el_az_s;
    struct rspd_BRDF_scia  *brdf_p;
    struct rspd_BRDF_scia  *brdf_s;
    size_t n_elev;
    size_t n_el_az; 
    size_t n_brdf;
};
/* END added KB 2006/08/11 */

/*
 * prototype declarations of Sciamachy level 1 functions
 */
extern void SCIA_LV1_ADD_DSD( const struct dsd_envi * );
extern void SCIA_LV1_EXPORT_NUM_STATE( int, unsigned short );
extern unsigned int SCIA_LV1_UPDATE_SQADS( struct sqads1_scia *);
extern unsigned int SCIA_LV1_UPDATE_LADS( struct lads_scia * );
extern unsigned int SCIA_LV1_UPDATE_STATE( struct state1_scia *);

/* #if defined _STDIO_INCLUDED || defined __STDIO_H__ */
#if defined _STDIO_H || defined S_SPLINT_S
extern unsigned int SCIA_LV1_RD_ASFP( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
		                      /*@out@*/ struct asfp_scia **asfp )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *asfp@*/;
extern void SCIA_LV1_WR_ASFP( FILE *fp, unsigned int, 
			      const struct asfp_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_AUX( FILE *fp, unsigned int, 
				     const struct dsd_envi *,
		                     /*@out@*/ struct aux_scia **aux )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *aux@*/;
extern void SCIA_LV1_WR_AUX( FILE *fp, unsigned int, 
			     const struct aux_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_BASE( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
				      /*@out@*/ struct base_scia *base )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *base@*/;
extern void SCIA_LV1_WR_BASE( FILE *fp, const struct base_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_CLCP( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
		                      /*@out@*/ struct clcp_scia *clcp )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *clcp@*/;
extern void SCIA_LV1_WR_CLCP( FILE *fp, unsigned int, 
			      const struct clcp_scia )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern void SCIA_LV1_WR_DSD_INIT( const struct param_record, FILE *fp_out, 
				  unsigned int, const struct dsd_envi * )
       /*@globals  errno, nadc_stat, nadc_err_stack, internalState;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp_out, internalState@*/;
extern void SCIA_LV1_SET_NUM_ATTACH( const struct param_record , FILE *fp_in, 
				     unsigned int, 
				     const struct dsd_envi *dsd_in )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc,
                   internalState;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc,
                   fp_in, internalState@*/;
extern void SCIA_LV1_WR_DSD_UPDATE( FILE *fp_in, FILE *fp_out )
       /*@globals  errno, stderr, nadc_stat, nadc_err_stack, internalState;@*/
       /*@modifies errno, stderr, nadc_stat, nadc_err_stack, fp_in, fp_out,
                   internalState@*/;
extern unsigned int SCIA_LV1_RD_DARK( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
		                      /*@out@*/ struct dark_scia **dark )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *dark@*/;
extern void SCIA_LV1_WR_DARK( FILE *fp, unsigned int, 
			      const struct dark_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_EKD( FILE *fp, unsigned int, 
				     const struct dsd_envi *,
		                     /*@out@*/ struct ekd_scia *ekd )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *ekd@*/;
extern void SCIA_LV1_WR_EKD( FILE *fp, unsigned int, 
			     const struct ekd_scia )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern void SCIA_LV1_WR_LADS( FILE *fp, unsigned int, 
			      const struct lads_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_LCPN( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
		                      /*@out@*/ struct lcpn_scia **lcpn )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *lcpn@*/;
extern void SCIA_LV1_WR_LCPN( FILE *fp, unsigned int, 
			      const struct lcpn_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_PMD( FILE *fp, unsigned int, 
				     const struct dsd_envi *,
		                     /*@out@*/ struct pmd_scia **pmd )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *pmd@*/;
extern void SCIA_LV1_WR_PMD( FILE *fp, unsigned int, 
			     const struct pmd_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_PPG( FILE *fp, unsigned int, 
				     const struct dsd_envi *,
		                     /*@out@*/ struct ppg_scia *ppg )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *ppg@*/;
extern void SCIA_LV1_WR_PPG( FILE *fp, unsigned int, 
			     const struct ppg_scia )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_PPGN( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
		                      /*@out@*/ struct ppgn_scia **ppgn )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *ppgn@*/;
extern void SCIA_LV1_WR_PPGN( FILE *fp, unsigned int, 
			      const struct ppgn_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_PSPL( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
				      /*@out@*/ struct psplo_scia **pspl )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *pspl@*/;
extern void SCIA_LV1_WR_PSPL( FILE *fp, unsigned int, 
			      const struct psplo_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_PSPN( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
		                      /*@out@*/ struct pspn_scia **pspn )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *pspn@*/;
extern void SCIA_LV1_WR_PSPN( FILE *fp, unsigned int, 
			      const struct pspn_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_PSPO( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
				      /*@out@*/ struct psplo_scia **pspo )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *pspo@*/;
extern void SCIA_LV1_WR_PSPO( FILE *fp, unsigned int, 
			      const struct psplo_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_RSPL( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
				      /*@out@*/ struct rsplo_scia **rspl )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *rspl@*/;
extern void SCIA_LV1_WR_RSPL( FILE *fp, unsigned int, 
			      const struct rsplo_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_RSPN( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
		                      /*@out@*/ struct rspn_scia **rspn )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *rspn@*/;
extern void SCIA_LV1_WR_RSPN( FILE *fp, unsigned int, 
			      const struct rspn_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_RSPO( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
				      /*@out@*/ struct rsplo_scia **rspo )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *rspo@*/;
extern void SCIA_LV1_WR_RSPO( FILE *fp, unsigned int, 
			      const struct rsplo_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_SCP( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
		                     /*@out@*/ struct scp_scia **scp )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *scp@*/;
extern void SCIA_LV1_WR_SCP( FILE *fp, unsigned int, 
			     const struct scp_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_SCPN( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
		                      /*@out@*/ struct scpn_scia **scpn )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *scpn@*/;
extern void SCIA_LV1_WR_SCPN( FILE *fp, unsigned int, 
			      const struct scpn_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_SFP( FILE *fp, unsigned int, 
				     const struct dsd_envi *,
		                     /*@out@*/ struct sfp_scia **sfp )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *sfp@*/;
extern void SCIA_LV1_WR_SFP( FILE *fp, unsigned int, 
			     const struct sfp_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_SIP( FILE *fp, unsigned int, 
				     const struct dsd_envi *,
				     /*@out@*/ struct sip_scia *sip )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *sip@*/;
extern void SCIA_LV1_WR_SIP( FILE *fp, unsigned int, 
			     const struct sip_scia )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern void SCIA_LV1_RD_SPH( FILE *fp, const struct mph_envi,
			     /*@out@*/ struct sph1_scia *sph )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *sph@*/;
extern void SCIA_LV1_WR_SPH( FILE *fp, const struct mph_envi,
			     const struct sph1_scia sph )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp@*/;
extern unsigned int SCIA_LV1_RD_SQADS( FILE *fp, unsigned int, 
				       const struct dsd_envi *,
		                       /*@out@*/ struct sqads1_scia **sqads )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *sqads@*/;
extern void SCIA_LV1_WR_SQADS( FILE *fp, unsigned int, 
			       const struct sqads1_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_SRS( FILE *fp, unsigned int, 
				     const struct dsd_envi *,
		                     /*@out@*/ struct srs_scia **srs )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *srs@*/;
extern void SCIA_LV1_WR_SRS( FILE *fp, unsigned int, 
			     const struct srs_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_SRSN( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
		                      /*@out@*/ struct srsn_scia **srsn )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *srsn@*/;
extern void SCIA_LV1_WR_SRSN( FILE *fp, unsigned int, 
			      const struct srsn_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_STATE( FILE *fp, unsigned int, 
				       const struct dsd_envi *,
		                       /*@out@*/ struct state1_scia **state )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *state@*/;
extern void SCIA_LV1_WR_STATE( FILE *fp, unsigned int, 
			       const struct state1_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;
extern unsigned int SCIA_LV1_RD_VLCP( FILE *fp, unsigned int, 
				      const struct dsd_envi *,
		                      /*@out@*/ struct vlcp_scia **vlcp )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *vlcp@*/;
extern void SCIA_LV1_WR_VLCP( FILE *fp, unsigned int, 
			      const struct vlcp_scia * )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;

extern unsigned int SCIA_LV1C_RD_CALOPT( FILE *fp, unsigned int, 
					 const struct dsd_envi *,
					 /*@out@*/ struct cal_options *calopt )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *calopt@*/;
extern void SCIA_LV1C_UPDATE_CALOPT( int, const struct param_record ,
				     struct cal_options * );
extern void SCIA_LV1C_WR_CALOPT( FILE *fp, unsigned int, 
				 const struct cal_options )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;

extern unsigned int SCIA_LV1_RD_MDS( FILE *fp, unsigned long long,
				     struct state1_scia *state,
		   /*@null@*/ /*@out@*/ /*@partial@*/ struct mds1_scia **mds )
     /*@globals  errno, nadc_stat, nadc_err_stack, internalState;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp, *mds, 
                  state->num_clus, state->Clcon[], internalState@*/;
extern void SCIA_LV1_WR_MDS( FILE *fd, unsigned int, const struct mds1_scia * )
     /*@globals  errno, nadc_stat, nadc_err_stack, internalState;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fd, internalState@*/;
extern unsigned int SCIA_LV1C_RD_MDS( FILE *fp, unsigned long long,
				      struct state1_scia *state,
		   /*@null@*/ /*@out@*/ /*@partial@*/ struct mds1c_scia **mds )
     /*@globals  errno, nadc_stat, nadc_err_stack, internalState;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp, state, *mds, 
	          state->num_clus, state->Clcon[], internalState@*/;
extern void SCIA_LV1C_WR_MDS( FILE *fd, unsigned int, 
			      const struct mds1c_scia *mds )
     /*@globals  errno, nadc_stat, nadc_err_stack, internalState;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fd, internalState@*/;
extern unsigned int SCIA_LV1C_RD_MDS_PMD( FILE *fp, const struct state1_scia *,
		  /*@out@*/ /*@partial@*/ struct mds1c_pmd **pmd )
     /*@globals  errno, nadc_stat, nadc_err_stack, internalState;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp, *pmd,
                   internalState@*/;
extern void SCIA_LV1C_WR_MDS_PMD( FILE *fd, const struct mds1c_pmd * )
     /*@globals  errno, nadc_stat, nadc_err_stack, internalState;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fd, internalState@*/;
extern unsigned int SCIA_LV1C_RD_MDS_POLV( FILE *fp, 
					   const struct state1_scia *, 
	           /*@out@*/ /*@partial@*/ struct mds1c_polV **polV )
     /*@globals  errno, nadc_stat, nadc_err_stack, internalState;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp, *polV,
                   internalState@*/;
extern void SCIA_LV1C_WR_MDS_POLV( FILE *fd, const struct mds1c_polV * )
     /*@globals  errno, nadc_stat, nadc_err_stack, internalState;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fd, internalState@*/;

extern unsigned int SCIA_LV1_SELECT_MDS( int, struct param_record,
					 FILE *fp, unsigned int,
					 const struct dsd_envi *,
	                  /*@null@*/ /*@out@*/ struct state1_scia **mds_state )
     /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc, 
	         fp, *mds_state@*/;

extern void SCIA_get_AtbdDark( FILE *fp, unsigned int, float,
			       /*@out@*/ float *analogOffs, 
			       /*@out@*/ float *darkCurrent, 
			       /*@out@*/ /*@null@*/ float *analogOffsError, 
			       /*@out@*/ /*@null@*/ float *darkCurrentError,
			       /*@out@*/ /*@null@*/ float *meanNoise )
     /*@globals  errno, nadc_stat, nadc_err_stack@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp@*/;

#endif   /* ---- defined _STDIO_INCLUDED || defined _STDIO_H ----- */

extern void GET_SCIA_LV1C_GEON( unsigned int, const struct geoN_scia *,
				unsigned int, 
			    /*@unique@*/ /*@out@*/ struct geoN_scia *geoN_1c )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, geoN_1c@*/;
extern void GET_SCIA_LV1C_GEOL( unsigned int, const struct geoL_scia *,
				unsigned int, 
			    /*@unique@*/ /*@out@*/ struct geoL_scia *geoL_1c )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, geoL_1c@*/;
extern void GET_SCIA_LV1C_GEOC( unsigned int, const struct geoC_scia *,
				unsigned int, 
			    /*@unique@*/ /*@out@*/ struct geoC_scia *geoC_1c )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, geoC_1c@*/;

extern void SCIA_LV1_CORR_LOS( const struct state1_scia *,
			       struct mds1_scia *mds_1b )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, mds_1b->geoN@*/;

extern unsigned int GET_SCIA_LV1C_MDS( const struct state1_scia *,
				       const struct mds1_scia *, 
				       /*@out@*/ struct mds1c_scia *mds )
     /*@globals errno, nadc_stat, nadc_err_stack@*/
     /*@modifies errno, nadc_stat, nadc_err_stack@, mds*/;
extern unsigned int GET_SCIA_LV1C_PMD( const struct state1_scia *,
				       const struct mds1_scia *, 
				       /*@out@*/ struct mds1c_pmd *mds_pmd )
     /*@globals errno, nadc_stat, nadc_err_stack@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, mds_pmd@*/;
extern unsigned int GET_SCIA_LV1C_POLV( const struct state1_scia *,
					const struct mds1_scia *, 
					/*@out@*/ struct mds1c_polV *mds_polV )
    /*@globals errno, nadc_stat, nadc_err_stack@*/
    /*@modifies errno, nadc_stat, nadc_err_stack, mds_polV@*/;

extern unsigned short GET_SCIA_MDS1_DATA( bool, unsigned char, 
					  const struct state1_scia *,
					  /*@null@*/ const struct mds1_scia *,
					  /*@null@*/ const struct mds1c_pmd *,
					  const struct mds1c_scia *, 
					  /*@out@*/ float **sign_out )
       /*@globals  nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies nadc_stat, nadc_err_stack, *sign_out@*/;

extern bool IS_SCIA_LV1C( unsigned int, const struct dsd_envi * );

extern unsigned long long SCIA_LV1_CHAN2CLUS( const struct param_record, 
					      const struct state1_scia * );

extern void SCIA_LV1_FREE_MDS( int, unsigned int, 
			       /*@only@*/ struct mds1_scia * );
extern void SCIA_LV1_WR_ASCII_SPH( struct param_record, 
				   const struct sph1_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_SQADS( struct param_record, unsigned int, 
				     const struct sqads1_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_SIP( struct param_record, 
				   const struct sip_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_CLCP( struct param_record, 
				    const struct clcp_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_VLCP( struct param_record, unsigned int, 
				    const struct vlcp_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_PPG( struct param_record, 
				   const struct ppg_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_BASE( struct param_record, 
				    const struct base_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_SCP( struct param_record, unsigned int, 
				   const struct scp_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_SRS( struct param_record, unsigned int, 
				   const struct srs_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_PSPN( struct param_record, unsigned int, 
				    const struct pspn_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_PSPL( struct param_record, unsigned int, 
				    const struct psplo_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_PSPO( struct param_record, unsigned int, 
				    const struct psplo_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_RSPN( struct param_record, unsigned int, 
				    const struct rspn_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_RSPL( struct param_record, unsigned int, 
				    const struct rsplo_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_RSPO( struct param_record, unsigned int, 
				    const struct rsplo_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_EKD( struct param_record,
				   const struct ekd_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_SFP( struct param_record, unsigned int, 
				   const struct sfp_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_ASFP( struct param_record, unsigned int, 
				    const struct asfp_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_STATE( struct param_record, unsigned int, 
				     const struct state1_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_PMD( struct param_record, unsigned int, 
				   const struct pmd_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_AUX( struct param_record, unsigned int, 
				   const struct aux_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_LCPN( struct param_record, unsigned int, 
				    const struct lcpn_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_DARK( struct param_record, unsigned int, 
				    const struct dark_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_PPGN( struct param_record, unsigned int, 
				    const struct ppgn_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_SCPN( struct param_record, unsigned int, 
				    const struct scpn_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_SRSN( struct param_record, unsigned int, 
				    const struct srsn_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_WR_ASCII_MDS( const struct param_record, unsigned int, 
				   const struct mds1_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1C_WR_ASCII_CALOPT( const struct param_record,
				       const struct cal_options * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1C_WR_ASCII_MDS( const struct param_record, unsigned int, 
				    const struct mds1c_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1C_WR_ASCII_MDS_PMD( const struct param_record, 
					const struct mds1c_pmd * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1C_WR_ASCII_MDS_POLV( const struct param_record, 
					 const struct mds1c_polV * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;

extern unsigned short SCIA_RD_H5_PSPN( /*@out@*/ struct pspn_scia **pspn_out )
       /*@globals  nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies nadc_stat, nadc_err_stack, *pspn_out@*/;
extern unsigned short SCIA_RD_H5_PSPL( /*@out@*/ struct psplo_scia **pspl_out )
       /*@globals  nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies nadc_stat, nadc_err_stack, *pspl_out@*/;
extern unsigned short SCIA_RD_H5_PSPO( /*@out@*/ struct psplo_scia **pspo_out )
       /*@globals  nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies nadc_stat, nadc_err_stack, *pspo_out@*/;
extern unsigned short SCIA_RD_H5_RSPN( /*@out@*/ struct rspn_scia **rspn_out )
       /*@globals  nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies nadc_stat, nadc_err_stack, *rspn_out@*/;
extern unsigned short SCIA_RD_H5_RSPL( /*@out@*/ struct rsplo_scia **rspl_out )
       /*@globals  nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies nadc_stat, nadc_err_stack, *rspl_out@*/;
extern unsigned short SCIA_RD_H5_RSPO( /*@out@*/ struct rsplo_scia **rspo_out )
       /*@globals  nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies nadc_stat, nadc_err_stack, *rspo_out@*/;
extern void SCIA_RD_H5_RSPD( /*@out@*/ struct rspd_key *key )
       /*@globals  nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies nadc_stat, nadc_err_stack, *key@*/;
extern void SCIA_RD_MFACTOR( enum mf_type, const char *, unsigned int , 
			     /*@out@*/ float * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1_MFACTOR_SRS( const char *, unsigned int ,
				  unsigned int, struct srs_scia *srs_out )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;

#ifdef _HDF5_H
extern void CRE_SCIA_LV1_H5_STRUCTS( struct param_record );
extern void SCIA_LV1_WR_H5_SPH( struct param_record, 
				const struct sph1_scia * );
extern void SCIA_LV1_WR_H5_SQADS( struct param_record, unsigned int,
				  const struct sqads1_scia * );
extern void SCIA_LV1_WR_H5_SIP( struct param_record, 
				const struct sip_scia * );
extern void SCIA_LV1_WR_H5_CLCP( struct param_record, 
				 const struct clcp_scia * );
extern void SCIA_LV1_WR_H5_VLCP( struct param_record, unsigned int,
				 const struct vlcp_scia * );
extern void SCIA_LV1_WR_H5_PPG( struct param_record, 
				const struct ppg_scia * );
extern void SCIA_LV1_WR_H5_BASE( struct param_record, 
				 const struct base_scia * );
extern void SCIA_LV1_WR_H5_SCP( struct param_record, unsigned int,
				const struct scp_scia * );
extern void SCIA_LV1_WR_H5_SRS( struct param_record, unsigned int,
				const struct srs_scia * );
extern void SCIA_LV1_WR_H5_PSPN( struct param_record, unsigned int,
				 const struct pspn_scia * );
extern void SCIA_LV1_WR_H5_PSPL( struct param_record, unsigned int,
				 const struct psplo_scia * );
extern void SCIA_LV1_WR_H5_PSPO( struct param_record, unsigned int,
				 const struct psplo_scia * );
extern void SCIA_LV1_WR_H5_RSPN( struct param_record, unsigned int,
				 const struct rspn_scia * );
extern void SCIA_LV1_WR_H5_RSPL( struct param_record, unsigned int,
				 const struct rsplo_scia * );
extern void SCIA_LV1_WR_H5_RSPO( struct param_record, unsigned int,
				 const struct rsplo_scia * );
extern void SCIA_LV1_WR_H5_EKD( struct param_record, 
				const struct ekd_scia * );
extern void SCIA_LV1_WR_H5_SFP( struct param_record, unsigned int,
				const struct sfp_scia * );
extern void SCIA_LV1_WR_H5_ASFP( struct param_record, unsigned int,
				 const struct asfp_scia * );
extern void SCIA_LV1_WR_H5_STATE( struct param_record, unsigned int,
				  const struct state1_scia * );
extern void SCIA_LV1_WR_H5_PMD( struct param_record, unsigned int,
				const struct pmd_scia * );
extern void SCIA_LV1_WR_H5_AUX( struct param_record, unsigned int,
				const struct aux_scia * );
extern void SCIA_LV1_WR_H5_LCPN( struct param_record, unsigned int,
				 const struct lcpn_scia * );
extern void SCIA_LV1_WR_H5_DARK( struct param_record, unsigned int,
				 const struct dark_scia * );
extern void SCIA_LV1_WR_H5_PPGN( struct param_record, unsigned int,
				 const struct ppgn_scia * );
extern void SCIA_LV1_WR_H5_SCPN( struct param_record, unsigned int,
				 const struct scpn_scia * );
extern void SCIA_LV1_WR_H5_SRSN( struct param_record, unsigned int,
				 const struct srsn_scia * );
extern unsigned int SCIA_LV1_RD_H5_MDS( const char *, 
					const struct state1_scia *, 
					/*@out@*/ struct mds1_scia **mds )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, *mds@*/;

extern void SCIA_LV1_WR_H5_MDS( const struct param_record, 
				unsigned int, const struct mds1_scia * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;

extern void SCIA_LV1C_WR_H5_MDS( const struct param_record, unsigned int, 
				 const struct mds1c_scia * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1C_WR_H5_MDS_PMD( const struct param_record, 
				     const struct mds1c_pmd * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV1C_WR_H5_MDS_POLV( const struct param_record, 
				      const struct mds1c_polV * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
#endif /* _HDF5_H */

#ifdef LIBPQ_FE_H
extern void SCIA_LV1_WR_SQL_META( PGconn *conn, bool, const char *, 
                                  const struct mph_envi *,
                                  const struct sph1_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void SCIA_LV1_WR_SQL_AUX( PGconn *conn, bool, const struct mph_envi *,
                                 unsigned int, const struct dsd_envi * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void SCIA_LV1_MATCH_STATE( PGconn *conn, bool, const struct mph_envi *,
                                  unsigned short, const struct lads_scia *,
                                  const struct sqads1_scia *,
                                  const struct state1_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void SCIA_LV1_DEL_ENTRY( PGconn *conn, bool, const char * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
#endif /* LIBPQ_FE_H */

#ifdef __cplusplus
  }
#endif
#endif /* __NADC_SCIA_LV1 */
