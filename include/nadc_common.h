/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_COMMON
.AUTHOR      R.M. van Hees
.KEYWORDS    header file
.LANGUAGE    ANSI C
.PURPOSE     macro and structures shared by all NADC routines
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     14-Mar-2013   initial release by R. M. van Hees
------------------------------------------------------------*/
#ifndef  __NADC_COMMON                            /* Avoid redefinitions */
#define  __NADC_COMMON

/*+++++ Additional include files +++++*/
#include <errno.h>
#include <stdbool.h>

#if !defined(_SIZE_T) && !defined(_SIZE_T_)
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*+++++ General Macros +++++*/
#ifndef TRUE
#define TRUE            true
#endif
#ifndef FALSE
#define FALSE           false
#endif

#ifdef __GNUC__
#define min_t(type,x,y) __extension__  \
        ({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })
#define max_t(type,x,y) __extension__ \
        ({ type __x = (x); type __y = (y); __x > __y ? __x: __y; })
#else
#define min_t(type,x,y) \
        ((type)(x) < (type)(y) ? (type)(x): (type)(y))
#define max_t(type,x,y) \
        ((type)(x) > (type)(y) ? (type)(x): (type)(y))
#endif

/*+++++ some useful constants +++++*/
#ifndef PI
#ifndef M_PI
#define PI              3.14159265358979323846
#else
#define PI              M_PI
#endif
#endif
#define DEG2RAD         (PI / 180.)

#define R_EARTH         6378.137     /* equatorial radius of the Earth (km) */

#define CHAR_ZERO    ((char) 0)
#define SCHAR_ZERO   ((signed char) 0)
#define UCHAR_ZERO   ((unsigned char) 0)
#define UCHAR_ONE    ((unsigned char) 1)
#define USHRT_ZERO   ((unsigned short) 0)
#define UINT_ZERO    ((unsigned int) 0)

/* constants used by nadc_akima.c */
#define AKIMA_EXTRA_POINTS         4

/* constants used by nadc_init_param.c */
#define PARAM_SET    ((unsigned char) TRUE)
#define PARAM_UNSET  ((unsigned char) FALSE)

#define SHORT_STRING_LENGTH        ((size_t) 80)
#define MAX_STRING_LENGTH          ((size_t) 256)
#define UTC_STRING_LENGTH          29
#define DATE_STRING_LENGTH         26
#define DATE_ONLY_STRING_LENGTH    11
#define TIME_ONLY_STRING_LENGTH    12

/* filename sizes for different platforms */
#define ENVI_FILENAME_SIZE         63
#define ERS2_FILENAME_SIZE         67

/* maximum number of SCIAMACHY states ID's and clusters */
#define MAX_NUM_CLUS      64
#define MAX_NUM_STATE     70

/*
 * general PDS definitions (not data level dependent)
 */
#define PDS_MPH_LENGTH        1247
#define PDS_DSD_LENGTH         280

#define PDS_ASCII_HDR_LENGTH   (PDS_DSD_LENGTH+2)
#define PDS_KEYWORD_LENGTH      41
#define PDS_KEYVAL_LENGTH      192

#define ENVI_CHAR   ((size_t) 1)
#define ENVI_UCHAR  ((size_t) 1)
#define ENVI_SHORT  ((size_t) 2)
#define ENVI_USHRT  ((size_t) 2)
#define ENVI_INT    ((size_t) 4)
#define ENVI_UINT   ((size_t) 4)
#define ENVI_LLONG  ((size_t) 8)
#define ENVI_FLOAT  ((size_t) 4)
#define ENVI_DBLE   ((size_t) 8)

/* 
 * macros to perform rounding-off to various integer types
 */
#define NINT(a) ((a) >= 0.f ? (int)((a)+0.5) : (int)((a)-0.5))

#define __ROUNDf_uc(val) __extension__ ({   \
     __typeof__(val) _val = roundf(val);    \
     (_val < 0) ? (unsigned char) 0 :       \
          (_val < UCHAR_MAX) ? (unsigned char) (_val) : UCHAR_MAX;  \
          })

