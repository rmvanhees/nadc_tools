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

.IDENTifer   SCIA_CFI_IDL
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY, IDL
.LANGUAGE    ANSI C
.PURPOSE     IDL wrapper for reading SCIAMACHY data (general)
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0   15-Feb-2010	Created by P. van der Meer
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <glob.h>

#include <ppf_pointing.h>    /* PPF_POINTING header file */
#include <ppf_orbit.h>       /* PPF_ORBIT header file */

/*+++++ Local Headers +++++*/
#define _SCIA_COMMON
#include <nadc_idl.h>

#define ESM0 220.

/*+++++ Global Variables +++++*/


/*+++++ Static Variables +++++*/

/*+++++ Predeclarations +++++*/

/* Geo-location code */

/*+++++++++++++++++++++++++
.IDENTifer   INITIALISE_DORIS_GEO
.PURPOSE     initialise Doris orbit/geolocation calculations
.INPUT/OUTPUT
  call as   stat = INITIALISE_DORIS_GEO( MJD, &por_flag, time_valid, mjdp );
     input:
            double MJD         : 
 in/output:
            double *mjdp       :
    output:
            bool   *por_flag   :
	    double *time_valid :

.RETURNS     status flag: no error PO_OK else PO_ERR
.COMMENTS    Read filelists using:
     glob: http://www.opengroup.org/onlinepubs/009695399/functions/glob.html
     Convert JD to date using ../../libNADC/nadc_date.c

     Kijkrichting naar de zon kan worden berekend met pp_target in ppf_pointing
-------------------------*/
static 
int INITIALISE_DORIS_GEO( const double MJD, 
			  bool *por_flag, double *time_valid, double *mjdp)
{
     const char prognm[] = "INITIALISE_DORIS_GEO";

     char   tmpstring[UTC_STRING_LENGTH], date1[9], date2[9];
     char   globstring[MAX_STRING_LENGTH];
     double mjd_int, mjd_frac;
     glob_t globvor[1];
     glob_t globpor[1];

     long   status;                         /* Main status flag */

     long   ndc, ndp, ner, ierr[10];
     double mjdr0, mjdr1, x[6], pos[3], vel[3], acc[3], res[54];

     long   selected = PO_NONE;

     long mode   = PO_INIT_FILE;
     long choice = PO_AUTO_SELECT;

     /* Set error handling mode to SILENT */
     status = po_silent();   /* Set error handling mode to SILENT */
/*
 * convert mjd
 */
     mjd_frac = modf( MJD, &mjd_int );
     mjd_frac *= 24.;
     if ( mjd_frac > (22.0+5./60.) ) mjd_int += 1;
     mjd_frac *= 3600.;
     MJD_2_YMD( (int) mjd_int - 1, mjd_frac, tmpstring );
     (void) nadc_strlcpy( date1, tmpstring, 9 );
     MJD_2_YMD( (int) mjd_int + 1, mjd_frac, tmpstring );
     (void) nadc_strlcpy( date2, tmpstring, 9 );

     {
	  char utce[28], dut1e[9];
	  long mjdt[4];
	  mjdt[0] = (long) mjd_int;
	  mjdt[1] = (long) mjd_frac;
	  mjdt[2] = (long) ((mjd_frac - mjdt[1]) * 1000.0);
	  mjdt[3] = 0;

	  status = pl_tmjd( mjdt, mjdp, utce, dut1e );
	  if ( status != PL_OK ) {
	       long  nm;
	       long  func_id = PL_TMJD_ID;
	       char  msg[PL_MAX_COD][PL_MAX_STR];

	       (void) pl_vector_msg( &func_id, ierr, &nm, msg );
	       while ( --nm >= 0 )
		    NADC_ERROR( prognm, NADC_ERR_DORIS, msg[nm] );
	       if ( status <= PL_ERR ) return PL_ERR;
	  }
     }
/*
 * obtain Doris precision file
 */
     *por_flag = TRUE;
     ndc = ndp = ner = 0;
     (void) snprintf( globstring, MAX_STRING_LENGTH,
		      "%s/vor/%6.6s/DOR_VOR_AXVF-P*_%s_*_%s_*",
		      DATA_DIR, date2, date1, date2 );
     (void) glob( globstring, GLOB_ERR, NULL, globvor );
     if ( globvor->gl_pathc == 0 ) {
	  (void) snprintf( globstring, MAX_STRING_LENGTH,
			   "%s/vor/latest_data/DOR_VOR_AXVF-P*_%s_*_%s_*",
			   DATA_DIR, date1, date2 );
	  (void) glob( globstring, GLOB_ERR, NULL, globvor );
     }
     if ( (ndc = globvor->gl_pathc) == 0 ) {
	  (void) snprintf( globstring, MAX_STRING_LENGTH,
			   "%s/por/%6.6s/DOR_POR_AXVF-P*_%s_*_%s_*",
			   DATA_DIR, date2, date1, date2 );
	  (void) glob( globstring, GLOB_ERR, NULL, globpor );
	  if ( globpor->gl_pathc == 0 ) {
	       (void) snprintf( globstring, MAX_STRING_LENGTH,
				"%s/por/latest_data/DOR_POR_AXVF-P*_%s_*_%s_*",
				DATA_DIR, date1, date2 );
	       (void) glob( globstring, GLOB_ERR, NULL, globvor );
	  }
	  if ( (ndp = globpor->gl_pathc) == 0 ) {
	       char str_msg[SHORT_STRING_LENGTH];

	       (void) snprintf( str_msg, SHORT_STRING_LENGTH, 
				"No DORIS precise file found for %s-%s", 
				date1, date2 );
	       NADC_ERROR( prognm, NADC_ERR_NONE, str_msg );
	       return PO_ERR;
	  }
     } else /* found Doris precise orbit files */
	  *por_flag = FALSE;
     
     /* Calling po_interpol (initialization) */
     mjdr0 = mjd_int + ((22.0 + 5. / 60.) / 24.) - 1.;
     mjdr1 = mjdr0 + 1.;
     status = po_interpol( &mode    , &choice,
			   &ndc     , globvor->gl_pathv,
			   &ndp     , globpor->gl_pathv,
			   &ner     , NULL,
			   &mjdr0   , &mjdr1, mjdp, x, pos, vel, acc,
			   &selected, res, ierr );
    //printf("pos = (%f, %f, %f)\n", pos[0], pos[1], pos[2]);
     if ( ndc > 0 ) globfree( globvor );
     if ( ndp > 0 ) globfree( globpor );
     if ( status == PO_ERR ) {
	  long  nm;
	  long  func_id = PO_INTERPOL_ID;
	  char  msg[PO_MAX_COD][PO_MAX_STR];

	  (void) po_vector_msg( &func_id, ierr, &nm, msg );
	  while ( --nm >= 0 )
	       NADC_ERROR( prognm, NADC_ERR_DORIS, msg[nm] );
	  return PO_ERR;
     }
     time_valid[0] = mjdr0;
     time_valid[1] = mjdr1;

     return PO_OK;
}

