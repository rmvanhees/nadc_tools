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

.IDENTifer   SCIA_OL2_WR_H5_LFIT
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 Off-line - HDF5
.LANGUAGE    ANSI C
.PURPOSE     write SCIAMACHY level 2 Limb/Occultation 
              Fitting Window Application Data Set(s)
.INPUT/OUTPUT
  call as    SCIA_OL2_WR_H5_LFIT(lfit_name, nr_lfit, lfit);
     input:  
             char lfit_name[]          : name of fitted species
	     unsigned int nr_lfit      : number of lfit MDS
	     struct lfit_scia *lfit    : Limb/Occultation Fitting Window Data

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   27-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              1.0   22-Jan-2003	created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

#define NFIELDS    22

static const size_t lfit_size = sizeof(struct lfit_scia);
static const size_t lfit_offs[NFIELDS] = {
     HOFFSET(struct lfit_scia, mjd),
     HOFFSET(struct lfit_scia, quality),
     HOFFSET(struct lfit_scia, criteria),
     HOFFSET(struct lfit_scia, method),
     HOFFSET(struct lfit_scia, refpsrc),
     HOFFSET(struct lfit_scia, num_rlevel),
     HOFFSET(struct lfit_scia, num_mlevel),
     HOFFSET(struct lfit_scia, num_species),
     HOFFSET(struct lfit_scia, num_closure),
     HOFFSET(struct lfit_scia, num_other),
     HOFFSET(struct lfit_scia, num_scale),
     HOFFSET(struct lfit_scia, intg_time),
     HOFFSET(struct lfit_scia, stvec_size),
     HOFFSET(struct lfit_scia, cmatrixsize),
     HOFFSET(struct lfit_scia, numiter),
     HOFFSET(struct lfit_scia, ressize),
     HOFFSET(struct lfit_scia, num_adddiag),
     HOFFSET(struct lfit_scia, summary),
     HOFFSET(struct lfit_scia, dsrlen),
     HOFFSET(struct lfit_scia, refh),
     HOFFSET(struct lfit_scia, refp),
     HOFFSET(struct lfit_scia, rms)
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_OL2_WR_H5_LFIT(const char lfit_name[],
			 unsigned int nr_lfit, const struct lfit_scia *lfit)
{
     register unsigned int nr;

     hid_t   fid, grp_id;
     hid_t   lfit_id;
     hid_t   type_id;
     hbool_t compress;
     hsize_t adim;
     hvl_t   *vdata;
     hid_t   lfit_type[NFIELDS];

     const char *lfit_names[NFIELDS] = {
          "dsr_time", "quality_flag", "criteria", "method", "refpsrc", 
	  "num_rlevel", "num_mlevel", "num_species", "num_closure", 
	  "num_other", "num_scale", "integr_time", "stvec_size", "cmatrixsize",
	  "numiter", "ressize", "num_adddiag", "summary", "dsrlen", "refh", 
	  "refp", "rms_chi_2_gof"
     };
/*
 * check number of LFIT records
 */
     if (nr_lfit == 0) return;
/*
 * set HDF5 boolean variable for compression
 */
     if (nadc_get_param_uint8("flag_deflate") == PARAM_SET)
          compress = TRUE;
     else
          compress = FALSE;
/*
 * create group /MDS/<lfit_name>
 */
     fid = nadc_get_param_hid("hdf_file_id");
     grp_id = NADC_OPEN_HDF5_Group(fid, "/MDS");
     if (grp_id < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, "/MDS");
     lfit_id = H5Gcreate(grp_id, lfit_name,
			  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
     if (lfit_id < 0) 
	  NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, lfit_name);
/*
 * define user-defined data types of the Table-fields
 */
     lfit_type[0] = H5Topen(fid, "mjd", H5P_DEFAULT);
     lfit_type[1] = H5Tcopy(H5T_NATIVE_CHAR);
     lfit_type[2] = H5Tcopy(H5T_NATIVE_CHAR);
     lfit_type[3] = H5Tcopy(H5T_NATIVE_UCHAR);
     lfit_type[4] = H5Tcopy(H5T_NATIVE_UCHAR);
     lfit_type[5] = H5Tcopy(H5T_NATIVE_UCHAR);
     lfit_type[6] = H5Tcopy(H5T_NATIVE_UCHAR);
     lfit_type[7] = H5Tcopy(H5T_NATIVE_UCHAR);
     lfit_type[8] = H5Tcopy(H5T_NATIVE_UCHAR);
     lfit_type[9] = H5Tcopy(H5T_NATIVE_UCHAR);
     lfit_type[10] = H5Tcopy(H5T_NATIVE_UCHAR);
     lfit_type[11] = H5Tcopy(H5T_NATIVE_USHORT);
     lfit_type[12] = H5Tcopy(H5T_NATIVE_USHORT);
     lfit_type[13] = H5Tcopy(H5T_NATIVE_USHORT);
     lfit_type[14] = H5Tcopy(H5T_NATIVE_USHORT);
     lfit_type[15] = H5Tcopy(H5T_NATIVE_USHORT);
     lfit_type[16] = H5Tcopy(H5T_NATIVE_USHORT);
     adim = 3;
     lfit_type[17] = H5Tarray_create(H5T_NATIVE_USHORT, 1, &adim);
     lfit_type[18] = H5Tcopy(H5T_NATIVE_UINT);
     lfit_type[19] = H5Tcopy(H5T_NATIVE_FLOAT);
     lfit_type[20] = H5Tcopy(H5T_NATIVE_FLOAT);
     adim = 3;
     lfit_type[21] = H5Tarray_create(H5T_NATIVE_FLOAT, 1, &adim);
/*
 * create table
 */
     (void) H5TBmake_table(lfit_name, lfit_id, "lfit", NFIELDS, 
			    nr_lfit, lfit_size, lfit_names, lfit_offs, 
			    lfit_type, nr_lfit, NULL, compress, lfit);
/*
 * close interface
 */
     for (nr = 0; nr < NFIELDS; nr++) (void) H5Tclose(lfit_type[nr]);
/*
 * +++++ create/write attributes in the /MDS/LFIT group
 */
     adim = (hsize_t) nr_lfit;
/*
 * variable length arrays: tangh, tangp, tangt, corrmatrix, residuals, adddiag
 */
     vdata = (hvl_t *) malloc(nr_lfit * sizeof(hvl_t));
     if (vdata == NULL) NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata");
     nr = 0;
     do {
          vdata[nr].len = (size_t) lfit[nr].num_rlevel;
          if (lfit[nr].num_rlevel > UCHAR_ZERO) {
               vdata[nr].p = malloc(vdata[nr].len * sizeof(float));
               if (vdata[nr].p == NULL) {
                    free(vdata);
                    NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata.p");
               }
               (void) memcpy(vdata[nr].p, lfit[nr].tangh,
                              vdata[nr].len * sizeof(float));
          }
     } while (++nr < nr_lfit);
     NADC_WR_HDF5_Vlen_Dataset(compress, lfit_id, "tangh",
                               H5T_NATIVE_FLOAT, 1, &adim, vdata);
     vdata = (hvl_t *) malloc(nr_lfit * sizeof(hvl_t));
     if (vdata == NULL) NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata");
     nr = 0;
     do {
          vdata[nr].len = (size_t) lfit[nr].num_rlevel;
          if (lfit[nr].num_rlevel > UCHAR_ZERO) {
               vdata[nr].p = malloc(vdata[nr].len * sizeof(float));
               if (vdata[nr].p == NULL) {
                    free(vdata);
                    NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata.p");
               }
               (void) memcpy(vdata[nr].p, lfit[nr].tangp,
                              vdata[nr].len * sizeof(float));
          }
     } while (++nr < nr_lfit);
     NADC_WR_HDF5_Vlen_Dataset(compress, lfit_id, "tangp",
                               H5T_NATIVE_FLOAT, 1, &adim, vdata);
     vdata = (hvl_t *) malloc(nr_lfit * sizeof(hvl_t));
     if (vdata == NULL) NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata");
     nr = 0;
     do {
          vdata[nr].len = (size_t) lfit[nr].num_rlevel;
          if (lfit[nr].num_rlevel > UCHAR_ZERO) {
               vdata[nr].p = malloc(vdata[nr].len * sizeof(float));
               if (vdata[nr].p == NULL) {
                    free(vdata);
                    NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata.p");
               }
               (void) memcpy(vdata[nr].p, lfit[nr].tangt,
                              vdata[nr].len * sizeof(float));
          }
     } while (++nr < nr_lfit);
     NADC_WR_HDF5_Vlen_Dataset(compress, lfit_id, "tangt",
                               H5T_NATIVE_FLOAT, 1, &adim, vdata);
     vdata = (hvl_t *) malloc(nr_lfit * sizeof(hvl_t));
     if (vdata == NULL) NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata");
     nr = 0;
     do {
          vdata[nr].len = (size_t) lfit[nr].cmatrixsize;
          if (lfit[nr].cmatrixsize > UCHAR_ZERO) {
               vdata[nr].p = malloc(vdata[nr].len * sizeof(float));
               if (vdata[nr].p == NULL) {
                    free(vdata);
                    NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata.p");
               }
               (void) memcpy(vdata[nr].p, lfit[nr].corrmatrix,
                              vdata[nr].len * sizeof(float));
          }
     } while (++nr < nr_lfit);
     NADC_WR_HDF5_Vlen_Dataset(compress, lfit_id, "corrmatrix",
                               H5T_NATIVE_FLOAT, 1, &adim, vdata);
     vdata = (hvl_t *) malloc(nr_lfit * sizeof(hvl_t));
     if (vdata == NULL) NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata");
     nr = 0;
     do {
          vdata[nr].len = (size_t) lfit[nr].ressize;
          if (lfit[nr].ressize > UCHAR_ZERO) {
               vdata[nr].p = malloc(vdata[nr].len * sizeof(float));
               if (vdata[nr].p == NULL) {
                    free(vdata);
                    NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata.p");
               }
               (void) memcpy(vdata[nr].p, lfit[nr].residuals,
                              vdata[nr].len * sizeof(float));
          }
     } while (++nr < nr_lfit);
     NADC_WR_HDF5_Vlen_Dataset(compress, lfit_id, "residuals",
                               H5T_NATIVE_FLOAT, 1, &adim, vdata);
     vdata = (hvl_t *) malloc(nr_lfit * sizeof(hvl_t));
     if (vdata == NULL) NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata");
     nr = 0;
     do {
          vdata[nr].len = (size_t) lfit[nr].num_adddiag;
          if (lfit[nr].num_adddiag > UCHAR_ZERO) {
               vdata[nr].p = malloc(vdata[nr].len * sizeof(float));
               if (vdata[nr].p == NULL) {
                    free(vdata);
                    NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata.p");
               }
               (void) memcpy(vdata[nr].p, lfit[nr].adddiag,
                              vdata[nr].len * sizeof(float));
          }
     } while (++nr < nr_lfit);
     NADC_WR_HDF5_Vlen_Dataset(compress, lfit_id, "adddiag",
                               H5T_NATIVE_FLOAT, 1, &adim, vdata);
/*
 * variable length struct-arrays: mainrec, scaledrec, mgrid, statevec
 */
     type_id = H5Topen(fid, "layer_rec", H5P_DEFAULT);
     vdata = (hvl_t *) malloc(nr_lfit * sizeof(hvl_t));
     if (vdata == NULL) NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata");
     nr = 0;
     do {
          vdata[nr].len = (size_t)(lfit[nr].num_rlevel * lfit[nr].num_species);
          if (vdata[nr].len > (size_t) 0) {
               vdata[nr].p = malloc(vdata[nr].len * sizeof(struct layer_rec));
               if (vdata[nr].p == NULL) {
                    free(vdata);
                    NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata.p");
               }
               (void) memcpy(vdata[nr].p, lfit[nr].mainrec,
                              vdata[nr].len * sizeof(struct layer_rec));
          }
     } while (++nr < nr_lfit);
     NADC_WR_HDF5_Vlen_Dataset(compress, lfit_id, "mainrec",
                               type_id, 1, &adim, vdata);
     vdata = (hvl_t *) malloc(nr_lfit * sizeof(hvl_t));
     if (vdata == NULL) NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata");
     nr = 0;
     do {
          vdata[nr].len = (size_t)(lfit[nr].num_rlevel * lfit[nr].num_scale);
          if (vdata[nr].len > (size_t) 0) {
               vdata[nr].p = malloc(vdata[nr].len * sizeof(struct layer_rec));
               if (vdata[nr].p == NULL) {
                    free(vdata);
                    NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata.p");
               }
               (void) memcpy(vdata[nr].p, lfit[nr].scaledrec,
                              vdata[nr].len * sizeof(struct layer_rec));
          }
     } while (++nr < nr_lfit);
     NADC_WR_HDF5_Vlen_Dataset(compress, lfit_id, "scaledrec",
                               type_id, 1, &adim, vdata);
     (void) H5Tclose(type_id);

     type_id = H5Topen(fid, "meas_grid", H5P_DEFAULT);
     vdata = (hvl_t *) malloc(nr_lfit * sizeof(hvl_t));
     if (vdata == NULL) NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata");
     nr = 0;
     do {
          vdata[nr].len = (size_t) lfit[nr].num_mlevel;
          if (vdata[nr].len > (size_t) 0) {
               vdata[nr].p = malloc(vdata[nr].len * sizeof(struct meas_grid));
               if (vdata[nr].p == NULL) {
                    free(vdata);
                    NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata.p");
               }
               (void) memcpy(vdata[nr].p, lfit[nr].mgrid,
                              vdata[nr].len * sizeof(struct meas_grid));
          }
     } while (++nr < nr_lfit);
     NADC_WR_HDF5_Vlen_Dataset(compress, lfit_id, "mgrid",
                               type_id, 1, &adim, vdata);
     (void) H5Tclose(type_id);

     type_id = H5Topen(fid, "state_vec", H5P_DEFAULT);
     vdata = (hvl_t *) malloc(nr_lfit * sizeof(hvl_t));
     if (vdata == NULL) NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata");
     nr = 0;
     do {
          vdata[nr].len = (size_t) lfit[nr].stvec_size;
          if (vdata[nr].len > (size_t) 0) {
               vdata[nr].p = malloc(vdata[nr].len * sizeof(struct state_vec));
               if (vdata[nr].p == NULL) {
                    free(vdata);
                    NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata.p");
               }
               (void) memcpy(vdata[nr].p, lfit[nr].statevec,
                              vdata[nr].len * sizeof(struct state_vec));
          }
     } while (++nr < nr_lfit);
     NADC_WR_HDF5_Vlen_Dataset(compress, lfit_id, "statevec",
                               type_id, 1, &adim, vdata);
     (void) H5Tclose(type_id);
/*
 * close interface
 */
     (void) H5Gclose(lfit_id);
     (void) H5Gclose(grp_id);
}
