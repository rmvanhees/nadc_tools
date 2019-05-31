/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 1999 - 2019 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_PARAM
.AUTHOR      R.M. van Hees
.KEYWORDS    command-line parameter handling
.LANGUAGE    ANSI C
.PURPOSE     access command-line parameters via get, set and repr functions
.CONTAINS    SCIA_SET_PARAM, SCIA_SHOW_PARAM
.RETURNS     Nothing (check global error status)
.COMMENTS    None
.ENVIRONment None
.VERSION      8.0   30-May-2019	replaced usage of struct param_record, RvH
              7.0   09-Mar-2013	created SCIA specific modules, RvH
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
#include <nadc_scia.h>
#include "../VERSION"

/*+++++ Static Variables +++++*/
static const
struct nadc_env {
     /*@null@*/ const char *opt_key;
     /*@null@*/ const char *opt_def_val;
     const char     *opt_desc;
     int            id_instr;
} nadc_envs[] = {
     {"NO_INFO_CORRECTION", "=0/1", 
      "\t[expert] do not perform validity checks on L0 info-records", 
      SCIA_LEVEL_0},
     {"NO_CLUSTER_CORRECTION", "=0/1", 
      "\t[expert] do not perform validity checks on L0 detector DSRs", 
      SCIA_LEVEL_0},
     {"SCIA_CORR_PET", "=0/1", "\tcorrect PET of SCIA Epitaxx detectors",
      SCIA_LEVEL_1},
     {"SCIA_CORR_LOS", "=0/1", "\tremove jumps in azi/zen angles (geoN)",
      SCIA_LEVEL_1},
     {"SCIA_NLCORR_NEW", "=0/1", 
      "\tapply experimental non-linearity correction",
      (SCIA_LEVEL_0|SCIA_LEVEL_1)},
     {"SCIA_MFACTOR_DIR", "=<dirname>", 
      "give path to directory with auxiliary files for m-factor correction",
      SCIA_LEVEL_1},
     {"USE_SDMF_VERSION", "=<2.4|3.0|3.1|3.2>", 
      "use SDMF version, default 3.0",
      SCIA_LEVEL_1},
     {"SCIA_TRANS_RANGE", "=min,max",
      "transmission average window, given as first and last pixel of channel 8",
      SCIA_LEVEL_1},
     {"SDMF24_SELECT", "=<NRT|CONS|HANS>",
      "SDMF2.4 select NRT or CONS based results or mimic approach Hans Schrijver",
      SCIA_LEVEL_1},
     {"SCIA_CORR_L1C", "=<filename>", "correct radiances in L1c product.\n" \
      "\t\tThe multiplication factors are read from an auxiliary file.\n" \
      "\t\tThis option only works in combination with option \"--cal=p\".",
      SCIA_LEVEL_1},
/* last and empty entry */
     {NULL, NULL, "", 0}
};

