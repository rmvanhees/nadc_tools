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

.IDENTifer   SCIA_LV1_WR_H5_VLCP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 VLCP data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_VLCP( param, nr_vlcp, vlcp );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned int nr_vlcp      : number of var. Leakage Current Param.
	     struct vlcp_scia *vlcp    : Leakage Current Paramameters (var.)

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   26-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.0   18-Nov-1999	Created by R. M. van Hees 
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

#define NFIELDS    10

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_VLCP( struct param_record param, unsigned int nr_vlcp,
			  const struct vlcp_scia *vlcp )
{
     const char prognm[] = "SCIA_LV1_WR_H5_VLCP";

     hid_t   gads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   vlcp_type[NFIELDS];

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;

     const char *vlcp_names[NFIELDS] = { 
	  "orbit_phase", "obm_pmd", "var_lc", "var_lc_error", "solar_stray", 
	  "solar_stray_error", "pmd_stray", "pmd_stray_error", "pmd_dark", 
	  "pmd_dark_error"
     };
     const size_t vlcp_size = sizeof( struct vlcp_scia );
     const size_t vlcp_offs[NFIELDS] = {
	  HOFFSET( struct vlcp_scia, orbit_phase ),
	  HOFFSET( struct vlcp_scia, obm_pmd ),
	  HOFFSET( struct vlcp_scia, var_lc ),
	  HOFFSET( struct vlcp_scia, var_lc_error ),
	  HOFFSET( struct vlcp_scia, solar_stray ),
	  HOFFSET( struct vlcp_scia, solar_stray_error ),
	  HOFFSET( struct vlcp_scia, pmd_stray ),
	  HOFFSET( struct vlcp_scia, pmd_stray_error ),
	  HOFFSET( struct vlcp_scia, pmd_dark ),
	  HOFFSET( struct vlcp_scia, pmd_dark_error )
     };
/*
 * check number of RSPO records
 */
     if ( nr_vlcp == 0 ) return;
/*
 * open/create group /GADS
 */
     gads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/GADS" );
     if ( gads_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/GADS" );
/*
 * write VLCP data sets
 */
     vlcp_type[0] = H5Tcopy( H5T_NATIVE_FLOAT );
     adim = IR_CHANNELS + PMD_NUMBER;
     vlcp_type[1] = H5Tcopy( H5T_NATIVE_FLOAT );
     adim = IR_CHANNELS * CHANNEL_SIZE;
     vlcp_type[2] = H5Tcopy( H5T_NATIVE_FLOAT );
     vlcp_type[3] = H5Tcopy( H5T_NATIVE_FLOAT );
     adim = SCIENCE_PIXELS;
     vlcp_type[4] = H5Tcopy( H5T_NATIVE_FLOAT );
     vlcp_type[5] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     adim = PMD_NUMBER;
     vlcp_type[6] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     vlcp_type[7] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     adim = IR_PMD_NUMBER;
     vlcp_type[8] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     vlcp_type[9] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );

     stat = H5TBmake_table( "vlcp", gads_id, "LEAKAGE_VARIABLE",
                            NFIELDS, nr_vlcp, vlcp_size, vlcp_names,
                            vlcp_offs, vlcp_type, 1,
                            NULL, compress, vlcp );
     if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, "vlcp" );
/*
 * close interface
 */
 done:
     (void) H5Tclose( vlcp_type[0] );
     (void) H5Tclose( vlcp_type[1] );
     (void) H5Tclose( vlcp_type[2] );
     (void) H5Tclose( vlcp_type[3] );
     (void) H5Tclose( vlcp_type[4] );
     (void) H5Tclose( vlcp_type[5] );
     (void) H5Tclose( vlcp_type[6] );
     (void) H5Tclose( vlcp_type[7] );
     (void) H5Tclose( vlcp_type[8] );
     (void) H5Tclose( vlcp_type[9] );
     (void) H5Gclose( gads_id );
}