#define __ROUNDf_us(val) __extension__ ({   \
     __typeof__(val) _val = roundf(val);    \
     (_val < 0) ? (unsigned short) 0 :      \
          (_val < USHRT_MAX) ? (unsigned short) (_val) : USHRT_MAX; \
          })

#define __ROUND_us(val) __extension__ ({    \
     __typeof__(val) _val = round(val);     \
     (_val < 0) ? (unsigned short) 0 :      \
          (_val < USHRT_MAX) ? (unsigned short) (_val) : USHRT_MAX; \
          })

/* return Longitude within range <-180, 180] */
#define LON_IN_RANGE(lon) ((lon > 180.f) ? (lon - 360.f) : (lon))

/*+++++ enumeration constants +++++*/
enum ctypes_def { 
     UINT8_T = 1, 
     UINT16_T, 
     INT16_T, 
     UINT32_T, 
     INT32_T, 
     FLT32_T, 
     FLT64_T 
};

/* constants used by nadc_flip.c */
enum nadc_flip {
     NADC_FLIP_NO = 0,
     NADC_FLIP_X,
     NADC_FLIP_Y,
     NADC_FLIP_XY
};

enum pds_hdr_type { PDS_Unknown, PDS_Optional, PDS_Spare, PDS_String,
                    PDS_Plain, PDS_Short, PDS_uShort, PDS_Long, PDS_uLong,
                    PDS_Ado06, PDS_Ado46, PDS_Ado73, PDS_Ado18e };

/* structure definitions */
struct param_record
{
     unsigned char flag_infile;                 /* name of input file given */
     unsigned char flag_outfile;               /* name of output file given */

     unsigned char flag_check;
     unsigned char flag_show;
     unsigned char flag_version;
     unsigned char flag_silent;
     unsigned char flag_verbose;

     unsigned char flag_cloud;
     unsigned char flag_geoloc;
     unsigned char flag_geomnmx;
     unsigned char flag_period;
     unsigned char flag_pselect;
     unsigned char flag_subset;
     unsigned char flag_sunz;
     unsigned char flag_wave;

     unsigned char qcheck;

     unsigned char write_pds;
     unsigned char write_ascii;
     unsigned char write_hdf5;
     unsigned char flag_deflate;
     unsigned char write_meta;
     unsigned char write_sql;
     unsigned char flag_sql_remove;
     unsigned char flag_sql_replace;

     unsigned char write_lv1c;

     unsigned char write_subset;
     unsigned char write_blind;
     unsigned char write_stray;
     unsigned char write_aux0;
     unsigned char write_pmd0;
     unsigned char write_aux;
     unsigned char write_det;
     unsigned char write_pmd;
     unsigned char write_pmd_geo;
     unsigned char write_polV;

     unsigned char write_ads;
     unsigned char write_gads;

     unsigned char write_limb;
     unsigned char write_moni;
     unsigned char write_moon;
     unsigned char write_nadir;
     unsigned char write_occ;
     unsigned char write_sun;

     unsigned char write_bias;
     unsigned char write_cld;
     unsigned char write_doas;

     unsigned char catID_nr;   
     unsigned char stateID_nr;   
     unsigned char clusID_nr;

     unsigned char chan_mask;

     unsigned short patch_scia;
     unsigned short calib_earth;
     unsigned short calib_limb;
     unsigned short calib_moon;
     unsigned short calib_sun;
     unsigned short calib_pmd;
     unsigned int   calib_scia;

     int   hdf_file_id;

     unsigned long long clus_mask;

     unsigned char  catID[MAX_NUM_STATE];
     unsigned char  stateID[MAX_NUM_STATE];

     char  bgn_date[DATE_STRING_LENGTH];
     char  end_date[DATE_STRING_LENGTH];

     char  pselect[MAX_STRING_LENGTH];

