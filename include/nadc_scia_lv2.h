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

.IDENTifer   nadc_scia
.AUTHOR      R.M. van Hees 
.KEYWORDS    header file
.LANGUAGE    ANSI-C
.PURPOSE     macros and structures for SCIAMACHY level 2 modules
.ENVIRONment none
.VERSION      1.4   31-Mar-2003	modified include for C++ code
              1.3   16-Aug-2002	OL2: updated to ENV-ID-DLR-SCI-2200-4, RvH 
              1.2   29-Apr-2002	added NL2_SQADS_PQF_FLAGS, OL2_SQADS_PQF_FLAGS 
              1.1   06-Mar-2002	v0.4: is SQADS_PQF_FLAGS really fixed?, RvH
              1.0   13-Aug-2001	Creation by R.M. van Hees 
------------------------------------------------------------*/
#ifndef  __NADC_SCIA_LV2                        /* Avoid redefinitions */
#define  __NADC_SCIA_LV2

#ifdef __cplusplus
extern "C" {
#endif

#define NL2_SQADS_PQF_FLAGS    152          /* is this a fixed number ??? */
#define OL2_SQADS_PQF_FLAGS    180          /* is this a fixed number ??? */

#define MAX_BIAS_MICRO_WIN     ((unsigned short) 3)
#define MAX_BIAS_FITTING_WIN   4
#define MAX_DOAS_FITTING_WIN   7
#define MAX_DOAS_SPECIES       ((unsigned short) 22)
#define MAX_BIAS_SPECIES       ((unsigned short) 10)

/*+++++ Structures & Unions +++++*/
struct bias_record
{
     unsigned short wv_min;
     unsigned short wv_max;
     unsigned short nr_micro;
     unsigned short micro_min[MAX_BIAS_MICRO_WIN];
     unsigned short micro_max[MAX_BIAS_MICRO_WIN];
};

struct doas_record
{
     unsigned short wv_min;
     unsigned short wv_max;
};

struct win_record
{
     char mol[5];
     unsigned short wv_min;
     unsigned short wv_max;
};

/* SCIA NRT level 2 PDS data structures */
struct sph2_scia
{
     char  fit_error[5];
     char  descriptor[29];
     char  start_time[UTC_STRING_LENGTH];
     char  stop_time[UTC_STRING_LENGTH];
     char  bias_mol[MAX_BIAS_SPECIES][9];
     char  doas_mol[MAX_DOAS_SPECIES][9];
     short stripline;
     short slice_pos;
     unsigned short no_slice;
     unsigned short no_bias_win;
     unsigned short no_bias_mol;
     unsigned short no_doas_win;
     unsigned short no_doas_mol;
     double start_lat;
     double start_lon;
     double stop_lat;
     double stop_lon;

     struct bias_record bias_win[MAX_BIAS_FITTING_WIN];
     struct doas_record doas_win[MAX_DOAS_FITTING_WIN];
};

struct sqads2_scia
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     unsigned char flag_pqf[NL2_SQADS_PQF_FLAGS];
};

struct state2_scia
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     unsigned short state_id;
     unsigned short duration;
     unsigned short longest_intg_time;
     unsigned short shortest_intg_time;
     unsigned short num_obs_state;
};

struct geo_scia
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     unsigned short intg_time;
     float sun_zen_ang[3];
     float los_zen_ang[3];
     float rel_azi_ang[3];
     float sat_h;
     float earth_rad;
     struct coord_envi corner[NUM_CORNERS];
     struct coord_envi center;
};

struct cld_scia
{
     struct mjd_envi mjd;
     signed char    quality;
     unsigned char  quality_cld;
     unsigned short outputflag;
     unsigned short intg_time;
     unsigned short numpmd;
     unsigned int   dsrlen;
     float  cloudfrac;
     float  toppress;
     float  aai;
     float  albedo;
     float  *pmdcloudfrac;
};

