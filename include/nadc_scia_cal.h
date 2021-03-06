#ifndef  __DEFS_SCIA_CAL                        /* Avoid redefinitions */
#define  __DEFS_SCIA_CAL

#define _SCIA_LEVEL_1
#include <nadc_scia.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCIA_MEM_SCALE_OLD    ((unsigned char) 0x1U)
#define SCIA_MEM_SCALE_ATBD   ((unsigned char) 0x2U)
#define SCIA_MEM_SCALE_PATCH  ((unsigned char) 0x4U)

#define SCIA_NLIN_SCALE_OLD   ((unsigned char) 0x1U)
#define SCIA_NLIN_SCALE_ATBD  ((unsigned char) 0x2U)
#define SCIA_NLIN_SCALE_PATCH ((unsigned char) 0x4U)

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
     size_t dims_ghost[2];
     float **ghosts;
};

#if defined _STDIO_H || defined _STDIO_H_ || defined S_SPLINT_S
struct file_rec {
     FILE *fp;
     char procName[ENVI_FILENAME_SIZE];
     char procStage[2];
    /* KB: Added for mfactors, agreed to switch according to start time */
     char sensing_start[UTC_STRING_LENGTH];
     char do_use_limb_dark[2];
     char do_pixelwise[SCIENCE_CHANNELS+1];
     char do_pol_point[NUM_FRAC_POLV];
     char do_var_lc_cha[4 * IR_CHANNELS+1];
     char do_stray_lc_cha[4 * SCIENCE_CHANNELS+1];
     bool flagInitFile;
     bool flagInitPhase;
     bool do_nonlin;
     unsigned char sdmf_version;
     unsigned char memScale;
     unsigned char nlinScale;
     int  absOrbit;
     unsigned int num_dsd;
     unsigned int calibFlag;
     float ppgError;
     float strayError;
     float alpha0_asm;
     float alpha0_esm;
     float lambda_end_gdf;
     unsigned char level_2_smr[SCIENCE_CHANNELS];
     float electron_bu[SCIENCE_CHANNELS];
     struct dsd_envi *dsd;
};
#endif

struct wvlen_rec
{
     float solar[SCIENCE_PIXELS];
     float science[SCIENCE_PIXELS];
     float error[SCIENCE_PIXELS];
};

struct DarkRec {
     float AnalogOffs[SCIENCE_PIXELS];
     float AnalogOffsError[SCIENCE_PIXELS];
     float MeanNoise[SCIENCE_PIXELS];
     float DarkCurrent[SCIENCE_PIXELS];
     float DarkCurrentError[SCIENCE_PIXELS];
};

struct RadSens_rec {
     unsigned short n_scan;
     double         **value;
};

/*
 * prototype declarations of Sciamachy calibration functions
 */
extern void SCIA_RD_H5_MEM( /*@out@*/ struct scia_memcorr *mem )
       /*@globals nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, mem@*/;
extern void SCIA_FREE_H5_MEM( struct scia_memcorr *mem );
extern void SCIA_RD_H5_NLIN( /*@out@*/ struct scia_nlincorr *nlin )
       /*@globals nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, nlin@*/;
extern void SCIA_FREE_H5_NLIN( struct scia_nlincorr *nlin );
extern void SCIA_RD_H5_STRAY( /*@out@*/ struct scia_straycorr *stray )
       /*@globals nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, stray@*/;
extern void SCIA_FREE_H5_STRAY( struct scia_straycorr *stray );

#if defined _STDIO_H || defined _STDIO_H_ || defined S_SPLINT_S
extern void SCIA_LV1_CAL( FILE *fp, unsigned int,
			  const struct state1_scia *,
			  const struct mds1_scia *, 
			  struct mds1c_scia *mds_1c )
     /*@globals errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc, 
                internalState;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc, 
                 internalState, fp, mds_1c->rad_units_flag, 
		 mds_1c->pixel_wv, mds_1c->pixel_wv_err, 
		 mds_1c->pixel_val, mds_1c->pixel_err@*/;

extern void SCIA_LV1_PATCH_MDS( FILE *fp, unsigned short, 
				const struct state1_scia *, 
				struct mds1_scia *mds )
      /*@globals  errno, stderr, nadc_stat, nadc_err_stack, internalState;@*/
      /*@modifies errno, stderr, nadc_stat, nadc_err_stack, fp, mds, 
	          internalState@*/;

#endif   /* ---- defined _STDIO_H || defined _STDIO_H_ ----- */

extern void SCIA_LV1C_FLAG_BDPM( unsigned short, unsigned short, 
				 struct mds1c_scia *mds_1c )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp,
       mds_1c->pixel_val@*/;

#if (defined _SCIA_LEVEL_1)
extern void SCIA_SRON_CAL_MEM( unsigned short, struct mds1c_scia * )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp,
       mds_1c->pixel_val@*/;
