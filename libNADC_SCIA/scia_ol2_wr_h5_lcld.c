/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2011 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_OL2_WR_H5_LCLD
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 2 Offline product - HDF5
.LANGUAGE    ANSI C
.PURPOSE     write SCIAMACHY level 2 Limb Clouds data sets
.INPUT/OUTPUT
  call as    SCIA_OL2_WR_H5_LCLD( lcld_name, param, nr_lcld, lcld );
     input:  
             char lcld_name[]          : name of fitted species
             struct param_record param : struct holding user-defined settings
	     unsigned int nr_lcld      : number of Nadir Fitting Windows
	     struct lcld_scia *lcld    : Limb Clouds Data Set(s)

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0   10-Jan-2011	created by R. M. van Hees 
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

static const size_t lcld_size = sizeof( struct lcld_scia );
static const size_t lcld_offs[NFIELDS] = {
     HOFFSET( struct lcld_scia, mjd ),
     HOFFSET( struct lcld_scia, dsrlen ),
     HOFFSET( struct lcld_scia, quality ),
     HOFFSET( struct lcld_scia, diag_cloud_algo ),
     HOFFSET( struct lcld_scia, flag_normal_water ),
     HOFFSET( struct lcld_scia, flag_water_clouds ),
     HOFFSET( struct lcld_scia, flag_ice_clouds ),
     HOFFSET( struct lcld_scia, hght_index_max_value_ice ),
     HOFFSET( struct lcld_scia, flag_polar_strato_clouds ),
     HOFFSET( struct lcld_scia, hght_index_max_value_strato ),
     HOFFSET( struct lcld_scia, flag_noctilucent_clouds ),
     HOFFSET( struct lcld_scia, hght_index_max_value_noctilucent ),
     HOFFSET( struct lcld_scia, intg_time ),
     HOFFSET( struct lcld_scia, num_tangent_hghts ),
     HOFFSET( struct lcld_scia, num_cir ),
     HOFFSET( struct lcld_scia, num_limb_para ),
     HOFFSET( struct lcld_scia, max_value_cir ),
     HOFFSET( struct lcld_scia, hght_max_value_cir ),
     HOFFSET( struct lcld_scia, max_value_cir_ice ),
     HOFFSET( struct lcld_scia, hght_max_value_cir_ice ),
     HOFFSET( struct lcld_scia, max_value_cir_strato ),
     HOFFSET( struct lcld_scia, hght_max_value_cir_strato ),
     HOFFSET( struct lcld_scia, hght_max_value_noctilucent )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_OL2_WR_H5_LCLD( struct param_record param, 
			  unsigned int nr_lcld, const struct lcld_scia *lcld )
{
     const char prognm[] = "SCIA_OL2_WR_H5_LCLD";

     register unsigned int nr;

     int     compress;
     hid_t   grp_id;
     hid_t   lcld_id;
     size_t  total;
     hsize_t adim;
     hvl_t   *vdata;
     hid_t   lcld_type[NFIELDS];

     const char dsd_name[] = "LIM_CLOUDS";

     const char *lcld_names[NFIELDS] = {
          "dsr_time", "dsr_length", "quality_flag", "diag_cloud_algo", 
	  "flag_normal_water", "flag_water_clouds", "flag_ice_clouds", 
	  "hght_index_max_value_ice", "flag_polar_strato_clouds", 
	  "hght_index_max_value_strato", "flag_noctilucent_clouds", 
	  "hght_index_max_value_noctilucent", "integr_time", 
	  "num_tangent_hghts", "num_cir;", "num_limb_para", 
	  "max_value_cir", "hght_max_value_cir", "max_value_cir_ice;",
	  "hght_max_value_cir_ice", "max_value_cir_strato", 
	  "hght_max_value_cir_strato", "hght_max_value_noctilucent"
     };
/*
 * check number of LCLD records
 */
     if ( nr_lcld == 0 ) return;
/*
 * set HDF5 boolean variable for compression
 */
     if ( param.flag_deflate == PARAM_SET )
          compress = 3;
     else
          compress = 0;
/*
 * create group /MDS/<lcld_name>
 */
     grp_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/MDS" );
     if ( grp_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/MDS" );
     lcld_id = H5Gcreate( grp_id, dsd_name,
			  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     if ( lcld_id < 0 ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, dsd_name );
/*
 * define user-defined data types of the Table-fields
 */
     lcld_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     lcld_type[1] = H5Tcopy( H5T_NATIVE_UINT );
     lcld_type[2] = H5Tcopy( H5T_NATIVE_CHAR );
     lcld_type[3] = H5Tcopy( H5T_NATIVE_UCHAR );
     lcld_type[4] = H5Tcopy( H5T_NATIVE_UCHAR );
     lcld_type[5] = H5Tcopy( H5T_NATIVE_UCHAR );
     lcld_type[6] = H5Tcopy( H5T_NATIVE_UCHAR );
     lcld_type[7] = H5Tcopy( H5T_NATIVE_UCHAR );
     lcld_type[8] = H5Tcopy( H5T_NATIVE_UCHAR );
     lcld_type[9] = H5Tcopy( H5T_NATIVE_UCHAR );
     lcld_type[10] = H5Tcopy( H5T_NATIVE_UCHAR );
     lcld_type[11] = H5Tcopy( H5T_NATIVE_UCHAR );
     lcld_type[12] = H5Tcopy( H5T_NATIVE_USHORT );
     lcld_type[13] = H5Tcopy( H5T_NATIVE_USHORT );
     lcld_type[14] = H5Tcopy( H5T_NATIVE_USHORT );
     lcld_type[15] = H5Tcopy( H5T_NATIVE_USHORT );
     lcld_type[16] = H5Tcopy( H5T_NATIVE_FLOAT );
     lcld_type[17] = H5Tcopy( H5T_NATIVE_FLOAT );
     lcld_type[18] = H5Tcopy( H5T_NATIVE_FLOAT );
     lcld_type[19] = H5Tcopy( H5T_NATIVE_FLOAT );
     lcld_type[20] = H5Tcopy( H5T_NATIVE_FLOAT );
     lcld_type[21] = H5Tcopy( H5T_NATIVE_FLOAT );
     lcld_type[22] = H5Tcopy( H5T_NATIVE_FLOAT );
/*
 * create table
 */
     (void) H5TBmake_table( dsd_name, lcld_id, "lcld", NFIELDS, 
			    nr_lcld, lcld_size, lcld_names, lcld_offs, 
			    lcld_type, nr_lcld, NULL, compress, lcld );
/*
 * close interface
 */
     for ( nr = 0; nr < NFIELDS; nr++ ) (void) H5Tclose( lcld_type[nr] );
/*
 * +++++ create/write datasets in the /MDS/LCLD group
 */
     adim = (hsize_t) nr_lcld;
/*
 * 
 */
     vdata = (hvl_t *) malloc( nr_lcld * sizeof( hvl_t ) );
     if ( vdata == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata" );
     nr = 0;
     total = 0;
     do {
          total += (vdata[nr].len = (size_t) lcld[nr].num_tangent_hghts);
          if ( lcld[nr].num_tangent_hghts > USHRT_ZERO ) {
               vdata[nr].p = malloc( vdata[nr].len * sizeof(float) );
               if ( vdata[nr].p == NULL ) {
                    free( vdata );
                    NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata.p" );
               }
               (void) memcpy( vdata[nr].p, lcld[nr].tangent_hghts,
                              vdata[nr].len * sizeof(float) );
          }
     } while ( ++nr < nr_lcld );
     if ( total > 0 ) 
	  NADC_WR_HDF5_Vlen_Dataset( compress, lcld_id, "tangent_hghts",
				     H5T_NATIVE_FLOAT, 1, &adim, vdata );
/*
 * 
 */
     vdata = (hvl_t *) malloc( nr_lcld * sizeof( hvl_t ) );
     if ( vdata == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata" );
     nr = 0;
     total = 0;
     do {
          total += (vdata[nr].len = lcld[nr].num_tangent_hghts * lcld[nr].num_cir);
          if ( vdata[nr].len > USHRT_ZERO ) {
               vdata[nr].p = malloc( vdata[nr].len * sizeof(float) );
               if ( vdata[nr].p == NULL ) {
                    free( vdata );
                    NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata.p" );
               }
               (void) memcpy( vdata[nr].p, lcld[nr].cir,
                              vdata[nr].len * sizeof(float) );
          }
     } while ( ++nr < nr_lcld );
     if ( total > 0 ) 
	  NADC_WR_HDF5_Vlen_Dataset( compress, lcld_id, "cir",
				     H5T_NATIVE_FLOAT, 1, &adim, vdata );
/*
 * 
 */
     vdata = (hvl_t *) malloc( nr_lcld * sizeof( hvl_t ) );
     if ( vdata == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata" );
     nr = 0;
     total = 0;
     do {
          total += (vdata[nr].len = (size_t) lcld[nr].num_limb_para);
          if ( lcld[nr].num_limb_para > USHRT_ZERO ) {
               vdata[nr].p = malloc( vdata[nr].len * sizeof(float) );
               if ( vdata[nr].p == NULL ) {
                    free( vdata );
                    NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "vdata.p" );
               }
               (void) memcpy( vdata[nr].p, lcld[nr].limb_para,
                              vdata[nr].len * sizeof(float) );
          }
     } while ( ++nr < nr_lcld );
     if ( total > 0 ) 
	  NADC_WR_HDF5_Vlen_Dataset( compress, lcld_id, "limb_para",
				     H5T_NATIVE_FLOAT, 1, &adim, vdata );
/*
 * close interface
 */
     (void) H5Gclose( lcld_id );
     (void) H5Gclose( grp_id );
}
