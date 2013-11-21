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

.IDENTifer   GOME_CRE_H5_FILE
.AUTHOR      R.M. van Hees
.KEYWORDS    HDF5 interface
.LANGUAGE    ANSI C
.PURPOSE     Create HDF5 file for GOME level 1 & 2 products
.INPUT/OUTPUT
  call as    GOME_CRE_H5_FILE( instument, param );
     input:
            int  instrument           : flag for Instrument & Product
            struct param_record param : command-line parameters

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
#include <nadc_gome.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   WRITE_HDF5_HISTORY
.PURPOSE     Write attribute in group holding user-defined settings
.INPUT/OUTPUT
  call as    WRITE_HDF5_HISTORY( file_id, instrument, param )
     input:
            hid_t  file_id            :   HDF5 file ID
            hid_t  instrument         :   flag for Instrument & Product
	    struct param_record param :   struct holding user-defined settings

.RETURNS     nothing: modifies global error status
.COMMENTS    none
-------------------------*/
static
void WRITE_HDF5_HISTORY( hid_t file_id, hid_t instrument, 
			 const struct param_record param )
{
     char    cbuff[MAX_STRING_LENGTH];
     char    string[SHORT_STRING_LENGTH];
     time_t  tp[1];

     unsigned int majnum, minnum, relnum;
/*
 * +++++ create/write attributes in root
 */
     (void) H5LTset_attribute_string( file_id, "/", "InputFilename", 
				      param.infile );
     (void) H5LTset_attribute_string( file_id, "/", "OutputFilename", 
				      param.hdf5_name );
/*
 * compression flag     
 */
     if ( param.flag_deflate == PARAM_SET )
	  (void) H5LTset_attribute_string( file_id, "/", "Compression", 
					   "TRUE" );
     else
	  (void) H5LTset_attribute_string( file_id, "/", "Compression", 
					   "FALSE" );
/*
 * write program name/version and HDF5 library version
 */
     (void) H5LTset_attribute_string( file_id, "/", "Processor", 
				      param.program );
     GOME_WR_H5_VERSION( file_id );
     (void) H5get_libversion( &majnum, &minnum, &relnum );
     (void) snprintf( cbuff, MAX_STRING_LENGTH, 
		      "version %-u.%-u release %-u",
		      majnum, minnum, relnum );
     (void) H5LTset_attribute_string( file_id, "/", "HDF5", cbuff );
/*
 * store time of creation
 */
     (void) time( tp );
     (void) strcpy( cbuff, ctime( tp ) );
     cbuff[strlen(cbuff)-1] = '\0';
     (void) H5LTset_attribute_string( file_id, "/", "ProcessingDate", cbuff );
/*
 * ----- General options
 */
/*
 * geolocation (bounding box)
 */
     (void) H5LTset_attribute_float( file_id, "/", "Latitude", 
				     param.geo_lat, 2 );
     (void) H5LTset_attribute_float( file_id, "/", "Longitude", 
				     param.geo_lon, 2 );
     if ( param.flag_geomnmx == PARAM_SET )
	  (void) H5LTset_attribute_string( file_id, "/", "WithinRegion", 
					   "TRUE" );
     else
	  (void) H5LTset_attribute_string( file_id, "/", "WithinRegion", 
					   "FALSE" );
/*
 * time window
 */
     if ( param.flag_period == PARAM_SET ) {
	  (void) H5LTset_attribute_string( file_id, "/", "StartDate", 
					   param.bgn_date );
	  (void) H5LTset_attribute_string( file_id, "/", "EndDate", 
					   param.end_date );
     }
     switch ( instrument ) {
/*
 * ----- GOME level 1b processor specific options
 */
     case GOME_LEVEL_1:
/*
 * pixel number range
 */
	  if ( param.flag_pselect == PARAM_SET ) {
	       (void) H5LTset_attribute_string( file_id, "/", 
						"GroundPixelRange", 
						param.pselect );
	  } else {
	       (void) H5LTset_attribute_string( file_id, "/", 
						"GroundPixelRange", "ALL" );
	  }
/*
 * ground pixel type selection
 */
	  if ( param.flag_subset == PARAM_SET ) {
	       (void) strcpy( string, "" );
	       if ( (param.write_subset & SUBSET_EAST) != UCHAR_ZERO )
		    (void) nadc_strlcat( string, "E", SHORT_STRING_LENGTH );
	       if ( (param.write_subset & SUBSET_CENTER) != UCHAR_ZERO )
		    (void) nadc_strlcat( string, "C", SHORT_STRING_LENGTH );
	       if ( (param.write_subset & SUBSET_WEST) != UCHAR_ZERO )
		    (void) nadc_strlcat( string, "W", SHORT_STRING_LENGTH );
	       if ( (param.write_subset & SUBSET_BACK) != UCHAR_ZERO )
		    (void) nadc_strlcat( string, "B", SHORT_STRING_LENGTH );
	       (void) H5LTset_attribute_string( file_id, "/", 
						"GroundPixelType", string );
	  } else {
	       (void) H5LTset_attribute_string( file_id, "/", 
						"GroundPixelType", "ALL" );
	  }
/*
 * selected measurement data sets
 */
	  (void) strcpy( cbuff, "" );
	  if ( param.write_limb == PARAM_UNSET ) {
	       if ( strlen( cbuff ) > 0 ) 
		    (void) nadc_strlcat( cbuff, ",", MAX_STRING_LENGTH );
	       (void) nadc_strlcat( cbuff, "NoLimb", MAX_STRING_LENGTH );
	  }
	  if ( param.write_moni == PARAM_UNSET ) {
	       if ( strlen( cbuff ) > 0 ) 
		    (void) nadc_strlcat( cbuff, ",", MAX_STRING_LENGTH );
	       (void) nadc_strlcat( cbuff, "NoMonitor", MAX_STRING_LENGTH );
	  }
	  if ( param.write_moon == PARAM_UNSET ) {
	       if ( strlen( cbuff ) > 0 ) 
		    (void) nadc_strlcat( cbuff, ",", MAX_STRING_LENGTH );
	       (void) nadc_strlcat( cbuff, "NoMoon", MAX_STRING_LENGTH );
	  }
	  if ( param.write_nadir == PARAM_UNSET ) {
	       if ( strlen( cbuff ) > 0 ) 
		    (void) nadc_strlcat( cbuff, ",", MAX_STRING_LENGTH );
	       (void) nadc_strlcat( cbuff, "NoNadir", MAX_STRING_LENGTH );
	  }
	  if ( param.write_occ == PARAM_UNSET ) {
	       if ( strlen( cbuff ) > 0 ) 
		    (void) nadc_strlcat( cbuff, ",", MAX_STRING_LENGTH );
	       (void) nadc_strlcat( cbuff, "NoOcc", MAX_STRING_LENGTH );
	  }
	  if ( param.write_sun == PARAM_UNSET ) {
	       if ( strlen( cbuff ) > 0 ) 
		    (void) nadc_strlcat( cbuff, ",", MAX_STRING_LENGTH );
	       (void) nadc_strlcat( cbuff, "NoSun", MAX_STRING_LENGTH );
	  }
	  if ( strlen( cbuff ) == 0 ) (void) strcpy( cbuff, "ALL" );
	  (void) H5LTset_attribute_string( file_id, "/", 
					   "MeasurementDataSets", cbuff );
/*
 * spectral bands
 */
	  if ( (param.chan_mask & BAND_ALL) != UCHAR_ZERO ) {
	       (void) strcpy( string, "" );
	       if ( (param.chan_mask & BAND_ONE_A) != UCHAR_ZERO ) {
		    if ( strlen( string ) > 0 ) 
			 (void) nadc_strlcat( string, ",", SHORT_STRING_LENGTH);
		    (void) nadc_strlcat( string, "1a", SHORT_STRING_LENGTH );
	       }	  
	       if ( (param.chan_mask & BAND_ONE_B) != UCHAR_ZERO ) {
		    if ( strlen( string ) > 0 ) 
			 (void) nadc_strlcat( string, ",", SHORT_STRING_LENGTH);
		    (void) nadc_strlcat( string, "1b", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_TWO_A) != UCHAR_ZERO ) {
		    if ( strlen( string ) > 0 ) 
			 (void) nadc_strlcat( string, ",", SHORT_STRING_LENGTH);
		    (void) nadc_strlcat( string, "2a", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_TWO_B) != UCHAR_ZERO ) {
		    if ( strlen( string ) > 0 ) 
			 (void) nadc_strlcat( string, ",", SHORT_STRING_LENGTH);
		    (void) nadc_strlcat( string, "2b", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_THREE) != UCHAR_ZERO ) {
		    if ( strlen( string ) > 0 ) 
			 (void) nadc_strlcat( string, ",", SHORT_STRING_LENGTH);
		    (void) nadc_strlcat( string, "3", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_FOUR) != UCHAR_ZERO ) {
		    if ( strlen( string ) > 0 ) 
			 (void) nadc_strlcat( string, ",", SHORT_STRING_LENGTH);
		    (void) nadc_strlcat( string, "4", SHORT_STRING_LENGTH );
	       }
	       if ( strlen( string ) == 0 ) {
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "Bands", "FALSE" );
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "BandCalibration", 
						     "FALSE" );
	       } else {
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "Bands", string );
	       }
	  } else {
	       (void) H5LTset_attribute_string( file_id, "/", "Bands", "ALL" );
	  }
