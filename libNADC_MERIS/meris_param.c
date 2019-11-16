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

.IDENTifer   MERIS_PARAM
.AUTHOR      R.M. van Hees
.KEYWORDS    command-line parameter handling
.LANGUAGE    ANSI C
.PURPOSE     initializes param-structure with command-line parameters
.CONTAINS    MERIS_SET_PARAM, MERIS_SHOW_PARAM
.RETURNS     Nothing (check global error status)
.COMMENTS    None
.ENVIRONment None
.VERSION      7.0   31-May-2019 replaced usage of struct param_record, RvH
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
#include <nadc_meris.h>
#include "../VERSION"

/*+++++ Static Variables +++++*/
static const
struct nadc_env {
     /*@null@*/ const char *opt_key;
     /*@null@*/ const char *opt_def_val;
     const char *opt_desc;
     int        id_instr;
} nadc_envs[] = {
/* last and empty entry */
     { NULL, NULL, "", 0 }
};

static const
struct nadc_opt {
     /*@null@*/ const char *opt_key;
     /*@null@*/ const char *opt_def_val;
     const char     *opt_desc;
     int            id_instr;
} nadc_opts[] = {
/* general options */
     { "-h", NULL, "\tdisplay this help and exit [default]",
       (MERIS_LEVEL_1|MERIS_LEVEL_2) },
     { "-help", NULL, "display this help and exit",
       (MERIS_LEVEL_1|MERIS_LEVEL_2) },
     { "-V", NULL, "\tdisplay version & copyright information and exit",
       (MERIS_LEVEL_1|MERIS_LEVEL_2) },
     { "-show_param", NULL, 
       "display setting of command-line parameters; no output generated",
       (MERIS_LEVEL_1|MERIS_LEVEL_2) },
     { "-version", NULL, "display version & copyright information and exit",
       (MERIS_LEVEL_1|MERIS_LEVEL_2) },
     { "-verbose", NULL, "verbose mode",
       (MERIS_LEVEL_1|MERIS_LEVEL_2) },
     { "-silent", NULL, "do not display any error messages",
       (MERIS_LEVEL_1|MERIS_LEVEL_2) },
     { "-ascii", NULL, "write output in ASCII format",
       (MERIS_LEVEL_1|MERIS_LEVEL_2) },
     { "--output", "=<outfile>", "(default: <infile> + appropriate extension)",
       (MERIS_LEVEL_1|MERIS_LEVEL_2) },
/* last and empty entry */
     { NULL, NULL, "", 0 }
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

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   MERIS_SET_PARAM
.PURPOSE     initializes struct with command-line parameters
.INPUT/OUTPUT
  call as   MERIS_SET_PARAM(argc, argv, instrument);
     input:  
             int  argc       :   number of parameters
	     char *argv[]    :   parameter values
	     int instrument  :   code for instrument en data product level

.RETURNS     Nothing (check global error status)
.COMMENTS    none
-------------------------*/
void MERIS_SET_PARAM(int argc, char *argv[], int instrument)
{
     char   *cpntr;
     char   *name_infile = NULL;
     char   prog_master[SHORT_STRING_LENGTH];
     int    narg;
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
     (void) nadc_set_param_string("program", argv[0]);
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
		    (void) nadc_set_param_uint8("flag_version", PARAM_SET);
		    return;
	       }
	       if (strncmp(argv[narg]+1, "show", 4) == 0) {
		    (void) nadc_set_param_uint8("flag_show", PARAM_SET);
		    (void) nadc_set_param_uint8("write_ascii", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "silent", 6) == 0) {
		    (void) nadc_set_param_uint8("flag_silent", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "verbose", 7) == 0) {
		    (void) nadc_set_param_uint8("flag_verbose", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "check", 5) == 0) {
		    (void) nadc_set_param_uint8("flag_check", PARAM_SET);
	       } else if (strncmp(argv[narg]+1, "ascii", 5) == 0) {
		    (void) nadc_set_param_uint8("write_ascii", PARAM_SET);
	       }
	  } else {
	       /* name of input file */
	       nadc_set_param_string("infile", argv[narg]);
	       name_infile = argv[narg];
	  }
     }
     /* User has to give the name of the input filename */
     if (nadc_get_param_string("infile") == NULL) 
	  Show_All_Options(stderr, instrument, prog_master);

     /* make sure that the output filename is defined */
     if (nadc_get_param_string("outfile") == NULL) {
	  if ((cpntr = strrchr(name_infile, '/')) != NULL)
	       nadc_set_param_string("outfile", cpntr+1);
	  else
	       nadc_set_param_string("outfile", name_infile);
     }
     
     /* default is to generate output in HDF5 format */
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  (void) nadc_set_param_add_ext("outfile", ".child");
     } else if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	  (void) nadc_set_param_uint8("write_hdf5", PARAM_UNSET);
     } else {
	  (void) nadc_set_param_uint8("write_hdf5", PARAM_SET);
	  (void) nadc_set_param_add_ext("outfile", ".h5");
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   MERIS_SHOW_PARAM
.PURPOSE     show command-line settings, as stored in the param-record
.INPUT/OUTPUT
  call as   MERIS_SHOW_PARAM(instrument);
     input:  
             int instrument     :   code for instrument en data product level

.RETURNS     Nothing (check global error status)
.COMMENTS    none
-------------------------*/
void MERIS_SHOW_PARAM(int instrument __attribute ((unused)))
{
     register unsigned int nr = 0;

     char    *cpntr, cbuff[MAX_STRING_LENGTH];
     time_t  tp[1];
     
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
	  cpntr = nadc_get_param_string("outfile");
	  nadc_write_text(outfl, ++nr, "Output filename", cpntr);
	  free(cpntr);
	  if (nadc_get_param_uint8("flag_deflate") == PARAM_SET)
	       nadc_write_text(outfl, ++nr, "Compression", "True");
	  else
	       nadc_write_text(outfl, ++nr, "Compression", "False");
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
}
