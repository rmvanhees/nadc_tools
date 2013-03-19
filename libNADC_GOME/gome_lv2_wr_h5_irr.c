/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 1999 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GOME_LV2_WR_H5_IRR
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 2 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write GOME level 2 IRR data
.INPUT/OUTPUT
  call as    GOME_LV2_WR_H5_IRR( param, nr_ddr, ddr );
  input: 
             struct param_record param : struct holding user-defined settings
	     int  nr_ddr               : number of ddr records
	     struct ddr_gome *ddr      : DOAS Data records

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   17-Jul-2008	complete rewrite, RvH
              2.0   11-Nov-2001	moved to the new Error handling routines, RvH
              1.1   06-Nov-2000	let this module create its own group, RvH 
              1.0   31-Aug-1999 created by R. M. van Hees
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
#define _GOME_COMMON
#include <nadc_gome.h>

#define NFIELDS_IRR1   29
#define NFIELDS_IRR2   29

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_LV2_WR_H5_IRR( struct param_record param, 
			 short nr_ddr, const struct ddr_gome *ddr )
{
     const char prognm[] = "GOME_LV2_WR_H5_IRR";

     register short nt;
     register short nd = 0;

     hid_t   grp_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   irr1_type[NFIELDS_IRR1];
     hid_t   irr2_type[NFIELDS_IRR2];

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;

     const size_t irr1_size = sizeof( struct irr1_gome );
     const char *irr1_names[NFIELDS_IRR1] = { 
          "indx_vcd", "indx_doas", "indx_amf", "indx_icfa", "indx_stats",
	  "cloud_frac", "cloud_pres", "err_cloud_frac", "err_cloud_pres", 
	  "surface_pres", "cca_cloud_frac", "cca_subpixel", 
	  "pmd_avg", "pmd_sdev", "pixel_color", "pixel_gradient", 
	  "total_vcd", "error_vcd", "sland_doas", "error_doas", 
	  "rms_doas", "chi2_doas", "fit_doas", "iter_doas", "ground_ams", 
	  "cloud_ams", "intensity_ground", "intensity_cloud", 
	  "intensity_measured"
     };
     const size_t irr1_sizes[NFIELDS_IRR1] = {
	  sizeof(short), sizeof(short), sizeof(short), sizeof(short), 
	  sizeof(short), sizeof(float), sizeof(float), sizeof(float), 
	  sizeof(float), sizeof(float), sizeof(float), 16, 3 * sizeof(float), 
	  3 * sizeof(float), 16 * sizeof(float), sizeof(float), 
	  LVL2_MAX_NMOL * sizeof(float), LVL2_MAX_NMOL * sizeof(float), 
	  LVL2_MAX_NMOL * sizeof(float), LVL2_MAX_NMOL * sizeof(float), 
	  LVL2_MAX_NWIN * sizeof(float), LVL2_MAX_NWIN * sizeof(float), 
	  LVL2_MAX_NWIN * sizeof(float), LVL2_MAX_NWIN * sizeof(float), 
	  LVL2_MAX_NMOL * sizeof(float), LVL2_MAX_NMOL * sizeof(float), 
	  LVL2_MAX_NWIN * sizeof(float), LVL2_MAX_NWIN * sizeof(float), 
	  LVL2_MAX_NWIN * sizeof(float)
     };
     const size_t irr1_offs[NFIELDS_IRR1] = {
	  HOFFSET( struct irr1_gome, indx_vcd ),
	  HOFFSET( struct irr1_gome, indx_doas ),
	  HOFFSET( struct irr1_gome, indx_amf ),
	  HOFFSET( struct irr1_gome, indx_icfa ),
	  HOFFSET( struct irr1_gome, indx_stats ),

	  HOFFSET( struct irr1_gome, cloud_frac ),
	  HOFFSET( struct irr1_gome, cloud_pres ),
	  HOFFSET( struct irr1_gome, err_cloud_frac ),
	  HOFFSET( struct irr1_gome, err_cloud_pres ),
	  HOFFSET( struct irr1_gome, surface_pres ),
	  HOFFSET( struct irr1_gome, cca_cloud_frac ),
	  HOFFSET( struct irr1_gome, cca_subpixel ),

	  HOFFSET( struct irr1_gome, pmd_avg ),
	  HOFFSET( struct irr1_gome, pmd_sdev ),
	  HOFFSET( struct irr1_gome, pixel_color ),
	  HOFFSET( struct irr1_gome, pixel_gradient ),

	  HOFFSET( struct irr1_gome, total_vcd ),
	  HOFFSET( struct irr1_gome, error_vcd ),
	  HOFFSET( struct irr1_gome, slant_doas ),
	  HOFFSET( struct irr1_gome, error_doas ),
	  HOFFSET( struct irr1_gome, rms_doas ),
	  HOFFSET( struct irr1_gome, chi_doas ),
	  HOFFSET( struct irr1_gome, fit_doas ),
	  HOFFSET( struct irr1_gome, iter_doas ),
	  HOFFSET( struct irr1_gome, ground_amf ),
	  HOFFSET( struct irr1_gome, cloud_amf ),
	  HOFFSET( struct irr1_gome, intensity_ground ),
	  HOFFSET( struct irr1_gome, intensity_cloud ),
	  HOFFSET( struct irr1_gome, intensity_measured )
     };

     const size_t irr2_size = sizeof( struct irr2_gome );
     const char *irr2_names[NFIELDS_IRR2] = { 
          "indx_vcd", "indx_doas", "indx_amf", "ozone_temperature", 
	  "ozone_ring_corr", "ghost_column", "cld_frac", "error_cld_frac", 
	  "cld_height", "error_cld_height", "cld_press", "error_cld_press", 
	  "cld_albedo", "error_cld_albedo", "surface_height", "surface_press",
	  "surface_albedo", "total_vcd", "error_vcd", "slant_doas", 
	  "error_doas", "rms_doas", "chi2_doas", "fit_doas", "iter_doas", 
	  "ground_amf", "error_ground_amf", "cloud_amf", "error_cloud_amf"
     };
     const size_t irr2_sizes[NFIELDS_IRR2] = {
	  sizeof(short), sizeof(short), sizeof(short), 
	  sizeof(float), sizeof(float), sizeof(float), sizeof(float), 
	  sizeof(float), sizeof(float), sizeof(float), sizeof(float), 
	  sizeof(float), sizeof(float), sizeof(float), sizeof(float), 
	  sizeof(float), sizeof(float), 
	  LVL2_V2_NMOL * sizeof(float), LVL2_V2_NMOL * sizeof(float), 
	  LVL2_V2_NMOL * sizeof(float), LVL2_V2_NMOL * sizeof(float), 
	  LVL2_V2_NWIN * sizeof(float), LVL2_V2_NWIN * sizeof(float), 
	  LVL2_V2_NWIN * sizeof(float), LVL2_V2_NWIN * sizeof(float), 
	  LVL2_V2_NMOL * sizeof(float), LVL2_V2_NMOL * sizeof(float), 
	  LVL2_V2_NMOL * sizeof(float), LVL2_V2_NMOL * sizeof(float)
     };
     const size_t irr2_offs[NFIELDS_IRR2] = {
          HOFFSET( struct irr2_gome, indx_vcd ),
	  HOFFSET( struct irr2_gome, indx_doas ),
	  HOFFSET( struct irr2_gome, indx_amf ),
	  HOFFSET( struct irr2_gome, ozone_temperature ),
	  HOFFSET( struct irr2_gome, ozone_ring_corr ),
	  HOFFSET( struct irr2_gome, ghost_column ),
	  HOFFSET( struct irr2_gome, cld_frac ),
	  HOFFSET( struct irr2_gome, error_cld_frac ),
	  HOFFSET( struct irr2_gome, cld_height ),
	  HOFFSET( struct irr2_gome, error_cld_height ),
	  HOFFSET( struct irr2_gome, cld_press ),
	  HOFFSET( struct irr2_gome, error_cld_press ),
	  HOFFSET( struct irr2_gome, cld_albedo ),
	  HOFFSET( struct irr2_gome, error_cld_albedo ),
	  HOFFSET( struct irr2_gome, surface_height ),
	  HOFFSET( struct irr2_gome, surface_press ),
	  HOFFSET( struct irr2_gome, surface_albedo ),
	  HOFFSET( struct irr2_gome, total_vcd ),
	  HOFFSET( struct irr2_gome, error_vcd ),
	  HOFFSET( struct irr2_gome, slant_doas ),
	  HOFFSET( struct irr2_gome, error_doas ),
	  HOFFSET( struct irr2_gome, rms_doas ),
	  HOFFSET( struct irr2_gome, chi_doas ),
	  HOFFSET( struct irr2_gome, fit_doas ),
	  HOFFSET( struct irr2_gome, iter_doas ),
	  HOFFSET( struct irr2_gome, ground_amf ),
	  HOFFSET( struct irr2_gome, error_ground_amf ),
	  HOFFSET( struct irr2_gome, cloud_amf ),
	  HOFFSET( struct irr2_gome, error_cloud_amf )
     };
/*
 * check number of DDR records
 */
     if ( nr_ddr == 0 ) return;
/*
 * create group /DDR
 */
     grp_id = H5Gopen( param.hdf_file_id, "/DDR", H5P_DEFAULT );
     if ( grp_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/DDR" );
/*
 * write IRR data sets
 */
     if ( ddr->irr1 != NULL ) {
	  irr1_type[0] = H5Tcopy( H5T_NATIVE_SHORT );
	  irr1_type[1] = H5Tcopy( H5T_NATIVE_SHORT );
	  irr1_type[2] = H5Tcopy( H5T_NATIVE_SHORT );
	  irr1_type[3] = H5Tcopy( H5T_NATIVE_SHORT );
	  irr1_type[4] = H5Tcopy( H5T_NATIVE_SHORT );
	  irr1_type[5] = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr1_type[6] = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr1_type[7] = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr1_type[8] = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr1_type[9] = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr1_type[10] = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr1_type[15] = H5Tcopy( H5T_NATIVE_FLOAT );
	  adim = 3;
	  irr1_type[12] = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr1_type[13] = H5Tcopy( H5T_NATIVE_FLOAT );
	  adim = 16;
	  irr1_type[11] = H5Tcopy( H5T_NATIVE_CHAR );
	  irr1_type[14] = H5Tcopy( H5T_NATIVE_FLOAT );
	  adim = LVL2_MAX_NMOL;
	  irr1_type[16] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr1_type[17] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr1_type[18] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr1_type[19] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr1_type[24] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr1_type[25] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  adim = LVL2_MAX_NWIN;
	  irr1_type[20] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr1_type[21] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr1_type[22] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr1_type[23] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr1_type[26] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr1_type[27] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr1_type[28] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );

	  stat = H5TBmake_table( "Intermediate Result Records", grp_id, "IRR", 
				 NFIELDS_IRR1, 1, irr1_size, irr1_names,
				 irr1_offs, irr1_type, 1,
				 NULL, compress, ddr->irr1 );
	  if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, "irr1" );

	  for ( nd = 1; nd < nr_ddr; nd++ )
	       H5TBappend_records( grp_id, "IRR", 1, irr1_size, irr1_offs, 
				   irr1_sizes, ddr[nd].irr1 ); 

	  for ( nt = 0; nt < NFIELDS_IRR1; nt++ )
	       (void) H5Tclose( irr1_type[nt] );
     } else {
	  irr2_type[0]  = H5Tcopy( H5T_NATIVE_SHORT );
	  irr2_type[1]  = H5Tcopy( H5T_NATIVE_SHORT );
	  irr2_type[2]  = H5Tcopy( H5T_NATIVE_SHORT );
	  irr2_type[3]  = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr2_type[4]  = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr2_type[5]  = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr2_type[6]  = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr2_type[7]  = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr2_type[8]  = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr2_type[9]  = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr2_type[10] = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr2_type[11] = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr2_type[12] = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr2_type[13] = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr2_type[14] = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr2_type[15] = H5Tcopy( H5T_NATIVE_FLOAT );
	  irr2_type[16] = H5Tcopy( H5T_NATIVE_FLOAT );
	  adim = LVL2_V2_NMOL;
	  irr2_type[17] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr2_type[18] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr2_type[19] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr2_type[20] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr2_type[25] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr2_type[26] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr2_type[27] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr2_type[28] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  adim = LVL2_V2_NWIN;
	  irr2_type[21] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr2_type[22] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr2_type[23] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
	  irr2_type[24] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );

	  stat = H5TBmake_table( "Intermediate Result Records", grp_id, "IRR", 
				 NFIELDS_IRR2, 1, irr2_size, irr2_names,
				 irr2_offs, irr2_type, 1,
				 NULL, compress, ddr->irr2 );
	  if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, "irr2" );

	  for ( nd = 1; nd < nr_ddr; nd++ )
	       H5TBappend_records( grp_id, "IRR", 1, irr2_size, irr2_offs, 
				   irr2_sizes, ddr[nd].irr2 ); 

	  for ( nt = 0; nt < NFIELDS_IRR2; nt++ )
	       (void) H5Tclose( irr2_type[nt] );
     }
/*
 * close interface
 */
done:
     (void) H5Gclose( grp_id );
}
