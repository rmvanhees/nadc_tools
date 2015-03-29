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

.IDENTifer   SCIA_LV1_WR_H5_RSP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 RSP data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_RSPN( param, nr_rsp, rspn );
             SCIA_LV1_WR_H5_RSPL( param, nr_rsp, rsplo );
             SCIA_LV1_WR_H5_RSPO( param, nr_rsp, rsplo );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned int nr_rsp       : number of nadir RSP's
	     struct rspn_scia *rspn  : Radiance Sensitivity Parameters
                                        (nadir)
	     struct rsplo_scia *rsplo: Radiance Sensitivity Parameters
                                        (limb/occultation)
.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   21-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.1   03-Jan-2001	separate functions for nadir 
                                   & limb/occultation parameter sets, RvH
              1.0   19-Nov-1999 created by R. M. van Hees
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

/*+++++++++++++++++++++++++ Main Program or Functions +++++++++++++++*/
void SCIA_LV1_WR_H5_RSPN( struct param_record param, unsigned int nr_rsp,
			  const struct rspn_scia *rspn )
{
     hid_t   gads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   rspn_type[2] = { H5T_NATIVE_FLOAT, -1 };

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;

     const char *rspn_names[2] = { "ang_esm", "sensitivity" };
     const size_t rspn_size = sizeof( struct rspn_scia );
     const size_t rspn_offs[2] = {
	  HOFFSET( struct rspn_scia, ang_esm ),
	  HOFFSET( struct rspn_scia, sensitivity )
     };
/*
 * check number of RSPN records
 */
     if ( nr_rsp == 0 ) return;
/*
 * open/create group /GADS
 */
     gads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/GADS" );
     if ( gads_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/GADS" );
/*
 * write RSPN data sets
 */
     adim = SCIENCE_PIXELS;
     rspn_type[1] = H5Tarray_create( H5T_NATIVE_DOUBLE, 1, &adim );

     stat = H5TBmake_table( "rspn", gads_id, "RAD_SENS_NADIR",
                            2, nr_rsp, rspn_size, rspn_names,
                            rspn_offs, rspn_type, 1,
                            NULL, compress, rspn );
     if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "rspn" );
/*
 * close interface
 */
 done:
     (void) H5Tclose( rspn_type[1] );
     (void) H5Gclose( gads_id );
}

void SCIA_LV1_WR_H5_RSPL( struct param_record param, unsigned int nr_rsp,
			  const struct rsplo_scia *rspl )
{
     hid_t   gads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   rspl_type[3] = { H5T_NATIVE_FLOAT, H5T_NATIVE_FLOAT, -1 };

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;

     const char *rspl_names[3] = { "ang_esm", "ang_asm", "sensitivity" };
     const size_t rspl_size = sizeof( struct rsplo_scia );
     const size_t rspl_offs[3] = {
	  HOFFSET( struct rsplo_scia, ang_esm ),
	  HOFFSET( struct rsplo_scia, ang_asm ),
	  HOFFSET( struct rsplo_scia, sensitivity )
     };
/*
 * check number of RSPL records
 */
     if ( nr_rsp == 0 ) return;
/*
 * open/create group /GADS
 */
     gads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/GADS" );
     if ( gads_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/GADS" );
/*
 * write RSPL data sets
 */
     adim = SCIENCE_PIXELS;
     rspl_type[2] = H5Tarray_create( H5T_NATIVE_DOUBLE, 1, &adim );

     stat = H5TBmake_table( "rspl", gads_id, "RAD_SENS_LIMB",
                            3, nr_rsp, rspl_size, rspl_names,
                            rspl_offs, rspl_type, 1,
                            NULL, compress, rspl );
     if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "rspl" );
/*
 * close interface
 */
 done:
     (void) H5Tclose( rspl_type[2] );
     (void) H5Gclose( gads_id );
}

void SCIA_LV1_WR_H5_RSPO( struct param_record param, unsigned int nr_rsp,
			  const struct rsplo_scia *rspo )
{
     hid_t   gads_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   rspo_type[3] = { H5T_NATIVE_FLOAT, H5T_NATIVE_FLOAT, -1 };

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;

     const char *rspo_names[3] = { "ang_esm", "ang_asm", "sensitivity" };
     const size_t rspo_size = sizeof( struct rsplo_scia );
     const size_t rspo_offs[3] = {
	  HOFFSET( struct rsplo_scia, ang_esm ),
	  HOFFSET( struct rsplo_scia, ang_asm ),
	  HOFFSET( struct rsplo_scia, sensitivity )
     };
/*
 * check number of RSPO records
 */
     if ( nr_rsp == 0 ) return;
/*
 * open/create group /GADS
 */
     gads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/GADS" );
     if ( gads_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/GADS" );
/*
 * write RSPO data sets
 */
     adim = SCIENCE_PIXELS;
     rspo_type[2] = H5Tarray_create( H5T_NATIVE_DOUBLE, 1, &adim );

     stat = H5TBmake_table( "rspo", gads_id, "RAD_SENS_OCC",
                            3, nr_rsp, rspo_size, rspo_names,
                            rspo_offs, rspo_type, 1,
                            NULL, compress, rspo );
     if ( stat < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_DATA, "rspo" );
/*
 * close interface
 */
 done:
     (void) H5Tclose( rspo_type[2] );
     (void) H5Gclose( gads_id );
}
