/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV2_WR_H5_SQADS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 2 SQADS data
.INPUT/OUTPUT
  call as    SCIA_LV2_WR_H5_SQADS( param, nr_sqads, sqads );
     input:  
             struct param_record param  : struct holding user-defined settings
	     unsigned int nr_sqads      : number of Summary of Quality Flags
	     struct sqads2_scia *sqads  : Summary of Quality Flags per State

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   26-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.1   03-Sep-2002	moved DSD-data to subgroup ADS, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.0   13-Sep-2001	created by R. M. van Hees 
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

static const size_t sqads_size = sizeof( struct sqads2_scia );
static const size_t sqads_offs[NFIELDS] = {
     HOFFSET( struct sqads2_scia, mjd ),
     HOFFSET( struct sqads2_scia, flag_mds ),
     HOFFSET( struct sqads2_scia, flag_pqf )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV2_WR_H5_SQADS( struct param_record param, unsigned int nr_sqads,
			   const struct sqads2_scia *sqads )
{
     const char prognm[] = "SCIA_LV2_WR_H5_SQADS";

     hid_t   ads_id;
     hbool_t compress;
     hsize_t adim;
     hid_t   sqads_type[NFIELDS];

     const char *sqads_names[NFIELDS] = {
          "dsr_time", "attach_flag", "sq_geophy_para"
     };
/*
 * check number of SQADS records
 */
     if ( nr_sqads == 0 ) return;
/*
 * set HDF5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
          compress = TRUE;
     else
          compress = FALSE;
/*
 * create group /ADS
 */
     ads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/ADS" );
     if ( ads_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/ADS" );
/*
 * define user-defined data types of the Table-fields
 */
     sqads_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     sqads_type[1] = H5Tcopy( H5T_NATIVE_UCHAR );
     adim = NL2_SQADS_PQF_FLAGS;
     sqads_type[2] = H5Tarray_create( H5T_NATIVE_UCHAR, 1, &adim );
/*
 * create table
 */
     (void) H5TBmake_table( "Summary Quality ADS", ads_id, "sqads", NFIELDS, 
			    nr_sqads, sqads_size, sqads_names, sqads_offs, 
			    sqads_type, nr_sqads, NULL, compress, sqads );
/*
 * close interface
 */
     (void) H5Tclose( sqads_type[0] );
     (void) H5Tclose( sqads_type[1] );
     (void) H5Tclose( sqads_type[2] );
     (void) H5Gclose( ads_id );
}
