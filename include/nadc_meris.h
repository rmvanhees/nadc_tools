/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2008 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_MERIS
.AUTHOR      R.M. van Hees 
.KEYWORDS    header file
.LANGUAGE    ANSI-C
.PURPOSE     macros and structures for MERIS data modules
.ENVIRONment none
.VERSION      1.0   18-Sep-2008 Creation by R.M. van Hees
------------------------------------------------------------*/
#ifndef  __NADC_MERIS                            /* Avoid redefinitions */
#define  __NADC_MERIS

#include <nadc_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Most routines to read SCIAMACHY data can allocate memory internally
 * However IDL requires the use of their own memory allocation routines
 */
extern bool Use_Extern_Alloc;

/*+++++ Macros +++++*/
/*
 * MERIS PDS definitions
 */
struct sph_meris
{
     bool   trans_err;
     bool   format_err;
     bool   database;
     bool   coarse_err;
     bool   ecmwf_type;
     bool   proc_mode;
     bool   offset_comp;
     char   start_time[UTC_STRING_LENGTH];
     char   stop_time[UTC_STRING_LENGTH];
     char   descriptor[29];
     char   band_wavelen[166];
     char   bandwidth[90];
     short  stripline;
     short  slice_pos;
     unsigned short num_slices;
     unsigned short num_bands;
     unsigned short num_trans_err;
     unsigned short num_format_err;
     unsigned short line_length;
     unsigned short lines_per_tie;
     unsigned short samples_per_tie;
     float  thres_trans_err;
     float  thres_format_err;
     float  column_spacing;
     double first_first_lat;
     double first_mid_lat;
     double first_last_lat;
     double last_first_lat;
     double last_mid_lat;
     double last_last_lat;
     double first_first_lon;
     double first_mid_lon;
     double first_last_lon;
     double last_first_lon;
     double last_mid_lon;
     double last_last_lon;
     double inst_fov;
     double line_time_interval;
};

#define MERIS_NUM_TIE_POINT  71
struct tie_meris
{
     struct mjd_envi   mjd;
     unsigned char     flag_mds;
     struct coord_envi coord[MERIS_NUM_TIE_POINT];
     int               dem_altitude[MERIS_NUM_TIE_POINT];
     unsigned int      dem_roughness[MERIS_NUM_TIE_POINT];
     int               dem_lat_corr[MERIS_NUM_TIE_POINT];
     int               dem_lon_corr[MERIS_NUM_TIE_POINT];
     unsigned int      sun_zen_angle[MERIS_NUM_TIE_POINT];
     int               sun_azi_angle[MERIS_NUM_TIE_POINT];
     unsigned int      view_zen_angle[MERIS_NUM_TIE_POINT];
     int               view_azi_angle[MERIS_NUM_TIE_POINT];
     short             zonal_wind[MERIS_NUM_TIE_POINT];
     short             merid_wind[MERIS_NUM_TIE_POINT];
     unsigned short    atm_press[MERIS_NUM_TIE_POINT];
     unsigned short    ozone[MERIS_NUM_TIE_POINT];
     unsigned short    humidity[MERIS_NUM_TIE_POINT];
};


/* +++++ structures for Level 1 (FR) +++++ */

/* +++++ structures for Level 2 (RR) +++++ */
struct sqads2_meris
{
     struct mjd_envi mjd;
     unsigned char   flag_mds;
     unsigned char   ocean_aerosols;
     unsigned char   ocean_climatology;
     unsigned char   ddv_land;
     unsigned char   turbit_climatology;
     unsigned char   consolidated;
     unsigned char   spare_1;
     unsigned char   spare_2;
     unsigned char   failed_in_vapour;
     unsigned char   failed_out_vapour;
     unsigned char   failed_in_cloud;
     unsigned char   failed_out_cloud;
     unsigned char   failed_in_land;
     unsigned char   failed_out_land;
     unsigned char   failed_in_ocean;
     unsigned char   failed_out_ocean;
     unsigned char   failed_in_case1;
     unsigned char   failed_out_case1;
     unsigned char   failed_in_case2;
     unsigned char   failed_out_case2;
};

struct sfgi_meris
{
     float sf_altitude;
     float sf_roughness;
     float sf_zonal_wind;
     float sf_merid_wind;
     float sf_atm_press;
     float sf_ozone;
     float sf_humidity;
     float sf_reflectance[13];
     float sf_pigment_index;
     float sf_yellow_sub;
     float sf_sediment;
     float sf_aer_epsilon;
     float sf_aer_opt_thick;
     float sf_cld_opt_thick;
     float sf_surf_press;
     float sf_water_vapour;
     float sf_photo_rad;
     float sf_toa_veg_index;
     float sf_boa_veg_index;
     float sf_cld_albedo;
     float sf_cld_press;
     float off_reflectance[13];
     float off_pigment_index;
     float off_yellow_sub;
     float off_sediment;
     float off_aer_epsilon;
     float off_aer_opt_thick;
     float off_cld_opt_thick;
     float off_surf_press;
     float off_water_vapour;
     float off_photo_rad;
     float off_toa_veg_index;
     float off_boa_veg_index;
     float off_cld_albedo;
     float off_cld_press;
     unsigned char gains[5 * 16];
     unsigned int  sampling_rate;
     float sun_flux[15];
     float sf_nir_refl;
     float off_nir_refl;
     float sf_red_refl;
     float off_red_refl;
     unsigned char spare[44];
};

struct mds_rr2_13_meris
{
     struct mjd_envi mjd;
     char            quality_flag;
     unsigned short  norm_surf_refl[1121];
};