struct doas_scia
{
     struct mjd_envi mjd;
     signed char    quality;
     signed char    dummy;
     unsigned short vcdflag;
     unsigned short escflag;
     unsigned short amfflag;
     unsigned short intg_time;
     unsigned short numfitp;
     unsigned short numiter;
     unsigned int   dsrlen;
     float vcd;
     float errvcd;
     float esc;
     float erresc;
     float rms;
     float chi2;
     float goodness;
     float amfgnd;
     float amfcld;
     float reflgnd;
     float reflcld;
     float refl;
     float *corrpar;
};

struct bias_scia
{
     struct mjd_envi mjd;
     signed char    quality;
     signed char    dummy;
     unsigned short hghtflag;
     unsigned short vcdflag;
     unsigned short intg_time;
     unsigned short numfitp;
     unsigned short numsegm;
     unsigned short numiter;
     unsigned int   dsrlen;
     float hght;
     float errhght;
     float vcd;
     float errvcd;
     float closure;
     float errclosure;
     float rms;
     float chi2;
     float goodness;
     float cutoff;
     float *corrpar;
};

/* SCIA OL level 2 PDS data structures */
struct sph_sci_ol
{
     char  dbserver[6];
     char  errorsum[5];
     char  descriptor[29];
     char  decont[42];
     char  nadir_win_uv0[31];
     char  nadir_win_uv1[31];
     char  nadir_win_uv2[31];
     char  nadir_win_uv3[31];
     char  nadir_win_uv4[31];
     char  nadir_win_uv5[31];
     char  nadir_win_uv6[31];
     char  nadir_win_uv7[31];
     char  nadir_win_uv8[31];
     char  nadir_win_uv9[31];
     char  nadir_win_ir0[31];
     char  nadir_win_ir1[31];
     char  nadir_win_ir2[31];
     char  nadir_win_ir3[31];
     char  nadir_win_ir4[31];
     char  nadir_win_ir5[31];
     char  limb_win_pth[31];
     char  limb_win_uv0[31];
     char  limb_win_uv1[31];
     char  limb_win_uv2[31];
     char  limb_win_uv3[31];
     char  limb_win_uv4[31];
     char  limb_win_uv5[31];
     char  limb_win_uv6[31];
     char  limb_win_uv7[31];
     char  limb_win_ir0[31];
     char  limb_win_ir1[31];
     char  limb_win_ir2[31];
     char  limb_win_ir3[31];
     char  limb_win_ir4[31];
     char  occl_win_pth[31];
     char  occl_win_uv0[31];
     char  occl_win_uv1[31];
     char  occl_win_uv2[31];
     char  occl_win_uv3[31];
     char  occl_win_uv4[31];
     char  occl_win_uv5[31];
     char  occl_win_uv6[31];
     char  occl_win_uv7[31];
     char  occl_win_ir0[31];
     char  occl_win_ir1[31];
     char  occl_win_ir2[31];
     char  occl_win_ir3[31];
     char  occl_win_ir4[31];
     char  start_time[UTC_STRING_LENGTH];
     char  stop_time[UTC_STRING_LENGTH];
     short stripline;
     short slice_pos;
     unsigned short no_slice;
     unsigned short no_nadir_win;
     unsigned short no_limb_win;
     unsigned short no_occl_win;
     double start_lat;
     double start_lon;
     double stop_lat;
     double stop_lon;
};

struct sqads_sci_ol
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     unsigned char flag_pqf[OL2_SQADS_PQF_FLAGS];
};

struct ngeo_scia
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     unsigned char pixel_type;
     unsigned short intg_time;
     float sun_zen_ang[3];
     float los_zen_ang[3];
     float rel_azi_ang[3];
     float sat_h;
     float radius;
     struct coord_envi subsat;
     struct coord_envi corner[NUM_CORNERS];
     struct coord_envi center;
};

struct lgeo_scia
{
     struct mjd_envi mjd;
     unsigned char flag_mds;
     unsigned char dummy;
     unsigned short intg_time;
     float sun_zen_ang[3];
     float los_zen_ang[3];
     float rel_azi_ang[3];
     float sat_h;
     float radius;
     float tan_h[3];
     struct coord_envi subsat;
     struct coord_envi tang[3];
};

