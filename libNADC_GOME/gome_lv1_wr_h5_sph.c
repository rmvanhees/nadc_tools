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

.IDENTifer   GOME_LV1_WR_H5_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write GOME level 1 SPH data

.INPUT/OUTPUT
  call as    GOME_LV1_WR_H5_SPH( param, &sph );

     input:  
             struct param_record param : struct holding user-defined settings
	     struct sph1_gome   *sph  : Specific Product Header data

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   11-Nov-2001	moved to the new Error handling routines, RvH
              2.4   19-Jul-2001	pass structures using pointers, RvH 
             2.3   04-Nov-2000   let this module create its own group, RvH
             2.2   23-Jul-1999   number of "Input Data References" can be one,
                                   RvH
             2.1   23-Jul-1999   Using struct param, RvH
             2.0   15-Jun-1999   Rewritten, RvH
             1.0   20-May-1999   Created by R. M. van Hees
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
void GOME_LV1_WR_H5_SPH( struct param_record param, 
			 const struct sph1_gome *sph )
{
     float   rbuff[3];
     double  dbuff[3];

     hid_t   grp_id;
     hsize_t adim;
/*
 * create group SPH
 */
     grp_id = H5Gcreate( param.hdf_file_id, "/SPH", 
			 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     if ( grp_id < 0 ) NADC_RETURN_ERROR( NADC_ERR_HDF_GRP, "/SPH" );
/*
 * +++++ create/write attributes in the /SPH group
 */
     adim = (hsize_t) strlen( sph->inref[0] );
     NADC_WR_HDF5_Attribute( grp_id, "InputDataReference_1", 
			    H5T_C_S1, 1, &adim, sph->inref[0] );

     if ( (adim = (hsize_t) strlen( sph->inref[1] )) > 0 )
	  NADC_WR_HDF5_Attribute( grp_id, "InputDataReference_2", 
				 H5T_C_S1, 1, &adim, sph->inref[1] );

     adim = (hsize_t) strlen( sph->soft_version );
     NADC_WR_HDF5_Attribute( grp_id, "GDP_SoftwareVersion", 
			    H5T_C_S1, 1, &adim, sph->soft_version );

     adim = (hsize_t) strlen( sph->calib_version );
     NADC_WR_HDF5_Attribute( grp_id, "CalibrationDataVersion", 
			    H5T_C_S1, 1, &adim, sph->calib_version );

     adim = 1;
     NADC_WR_HDF5_Attribute( grp_id, "ProductFormatVersion", 
			    H5T_NATIVE_SHORT, 1, &adim, &sph->prod_version );

     NADC_WR_HDF5_Attribute( grp_id, "OrbitNumber", H5T_NATIVE_UINT, 
			    1, &adim, &sph->time_orbit );

     NADC_WR_HDF5_Attribute( grp_id, "UTC_date", H5T_NATIVE_UINT, 
			    1, &adim, &sph->time_utc_day );
     NADC_WR_HDF5_Attribute( grp_id, "UTC_time", H5T_NATIVE_UINT, 
			    1, &adim, &sph->time_utc_ms );

     NADC_WR_HDF5_Attribute( grp_id, "SatelliteBinaryCounter", 
			    H5T_NATIVE_UINT, 1, &adim, &sph->time_counter );

     NADC_WR_HDF5_Attribute( grp_id, "SatelliteBinaryPeriod", 
			    H5T_NATIVE_UINT, 1, &adim, &sph->time_period );

     NADC_WR_HDF5_Attribute( grp_id, "PMD_EntryPoint", 
			    H5T_NATIVE_SHORT, 1, &adim, &sph->pmd_entry );

     NADC_WR_HDF5_Attribute( grp_id, "SubsetCounterEntryPoint", 
			    H5T_NATIVE_SHORT, 1, &adim, &sph->subset_entry );

     NADC_WR_HDF5_Attribute( grp_id, "IntegrationEntryPoint", 
			    H5T_NATIVE_SHORT, 1, &adim, &sph->intgstat_entry );

     NADC_WR_HDF5_Attribute( grp_id, "PeltierEntryPoint", 
			    H5T_NATIVE_SHORT, 1, &adim, &sph->peltier_entry );

     NADC_WR_HDF5_Attribute( grp_id, "Status_2EntryPoint", 
			    H5T_NATIVE_SHORT, 1, &adim, &sph->status2_entry );

     NADC_WR_HDF5_Attribute( grp_id, "ESOC_UTC_date", 
			    H5T_NATIVE_UINT, 1, &adim, &sph->state_utc_day );

     NADC_WR_HDF5_Attribute( grp_id, "ESOC_UTC_time", 
			    H5T_NATIVE_UINT, 1, &adim, &sph->state_utc_ms );

     NADC_WR_HDF5_Attribute( grp_id, "ESOC_OrbitNumber", 
			    H5T_NATIVE_UINT, 1, &adim, &sph->state_orbit );

     adim = 6;
     NADC_WR_HDF5_Attribute( grp_id, "PMD_ConvertionFactors", 
			    H5T_NATIVE_FLOAT, 1, &adim, &sph->pmd_conv );
     adim = 3;
     rbuff[0] = sph->state_x;
     rbuff[1] = sph->state_y;
     rbuff[2] = sph->state_z;
     NADC_WR_HDF5_Attribute( grp_id, "ESOC_PositionVector", 
			    H5T_NATIVE_FLOAT, 1, &adim, rbuff );
     rbuff[0] = sph->state_dx;
     rbuff[1] = sph->state_dy;
     rbuff[2] = sph->state_dz;
     NADC_WR_HDF5_Attribute( grp_id, "ESOC_VelocityVector", 
			    H5T_NATIVE_FLOAT, 1, &adim, rbuff );
     dbuff[0] = sph->att_yaw;
     dbuff[1] = sph->att_pitch;
     dbuff[2] = sph->att_roll;
     NADC_WR_HDF5_Attribute( grp_id, "ATT_AOCS_Mispointing", 
			    H5T_NATIVE_DOUBLE, 1, &adim, dbuff );
     dbuff[0] = sph->att_dyaw;
     dbuff[1] = sph->att_dpitch;
     dbuff[2] = sph->att_droll;
     NADC_WR_HDF5_Attribute( grp_id, "DATT_AOCS_Mispointing", 
			    H5T_NATIVE_DOUBLE, 1, &adim, dbuff );

     adim = 1;
     NADC_WR_HDF5_Attribute( grp_id, "IATTAttitudeFlag", 
			    H5T_NATIVE_INT, 1, &adim, &sph->att_flag );
     NADC_WR_HDF5_Attribute( grp_id, "StatusGOME_INIT", 
			    H5T_NATIVE_INT, 1, &adim, &sph->att_stat );

     NADC_WR_HDF5_Attribute( grp_id, "MJD50atANC", 
			    H5T_NATIVE_DOUBLE, 1, &adim, &sph->julian );
     NADC_WR_HDF5_Attribute( grp_id, "SemiMajorAxis", 
			    H5T_NATIVE_DOUBLE, 1, &adim, &sph->semi_major );
     NADC_WR_HDF5_Attribute( grp_id, "Excentricity", 
			    H5T_NATIVE_DOUBLE, 1, &adim, &sph->excen );
     NADC_WR_HDF5_Attribute( grp_id, "Inclination", 
			    H5T_NATIVE_DOUBLE, 1, &adim, &sph->incl );
     NADC_WR_HDF5_Attribute( grp_id, "RightAscension", 
			    H5T_NATIVE_DOUBLE, 1, &adim, &sph->right_asc );
     NADC_WR_HDF5_Attribute( grp_id, "ArgumentofPerigee", 
			    H5T_NATIVE_DOUBLE, 1, &adim, &sph->perigee );
     NADC_WR_HDF5_Attribute( grp_id, "MeanAnomaly", 
			    H5T_NATIVE_DOUBLE, 1, &adim, &sph->mn_anom );
/*
 * close interface
 */
     (void) H5Gclose( grp_id );
}
