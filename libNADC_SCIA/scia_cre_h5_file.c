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

.IDENTifer   SCIA_CRE_H5_FILE
.AUTHOR      R.M. van Hees
.KEYWORDS    HDF5 interface
.LANGUAGE    ANSI C
.PURPOSE     Create HDF5 file for SCIA level 1 & 2 products
.INPUT/OUTPUT
  call as    SCIA_CRE_H5_FILE( instument, param );
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
              1.2   03-Aug-1999 Added CRE_GROUPS_SCIA_2, RvH
              1.1   23-Jul-1999 Added CRE_GROUPS_SCIA_1, RvH
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
#include <nadc_scia.h>

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
     register int nr;

     char    cbuff[MAX_STRING_LENGTH];
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
     SCIA_WR_H5_VERSION( file_id );
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
 *  ----- SCIAMACHY level 0 processor specific options
 */
     case SCIA_LEVEL_0:
/*
 * selected measurement data sets
 */
	  (void) strcpy( cbuff, "" );
	  if ( param.write_aux == PARAM_UNSET ) {
	       if ( strlen( cbuff ) > 0 ) 
		    (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
	       (void) nadc_strlcat( cbuff, "NoAuxiliary", SHORT_STRING_LENGTH );
	  }
	  if ( param.write_det == PARAM_UNSET ) {
	       if ( strlen( cbuff ) > 0 ) 
		    (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
	       (void) nadc_strlcat( cbuff, "NoDetector", SHORT_STRING_LENGTH );
	  }
	  if ( param.write_pmd == PARAM_UNSET ) {
	       if ( strlen( cbuff ) > 0 ) 
		    (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
	       (void) nadc_strlcat( cbuff, "NoPMD", SHORT_STRING_LENGTH );
	  }
	  if ( strlen( cbuff ) == 0 ) (void) strcpy( cbuff, "ALL" );
	  (void) H5LTset_attribute_string( file_id, "/", 
					   "MeasurementDataSets", cbuff );
/*
 * spectral bands
 */
	  if ( (param.chan_mask & BAND_ALL) != UCHAR_ZERO ) {
	       (void) strcpy( cbuff, "" );
	       if ( (param.chan_mask & BAND_ONE) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
		    (void) nadc_strlcat( cbuff, "1", SHORT_STRING_LENGTH );
	       }	  
	       if ( (param.chan_mask & BAND_TWO) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
		    (void) nadc_strlcat( cbuff, "2", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_THREE) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
		    (void) nadc_strlcat( cbuff, "3", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_FOUR) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
		    (void) nadc_strlcat( cbuff, "4", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_FIVE) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
		    (void) nadc_strlcat( cbuff, "5", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_SIX) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH);
		    (void) nadc_strlcat( cbuff, "6", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_SEVEN) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
		    (void) nadc_strlcat( cbuff, "7", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_EIGHT) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
		    (void) nadc_strlcat( cbuff, "8", SHORT_STRING_LENGTH );
	       }
	       if ( strlen( cbuff ) == 0 )
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "Bands", "FALSE" );
	       else
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "Bands", cbuff );
	  } else
	       (void) H5LTset_attribute_string( file_id, "/", "Bands", "ALL" );
/*
 * selected States
 */
	  if ( param.stateID_nr == PARAM_UNSET ) {
	       (void) H5LTset_attribute_string( file_id, "/", "StateID", 
						"ALL" );
	  } else {
	       (void) snprintf( cbuff, MAX_STRING_LENGTH, "%-d", 
				(int) param.stateID[0] );
	       for ( nr = 1; nr < (int) param.stateID_nr; nr++ ) {
		    (void) snprintf( cbuff, MAX_STRING_LENGTH, "%s,%-d", 
				     cbuff, (int) param.stateID[nr] );
	       }
	       (void) H5LTset_attribute_string( file_id, "/", "StateID", 
						cbuff );
	  }
	  break;
/*
 *  ----- SCIAMACHY level 1 processor specific options
 */
     case SCIA_LEVEL_1:
