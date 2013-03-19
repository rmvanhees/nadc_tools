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

.IDENTifer   SCIA_LV1_WR_H5_CLCP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 CLCP data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_CLCP( param, clcp );
     input:  
             struct param_record param : struct holding user-defined settings
	     struct clcp_scia *clcp    : Leakage Current Paramameters (const)

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   20-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.0   18-Nov-1999	created by R. M. van Hees 
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

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_CLCP( struct param_record param, 
			  const struct clcp_scia *clcp )
{
     const char prognm[] = "SCIA_LV1_WR_H5_CLCP";

     hid_t   gads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   clcp_type[7];

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;

     const char *clcp_names[] = { "fpn", "fpn_error", "lc", "lc_error", 
				  "pmd_dark", "pmd_dark_error", "mean_noise" };
     const size_t clcp_size = sizeof( struct clcp_scia );
     const size_t clcp_offs[] = {
	  HOFFSET( struct clcp_scia, fpn ),
	  HOFFSET( struct clcp_scia, fpn_error ),
	  HOFFSET( struct clcp_scia, lc ),
	  HOFFSET( struct clcp_scia, lc_error ),
	  HOFFSET( struct clcp_scia, pmd_dark ),
	  HOFFSET( struct clcp_scia, pmd_dark_error ),
	  HOFFSET( struct clcp_scia, mean_noise )
     };
/*
 * create group /GADS
 */
     gads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/GADS" );
     if ( gads_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/GADS" );
/*
 * write CLCP data sets
 */
     adim = SCIENCE_PIXELS;
     clcp_type[0] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     clcp_type[1] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     clcp_type[2] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     clcp_type[3] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     clcp_type[6] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     adim = 2 * PMD_NUMBER;
     clcp_type[4] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );
     clcp_type[5] = H5Tarray_create( H5T_NATIVE_FLOAT, 1, &adim );

     stat = H5TBmake_table( "clcp", gads_id, "LEAKAGE_CONSTANT", 
                            7, 1, clcp_size, clcp_names,
                            clcp_offs, clcp_type, 1,
                            NULL, compress, clcp );
     if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, "clcp" );
/*
 * close interface
 */
 done:
     (void) H5Tclose( clcp_type[0] );
     (void) H5Tclose( clcp_type[1] );
     (void) H5Tclose( clcp_type[2] );
     (void) H5Tclose( clcp_type[3] );
     (void) H5Tclose( clcp_type[4] );
     (void) H5Tclose( clcp_type[5] );
     (void) H5Tclose( clcp_type[6] );
     (void) H5Gclose( gads_id );
}