/*+++++++++++++++++++++++++
.IDENTifer   INTERPOLATE_DORIS_GEO
.PURPOSE     perform interpolation to calculate Doris orbit/geolocation
.INPUT/OUTPUT
  call as   stat = INTERPOLATE_DORIS_GEO( MJD, geo, pos, vel, acc, mjdp );
     input:
            double MJD    : 
 in/output:
            double *mjdp  :
    output:
            double *geo   :
            double *pos   :
            double *vel   :
            double *acc   :

.RETURNS     status flag: no error PO_OK else PO_ERR
.COMMENTS    none
-------------------------*/
static 
int INTERPOLATE_DORIS_GEO( const double MJD, double *geo, 
			   double *pos, double *vel, 
			   double *acc, double *mjdp )
{
     const char prognm[] = "INTERPOLATE_DORIS_GEO";

     /* po_interpol variables */
     long status;                         /* Main status flag */

     long mode;
     long ierr[10], dummy_n=0;
     double mjdr0, x[6], res[54];

     long   selected = PO_NONE;
     long   dummy_c  = PO_NONE;
     double dummy    = 0.;
     char   *dummy_file;
  
     /* Set error handling mode to SILENT */
     status = po_silent();             /* Set error handling mode to SILENT */
  
     /* Calling po_interpol (interpolation mode) */
     mode = PO_INTERPOLATE + PO_INTERPOL_RES_BAS + PO_INTERPOL_RES_AUX;
     mjdr0 = MJD;
     status = po_interpol( &mode    , &dummy_c  ,
			   &dummy_n , &dummy_file,
			   &dummy_n , &dummy_file,
			   &dummy_n , &dummy_file,
			   &mjdr0   , &dummy, mjdp, 
			   x/*state vector*/, pos, vel, acc,
			   &selected, res, ierr );
     if ( status != PO_OK ) {
	  long  nm;
	  long  func_id = PO_INTERPOL_ID;
	  char  msg[PO_MAX_COD][PO_MAX_STR];

	  (void) po_vector_msg( &func_id, ierr, &nm, msg );
	  if ( status == PO_ERR ) {
	       while ( --nm >= 0 ) 
		    NADC_ERROR( prognm, NADC_ERR_DORIS, msg[nm] );
	       return PO_ERR;
	  } else {
	       while ( --nm >= 0 ) 
		    NADC_ERROR( prognm, NADC_ERR_NONE, msg[nm] );
	  }
     }
     geo[0] = res[PO_PPFORB_RES_GEOC_LAT];                       /* latitude */
     geo[1] = res[PO_PPFORB_RES_GEOC_LONG];                     /* longitude */
     geo[2] = res[PO_PPFORB_RES_TRUE_SUN_SEMI_DIAM];   /* true Sun semi diam */
     geo[3] = res[PO_PPFORB_RES_MOON_AREA_LIT];  /* moon area lit by the Sun */

     return PO_OK;
}

