/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2002 - 2019 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_OL2_WR_H5_CLD
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 2 Cloud/Aerosol Data Set(s)
.INPUT/OUTPUT
  call as    SCIA_OL2_WR_H5_CLD(nr_cld, cld);
     input:  
	     unsigned int nr_cld        : number of Cloud/Aerosol data sets
	     struct cld_sci_ol *cld     : Cloud/Aerosol Data Set(s)

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   27-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              1.0   14-May-2002	created by R. M. van Hees 
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

#define NFIELDS    23

static const size_t cld_size = sizeof(struct cld_sci_ol);
static const size_t cld_offs[NFIELDS] = {
     HOFFSET(struct cld_sci_ol, mjd),
     HOFFSET(struct cld_sci_ol, quality),
     HOFFSET(struct cld_sci_ol, intg_time),
     HOFFSET(struct cld_sci_ol, numpmdpix),
     HOFFSET(struct cld_sci_ol, cloudtype),
     HOFFSET(struct cld_sci_ol, cloudflag),
     HOFFSET(struct cld_sci_ol, aaiflag),
     HOFFSET(struct cld_sci_ol, numaeropars),
     HOFFSET(struct cld_sci_ol, fullfree),
     HOFFSET(struct cld_sci_ol, dsrlen),
     HOFFSET(struct cld_sci_ol, surfpress),
     HOFFSET(struct cld_sci_ol, cloudfrac),
     HOFFSET(struct cld_sci_ol, errcldfrac),
     HOFFSET(struct cld_sci_ol, toppress),
     HOFFSET(struct cld_sci_ol, errtoppress),
     HOFFSET(struct cld_sci_ol, cldoptdepth),
     HOFFSET(struct cld_sci_ol, errcldoptdep),
     HOFFSET(struct cld_sci_ol, cloudbrdf),
     HOFFSET(struct cld_sci_ol, errcldbrdf),
     HOFFSET(struct cld_sci_ol, effsurfrefl),
     HOFFSET(struct cld_sci_ol, erreffsrefl),
     HOFFSET(struct cld_sci_ol, aai),
     HOFFSET(struct cld_sci_ol, aaidiag)
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_OL2_WR_H5_CLD(unsigned int nr_cld, const struct cld_sci_ol *cld)
{
     register unsigned int  nr;

     hid_t   fid, grp_id;
     hbool_t compress;
     hsize_t adim;
     hvl_t   *vdata;
     hid_t   cld_type[NFIELDS];

     const char *cld_names[NFIELDS] = {
          "dsr_time", "quality_flag", "integr_time", "pmd_read", 
	  "cl_type_flags", "cloud_flags", "flag_output_flags", 
	  "num_aero_param", "pmd_read_cl", "dsr_length",
	  "surface_pres", "cl_frac", "cl_frac_err", "cl_top_pres", 
	  "cl_top_pres_err", "cl_opt_depth", "cl_opt_depth_err", 
	  "cl_reflectance", "cl_reflectance_err", 
	  "surface_reflectance", "surface_reflectance_err", 
	  "aero_abso_ind", "aero_ind_diag"
     };
/*
 * check number of CLD records
 */
     if (nr_cld == 0) return;
/*
 * set HDF5 boolean variable for compression
 */
     if (nadc_get_param_uint8("flag_deflate") == PARAM_SET)
          compress = TRUE;
     else
          compress = FALSE;
/*
 * create group /MDS
 */
     fid = nadc_get_param_hid("hdf_file_id");
     grp_id = NADC_OPEN_HDF5_Group(fid, "/MDS");
     if (grp_id < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, "/MDS");
/*
 * define user-defined data types of the Table-fields
 */
     cld_type[0] = H5Topen(fid, "mjd", H5P_DEFAULT);
     cld_type[1] = H5Tcopy(H5T_NATIVE_CHAR);
     cld_type[2] = H5Tcopy(H5T_NATIVE_USHORT);
     cld_type[3] = H5Tcopy(H5T_NATIVE_USHORT);
     cld_type[4] = H5Tcopy(H5T_NATIVE_USHORT);
     cld_type[5] = H5Tcopy(H5T_NATIVE_USHORT);
     cld_type[6] = H5Tcopy(H5T_NATIVE_USHORT);
     cld_type[7] = H5Tcopy(H5T_NATIVE_USHORT);
     adim = 2;
     cld_type[8] = H5Tarray_create(H5T_NATIVE_USHORT, 1, &adim);
     cld_type[9] = H5Tcopy(H5T_NATIVE_UINT);
     cld_type[10] = H5Tcopy(H5T_NATIVE_FLOAT);
     cld_type[11] = H5Tcopy(H5T_NATIVE_FLOAT);
     cld_type[12] = H5Tcopy(H5T_NATIVE_FLOAT);
     cld_type[13] = H5Tcopy(H5T_NATIVE_FLOAT);
     cld_type[14] = H5Tcopy(H5T_NATIVE_FLOAT);
     cld_type[15] = H5Tcopy(H5T_NATIVE_FLOAT);
     cld_type[16] = H5Tcopy(H5T_NATIVE_FLOAT);
     cld_type[17] = H5Tcopy(H5T_NATIVE_FLOAT);
     cld_type[18] = H5Tcopy(H5T_NATIVE_FLOAT);
     cld_type[19] = H5Tcopy(H5T_NATIVE_FLOAT);
     cld_type[20] = H5Tcopy(H5T_NATIVE_FLOAT);
     cld_type[21] = H5Tcopy(H5T_NATIVE_FLOAT);
     cld_type[22] = H5Tcopy(H5T_NATIVE_FLOAT);
/*
 * create table
 */
     (void) H5TBmake_table("Cloud end Aerosol MDS", grp_id, "cld", NFIELDS, 
			    nr_cld, cld_size, cld_names, cld_offs, 
			    cld_type, nr_cld, NULL, compress, cld);
/*
 * close interface
 */
     for (nr = 0; nr < NFIELDS; nr++) (void) H5Tclose(cld_type[nr]);
/*
 * +++++ create/write variable part of the CLOUDS_AEROSOL record
 */
     adim = (hsize_t) nr_cld;
/*
 * Additional aerosol parameters
 */
     vdata = (hvl_t *) malloc(nr_cld * sizeof(hvl_t));
     if (vdata == NULL) NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata");
     nr = 0;
     do {
	  vdata[nr].len = (size_t) cld[nr].numaeropars;
	  if (cld[nr].numaeropars > 0) {
	       vdata[nr].p = malloc(vdata[nr].len * sizeof(float));
	       if (vdata[nr].p == NULL) {
		    free(vdata);
		    NADC_RETURN_ERROR(NADC_ERR_ALLOC, "vdata.p");
	       }
	       (void) memcpy(vdata[nr].p , cld[nr].aeropars,
			      vdata[nr].len * sizeof(float));
	  }
     } while (++nr < nr_cld);
     NADC_WR_HDF5_Vlen_Dataset(compress, grp_id, "aeropars",
				H5T_NATIVE_FLOAT, 1, &adim, vdata);
/*
 * close interface
 */
     (void) H5Gclose(grp_id);
}
