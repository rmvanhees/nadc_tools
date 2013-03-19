/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2006 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_PYTABLE_API
.AUTHOR      R.M. van Hees
.KEYWORDS    HDF5/PyTable interface
.LANGUAGE    ANSI C
.PURPOSE     subroutines to easily create/write/read attributes and datasets
.CONTAINS    PYTABLE_open_file, PYTABLE_open_group, PYTABLE_make_array,
             PYTABLE_append_array, PYTABLE_write_array
.RETURNS     status: negative value is returned on failure
.COMMENTS    none
.ENVIRONment none
.VERSION      1.0   01-Nov-2006 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

#define PY_GROUP_CLASS    "GROUP"
#define PY_FORMAT_VERSION "2.0"
#define PY_GROUP_VERSION  "1.0"

#define PY_ARRAY_CLASS    "EARRAY"
#define PY_ARRAY_FLAVOR   "numpy"
#define PY_ARRAY_VERSION  "2.3"

/*+++++++++++++++++++++++++
.IDENTifer   PYTABLE_open_file
.PURPOSE     Open/Create a PyTable file
.INPUT/OUTPUT
  call as    grpID = PYTABLE_open_file( filename, title ); 

     input:
            char   filename[] :   file name 
	    char   title[]    :   title of the PyTable database

.RETURNS     A negative value is returned on failure. 
.COMMENTS    none
-------------------------*/
hid_t PYTABLE_open_file( const char *filename, const char *title )
{
     hid_t   fileID;
/*
 * check if the file exists, if not create it
 */
     H5E_BEGIN_TRY {
	  fileID = H5Fopen( filename, H5F_ACC_RDWR, H5P_DEFAULT );
	  if ( fileID < 0 ) {
	       fileID = H5Fcreate( filename, H5F_ACC_TRUNC, 
				   H5P_DEFAULT, H5P_DEFAULT );
	       if ( fileID < 0 ) return -1;
/*
 * attach PyTables required attributes
 */
	       (void) H5LTset_attribute_string( fileID, "/", "CLASS", 
						PY_GROUP_CLASS );
	       (void) H5LTset_attribute_string( fileID, "/", 
						"PYTABLES_FORMAT_VERSION",
						PY_FORMAT_VERSION );
	       (void) H5LTset_attribute_string( fileID, "/", "TITLE", 
						title );
	       (void) H5LTset_attribute_string( fileID, "/", "VERSION", 
						PY_GROUP_VERSION );
	  }
     } H5E_END_TRY;
/*
 * return id of the file
 */
     return fileID;
}