/* Pointing code */

static
int GET_ATTITUDE( double MJD, /*@out@*/ double *aocs, /*@out@*/ double *att, 
		  /*@out@*/ double *datt )
{
     const char prognm[] = "GET_ATTITUDE";

     int   status;
     long  ierr[1];
     long  perfo_flag;
     char  mode[5];
    
     status = pp_get_attitude_aocs( &MJD, aocs, att, datt, &perfo_flag, 
				    mode, ierr);
     if ( status != PP_OK ) {
	  long  nm;
	  long  func_id = PP_GET_ATTITUDE_AOCS_ID;
	  char  msg[PP_MAX_COD][PP_MAX_STR];

	  (void) pp_vector_msg( &func_id, ierr, &nm, msg );
	  while ( --nm >= 0 ) 
	       NADC_ERROR( prognm, NADC_ERR_DORIS, msg[nm] );
	  if ( status <= PP_ERR ) return PP_ERR;
     }
     return PP_OK;
}

static 
int INITIALISE_AUX_FRA( const double MJD, double *time_valid )
{
     const char prognm[] = "INITIALISE_AUX_FRA";
    
     double  mjd_int, mjd_frac;
     glob_t  globvor[1];
     char    aux_fra_dir[]="/SCIA/share/Doris/aux_fra";
     char    globstring[MAX_STRING_LENGTH];
     char    aux_fra_file[MAX_STRING_LENGTH];
     char    year[5], month[3], day[3];
     char    tmpstring[UTC_STRING_LENGTH];
     int     status;
     long    ierr[1];

     /* Set error handling mode to SILENT */
     status = pp_silent();             /* Set error handling mode to SILENT */
/*
 *  Find right AUX_FRA filename
 */
     mjd_frac = modf( MJD, &mjd_int );
     mjd_frac *= 24.;
     mjd_frac *= 60 * 60;
     MJD_2_YMD( (int) mjd_int, mjd_frac, tmpstring );

     (void) nadc_strlcpy( year , &tmpstring[0], 5 );
     (void) nadc_strlcpy( month, &tmpstring[4], 3 );
     (void) nadc_strlcpy( day  , &tmpstring[6], 3 );

     (void) snprintf( globstring, MAX_STRING_LENGTH,
		      "%s/%4s/%2s/%2s/AUX_FRA_AX*",
		      aux_fra_dir, year, month, day );
     (void) glob( globstring, GLOB_ERR, NULL, globvor );
     if ( globvor->gl_pathc == 0 ) {
	  NADC_ERROR( prognm, NADC_ERR_NONE, "No AUX_FRA file found" );
	  return PP_WARN;
     }
     (void) nadc_strlcpy( aux_fra_file, globvor->gl_pathv[globvor->gl_pathc-1], 
			  MAX_STRING_LENGTH );
/*
 *  Initialise Envisat Attitude calculation using the AUX_FRA file
 */
     {
	  double perfo_params[6]={0,0,0,0,0,0};
	  double bias_good[3], rms_good[3], bias_flag[3], rms_flag[3];
	  double flag_stats[2];
	  double utcstart, utcstop;
	  long   mode_statistic, mode_out, mode_perfo;

	  mode_out   = PP_ATT_HARMONIC;
	  mode_perfo = PP_NO_PERFO;
	  mode_statistic = PP_NO_STATISTIC;
	  utcstart = mjd_int; utcstop=mjd_int+1.0;
	  status = pp_init_attitude_file( aux_fra_file, &mode_out, &utcstart, 
					  &utcstop, &mode_perfo, perfo_params, 
					  &mode_statistic, bias_good, rms_good,
					  bias_flag, rms_flag, flag_stats, 
					  ierr );
	  if ( status != 0 ) {
	       long  nm;
	       long  func_id = PP_INIT_ATTITUDE_FILE_ID;
	       char  msg[PP_MAX_COD][PP_MAX_STR];

	       (void) pp_vector_msg( &func_id, ierr, &nm, msg );
	       while ( --nm >= 0 ) 
		    NADC_ERROR( prognm, NADC_ERR_DORIS, msg[nm] );
	       if ( status <= PP_ERR ) return PP_ERR;
	  }
     }
     time_valid[0] = (double) mjd_int;
     time_valid[1] = (double) mjd_int + 1.;

     return PP_OK;
}

