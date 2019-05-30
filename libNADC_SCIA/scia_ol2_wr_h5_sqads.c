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

.IDENTifer   SCIA_OL2_WR_H5_SQADS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 offline product - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 2 SQADS data
.INPUT/OUTPUT
  call as    SCIA_OL2_WR_H5_SQADS(nr_sqads, sqads);
     input:  
	     unsigned int nr_sqads      : number of Summary of Quality Flags
	     struct sqads_sci_ol *sqads : Summary of Quality Flags per State

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   26-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              1.0   29-Apr-2002	created by R. M. van Hees 
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

#define NFIELDS    3

static const size_t sqads_size = sizeof(struct sqads_sci_ol);
static const size_t sqads_offs[NFIELDS] = {
     HOFFSET(struct sqads_sci_ol, mjd),
     HOFFSET(struct sqads_sci_ol, flag_mds),
     HOFFSET(struct sqads_sci_ol, flag_pqf)
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_OL2_WR_H5_SQADS(unsigned int nr_sqads,
			  const struct sqads_sci_ol *sqads)
{
     hid_t   fid, ads_id;
     hbool_t compress;
     hsize_t adim;
     hid_t   sqads_type[NFIELDS];

     const char *sqads_names[NFIELDS] = {
          "dsr_time", "attach_flag", "sq_geophy_para"
     };
/*
 * check number of SQADS records
 */
     if (nr_sqads == 0) return;
/*
 * set HDF5 boolean variable for compression
 */
     if (nadc_get_param_uint8("flag_deflate") == PARAM_SET)
          compress = TRUE;
     else
          compress = FALSE;
/*
 * create group /ADS
 */
     fid = nadc_get_param_hid("hdf_file_id");
     ads_id = NADC_OPEN_HDF5_Group(fid, "/ADS");
     if (ads_id < 0) NADC_RETURN_ERROR(NADC_ERR_HDF_GRP, "/ADS");
/*
 * define user-defined data types of the Table-fields
 */
     sqads_type[0] = H5Topen(fid, "mjd", H5P_DEFAULT);
     sqads_type[1] = H5Tcopy(H5T_NATIVE_UCHAR);
     adim = OL2_SQADS_PQF_FLAGS;
     sqads_type[2] = H5Tarray_create(H5T_NATIVE_UCHAR, 1, &adim);
/*
 * create table
 */
     (void) H5TBmake_table("Summary Quality ADS", ads_id, "sqads", NFIELDS, 
			    nr_sqads, sqads_size, sqads_names, sqads_offs, 
			    sqads_type, nr_sqads, NULL, compress, sqads);
/*
 * close interface
 */
     (void) H5Tclose(sqads_type[0]);
     (void) H5Tclose(sqads_type[1]);
     (void) H5Tclose(sqads_type[2]);
     (void) H5Gclose(ads_id);
}
