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

.IDENTifer   NADC_HDF5_API
.AUTHOR      R.M. van Hees
.KEYWORDS    HDF5 interface
.LANGUAGE    ANSI C
.PURPOSE     subroutines to easily create/write/read attributes and datasets
.CONTAINS    NADC_OPEN_HDF5_Group, 
             EXISTS_HDF5_Attribute, EXISTS_HDF5_Dataset,
             NADC_WR_HDF5_Attribute, 
             NADC_RD_HDF5_Dataset, NADC_WR_HDF5_Dataset, 
             NADC_WR_HDF5_Vlen_Dataset, 
             Create_HDF5_NADC_FILE
.RETURNS     nothing: modifies global error status
.COMMENTS    none
.ENVIRONment none
.VERSION      5.1   23-Sep-2003	removed fill_value parameter from
                                NADC_WR_HDF5_Dataset and 
				NADC_WR_HDF5_Vlen_Dataset, and apply shuffle,
				to improve compression, RvH
              5.0   23-Sep-2003	renamed all modules, fixed some typos, RvH
              4.7   16-Sep-2003	added NADC_RD_HDF5_Dataset, RvH
              4.6   18-Aug-2003	use HDF5 macros: H5E_BEGIN_TRY/H5E_END_TRY, RvH
              4.5   10-Nov-2002	use new CalibStr routine, RvH
              4.4   02-Aug-2002	synchronized History with command-line 
                                options, RvH
              4.3   12-Jul-2002	updated history with SCIA options, RvH 
              4.2   18-Apr-2002	added EXISTS_HDF5_Dataset, RvH 
              4.1   18-Apr-2002	added EXISTS_HDF5_Attribute, RvH
              4.0   08-Nov-2001	moved to new Error handling routines, RvH 
              3.2   18-Sep-2001 NADC_WR_HDF5_Vlen_Dataset reclaims memory
                                  of variable dataset, RvH
              3.1   01-May-2001 adapted new output filename scheme, RvH
              3.0   27-Mar-2001 removed redundent routines
                                 replaced by Create_HDF5_NADC_FILE, RvH
              2.3   13-Jan-2001 Added CRE_GROUPS_SCIA_2, RvH
              2.2   12-Jan-2001 Added NADC_WR_HDF5_Vlen_Dataset, RvH
              2.1   02-Jan-2001 Added/Changed "History" attributes, RvH
              2.0   09-Nov-2000 Made "history" processor specific, RvH
              1.8   09-Nov-2000 Changed name of history attributes, RvH
              1.7   06-Nov-2000 Added NADC_OPEN_HDF5_Group, RvH
              1.6   22-Feb-2000 Renamed NADC_Params -> HISTORY, RvH
              1.5   31-Aug-1999 Added global "Processing Date", RvH
              1.4   18-Aug-1999 Added WRITE_GRP_NADC_Params, RvH
              1.3   12-Aug-1999 Added CRE_GROUPS_SCIA_1, RvH
              1.2   03-Aug-1999 Added CRE_GROUPS_GOME_2, RvH
              1.1   23-Jul-1999 Added CRE_GROUPS_GOME_1, RvH
              1.0   01-Jul-1999 Created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function(s) +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   NADC_OPEN_HDF5_Group
.PURPOSE     Open/Create a group in an exsisting HDF5-file

.INPUT/OUTPUT
  call as    grp_id = NADC_OPEN_HDF5_Group( loc_id, name ); 

     input:
            hid_t   loc_id  :   HDF5 object id
	    char    name[]  :   name of the group

