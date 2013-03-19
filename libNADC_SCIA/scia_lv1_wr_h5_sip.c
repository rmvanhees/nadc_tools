/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2000 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_WR_H5_SIP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 SIP data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_SIP( param, sip );
     input:  
             struct param_record param : struct holding user-defined settings
	     struct sip_scia   *sip    : Static Instrument Parameters

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   20-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.1   06-Mar-2002	update to v.4: level_2_smr, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.0   18-Dec-2000	created by R. M. van Hees 
------------------------------------------------------------*/
/* * Define _POSIX_SOURCE to indicate
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

#define NFIELDS    31

static const size_t sip_size = sizeof( struct sip_scia );
static const size_t sip_offs[NFIELDS] = {
     HOFFSET( struct sip_scia, do_use_limb_dark ),
     HOFFSET( struct sip_scia, do_pixelwise ),
     HOFFSET( struct sip_scia, do_ib_oc_etn ),
     HOFFSET( struct sip_scia, do_ib_sd_etn ),
     HOFFSET( struct sip_scia, do_fraunhofer ),
     HOFFSET( struct sip_scia, do_etalon ),
     HOFFSET( struct sip_scia, do_var_lc_cha ),
     HOFFSET( struct sip_scia, do_stray_lc_cha ),
     HOFFSET( struct sip_scia, do_var_lc_pmd ),
     HOFFSET( struct sip_scia, do_stray_lc_pmd ),
     HOFFSET( struct sip_scia, do_pol_point ),
     HOFFSET( struct sip_scia, n_lc_min ),
     HOFFSET( struct sip_scia, ds_n_phases ),
     HOFFSET( struct sip_scia, sp_n_phases ),
     HOFFSET( struct sip_scia, ds_poly_order ),
     HOFFSET( struct sip_scia, lc_harm_order ),
     HOFFSET( struct sip_scia, level_2_smr ),
     HOFFSET( struct sip_scia, sat_level ),
     HOFFSET( struct sip_scia, pmd_sat_limit ),
     HOFFSET( struct sip_scia, startpix_6 ),
     HOFFSET( struct sip_scia, startpix_8 ),
     HOFFSET( struct sip_scia, alpha0_asm ),
     HOFFSET( struct sip_scia, alpha0_esm ),
     HOFFSET( struct sip_scia, ppg_error ),
     HOFFSET( struct sip_scia, stray_error ),
     HOFFSET( struct sip_scia, h_toa ),
     HOFFSET( struct sip_scia, lambda_end_gdf ),
     HOFFSET( struct sip_scia, ds_phase_boundaries ),
     HOFFSET( struct sip_scia, sp_phase_boundaries ),
     HOFFSET( struct sip_scia, lc_stray_indx ),
     HOFFSET( struct sip_scia, electrons_bu )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_SIP( struct param_record param, 
			 const struct sip_scia *sip )
{
     const char prognm[] = "SCIA_LV1_WR_H5_SIP";

     hid_t   gads_id;
     hsize_t adim;

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;
     const char *sip_names[NFIELDS] = {
          "do_use_limb_dark", "do_pixelwise", "do_ib_oc_etn", "do_ib_sd_etn", 
	  "do_fraunhofer", "do_etalon", "do_var_lc_cha", "do_stray_lc_cha", 
	  "do_var_lc_pmd", "do_stray_lc_pmd", "do_pol_point", "n_lc_min", 
	  "ds_n_phases", "sp_n_phases", "ds_poly_order", "lc_harm_order", 
	  "level_2_smr", "sat_level", "pmd_sat_limit", "startpix_6", 
	  "startpix_8", "alpha0_asm", "alpha0_esm", "ppg_error", 
	  "stray_error", "h_toa", "lambda_end_gdf", "ds_phase_boundaries", 
	  "sp_phase_boundaries", "lc_stray_indx", "electrons_bu"
     };
#if !defined(__mips) && !defined (__hpux)
     hid_t sip_type[NFIELDS] = {
          -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  H5T_NATIVE_UCHAR, H5T_NATIVE_UCHAR, H5T_NATIVE_UCHAR, 
	  H5T_NATIVE_UCHAR, H5T_NATIVE_UCHAR, -1, -1, H5T_NATIVE_USHORT,
	  H5T_NATIVE_SHORT, H5T_NATIVE_SHORT, H5T_NATIVE_FLOAT,
	  H5T_NATIVE_FLOAT, H5T_NATIVE_FLOAT, H5T_NATIVE_FLOAT, 
	  H5T_NATIVE_FLOAT, H5T_NATIVE_FLOAT, -1, -1, -1, -1
     };
#else
     hid_t sip_type[NFIELDS];

     sip_type[0] = -1;
     sip_type[1] = -1;
     sip_type[2] = -1;
     sip_type[3] = -1;
     sip_type[4] = -1;
     sip_type[5] = -1;
     sip_type[6] = -1;
     sip_type[7] = -1;
     sip_type[8] = -1;
     sip_type[9] = -1;
     sip_type[10] = -1;
     sip_type[11] = H5T_NATIVE_UCHAR;
     sip_type[12] = H5T_NATIVE_UCHAR;
     sip_type[13] = H5T_NATIVE_UCHAR;
     sip_type[14] = H5T_NATIVE_UCHAR;
     sip_type[15] = H5T_NATIVE_UCHAR;
     sip_type[16] = -1;
     sip_type[17] = -1;
     sip_type[18] = H5T_NATIVE_USHORT;
     sip_type[19] = H5T_NATIVE_USHORT;
     sip_type[20] = H5T_NATIVE_USHORT;
     sip_type[21] = H5T_NATIVE_FLOAT;
     sip_type[22] = H5T_NATIVE_FLOAT;
     sip_type[23] = H5T_NATIVE_FLOAT;
     sip_type[24] = H5T_NATIVE_FLOAT;
     sip_type[25] = H5T_NATIVE_FLOAT;
     sip_type[26] = H5T_NATIVE_FLOAT;
     sip_type[27] = -1;
     sip_type[28] = -1;
     sip_type[29] = -1;
     sip_type[30] = -1;
#endif
/*
 * create group /GADS/SIP
 */
     gads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/GADS" );
     if ( gads_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/GADS" );
/*
 * define user-defined data types of the Table-fields
 */
     sip_type[0] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( sip_type[0], (size_t) 2 );
     sip_type[1] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( sip_type[1], (size_t) SCIENCE_CHANNELS+1 );
     sip_type[2] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( sip_type[2], (size_t) PMD_NUMBER+1 );
     sip_type[3] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( sip_type[3], (size_t) PMD_NUMBER+1 );
     sip_type[4] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( sip_type[4], (size_t) 5 * SCIENCE_CHANNELS+1 );
     sip_type[5] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( sip_type[5], (size_t) 3 * SCIENCE_CHANNELS+1 );
     sip_type[6] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( sip_type[6], (size_t) 4 * IR_CHANNELS+1 );
     sip_type[7] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( sip_type[7], (size_t) 4 * SCIENCE_CHANNELS+1 );
     sip_type[8] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( sip_type[8], (size_t) 4 * IR_PMD_NUMBER+1 );
     sip_type[9] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( sip_type[9], (size_t) 4 * PMD_NUMBER+1 );
     sip_type[10] = H5Tcopy( H5T_C_S1 );
     (void) H5Tset_size( sip_type[10], (size_t) NUM_FRAC_POLV+1 );
     adim = SCIENCE_CHANNELS;
     sip_type[16] = H5Tarray_create( H5T_NATIVE_UCHAR, 1, &adim );
     sip_type[17] = H5Tarray_create( H5T_NATIVE_USHORT, 1, &adim );
     adim = MaxBoundariesSIP;
     sip_type[27] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     sip_type[28] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     adim = 2;
     sip_type[29] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     adim = SCIENCE_CHANNELS;
     sip_type[30] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
/*
 * create table
 */
     (void) H5TBmake_table( "sip", gads_id, "INSTRUMENT_PARAMS",
			    NFIELDS, 1, sip_size, sip_names, sip_offs, 
			    sip_type, 1, NULL, compress, sip );
/*
 * close interface
 */
     (void) H5Tclose( sip_type[0] );
     (void) H5Tclose( sip_type[1] );
     (void) H5Tclose( sip_type[2] );
     (void) H5Tclose( sip_type[3] );
     (void) H5Tclose( sip_type[4] );
     (void) H5Tclose( sip_type[5] );
     (void) H5Tclose( sip_type[6] );
     (void) H5Tclose( sip_type[7] );
     (void) H5Tclose( sip_type[8] );
     (void) H5Tclose( sip_type[9] );
     (void) H5Tclose( sip_type[10] );
     (void) H5Tclose( sip_type[16] );
     (void) H5Tclose( sip_type[17] );
     (void) H5Tclose( sip_type[27] );
     (void) H5Tclose( sip_type[28] );
     (void) H5Tclose( sip_type[29] );
     (void) H5Tclose( sip_type[30] );
/*
 * close interface
 */
     (void) H5Gclose( gads_id );
}