     char  program[SHORT_STRING_LENGTH];
     char  infile[MAX_STRING_LENGTH-3];
     char  outfile[MAX_STRING_LENGTH];
     char  hdf5_name[MAX_STRING_LENGTH];

     float cloud[2];
     float geo_lat[2];
     float geo_lon[2];
     float sunz[2];
     float wave[2];
};

#define MAX_ADAGUC_INFILES   256
struct param_adaguc
{
     unsigned char flag_show;
     unsigned char flag_version;
     unsigned char flag_silent;
     unsigned char flag_verbose;

     unsigned char flag_indir;
     unsigned char flag_outdir;
     unsigned char flag_clip;

     char prodClass[5];

     char clipStart[16];
     char clipStop[16];

     unsigned short num_infiles;
     char *name_infiles[MAX_ADAGUC_INFILES];

     char indir[MAX_STRING_LENGTH];
     char outdir[MAX_STRING_LENGTH];
};

/* common Envisat PDS data structures */
typedef struct NADC_pds_hdr_t {
     const char key[PDS_KEYWORD_LENGTH];
     int  type;
     const unsigned int length;
     char value[PDS_KEYVAL_LENGTH];
     /*@null@*/ const char *unit;
} NADC_pds_hdr_t;

struct mjd_envi
{
     int days;
     unsigned int secnd;
     unsigned int musec;
};

struct coord_envi
{
     int lat;
     int lon;
};

struct mph_envi
{
     char  product[ENVI_FILENAME_SIZE];
     char  proc_stage[2];
     char  ref_doc[24];

     char  acquis[21];
     char  proc_center[7];
     char  proc_time[UTC_STRING_LENGTH];
     char  soft_version[15];

     char  sensing_start[UTC_STRING_LENGTH];
     char  sensing_stop[UTC_STRING_LENGTH];

     char  phase[2];
     short cycle;
     int   rel_orbit;
     int   abs_orbit;
     char  state_vector[UTC_STRING_LENGTH];
     double delta_ut;
     double x_position;
     double y_position;
     double z_position;
     double x_velocity;
     double y_velocity;
     double z_velocity;
     char  vector_source[3];

     char  utc_sbt_time[UTC_STRING_LENGTH];
     unsigned int sat_binary_time;
     unsigned int clock_step;

     char  leap_utc[UTC_STRING_LENGTH];
     short leap_sign;
     char  leap_err[2];

     char  product_err[2];
     unsigned int tot_size;
     unsigned int sph_size;
     unsigned int num_dsd;
     unsigned int dsd_size;
     unsigned int num_data_sets;
};

struct dsd_envi
{
     char name[29];
     char type[2];
     char flname[ENVI_FILENAME_SIZE];
     unsigned int offset;
     unsigned int size;
     unsigned int num_dsr;
     int dsr_size;
};

struct state_list_rec
{
     unsigned char stateID;
     unsigned char intg_time;
     unsigned short numObs;
};

/* macro definitions and prototype declarations error handling */
#include <nadc_error.h>

/* 
 * prototype declarations of functions common to all libraries
 */
extern /*@null@*/ /*@out@*/ /*@only@*/ 
       unsigned char **ALLOC_UC2D( size_t, size_t )
     /*@globals nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/;
extern /*@null@*/ /*@out@*/ /*@only@*/ 
       char **ALLOC_C2D( size_t, size_t )
     /*@globals nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/;
extern /*@null@*/ /*@out@*/ /*@only@*/ 
       unsigned short **ALLOC_US2D( size_t, size_t )
     /*@globals nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/;
extern /*@null@*/ /*@out@*/ /*@only@*/ 
       short **ALLOC_S2D( size_t, size_t )
     /*@globals nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/;
extern /*@null@*/ /*@out@*/ /*@only@*/ 
       short ***ALLOC_S3D( size_t, size_t, size_t )
     /*@globals nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/;
extern /*@null@*/ /*@out@*/ /*@only@*/ 
       int **ALLOC_I2D( size_t, size_t )
     /*@globals nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/;
extern /*@null@*/ /*@out@*/ /*@only@*/ 
       float **ALLOC_R2D( size_t, size_t )
     /*@globals nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/;
extern /*@null@*/ /*@out@*/ /*@only@*/ 
       double **ALLOC_D2D( size_t, size_t )
     /*@globals nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/;
extern /*@null@*/ /*@out@*/ /*@only@*/ 
       int ***ALLOC_I3D( size_t, size_t, size_t )
     /*@globals nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/;
extern /*@null@*/ /*@out@*/ /*@only@*/ 
       int ****ALLOC_I4D( size_t, size_t, size_t, size_t )
     /*@globals nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/;
extern void FREE_2D( /*@notnull@*/ /*@only@*/ void **p ) 
     /*@modifies p, *p@*/;
