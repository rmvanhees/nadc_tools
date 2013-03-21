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

.IDENTifer   GOME_PARAM
.AUTHOR      R.M. van Hees
.KEYWORDS    command-line parameter handling
.LANGUAGE    ANSI C
.PURPOSE     initializes param-structure with command-line parameters
.CONTAINS    GOME_SET_PARAM, GOME_SHOW_PARAM
.RETURNS     Nothing (check global error status)
.COMMENTS    None
.ENVIRONment None
.VERSION      7.0   09-Mar-2013	created GOME specific modules, RvH
              6.1.1 09-Jan-2013	added SDMF_SELECT_NRT, RvH
              6.1   20-May-2012	added SCIA_TRANS_RANGE, RvH
              6.0   10-Apr-2012	separated modules NADC_INIT_PARAM
                                and NADC_SET_PARAM, RvH
              5.9   20-Jun-2008	removed HDF4 support, RvH
              5.8   23-Nov-2006	added options for SDMF software, RvH
              5.7   23-Oct-2006	bugfix hdf4/5 request without HDF support, RvH
              5.6   17-Nov-2005	bugfix option no_pmd_geo and -meta (gome_lv1)
              5.5   11-Oct-2005	added options -pds and -pds_1b for SCIA L1, RvH
              5.4   14-Jul-2005	added option -meta for all processors, RvH
              5.3   23-Dec-2003	debugged MDS selection, RvH
              5.2   25-Aug-2003	added instrument equals SCIA_PATCH_1, RvH
              5.1   10-Feb-2003	bugfixes and added more parameters, RvH
              5.0   06-Feb-2003	changes to optional parameters (!), 
                                major rewrite of the code with 
                                improved parameter checking, RvH
              4.1   23-Jan-2003	more robuust against invalid input, RvH 
              4.0   12-Dec-2002	added option to calculate reflectance, RvH
              3.9   10-Nov-2002	implemented new Scia calib options, RvH 
              3.8   04-Oct-2002	new options for scia_ol2, RvH 
              3.7   18-Sep-2002	added geolocation selection to scia_nl1, RvH 
              3.6   10-Sep-2002	modified/added options to SCIA-progs, RvH
              3.5   04-Sep-2002	added option for alternative calibration 
                                algorithms (--altcal), RvH
              3.4   02-Aug-2002	added calibration option "B" 
                                (SCIA level 1), RvH
              3.3   02-Aug-2002	added selection on MDS (SCIA level 2), RvH 
              3.2   02-Aug-2002	added option "--info", RvH 
              3.1   02-Aug-2002	added category selection (SCIA level 1), RvH 
              3.0   02-Aug-2002	added clusters selection (SCIA level 1), RvH 
              2.9   17-Jun-2002	added calib options for SCIA data, RvH 
              2.8   21-Dec-2001	added more options for scia_lv0, RvH 
              2.7   21-Dec-2001	Added parameters for scia_lv0, RvH 
              2.6   28-Oct-2001	bugs in geolocation, RvH 
              2.5   25-Oct-2001 rename modules to NADC_..., RvH
              2.4   20-Sep-2001 changed options for output generation, RvH
              2.3   05-Sep-2001 compiles without HDF4/5 library, RvH
              2.2   07-Aug-2001 sequence of reading PMD options is important
              2.1   26-Jul-2001 improved help output, RvH
              2.0   01-May-2001 new outfile naming scheme, RvH
              1.5   21-Apr-2001 added HDF-4 options, RvH
              1.4   16-Nov-2000 improved help displayed with GIVE_HELP, RvH
              1.3   09-Nov-2000 set default to NO calibration, RvH
              1.2   11-Feb-2000 changed calibration selection conform 
                                  the DLR Extractor, RvH
              1.1   08-Feb-2000 handles various date-strings correctly, RvH
              1.0   20-May-1999 Created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*+++++ Local Headers +++++*/
#include <nadc_gome.h>
#include "../VERSION"