/*+++++++++++++++++++++++++
.IDENTifer   PYTABLE_open_group
.PURPOSE     Open/Create a group in an exsisting HDF5-file
.INPUT/OUTPUT
  call as    grpID = PYTABLE_open_group( locID, name, sz_hint ); 

     input:
            hid_t   locID  :   HDF5 object id
	    char    name[]  :   name of the group

.RETURNS     A negative value is returned on failure. 
.COMMENTS    none
-------------------------*/
hid_t PYTABLE_open_group( hid_t locID, const char *name )
{
     hid_t   grpID;
/*
 * check if the group exists, if not create it
 */
     H5E_BEGIN_TRY {
	  if ( (grpID = H5Gopen( locID, name, H5P_DEFAULT )) < 0 )
	       grpID = H5Gcreate( locID, name, 
				  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	       (void) H5LTset_attribute_string( locID, name, "CLASS", 
					        PY_GROUP_CLASS );
	       (void) H5LTset_attribute_string( locID, name, 
						"PYTABLES_FORMAT_VERSION",
						PY_FORMAT_VERSION );
	       (void) H5LTset_attribute_string( locID, name, "TITLE", 
						name );
	       (void) H5LTset_attribute_string( locID, name, "VERSION", 
						PY_GROUP_VERSION );
     } H5E_END_TRY;
/*
 * return id of the group
 */
     return grpID;
}

/*+++++++++++++++++++++++++
.IDENTifer   PYTABLE_make_array
.PURPOSE     create extensible HDF5 dataset
.INPUT/OUTPUT
  call as    stat = PYTABLE_make_array( locID, dset_name, title, rank, dims,
			                extdim, typeID, dims_chunk, fill_data,
			                compress, shuffle, fletcher32, buff );
     input:
            hid_t locID           :  HDF5 identifier of file or group
	    char *dset_name       :  name of dataset
	    char *title           :
	    int rank              :  number of dimensions
	    hsize_t *dims         :  size of each dimension
	    int extdim            :  index of expendable dimension
	    hid_t typeID          :  data type (HDF5 identifier)
	    hsize_t *dims_chunk   :  chunk sizes
	    void *fill_data       :  Fill value for data
	    unsigned int compress :  compression level (zero for no compression)
	    bool shuffle          :  shuffel data for better compression
	    bool fletcher32       :  
	    void *buffer          :  buffer with data to write (or NULL)
	    
.RETURNS     A negative value is returned on failure. 
.COMMENTS    none
-------------------------*/
herr_t PYTABLE_make_array( hid_t locID, const char *dset_name, 
			   const char *title, const int rank, 
			   const hsize_t *dims, int extdim, hid_t typeID,
			   const hsize_t *dims_chunk, void *fill_data,
			   unsigned int compress, bool shuffle, 
			   bool fletcher32, const void *buffer )
{
     const char prognm[] = "PYTABLE_make_array";

     register int ni;

     hid_t   dataID = -1, spaceID = -1;
     herr_t  stat;

/* check if the array has to be chunked or not */
     if ( dims_chunk != NULL ) {
	  hid_t   plistID;

	  hsize_t *maxdims = (hsize_t *) malloc( rank * sizeof(hsize_t) );
	  if ( maxdims == NULL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "maxdims" );

	  for ( ni = 0; ni < rank; ni++ ) {
	       if ( ni == extdim )
		    maxdims[ni] = H5S_UNLIMITED;
	       else
		    maxdims[ni] = 
			 dims[ni] < dims_chunk[ni] ? dims_chunk[ni] : dims[ni];
	  }
	  spaceID = H5Screate_simple( rank, dims, maxdims );
	  free( maxdims );
	  if ( spaceID < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_SPACE, "" );

	  /* Modify dataset creation properties, i.e. enable chunking  */
	  plistID = H5Pcreate( H5P_DATASET_CREATE );
	  if ( H5Pset_chunk( plistID, rank, dims_chunk ) < 0 ) goto done;

          /* set the fill value using a struct as the data type */
	  if ( fill_data != NULL 
	       && H5Pset_fill_value( plistID, typeID, fill_data ) < 0 )
	       goto done;

          /* dataset creation property list is modified to use */
          /* fletcher must be first */
	  if ( fletcher32 ) {
	       if ( H5Pset_fletcher32( plistID ) < 0 ) goto done;
	  }
          /* then shuffle */
	  if ( shuffle ) {
	       if ( H5Pset_shuffle( plistID ) < 0 ) goto done;
	  }
          /* finally compression */
	  if ( compress > 0 ) {
	       if ( H5Pset_deflate( plistID, compress ) < 0 ) goto done;
	  }
          /* create the (chunked) dataset */
	  dataID = H5Dcreate( locID, dset_name, typeID, spaceID, 
			      H5P_DEFAULT, plistID, H5P_DEFAULT );
	  if ( dataID < 0 ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, dset_name );

          /* end access to the property list */
	  if ( H5Pclose( plistID ) < 0 ) goto done;
     } else {
	  spaceID = H5Screate_simple( rank, dims, NULL );
	  if ( spaceID < 0 ) return -1;

          /* create the dataset (not chunked) */
	  dataID = H5Dcreate( locID, dset_name, typeID, spaceID, 
			      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	  if ( dataID < 0 ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, dset_name );
     }
/*
 * write the data
 */
     stat = H5Dwrite( dataID, typeID, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer );
     if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_WR, "" );

     (void) H5Dclose( dataID );
     (void) H5Sclose( spaceID );
/*
 * Set the conforming array attributes
 *
 * attach the CLASS attribute
 */
     (void) H5LTset_attribute_string( locID, dset_name, "CLASS", 
				      PY_ARRAY_CLASS );

/* attach the EXTDIM attribute in case of enlargeable arrays */
     (void) H5LTset_attribute_int( locID, dset_name, "EXTDIM", &extdim, 1 );

/* attach the FLAVOR attribute */
     (void) H5LTset_attribute_string( locID, dset_name, "FLAVOR", 
				      PY_ARRAY_FLAVOR );
/* attach the VERSION attribute */
     (void) H5LTset_attribute_string( locID, dset_name, "VERSION", 
				      PY_ARRAY_VERSION );

/* attach the TITLE attribute */
     (void) H5LTset_attribute_string( locID, dset_name, "TITLE", title );

     return 0;
 done:
     if ( dataID > 0 ) (void) H5Dclose( dataID );
     if ( spaceID > 0 ) (void) H5Sclose( spaceID );
     return -1;
}

/*+++++++++++++++++++++++++
.IDENTifer   PYTABLE_append_array
.PURPOSE     append data to HDF5 dataset, extending the dimension ''extdim''
.INPUT/OUTPUT
  call as    stat = PYTABLE_append_array( locID, dset_name, 
                                          extdim, count, buffer );

     input:
            hid_t locID      :   HDF5 identifier of file or group
	    char  *dset_name :   name of dataset
	    int   extdim     :   dimension to extend
            int   count      :   number of arrays to write
	    void  *buffer    :   data to write
	    
.RETURNS     A negative value is returned on failure. 
.COMMENTS    none
-------------------------*/
herr_t PYTABLE_append_array( hid_t locID, const char *dset_name, 
			     int extdim, int count, const void *buffer )
{
     int      rank;

     hid_t    dataID;
     hid_t    spaceID = -1;
     hid_t    mem_spaceID = -1;
     hid_t    typeID = -1;
     hsize_t  *dims = NULL;
     hsize_t  *dims_ext = NULL;
     hsize_t  *offset = NULL;
     herr_t   stat;

/* open the dataset. */
     if ( (dataID = H5Dopen( locID, dset_name, H5P_DEFAULT )) < 0 ) return -1;

/* get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 ) goto done;

/* get rank */
     if ( (rank = H5Sget_simple_extent_ndims( spaceID )) < 0 ) goto done;

/* get dimensions */
     dims = (hsize_t *) malloc( rank * sizeof(hsize_t) );
     dims_ext = (hsize_t *) malloc( rank * sizeof(hsize_t) );
     offset = (hsize_t *) calloc( rank, sizeof(hsize_t) );
     if ( H5Sget_simple_extent_dims( spaceID, dims, NULL ) < 0 )
       goto done;
     offset[extdim] = dims[extdim];
     (void) memcpy( dims_ext, dims, rank * sizeof(hsize_t) );
     dims_ext[extdim] = count;
     dims[extdim] += count;

/* terminate access to the dataspace */
     if ( H5Sclose( spaceID ) < 0 ) goto done;

/* extend the dataset */
     if ( H5Dset_extent( dataID, dims ) < 0 ) goto done;

/* select a hyperslab */
     if ( (spaceID = H5Dget_space( dataID )) < 0 ) goto done;
     stat = H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, offset, NULL,
				 dims_ext, NULL );
     free( dims );
     free( offset );
     if ( stat < 0 ) goto done;

/* define memory space */
     if ( (mem_spaceID = H5Screate_simple( rank, dims_ext, NULL )) < 0 )
       goto done;
     free( dims_ext );

/* get an identifier for the datatype. */
     if ( (typeID = H5Dget_type( dataID )) < 0 ) goto done;

/* write the data to the hyperslab */
     stat = H5Dwrite( dataID, typeID, mem_spaceID, spaceID, H5P_DEFAULT, 
		      buffer );
     if ( stat < 0 ) goto done;

/* end access to the dataset */
     if ( H5Dclose( dataID ) ) goto done;

/* terminate access to the datatype */
     if ( H5Tclose( typeID ) < 0 ) goto done;

/* terminate access to the dataspace */
     if ( H5Sclose( mem_spaceID ) < 0 ) goto done;
     if ( H5Sclose( spaceID ) < 0 ) goto done;
     return 0;
 done:
     if ( dims     != 0 ) free( dims );
     if ( dims_ext != 0 ) free( dims_ext );
     if ( offset   != 0 ) free( offset );
     if ( typeID > 0 ) (void) H5Tclose( typeID );
     if ( spaceID > 0 ) (void) H5Sclose( spaceID );
     if ( mem_spaceID > 0 ) (void) H5Sclose( mem_spaceID );
     if ( dataID > 0 ) (void) H5Dclose( dataID );
     return -1;
}