extern void FREE_3D( /*@notnull@*/ /*@only@*/ void ***p ) 
     /*@modifies p, *p, **p@*/;
extern void FREE_4D( /*@notnull@*/ /*@only@*/ void ****p ) 
     /*@modifies p, *p, **p, ***p@*/;

extern int BinarySearch( int, const int *, const int *, int );

extern size_t NADC_BIWEIGHT( const size_t, const float *, 
			     /*@out@*/ float *median,
			     /*@out@*/ /*@null@*/ float *scale )
     /*@globals  nadc_stat, nadc_err_stack, errno;@*/
     /*@modifies nadc_stat, nadc_err_stack, errno, median, scale@*/;
extern void NADC_INTERPOL( float, float, float, unsigned int, const float *, 
			   const float *, /*@unique@*/ float *Y )
      /*@globals  nadc_stat, nadc_err_stack;@*/
      /*@modifies nadc_stat, nadc_err_stack, Y@*/;
extern void NADC_INTERPOL_d( float, float, float, unsigned int, const float *, 
			   const float *, /*@unique@*/ double *Y )
      /*@globals  nadc_stat, nadc_err_stack;@*/
      /*@modifies nadc_stat, nadc_err_stack, Y@*/;

extern unsigned char SELECTuc( const size_t, const size_t, 
			       const unsigned char * );
extern short  SELECTs( const size_t, const size_t, const short  * );
extern int    SELECTi( const size_t, const size_t, const int    * );
extern float  SELECTr( const size_t, const size_t, const float  * );
extern double SELECTd( const size_t, const size_t, const double * );

extern size_t NADC_SIGMACLIPPED( const size_t, const float *, 
				 /*@out@*/ float *mean,
				 /*@out@*/ float *sdev )
     /*@globals  nadc_stat, nadc_err_stack, errno;@*/
     /*@modifies nadc_stat, nadc_err_stack, errno, mean, sdev@*/;

extern size_t nadc_strlcpy( /*@out@*/ char *, 
			    /*@unique@*/ const char *, size_t );
extern size_t nadc_strlcat( /*@out@*/ char *, 
			    /*@unique@*/ const char *, size_t );
extern void nadc_rstrip( /*@out@*/ char *, /*@unique@*/ const char * );

extern void Set_Bit_LL( unsigned long long *, unsigned char );
extern unsigned long long Get_Bit_LL( unsigned long long, unsigned char )
     __attribute__ ((const));

extern void Julian_2_MJD( double, /*@out@*/ double *, 
			  /*@out@*/ unsigned int *, /*@out@*/ unsigned int * );
extern void MJD_2_Julian( double, unsigned int, unsigned int, 
			  /*@out@*/ double * );
extern void ASCII_2_UTC( const char *, 
			/*@out@*/ unsigned int *, /*@out@*/ unsigned int * )
     /*@globals errno;@*/;
extern void ASCII_2_MJD( const char ASCII_DateTime[], /*@out@*/ int *, 
			 /*@out@*/ unsigned int *, /*@out@*/ unsigned int * )
     /*@globals errno;@*/;
extern double DATETIME_2_JULIAN( const char *, unsigned int )
     /*@globals errno;@*/;