/*+++++ Static Variables +++++*/
static const
struct nadc_opt {
     /*@null@*/ const char *opt_key;
     /*@null@*/ const char *opt_def_val;
     const char     *opt_desc;
     unsigned short num_instr;
     int            id_instr[4];
} nadc_opts[] = {
/* general options */
     { "-h", NULL, "\tdisplay this help and exit [default]",
       0, {ALL_INSTR, 0, 0, 0} },
     { "-help", NULL, "display this help and exit",
       0, {ALL_INSTR, 0, 0, 0} },
     { "-V", NULL, "\tdisplay version & copyright information and exit",
       0, {ALL_INSTR, 0, 0, 0} },
     { "-version", NULL, "display version & copyright information and exit",
       0, {ALL_INSTR, 0, 0, 0} },
     { "-show_param", NULL, 
       "display setting of command-line parameters; no output generated",
       0, {ALL_INSTR, 0, 0, 0} },
     { "-verbose", NULL, "verbose mode",
       0, {ALL_INSTR, 0, 0, 0} },
     { "-silent", NULL, "do not display any error messages",
       0, {ALL_INSTR, 0, 0, 0} },
     { "-check", NULL, "check inputfile by reading it; no output generated",
       0, {ALL_INSTR, 0, 0, 0} },
     { "-meta", NULL, 
       "write (in ASCII format) NL-SCIA-DC meta-Database information",
       0, {ALL_INSTR, 0, 0, 0} },
     { "-ascii", NULL, "write output in ASCII format",
       0, {ALL_INSTR, 0, 0, 0} },
#if defined(_WITH_SQL)
     { "-sql", NULL, "\twrite to PostgreSQL database",
       0, {ALL_INSTR, 0, 0, 0} },
     { "-remove", NULL, "removes current entry of product from SQL database",
       0, {ALL_INSTR, 0, 0, 0} },
     { "-replace", NULL, "replace current entry of product in SQL database",
       0, {ALL_INSTR, 0, 0, 0} },
#endif
     { "-hdf5", NULL, "generate a HDF5 file",
       0, {ALL_INSTR, 0, 0, 0} },
     { "-compress", NULL, "compress data sets in HDF5-file",
       0, {ALL_INSTR, 0, 0, 0} },
/* MDS selection */
     { "-no_mds", NULL, "do not extract Measurement Data Sets (MDS)",
       1, {GOME_LEVEL_1, 0, 0, 0} },
     { "-no_pmd", NULL, "do not extract PMD MDS records",
       1, {GOME_LEVEL_1, 0, 0, 0} },
     { "-no_pmd_geo", NULL, "do not extract PMD geolocations produced",
       1, {GOME_LEVEL_1, 0, 0, 0} },
     { "-moon", NULL, "extract Moon MDS records",
       1, {GOME_LEVEL_1, 0, 0, 0} },
     { "-no_moon", NULL, "do not extract Moon MDS records",
       1, {GOME_LEVEL_1, 0, 0, 0} },
     { "-sun", NULL, "\textract Sun MDS records",
       1, {GOME_LEVEL_1, 0, 0, 0} },
     { "-no_sun", NULL, "do not extract Sun MDS records",
       1, {GOME_LEVEL_1, 0, 0, 0} },
/* spectral band/channel selection */
     { "-blind", NULL, "\twrite Blind pixel data records",
       1, {GOME_LEVEL_1, 0, 0, 0} },
     { "-stray", NULL, "\twrite Straylight data records",
       1, {GOME_LEVEL_1, 0, 0, 0} },
     { "--band", "[=1a,1b,...,4]", "write data of selected spectral bands",
       1, {GOME_LEVEL_1, 0, 0, 0} },
/* pixel selection */
     { "--ipixel", "=min:max:step,...", "pixel number (default: all)",
       0, {ALL_INSTR, 0, 0, 0} },
     { "--xpixel", "=[ECWB]", "select ground pixels (default: all)",
       1, {GOME_LEVEL_1, 0, 0, 0} },
     { "--wave", "=min,max", "apply selection on wavelength range",
       1, {GOME_LEVEL_1, 0, 0, 0} },
/* time selection */
     { "--time", " start_date start_time [end_date] end_time", 
       "apply time-window",
       0, {ALL_INSTR, 0, 0, 0} },
/* geolocation selection */
     { "--region", "=lat_min,lat_max,lon_min,lon_max",
       "apply selection on geolocation", 
       0, {ALL_INSTR, 0, 0, 0} },
/* MDS calibration */
     { "--cal_nadir", "[=LAFSNPI]", "apply spectral calibration on Nadir MDS",
       1, {GOME_LEVEL_1, 0, 0, 0} },
     { "--cal_moon", "[=LFSNI]", "\tapply spectral calibration on Moon MDS",
       1, {GOME_LEVEL_1, 0, 0, 0} },
     { "--cal_sun", "[=LFSNBI]", "\tapply spectral calibration on Sun MDS",
       1, {GOME_LEVEL_1, 0, 0, 0} },
     { "--cal_pmd", "[=LPI]", "\tapply spectral calibration on PMD MDS",
       1, {GOME_LEVEL_1, 0, 0, 0} },
/* observation selection */
     { "--cloud", "=min,max", "apply selection on cloud cover",
       1, {GOME_LEVEL_2, 0, 0, 0} },
     { "--sunz", "=min,max", "apply selection on Solar zenith angle",
       1, {GOME_LEVEL_2, 0, 0, 0} },
/* output filename */
     { "-o", " <outfile>", "(default: <infile> + appropriate extension)",
       0, {ALL_INSTR, 0, 0, 0} },
     { "--output", "=<outfile>", "(default: <infile> + appropriate extension)",
       0, {ALL_INSTR, 0, 0, 0} },
/* last and empty entry */
     { NULL, NULL, "", 0, {0, 0, 0, 0} }
};

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   Show_All_Options
.PURPOSE     display (optional) command parameters
.INPUT/OUTPUT
  call as   Show_All_Options( stream, instrument, prognm );
     input:  
            FILE *stream      :   open (file) stream pointer
	    int instrument    :   code for instrument en data product level
	    char prognm[]     :   string with name of the master program

.RETURNS     exits with EXIT_FAILURE
.COMMENTS    static function
-------------------------*/
static /*@exits@*/ 
void Show_All_Options( FILE *stream, int instrument, 
		       /*@notnull@*/ const char prognm[] )
     /*@modifies stream@*/
{
     register unsigned short ni;

     register short nr;

     register bool  found;
/*
 * intro of message
 */
     (void) fprintf( stream, "Usage: %s [OPTIONS] FILE\n\n", prognm );
/*
 * display all available NADC options
 */
     nr = -1;
     while ( nadc_opts[++nr].opt_key != NULL ) {
	  found = FALSE;
	  for ( ni = 0; ni < nadc_opts[nr].num_instr; ni++ ) {
	       if ( instrument == nadc_opts[nr].id_instr[ni] ) {
		    found = TRUE;
		    break;
	       }
	  }
	  if ( found || nadc_opts[nr].num_instr == 0 ) {
	       (void) fprintf( stream, "   %s", nadc_opts[nr].opt_key );
	       if ( nadc_opts[nr].opt_def_val != NULL )
		    (void) fprintf( stream, "%s", nadc_opts[nr].opt_def_val );
	       (void) fprintf( stream, "\t%s\n", nadc_opts[nr].opt_desc );
	  }
     }
/*
 * show instrument specific options
 */
     if ( instrument == GOME_LEVEL_1 ) {
	  (void) fprintf( stream, "\nRecognised calibration options are:\n" );
	  GOME_SHOW_CALIB( stream );
     }
     exit( EXIT_FAILURE );
}

/*+++++++++++++++++++++++++
.IDENTifer   Check_User_Option
.PURPOSE     check command-line parameters is valid given instrument/prognm
.INPUT/OUTPUT
  call as   Check_User_Option( stream, instrument, prognm, argv );

     input:
            FILE *stream      :   open (file) stream pointer for error-mesg
	    int instrument    :   code for instrument en data product level
	    char prognm[]     :   string with name of the master program
	    const char argv[] :   command-line parameter

.RETURNS     nothing, exits on failure
.COMMENTS    static function
-------------------------*/
static
void Check_User_Option( FILE *stream, int instrument,
			/*@notnull@*/ const char prognm[],
			/*@notnull@*/ const char argv[] )
     /*@modifies stream@*/
{
     register unsigned short ni;

     register short nr = -1;
/*
 * catch the string with the name of the input file
 */
     if ( argv[0] != '-' ) return;
/*
 * check options
 */
     while ( nadc_opts[++nr].opt_key != NULL ) {
	  size_t opt_len = strlen( nadc_opts[nr].opt_key );

	  if ( strncmp( argv, nadc_opts[nr].opt_key, opt_len ) == 0 ) {
	       if ( nadc_opts[nr].num_instr == 0 ) return;

	       for ( ni = 0; ni < nadc_opts[nr].num_instr; ni++ ) {
		    if ( instrument == nadc_opts[nr].id_instr[ni] )
			 return;
	       }
	  }
     }
     (void) fprintf( stream, 
		     "%s: FATAL, unknown command-line option \"%s\"\n", 
		     prognm, argv );
     exit( EXIT_FAILURE );
}