/*
 * selected Key DSD's
 */
	  (void) strcpy( cbuff, "" );
	  if ( param.write_ads == PARAM_UNSET ) {
	       if ( strlen( cbuff ) > 0 )
		    (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
	       (void) nadc_strlcat( cbuff, "NoADS", SHORT_STRING_LENGTH );
	  }
	  if ( param.write_gads == PARAM_UNSET ) {
	       if ( strlen( cbuff ) > 0 ) 
		    (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
	       (void) nadc_strlcat( cbuff, "NoGADS", SHORT_STRING_LENGTH );
	  }
	  if ( strlen( cbuff ) == 0 ) (void) strcpy( cbuff, "ALL" );
	  (void) H5LTset_attribute_string( file_id, "/", "KeyDataSets", 
					   cbuff );
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
	       (void) nadc_strlcat( cbuff, "NoMonitoring", MAX_STRING_LENGTH );
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
	  if ( strlen( cbuff ) == 0 ) (void) strcpy( cbuff, "ALL" );
	  (void) H5LTset_attribute_string( file_id, "/", 
					   "MeasurementDataSets", cbuff );
/*
 * PMD data
 */
	  if ( param.write_pmd == PARAM_SET )
	       (void) H5LTset_attribute_string( file_id, "/", "PMD", "TRUE" );
	  else
	       (void) H5LTset_attribute_string( file_id, "/", "PMD", "FALSE" );
/*
 * fraction polarisation data
 */
	  if ( param.write_polV == PARAM_SET )
	       (void) H5LTset_attribute_string( file_id, "/", "FracPol", 
						"TRUE" );
	  else
	       (void) H5LTset_attribute_string( file_id, "/", "FracPol", 
						"FALSE" );
/*
 * spectral bands
 */
	  if ( (param.chan_mask & BAND_ALL) != UCHAR_ZERO ) {
	       (void) strcpy( cbuff, "" );
	       if ( (param.chan_mask & BAND_ONE) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
		    (void) nadc_strlcat( cbuff, "1", SHORT_STRING_LENGTH );
	       }	  
	       if ( (param.chan_mask & BAND_TWO) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
		    (void) nadc_strlcat( cbuff, "2", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_THREE) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
		    (void) nadc_strlcat( cbuff, "3", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_FOUR) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
		    (void) nadc_strlcat( cbuff, "4", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_FIVE) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
		    (void) nadc_strlcat( cbuff, "5", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_SIX) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
		    (void) nadc_strlcat( cbuff, "6", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_SEVEN) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
		    (void) nadc_strlcat( cbuff, "7", SHORT_STRING_LENGTH );
	       }
	       if ( (param.chan_mask & BAND_EIGHT) != UCHAR_ZERO ) {
		    if ( strlen( cbuff ) > 0 ) 
			 (void) nadc_strlcat( cbuff, ",", SHORT_STRING_LENGTH );
		    (void) nadc_strlcat( cbuff, "8", SHORT_STRING_LENGTH );
	       }
	       if ( strlen( cbuff ) == 0 )
		    (void) H5LTset_attribute_string( file_id, "/", "Bands", 
						     "FALSE" );
	       else
		    (void) H5LTset_attribute_string( file_id, "/", "Bands",
						     cbuff );
	  } else
	       (void) H5LTset_attribute_string( file_id, "/", "Bands", "ALL" );
/*
 * selected Categories
 */
	  if ( param.catID_nr == PARAM_UNSET ) {
	       (void) H5LTset_attribute_string( file_id, "/", "Category", 
						"ALL" );
	  } else {
	       (void) snprintf( cbuff, MAX_STRING_LENGTH, "%-d", 
				(int) param.catID[0] );
	       for ( nr = 1; nr < (int) param.catID_nr; nr++ ) {
		    (void) snprintf( cbuff, MAX_STRING_LENGTH, "%s,%-d", 
				     cbuff, (int) param.catID[nr] );
	       }
	       (void) H5LTset_attribute_string( file_id, "/", "Category", 
						cbuff );
	  }
/*
 * selected States
 */
	  if ( param.stateID_nr == PARAM_UNSET ) {
	       (void) H5LTset_attribute_string( file_id, "/", "StateID", 
						"ALL" );
	  } else {
	       (void) snprintf( cbuff, MAX_STRING_LENGTH, "%-d", 
				(int) param.stateID[0] );
	       for ( nr = 1; nr < (int) param.stateID_nr; nr++ ) {
		    (void) snprintf( cbuff, MAX_STRING_LENGTH, "%s,%-d", 
				     cbuff, (int) param.stateID[nr] );
	       }
	       (void) H5LTset_attribute_string( file_id, "/", "StateID", 
						cbuff );
	  }
/*
 * selected Clusters
 */
	  if ( param.clusID_nr == PARAM_UNSET ) {
	       (void) H5LTset_attribute_string( file_id, "/", "Clusters", 
						"ALL" );
	  } else {
	       (void) strcpy( cbuff, "" );
	       for ( nr = 0; nr < MAX_NUM_STATE; nr++ ) {
		    if (Get_Bit_LL(param.clus_mask,(unsigned char)nr) == 0ULL)
			 (void) nadc_strlcat( cbuff, "0", MAX_STRING_LENGTH );
		    else
			 (void) nadc_strlcat( cbuff, "1", MAX_STRING_LENGTH );
	       }
	       (void) H5LTset_attribute_string( file_id, "/", "Clusters", 
						cbuff );
	  }
/*
 * calibration of the spectral band data
 */
	  if ( param.calib_scia == CALIB_NONE ) {
	       (void) H5LTset_attribute_string( file_id, "/", "Calibration", 
						"FALSE" );
	  } else {
	       SCIA_GET_CALIB( param.calib_scia, cbuff );
	       if ( strlen(cbuff) == 0 ) {
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "Calibration", "FALSE" );
	       } else {
		    (void) H5LTset_attribute_string( file_id, "/", 
						     "Calibration", cbuff );
	       }
	  }
	  break;
/*
 *  ----- SCIAMACHY level 2 processor specific options
 */
	  case SCIA_LEVEL_2:
	  break;
     }
}

/*+++++++++++++++++++++++++ Main Program or Function(s) +++++++++++++++*/
hid_t SCIA_CRE_H5_FILE( int instrument, const struct param_record *param )
{
     /* create HDF5-file */
     hid_t file_id = H5Fcreate( param->hdf5_name, H5F_ACC_TRUNC, 
				H5P_DEFAULT, H5P_DEFAULT );
     if ( file_id < 0 ) 
	  NADC_GOTO_ERROR( NADC_ERR_HDF_FILE, param->hdf5_name );

     /* write global attributes */
     WRITE_HDF5_HISTORY( file_id, instrument, *param );
done:
     return file_id;
}