extern void UTC_2_ASCII( unsigned int, unsigned int, /*@out@*/ char * );
extern void UTC_2_DATETIME( unsigned int, unsigned int, /*@out@*/ char * );
extern void MJD_2_ASCII( int, unsigned int, unsigned int, /*@out@*/ char * );
extern void MJD_2_DATETIME( int, unsigned int, unsigned int, /*@out@*/char * );
extern void MJD_2_YMD( int, unsigned int, /*@out@*/ char * );
extern void GomeJDAY2adaguc( double, /*@out@*/ char *dateTime );
extern double Adaguc2gomeJDAY( const char * );
extern void SciaJDAY2adaguc( double, /*@out@*/ char *dateTime );
extern double Adaguc2sciaJDAY( const char * );

extern void NADC_AKIMA_SU( int, int, size_t, const void *, const void *, 
		    /*@out@*/ double *a_coef, /*@out@*/ double *b_coef, 
		    /*@out@*/ double *c_coef, /*@out@*/ double *d_coef )
      /*@globals  nadc_stat, nadc_err_stack;@*/
      /*@modifies nadc_stat, nadc_err_stack, a_coef, b_coef, c_coef, d_coef@*/;
extern double NADC_AKIMA_PO( size_t, const double *, const double *, 
			     const double *, const double *, const double *, 
			     double );
extern void  FIT_GRID_AKIMA( int, int, size_t, const void *, const void *, 
			     int, int, size_t, const void *, 
			     /*@out@*/ void *yres )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, yres@*/;

extern void NADC_FIT( size_t, const float *, const float *, const float *,
		      /*@out@*/ float *fit_a, 
		      /*@null@*/ /*@out@*/ float *fit_b, 
		      /*@out@*/ float *sig_a, /*@out@*/ float *sig_b, 
		      /*@out@*/ float *chisq, 
		      /*@null@*/ /*@out@*/ float *q_fit  )
       /*@globals  errno;@*/
       /*@modifies errno, *fit_a, *fit_b, *sig_a, *sig_b, *chisq, *q_fit@*/;

extern void NADC_MEDFIT( size_t, const float *, const float *, 
		      /*@out@*/ float *fit_a, /*@out@*/ float *fit_b, 
		      /*@out@*/ float *abdev )
       /*@globals  errno;@*/
       /*@modifies errno, *fit_a, *fit_b, *abdev@*/;

extern void NADC_RECEIVEDATE( const char *, /*@out@*/ char *datetime )
       /*@modifies datetime@*/;

extern bool NADC_CHECK_FOR_SAA( const double, const double )
     __attribute__ ((const));

extern unsigned int nadc_file_size( const char * );

extern bool nadc_file_equal( const char *, const char * )
       /*@globals  errno;@*/
       /*@modifies errno@*/;

extern bool nadc_file_exists( const char * )
       /*@globals  errno;@*/
       /*@modifies errno@*/;

extern void NADC_FLIPc( enum nadc_flip, const unsigned int *, signed char *matrix )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies matrix, nadc_stat, nadc_err_stack@*/;
extern void NADC_FLIPu( enum nadc_flip, const unsigned int *, unsigned char *matrix )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies matrix, nadc_stat, nadc_err_stack@*/;
extern void NADC_FLIPs( enum nadc_flip, const unsigned int *, short *matrix )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies matrix, nadc_stat, nadc_err_stack@*/;
extern void NADC_FLIPr( enum nadc_flip, const unsigned int *, float *matrix )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies matrix, nadc_stat, nadc_err_stack@*/;

extern void ADAGUC_INIT_PARAM( int, char **, 
			       /*@out@*/ struct param_adaguc *param )
     /*@globals  errno, stderr, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, stderr, nadc_stat, nadc_err_stack, param@*/;

extern void NADC_INIT_PARAM( /*@out@*/ struct param_record *param )
     /*@globals  errno, stderr, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, stderr, nadc_stat, nadc_err_stack, param@*/;

extern void NADC_USRINP( int , /*@unique@*/ const char *, int , 
			/*@out@*/ void *pntr, /*@out@*/ int *nrval )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, *nrval, *pntr@*/;