/*+++++++++++++++++++++++++
.IDENTifer   Conv_Date
.PURPOSE     convert Date string containing 01, 02,... to Jan, Feb,...
.INPUT/OUTPUT
  call as   Conv_Date( in_date, out_date );

     input:  
            char in_date[]   :  one of DD-MMM-YYYY, YYYY-MM-DD, 
                                       DD-MM-YYYY, YYYY-MM-DD
    output:  
            char out_date[]  :  DD-MMM-YYYY

.RETURNS     error status (0=ok)
.COMMENTS    static function
-------------------------*/
static inline
int Conv_Date( const char in_date[], /*@out@*/ char out_date[] )
{
     char *pntr, day[3], mon[4], year[5];

     (void) sprintf( out_date, "UNKOWN" );
     if ( strlen( in_date ) > DATE_ONLY_STRING_LENGTH )
	  return NADC_ERR_FATAL;

     if ( (pntr = strchr( in_date, '-' )) != NULL ) {
	  if ( (pntr - in_date) == 4 ) 
	       (void) strlcpy( year, in_date, 5 );
	  else if ( (pntr - in_date) == 2 )
	       (void) strlcpy( day, in_date, 3 );
	  else
	       return NADC_ERR_FATAL;
	  in_date = pntr + 1;
     }
     if ( (pntr = strchr( in_date, '-' )) != NULL ) {
	  if ( (pntr - in_date) == 3 ) {
	       (void) strlcpy( mon, in_date, 4 );
	  } else if ( (pntr - in_date) == 2 ) {
	       int mon_num;
	       const char *mon_str[] =
		    { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", 
		      "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

	       mon_num = atoi( strncpy( mon, in_date, 2 ));
	       if ( mon_num > 0 && mon_num <= 12 )
		    (void) strcpy( mon, mon_str[mon_num-1] );
	       else
		    return NADC_ERR_FATAL;
	  } else
	       return NADC_ERR_FATAL;
	  in_date = pntr + 1;
     }
     if ( strlen( in_date ) == 2 )
	  (void) strlcpy( day, in_date, 3 );
     else if ( strlen( in_date ) == 4 )
	  (void) strlcpy( year, in_date, 5 );
     else
	  return NADC_ERR_FATAL;

     (void) snprintf( out_date, 12, "%s-%s-%s", day, mon, year );
     return NADC_ERR_NONE;
}

/*+++++++++++++++++++++++++
.IDENTifer   Set_Time_Window
.PURPOSE     
.INPUT/OUTPUT
  call as   Set_Time_Window( argc, argv[], &narg, bgn_date, end_date );
     input:  
            int argc
	    char *argv[]
 in/output:
	    int *narg
    output:  
	    char *bgn_date
            char *end_date

.RETURNS     nothing (check global error status)
.COMMENTS    static function
-------------------------*/
static
void Set_Time_Window( int argc, char *argv[], int *narg,
		      /*@out@*/ char *bgn_date, /*@out@*/ char *end_date )
{
     const char prognm[] = "Set_Time_Window";

     int num;
/*
 * first parameter has to be a Date-string
 */
     *bgn_date = *end_date = '\0';
     if ( ++(*narg) >= argc ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PARAM, argv[(*narg)] );
     if ( strchr( argv[(*narg)], '-' ) == NULL ||
	  Conv_Date( argv[(*narg)], bgn_date ) != NADC_ERR_NONE )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PARAM, argv[(*narg)] );
/*
 * second parameter has to be a Time-string (always HH:MM:SS[.SSS])
 */
     if ( ++(*narg) >= argc ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PARAM, argv[(*narg)] );
     if ( strchr( argv[(*narg)], ':' ) != NULL ) {
	  (void) strcat( bgn_date, " " );
	  num = (int) strlen( argv[(*narg)] );
	  if ( num <= TIME_ONLY_STRING_LENGTH )
	       (void) strcat( bgn_date, argv[(*narg)]);
	  else
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PARAM, argv[(*narg)] );
     } else
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PARAM, argv[(*narg)] );
/*
 * third parameter can be a Time-string, then duplicate previous Date-string
 * otherwise the third parameter has to be Date-string, 
 * and fourth parameter has to be Time-string
 */
     if ( ++(*narg) >= argc )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PARAM, argv[(*narg)] );
     if ( strchr( argv[(*narg)], ':' ) != NULL ) {
	  (void) Conv_Date( argv[(*narg)-2], end_date);
	  (void) strcat( end_date, " " );
	  num = (int) strlen( argv[(*narg)] );
	  if ( num <= TIME_ONLY_STRING_LENGTH )
	       (void) strcat( end_date, argv[(*narg)]);
	  else
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PARAM, argv[(*narg)] );
     } else if ( strchr( argv[(*narg)], '-' ) != NULL ) {
	  if ( Conv_Date( argv[(*narg)], end_date ) \
	       != NADC_ERR_NONE ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PARAM, argv[(*narg)] );
	  (void) strcat( end_date, " " );
	  if ( ++(*narg) >= argc ) 
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PARAM, argv[(*narg)] );
	  num = (int) strlen( argv[(*narg)] );
	  if ( strchr( argv[(*narg)], ':' ) != NULL &&
	       num <= TIME_ONLY_STRING_LENGTH )
	       (void) strcat( end_date, argv[(*narg)]);
	  else
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PARAM, argv[(*narg)] );
     } else
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PARAM, argv[(*narg)] );
}

