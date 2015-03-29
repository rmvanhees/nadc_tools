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

.IDENTifer   GOME_WR_H5_PIR
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write GOME level 1 PIR data
.INPUT/OUTPUT
  call as    GOME_WR_H5_PIR( param, &pir );
  input: 
             struct param_record param : struct holding user-defined settings
	     struct pir_gome   *pir  : Product Identifier Record data

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   11-Nov-2001	moved to the new Error handling routines, RvH
              2.0   21-Aug-2001	combined level 1 and 2 modules, RvH 
	      1.3   19-Jul-2001 pass structures using pointers, RvH
	      1.2   04-Nov-2000 let this module create its own group, RvH
	      1.1   23-Jul-1999 using struct param, RvH
	      1.0   15-Jun-1999 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <string.h>

#include <hdf5.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_WR_H5_PIR( struct param_record param, const struct pir_gome *pir )
{
     hid_t   grp_id;
     hsize_t adim;
/*
 * create group PIR
 */
     grp_id = H5Gcreate( param.hdf_file_id, "/PIR", 
			 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     if ( grp_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/PIR" );
/*
 * +++++ create/write attributes in the /PIR group
 */
     adim = (hsize_t) strlen( pir->mission );
     NADC_WR_HDF5_Attribute( grp_id, "MissionID", H5T_C_S1, 
			    1, &adim, pir->mission );
     adim = (hsize_t) strlen( pir->sensor );
     NADC_WR_HDF5_Attribute( grp_id, "SensorIdentifier", H5T_C_S1,
			    1, &adim, pir->sensor );
     adim = (hsize_t) strlen( pir->orbit );
     NADC_WR_HDF5_Attribute( grp_id, "StartOrbitNumber", H5T_C_S1,
			    1, &adim, pir->orbit );
     adim = (hsize_t) strlen( pir->nr_orbit );
     NADC_WR_HDF5_Attribute( grp_id, "OrbitNumber", H5T_C_S1,
			    1, &adim, pir->nr_orbit );
     adim = (hsize_t) strlen( pir->acquis );
     NADC_WR_HDF5_Attribute( grp_id, "AcquisitionFacility", H5T_C_S1,
			    1, &adim, pir->acquis );
     adim = (hsize_t) strlen( pir->product );
     NADC_WR_HDF5_Attribute( grp_id, "ProductType", H5T_C_S1,
			    1, &adim, pir->product );
     adim = (hsize_t) strlen( pir->proc_id );
     NADC_WR_HDF5_Attribute( grp_id, "ProcessingFacility", H5T_C_S1,
			    1, &adim, pir->proc_id );
     adim = (hsize_t) strlen( pir->proc_date );
     NADC_WR_HDF5_Attribute( grp_id, "ProcessingDate", H5T_C_S1,
			    1, &adim, pir->proc_date );
     adim = (hsize_t) strlen( pir->proc_time );
     NADC_WR_HDF5_Attribute( grp_id, "ProcessingTime", H5T_C_S1,
			    1, &adim, pir->proc_time );
/*
 * close the interface
 */
     (void) H5Gclose( grp_id );
}