static 
int GET_ORBIT_PARAMETERS( double MJD, float esm_angle, 
			  /*@out@*/ bool   *sun_flag,
			  /*@out@*/ double *mjdp, 
			  /*@out@*/ float  *sun_az_out, 
			  /*@out@*/ float  *sun_el_out, 
			  /*@out@*/ double *geo )
{
     const char prognm[] = "GET_ORBIT_PARAMETERS";

     register int  ii;

     double pos[3], vel[3], acc[3];
     double aocs[3], att[3], datt[3], dir[8], freq, res[75];
     long status, idir, iray, ieres, ierr[15];

     const double los_az_angle = -90.;
     const double los_el_angle = esm_angle - ESM0 + 90.;

     const double scia_offset_manfred[3]={-0.026,-0.020, -0.21846};

     /*
      * see SCIAMACHY Extra Misalignment Model, PO-TN-DLR-SH-0016, 
      * Issue 2, 14 March 2008
      */
     double sun_az, sun_el;
     short  sun_vis_flag;

     //double moon_az, moon_el;
     //double sat_pitch, sat_roll, sat_yaw;
     //short  moon_vis_flag

     // compute results + target to sun and satellite to target pointinG
     ieres = PP_EXT_RES + PP_TARG_RES_TARG2SUN + PP_TARG_RES_SAT2TARG;
     idir  = PP_INTER_1ST; // first intersection
     iray  = PP_NO_REF;    // No refraction for ray tracing
  
     for ( ii = 0; ii < 3; ii++ ) {
          aocs[ii] = 0.0;
	  att[ii]  = 0.0;     /* Envisat-1 SRAR mispointing angles [deg] */
	  datt[ii] = 0.0;     /* Envisat-1 SRAR mispointing rates [deg/s] */
     }

     // TODO: I guess substituding esm angle for roll is pretty useless?
     aocs[0] = 0;
     aocs[1] = 0;
     aocs[2] = 0;

     status = INTERPOLATE_DORIS_GEO( MJD, geo, pos, vel, acc, mjdp );
     if ( status < 0 ) goto error;
    
     status = GET_ATTITUDE( MJD, aocs, att, datt );
     if ( status < 0 ) goto error;

     // Add pointing correction of Manfred
     for ( ii = 0; ii < 3; ii++ ) att[ii] = att[ii] + scia_offset_manfred[ii];

     /* Pass 1: geolocation */
     dir[0] = los_az_angle;
     dir[1] = los_el_angle;

     status = pp_target( mjdp, pos, vel, acc, aocs, att, datt, &idir, dir, 
			 &iray, &freq, &ieres, res, ierr );
     if ( status != PP_OK ) {
	  long  nm;
	  long  func_id = PP_TARGET_ID;
	  char  msg[PP_MAX_COD][PP_MAX_STR];

	  (void) pp_vector_msg( &func_id, ierr, &nm, msg );
          (void) printf("oh noes! pp_target error (pass 1: geoloc) [%d]!\n", 
			(int) *ierr);
	  while ( --nm >= 0 ) {
	       printf("%s\n", msg[nm]);
	       NADC_ERROR( prognm, NADC_ERR_DORIS, msg[nm] );
          }
	  if ( status <= PP_ERR ) goto error;
     }

     // overwrite sub-satellite point with intersection point
     geo[0]       = res[PP_TARG_RES_GEOC_LAT];
     geo[1]       = res[PP_TARG_RES_GEOC_LONG];

     /* Pass 2: sun angles */
     dir[0] = los_az_angle;
     dir[1] = los_el_angle;
     /* geodetic altitude, intended for single scattering (in meters) */
     dir[2] = 100000.; 
    /* TODO: what about the rates? */
    //dir[4] = los_az_rate;
    //dir[5] = los_el_rate;

     status = pp_target( mjdp, pos, vel, acc, aocs, att, datt, &idir, dir, 
			 &iray, &freq, &ieres, res, ierr );
     if ( status != PP_OK ) {
	  long  nm;
	  long  func_id = PP_TARGET_ID;
	  char  msg[PP_MAX_COD][PP_MAX_STR];

	  (void) pp_vector_msg( &func_id, ierr, &nm, msg );
          (void) printf("oh noes! pp_target error (pass 2: sun angles) [%d]!\n",
			(int) *ierr);
	  while ( --nm >= 0 ) {
	       printf("%s\n", msg[nm]);
	       NADC_ERROR( prognm, NADC_ERR_DORIS, msg[nm] );
          }
	  if ( status <= PP_ERR ) goto error;
     }

     sun_az        = res[PP_TARG_RES_TARG2SUN_AZ_TOP];
     sun_el        = res[PP_TARG_RES_TARG2SUN_EL_TOP];
     sun_vis_flag  = res[PP_TARG_RES_SAT2SUN_VIS_FLAG];
     //moon_az       = res[PP_TARG_RES_SAT2MOON_AZ_SRAR];
     //moon_el       = res[PP_TARG_RES_SAT2MOON_EL_SRAR];
     //moon_vis_flag = res[PP_TARG_RES_SAT2MOON_VIS_FLAG];
     //sat_pitch     = res[PP_TARG_RES_PITCH];
     //sat_roll      = res[PP_TARG_RES_ROLL];
     //sat_yaw       = res[PP_TARG_RES_YAW];

     *sun_az_out   = (float) sun_az;
     *sun_el_out   = (float) sun_el;
     *sun_flag     = (bool) sun_vis_flag;

     return PP_OK;
error:
     return PP_ERR;
}


