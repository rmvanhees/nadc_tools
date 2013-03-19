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

.IDENTifer   SCIA_LV1_WR_H5_PMD
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write SCIAMACHY level 1 PMD data
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_H5_PMD( param, nr_pmd, pmd );
     input:  
             struct param_record param : struct holding user-defined settings
	     unsigned int nr_pmd       : number of PMD data packets
	     struct pmd_scia *pmd      : structure with PMD data packets

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   22-Jan-2004 moved to the NSCA hdf5_hl routines, RvH
              2.1   21-Feb-2002	assign names to MDS0 data, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.0   24-Nov-1999	created by R. M. van Hees 
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

#define NFIELDS    5

static const size_t pmd_size = sizeof( struct pmd_scia );
static const size_t pmd_offs[NFIELDS] = {
     HOFFSET( struct pmd_scia, mjd ),
     HOFFSET( struct pmd_scia, flag_mds ),
     HOFFSET( struct pmd_scia, mds0.packet_hdr ),
     HOFFSET( struct pmd_scia, mds0.data_hdr ),
     HOFFSET( struct pmd_scia, mds0.data_src )
};

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_WR_H5_PMD( struct param_record param, unsigned int nr_pmd,
			 const struct pmd_scia *pmd )
{
     const char prognm[] = "SCIA_LV1_WR_H5_PMD";

     hid_t   ads_id;
     hid_t   pmd_type[NFIELDS];

     const hbool_t compress = (param.flag_deflate == PARAM_SET) ? TRUE : FALSE;
     const char *pmd_names[NFIELDS] = {
          "dsr_time", "attach_flag", "packet_hdr", "data_hdr", "data_src"
     };
/*
 * check number of PMD records
 */
     if ( nr_pmd == 0 ) return;
/*
 * open/create group /ADS
 */
     ads_id = NADC_OPEN_HDF5_Group( param.hdf_file_id, "/ADS" );
     if ( ads_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/ADS" );
/*
 * define data types of the Table-fields
 */
     pmd_type[0] = H5Topen( param.hdf_file_id, "mjd", H5P_DEFAULT );
     pmd_type[1] = H5Tcopy( H5T_NATIVE_UCHAR );
     pmd_type[2] = H5Topen( param.hdf_file_id, "packet_hdr", H5P_DEFAULT );
     pmd_type[3] = H5Topen( param.hdf_file_id, "data_hdr", H5P_DEFAULT );
     pmd_type[4] = H5Topen( param.hdf_file_id, "pmd_src", H5P_DEFAULT );
/*
 * create table
 */
     (void) H5TBmake_table( "pmd", ads_id, "PMD_PACKETS", NFIELDS, nr_pmd,
                            pmd_size, pmd_names, pmd_offs, pmd_type, 
			    nr_pmd, NULL, compress, pmd );
/*
 * close interface
 */
     (void) H5Tclose( pmd_type[0] );
     (void) H5Tclose( pmd_type[1] );
     (void) H5Tclose( pmd_type[2] );
     (void) H5Tclose( pmd_type[3] );
     (void) H5Tclose( pmd_type[4] );
     (void) H5Gclose( ads_id );
}