struct cld_sci_ol
{
     struct mjd_envi mjd;
     signed char quality;
     signed char dummy;
     unsigned short intg_time;
     unsigned short numpmdpix;
     unsigned short cloudtype;
     unsigned short cloudflag;
     unsigned short aaiflag;
     unsigned short numaeropars;
     unsigned short fullfree[2];
     unsigned int   dsrlen;
     float  surfpress;
     float  cloudfrac;
     float  errcldfrac;
     float  toppress;
     float  errtoppress;
     float  cldoptdepth;
     float  errcldoptdep;
     float  cloudbrdf;
     float  errcldbrdf;
     float  effsurfrefl;
     float  erreffsrefl;
     float  aai;
     float  aaidiag;
     float  *aeropars;
};

struct nfit_scia
{
     struct mjd_envi mjd;

     signed char quality;
     signed char dummy;

     unsigned short intg_time;
     unsigned short numvcd;
     unsigned short vcdflag;
     unsigned short num_fitp;
     unsigned short num_nfitp;
     unsigned short numiter;
     unsigned short fitflag;
     unsigned short amfflag;

     unsigned int dsrlen;

     float esc;
     float erresc;
     float rms;
     float chi2;
     float goodness;
     float amfgrd;
     float erramfgrd;
     float amfcld;
     float erramfcld;
     float temperature;

     float *vcd;
     float *errvcd;
     float *linpars;
     float *errlinpars;
     float *lincorrm;
     float *nlinpars;
     float *errnlinpars;
     float *nlincorrm;
};

struct layer_rec   /* layout must be equal to product definition */
{
     float tangvmr;
     float errtangvmr;
     float vertcol;
     float errvertcol;
};

struct meas_grid   /* layout must be equal to product definition */
{
     struct mjd_envi mjd;
     float tangh;
     float tangp;
     float tangt;
     unsigned char num_win;
     float win_limits[2];
};

struct state_vec   /* layout must be equal to product definition */
{
     float value;
     float error;
     signed char type[4];
};

struct lfit_scia
{
     struct mjd_envi mjd;

     signed char quality;
     signed char criteria;

     unsigned char method;
     unsigned char refpsrc;
     unsigned char num_rlevel;
     unsigned char num_mlevel;
     unsigned char num_species;
     unsigned char num_closure;
     unsigned char num_other;
     unsigned char num_scale;

     unsigned short intg_time;
     unsigned short stvec_size;
     unsigned short cmatrixsize;
     unsigned short numiter;
     unsigned short ressize;
     unsigned short num_adddiag;

     unsigned short summary[2];

     unsigned int  dsrlen;

     float refh;
     float refp;
     float rms;
     float chi2;
     float goodness;

     float *tangh;
     float *tangp;
     float *tangt;
     float *corrmatrix;
     float *residuals;
     float *adddiag;

     struct layer_rec *mainrec;
     struct layer_rec *scaledrec;
     struct meas_grid *mgrid;
     struct state_vec *statevec;
};

struct lcld_scia
{
     struct mjd_envi mjd;
     unsigned int   dsrlen;
     signed char    quality;
     unsigned char  diag_cloud_algo;
     unsigned char  flag_normal_water;
     unsigned char  flag_water_clouds;
     unsigned char  flag_ice_clouds;
     unsigned char  hght_index_max_value_ice;
     unsigned char  flag_polar_strato_clouds;
     unsigned char  hght_index_max_value_strato;
     unsigned char  flag_noctilucent_clouds;
     unsigned char  hght_index_max_value_noctilucent;
     unsigned short intg_time;
     unsigned short num_tangent_hghts;
     unsigned short num_cir;
     unsigned short num_limb_para;
     float max_value_cir;
     float hght_max_value_cir;
     float max_value_cir_ice;
     float hght_max_value_cir_ice;
     float max_value_cir_strato;
     float hght_max_value_cir_strato;
     float hght_max_value_noctilucent;
     float *tangent_hghts;
     float *cir;
     float *limb_para;
};

/*
 * prototype declarations of Sciamachy level 2 functions
 */
