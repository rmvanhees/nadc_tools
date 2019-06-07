/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2019 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV2_IDL
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 2, IDL
.LANGUAGE    ANSI C
.PURPOSE     IDL wrapper for reading SCIAMACHY level 2 data
.COMMENTS    None
.ENVIRONment None
.VERSION      1.3   11-Jan-2011	added read routine for SCIA LV2 LIM_CLOUDS, RvH
              1.2   12-Oct-2002	consistently return, in case of error, -1, RvH 
              1.1   02-Jul-2002	added more error checking, RvH
              1.0   12-Jan-2002	created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_2
#include <nadc_idl.h>

/*+++++ Global Variables +++++*/
extern FILE *fd_nadc;

/*+++++ Static Variables +++++*/
static const char err_msg[] = "invalid number of function arguments";

/*+++++++++++++++++++++++++ Main Program or Functions +++++++++++++++*/
/*
 * here all the Offline Product routines (alphabetically)
 */
int IDL_STDCALL _SCIA_OL2_RD_CLD (int argc, void *argv[])
{
     register int nr;

     int          nr_cld;
     unsigned int num_dsd;
     float        *data;

     struct dsd_envi   *dsd;
     struct cld_sci_ol *C_cld;

     struct IDL_cld_sci_ol
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
	  IDL_ULONG  pntr_aeropars;            /* IDL uses 32-bit addresses */
     } *cld;

    if (argc != 4) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     if (fileno(fd_nadc) == -1) 
	  NADC_GOTO_ERROR(NADC_ERR_FILE, "No open stream");

     num_dsd = *(unsigned int *) argv[0];
     dsd = (struct dsd_envi *) argv[1];
     cld = (struct IDL_cld_sci_ol *) argv[2];
     data = (float *) argv[3];

     Use_Extern_Alloc = FALSE;
     nr_cld = (int) SCIA_OL2_RD_CLD(fd_nadc, num_dsd, dsd, &C_cld);
     Use_Extern_Alloc = TRUE;
     if (IS_ERR_STAT_FATAL) return -1;

     for (nr = 0; nr < nr_cld; nr++) {
	  (void) memcpy(data, C_cld[nr].aeropars, 
			 C_cld[nr].numaeropars * sizeof(float));
	  data += C_cld[nr].numaeropars;
	  free(C_cld[nr].aeropars);

	  (void) memcpy(&cld[nr].mjd, &C_cld[nr].mjd, 
			 sizeof(struct mjd_envi));
	  cld[nr].quality = C_cld[nr].quality;
	  cld[nr].intg_time = C_cld[nr].intg_time;
	  cld[nr].numpmdpix = C_cld[nr].numpmdpix;
	  cld[nr].cloudtype = C_cld[nr].cloudtype;
	  cld[nr].cloudflag = C_cld[nr].cloudflag;
	  cld[nr].aaiflag = C_cld[nr].aaiflag;
	  cld[nr].numaeropars = C_cld[nr].numaeropars;
	  cld[nr].fullfree[0] = C_cld[nr].fullfree[0];
	  cld[nr].fullfree[1] = C_cld[nr].fullfree[1];
	  cld[nr].dsrlen = C_cld[nr].dsrlen;
	  cld[nr].surfpress = C_cld[nr].surfpress;
	  cld[nr].cloudfrac = C_cld[nr].cloudfrac;
	  cld[nr].errcldfrac = C_cld[nr].errcldfrac;
	  cld[nr].toppress = C_cld[nr].toppress;
	  cld[nr].errtoppress = C_cld[nr].errtoppress;
	  cld[nr].cldoptdepth = C_cld[nr].cldoptdepth;
	  cld[nr].errcldoptdep = C_cld[nr].errcldoptdep;
	  cld[nr].cloudbrdf = C_cld[nr].cloudbrdf;
	  cld[nr].errcldbrdf = C_cld[nr].errcldbrdf;
	  cld[nr].effsurfrefl = C_cld[nr].effsurfrefl;
	  cld[nr].erreffsrefl = C_cld[nr].erreffsrefl;
	  cld[nr].aai = C_cld[nr].aai;
	  cld[nr].aaidiag = C_cld[nr].aaidiag;
     }
     free(C_cld);

     return nr_cld;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_OL2_RD_LGEO (int argc, void *argv[])
{
     int          nr_geo;
     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct lgeo_scia *geo;

     if (argc != 3) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     if (fileno(fd_nadc) == -1) 
	  NADC_GOTO_ERROR(NADC_ERR_FILE, "No open stream");

     num_dsd = *(unsigned int *) argv[0];
     dsd = (struct dsd_envi *) argv[1];
     geo = (struct lgeo_scia *) argv[2];
     nr_geo = (int) SCIA_OL2_RD_LGEO(fd_nadc, num_dsd, dsd, &geo);
     if (IS_ERR_STAT_FATAL) return -1;

     return nr_geo;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_OL2_RD_NGEO (int argc, void *argv[])
{
     int          nr_geo;
     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct ngeo_scia *geo;

     if (argc != 3) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     if (fileno(fd_nadc) == -1) 
	  NADC_GOTO_ERROR(NADC_ERR_FILE, "No open stream");

     num_dsd = *(unsigned int *) argv[0];
     dsd = (struct dsd_envi *) argv[1];
     geo = (struct ngeo_scia *) argv[2];
     nr_geo = (int) SCIA_OL2_RD_NGEO(fd_nadc, num_dsd, dsd, &geo);
     if (IS_ERR_STAT_FATAL) return -1;

     return nr_geo;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_OL2_RD_LCLD (int argc, void *argv[])
{
     register int    nr;

     int          nr_lcld;
     unsigned int num_dsd;
     float        *tangent_hghts;
     float        *cir;
     float        *limb_para;

     struct dsd_envi  *dsd;
     struct lcld_scia *C_lcld;

     struct IDL_lcld_scia
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

	  IDL_ULONG pntr_tangent_hghts;        /* IDL uses 32-bit addresses */
	  IDL_ULONG pntr_cir;                  /* IDL uses 32-bit addresses */
	  IDL_ULONG pntr_limb_para;            /* IDL uses 32-bit addresses */
     } *lcld;

     if (argc != 6) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     if (fileno(fd_nadc) == -1) 
	  NADC_GOTO_ERROR(NADC_ERR_FILE, "No open stream");

     num_dsd = *(unsigned int *) argv[0];
     dsd = (struct dsd_envi *) argv[1];
     lcld = (struct IDL_lcld_scia *) argv[2];
     tangent_hghts = (float *) argv[3];
     cir = (float *) argv[4];
     limb_para = (float *) argv[5];

     Use_Extern_Alloc = FALSE;
     nr_lcld = (int) SCIA_OL2_RD_LCLD(fd_nadc, num_dsd, dsd, &C_lcld);
     Use_Extern_Alloc = TRUE;
     if (IS_ERR_STAT_FATAL) return -1;

     for (nr = 0; nr < nr_lcld; nr++) {
	  size_t adim = C_lcld[nr].num_tangent_hghts * C_lcld[nr].num_cir;

	  if (C_lcld[nr].num_tangent_hghts > 0) {
	       (void) memcpy(tangent_hghts, C_lcld[nr].tangent_hghts, 
			      C_lcld[nr].num_tangent_hghts * sizeof(float));
	       tangent_hghts += C_lcld[nr].num_tangent_hghts;
	       free(C_lcld[nr].tangent_hghts);
	  }
	  if (adim > 0) {
	       (void) memcpy(cir, C_lcld[nr].cir, adim * sizeof(float));
	       cir += adim;
	       free(C_lcld[nr].cir);
	  }
	  if (C_lcld[nr].num_limb_para > 0) {
	       (void) memcpy(limb_para, C_lcld[nr].limb_para, 
			      C_lcld[nr].num_limb_para * sizeof(float));
	       limb_para += C_lcld[nr].num_limb_para;
	       free(C_lcld[nr].limb_para);
	  }
	  (void) memcpy(&lcld[nr].mjd, &C_lcld[nr].mjd, 
			 sizeof(struct mjd_envi));
	  lcld[nr].dsrlen = C_lcld[nr].dsrlen;
	  lcld[nr].quality = C_lcld[nr].quality;
	  lcld[nr].diag_cloud_algo = C_lcld[nr].diag_cloud_algo;
	  lcld[nr].flag_normal_water = C_lcld[nr].flag_normal_water;
	  lcld[nr].flag_water_clouds = C_lcld[nr].flag_water_clouds;
	  lcld[nr].flag_ice_clouds = C_lcld[nr].flag_ice_clouds;
	  lcld[nr].hght_index_max_value_ice = 
	       C_lcld[nr].hght_index_max_value_ice;
	  lcld[nr].flag_polar_strato_clouds = 
	       C_lcld[nr].flag_polar_strato_clouds;
	  lcld[nr].hght_index_max_value_strato = 
	       C_lcld[nr].hght_index_max_value_strato;
	  lcld[nr].flag_noctilucent_clouds = 
	       C_lcld[nr].flag_noctilucent_clouds;
	  lcld[nr].hght_index_max_value_noctilucent = 
	       C_lcld[nr].hght_index_max_value_noctilucent;
	  lcld[nr].intg_time = C_lcld[nr].intg_time;
	  lcld[nr].num_tangent_hghts = C_lcld[nr].num_tangent_hghts;
	  lcld[nr].num_cir = C_lcld[nr].num_cir;
	  lcld[nr].num_limb_para = C_lcld[nr].num_limb_para;
	  lcld[nr].max_value_cir = C_lcld[nr].max_value_cir;
	  lcld[nr].hght_max_value_cir = C_lcld[nr].hght_max_value_cir;
	  lcld[nr].max_value_cir_ice = C_lcld[nr].max_value_cir_ice;
	  lcld[nr].hght_max_value_cir_ice = C_lcld[nr].hght_max_value_cir_ice;
	  lcld[nr].max_value_cir_strato = C_lcld[nr].max_value_cir_strato;
	  lcld[nr].hght_max_value_cir_strato = 
	       C_lcld[nr].hght_max_value_cir_strato;
	  lcld[nr].hght_max_value_noctilucent = 
	       C_lcld[nr].hght_max_value_noctilucent;
     }
     free(C_lcld);

     return nr_lcld;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_OL2_RD_LFIT (int argc, void *argv[])
{
     register int    nr;
     register size_t nr_rec;

     int          nr_lfit;
     unsigned int num_dsd;
     float        *tangh, *tangp, *tangt;
     float        *corrmatrix;
     float        *residuals;
     float        *adddiag;
     IDL_STRING   *lfit_name;

     struct dsd_envi  *dsd;
     struct layer_rec *mainrec, *scaledrec;
     struct meas_grid *mgrid;
     struct state_vec *statevec;
     struct lfit_scia *C_lfit;

     struct IDL_lfit_scia
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

	  IDL_ULONG pntr_tangh;                /* IDL uses 32-bit addresses */
	  IDL_ULONG pntr_tangp;                /* IDL uses 32-bit addresses */
	  IDL_ULONG pntr_tangt;                /* IDL uses 32-bit addresses */
	  IDL_ULONG pntr_corrmatrix;           /* IDL uses 32-bit addresses */
	  IDL_ULONG pntr_residuals;            /* IDL uses 32-bit addresses */
	  IDL_ULONG pntr_adddiag;              /* IDL uses 32-bit addresses */

	  IDL_ULONG pntr_mainrec;              /* IDL uses 32-bit addresses */
	  IDL_ULONG pntr_scaledrec;            /* IDL uses 32-bit addresses */
	  IDL_ULONG pntr_mgrid;                /* IDL uses 32-bit addresses */
	  IDL_ULONG pntr_statevec;             /* IDL uses 32-bit addresses */
     } *lfit;

     if (argc != 14) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     if (fileno(fd_nadc) == -1) 
	  NADC_GOTO_ERROR(NADC_ERR_FILE, "No open stream");

     lfit_name = (IDL_STRING *) argv[0];
     num_dsd = *(unsigned int *) argv[1];
     dsd = (struct dsd_envi *) argv[2];
     lfit = (struct IDL_lfit_scia *) argv[3];
     tangh = (float *) argv[4];
     tangp = (float *) argv[5];
     tangt = (float *) argv[6];
     corrmatrix = (float *) argv[7];
     residuals = (float *) argv[8];
     adddiag = (float *) argv[9];
     mainrec = (struct layer_rec *) argv[10];
     scaledrec = (struct layer_rec *) argv[11];
     mgrid = (struct meas_grid *) argv[12];
     statevec = (struct state_vec *) argv[13];

     Use_Extern_Alloc = FALSE;
     nr_lfit = (int) SCIA_OL2_RD_LFIT(fd_nadc, lfit_name[0].s,
				       num_dsd, dsd, &C_lfit);
     Use_Extern_Alloc = TRUE;
     if (IS_ERR_STAT_FATAL) return -1;

     for (nr = 0; nr < nr_lfit; nr++) {
	  (void) memcpy(tangh, C_lfit[nr].tangh, 
                         C_lfit[nr].num_rlevel * sizeof(float));
          tangh += C_lfit[nr].num_rlevel;
          free(C_lfit[nr].tangh);
	  (void) memcpy(tangp, C_lfit[nr].tangp, 
                         C_lfit[nr].num_rlevel * sizeof(float));
          tangp += C_lfit[nr].num_rlevel;
          free(C_lfit[nr].tangp);
	  (void) memcpy(tangt, C_lfit[nr].tangt, 
                         C_lfit[nr].num_rlevel * sizeof(float));
          tangt += C_lfit[nr].num_rlevel;
          free(C_lfit[nr].tangt);
	  if (C_lfit[nr].cmatrixsize > 0) {
	       (void) memcpy(corrmatrix, C_lfit[nr].corrmatrix, 
			      C_lfit[nr].cmatrixsize * sizeof(float));
	       corrmatrix += C_lfit[nr].cmatrixsize;
	       free(C_lfit[nr].corrmatrix);
	  }
	  if (C_lfit[nr].ressize > 0) {
	       (void) memcpy(residuals, C_lfit[nr].residuals, 
			      C_lfit[nr].ressize * sizeof(float));
	       residuals += C_lfit[nr].ressize;
	       free(C_lfit[nr].residuals);
	  }
	  if (C_lfit[nr].num_adddiag > 0) {
	       (void) memcpy(adddiag, C_lfit[nr].adddiag, 
			      C_lfit[nr].num_adddiag * sizeof(float));
	       adddiag += C_lfit[nr].num_adddiag;
	       free(C_lfit[nr].adddiag);
	  }

	  nr_rec = C_lfit[nr].num_rlevel * C_lfit[nr].num_species;
	  if (nr_rec > 0) { 
	       (void) memcpy(mainrec, C_lfit[nr].mainrec,
			      nr_rec * sizeof(struct layer_rec));
	       mainrec += nr_rec;
	       free(C_lfit[nr].mainrec);
	  }
	  nr_rec = C_lfit[nr].num_rlevel * C_lfit[nr].num_scale;
	  if (nr_rec > 0) { 
	       (void) memcpy(scaledrec, C_lfit[nr].scaledrec,
			      nr_rec * sizeof(struct layer_rec));
	       scaledrec += nr_rec;
	       free(C_lfit[nr].scaledrec);
	  }
	  (void) memcpy(mgrid, C_lfit[nr].mgrid,
			 C_lfit[nr].num_mlevel * sizeof(struct meas_grid));
	  mgrid += C_lfit[nr].num_mlevel;
          free(C_lfit[nr].mgrid);

	  (void) memcpy(statevec, C_lfit[nr].statevec,
			 C_lfit[nr].stvec_size * sizeof(struct state_vec));
	  statevec += C_lfit[nr].stvec_size;
          free(C_lfit[nr].statevec);

	  (void) memcpy(&lfit[nr].mjd, &C_lfit[nr].mjd, 
			 sizeof(struct mjd_envi));
	  lfit[nr].quality = C_lfit[nr].quality;
	  lfit[nr].criteria = C_lfit[nr].criteria;
	  lfit[nr].method = C_lfit[nr].method;
	  lfit[nr].refpsrc = C_lfit[nr].refpsrc;
	  lfit[nr].num_rlevel = C_lfit[nr].num_rlevel;
	  lfit[nr].num_mlevel = C_lfit[nr].num_mlevel;
	  lfit[nr].num_species = C_lfit[nr].num_species;
	  lfit[nr].num_closure = C_lfit[nr].num_closure;
	  lfit[nr].num_other = C_lfit[nr].num_other;
	  lfit[nr].num_scale = C_lfit[nr].num_scale;
	  lfit[nr].intg_time = C_lfit[nr].intg_time;
	  lfit[nr].stvec_size = C_lfit[nr].stvec_size;
	  lfit[nr].cmatrixsize = C_lfit[nr].cmatrixsize;
	  lfit[nr].numiter = C_lfit[nr].numiter;
	  lfit[nr].ressize = C_lfit[nr].ressize;
	  lfit[nr].num_adddiag = C_lfit[nr].num_adddiag;
	  lfit[nr].summary[0] = C_lfit[nr].summary[0];
	  lfit[nr].summary[1] = C_lfit[nr].summary[1];
	  lfit[nr].dsrlen = C_lfit[nr].dsrlen;
	  lfit[nr].refh = C_lfit[nr].refh;
	  lfit[nr].refp = C_lfit[nr].refp;
	  lfit[nr].rms = C_lfit[nr].rms;
	  lfit[nr].chi2 = C_lfit[nr].chi2;
	  lfit[nr].goodness = C_lfit[nr].goodness;
     }
     free(C_lfit);

     return nr_lfit;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_OL2_RD_NFIT (int argc, void *argv[])
{
     register int    nr;
     register size_t n_cross;

     int          nr_nfit;
     unsigned int num_dsd;
     float        *vcd, *errvcd;
     float        *linpars, *errlinpars, *lincorrm;
     float        *nlinpars, *errnlinpars, *nlincorrm;
     IDL_STRING   *nfit_name;

     struct dsd_envi  *dsd;
     struct nfit_scia *C_nfit;

     struct IDL_nfit_scia
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

	  IDL_ULONG  pntr_vcd;                 /* IDL uses 32-bit addresses */
	  IDL_ULONG  pntr_errvcd;              /* IDL uses 32-bit addresses */
	  IDL_ULONG  pntr_linpars;             /* IDL uses 32-bit addresses */
	  IDL_ULONG  pntr_errlinpars;          /* IDL uses 32-bit addresses */
	  IDL_ULONG  pntr_lincorrm;            /* IDL uses 32-bit addresses */
	  IDL_ULONG  pntr_nlinpars;            /* IDL uses 32-bit addresses */
	  IDL_ULONG  pntr_errnlinpars;         /* IDL uses 32-bit addresses */
	  IDL_ULONG  pntr_nlincorrm;           /* IDL uses 32-bit addresses */
     } *nfit;

     if (argc != 12) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     if (fileno(fd_nadc) == -1) 
	  NADC_GOTO_ERROR(NADC_ERR_FILE, "No open stream");

     nfit_name = (IDL_STRING *) argv[0];
     num_dsd = *(unsigned int *) argv[1];
     dsd = (struct dsd_envi *) argv[2];
     nfit = (struct IDL_nfit_scia *) argv[3];
     vcd = (float *) argv[4];
     errvcd = (float *) argv[5];
     linpars = (float *) argv[6];
     errlinpars = (float *) argv[7];
     lincorrm = (float *) argv[8];
     nlinpars = (float *) argv[9];
     errnlinpars = (float *) argv[10];
     nlincorrm = (float *) argv[11];

     Use_Extern_Alloc = FALSE;
     nr_nfit = (int) SCIA_OL2_RD_NFIT(fd_nadc, nfit_name[0].s, 
				       num_dsd, dsd, &C_nfit);
     Use_Extern_Alloc = TRUE;
     if (IS_ERR_STAT_FATAL) return -1; 
     for (nr = 0; nr < nr_nfit; nr++) {
	  (void) memcpy(vcd, C_nfit[nr].vcd, 
                         C_nfit[nr].numvcd * sizeof(float));
          vcd += C_nfit[nr].numvcd;
          free(C_nfit[nr].vcd);
	  (void) memcpy(errvcd, C_nfit[nr].errvcd, 
                         C_nfit[nr].numvcd * sizeof(float));
          errvcd += C_nfit[nr].numvcd;
          free(C_nfit[nr].errvcd);
	  if (C_nfit[nr].num_fitp > 0) {
	       (void) memcpy(linpars, C_nfit[nr].linpars, 
			      C_nfit[nr].num_fitp * sizeof(float));
	       linpars += C_nfit[nr].num_fitp;
	       free(C_nfit[nr].linpars);

	       (void) memcpy(errlinpars, C_nfit[nr].errlinpars, 
			      C_nfit[nr].num_fitp * sizeof(float));
	       errlinpars += C_nfit[nr].num_fitp;
	       free(C_nfit[nr].errlinpars);
	  }
	  n_cross = (C_nfit[nr].num_fitp * (C_nfit[nr].num_fitp-1)) / 2;
	  if (n_cross > 0) {
	       (void) memcpy(lincorrm, C_nfit[nr].lincorrm, 
			      n_cross * sizeof(float));
	       lincorrm += n_cross;
	       free(C_nfit[nr].lincorrm);
	  }
	  (void) memcpy(nlinpars, C_nfit[nr].nlinpars, 
                         C_nfit[nr].num_nfitp * sizeof(float));
          nlinpars += C_nfit[nr].num_nfitp;
          free(C_nfit[nr].nlinpars);
	  (void) memcpy(errnlinpars, C_nfit[nr].errnlinpars, 
                         C_nfit[nr].num_nfitp * sizeof(float));
          errnlinpars += C_nfit[nr].num_nfitp;
          free(C_nfit[nr].errnlinpars);
	  n_cross = (C_nfit[nr].num_nfitp * (C_nfit[nr].num_nfitp-1)) / 2;
	  if (n_cross > 0) {
	       (void) memcpy(nlincorrm, C_nfit[nr].nlincorrm, 
			      n_cross * sizeof(float));
	       nlincorrm += n_cross;
	       free(C_nfit[nr].nlincorrm);
	  }
	  (void) memcpy(&nfit[nr].mjd, &C_nfit[nr].mjd, 
			 sizeof(struct mjd_envi));
	  nfit[nr].quality = C_nfit[nr].quality;
	  nfit[nr].intg_time = C_nfit[nr].intg_time;
	  nfit[nr].numvcd = C_nfit[nr].numvcd;
	  nfit[nr].vcdflag = C_nfit[nr].vcdflag;
	  nfit[nr].num_fitp = C_nfit[nr].num_fitp;
	  nfit[nr].num_nfitp = C_nfit[nr].num_nfitp;
	  nfit[nr].numiter = C_nfit[nr].numiter;
	  nfit[nr].fitflag = C_nfit[nr].fitflag;
	  nfit[nr].amfflag = C_nfit[nr].amfflag;
	  nfit[nr].dsrlen = C_nfit[nr].dsrlen;
	  nfit[nr].esc = C_nfit[nr].esc;
	  nfit[nr].erresc = C_nfit[nr].erresc;
	  nfit[nr].rms = C_nfit[nr].rms;
	  nfit[nr].chi2 = C_nfit[nr].chi2;
	  nfit[nr].goodness = C_nfit[nr].goodness;
	  nfit[nr].amfgrd = C_nfit[nr].amfgrd;
	  nfit[nr].erramfgrd = C_nfit[nr].erramfgrd;
	  nfit[nr].amfcld = C_nfit[nr].amfcld;
	  nfit[nr].erramfcld = C_nfit[nr].erramfcld;
	  nfit[nr].temperature = C_nfit[nr].temperature;
     }
     free(C_nfit);

     return nr_nfit;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_OL2_RD_SPH (int argc, void *argv[])
{
     struct mph_envi  mph;
     struct sph_sci_ol *sph;

     if (argc != 2) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     if (fileno(fd_nadc) == -1) 
	  NADC_GOTO_ERROR(NADC_ERR_FILE, "No open stream");

     mph = *(struct mph_envi *) argv[0];
     sph = (struct sph_sci_ol *) argv[1];

     SCIA_OL2_RD_SPH(fd_nadc, mph, sph);
     if (IS_ERR_STAT_FATAL) return -1;

     return 1;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_OL2_RD_SQADS (int argc, void *argv[])
{
     int          nr_sqads;
     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct sqads_sci_ol *sqads;

     if (argc != 3) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     if (fileno(fd_nadc) == -1) 
	  NADC_GOTO_ERROR(NADC_ERR_FILE, "No open stream");

     num_dsd = *(unsigned int *) argv[0];
     dsd   = (struct dsd_envi *) argv[1];
     sqads = (struct sqads_sci_ol *) argv[2];
     nr_sqads = (int) SCIA_OL2_RD_SQADS(fd_nadc, num_dsd, dsd, &sqads);
     if (IS_ERR_STAT_FATAL) return -1;

     return nr_sqads;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_OL2_RD_STATE (int argc, void *argv[])
{
     int          nr_state;
     unsigned int num_dsd;

     struct dsd_envi *dsd;
     struct state2_scia *state;

     if (argc != 3) NADC_GOTO_ERROR(NADC_ERR_PARAM, err_msg);
     if (fileno(fd_nadc) == -1) 
	  NADC_GOTO_ERROR(NADC_ERR_FILE, "No open stream");

     num_dsd = *(unsigned int *) argv[0];
     dsd = (struct dsd_envi *) argv[1];
     state = (struct state2_scia *) argv[2];
     nr_state = (int) SCIA_OL2_RD_STATE(fd_nadc, num_dsd, dsd, &state);
     if (IS_ERR_STAT_FATAL) return -1;

     return nr_state;
 done:
     return -1;
}