.RETURNS     A negative value is returned on failure. 
.COMMENTS    none
-------------------------*/
hid_t NADC_OPEN_HDF5_Group( hid_t loc_id, const char *name )
{
     hid_t   grp_id;
/*
 * check if the group exists, if not create it
 */
     H5E_BEGIN_TRY {
	  if ( (grp_id = H5Gopen( loc_id, name, H5P_DEFAULT )) < 0 )
	       grp_id = H5Gcreate( loc_id, name, H5P_DEFAULT, H5P_DEFAULT,
				   H5P_DEFAULT );
     } H5E_END_TRY;
/*
 * return id of the group
 */
     return grp_id;
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_WR_HDF5_Attribute
.PURPOSE     Create/write an attribute which is attached to an object 
.INPUT/OUTPUT
  call as    NADC_WR_HDF5_Attribute( loc_id, name, type_id, rank, dims, data );
     input:
            hid_t   loc_id  :   HDF5 object id
	    char    name[]  :   name of the attribute
	    hid_t   type_id :   data type of the attribute's data
	    int     rank    :   number of dimensions
	    hsize_t dims[]  :   dimension specification
	    void    *data   :   data for the attribute

.RETURNS     nothing: modifies global error status
.COMMENTS    none
-------------------------*/
void NADC_WR_HDF5_Attribute( hid_t loc_id, const char name[], hid_t type_id,
			     int rank, const hsize_t dims[], const void *data )
{
     hid_t   att_id;
     hid_t   space_id;

     if ( type_id == H5T_C_S1 ) {
	  hid_t atype = H5Tcopy( type_id );

	  space_id = H5Screate( H5S_SCALAR );
	  (void) H5Tset_size( atype, (size_t) dims[0] );
	  att_id = H5Acreate( loc_id, name, atype, space_id, 
			      H5P_DEFAULT, H5P_DEFAULT );
	  if ( att_id < 0 ) 
	       NADC_RETURN_ERROR( NADC_ERR_HDF_ATTR, name );
	  if ( H5Awrite( att_id, atype, data ) < 0 )
	       NADC_RETURN_ERROR( NADC_ERR_HDF_ATTR, name );
	  (void) H5Tclose( atype );
     } else {
	  if ( rank == 1 && dims[0] == 1 ) {
	       space_id = H5Screate( H5S_SCALAR );
	       att_id = H5Acreate( loc_id, name, type_id, space_id, 
				   H5P_DEFAULT, H5P_DEFAULT );
	  } else {
	       space_id = H5Screate_simple( rank, dims, NULL );
	       att_id = H5Acreate( loc_id, name, type_id, space_id, 
				   H5P_DEFAULT, H5P_DEFAULT );
	  }
	  if ( att_id < 0 ) 
	       NADC_RETURN_ERROR( NADC_ERR_HDF_ATTR, name );
	  if ( H5Awrite( att_id, type_id, data ) < 0 )
	       NADC_RETURN_ERROR( NADC_ERR_HDF_ATTR, name );
     }
     if ( H5Sclose( space_id ) < 0 )
	  NADC_RETURN_ERROR( NADC_ERR_HDF_SPACE, name );
     if ( H5Aclose( att_id ) < 0 )
	  NADC_RETURN_ERROR( NADC_ERR_HDF_ATTR, name );
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_RD_HDF5_Dataset
.PURPOSE     read a HDF5 dataset which is attached to loc_id
.INPUT/OUTPUT
  call as    NADC_RD_HDF5_Dataset( loc_id, name, type_id, &rank, dims, &data );
     input:
            hid_t   loc_id      :   HDF5 object id
	    char    name[]      :   name of the dataset
	    hid_t   type_id     :   data type of the attribute's data
    output:
    	    int     *rank       :   number of dimensions
	    hsize_t dims[]      :   dimension specification
	    void    **data_out  :   data for the dataset

.RETURNS     nothing: modifies global error status
.COMMENTS    none
-------------------------*/
void NADC_RD_HDF5_Dataset( hid_t loc_id, const char *name, hid_t type_out_id,
			   int *rank, hsize_t *dims, void **data_out )
{
     register int ni;

     hid_t   data_id;
     hid_t   type_id;
     hid_t   space_id;
     hid_t   stat;
     size_t  nrbyte, nrpix;

     void *data;
/*
 * open dataset
 */
     if ( (data_id = H5Dopen( loc_id, name, H5P_DEFAULT )) < 0 )
	  NADC_RETURN_ERROR( NADC_ERR_HDF_DATA, name );
/*
 * get dataset rank and dimension(s)
 */
     if ( (space_id = H5Dget_space( data_id )) < 0 )
	  NADC_RETURN_ERROR( NADC_ERR_HDF_SPACE, name );
     *rank = H5Sget_simple_extent_ndims( space_id );
     (void) H5Sget_simple_extent_dims( space_id, dims, NULL );
     (void) H5Sclose( space_id );
     nrpix = 1;
     for ( ni = 0; ni < *rank; ni++ ) nrpix *= (size_t) dims[ni];
/*
 * get dataset type and size
 */
     if ( (type_id = H5Dget_type( data_id )) < 0 )
	  NADC_RETURN_ERROR( NADC_ERR_HDF_DTYPE, name );
     nrbyte = nrpix * H5Tget_size( type_id );
     (void) H5Tclose( type_id );
/*
 * allocate memory to store dataset
 */
     if ( (data_out[0] = data = (void *) malloc( nrbyte )) == NULL ) 
	  NADC_RETURN_ERROR( NADC_ERR_ALLOC, "data" );
/*
 * read all elements of dataset
 */
     stat = H5Dread( data_id, type_out_id, 
		     H5S_ALL, H5S_ALL, H5P_DEFAULT, data );
     if ( stat < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_DATA, name );
/*
 * close dataset
 */
     if ( H5Dclose( data_id ) < 0 ) 
	  NADC_RETURN_ERROR( NADC_ERR_HDF_DATA, name );
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_WR_HDF5_Dataset
.PURPOSE     Create/write a dataset which is attached to an object 
.INPUT/OUTPUT
  call as    NADC_WR_HDF5_Dataset( compress, loc_id, name, 
                                   type_id, rank, dims, data );
     input:
            hbool_t compress    :   sets the compression method to H5Z_DEFLATE
            hid_t   loc_id      :   HDF5 object id
	    char    name[]      :   name of the dataset
	    hid_t   type_id     :   data type of the attribute's data
	    int     rank        :   number of dimensions
	    hsize_t dims[]      :   dimension specification
	    void    *data       :   data for the dataset

.RETURNS     nothing: modifies global error status
.COMMENTS    none
-------------------------*/
void NADC_WR_HDF5_Dataset( hbool_t compress, hid_t loc_id, const char name[], 
			   hid_t type_id, int rank, const hsize_t dims[], 
			   const void *data )
{
     hid_t   data_id;
     hid_t   space_id;
     hid_t   plist;
     hid_t   stat;
/*
 * remove useless dimensions
 */
     while ( rank > 1 && dims[0] == 1 ) {
	  rank--;
	  dims++;
     }
/*
 * create data set
 */
     if ( rank == 1 && dims[0] == 1 ) {
	  space_id = H5Screate( H5S_SCALAR );
	  data_id = H5Dcreate( loc_id, name, type_id, space_id, 
			       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     } else {
	  space_id = H5Screate_simple( rank, dims, NULL );
	  plist = H5Pcreate( H5P_DATASET_CREATE );
	  if ( compress ) {
	       if ( H5Zfilter_avail( H5Z_FILTER_DEFLATE ) > 0 ) {
		    (void) H5Pset_chunk( plist, rank, dims );
		    (void) H5Pset_shuffle( plist );
		    (void) H5Pset_deflate( plist, 6 );
	       } else {
		    NADC_ERROR( NADC_ERR_WARN, 
				"no compression available in HDF5 library" );
	       }
	  }
	  data_id = H5Dcreate( loc_id, name, type_id, space_id, 
			       H5P_DEFAULT, plist, H5P_DEFAULT );
	  (void) H5Pclose( plist );
     }
     if ( data_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_DATA, name );
     (void) H5Sclose( space_id );
/*
 * write dataset to file
 */
     stat = H5Dwrite( data_id, type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, data );
     if ( stat < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_DATA, name );
/*
 * close dataset
 */
     if ( H5Dclose( data_id ) < 0 ) 
	  NADC_RETURN_ERROR( NADC_ERR_HDF_DATA, name );
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_WR_HDF5_Vlen_Dataset
.PURPOSE     Create/write a variable length HDF5 dataset
.INPUT/OUTPUT
  call as    NADC_WR_HDF5_Vlen_Dataset( compress, loc_id, name, type_id, 
                                        rank, dims, fill_value, vdata );
     input:
            hbool_t compress    :   sets the compression method to H5Z_DEFLATE
            hid_t   loc_id      :   HDF5 object id
	    char    name[]      :   name of the dataset
	    hid_t   type_id     :   data type of the attribute's data
	    int     rank        :   number of dimensions
	    hsize_t dims[]      :   dimension specification
     in/output:
	    void    *vdata      :   variable length data for the dataset
	                            at return the memory buffers are reclaimed

.RETURNS     nothing: modifies global error status
.COMMENTS    the memory buffers of "vdata" are reclaimed
-------------------------*/
void NADC_WR_HDF5_Vlen_Dataset( hbool_t compress, hid_t loc_id, 
				const char name[], hid_t type_id, 
				int rank, const hsize_t dims[], void *vdata )
{
     hid_t   data_id;
     hid_t   vtype_id;
     hid_t   space_id;
     hid_t   plist;
     hid_t   stat;

     if ( rank == 1 && dims[0] == 1 ) {
/*
 * create data set
 */
	  space_id = H5Screate( H5S_SCALAR );
	  data_id = H5Dcreate( loc_id, name, type_id, space_id, 
			       H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	  if ( data_id < 0 ) 
	       NADC_RETURN_ERROR( NADC_ERR_HDF_DATA, name );
/*
 * write dataset to file
 */
	  stat = H5Dwrite( data_id, type_id, 
			   H5S_ALL, H5S_ALL, H5P_DEFAULT, vdata );
	  if ( stat < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_DATA, name );
	  free( vdata );
	  if ( H5Sclose( space_id ) < 0 )
	       NADC_RETURN_ERROR( NADC_ERR_HDF_SPACE, name );
	  if ( H5Dclose( data_id ) < 0 ) 
	       NADC_RETURN_ERROR( NADC_ERR_HDF_DATA, name );
     } else {
/*
 * create data set
 */
	  space_id = H5Screate_simple( rank, dims, NULL );
	  vtype_id = H5Tvlen_create( type_id );
	  plist = H5Pcreate( H5P_DATASET_CREATE );
	  if ( compress ) {
	       (void) H5Pset_chunk( plist, rank, dims );
	       (void) H5Pset_shuffle( plist );
	       (void) H5Pset_deflate( plist, 6 );
	  }
	  data_id = H5Dcreate( loc_id, name, vtype_id, space_id, 
			       H5P_DEFAULT, plist, H5P_DEFAULT );
	  if ( data_id < 0 ) 
	       NADC_RETURN_ERROR( NADC_ERR_HDF_DATA, name );
	  (void) H5Pclose( plist );
/*
 * write dataset to file
 */
	  stat = H5Dwrite( data_id, vtype_id, 
			   H5S_ALL, H5S_ALL, H5P_DEFAULT, vdata );
	  if ( stat < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_DATA, name );
/*
 * free allocated memory
 */
	  stat = H5Dvlen_reclaim( vtype_id, space_id, H5P_DEFAULT, vdata );
	  if ( stat < 0 ) 
	       NADC_RETURN_ERROR( NADC_ERR_HDF_SPACE, name );
	  free( vdata );
/*
 * close variable dataset and data type
 */
	  if ( H5Sclose( space_id ) < 0 )
	       NADC_RETURN_ERROR( NADC_ERR_HDF_SPACE, name );
	  if ( H5Dclose( data_id ) < 0 ) 
	       NADC_RETURN_ERROR( NADC_ERR_HDF_DATA, name );
	  if (  H5Tclose( vtype_id ) < 0 )
	       NADC_RETURN_ERROR( NADC_ERR_HDF_DTYPE, name );
     }
}