/*
 * blind data
 */
	  if ( param.write_blind == PARAM_SET )
	       (void) H5LTset_attribute_string( file_id, "/", "Blind", 
						"TRUE" );
	  else
	       (void) H5LTset_attribute_string( file_id, "/", "Blind", 
						"FALSE" );
/*
 * straylight data
 */
	  if ( param.write_stray == PARAM_SET )
	       (void) H5LTset_attribute_string( file_id, "/", "Straylight", 
						"TRUE" );
	  else
	       (void) H5LTset_attribute_string( file_id, "/", "Straylight", 
						"FALSE" );
/*
 * wavelength range
 */
	  (void) H5LTset_attribute_float( file_id, "/", "Wavelengths", 
					  param.wave, 2 );
/*
 * calibration of the spectral band data
 */
	  if ( param.calib_earth == CALIB_NONE ) {
	       (void) H5LTset_attribute_string( file_id, "/", 
						"EarthCalibration", "FALSE" );
	  } else {
	       char *cpntr;
	       GOME_GET_CALIB( param.calib_earth, string );
	       if ( strlen(string) == 0 ) {
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "EarthCalibration", 
						     "FALSE" );
	       } else {
		    if ( (cpntr = strchr( string, 'B' )) != NULL ) 
			 *cpntr = 'A';
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "EarthCalibration", 
						     string );
	       }
	  }
	  if ( param.calib_moon == CALIB_NONE ) {
	       (void) H5LTset_attribute_string( file_id, "/", 
						"MoonCalibration", "FALSE" );
	  } else {
	       GOME_GET_CALIB( param.calib_moon, string );
	       if ( strlen(string) == 0 ) {
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "MoonCalibration", 
						     "FALSE" );
	       } else
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "MoonCalibration", 
						     string );
	  }
	  if ( param.calib_sun == CALIB_NONE ) {
	       (void) H5LTset_attribute_string( file_id, "/", 
						"SunCalibration", "FALSE" );
	  } else {
	       GOME_GET_CALIB( param.calib_sun, string );
	       if ( strlen(string) == 0 ) {
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "SunCalibration", 
						     "FALSE" );
	       } else
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "SunCalibration", 
						     string );
	  }
