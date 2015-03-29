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

.IDENTifer   SCIA_LV2_WR_H5_BIAS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIA level 2 BIAS Fitting Application Data Set(s)
.INPUT/OUTPUT
  call as    SCIA_LV2_WR_H5_BIAS( bias_name, param, nr_bias, bias );
     input:  
             char bias_name[]           : name of BIAS data set
             struct param_record param  : struct holding user-defined settings
	     unsigned int nr_bias       : number of Cloud/Aerosol data sets
	     struct bias_scia *bias     : BIAS Fitting Application Data Set(s)

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   26-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.0   18-Sep-2001	Created by R. M. van Hees 
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

#define NFIELDS    15

static const size_t bias_size = sizeof( struct bias_scia );
static const size_t bias_offs[NFIELDS] = {
     HOFFSET( struct bias_scia, mjd ),
     HOFFSET( struct bias_scia, quality ),
     HOFFSET( struct bias_scia, hghtflag ),
     HOFFSET( struct bias_scia, vcdflag ),
     HOFFSET( struct bias_scia, intg_time ),
     HOFFSET( struct bias_scia, numfitp ),
     HOFFSET( struct bias_scia, numsegm ),
     HOFFSET( struct bias_scia, numiter ),
     HOFFSET( struct bias_scia, dsrlen ),
     HOFFSET( struct bias_scia, hght ),
     HOFFSET( struct bias_scia, vcd ),
     HOFFSET( struct bias_scia, errvcd ),
     HOFFSET( struct bias_scia, closure ),
     HOFFSET( struct bias_scia, errclosure ),
     HOFFSET( struct bias_scia, rms )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV2_WR_H5_BIAS( const char bias_name[], struct param_record param, 
			  unsigned int nr_bias, const struct bias_scia *bias )
{
     register unsigned int nr;

     hid_t   grp_id;
     hid_t   bias_id;
     hbool_t compress;
     hsize_t adim;
     hvl_t   *vdata;
     hid_t   bias_type[NFIELDS];

     const char *bias_names[NFIELDS] = {
          "dsr_time", "quality_flag", "flag_ht_var_usg", 
	  "flag_bias_vcd_fit_flag", "integr_time", "num_geophy_param_fit", 
	  "num_seg_fit_win", "iter_num_fit_win", "dsr_length",
	  "ht_val_and_err", "trc_gas_vcd", "err_vcd", "fit_fc_param", 
	  "err_fit_fc_param", "rms_good_fit_win"
     };
/*
 * check number of BIAS records
 */
     if ( nr_bias == 0 ) return;
/*
 * set HDF5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
          compress = TRUE;
     else
          compress = FALSE;
/*
 * create group /MDS/<bias_name>
 */
     grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/MDS" );
     if ( grp_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/MDS" );
     bias_id = H5Gcreate( grp_id, bias_name, 
			  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT  );
     if ( bias_id < 0 ) 
	  NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, bias_name );
/*
 * define user-defined data types of the Table-fields
 */
     bias_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     bias_type[1] = H5Tcopy( H5T_NATIVE_CHAR );
     bias_type[2] = H5Tcopy( H5T_NATIVE_USHORT );
     bias_type[3] = H5Tcopy( H5T_NATIVE_USHORT );
     bias_type[4] = H5Tcopy( H5T_NATIVE_USHORT );
     bias_type[5] = H5Tcopy( H5T_NATIVE_USHORT );
     bias_type[6] = H5Tcopy( H5T_NATIVE_USHORT );
     bias_type[7] = H5Tcopy( H5T_NATIVE_USHORT );
     bias_type[8] = H5Tcopy( H5T_NATIVE_UINT );
     adim = 2;
     bias_type[9] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     bias_type[10] = H5Tcopy( H5T_NATIVE_FLOAT );
     bias_type[11] = H5Tcopy( H5T_NATIVE_FLOAT );
     bias_type[12] = H5Tcopy( H5T_NATIVE_FLOAT );
     bias_type[13] = H5Tcopy( H5T_NATIVE_FLOAT );
     adim = 4;
     bias_type[14] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
/*
 * create table
 */
     (void) H5TBmake_table( bias_name, bias_id, "bias", NFIELDS, 
			    nr_bias, bias_size, bias_names, bias_offs, 
			    bias_type, nr_bias, NULL, compress, bias );
/*
 * close interface
 */
     for ( nr = 0; nr < NFIELDS; nr++ ) (void) H5Tclose( bias_type[nr] );
/*
 * +++++ create/write variable part of the <bias_name> group
 */
     adim = (hsize_t) nr_bias;
/*
 * cross correlation parameters
 */
     vdata = (hvl_t *) malloc( nr_bias * sizeof(hvl_t) );
     if ( vdata == NULL ) NADC_RETURN_ERROR( NADC_ERR_ALLOC, "vdata" );
     nr = 0;
     do {
	  vdata[nr].len = (size_t) 
	       (bias[nr].numfitp * (bias[nr].numfitp-1)) / 2;
	  vdata[nr].p = malloc( vdata[nr].len * sizeof(float) );
	  if ( vdata[nr].p == NULL ) {
	       free( vdata );
	       NADC_RETURN_ERROR( NADC_ERR_ALLOC, "vdata.p" );
	  }
	  (void) memcpy( vdata[nr].p , bias[nr].corrpar,
			 vdata[nr].len * sizeof(float) );

     } while ( ++nr < nr_bias );
     NADC_WR_HDF5_Vlen_Dataset( compress, bias_id, "corrpar",
				H5T_NATIVE_FLOAT, 1, &adim, vdata );
/*
 * close interface
 */
     (void) H5Gclose( bias_id );
     (void) H5Gclose( grp_id );
}