#if defined _STDIO_INCLUDED || defined _STDIO_H || defined __STDIO_H__
extern void SCIA_LV2_RD_SPH( FILE *fp, const struct mph_envi,
			     /*@out@*/ struct sph2_scia *sph )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *sph@*/;
extern void SCIA_OL2_RD_SPH( FILE *fp, const struct mph_envi,
			     /*@out@*/ struct sph_sci_ol *sph )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *sph@*/;
extern void SCIA_OL2_WR_SPH( FILE *fp, const struct mph_envi,
			     /*@out@*/ struct sph_sci_ol sph )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp@*/;
extern unsigned int SCIA_LV2_RD_SQADS( FILE *fp, unsigned int,
				       const struct dsd_envi *,
				       /*@out@*/ struct sqads2_scia **sqads )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *sqads@*/;
extern unsigned int SCIA_OL2_RD_SQADS( FILE *fp, unsigned int,
				       const struct dsd_envi *,
				       /*@out@*/ struct sqads_sci_ol **sqads )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *sqads@*/;
extern unsigned int SCIA_LV2_RD_STATE( FILE *fp, unsigned int,
				       const struct dsd_envi *,
				       /*@out@*/ struct state2_scia **state )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *state@*/;
extern unsigned int SCIA_LV2_RD_GEO( FILE *fp, unsigned int,
				     const struct dsd_envi *,
				     /*@out@*/ struct geo_scia **geo )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *geo@*/;
extern unsigned int SCIA_OL2_RD_NGEO( FILE *fp, unsigned int,
				      const struct dsd_envi *,
				      /*@out@*/ struct ngeo_scia **geo )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *geo@*/;
extern unsigned int SCIA_OL2_RD_LGEO( FILE *fp, unsigned int,
				      const struct dsd_envi *,
				      /*@out@*/ struct lgeo_scia **geo )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *geo@*/;
extern unsigned int SCIA_LV2_RD_CLD( FILE *fp, unsigned int,
				     const struct dsd_envi *,
				     /*@out@*/ struct cld_scia **cld )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *cld@*/;
extern unsigned int SCIA_OL2_RD_CLD( FILE *fp, unsigned int,
				     const struct dsd_envi *,
				     /*@out@*/ struct cld_sci_ol **cld )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *cld@*/;
extern unsigned int SCIA_LV2_RD_DOAS( FILE *fp, const char *, unsigned int,
				      const struct dsd_envi *,
				      /*@out@*/ struct doas_scia **doas )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *doas@*/;
extern unsigned int SCIA_LV2_RD_BIAS( FILE *fp, const char *, unsigned int,
				      const struct dsd_envi *,
				      /*@out@*/ struct bias_scia **bias )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *bias@*/;
extern unsigned int SCIA_OL2_RD_NFIT( FILE *fp, const char *, unsigned int,
				      const struct dsd_envi *,
				      /*@out@*/ struct nfit_scia **nfit )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *nfit@*/;
extern unsigned int SCIA_OL2_RD_LFIT( FILE *fp, const char *, unsigned int,
				      const struct dsd_envi *,
				      /*@out@*/ struct lfit_scia **lfit )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *lfit@*/;
extern unsigned int SCIA_OL2_RD_LCLD( FILE *fp, unsigned int,
				      const struct dsd_envi *,
				      /*@out@*/ struct lcld_scia **lcld )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *lcld@*/;
#endif   /* ---- defined _STDIO_INCLUDED || defined _STDIO_H ----- */