extern void SCIA_SRON_CAL_NLIN( unsigned short, struct mds1c_scia * )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp,
       mds_1c->pixel_val@*/;

#if defined _STDIO_H || defined _STDIO_H_ || defined S_SPLINT_S
extern void SCIA_ATBD_FLAG_BDPM( const struct file_rec *fileParam,
				 const struct state1_scia *, 
				 struct mds1c_scia *mds_1c )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp,
       mds_1c->pixel_val@*/;

extern void SCIA_SRON_FLAG_BDPM( const struct file_rec *fileParam,
				 const struct state1_scia *, 
				 struct mds1c_scia * )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp,
       mds_1c->pixel_val@*/;

extern void SCIA_ATBD_CAL_DARK( const struct file_rec *fileParam,
				const struct state1_scia *,
				struct mds1c_scia *mds_1c )
     /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp,
       mds_1c->pixel_val, mds_1c->pixel_err@*/;
extern void SCIA_SRON_CAL_DARK( const struct file_rec *fileParam,
				const struct state1_scia *,
				struct mds1c_scia *mds_1c )
     /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp,
       mds_1c->pixel_val, mds_1c->pixel_err@*/;
extern void SCIA_STATE_CAL_DARK( const struct file_rec *fileParam,
				 const struct state1_scia *,
				 struct mds1c_scia *mds_1c )
     /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp,
       mds_1c->pixel_val, mds_1c->pixel_err@*/;

extern void SCIA_ATBD_CAL_ETALON( const struct file_rec *,
				  const struct state1_scia *, 
				  struct mds1c_scia * );

extern void SCIA_SRON_CAL_TRANS( const struct file_rec *,
				 const struct state1_scia *, 
				 struct mds1c_scia * )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp,
       mds_1c->pixel_val@*/;

extern void SCIA_ATBD_CAL_PPG( const struct file_rec *fileParam,
			       const struct state1_scia *, 
			       struct mds1c_scia *mds_1c )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp,
       mds_1c->pixel_val, mds_1c->pixel_err@*/;
extern void SCIA_SRON_CAL_PPG( const struct file_rec *fileParam,
			       const struct state1_scia *, 
			       struct mds1c_scia *mds_1c )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp,
       mds_1c->pixel_val, mds_1c->pixel_err@*/;

extern void SCIA_ATBD_CAL_POL( const struct file_rec *,
			       const struct wvlen_rec ,
			       const struct state1_scia *, 
			       const struct mds1_scia *,
			       struct mds1c_scia *mds_1c )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp,
       mds_1c->pixel_val, mds_1c->pixel_err@*/;

extern void SCIA_ATBD_CAL_RAD( const struct file_rec *,
			       const struct wvlen_rec,
			       const struct state1_scia *,
			       struct mds1c_scia * )
     /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp,
       mds_1c->pixel_val, mds_1c->pixel_err@*/;

extern void SCIA_ATBD_CAL_REFL( const struct file_rec *,
				const struct state1_scia *,
				struct mds1c_scia * )
     /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/;

extern void SCIA_SRON_CAL_REFL( const struct file_rec *fileParam,
                                const struct state1_scia *state,
                                struct mds1c_scia *mds_1c )
     /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/;

extern void SCIA_ATBD_INIT_WAVE( const struct file_rec *fileParam,
				 float, struct wvlen_rec *wvlen )
      /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp,
	wvlen->science, wvlen->error, wvlen->solar@*/;

extern void SCIA_SRON_CAL_NOISE( const struct file_rec *,
				 const struct state1_scia *, 
				 struct mds1c_scia *mds_1c )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, mds_1c->pixel_err@*/;
#endif   /* ---- defined _STDIO_H || defined _STDIO_H_ ----- */

extern void SCIA_ATBD_CAL_MEM( unsigned char,
			       const struct state1_scia *,
			       const struct mds1_scia *,
			       struct mds1c_scia * );

extern void SCIA_ATBD_CAL_NLIN( unsigned char,
				const struct state1_scia *,
				const struct mds1_scia *,
				struct mds1c_scia * );

extern void SCIA_ATBD_CAL_STRAY( float, const struct state1_scia *, 
				 const struct mds1_scia *,
				 struct mds1c_scia * );

extern void SCIA_ATBD_CAL_WAVE( const struct wvlen_rec, 
				const struct state1_scia *,
				struct mds1c_scia * );

extern void SCIA_LV1C_SCALE( unsigned int, struct mds1c_scia *mds_1c )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, mds_1c->pixel_val@*/;
#endif

extern void SCIA_SMR_CAL_RAD( unsigned short, unsigned short, 
			      float, float, /*@null@*/ const float *, 
			      /*@out@*/ float *smr )
      /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack, smr@*/;

#ifdef __cplusplus
  }
#endif
#endif /* __DEFS_SCIA_CAL */