struct mds_rr2_14_meris
{
     struct mjd_envi mjd;
     char            quality_flag;
     unsigned char   wvaphour_content[1121];
};

struct mds_rr2_15_meris
{
     struct mjd_envi mjd;
     char            quality_flag;
     unsigned char   algal_toavi_cld[1121];
};

struct mds_rr2_16_meris
{
     struct mjd_envi mjd;
     char            quality_flag;
     struct {
	  unsigned char  ys;
	  unsigned char  tsm;
     } pixel[1121];
};

struct mds_rr2_17_meris
{
     struct mjd_envi mjd;
     char            quality_flag;
     unsigned char   algal2_boavi[1121];
};

struct mds_rr2_18_meris
{
     struct mjd_envi mjd;
     char            quality_flag;
     unsigned char   fpar_press_albedo[1121];
};

struct mds_rr2_19_meris
{
     struct mjd_envi mjd;
     char            quality_flag;
     struct {
	  unsigned char type;
	  unsigned char thick;
     } aerosol_cloud[1121];
};

struct mds_rr2_20_meris
{
     struct mjd_envi mjd;
     char            quality_flag;
     struct {
	  unsigned char flag_1;
	  unsigned char flag_2;
	  unsigned char flag_3;
     } pixel[1121];
};

/*
 * prototype declaration of Meris functions
 */
extern void MERIS_SET_PARAM( int, char **, int,
                             /*@out@*/ struct param_record *param )
     /*@globals  errno, stderr, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, stderr, nadc_stat, nadc_err_stack, param@*/;
extern void MERIS_SHOW_PARAM( int, struct param_record );

#if defined _STDIO_INCLUDED || defined _STDIO_H || defined __STDIO_H__
extern void MERIS_SHOW_VERSION( FILE *stream, const char * )
     /*@modifies stream@*/;

extern void MERIS_RD_SPH( FILE *fp, const struct mph_envi,
			  /*@out@*/ struct sph_meris *sph )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *sph@*/;

extern unsigned int MERIS_RD_TIE( FILE *, unsigned int, 
				  const struct dsd_envi *, 
				  /*@out@*/ struct tie_meris **tie )
       /*@globals  errno;@*/
       /*@modifies errno, fp, tie@*/;

extern unsigned int MERIS_RR2_RD_SQADS( FILE *fp, unsigned int, 
					const struct dsd_envi *, 
					/*@out@*/ struct sqads2_meris **sqads )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;

extern unsigned int MERIS_RR2_RD_SFGI( FILE *fp, unsigned int, 
				       const struct dsd_envi *, 
				       /*@out@*/ struct sfgi_meris *sfgi )
       /*@globals  errno;@*/
       /*@modifies errno, fp, sfgi@*/;

extern unsigned int MERIS_RR2_RD_MDS_13( int, FILE *, unsigned int, 
					 const struct dsd_envi *, /*@out@*/ 
					 struct mds_rr2_13_meris **mds )
       /*@globals  errno;@*/
       /*@modifies errno, fp, mds@*/;
extern unsigned int MERIS_RR2_RD_MDS_14( FILE *, unsigned int, 
					 const struct dsd_envi *, /*@out@*/ 
					 struct mds_rr2_14_meris **mds )
       /*@globals  errno;@*/
       /*@modifies errno, fp, mds@*/;
extern unsigned int MERIS_RR2_RD_MDS_15( FILE *, unsigned int, 
					 const struct dsd_envi *, /*@out@*/ 
					 struct mds_rr2_15_meris **mds )
       /*@globals  errno;@*/
       /*@modifies errno, fp, mds@*/;
extern unsigned int MERIS_RR2_RD_MDS_16( FILE *, unsigned int, 
					 const struct dsd_envi *, /*@out@*/ 
					 struct mds_rr2_16_meris **mds )
       /*@globals  errno;@*/
       /*@modifies errno, fp, mds@*/;
extern unsigned int MERIS_RR2_RD_MDS_17( FILE *, unsigned int, 
					 const struct dsd_envi *, /*@out@*/ 
					 struct mds_rr2_17_meris **mds )
       /*@globals  errno;@*/
       /*@modifies errno, fp, mds@*/;
extern unsigned int MERIS_RR2_RD_MDS_18( FILE *, unsigned int, 
					 const struct dsd_envi *, /*@out@*/ 
					 struct mds_rr2_18_meris **mds )
       /*@globals  errno;@*/
       /*@modifies errno, fp, mds@*/;
extern unsigned int MERIS_RR2_RD_MDS_19( FILE *, unsigned int, 
					 const struct dsd_envi *, /*@out@*/ 
					 struct mds_rr2_19_meris **mds )
       /*@globals  errno;@*/
       /*@modifies errno, fp, mds@*/;
extern unsigned int MERIS_RR2_RD_MDS_20( FILE *, unsigned int, 
					 const struct dsd_envi *, /*@out@*/ 
					 struct mds_rr2_20_meris **mds )
       /*@globals  errno;@*/
       /*@modifies errno, fp, mds@*/;
#endif   /* ---- defined _STDIO_INCLUDED || defined _STDIO_H ----- */

extern void MERIS_WR_ASCII_SPH( struct param_record, 
				const struct sph_meris * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;

extern void MERIS_WR_ASCII_TIE( struct param_record, unsigned int,
				const struct tie_meris * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;

#ifdef _HDF5_H
extern void MERIS_WR_H5_VERSION( hid_t )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern hid_t MERIS_CRE_H5_FILE( int instrument, const struct param_record * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern void MERIS_WR_H5_MPH( struct param_record, const struct mph_envi * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
#endif /* _HDF5_H */

#ifdef __cplusplus
  }
#endif
#endif /* __NADC_MERIS */