/*+++++++++++++++++++++++++
.IDENTifer   Set_Band_Val
.PURPOSE     decode string of characters to a mask
.INPUT/OUTPUT
  call as   band_mask = Set_Band_Val( band_str );

     input:
            char band_str[] : string spectral band numbers

.RETURNS     spectral band mask
.COMMENTS    static function
-------------------------*/
static inline
unsigned char Set_Band_Val( /*@notnull@*/ const char band_str[] )
{
     unsigned char band_mask = BAND_NONE;

     if ( strstr( band_str, "1a" ) != NULL )
	  band_mask |= BAND_ONE_A;
     if ( strstr( band_str, "1b" ) != NULL )
	  band_mask |= BAND_ONE_B;
     if ( strstr( band_str, "2a" ) != NULL )
	  band_mask |= BAND_TWO_A;
     if ( strstr( band_str, "2b" ) != NULL )
	  band_mask |= BAND_TWO_B;
     if ( strstr( band_str, "3" ) != NULL )
	  band_mask |= BAND_THREE;
     if ( strstr( band_str, "4" ) != NULL )
	  band_mask |= BAND_FOUR;

     return band_mask;
}

/*+++++++++++++++++++++++++
.IDENTifer   Do_Not_Extract_MDS
.PURPOSE     set user-defined settings not to write MDS data
.INPUT/OUTPUT
  call as   Do_Not_Extract_MDS( instrument, param );
     input:
            int instrument          : code for instrument en data product level
	    struct param_record *param : struct holding user-defined settings

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
void Do_Not_Extract_MDS( int instrument, struct param_record *param )
{
     if ( instrument == GOME_LEVEL_1 ) {
	  param->write_moon  = PARAM_UNSET;
	  param->write_nadir = PARAM_UNSET;
	  param->write_sun   = PARAM_UNSET;
	  param->chan_mask   = BAND_NONE;

	  param->calib_pmd   = GOME_CAL_PMD;
     }
     param->write_pmd     = PARAM_UNSET;
     param->write_pmd_geo = PARAM_UNSET;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   GOME_SET_PARAM
.PURPOSE     initializes struct with command-line parameters
.INPUT/OUTPUT
  call as   GOME_SET_PARAM( argc, argv, instrument, param );
     input:  
             int  argc       :   number of parameters
	     char *argv[]    :   parameter values
	     int instrument  :   code for instrument en data product level
    output:  
	     struct param_record 
	            *param   :     struct holding user-defined settings

.RETURNS     Nothing (check global error status)
.COMMENTS    none
-------------------------*/
void GOME_SET_PARAM( int argc, char *argv[], int instrument,
		     struct param_record *param )
{
     const char prognm[] = "GOME_SET_PARAM";

     char   *cpntr;
     char   outfile[MAX_STRING_LENGTH];
     char   prog_master[SHORT_STRING_LENGTH];
     int    narg, num;
     float  rbuff[4];

