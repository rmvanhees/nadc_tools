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

.IDENTifer   SCIA_LV1_WR_H5_ASFP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 ASFP data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_ASFP( param, nr_asfp, asfp );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned int nr_asfp      : number of ASFP parameters
	     struct asfp_scia *asfp    : small aperture slit function 
                                         parameters
.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   21-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH
              1.0   19-Dec-2000	Created by R. M. van Hees 
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

#define NFIELDS    4

static const size_t asfp_size = sizeof( struct asfp_scia );
static const size_t asfp_offs[NFIELDS] = {
     HOFFSET( struct asfp_scia, pix_pos_slit_fun ),
     HOFFSET( struct asfp_scia, type_slit_fun ),
     HOFFSET( struct asfp_scia, fwhm_slit_fun ),
     HOFFSET( struct asfp_scia, f_voi_fwhm_gauss )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_ASFP( struct param_record param, unsigned int nr_asfp,
			  const struct asfp_scia *asfp )
{
     hid_t   gads_id;

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;

     const char *asfp_names[NFIELDS] = {
          "pix_pos_slit_fun", "type_slit_fun", 
	  "fwhm_slit_fun", "f_voi_fwhm_gaus"
     };
#if !defined(__mips) && !defined (__hpux)
     const hid_t asfp_type[NFIELDS] = {
	  H5T_NATIVE_USHORT, H5T_NATIVE_UCHAR, 
	  H5T_NATIVE_FLOAT, H5T_NATIVE_FLOAT
     };
#else
     hid_t asfp_type[NFIELDS];

     asfp_type[0] = H5T_NATIVE_USHORT;
     asfp_type[1] = H5T_NATIVE_UCHAR;
     asfp_type[2] = H5T_NATIVE_FLOAT;
     asfp_type[3] = H5T_NATIVE_FLOAT;
#endif
/*
 * check number of ASFP records
 */
     if ( nr_asfp == 0 ) return;
/*
 * open/create group /GADS
 */
     gads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/GADS" );
     if ( gads_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/GADS" );
/*
 * create table
 */
     (void) H5TBmake_table( "asfp", gads_id, "SMALL_AP_SLIT_FUNCTION", 
			    NFIELDS, nr_asfp, asfp_size, asfp_names, 
			    asfp_offs, asfp_type, nr_asfp, NULL, compress, 
			    asfp );
/*
 * close interface
 */
     (void) H5Gclose( gads_id );
}