static const
struct nadc_opt {
     /*@null@*/ const char *opt_key;
     /*@null@*/ const char *opt_def_val;
     const char     *opt_desc;
     int            id_instr;
} nadc_opts[] = {
/* general options */
     {"-h", NULL, "\tdisplay this help and exit [default]",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-help", NULL, "display this help and exit",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-V", NULL, "\tdisplay version & copyright information and exit",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-show_param", NULL, 
      "display setting of command-line parameters; no output generated",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-version", NULL, "display version & copyright information and exit",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-verbose", NULL, "verbose mode",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-silent", NULL, "do not display any error messages",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-check", NULL, "check inputfile by reading it; no output generated",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-meta", NULL, 
      "write (in ASCII format) NL-SCIA-DC meta-Database information",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-pds_1b", NULL, "write output in Payload Data Segment 1B format",
      SCIA_LEVEL_1},
     {"-pds_1c", NULL, "write output in Payload Data Segment 1C format",
      SCIA_LEVEL_1},
     {"-ascii", NULL, "write output in ASCII format",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-hdf5", NULL, "write output in HDF5 format",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-compress", NULL, "compress data sets in HDF5-file",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
#if defined(_WITH_SQL)
     {"-sql", NULL, "\twrite to PostgreSQL database",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-remove", NULL, "removes current entry of product from SQL database",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-replace", NULL, "replace current entry of product in SQL database",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
#endif
/* (G)ADS selection */
     {"-no_gads", NULL, "do not extract Global Annotation Data Sets (GDS)",
      (SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-no_ads", NULL, "do not extract Annotation Data Sets (ADS)",
      (SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-no_aux0", NULL, "do not extract Auxiliary ADS records",
      SCIA_LEVEL_1},
     {"-no_pmd0", NULL, "do not extract PMD ADS records",
      SCIA_LEVEL_1},
/* MDS selection */
     {"-no_mds", NULL, "do not extract Measurement Data Sets (MDS)",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
     {"-aux", NULL, "\textract Auxiliary MDS records",
      SCIA_LEVEL_0},
     {"-no_aux", NULL, "do not extract Auxiliary MDS records",
      SCIA_LEVEL_0},
     {"-det", NULL, "\textract Detector MDS records",
      SCIA_LEVEL_0},
     {"-no_det", NULL, "do not extract Detector MDS records",
      SCIA_LEVEL_0},
     {"-pmd", NULL, "\textract PMD MDS records",
      (SCIA_LEVEL_0|SCIA_LEVEL_1)},
     {"-no_pmd", NULL, "do not extract PMD MDS records",
      (SCIA_LEVEL_0|SCIA_LEVEL_1)},
     {"-no_polV", NULL, "do not extract fractional polarisation MDS",
      SCIA_LEVEL_1},
     {"-limb", NULL, "extract Limb MDS records",
      SCIA_LEVEL_1},
     {"-no_limb", NULL, "do not extract Limb MDS records",
      SCIA_LEVEL_1},
     {"-moni", NULL, "extract Monitoring MDS records",
      SCIA_LEVEL_1},
     {"-no_moni", NULL, "do not extract Monitoring MDS records",
      SCIA_LEVEL_1},
     {"-nadir", NULL, "extract Nadir MDS records",
      SCIA_LEVEL_1},
     {"-no_nadir", NULL, "do not extract Nadir MDS records",
      SCIA_LEVEL_1},
     {"-occ", NULL, "\textract Occultation MDS records",
      SCIA_LEVEL_1},
     {"-no_occ", NULL, "do not extract Occultation MDS records",
      SCIA_LEVEL_1},
     {"-cld", NULL, "\textract Cloud and Aerosol MDS records",
      SCIA_LEVEL_2},
     {"-no_cld", NULL, "do not extract Cloud and Aerosol MDS records",
      SCIA_LEVEL_2},
     {"-bias", NULL, "extract BIAS MDS records extracted (NRT)",
      SCIA_LEVEL_2},
     {"-no_bias", NULL, "do not extract BIAS MDS records extracted (NRT)",
      SCIA_LEVEL_2},
     {"-doas", NULL, "extract DOAS MDS records (NRT)",
      SCIA_LEVEL_2},
     {"-no_doas", NULL, "do not extract DOAS MDS records extracted (NRT)",
      SCIA_LEVEL_2},
/* time selection */
     {"--time", " start_date start_time [end_date] end_time", 
      "apply time-window",
      (SCIA_LEVEL_0|SCIA_LEVEL_1)},
/* geolocation selection */
     {"--region", "=lat_min,lat_max,lon_min,lon_max",
      "apply selection on geolocation", 
      SCIA_LEVEL_1},
/* category/state selection */
     {"--cat", "=[1,2,...,26]", "write MDS data of selected categories",
      SCIA_LEVEL_1},
     {"--state", "=[1,2,...,70]", "write MDS data of selected states",
      (SCIA_LEVEL_0|SCIA_LEVEL_1)},
/* spectral band/channel selection */
     {"--chan", "[=1,2,...,8]", "write data of selected spectral bands",
      SCIA_LEVEL_1},
     {"--clus", "=[1,2,...,64]", "write data of selected clusters",
      SCIA_LEVEL_1},
/* MDS quality check */
     {"-no_qcheck", NULL, "no check on incomplete and/or corrupted states",
      SCIA_LEVEL_0},
/* MDS calibration */
     {"--cal", "[=0,1,...,9]", "apply spectral calibration, impies L1c format",
      SCIA_LEVEL_1},
/* keydata patch */
     {"-no_patch", NULL, "do not apply any patches on annotation datasets",
      SCIA_PATCH_1},
     {"--patch", "=[0,1,..,9]", "apply corrections on annotation datasets",
      SCIA_PATCH_1},
/* name of output file */
     {"--output", "=<outfile>", "(default: <infile> + appropriate extension)",
      (SCIA_LEVEL_0|SCIA_LEVEL_1|SCIA_LEVEL_2)},
/* last and empty entry */
     {NULL, NULL, "", 0}
};

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   Show_All_Options
.PURPOSE     display (optional) command parameters
.INPUT/OUTPUT
  call as   Show_All_Options(stream, instrument, prognm);
     input:  
            FILE *stream      :   open (file) stream pointer
	    int instrument    :   code for instrument en data product level
	    char prognm[]     :   string with name of the master program

.RETURNS     exits with EXIT_FAILURE
.COMMENTS    static function
-------------------------*/
static /*@exits@*/ 
void Show_All_Options(FILE *stream, int instrument, 
		       /*@notnull@*/ const char prognm[])
     /*@modifies stream@*/
{
     register short nr;
/*
 * intro of message
 */
     (void) fprintf(stream, "Usage: %s [OPTIONS] FILE\n\n", prognm);
/*
 * display all available NADC options
 */
     nr = -1;
     while (nadc_opts[++nr].opt_key != NULL) {
	  if ((instrument & nadc_opts[nr].id_instr) != 0) {
	       (void) fprintf(stream, "   %s", nadc_opts[nr].opt_key);
	       if (nadc_opts[nr].opt_def_val != NULL)
		    (void) fprintf(stream, "%s", nadc_opts[nr].opt_def_val);
	       (void) fprintf(stream, "\t%s\n", nadc_opts[nr].opt_desc);
	  }
     }
/*
 * show instrument specific options
 */
     if (instrument == SCIA_LEVEL_1) {
	  (void) fprintf(stream, "\nRecognised calibration options are:\n");
	  scia_show_calib(stream);
     } else if (instrument == SCIA_PATCH_1) {
	  (void) fprintf(stream, "\nRecognised patch options are:\n");
	  scia_show_patch(stream);
     }
/*
 * display all available NADC environment variables
 */
     nr = -1;
     while (nadc_envs[++nr].opt_key != NULL) {
	  if ((instrument & nadc_envs[nr].id_instr) != 0) {
	       if (nr == 0) 
		    (void) fprintf(stream, "\nEnvironment variables:\n");
	       (void) fprintf(stream, "   %s", nadc_envs[nr].opt_key);
	       if (nadc_envs[nr].opt_def_val != NULL)
		    (void) fprintf(stream, "%s", nadc_envs[nr].opt_def_val);
	       (void) fprintf(stream, "\t%s\n", nadc_envs[nr].opt_desc);
	  }
     }
     exit(EXIT_FAILURE);
}

/*+++++++++++++++++++++++++
.IDENTifer   Check_User_Option
.PURPOSE     check command-line parameters is valid given instrument/prognm
.INPUT/OUTPUT
  call as   Check_User_Option(stream, instrument, prognm, argv);

     input:
            FILE *stream      :   open (file) stream pointer for error-mesg
	    int instrument    :   code for instrument en data product level
	    char prognm[]     :   string with name of the master program
	    const char argv[] :   command-line parameter

.RETURNS     nothing, exits on failure
.COMMENTS    static function
-------------------------*/
static
void Check_User_Option(FILE *stream, int instrument,
		       /*@notnull@*/ const char prognm[],
		       /*@notnull@*/ const char argv[])
     /*@modifies stream@*/
{
     register short nr = -1;
/*
 * catch the string with the name of the input file
 */
     if (argv[0] != '-') return;
/*
 * check options
 */
     while (nadc_opts[++nr].opt_key != NULL) {
	  size_t opt_len = strlen(nadc_opts[nr].opt_key);
	  
	  if (strncmp(argv, nadc_opts[nr].opt_key, opt_len) == 0) {
	       if ((instrument & nadc_opts[nr].id_instr) != 0)
		    return;
	  }
     }
     (void) fprintf(stream, 
		    "%s: FATAL, unknown command-line option \"%s\"\n", 
		    prognm, argv);
     exit(EXIT_FAILURE);
}

/*+++++++++++++++++++++++++
.IDENTifer   Conv_Date
.PURPOSE     convert Date string containing 01, 02,... to Jan, Feb,...
.INPUT/OUTPUT
  call as   Conv_Date(in_date, out_date);

     input:  
            char in_date[]   :  one of DD-MMM-YYYY, YYYY-MM-DD, 
                                       DD-MM-YYYY, YYYY-MM-DD
    output:  
            char out_date[]  :  DD-MMM-YYYY

.RETURNS     error status (0=ok)
.COMMENTS    static function
-------------------------*/
static inline
int Conv_Date(const char in_date[], /*@out@*/ char out_date[])
{
     char *pntr, day[3], mon[4], year[5];

     (void) sprintf(out_date, "UNKOWN");
     if (strlen(in_date) > DATE_ONLY_STRING_LENGTH)
	  return NADC_ERR_FATAL;

     if ((pntr = strchr(in_date, '-')) != NULL) {
	  if ((pntr - in_date) == 4) 
	       (void) nadc_strlcpy(year, in_date, 5);
	  else if ((pntr - in_date) == 2)
	       (void) nadc_strlcpy(day, in_date, 3);
	  else
	       return NADC_ERR_FATAL;
	  in_date = pntr + 1;
     }
     if ((pntr = strchr(in_date, '-')) != NULL) {
	  if ((pntr - in_date) == 3) {
	       (void) nadc_strlcpy(mon, in_date, 4);
	  } else if ((pntr - in_date) == 2) {
	       int mon_num;
	       const char *mon_str[] =
		    {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", 
		     "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

	       mon_num = atoi(strncpy(mon, in_date, 2));
	       if (mon_num > 0 && mon_num <= 12)
		    (void) strcpy(mon, mon_str[mon_num-1]);
	       else
		    return NADC_ERR_FATAL;
	  } else
	       return NADC_ERR_FATAL;
	  in_date = pntr + 1;
     }
     if (strlen(in_date) == 2)
	  (void) nadc_strlcpy(day, in_date, 3);
     else if (strlen(in_date) == 4)
	  (void) nadc_strlcpy(year, in_date, 5);
     else
	  return NADC_ERR_FATAL;

     (void) snprintf(out_date, 12, "%s-%s-%s", day, mon, year);
     return NADC_ERR_NONE;
}

/*+++++++++++++++++++++++++
.IDENTifer   Set_Time_Window
.PURPOSE     
.INPUT/OUTPUT
  call as   Set_Time_Window(argc, argv[], &narg, bgn_date, end_date);
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
void Set_Time_Window(int argc, char *argv[], int *narg,
		      /*@out@*/ char *bgn_date, /*@out@*/ char *end_date)
{
     int num;
/*
 * first parameter has to be a Date-string
 */
     *bgn_date = *end_date = '\0';
     if (++(*narg) >= argc) 
	  NADC_RETURN_ERROR(NADC_ERR_PARAM, argv[(*narg)]);
     if (strchr(argv[(*narg)], '-') == NULL ||
	 Conv_Date(argv[(*narg)], bgn_date) != NADC_ERR_NONE)
	  NADC_RETURN_ERROR(NADC_ERR_PARAM, argv[(*narg)]);
/*
 * second parameter has to be a Time-string (always HH:MM:SS[.SSS])
 */
     if (++(*narg) >= argc) 
	  NADC_RETURN_ERROR(NADC_ERR_PARAM, argv[(*narg)]);
     if (strchr(argv[(*narg)], ':') != NULL) {
	  (void) strcat(bgn_date, " ");
	  num = (int) strlen(argv[(*narg)]);
	  if (num <= TIME_ONLY_STRING_LENGTH)
	       (void) strcat(bgn_date, argv[(*narg)]);
	  else
	       NADC_RETURN_ERROR(NADC_ERR_PARAM, argv[(*narg)]);
    } else
	  NADC_RETURN_ERROR(NADC_ERR_PARAM, argv[(*narg)]);
/*
 * third parameter can be a Time-string, then duplicate previous Date-string
 * otherwise the third parameter has to be Date-string, 
 * and fourth parameter has to be Time-string
 */
     if (++(*narg) >= argc)
	  NADC_RETURN_ERROR(NADC_ERR_PARAM, argv[(*narg)]);
     if (strchr(argv[(*narg)], ':') != NULL) {
	  (void) Conv_Date(argv[(*narg)-2], end_date);
	  (void) strcat(end_date, " ");
	  num = (int) strlen(argv[(*narg)]);
	  if (num <= TIME_ONLY_STRING_LENGTH)
	       (void) strcat(end_date, argv[(*narg)]);
	  else
	       NADC_RETURN_ERROR(NADC_ERR_PARAM, argv[(*narg)]);
    } else if (strchr(argv[(*narg)], '-') != NULL) {
	  if (Conv_Date(argv[(*narg)], end_date)	\
	      != NADC_ERR_NONE) 
	       NADC_RETURN_ERROR(NADC_ERR_PARAM, argv[(*narg)]);
	  (void) strcat(end_date, " ");
	  if (++(*narg) >= argc) 
	       NADC_RETURN_ERROR(NADC_ERR_PARAM, argv[(*narg)]);
	  num = (int) strlen(argv[(*narg)]);
	  if (strchr(argv[(*narg)], ':') != NULL &&
	      num <= TIME_ONLY_STRING_LENGTH)
	       (void) strcat(end_date, argv[(*narg)]);
	  else
	       NADC_RETURN_ERROR(NADC_ERR_PARAM, argv[(*narg)]);
    } else
	  NADC_RETURN_ERROR(NADC_ERR_PARAM, argv[(*narg)]);
}

/*+++++++++++++++++++++++++
.IDENTifer   Do_Not_Extract_MDS
.PURPOSE     set user-defined settings not to write MDS data
.INPUT/OUTPUT
  call as   Do_Not_Extract_MDS(instrument);
     input:
            int instrument          : code for instrument en data product level

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static inline
void Do_Not_Extract_MDS(int instrument)
{
     int res;
     
     switch (instrument) {
     case SCIA_LEVEL_0:
	  res = nadc_set_param_uint8("write_aux", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_det", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_pmd", PARAM_UNSET);
	  break;
     case SCIA_LEVEL_1:
	  res = nadc_set_param_uint8("write_limb", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_moni", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_nadir", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_occ", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_pmd", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_polV", PARAM_UNSET);
	  break;
     case SCIA_LEVEL_2:
	  res = nadc_set_param_uint8("write_bias", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_doas", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_limb", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_nadir", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_occ", PARAM_UNSET);
	  break;
    }
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_SET_PARAM
.PURPOSE     initializes struct with command-line parameters
.INPUT/OUTPUT
  call as   SCIA_SET_PARAM(argc, argv, instrument);
     input:  
             int  argc       :   number of parameters
	     char *argv[]    :   parameter values
	     int instrument  :   code for instrument en data product level

.RETURNS     Nothing (check global error status)
.COMMENTS    none
-------------------------*/
void SCIA_SET_PARAM(int argc, char *argv[], int instrument)
{
     char   *cpntr;
     char   *name_infile;
     char   prog_master[SHORT_STRING_LENGTH];
     int    narg, num, res;
     float  rbuff[4];

     bool select_mds = FALSE;
/*
 * check number of options
 */
     if (argc == 0 || argv[0] == NULL) {
	  (void) nadc_set_param_string("program", "foo");
	  NADC_RETURN_ERROR(NADC_ERR_PARAM, "none found!?!");
     }
/*
 * set name of calling program
 */
     res = nadc_set_param_string("program", argv[0]);
/*
 * strip path to program
 */
     if ((cpntr = strrchr(argv[0], '/')) != NULL)
	  (void) nadc_strlcpy(prog_master, ++cpntr, SHORT_STRING_LENGTH);
     else
	  (void) nadc_strlcpy(prog_master, argv[0], SHORT_STRING_LENGTH);
/*
 * get command-line parameters
 */
     narg = 0;
     while (++narg < argc) {
	  Check_User_Option(stderr, instrument, prog_master, argv[narg]);

	  if (argv[narg][0] == '-' && argv[narg][1] == '-') {
	       /* command-line options starting with "--" */
	       if (strncmp(argv[narg]+2, "time", 4) == 0) {
		    /* perform selection on time-window */
		    char bgn_date[DATE_STRING_LENGTH],
			 end_date[DATE_STRING_LENGTH];
		    
		    if (nadc_get_param_uint8("flag_period") == PARAM_UNSET) {
			 res = nadc_set_param_uint8("flag_period", PARAM_SET);
			 Set_Time_Window(argc, argv, &narg, bgn_date, end_date);
			 if (IS_ERR_STAT_FATAL)
			      NADC_RETURN_ERROR(NADC_ERR_PARAM, "period");
			 res = nadc_set_param_string("bgn_date", bgn_date);
			 res = nadc_set_param_string("end_date", end_date);
		    }
	       } else if (strncmp(argv[narg]+2, "region", 6) == 0) {
		    /* perform selection on geo-location */
		    if (nadc_get_param_uint8("flag_geoloc") == PARAM_UNSET
			&& (cpntr = strchr(argv[narg], '=')) != NULL) {
			 
			 (void) NADC_USRINP(FLT32_T, cpntr+1, 4, rbuff, &num);
			 if (num == 4) {
			      float lat_range[2] = {
				   min_t(float, rbuff[0], rbuff[1]),
				   max_t(float, rbuff[0], rbuff[1])};
			      float lon_range[2] = {
				   min_t(float, rbuff[2], rbuff[3]),
				   max_t(float, rbuff[2], rbuff[3])};

			      res = nadc_set_param_range("latitude", lat_range);
			      res = nadc_set_param_range("longitude", lon_range);
			      if (rbuff[2] < rbuff[3])
				   res = nadc_set_param_uint8(
					"flag_geomnmx", PARAM_SET);
			      else
				   res = nadc_set_param_uint8(
					"flag_geomnmx", PARAM_UNSET);
			 } else
			      NADC_RETURN_ERROR(NADC_ERR_PARAM, argv[narg]);

			 res = nadc_set_param_uint8("flag_geoloc", PARAM_SET);
		    }
	       } else if (strncmp(argv[narg]+2, "cat", 3) == 0) {
		    /* perform selection on measurement category */
		    unsigned char cat_list[MAX_NUM_CLUS];
		    
		    if ((cpntr = strchr(argv[narg], '=')) == NULL) {
			 res = nadc_set_param_cat(cat_list, 0);
		    } else {
			 (void) NADC_USRINP(UINT8_T, cpntr+1, 
					    MAX_NUM_CLUS, cat_list, &num);
			 res = nadc_set_param_cat(cat_list, num);
		    }
	       } else if (strncmp(argv[narg]+2, "state", 5) == 0) {
		    /* perform selection on measurement state ID(s) */
		    unsigned char state_list[MAX_NUM_STATE];

		    if ((cpntr = strchr(argv[narg], '=')) == NULL) {
			 res = nadc_set_param_state(state_list, 0);
		    } else {
			 (void) NADC_USRINP(UINT8_T, cpntr+1, 
					    MAX_NUM_STATE, state_list, &num);
			 res = nadc_set_param_state(state_list, num);
		    }
	       } else if (strncmp(argv[narg]+2, "chan", 4) == 0) {
		    /* perform selection on science channel(s) */
		    unsigned char chan_list[8];
		    
		    if ((cpntr = strchr(argv[narg], '=')) == NULL) {
			 res = nadc_set_param_chan(chan_list, 0);
		    } else {
			 (void) NADC_USRINP(UINT8_T, cpntr+1,
					    8, chan_list, &num);
			 res = nadc_set_param_chan(chan_list, num);
		    }
	       } else if (strncmp(argv[narg]+2, "clus", 4) == 0) {
		    /* perform selection on cluster ID(s) */
		    unsigned char clus_list[MAX_NUM_CLUS];
		    
		    if ((cpntr = strchr(argv[narg], '=')) == NULL) {
			 res = nadc_set_param_clus(clus_list, 0);
		    } else {
			 (void) NADC_USRINP(UINT8_T, cpntr+1, 
					    MAX_NUM_CLUS, clus_list, &num);
			 res = nadc_set_param_clus(clus_list, num);
		    }
	       } else if (strncmp(argv[narg]+2, "cal", 3) == 0) {
		    /* perform calibration on measurement data (L1b only) */
		    res = nadc_set_param_uint8("write_lv1c", PARAM_SET);
		    if ((cpntr = strchr(argv[narg], '=')) == NULL)
			 scia_set_calib("atbd");
		    else
			 scia_set_calib(cpntr+1);

	       } else if (strncmp(argv[narg]+2, "patch", 5) == 0) {
		    /* perform patches to calibration key data in L1b product */
		    if ((cpntr = strchr(argv[narg], '=')) == NULL)
			 scia_set_patch("all");
		    else
			 scia_set_patch(cpntr+1);
	       }

	       if (strncmp(argv[narg]+2, "output=", 7) == 0) {
		    char *name_outfile;
		    
		    if (strlen(argv[narg]+9) == 0)
			 NADC_RETURN_ERROR(NADC_ERR_PARAM, argv[narg]);

		    nadc_set_param_string("outfile", argv[narg]+9);
		    name_outfile = nadc_get_param_string("outfile");

		    /* remove known extensions */
		    if ((cpntr = strstr(name_outfile, ".h5")) != NULL)
			 *cpntr = '\0';
		    if ((cpntr = strstr(name_outfile, ".hdf")) != NULL)
			 *cpntr = '\0';
		    if ((cpntr = strstr(name_outfile, ".txt")) != NULL)
			 *cpntr = '\0';
		    if ((cpntr = strstr(name_outfile, ".child")) != NULL)
			 *cpntr = '\0';
		    nadc_set_param_string("outfile", name_outfile);
		    free(name_outfile);
	       }
	  } else if (argv[narg][0] == '-') {
	       /* command-line options starting with "-" */
	       if ((argv[narg][1] == 'h' && argv[narg][2] == '\0')
		   || strncmp(argv[narg]+1, "help", 4) == 0) {
		    Show_All_Options(stdout, instrument, prog_master);
	       }
	       if (argv[narg][1] == 'V' 
		   || strncmp(argv[narg]+1, "version", 7) == 0) {
		    res = nadc_set_param_uint8("flag_version", PARAM_SET);
		    return;                             /* nothing else todo */
	       }
	       if (strncmp(argv[narg]+1, "show", 4) == 0) {
		    res = nadc_set_param_uint8("flag_show", PARAM_SET);
		    res = nadc_set_param_uint8("write_ascii", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "silent", 6) == 0) {
		    res = nadc_set_param_uint8("flag_silent", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "verbose", 7) == 0) {
		    res = nadc_set_param_uint8("flag_verbose", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "check", 5) == 0) {
		    res = nadc_set_param_uint8("flag_check", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "meta", 4) == 0) {
		    res = nadc_set_param_uint8("write_meta", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "pds_1b", 6) == 0) {
		    res = nadc_set_param_uint8("write_pds", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "pds_1c", 6) == 0) {
		    res = nadc_set_param_uint8("write_pds", PARAM_SET);
		    res = nadc_set_param_uint8("write_lv1c", PARAM_SET);
		    res = nadc_set_param_uint8("write_ads", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "ascii", 5) == 0) {
		    res = nadc_set_param_uint8("write_ascii", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "hdf5", 4) == 0) {
		    res = nadc_set_param_uint8("write_hdf5", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "compress", 8) == 0) {
		    res = nadc_set_param_uint8("flag_deflate", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "sql", 3) == 0) {
#if defined(_WITH_SQL)
		    res = nadc_set_param_uint8("write_sql", PARAM_SET);
#else
		    NADC_RETURN_ERROR(NADC_ERR_FATAL, 
				      "no PostgreSQL support, recompile");
#endif
	       } else if (strncmp(argv[narg]+1, "remove", 6) == 0) {
		    res = nadc_set_param_uint8("flag_sql_remove", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "replace", 7) == 0) {
		    res = nadc_set_param_uint8("flag_sql_replace", PARAM_SET);

		    /* selection on all kind of data sets */
	       } else if (strncmp(argv[narg]+1, "no_gads", 7) == 0) {
		    res = nadc_set_param_uint8("write_gads", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_ads", 6) == 0) {
		    res = nadc_set_param_uint8("write_ads", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_aux0", 7) == 0) {
		    res = nadc_set_param_uint8("write_aux0", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_pmd0", 7) == 0) {
		    res = nadc_set_param_uint8("write_pmd0", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_aux", 6) == 0) {
		    res = nadc_set_param_uint8("write_aux", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_det", 6) == 0) {
		    res = nadc_set_param_uint8("write_det", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_pmd", 6) == 0) {
		    res = nadc_set_param_uint8("write_pmd", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_mds", 6) == 0) {
		    if (! select_mds) {
			 select_mds = TRUE;
			 Do_Not_Extract_MDS(instrument);
		    }
	       } else if (strncmp(argv[narg]+1, "aux", 3) == 0) {
		    if (! select_mds) {
			 select_mds = TRUE;
			 Do_Not_Extract_MDS(instrument);
		    }
		    res = nadc_set_param_uint8("write_aux", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "det", 3) == 0) {
		    if (! select_mds) {
			 select_mds = TRUE;
			 Do_Not_Extract_MDS(instrument);
		    }
		    res = nadc_set_param_uint8("write_det", PARAM_SET);
	       } else if (instrument == SCIA_LEVEL_0
			  && strncmp(argv[narg]+1, "pmd", 3) == 0) {
		    if (! select_mds) {
			 select_mds = TRUE;
			 Do_Not_Extract_MDS(instrument);
		    }
		    res = nadc_set_param_uint8("write_pmd", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "nadir", 5) == 0) {
		    if (! select_mds) {
			 select_mds = TRUE;
			 Do_Not_Extract_MDS(instrument);
		    }
		    res = nadc_set_param_uint8("write_nadir", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "limb", 4) == 0) {
		    if (! select_mds) {
			 select_mds = TRUE;
			 Do_Not_Extract_MDS(instrument);
		    }
		    res = nadc_set_param_uint8("write_limb", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "occ", 3) == 0) {
		    if (! select_mds) {
			 select_mds = TRUE;
			 Do_Not_Extract_MDS(instrument);
		    }
		    res = nadc_set_param_uint8("write_occ", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "moni", 4) == 0) {
		    if (! select_mds) {
			 select_mds = TRUE;
			 Do_Not_Extract_MDS(instrument);
		    }
		    res = nadc_set_param_uint8("write_moni", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "no_nadir", 8) == 0) {
		    if (! select_mds) select_mds = TRUE;
		    res = nadc_set_param_uint8("write_nadir", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_limb", 7) == 0) {
		    if (! select_mds) select_mds = TRUE;
		    res = nadc_set_param_uint8("write_limb", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_occ", 6) == 0) {
		    if (! select_mds) select_mds = TRUE;
		    res = nadc_set_param_uint8("write_occ", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_pmd", 6) == 0) {
		    res = nadc_set_param_uint8("write_pmd", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_polV", 7) == 0) {
		    res = nadc_set_param_uint8("write_polV", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_cld", 6) == 0) {
		    res = nadc_set_param_uint8("write_cld", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_bias", 7) == 0) {
		    res = nadc_set_param_uint8("write_bias", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_doas", 7) == 0) {
		    res = nadc_set_param_uint8("write_doas", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_moon", 7) == 0) {
		    res = nadc_set_param_uint8("write_moon", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_sun", 6) == 0) {
		    res = nadc_set_param_uint8("write_sun", PARAM_UNSET);
	       } else if (strncmp(argv[narg]+1, "no_patch", 8) == 0) {
		    res = nadc_set_param_uint8("patch_scia", SCIA_PATCH_NONE);
	       } else if (strncmp(argv[narg]+1, "no_qcheck", 9) == 0) {
		    res = nadc_set_param_uint8("qcheck", PARAM_UNSET);
	       }
	  } else {
	       /* name of input file */
	       nadc_set_param_string("infile", argv[narg]);
	       name_infile = argv[narg];
	  }
     }
     if (nadc_get_param_uint8("flag_check") == PARAM_SET) {
	  res = nadc_set_param_uint8("write_pds", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_hdf5", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_sql", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_ascii", PARAM_UNSET);
     } else if (nadc_get_param_uint8("write_sql") == PARAM_SET) {
	  res = nadc_set_param_uint8("write_pds", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_hdf5", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_ascii", PARAM_UNSET);
	  if (instrument == SCIA_LEVEL_1) {
	       res = nadc_set_param_uint8("write_ads", PARAM_UNSET);
	       res = nadc_set_param_uint8("write_gads", PARAM_UNSET);
	       res = nadc_set_param_uint8("write_limb", PARAM_UNSET);
	       res = nadc_set_param_uint8("write_moni", PARAM_UNSET);
	       res = nadc_set_param_uint8("write_nadir", PARAM_SET);
	       res = nadc_set_param_uint8("write_occ", PARAM_UNSET);
	       res = nadc_set_param_uint8("write_lv1c", PARAM_UNSET);
	  }
	  if (instrument == SCIA_LEVEL_2) {
	       res = nadc_set_param_uint8("write_ads", PARAM_UNSET);
	       res = nadc_set_param_uint8("write_gads", PARAM_UNSET);
	  }
     } else if (nadc_get_param_uint8("write_meta") == PARAM_SET) {
	  res = nadc_set_param_uint8("write_pds", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_hdf5", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_ascii", PARAM_SET);
	  Do_Not_Extract_MDS(instrument);
	  res = nadc_set_param_uint8("write_ads", PARAM_UNSET);
	  res = nadc_set_param_uint8("write_gads", PARAM_UNSET);
     } else {
	  if (nadc_get_param_uint8("write_pds") == PARAM_UNSET 
	      && nadc_get_param_uint8("write_hdf5") == PARAM_UNSET 
	      && nadc_get_param_uint8("write_ascii") == PARAM_UNSET) {
	       if (instrument == SCIA_LEVEL_1) {
		    res = nadc_set_param_uint8("write_pds", PARAM_SET);
		    res = nadc_set_param_uint8("write_lv1c", PARAM_SET);
		    res = nadc_set_param_uint8("write_ads", PARAM_UNSET);
	       } else
		    res = nadc_set_param_uint8("write_hdf5", PARAM_SET);
	  }
     }

     /* User has to give the name of the input filename */
     if (nadc_get_param_string("infile") == NULL) 
	  Show_All_Options(stderr, instrument, prog_master);

     /* make sure that the output filename is defined */
     if ((cpntr = nadc_get_param_string("outfile")) == NULL) {
	  if ((cpntr = strrchr(name_infile, '/')) != NULL)
	       nadc_set_param_string("outfile", cpntr+1);
	  else
	       nadc_set_param_string("outfile", name_infile);

	  if (instrument == SCIA_PATCH_1) {
	       cpntr = nadc_get_param_string("outfile");
	       cpntr[10] = SCIA_PATCH_ID[0];
	       nadc_set_param_string("outfile", cpntr);
	  }
     } else
	  free(cpntr);

     if (nadc_get_param_uint8("write_pds") == PARAM_SET)
	  res = nadc_set_param_add_ext("outfile", ".child");

     if (nadc_get_param_uint8("write_hdf5") == PARAM_SET)
	  res = nadc_set_param_add_ext("outfile", ".h5");
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_SHOW_PARAM
.PURPOSE     show command-line settings, as stored in the param-record
.INPUT/OUTPUT
  call as   SCIA_SHOW_PARAM(instrument);
     input:  
             int instrument     :   code for instrument en data product level

.RETURNS     Nothing (check global error status)
.COMMENTS    none
-------------------------*/
void SCIA_SHOW_PARAM(int instrument)
{
     register unsigned int nr = 0;
     
     char    *cpntr, cbuff[MAX_STRING_LENGTH];
     char    string[SHORT_STRING_LENGTH];
     time_t  tp[1];
     float   rbuff[2];
     
     FILE *outfl = stdout;
/*
 * show program name and version
 */
     cpntr = nadc_get_param_string("program");
     (void) snprintf(cbuff, MAX_STRING_LENGTH, "%s (version %-d.%-d.%-d)", 
		     cpntr, nadc_vers_major, nadc_vers_minor,
		     nadc_vers_release);
     free(cpntr);
     nadc_write_text(outfl, ++nr, "Program", cbuff);
/*
 * show time of call
 */
     (void) time(tp);
     (void) nadc_strlcpy(cbuff, ctime(tp), MAX_STRING_LENGTH);
     cbuff[strlen(cbuff)-1] = '\0';
     nadc_write_text(outfl, ++nr, "ProcessingDate", cbuff);
/*
 * show output files
 */
     cpntr = nadc_get_param_string("infile");
     nadc_write_text(outfl, ++nr, "InputFilename", cpntr);
     free(cpntr);
     if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	  cpntr = nadc_get_param_string("outfile");
	  nadc_write_text(outfl, ++nr, "OutputFilename", cpntr);
	  free(cpntr);
     }
     if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	  if (nadc_get_param_uint8("flag_deflate") == PARAM_SET)
	       nadc_write_text(outfl, ++nr, "HDF5 compression", "True");
	  else
	       nadc_write_text(outfl, ++nr, "HDF5 compression", "False");
     }
     if (instrument == SCIA_LEVEL_1) {
	  if (nadc_get_param_uint8("write_lv1c") == PARAM_SET)
	       nadc_write_text(outfl, ++nr, "lv1cOutput", "True");
	  else
	       nadc_write_text(outfl, ++nr, "lv1cOutput", "False"); 
     }
/*
 * ----- General options
 */
     if (nadc_get_param_uint8("flag_silent") == PARAM_SET)
	  nadc_write_text(outfl, ++nr, "Silent", "True");
     else
	  nadc_write_text(outfl, ++nr, "Silent", "False");
/*
 * time window
 */
     if (nadc_get_param_uint8("flag_period") == PARAM_SET) {
	  cpntr = nadc_get_param_string("bgn_date");
	  nadc_write_text(outfl, ++nr, "StartDate", cpntr);
	  free(cpntr);
	  cpntr = nadc_get_param_string("end_date");
	  nadc_write_text(outfl, ++nr, "EndDate", cpntr);
	  free(cpntr);
     }
     
     switch (instrument) {
/*
 *  ----- SCIAMACHY level 0 processor specific options
 */
     case SCIA_LEVEL_0:
/*
 * selected measurement data sets
 */
	  (void) strcpy(cbuff, "");
	  if (nadc_get_param_uint8("write_aux") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoAuxiliary");
	  }
	  if (nadc_get_param_uint8("write_det") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoDetector");
	  }
	  if (nadc_get_param_uint8("write_pmd") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoPMD");
	  }
	  if (strlen(cbuff) == 0) (void) strcpy(cbuff, "All");
	  nadc_write_text(outfl, ++nr, "MeasurementDataSets", cbuff);
/*
 * selected States
 */
	  nadc_repr_param_state(MAX_STRING_LENGTH, cbuff);
	  nadc_write_text(outfl, ++nr, "StateID", cbuff);
/*
 * DSR quality check
 */
	  if (nadc_get_param_uint8("qcheck") == PARAM_UNSET)
	       nadc_write_text(outfl, ++nr, "MdsQualityCheck", "Off");
	  else
	       nadc_write_text(outfl, ++nr, "MdsQualityCheck", "On");
	  break;
/*
 *  ----- Patch SCIAMACHY level 1 processor specific options
 */
     case SCIA_PATCH_1:
	  scia_get_patch(string);
	  nadc_write_text(outfl, ++nr, "Patch Options", string);
	  break;
/*
 *  ----- SCIAMACHY level 1 processor specific options
 */
     case SCIA_LEVEL_1:
/*
 * geolocation (bounding box)
 */
	  if (nadc_get_param_uint8("flag_geoloc") == PARAM_SET) {
	       unsigned int dims[2] = {2, 0};
	       
	       nadc_get_param_range("latitude", rbuff);
	       nadc_write_arr_float(outfl, ++nr, "Latitude",
				    1, dims, 3, rbuff);
	       nadc_get_param_range("longitude", rbuff);
	       nadc_write_arr_float(outfl, ++nr, "Longitude",
				    1, dims, 3, rbuff);
	       if (nadc_get_param_uint8("flag_geomnmx") == PARAM_SET)
		    nadc_write_text(outfl, ++nr, "WithinRegion", "True");
	       else
		    nadc_write_text(outfl, ++nr, "WithinRegion", "False");
	  }
/*
 * selected Key DSD's
 */
	  (void) strcpy(cbuff, "");
	  if (nadc_get_param_uint8("write_ads") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoADS");
	  }
	  if (nadc_get_param_uint8("write_gads") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoGADS");
	  }
	  if (strlen(cbuff) == 0) (void) strcpy(cbuff, "All");
	  nadc_write_text(outfl, ++nr, "KeyDataSets", cbuff);
/*
 * selected measurement data sets
 */
	  (void) strcpy(cbuff, "");
	  if (nadc_get_param_uint8("write_limb") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoLimb");
	  }
	  if (nadc_get_param_uint8("write_moni") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoMonitoring");
	  }
	  if (nadc_get_param_uint8("write_nadir") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoNadir");
	  }
	  if (nadc_get_param_uint8("write_occ") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoOcc");
	  }
	  if (strlen(cbuff) == 0) (void) strcpy(cbuff, "All");
	  nadc_write_text(outfl, ++nr, "MeasurementDataSets", cbuff);
/*
 * PMD data
 */
	  if (nadc_get_param_uint8("write_pmd") == PARAM_SET)
	       nadc_write_text(outfl, ++nr, "PMD", "True");
	  else
	       nadc_write_text(outfl, ++nr, "PMD", "None");
/*
 * fractional polarisation data
 */
	  if (nadc_get_param_uint8("write_polV") == PARAM_SET)
	       nadc_write_text(outfl, ++nr, "FracPol", "True");
	  else
	       nadc_write_text(outfl, ++nr, "FracPol", "None");
/*
 * selected Categories
 */
	  nadc_repr_param_cat(MAX_STRING_LENGTH, cbuff);
	  nadc_write_text(outfl, ++nr, "Category", cbuff);
/*
 * selected States
 */
	  nadc_repr_param_state(MAX_STRING_LENGTH, cbuff);
	  nadc_write_text(outfl, ++nr, "States", cbuff);
/*
 * spectral bands
 */
	  nadc_repr_param_chan(SHORT_STRING_LENGTH, cbuff);
	  nadc_write_text(outfl, ++nr, "Bands", cbuff);
/*
 * selected Clusters
 */
	  nadc_repr_param_clus(MAX_STRING_LENGTH, cbuff);
	  nadc_write_text(outfl, ++nr, "Clusters", cbuff);
/*
 * calibration of the spectral band data
 */
	  scia_get_calib(string);
	  nadc_write_text(outfl, ++nr, "Calibration", string);
	  break;
/*
 *  ----- SCIAMACHY level 2 processor specific options
 */
     case SCIA_LEVEL_2:
/*
 * selected Key DSD's
 */
	  (void) strcpy(cbuff, "");
	  if (nadc_get_param_uint8("write_ads") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoADS");
	  }
	  if (nadc_get_param_uint8("write_gads") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoGADS");
	  }
	  if (strlen(cbuff) == 0) (void) strcpy(cbuff, "All");
	  nadc_write_text(outfl, ++nr, "KeyDataSets", cbuff);
/*
 * selected measurement data sets
 */
	  (void) strcpy(cbuff, "");
	  if (nadc_get_param_uint8("write_cld") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoCLD");
	  }
/* NRT, only */
	  if (nadc_get_param_uint8("write_bias") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoBIAS");
	  }
	  if (nadc_get_param_uint8("write_doas") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoDOAS");
	  }
/* offline, only */
	  if (nadc_get_param_uint8("write_limb") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoLimb");
	  }
	  if (nadc_get_param_uint8("write_nadir") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoNadir");
	  }
	  if (nadc_get_param_uint8("write_occ") == PARAM_UNSET) {
	       if (strlen(cbuff) > 0) (void) strcat(cbuff, ",");
	       (void) strcat(cbuff, "NoOcc");
	  }
	  if (strlen(cbuff) == 0) (void) strcpy(cbuff, "All");
	  nadc_write_text(outfl, ++nr, "MeasurementDataSets", cbuff);
	  break;
    }
}