extern void SCIA_LV2_WR_ASCII_SPH( struct param_record, 
				   const struct sph2_scia  * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV2_WR_ASCII_SQADS( struct param_record, unsigned int,
				     const struct sqads2_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV2_WR_ASCII_STATE( struct param_record, unsigned int,
				     const struct state2_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV2_WR_ASCII_GEO( struct param_record, unsigned int,
				   const struct geo_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV2_WR_ASCII_CLD( struct param_record, unsigned int,
				   const struct cld_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV2_WR_ASCII_DOAS( const char mds_name[],
				    struct param_record, unsigned int,
				    const struct doas_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_LV2_WR_ASCII_BIAS( const char mds_name[],
				    struct param_record, unsigned int,
				    const struct bias_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;

extern void SCIA_OL2_WR_ASCII_SPH( struct param_record, 
				   const struct sph_sci_ol  * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_OL2_WR_ASCII_SQADS( struct param_record, unsigned int,
				     const struct sqads_sci_ol * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_OL2_WR_ASCII_NGEO( struct param_record, unsigned int,
				   const struct ngeo_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_OL2_WR_ASCII_LGEO( struct param_record, unsigned int,
				   const struct lgeo_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_OL2_WR_ASCII_CLD( struct param_record, unsigned int,
				   const struct cld_sci_ol * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_OL2_WR_ASCII_NFIT( const char mds_name[],
				    struct param_record, unsigned int,
				    const struct nfit_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_OL2_WR_ASCII_LFIT( const char mds_name[],
				    struct param_record, unsigned int,
				    const struct lfit_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void SCIA_OL2_WR_ASCII_LCLD( struct param_record, unsigned int,
				    const struct lcld_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;

#ifdef _HDF5_H
extern void CRE_SCIA_LV2_H5_STRUCTS( struct param_record );
extern void CRE_SCIA_OL2_H5_STRUCTS( struct param_record );
extern void SCIA_LV2_WR_H5_SPH( struct param_record, 
				const struct sph2_scia * );
extern void SCIA_OL2_WR_H5_SPH( struct param_record, 
				const struct sph_sci_ol * );
extern void SCIA_LV2_WR_H5_SQADS( struct param_record, unsigned int,
				  const struct sqads2_scia * );
extern void SCIA_OL2_WR_H5_SQADS( struct param_record, unsigned int,
				  const struct sqads_sci_ol * );
extern void SCIA_LV2_WR_H5_STATE( struct param_record, unsigned int,
				  const struct state2_scia * );
extern void SCIA_LV2_WR_H5_GEO( struct param_record, unsigned int,
				const struct geo_scia * );
extern void SCIA_OL2_WR_H5_NGEO( struct param_record, unsigned int,
				 const struct ngeo_scia * );
extern void SCIA_OL2_WR_H5_LGEO( struct param_record, unsigned int,
				 const struct lgeo_scia * );
extern void SCIA_LV2_WR_H5_CLD( struct param_record, unsigned int,
				const struct cld_scia * );
extern void SCIA_OL2_WR_H5_CLD( struct param_record, unsigned int,
				const struct cld_sci_ol * );
extern void SCIA_LV2_WR_H5_DOAS( const char *, struct param_record, 
				 unsigned int, const struct doas_scia * );
extern void SCIA_LV2_WR_H5_BIAS( const char *, struct param_record, 
				 unsigned int, const struct bias_scia * );
extern void SCIA_OL2_WR_H5_NFIT( const char *, struct param_record, 
				 unsigned int, const struct nfit_scia * );
extern void SCIA_OL2_WR_H5_LFIT( const char *, struct param_record, 
				 unsigned int, const struct lfit_scia * );
extern void SCIA_OL2_WR_H5_LCLD( struct param_record, 
				 unsigned int, const struct lcld_scia * );
#endif /* _HDF5_H */

#ifdef LIBPQ_FE_H
extern void SCIA_OL2_WR_SQL_META( PGconn *conn, bool, const char *, 
                                  const char *, const struct mph_envi *,
                                  const struct sph_sci_ol * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void SCIA_OL2_WR_SQL_CLD( PGconn *conn, bool, const char *, unsigned int,
                                 const struct ngeo_scia *,
                                 const struct cld_sci_ol * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;

extern void SCIA_OL2_WR_SQL_NFIT( PGconn *conn, bool, const char *, 
                                  const char *, 
                                  unsigned int, const struct ngeo_scia *,
                                  unsigned int, const struct nfit_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void SCIA_OL2_MATCH_STATE( PGconn *conn, bool, const struct mph_envi *,
                                  unsigned short, const struct state2_scia * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void SCIA_OL2_DEL_ENTRY( PGconn *conn, bool, const char * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
#endif /* LIBPQ_FE_H */

#ifdef __cplusplus
  }
#endif
#endif /* __NADC_SCIA_LV2 */
