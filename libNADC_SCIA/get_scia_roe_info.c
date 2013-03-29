/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GET_SCIA_ROE_INFO
.AUTHOR      R.M. van Hees
.KEYWORDS    Envisat PDS 
.LANGUAGE    ANSI C
.PURPOSE     get orbit phase and SAA flag from ROE records
.INPUT/OUTPUT
  call as   GET_SCIA_ROE_INFO( eclipseMode, jday, 
                               &absOrbit, &saaFlag, &orbitPhase );
     input:
             bool   eclipseMode :  TRUE  - orbit phase used for SDMF (v2.4)
                                   FALSE - orbit phase as used by ESA
	     double jday        :  julian Day (# days since 2000-01-01)
    output:
             int   *absOrbit    :  absolute orbit number
             float *saaFlag     :  in-precise SAA flag
             float *orbitPhase  :  orbit phase [0,1]

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     2.0   18-Jan-2008  rewrite and combined different routines, RvH
             1.1   18-Jan-2008  added GET_SCIA_ROE_ORBIT, RvH
             1.0   18-Dec-2007	created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <math.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define _SCIA_COMMON
#include <nadc_scia.h>

/*+++++ Static Variables +++++*/
static const char name_ROE_db[] = "ROE_EXC_all.h5";

#define NFIELDS  15
struct roe_rec {
     double         julianDay;
     unsigned short orbit;
     unsigned short relOrbit;
     unsigned char  phase;
     unsigned char  cycle;
     unsigned short repeat;
     unsigned char  saaDay;
     unsigned char  saaEclipse;
     double         eclipseExit;
     double         eclipseEntry;
     double         period;
     double         anxLongitude;
     char           UTC_anx[28];
     char           UTC_flt[28];
     char           mlst[8];
};

static const size_t roeSize = sizeof( struct roe_rec );
static const size_t roeOffs[NFIELDS] = {
     HOFFSET(struct roe_rec, julianDay),
     HOFFSET(struct roe_rec, orbit),
     HOFFSET(struct roe_rec, relOrbit),
     HOFFSET(struct roe_rec, phase),
     HOFFSET(struct roe_rec, cycle),
     HOFFSET(struct roe_rec, repeat),
     HOFFSET(struct roe_rec, saaDay),
     HOFFSET(struct roe_rec, saaEclipse),
     HOFFSET(struct roe_rec, eclipseExit),
     HOFFSET(struct roe_rec, eclipseEntry),
     HOFFSET(struct roe_rec, period),
     HOFFSET(struct roe_rec, anxLongitude),
     HOFFSET(struct roe_rec, UTC_anx),
     HOFFSET(struct roe_rec, UTC_flt),
     HOFFSET(struct roe_rec, mlst)
};

/*+++++ Global Variables +++++*/

/*+++++++++++++++++++++++++ Static Function(s) +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   _GET_ROE_INDEX
.PURPOSE     return index in ROE table for given Julian day
.INPUT/OUTPUT
  call as   roeIndx = _GET_ROE_INDEX( julianDay, &jday_mn, &jday_mx );
     input:
	     double julianDay  :  julian Day (# days since 2000-01-01)

.RETURNS     index to ROE table for given julianDay
             error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
size_t _GET_ROE_INDEX( hid_t fileID, double jday, 
		       /*@out@*/ double *jday_mn, /*@out@*/ double *jday_mx )
{
     const char prognm[] = "_GET_ROE_INDEX";

     register size_t indx = 0;

     size_t  numRoe;
     hsize_t adim;
     herr_t  stat;

     double  *jday_list = NULL;

     *jday_mn = *jday_mx = -1.;
/*
 * read info_h5 records
 */
     stat = H5LTget_dataset_info( fileID, "julianDay", &adim, NULL, NULL );
     if ( stat < 0 || (numRoe = (size_t) adim) == 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, "julianDay" );
/*
 * read julian dates
 */
     if ( (jday_list = (double *) malloc( numRoe * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "jday_list" );
     if ( H5LTread_dataset_double ( fileID, "julianDay", jday_list ) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, "julianDay" );
/*
 * search for match
 */
     while( indx < (numRoe-1) && jday >= jday_list[indx+1] ) indx++;
     if ( indx == (numRoe-1) || jday < jday_list[indx] ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_WARN, "no solution found" );
     *jday_mn = jday_list[indx];
     *jday_mx = jday_list[indx+1];

     free( jday_list );
     return indx;
 done:
     if ( jday_list != NULL ) free( jday_list );
     return 0;
}

/*+++++++++++++++++++++++++
.IDENTifer   _GET_ROE_ENTRY
.PURPOSE     return ROE record for given Julian day (fast)
.INPUT/OUTPUT
  call as   orbit = _GET_ROE_ENTRY( julianDay, &roe );
     input:
	     double julianDay  :  julian Day (# days since 2000-01-01)
    output:
             struct roe_rec *roe : ROE record (required static in caller)

.RETURNS     absolute orbit number for given julianDay
             error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
int _GET_ROE_ENTRY( double jday, /*@out@*/ struct roe_rec *roe )
{
     const char prognm[] = "_GET_ROE_ENTRY";

     char    string[MAX_STRING_LENGTH];

     hid_t   fileID = -1;
     herr_t  stat;

     static size_t  indxEntry = 0;
     static double  jday_mn = -1.;
     static double  jday_mx = -1.;

     const size_t roeSizes[NFIELDS] = {
	  sizeof(roe->julianDay),
	  sizeof(roe->orbit),
	  sizeof(roe->relOrbit),
	  sizeof(roe->phase),
	  sizeof(roe->cycle),
	  sizeof(roe->repeat),
	  sizeof(roe->saaDay),
	  sizeof(roe->saaEclipse),
	  sizeof(roe->eclipseExit),
	  sizeof(roe->eclipseEntry),
	  sizeof(roe->period),
	  sizeof(roe->anxLongitude),
	  sizeof(roe->UTC_anx),
	  sizeof(roe->UTC_flt),
	  sizeof(roe->mlst)
     };
/*
 * do we need an update of the ROE record?
 */
     if ( jday >= jday_mn && jday < jday_mx ) return roe->orbit;
/*
 * open output HDF5-file
 */
     (void) snprintf( string, MAX_STRING_LENGTH, "./%s", name_ROE_db );
     H5E_BEGIN_TRY {
          fileID = H5Fopen( string, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( fileID < 0 ) {
          (void) snprintf( string, MAX_STRING_LENGTH, "%s/%s", 
			   DATA_DIR, name_ROE_db );
          fileID = H5Fopen( string, H5F_ACC_RDONLY, H5P_DEFAULT );
          if ( fileID < 0 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, string );
     }
/*
 * get index to list with ROE entries
 */
     indxEntry = _GET_ROE_INDEX( fileID, jday, &jday_mn, &jday_mx );
     if ( jday_mx < 0. ) goto done;
/*
 * read whole ROE-record
 */
     stat = H5TBread_records( fileID, "roe_entry", (hsize_t) indxEntry, 1,
			      roeSize, roeOffs, roeSizes, roe );
     if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, "roe_entry" );
     (void) H5Fclose( fileID );
     return roe->orbit;
done:
     if ( fileID > 0 ) (void) H5Fclose( fileID );
     return -1;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GET_SCIA_ROE_INFO( bool eclipseMode, const double jday, 
			int *absOrbit, bool *saaFlag, float *orbitPhase )
{
     double tdiff;

     const double secPerDay = 60. * 60 * 24;

     static struct roe_rec roe;
/*
 * initialise return values
 */
     *absOrbit   = -1;
     *saaFlag    = FALSE;
     *orbitPhase = -1.f;
/*
 * get absOrbit and ROE entry for given julianDay
 */
     if ( _GET_ROE_ENTRY( jday, &roe ) < 0 ) return;

     *absOrbit = roe.orbit;
     tdiff = (jday - roe.julianDay) * secPerDay - roe.eclipseEntry;
     if ( ! eclipseMode ) {
	  double orbitPhaseDiff = 0.5 *
	       ((roe.eclipseEntry - roe.eclipseExit) / roe.period - 0.5);

	  *orbitPhase =  (float)
	       (fmod( tdiff, roe.period ) / roe.period + orbitPhaseDiff);
     } else {
	  *orbitPhase = fmodf( tdiff, roe.period ) / roe.period;
     }
     if ( *orbitPhase < 0.f ) *orbitPhase += 1.f;
     if ( *orbitPhase > 0.5f ) 
	  *saaFlag = (roe.saaDay == UCHAR_ZERO);
     else
	  *saaFlag = (roe.saaEclipse == UCHAR_ZERO);
}

double GET_SCIA_ROE_JDAY( unsigned short absOrbit )
{
     const char prognm[] = "GET_SCIA_ROE_JDAY";

     register size_t nr;

     char    string[MAX_STRING_LENGTH];

     hid_t   fileID = -1;
     size_t numRoe;
     hsize_t adim;
     herr_t  stat;

     double jday = -1.;

     unsigned short *orbit_list = NULL;
     double         *jday_list = NULL;
/*
 * open output HDF5-file
 */
     (void) snprintf( string, MAX_STRING_LENGTH, "./%s", name_ROE_db );
     H5E_BEGIN_TRY {
          fileID = H5Fopen( string, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( fileID < 0 ) {
          (void) snprintf( string, MAX_STRING_LENGTH, "%s/%s", 
			   DATA_DIR, name_ROE_db );
          fileID = H5Fopen( string, H5F_ACC_RDONLY, H5P_DEFAULT );
          if ( fileID < 0 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, string );
     }
/*
 * read info_h5 records
 */
     stat = H5LTget_dataset_info( fileID, "julianDay", &adim, NULL, NULL );
     if ( stat < 0 || (numRoe = (size_t) adim) == 0 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, "julianDay" );
/*
 * read julian dates
 */
     if ( (jday_list = (double *) malloc( numRoe * sizeof(double))) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "jday_list" );
     if ( H5LTread_dataset_double( fileID, "julianDay", jday_list ) < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, "julianDay" );
/*
 * read orbit numbers
 */
     orbit_list = (unsigned short *) malloc( numRoe * sizeof(short) );
     if ( orbit_list == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "orbit_list" );
     stat = H5LTread_dataset( fileID, "orbitList", H5T_NATIVE_USHORT, 
			      orbit_list );
     if ( stat < 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_RD, "orbitList" );
/*
 * search for match
 */
     nr = 0;
     while( nr < (numRoe-1) && absOrbit >= orbit_list[nr+1] ) nr++;

     if ( nr == (numRoe-1) || absOrbit < orbit_list[nr] ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_WARN, "no solution found" );
     jday = jday_list[nr];
 done:
     if ( orbit_list != NULL ) free( orbit_list );
     if ( jday_list != NULL ) free( jday_list );
     if ( fileID > 0 ) (void) H5Fclose( fileID );
     return jday;
}