/*
 * PMD data
 */
	  if ( param.write_pmd == PARAM_SET ) {
	       (void) H5LTset_attribute_string( file_id, "/", "PMD", "TRUE" );
	       if ( param.calib_pmd == CALIB_NONE ) {
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "PMD_Calibration", 
						     "FALSE" );
	       } else {
		    GOME_GET_CALIB( param.calib_pmd, string );
		    if ( strlen(string) == 0 ) {
			 (void) H5LTset_attribute_string( file_id, "/", 
							  "PMD_Calibration", 
							  "FALSE" );
		    } else
			 (void) H5LTset_attribute_string( file_id, "/", 
							  "PMD_Calibration", 
							  string );
	       }
	       if ( param.write_pmd_geo == PARAM_SET ) {
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "PMD_Geolocation", 
						     "TRUE" );
	       } else {
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "PMD_Geolocation", 
						     "FALSE" );
	       }
	  } else {
	       (void) H5LTset_attribute_string( file_id, "/", "PMD", "FALSE" );
	       (void) H5LTset_attribute_string( file_id, "/", 
						"PMD_Calibration", "FALSE" );
	       (void) H5LTset_attribute_string( file_id, "/", 
						"PMD_Geolocation", "FALSE" );
	  }
	  break;
/*
 *  ----- GOME level 2 processor specific options
 */
     case GOME_LEVEL_2:
/*
 * pixel selection
 */
	  if ( param.flag_pselect == PARAM_SET ) {
	       (void) H5LTset_attribute_string( file_id, "/", 
						"GroundPixelRange",
						param.pselect );
	  } else {
	       (void) H5LTset_attribute_string( file_id, "/", 
						"GroundPixelRange", "ALL" );
	  }
/*
 * Sun zenith angle range
 */
	  (void) H5LTset_attribute_float( file_id, "/", "SunZenithAngle", 
					  param.sunz, 2 );
/*
 * cloud cover range
 */
	  (void) H5LTset_attribute_float( file_id, "/", "CloudCover", 
					  param.cloud, 2 );
	  break;
     }
}

/*+++++++++++++++++++++++++ Main Program or Function(s) +++++++++++++++*/
hid_t GOME_CRE_H5_FILE( int instrument, const struct param_record *param )
{
     const char prognm[] = "GOME_CRE_H5_FILE";
/*
 * create HDF5-file
 */
     hid_t file_id = H5Fcreate( param->hdf5_name, H5F_ACC_TRUNC, 
				H5P_DEFAULT, H5P_DEFAULT );
     if ( file_id < 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, param->hdf5_name );
/*
 * write global attributes
 */
     WRITE_HDF5_HISTORY( file_id, instrument, *param );
done:
     return file_id;
}
