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

.IDENTifer   GOME_LV2_WR_H5_SPH
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 2 - HDF5
.LANGUAGE    ANSI C
.PURPOSE     define and write GOME level 2 SPH data

.INPUT/OUTPUT
  call as    GOME_LV2_WR_H5_SPH( param, sph );

     input:  
             struct param_record param : struct holding user-defined settings
	     struct sph_gome   *sph  : Specific Product Header data

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      2.0   11-Nov-2001	moved to the new Error handling routines, RvH
              1.2   21-Aug-2001	pass structures using pointers, RvH 
              1.1   06-Nov-2000 let this module create its own group, RvH
              1.0   20-May-1999 copied from GOME_lv1/group_sph.c, RvH
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
void GOME_LV2_WR_H5_SPH( struct param_record param, 
			 const struct sph2_gome *sph )
{
     const char prognm[] = "GOME_LV2_WR_H5_SPH";

     char    cbuff[6 * LVL2_MAX_NMOL];
     short   nr;

     hid_t   grp_id;
     hsize_t adim;
/*
 * create group /SPH
 */
     grp_id = H5Gcreate( param.hdf_file_id, "/SPH", 
			 H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
     if ( grp_id < 0 ) NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_GRP, "/SPH" );
/*
 * +++++ create/write attributes in the /SPH group
 */
     adim = (hsize_t) strlen( sph->inref );
     NADC_WR_HDF5_Attribute( grp_id, "Input Data Reference", 
			    H5T_C_S1, 1, &adim, sph->inref );

     adim = (hsize_t) strlen( sph->soft_version );
     NADC_WR_HDF5_Attribute( grp_id, "GDP Software Version", 
			    H5T_C_S1, 1, &adim, sph->soft_version );

     adim = (hsize_t) strlen( sph->param_version );
     NADC_WR_HDF5_Attribute( grp_id, "GDP File Version", 
			    H5T_C_S1, 1, &adim, sph->param_version );

     adim = (hsize_t) strlen( sph->format_version );
     NADC_WR_HDF5_Attribute( grp_id, "GDP Format Version", 
			    H5T_C_S1, 1, &adim, sph->format_version );
     adim = 1;
     NADC_WR_HDF5_Attribute( grp_id, "Number of Windows", 
			    H5T_NATIVE_SHORT, 1, &adim, &sph->nwin );
     adim = sph->nwin;
     NADC_WR_HDF5_Attribute( grp_id, "Window Pair", 
			    H5T_NATIVE_FLOAT, 1, &adim, sph->window );
     adim = 1;
     NADC_WR_HDF5_Attribute( grp_id, "Number of Molecules", 
			    H5T_NATIVE_SHORT, 1, &adim, &sph->nmol );

     (void) strlcpy( cbuff, sph->mol_name[0], LVL2_MAX_NMOL );
     for ( nr = 1; nr < sph->nmol; nr++ )
	  (void) strlcat( cbuff, sph->mol_name[nr], (6 * LVL2_MAX_NMOL) );
     adim = (hsize_t) strlen( cbuff );
     NADC_WR_HDF5_Attribute( grp_id, "Molecule Pair Names", 
			    H5T_C_S1, 1, &adim, cbuff );
     adim = sph->nmol;
     NADC_WR_HDF5_Attribute( grp_id, "Molecule Pair Index", 
			    H5T_NATIVE_SHORT, 1, &adim, sph->mol_win );
     adim = 1;
     NADC_WR_HDF5_Attribute( grp_id, "Atmosphere Height", 
			    H5T_NATIVE_FLOAT, 1, &adim, &sph->height );
/*
 * close interface
 */
     (void) H5Gclose( grp_id );
}
