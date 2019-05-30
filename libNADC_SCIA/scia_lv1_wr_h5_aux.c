/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2000 - 2019 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_WR_H5_AUX
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 Auxiliary data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_AUX(nr_aux, aux);
     input:  
	     unsigned int nr_aux       : number of Auxiliary data packets
	     struct aux_scia *aux      : structure with Auxiliary data packets

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   22-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.1   21-Feb-2002	assign names to MDS0 data, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.0   24-Nov-1999	created by R. M. van Hees 
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
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

#define NFIELDS    6

static const size_t aux_size = sizeof(struct mds1_aux);
static const size_t aux_offs[NFIELDS] = {
     HOFFSET(struct mds1_aux, mjd),
     HOFFSET(struct mds1_aux, flag_mds),
     HOFFSET(struct mds1_aux, packet_hdr),
     HOFFSET(struct mds1_aux, data_hdr),
     HOFFSET(struct mds1_aux, pmtc_hdr),
     HOFFSET(struct mds1_aux, data_src)
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_AUX(unsigned int nr_aux, const struct mds1_aux *aux)
{
     hid_t   fid, ads_id, tid_pmtc;
     hid_t   aux_type[NFIELDS];

     hsize_t adim;

     const hbool_t compress = \
	  (nadc_get_param_uint8("flag_deflate") == PARAM_SET) ? TRUE : FALSE;
     const char *aux_names[NFIELDS] = {
          "dsr_time", "attach_flag", "packet_hdr", 
	  "data_hdr", "pmtc_hdr", "data_src"
     };
/*
 * check number of AUX records
 */
     if (nr_aux == 0) return;
/*
 * open/create group /ADS
 */
     fid = nadc_get_param_hid("hdf_file_id");
     ads_id = NADC_OPEN_HDF5_Group(fid, "/ADS");
     if (ads_id < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, "/ADS");
/*
 * define user-defined data types of the Table-fields
 */
     aux_type[0] = H5Topen(fid, "mjd", H5P_DEFAULT);
     aux_type[1] = H5Tcopy(H5T_NATIVE_UCHAR);
     aux_type[2] = H5Topen(fid, "packet_hdr", H5P_DEFAULT);
     aux_type[3] = H5Topen(fid, "data_hdr", H5P_DEFAULT);
     aux_type[4] = H5Topen(fid, "pmtc_hdr", H5P_DEFAULT);
     tid_pmtc = H5Topen(fid, "pmtc_frame", H5P_DEFAULT);
     adim = NUM_LV0_AUX_PMTC_FRAME;
     aux_type[5] = H5Tarray_create(tid_pmtc, 1, &adim);
     (void) H5Tclose(tid_pmtc);
/*
 * create table
 */
     (void) H5TBmake_table("aux", ads_id, "AUXILIARY_PACKETS", NFIELDS, 
			    nr_aux, aux_size, aux_names, aux_offs, aux_type, 
			    nr_aux, NULL, compress, aux);
/*
 * close interface
 */
     (void) H5Tclose(aux_type[0]);
     (void) H5Tclose(aux_type[1]);
     (void) H5Tclose(aux_type[2]);
     (void) H5Tclose(aux_type[3]);
     (void) H5Tclose(aux_type[4]);
     (void) H5Tclose(aux_type[5]);
     (void) H5Gclose(ads_id);
}