extern short NADC_USRINDX( const char *, int, /*@out@*/ short *indices )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, *indices@*/;

extern void ENVI_WR_ASCII_MPH( struct param_record, 
			       const struct mph_envi * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;
extern void ENVI_WR_ASCII_DSD( struct param_record, unsigned int, 
			       const struct dsd_envi * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack@*/;

extern unsigned int ENVI_GET_DSD_INDEX( unsigned int, const struct dsd_envi *,
					const char * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;

#if defined _STDIO_H || defined _STDIO_H_
extern unsigned int ENVI_RD_PDS_INFO( FILE *fp, /*@out@*/ char *keyword, 
                                      /*@out@*/ char *keyvalue )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, keyword, keyvalue@*/;

extern void ENVI_RD_MPH( FILE *fp, /*@out@*/ struct mph_envi *mph )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *mph@*/;
extern void ENVI_WR_MPH( FILE *fp, const struct mph_envi )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp@*/;
extern unsigned int ENVI_RD_DSD( FILE *fp, const struct mph_envi,
				 /*@out@*/ struct dsd_envi *dsd_out )
       /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fp, *dsd_out@*/;
extern void ENVI_WR_DSD( FILE *fp, const unsigned int,
			 const struct dsd_envi *dsd_out )
       /*@globals  errno;@*/
       /*@modifies errno, fp@*/;

extern void NADC_CopyRight( FILE *stream )
     /*@modifies stream@*/;
extern void NADC_SHOW_VERSION( FILE *stream, const char * )
     /*@modifies stream@*/;

extern void NADC_Info_Proc( FILE *stream, const char *, unsigned int )
     /*@modifies stream@*/;
extern void NADC_Info_Update( FILE *stream, unsigned short, unsigned int )
     /*@modifies stream@*/;
extern void NADC_Info_Finish( FILE *stream, unsigned short, unsigned int )
     /*@modifies stream@*/;

extern /*@null@*/ /*@dependent@*/
       FILE *CRE_ASCII_File( const char *, const char * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern /*@null@*/ /*@dependent@*/
       FILE *CAT_ASCII_File( const char *, const char * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;
extern void nadc_write_text( FILE *fp, unsigned int, const char *, 
			     const char * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_bool( FILE *fp, unsigned int, const char *, bool )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_schar( FILE *fp, unsigned int, const char *, 
			      signed char )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_uchar( FILE *fp, unsigned int, const char *, 
			      unsigned char )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_short( FILE *fp, unsigned int, const char *,  short )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_ushort( FILE *fp, unsigned int, const char *, 
			       unsigned short )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_int( FILE *fp, unsigned int, const char *, int )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_uint( FILE *fp, unsigned int, const char *, 
			     unsigned int )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_long( FILE *fp, unsigned int, const char *, long )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_float( FILE *fp, unsigned int, const char *, int, 
			      float )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_double( FILE *fp, unsigned int, const char *, int, 
			       double )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_arr_schar( FILE *fp, unsigned int, const char *, int, 
				  const unsigned int *, const signed char * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_arr_uchar( FILE *fp, unsigned int, const char *, int, 
				  const unsigned int *, 
				  const unsigned char * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_arr_short( FILE *fp, unsigned int, const char *, int, 
				  const unsigned int *, const short * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_arr_ushort( FILE *fp, unsigned int, const char *, int, 
				   const unsigned int *, 
				   const unsigned short * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_arr_int( FILE *fp, unsigned int, const char *, int, 
				const unsigned int *, const int * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_arr_uint( FILE *fp, unsigned int, const char *, int, 
				 const unsigned int *, const unsigned int * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_arr_float( FILE *fp, unsigned int, const char *, int, 
				  const unsigned int *, int, const float * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_arr_double( FILE *fp, unsigned int, const char *, int, 
				   const unsigned int *, int, const double *)
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;
extern void nadc_write_header( FILE *fp, unsigned int, const char *, 
			       const char * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies fp, nadc_stat, nadc_err_stack@*/;

extern void NADC_GET_XML_METADB( FILE *stream, /*@out@*/ char *, 
				 /*@out@*/ char *, /*@out@*/ char *,
				 /*@out@*/ char * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies stream, nadc_stat, nadc_err_stack@*/;

#endif   /* ---- defined _STDIO_H || defined _STDIO_H_ ----- */

#ifdef _HDF5_H
extern hid_t NADC_OPEN_HDF5_Group( hid_t, const char * );

extern void NADC_WR_HDF5_Attribute( hid_t, const char *, hid_t,
				    int, const hsize_t *, const void * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;

extern void NADC_RD_HDF5_Dataset( hid_t, const char *, hid_t, 
				  /*@out@*/ int *rank, 
				  /*@out@*/ hsize_t *dims, 
                                  /*@out@*/ void **data_out )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, rank, dims, *data_out*/;

extern void NADC_WR_HDF5_Dataset( hbool_t, hid_t, const char *,
				  hid_t, int, const hsize_t *, const void * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;

extern void NADC_WR_HDF5_Vlen_Dataset( hbool_t, hid_t, const char *, 
				       hid_t, int, const hsize_t *, 
				       /*@only@*/ void * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;

extern void NADC_CRE_H5_EArray_uint8( hid_t, const char *,
				      const hsize_t, int, 
				      const unsigned char * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;
extern void NADC_CRE_H5_EArray_uint16( hid_t, const char *,
				       const hsize_t, int,
				       const unsigned short * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;
extern void NADC_CRE_H5_EArray_int32( hid_t, const char *,
				      const hsize_t, int,
				      const int * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;
extern void NADC_CRE_H5_EArray_uint32( hid_t, const char *,
				       const hsize_t, int,
				       const unsigned int * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;
extern void NADC_CRE_H5_EArray_float( hid_t, const char *,
				      const hsize_t, int, const float * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;
extern void NADC_CRE_H5_EArray_struct( hid_t, const char *, const hsize_t, 
				       int, hid_t, const void * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;
extern void NADC_CAT_H5_EArray( hid_t, const char *, 
				int, size_t, const void * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;
extern void NADC_WR_H5_EArray( hid_t, const char *, 
			       size_t, size_t, const void * )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack*/;

extern void NADC_RD_H5_EArray_uint8( hid_t, const char *, hsize_t, int, 
				     const int *, 
				     /*@out@*/ unsigned char *buffer )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, buffer*/;
extern void NADC_RD_H5_EArray_uint16( hid_t, const char *, hsize_t, int, 
				      const int *, 
				      /*@out@*/ unsigned short *buffer )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, buffer*/;
extern void NADC_RD_H5_EArray_int32( hid_t, const char *, hsize_t, int, 
				     const int *, /*@out@*/ int *buffer )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, buffer*/;
extern void NADC_RD_H5_EArray_uint32( hid_t, const char *, hsize_t, int, 
				      const int *, 
				      /*@out@*/ unsigned int *buffer )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, buffer*/;
extern void NADC_RD_H5_EArray_float( hid_t, const char *, hsize_t, int, 
				     const int *, /*@out@*/ float *buffer )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, buffer*/;

extern hid_t PYTABLE_open_file( const char *, const char * );
extern hid_t PYTABLE_open_group( hid_t, const char * );
extern herr_t PYTABLE_make_array( hid_t, const char *, const char *,
				  const int, const hsize_t *, int, hid_t, 
				  const hsize_t *, /*@null@*/ void *,
				  unsigned int, bool, bool, const void * );
extern herr_t PYTABLE_append_array( hid_t, const char *, 
				    int, int, const void * );
extern herr_t PYTABLE_write_records( hid_t, const char *, hsize_t *, hsize_t *,
				     hsize_t *, const void * );
#endif   /* ---- defined _HDF5_H ----- */

#ifdef __cplusplus
  }
#endif
#endif    /* __NADC_COMMON */