/*+++++++ main module +++++++*/
static 
int SCIA_GET_ORBIT_PARAMS( double *mjds, float *esms, int n_mjds, float *lats, 
			   float *lons, float *sunels, float *sunazs )
{
     register int ii;

     int stat;

     double acc[3], geo[4], mjdp[2], pos[3], vel[3];
     float sun_az_out, sun_el_out;
     bool sun_flag, por_flag;

     //float sunSemiDiam, moonAreaSunlit;
     //unsigned char flags;
        
     double time_valid[2] = {0., 0.};
     double time_valid_aux_fra[2] = {0., 0.};

     (void) printf( "NOTE: lat lon at 0 km, sun angles at 100 km\n" );

     for ( ii = 0; ii < n_mjds; ii++ ) {

	  if (mjds[ii] > time_valid[1]) {
	       if ( INITIALISE_DORIS_GEO(mjds[ii], &por_flag, time_valid, mjdp) < 0 )
		    return -1;
	  }
    
	  if (mjds[ii] > time_valid_aux_fra[1]) {
	       if ( INITIALISE_AUX_FRA(mjds[ii], time_valid_aux_fra) < 0 )
		    return -1;
	  }
    
	  if ( INTERPOLATE_DORIS_GEO(mjds[ii], geo, pos, vel, acc, mjdp) < 0 )
	       return -1;
    
	  if ( esms[ii] < 360. ) {
	       stat = GET_ORBIT_PARAMETERS( mjds[ii], esms[ii], &sun_flag, mjdp, 
					    &sun_az_out, &sun_el_out, geo );
	  } else {
	       stat = GET_ORBIT_PARAMETERS( mjds[ii], ESM0, &sun_flag, mjdp, 
					    &sun_az_out, &sun_el_out, geo );
	  }
	  if ( stat < 0 ) return -1;
    
	  //latitude       = (float) geo[0];
	  //longitude      = (float) geo[1];
	  //sunSemiDiam    = (float) geo[2];
	  //moonAreaSunlit = (float) geo[3];
	  //flags = (unsigned char) NADC_CHECK_FOR_SAA( geo[0], geo[1] );
    
	  lats[ii]   = (float) geo[0];  
	  lons[ii]   = (float) geo[1]; 
	  sunels[ii] = sun_el_out;
	  sunazs[ii] = sun_az_out; 
    }
    return ii;
}

unsigned short IDL_STDCALL _SCIA_GET_ORBIT_PARAMS( int argc, void *argv[] )
{
    const char prognm[] = "_SCIA_GET_ORBIT_PARAMS";
    int n_mjds;
    double *mjds;
    float *lats, *lons, *sunels, *sunazs, *esms;

    if (argc != 7) {
        NADC_GOTO_ERROR(prognm, NADC_ERR_PARAM, "wrong nr of arguments!\n");
    }
    mjds     = (double *) argv[0];
    esms     = (float *) argv[1];
    n_mjds   = *(int *) argv[2];
    lats     = (float *) argv[3];
    lons     = (float *) argv[4];
    sunels   = (float *) argv[5];
    sunazs   = (float *) argv[6];

    return SCIA_GET_ORBIT_PARAMS( mjds, esms, n_mjds, lats, lons, sunels, sunazs );
done:
    return -1;
}