     bool select_mds = FALSE;
/*
 * initialise param-structure
 */
     NADC_INIT_PARAM( param );
     (void) snprintf( param->program, MAX_STRING_LENGTH, "%s", argv[0] );
/*
 * check number of options
 */
     if ( argc == 0 || argv[0] == NULL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PARAM, "none found!?!" );
/*
 * strip path to program
 */
     if ( (cpntr = strrchr( argv[0], '/' )) != NULL ) {
	  (void) strlcpy( prog_master, ++cpntr, SHORT_STRING_LENGTH );
     } else {
	  (void) strlcpy( prog_master, argv[0], SHORT_STRING_LENGTH );
     }
/*
 * get command-line parameters
 */
     narg = 0;
     while ( ++narg < argc ) {
	  Check_User_Option( stderr, instrument, prog_master, argv[narg] );
/*
 * obtain name of input file
 */
	  if ( argv[narg][0] != '-' ) {
	       if ( param->flag_infile == PARAM_UNSET ) {
		    if ( strlen(argv[narg]) < MAX_STRING_LENGTH ) {
			 (void) strcpy( param->infile, argv[narg] );
		    } else {
			 char cbuff[MAX_STRING_LENGTH];

			 (void) snprintf( cbuff, MAX_STRING_LENGTH,
					 "Filename too long (max: %d)\n",
					 (int) MAX_STRING_LENGTH );
			 
			 NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, cbuff );
		    }
		    param->flag_infile = PARAM_SET;
	       } else {
		    NADC_RETURN_ERROR( prognm, NADC_ERR_PARAM, argv[narg] );
	       }
/*
 * process command-line options starting with one "-" (= standalone options)
 */
	  } else if ( argv[narg][0] == '-' && argv[narg][1] != '-' ) {
	       if ( (argv[narg][1] == 'h' && argv[narg][2] == '\0')
		    || strncmp( argv[narg]+1, "help", 4 ) == 0 )
		    Show_All_Options( stdout, instrument, prog_master );
	       if ( argv[narg][1] == 'V' 
		    || strncmp( argv[narg]+1, "version", 7 ) == 0 ) {
		    param->flag_version = PARAM_SET;
		    return;                             /* nothing else todo */
	       }
	       if ( strncmp( argv[narg]+1, "show", 4 ) == 0 ) {
		    param->flag_show = PARAM_SET;
		    param->write_ascii = PARAM_SET;
	       } else if ( strncmp( argv[narg]+1, "silent", 6 ) == 0 ) {
		    param->flag_silent = PARAM_SET;
	       } else if ( strncmp( argv[narg]+1, "verbose", 7 ) == 0 ) {
		    param->flag_verbose = PARAM_SET;
	       } else if ( strncmp( argv[narg]+1, "check", 5 ) == 0 ) {
		    param->flag_check = PARAM_SET;
	       } else if ( strncmp( argv[narg]+1, "meta", 4 ) == 0 ) {
		    param->write_meta = PARAM_SET;
	       } else if ( strncmp( argv[narg]+1, "ascii", 5 ) == 0 ) {
		    param->write_ascii = PARAM_SET;
	       } else if ( strncmp( argv[narg]+1, "sql", 3 ) == 0 ) {
#if defined(_WITH_SQL)
		    param->write_sql = PARAM_SET;
		    if ( instrument == GOME_LEVEL_1 ) {
			 param->calib_pmd = GOME_CAL_PMD;
			 param->write_pmd_geo = PARAM_UNSET;
		    }
#else
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, 
				"no PostgreSQL support, recompile" );
#endif
	       } else if ( strncmp( argv[narg]+1, "remove", 6 ) == 0 ) {
		    param->flag_sql_remove = PARAM_SET;
	       } else if ( strncmp( argv[narg]+1, "replace", 7 ) == 0 ) {
		    param->flag_sql_replace = PARAM_SET;
	       } else if ( strncmp( argv[narg]+1, "hdf5", 4 ) == 0 ) {
		    param->write_hdf5 = PARAM_SET;
	       } else if ( strncmp( argv[narg]+1, "compress", 8 ) == 0 ) {
		    param->flag_deflate = PARAM_SET;
/*
 * selection on all kind of data sets
 */
	       } else if ( strncmp( argv[narg]+1, "no_mds", 6 ) == 0 ) {
		    if ( ! select_mds ) {
			 select_mds = TRUE;
			 Do_Not_Extract_MDS( instrument, param );
		    }
	       } else if ( strncmp( argv[narg]+1, "moon", 4 ) == 0 ) {
		    if ( ! select_mds ) {
			 select_mds = TRUE;
			 Do_Not_Extract_MDS( instrument, param );
		    }
		    param->write_moon = PARAM_SET;
	       } else if ( strncmp( argv[narg]+1, "sun", 3 ) == 0 ) {
		    if ( ! select_mds ) {
			 select_mds = TRUE;
			 Do_Not_Extract_MDS( instrument, param );
		    }
		    param->write_sun = PARAM_SET;
	       } else if ( strncmp( argv[narg]+1, "no_pmd_geo", 10 ) == 0 ) {
		    param->write_pmd_geo = PARAM_UNSET;
	       } else if ( strncmp( argv[narg]+1, "no_pmd", 6 ) == 0 ) {
		    param->write_pmd = PARAM_UNSET;
		    param->write_pmd_geo = PARAM_UNSET;
	       } else if ( strncmp( argv[narg]+1, "no_moon", 7 ) == 0 ) {
		    param->write_moon = PARAM_UNSET;
	       } else if ( strncmp( argv[narg]+1, "no_sun", 6 ) == 0 ) {
		    param->write_sun = PARAM_UNSET;
	       } else if ( strncmp( argv[narg]+1, "blind", 5 ) == 0 ) {
		    param->write_blind = PARAM_SET;
	       } else if ( strncmp( argv[narg]+1, "stray", 5 ) == 0 ) {
		    param->write_stray = PARAM_SET;
	       } 
	  } else if ( argv[narg][0] == '-' && argv[narg][1] == '-' ) {
	       if ( strncmp( argv[narg]+2, "band", 4 ) == 0 ) {
		    if ( (cpntr = strchr( argv[narg], '=' )) == NULL ) { 
			 param->chan_mask = BAND_NONE;
		    } else {
			 param->chan_mask = Set_Band_Val( ++cpntr );
			 if ( param->chan_mask == BAND_NONE ) {
			      NADC_RETURN_ERROR( prognm, 
						 NADC_ERR_PARAM, argv[narg] );
			 }
		    }
	       } else if ( strncmp( argv[narg]+2, "ipixel", 6 ) == 0 ) {
		    if ( param->flag_pselect == PARAM_UNSET 
			 && (cpntr = strchr( argv[narg], '=' )) != NULL ) {
                         if ( strlen( ++cpntr ) < SHORT_STRING_LENGTH ) {
                              (void) strcpy( param->pselect, cpntr );
                         } else {
                              char cbuff[SHORT_STRING_LENGTH];

                              (void) snprintf( cbuff, SHORT_STRING_LENGTH,
                                       "Pixel selection too long (max: %d)\n",
                                        (int) SHORT_STRING_LENGTH );
                              NADC_RETURN_ERROR( prognm,NADC_ERR_FATAL,cbuff );
                         }
                         param->flag_pselect = PARAM_SET;
                    }
	       } else if ( strncmp( argv[narg]+2, "xpixel", 6 ) == 0 ) {
		    if ( param->flag_subset == PARAM_UNSET 
			 && (cpntr = strchr( argv[narg], '=' )) != NULL ) {
			 param->flag_subset = PARAM_SET;
                         param->write_subset = SUBSET_NONE;

			 if ( strchr( cpntr, 'E' ) != NULL )
			      param->write_subset += SUBSET_EAST;

			 if ( strchr( cpntr, 'C' ) != NULL )
			      param->write_subset += SUBSET_CENTER;

			 if ( strchr( cpntr, 'W' ) != NULL )
			      param->write_subset += SUBSET_WEST;

			 if ( strchr( cpntr, 'B' ) != NULL )
			      param->write_subset += SUBSET_BACK;
		    }
		    if ( param->write_subset == SUBSET_NONE )
			 NADC_RETURN_ERROR(prognm, NADC_ERR_PARAM, argv[narg]);
	       } else if ( strncmp( argv[narg]+2, "wave", 4 ) == 0 ) {
		    if ( param->flag_wave == PARAM_UNSET 
			 && (cpntr = strchr( argv[narg], '=' )) != NULL ) {
                         (void) NADC_USRINP( FLT32_T, cpntr+1, 
                                             2, param->wave, &num );
                         param->flag_wave = PARAM_SET;
                    }
	       } else if ( strncmp( argv[narg]+2, "time", 4 ) == 0 ) {
		    if ( param->flag_period == PARAM_UNSET ) {
			 param->flag_period = PARAM_SET;
			 Set_Time_Window( argc, argv, &narg, 
					  param->bgn_date, param->end_date );
			 if ( IS_ERR_STAT_FATAL )
			      NADC_RETURN_ERROR(prognm, NADC_ERR_PARAM, "");
		    }
	       } else if ( strncmp( argv[narg]+2, "region", 6 ) == 0 ) {
		    if ( param->flag_geoloc == PARAM_UNSET
			 && (cpntr = strchr( argv[narg], '=' )) != NULL ) {

			 (void) NADC_USRINP( FLT32_T, cpntr+1, 4, rbuff, &num);
			 if ( num == 4 ) {
			      param->geo_lat[0] = 
				   min_t( float, rbuff[0], rbuff[1] );
			      param->geo_lat[1] = 
				   max_t( float, rbuff[0], rbuff[1] );

			      if ( rbuff[2] < rbuff[3] )
				   param->flag_geomnmx = PARAM_SET;
			      else
				   param->flag_geomnmx = PARAM_UNSET;
			      param->geo_lon[0] = 
				   min_t( float, rbuff[2], rbuff[3] );
			      param->geo_lon[1] = 
				   max_t( float, rbuff[2], rbuff[3] );
			 } else {
			      NADC_RETURN_ERROR( prognm, 
						 NADC_ERR_PARAM, argv[narg] );
			 }
			 param->flag_geoloc = PARAM_SET;
		    }
               } else if ( strncmp( argv[narg]+2, "cloud", 5 ) == 0 ) {
                    if ( param->flag_cloud == PARAM_UNSET 
                         && (cpntr = strchr( argv[narg], '=' )) != NULL ) {
                         (void) NADC_USRINP( FLT32_T, ++cpntr, 
                                             2, param->cloud, &num );
                         param->flag_cloud = PARAM_SET;
                    } else {
                         NADC_RETURN_ERROR(prognm, NADC_ERR_PARAM, argv[narg]);
                    }
               } else if ( strncmp( argv[narg]+2, "sunz", 4 ) == 0 ) {
                    if ( param->flag_sunz == PARAM_UNSET 
                         && (cpntr = strchr( argv[narg], '=' )) != NULL ) {
                         (void) NADC_USRINP( FLT32_T, ++cpntr, 
                                             2, param->sunz, &num );
                         param->flag_sunz = PARAM_SET;
                    } else
                         NADC_RETURN_ERROR(prognm, NADC_ERR_PARAM, argv[narg]);
/*
 * selected calibration steps
 */
	       } else if ( strncmp( argv[narg]+2, "cal_nadir", 9 ) == 0 ) {
		    if ( (cpntr = strchr( argv[narg], '=' )) == NULL )
			 param->calib_earth = GOME_CAL_EARTH;
		    else
			 GOME_SET_CALIB( cpntr+1, &param->calib_earth );
	       } else if ( strncmp( argv[narg]+2, "cal_moon", 8 ) == 0 ) {
		    if ( (cpntr = strchr( argv[narg], '=' )) == NULL )
			 param->calib_moon = GOME_CAL_MOON;
		    else
			 GOME_SET_CALIB( cpntr+1, &param->calib_moon );
	       } else if ( strncmp( argv[narg]+2, "cal_sun", 7 ) == 0 ) {
		    if ( (cpntr = strchr( argv[narg], '=' )) == NULL )
			 param->calib_sun = GOME_CAL_SUN;
		    else
			 GOME_SET_CALIB( cpntr+1, &param->calib_sun );
	       } else if ( strncmp( argv[narg]+2, "cal_pmd", 7 ) == 0 ) {
		    if ( (cpntr = strchr( argv[narg], '=' )) == NULL )
			 param->calib_pmd = GOME_CAL_PMD;
		    else
			 GOME_SET_CALIB(cpntr+1, &param->calib_pmd );
	       } else if ( strncmp( argv[narg]+2, "cal", 3 ) == 0 ) {
		    if ( (cpntr = strchr( argv[narg], '=' )) == NULL ) {
			 param->calib_earth = GOME_CAL_EARTH;
			 param->calib_moon = GOME_CAL_MOON;
			 param->calib_sun = GOME_CAL_SUN;
		    } else {
			 GOME_SET_CALIB(cpntr+1, &param->calib_sun );
			 param->calib_earth = param->calib_sun;
			 param->calib_moon = param->calib_sun;
		    }
	       }
	  }
	  if ( strncmp( argv[narg], "-o", 2 ) == 0 
	       || strncmp( argv[narg], "--output=", 9 ) == 0 ) {
	       while ( ++narg < argc && argv[narg][0] == '-' );
	       if ( ++narg >= argc 
		    || argv == NULL || argv[narg][0] == '-' ) 
		    NADC_RETURN_ERROR(prognm, NADC_ERR_PARAM, argv[narg]);

	       if ( param->flag_outfile == PARAM_UNSET ) {
		    (void) snprintf( outfile, MAX_STRING_LENGTH, 
				     "%s", argv[narg] );
		    if ( (cpntr = strstr( outfile, ".h5" )) != NULL )
			 *cpntr = '\0';
		    if ( (cpntr = strstr( outfile, ".hdf" )) != NULL )
			 *cpntr = '\0';
		    if ( (cpntr = strstr( outfile, ".txt" )) != NULL )
			 *cpntr = '\0';
		    if ( (cpntr = strstr( outfile, ".child" )) != NULL )
			 *cpntr = '\0';
		    param->flag_outfile = PARAM_SET;
	       }
	  }
     }
     if ( param->flag_check == PARAM_SET ) {
	  param->write_pds   = PARAM_UNSET;
	  param->write_hdf5  = PARAM_UNSET;
	  param->write_sql   = PARAM_UNSET;
	  param->write_ascii = PARAM_UNSET;
     } else if ( param->write_sql == PARAM_SET ) {
	  param->write_pds   = PARAM_UNSET;
	  param->write_hdf5  = PARAM_UNSET;
	  param->write_ascii = PARAM_UNSET;
     } else if ( param->write_meta == PARAM_SET ) {
	  param->write_pds   = PARAM_UNSET;
	  param->write_hdf5  = PARAM_UNSET;
	  param->write_ascii = PARAM_SET;
	  Do_Not_Extract_MDS( instrument, param );
     } else {
	  if ( param->write_pds == PARAM_UNSET 
	       && param->write_hdf5 == PARAM_UNSET 
	       && param->write_ascii == PARAM_UNSET ) {
	       param->write_hdf5 = PARAM_SET;
	  }
     }