/*+++++++++++++++++++++++++
.IDENTifer   PYTABLE_write_records
.PURPOSE     Write records to an HDF5 array
.INPUT/OUTPUT
  call as    stat = PYTABLE_write_records( locID, dset_name, start, step,
			                   count, buffer );
     input:
            hid_t locID      :  HDF5 identifier of file or group
	    char *dset_name  :  name of dataset
	    hsize_t *start   :  index of first row to overwrite
	    hsize_t *step    :  
	    hsize_t *count   :  number of rows to write
	    void *buffer     :  data to write
	    
.RETURNS     A negative value is returned on failure. 
.COMMENTS    none
-------------------------*/
herr_t PYTABLE_write_records( hid_t locID, const char *dset_name, 
			      hsize_t *start, hsize_t *step,
			      hsize_t *count, const void *buffer )
{
     int      rank;

     hid_t    dataID;
     hid_t    spaceID = -1;
     hid_t    mem_spaceID = -1;
     hid_t    typeID = -1;

/* open the dataset. */
     if ( (dataID = H5Dopen( locID, dset_name, H5P_DEFAULT )) < 0 )
	  return -1;

/* get the dataspace handle */
     if ( (spaceID = H5Dget_space( dataID )) < 0 )
	  goto done;

/* get rank */
     if ( (rank = H5Sget_simple_extent_ndims( spaceID )) <= 0 )
	  goto done;

/* create a simple memory data space */
     if ( (mem_spaceID = H5Screate_simple( rank, count, NULL )) < 0 )
	  goto done;

/* define a hyperslab in the dataset */
     if ( H5Sselect_hyperslab( spaceID, H5S_SELECT_SET, start,
			       step, count, NULL ) < 0 )
	  goto done;

/* get an identifier for the datatype. */
     if ( (typeID = H5Dget_type( dataID )) < 0 ) goto done;

/* write data to hyperslap */
     if ( H5Dwrite( dataID, typeID, mem_spaceID, spaceID, 
		    H5P_DEFAULT, buffer ) < 0 )
	  goto done;

/* terminate access to the datatype */
     if ( H5Tclose( typeID ) < 0 ) goto done;

/* end access to the dataset */
     if ( H5Dclose( dataID ) ) goto done;

/* terminate access to the dataspace */
     if ( H5Sclose( mem_spaceID ) < 0 ) goto done;
     if ( H5Sclose( spaceID ) < 0 ) goto done;

     return 0;
 done:
     if ( typeID > 0 ) (void) H5Tclose( typeID );
     if ( spaceID > 0 ) (void) H5Sclose( spaceID );
     if ( mem_spaceID > 0 ) (void) H5Sclose( mem_spaceID );
     if ( dataID > 0 ) (void) H5Dclose( dataID );
     return -1;
}
