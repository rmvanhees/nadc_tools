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

.IDENTifer   SCIA_WR_H5_LADS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1b/2 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1b/2 LADS data
.INPUT/OUTPUT
  call as    SCIA_WR_H5_LADS( param, nr_lads, lads );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned int nr_lads      : number of Geolocation of States
	     struct lads_scia *lads    : Geolocation of States

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.1   07-Dec-2005	removed delaration of unused variables, RvH
              3.0   13-Oct-2003	moved to the NSCA hdf5_hl routines, RvH
              2.1   03-Sep-2002	moved DSD-data to subgroup ADS, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.1   13-Sep-2001	works for both level 1b and level 2, RvH 
              1.0   18-Nov-1999 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <string.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_COMMON
#include <nadc_scia.h>

#define NFIELDS    3
#define TBL_NAME   "GEOLOCATION"

static const size_t lads_size = sizeof( struct lads_scia );
static const size_t lads_offs[NFIELDS] = {
     HOFFSET( struct lads_scia, mjd ),
     HOFFSET( struct lads_scia, flag_mds ),
     HOFFSET( struct lads_scia, corner )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int SCIA_RD_H5_LADS( struct param_record param, 
			      struct lads_scia *lads )
     /*@globals lads_size, lads_offs@*/
{
     hid_t   ads_id;
     hsize_t nfields, num_lads;

     const size_t lads_sizes[NFIELDS] = {
          sizeof( lads->mjd ),
          sizeof( lads->flag_mds ),
          sizeof( lads->corner )
     };
/*
 * create/open group /ADS/LADS
 */
     ads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/ADS" );
     if ( ads_id < 0 ) NADC_GOTO_ERROR( NADC_ERR_HDF_GRP, "/ADS" );
/*
 * read info_h5 records
 */
     (void) H5TBget_table_info( ads_id, TBL_NAME, &nfields, &num_lads );
     (void) H5TBread_table( ads_id, TBL_NAME, lads_size, lads_offs, 
                            lads_sizes, lads );
     (void) H5Gclose( ads_id );

     return (unsigned int) num_lads;
 done:
     return 0u;
}


void SCIA_WR_H5_LADS( struct param_record param, unsigned int nr_lads,
		      const struct lads_scia *lads )
     /*@globals lads_size, lads_offs@*/
{
     hid_t   ads_id;
     hsize_t adim;
     hid_t   lads_type[NFIELDS];
     hid_t   type_id, temp_type00, temp_type01;

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;
     const char *lads_names[NFIELDS] = {
	  "dsr_time", "attach_flag", "corner_grd"
     };
/*
 * check number of LADS records
 */
     if ( nr_lads == 0 ) return;
/*
 * create/open group /ADS
 */
     ads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/ADS" );
     if ( ads_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/ADS" );
/*
 * define user-defined data types of the Table-fields
 */
     temp_type00 = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     adim = NUM_CORNERS;
     type_id = H5Topen( param.hdf_file_id, "coord", H5P_DEFAULT );
     temp_type01 = H5Tarray_create( type_id, 1, &adim );

     lads_type[0] = temp_type00;
     lads_type[1] = H5T_NATIVE_UCHAR;
     lads_type[2] = temp_type01;
/*
 * create table
 */
     (void) H5TBmake_table( "lads", ads_id, TBL_NAME, NFIELDS,
			    nr_lads, lads_size, lads_names, lads_offs,
			    lads_type, nr_lads, NULL, compress, lads );
/*
 * close interface
 */
     (void) H5Tclose( type_id );
     (void) H5Tclose( temp_type00 );
     (void) H5Tclose( temp_type01 );
     (void) H5Gclose( ads_id );
}