/*
 * User has to give the name of the input filename
 */
     if ( param->flag_infile == PARAM_UNSET ) 
	  Show_All_Options( stderr, instrument, prog_master );
/*
 * set output filename, if required
 */
     if ( param->write_ascii == PARAM_SET ) {
	  if ( param->flag_outfile == PARAM_UNSET ) {
	       char *pntr = strrchr( param->infile, '/' );

	       if ( pntr != NULL ) {
		    (void) snprintf( param->outfile, MAX_STRING_LENGTH,
				     "%s", pntr+1 );
	       } else
		    (void) snprintf( param->outfile, MAX_STRING_LENGTH,
				     "%s", param->infile );
	  } else {
	       (void) snprintf( param->outfile, MAX_STRING_LENGTH,
				"%s", outfile );
	  }
     }
     if ( param->write_hdf5 == PARAM_SET ) {
	  if ( param->flag_outfile == PARAM_UNSET ) {
	       char *pntr = strrchr( param->infile, '/' );

	       if ( pntr != NULL )
		    (void) snprintf( param->hdf5_name, MAX_STRING_LENGTH,
				     "%s.%s", pntr+1, "h5" );
	       else
		    (void) snprintf( param->hdf5_name, MAX_STRING_LENGTH,
				     "%s.%s", param->infile, "h5" );
	  } else 
	       (void) snprintf( param->hdf5_name, MAX_STRING_LENGTH,
				"%s.%s", outfile, "h5" );
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   GOME_SHOW_PARAM
.PURPOSE     show command-line settings, as stored in the param-record
.INPUT/OUTPUT
  call as   GOME_SHOW_PARAM( instrument, param );
     input:  
             int instrument     :   code for instrument en data product level
	     struct param_record 
                          param :   struct holding user-defined settings

.RETURNS     Nothing (check global error status)
.COMMENTS    none
-------------------------*/
void GOME_SHOW_PARAM( int instrument, const struct param_record param )
{
     register unsigned int nr = 0;

     char    cbuff[MAX_STRING_LENGTH];
     char    string[SHORT_STRING_LENGTH];
     time_t  tp[1];
     unsigned int dims[2];

     FILE *outfl = stdout;
/*
 * show program name and version
 */
     (void) snprintf( string, SHORT_STRING_LENGTH, "%s (version %-d.%-d.%-d)", 
		      param.program, nadc_vers_major, nadc_vers_minor, 
		      nadc_vers_release );
     nadc_write_text( outfl, ++nr, "Program", string );
/*
 * show time of call
 */
     (void) time( tp );
     (void) strlcpy( cbuff, ctime( tp ), MAX_STRING_LENGTH );
     cbuff[strlen(cbuff)-1] = '\0';
     nadc_write_text( outfl, ++nr, "ProcessingDate", cbuff );
/*
 * show output files
 */
     nadc_write_text( outfl, ++nr, "InputFilename", param.infile );
     if ( param.write_ascii == PARAM_SET )
	  nadc_write_text( outfl, ++nr, "OutputFilename", param.outfile );
     if ( param.write_hdf5 == PARAM_SET ) {
	  nadc_write_text( outfl, ++nr, "Output filename", param.hdf5_name );
	  if ( param.flag_deflate == PARAM_SET )
	       nadc_write_text( outfl, ++nr, "Compression", "True" );
	  else
	       nadc_write_text( outfl, ++nr, "Compression", "False" );
     }
/*
 * ----- General options
 */
     if ( param.flag_silent == PARAM_SET )
	  nadc_write_text( outfl, ++nr, "Silent", "True" );
     else
	  nadc_write_text( outfl, ++nr, "Silent", "False" );
/*
 * time window
 */
     if ( param.flag_period == PARAM_SET ) {
	  nadc_write_text( outfl, ++nr, "StartDate", param.bgn_date );
	  nadc_write_text( outfl, ++nr, "EndDate", param.end_date );
     }

     switch ( instrument ) {
/*
 * ----- GOME level 1b processor specific options
 */
     case GOME_LEVEL_1:
/*
 * geolocation (bounding box)
 */
	  if ( param.flag_geoloc == PARAM_SET ) {
	       dims[0] = 2;
	       nadc_write_arr_float( outfl, ++nr, "Latitude",
				      1, dims, 3, param.geo_lat );
	       nadc_write_arr_float( outfl, ++nr, "Longitude",
				      1, dims, 3, param.geo_lon );
	       if ( param.flag_geomnmx == PARAM_SET )
		    nadc_write_text( outfl, ++nr, "WithinRegion", "True" );
	       else
		    nadc_write_text( outfl, ++nr, "WithinRegion", "False" );
	  }
/*
 * selected MDS records
 */
	  (void) strcpy( cbuff, "" );
	  if ( param.write_nadir == PARAM_UNSET ) {
	       if ( strlen( cbuff ) > 0 ) (void) strcat( cbuff, "," );
	       (void) strcat( cbuff, "NoNadir" );
	  }
	  if ( param.write_moon == PARAM_UNSET ) {
	       if ( strlen( cbuff ) > 0 ) (void) strcat( cbuff, "," );
	       (void) strcat( cbuff, "NoMoon" );
	  }
	  if ( param.write_sun == PARAM_UNSET ) {
	       if ( strlen( cbuff ) > 0 ) (void) strcat( cbuff, "," );
	       (void) strcat( cbuff, "NoSun" );
	  }
	  if ( strlen( cbuff ) == 0 ) (void) strcpy( cbuff, "All" );
	  nadc_write_text( outfl, ++nr, "MeasurementDataSets", cbuff );
/*
 * PMD data
 */
	  if ( param.write_pmd == PARAM_SET ) {
	       nadc_write_text( outfl, ++nr, "PMD", "True" );

	       if ( param.calib_pmd == CALIB_NONE ) {
		    nadc_write_text( outfl, ++nr, 
				      "PMD_Calibration", "None" );
	       } else {
		    GOME_GET_CALIB( param.calib_pmd, string );
		    if ( strlen(string) == 0 )
			 nadc_write_text( outfl, ++nr, 
				      "PMD_Calibration", "None" );
		    else
			 nadc_write_text( outfl, ++nr, 
				      "PMD_Calibration", string );
	       }
	       if ( param.write_pmd_geo == PARAM_SET )
		    nadc_write_text( outfl, ++nr, 
				      "PMD_Geolocation", "True" );
	       else
		    nadc_write_text( outfl, ++nr, 
				      "PMD_Geolocation", "False" );
	  }
/*
 * pixel number range
 */
	  if ( param.flag_pselect == PARAM_SET ) {
	       nadc_write_text( outfl, ++nr, "GroundPixelRange", 
				 param.pselect );
	  }
/*
 * ground pixel type selection
 */
	  if ( param.flag_subset == PARAM_SET ) {
	       (void) strcpy( string, "" );
	       if ( (param.write_subset & SUBSET_EAST) != UCHAR_ZERO )
		    (void) strcat( string, "E" );
	       if ( (param.write_subset & SUBSET_CENTER) != UCHAR_ZERO )
		    (void) strcat( string, "C" );
	       if ( (param.write_subset & SUBSET_WEST) != UCHAR_ZERO )
		    (void) strcat( string, "W" );
	       if ( (param.write_subset & SUBSET_BACK) != UCHAR_ZERO )
		    (void) strcat( string, "B" );
	       nadc_write_text( outfl, ++nr, "GroundPixelType", string );
	  }
/*
 * spectral bands
 */
	  if ( (param.chan_mask & BAND_ALL) != UCHAR_ZERO ) {
	       (void) strcpy( string, "" );
	       if ( (param.chan_mask & BAND_ONE_A) != UCHAR_ZERO ) {
		    if ( strlen( string ) > 0 ) (void) strcat( string, "," );
		    (void) strcat( string, "1a" );
	       }
	       if ( (param.chan_mask & BAND_ONE_B) != UCHAR_ZERO ) {
		    if ( strlen( string ) > 0 ) (void) strcat( string, "," );
		    (void) strcat( string, "1b" );
	       }
	       if ( (param.chan_mask & BAND_TWO_A) != UCHAR_ZERO ) {
		    if ( strlen( string ) > 0 ) (void) strcat( string, "," );
		    (void) strcat( string, "2a" );
	       }
	       if ( (param.chan_mask & BAND_TWO_B) != UCHAR_ZERO ) {
		    if ( strlen( string ) > 0 ) (void) strcat( string, "," );
		    (void) strcat( string, "2b" );
	       }
	       if ( (param.chan_mask & BAND_THREE) != UCHAR_ZERO ) {
		    if ( strlen( string ) > 0 ) (void) strcat( string, "," );
		    (void) strcat( string, "3" );
	       }
	       if ( (param.chan_mask & BAND_FOUR) != UCHAR_ZERO ) {
		    if ( strlen( string ) > 0 ) (void) strcat( string, "," );
		    (void) strcat( string, "4" );
	       }
	       if ( strlen( string ) == 0 )
		    nadc_write_text( outfl, ++nr, "Bands", "None" );
	       else
		    nadc_write_text( outfl, ++nr, "Bands", string );
	  }
/*
 * blind data
 */
	  if ( param.write_blind == PARAM_SET )
	       nadc_write_text( outfl, ++nr, "Blind", "True" );
	  else
	       nadc_write_text( outfl, ++nr, "Blind", "False" );
/*
 * straylight data
 */
	  if ( param.write_stray == PARAM_SET )
	       nadc_write_text( outfl, ++nr, "Straylight", "True" );
	  else
	       nadc_write_text( outfl, ++nr, "Straylight", "False" );
/*
 * wavelength range
 */
	  if ( param.flag_wave == PARAM_SET ) {
	       dims[0] = 2;
	       nadc_write_arr_float( outfl, ++nr, "wave",
				      1, dims, 3, param.wave );
	  }
/*
 * calibration of the spectral band data
 */
	  if ( param.calib_earth == CALIB_NONE ) {
	       nadc_write_text( outfl, ++nr, "BandCalibration", "None" );
	  } else {
	       char *cpntr;

	       GOME_GET_CALIB( param.calib_earth, string );
	       if ( strlen(string) == 0 ) {
		    nadc_write_text( outfl, ++nr, 
				      "EarthCalibration", "None" );
	       } else {
		    if ( (cpntr = strchr( string, 'B' )) != NULL ) 
			 *cpntr = 'A';
		    nadc_write_text( outfl, ++nr, 
				      "EarthCalibration", string );
	       }
	  }
	  if ( param.calib_moon == CALIB_NONE ) {
	       nadc_write_text( outfl, ++nr, "MoonCalibration", "None" );
	  } else {
	       GOME_GET_CALIB( param.calib_moon, string );
	       if ( strlen(string) == 0 )
		    nadc_write_text( outfl, ++nr, 
				      "MoonCalibration", "None" );
	       else
		    nadc_write_text( outfl, ++nr, 
				      "MoonCalibration", string );
	  }
	  if ( param.calib_sun == CALIB_NONE ) {
	       nadc_write_text( outfl, ++nr, "SunCalibration", "None" );
	  } else {
	       GOME_GET_CALIB( param.calib_sun, string );
	       if ( strlen(string) == 0 )
		    nadc_write_text( outfl, ++nr, 
				      "SunCalibration", "None" );
	       else
		    nadc_write_text( outfl, ++nr, 
				      "SunCalibration", string );
	  }
	  break;
/*
 *  ----- GOME level 2 processor specific options
 */
     case GOME_LEVEL_2:
/*
 * geolocation (bounding box)
 */
	  if ( param.flag_geoloc == PARAM_SET ) {
	       dims[0] = 2;
	       nadc_write_arr_float( outfl, ++nr, "Latitude",
				      1, dims, 3, param.geo_lat );
	       nadc_write_arr_float( outfl, ++nr, "Longitude",
				      1, dims, 3, param.geo_lon );
	       if ( param.flag_geomnmx == PARAM_SET )
		    nadc_write_text( outfl, ++nr, "WithinRegion", "True" );
	       else
		    nadc_write_text( outfl, ++nr, "WithinRegion", "False" );
	  }
/*
 * pixel number range
 */
	  if ( param.flag_pselect == PARAM_SET ) {
	       nadc_write_text( outfl, ++nr, "GroundPixelRange", 
				 param.pselect );
	  }
/*
 * Sun zenith angle range
 */
	  if ( param.flag_sunz == PARAM_SET ) {
	       dims[0] = 2;
	       nadc_write_arr_float( outfl, ++nr, "SunZenithAngle",
				      1, dims, 3, param.sunz );
	  }
/*
 * cloud cover range
 */
	  if ( param.flag_cloud == PARAM_SET ) {
	       dims[0] = 2;
	       nadc_write_arr_float( outfl, ++nr, "CloudCover",
				      1, dims, 3, param.cloud );
	  }
	  break;
     }
}
