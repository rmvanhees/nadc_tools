/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2008 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   ADAGUC_INIT_PARAM
.AUTHOR      R.M. van Hees
.KEYWORDS    command-line parameter handling
.LANGUAGE    ANSI C
.PURPOSE     initializes struct with command-line parameters
.INPUT/OUTPUT
  call as   ADAGUC_INIT_PARAM(argc, argv, instrument, param);

     input:  
             int  argc       :   number of parameters
	     char *argv[]    :   parameter values
	     int instrument  :   code for instrument en data product level
    output:  
	     struct param_record 
	            *param   :     struct holding user-defined settings

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0   24-Nov-2008 Created by R. M. van Hees
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
#include <ctype.h>
#include <math.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++ Static Variables +++++*/
static const
struct adaguc_opt {
     /*@null@*/ const char *opt_key;
     /*@null@*/ const char *opt_def_val;
     const char     *opt_desc;
} adaguc_opts[] = {
/* general options */
     { "-h", NULL, "\tdisplay this help and exit [default]" },
     { "-help", NULL, "display this help and exit" },
     { "-V", NULL, "\tdisplay version & copyright information and exit" },
     { "-version", NULL, "display version & copyright information and exit" },
     { "-silent", NULL, "do not display any error messages" },
     { "--inputdir", NULL, "specifies input directory [default: ./]" },
     { "--outputdir", NULL, "specifies output directory [default: ./]" },
     { "--files", NULL, "provide a list of input files [ignored]" },
     { "--class", NULL, "file class, one out of OPER, CONS or TEST" },
     { "--start", "<YYYYMMDDThhmmss>", 
       "include all measerements equal or more than start" },
     { "--stop", "<YYYYMMDDThhmmss>", 
       "include all measerements equal or less than stop" },
     { "--clip", "<YYYYMMDD>", 
       "clips a certain day or month from input data, format YYYYMM[DD]" },
/* last and empty entry */
     { NULL, NULL, "" }
};

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   Show_All_Options
.PURPOSE     display (optional) command parameters
.INPUT/OUTPUT
  call as   Show_All_Options(stream, prognm);

     input:  
            FILE *stream      :   open (file) stream pointer
	    char prognm[]     :   string with name of the master program

.RETURNS     exits with EXIT_FAILURE
.COMMENTS    static function
-------------------------*/
static /*@exits@*/ 
void Show_All_Options(FILE *stream, /*@notnull@*/ const char prognm[])
     /*@modifies stream@*/
{
     register short nr;
/*
 * intro of message
 */
     (void) fprintf(stream, "Usage: %s [OPTIONS] FILE(S)\n\n", prognm);
/*
 * display all available NADC options
 */
     nr = -1;
     while (adaguc_opts[++nr].opt_key != NULL) {
	  (void) fprintf(stream, "   %s", adaguc_opts[nr].opt_key);
	  if (adaguc_opts[nr].opt_def_val != NULL)
	       (void) fprintf(stream, " %s", adaguc_opts[nr].opt_def_val);
	  (void) fprintf(stream, "\t%s\n", adaguc_opts[nr].opt_desc);
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
void Check_User_Option(FILE *stream, /*@notnull@*/ const char prognm[],
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
     while (adaguc_opts[++nr].opt_key != NULL) {
	  size_t opt_len = strlen(adaguc_opts[nr].opt_key);

	  if (strncmp(argv, adaguc_opts[nr].opt_key, opt_len) == 0) return;
     }
     (void) fprintf(stream, 
		     "%s: FATAL, unknown command-line option \"%s\"\n", 
		     prognm, argv);
     exit(EXIT_FAILURE);
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void ADAGUC_INIT_PARAM(int argc, char *argv[], struct param_adaguc *param)
{
     char   *cpntr;
     char   prog_master[SHORT_STRING_LENGTH];
     int    narg;
/*
 * check number of options
 */
     if (argc == 0 || argv[0] == NULL)
	  NADC_RETURN_ERROR(NADC_ERR_PARAM, "none found!?!");
/*
 * strip path to program
 */
     if ((cpntr = strrchr(argv[0], '/')) != NULL) {
	  (void) nadc_strlcpy(prog_master, ++cpntr, SHORT_STRING_LENGTH);
     } else {
	  (void) nadc_strlcpy(prog_master, argv[0], SHORT_STRING_LENGTH);
     }
/*
 * initialize struct param
 */
     param->flag_show    = PARAM_UNSET;
     param->flag_version = PARAM_UNSET;
     param->flag_silent  = PARAM_UNSET;
     param->flag_verbose = PARAM_UNSET;

     param->flag_indir   = PARAM_UNSET;
     param->flag_outdir  = PARAM_UNSET;
     param->flag_clip    = PARAM_UNSET;

     (void) nadc_strlcpy(param->prodClass, "CONS", 5);

     (void) nadc_strlcpy(param->clipStart, "19500101T000000", 16);
     (void) nadc_strlcpy(param->clipStop , "20500101T000000", 16);

     param->num_infiles = 0;
     param->name_infiles[0] = NULL;

     (void) strcpy(param->indir, ".");
     (void) strcpy(param->outdir, ".");
/*
 * get command-line parameters
 */
     narg = 0;
     while (++narg < argc) {
	  Check_User_Option(stderr, prog_master, argv[narg]);
/*
 * obtain name of input file
 */
	  if (argv[narg][0] != '-') {
	       if (param->num_infiles < MAX_ADAGUC_INFILES) {
		    param->name_infiles[param->num_infiles++] = argv[narg];
	       } else {
		    NADC_RETURN_ERROR(NADC_ERR_PARAM, argv[narg]);
	       }
/*
 * process command-line options starting with one "-" (= standalone options)
 */
	  } else if (argv[narg][0] == '-' && argv[narg][1] != '-') {
	       if (argv[narg][1] == 'V' 
		    || strncmp(argv[narg]+1, "version", 7) == 0) {
		    param->flag_version = PARAM_SET;
		    return;                             /* nothing else todo */
	       }
	       if ((argv[narg][1] == 'h' && argv[narg][2] == '\0')
		    || strncmp(argv[narg]+1, "help", 4) == 0)
		    Show_All_Options(stdout, prog_master);

	       if (strncmp(argv[narg]+1, "silent", 6) == 0) {
		    param->flag_silent = PARAM_SET;
	       } else if (strncmp(argv[narg]+1, "verbose", 7) == 0) {
		    param->flag_verbose = PARAM_SET;
	       }
	  } else if (argv[narg][0] == '-' && argv[narg][1] == '-' 
		      && narg+1 < argc) {
	       if (strncmp(argv[narg]+2, "inputdir", 5) == 0) {
		    param->flag_indir = PARAM_SET;
		    (void) nadc_strlcpy(param->indir, argv[narg+1], 
				    MAX_STRING_LENGTH);
	       } else if (strncmp(argv[narg]+2, "outputdir", 6) == 0) {
		    param->flag_outdir = PARAM_SET;
		    (void) nadc_strlcpy(param->outdir, argv[narg+1], 
				    MAX_STRING_LENGTH);
	       } else if (strncmp(argv[narg]+2, "class", 5) == 0) {
		    (void) nadc_strlcpy(param->prodClass, argv[narg+1], 5);
	       } else if (strncmp(argv[narg]+2, "clip", 4) == 0) {
		    int res;
		 
		    param->flag_clip = PARAM_SET;
		    if (strlen(argv[narg+1]) == 8) {
			 int ibuff = atoi(argv[narg+1]);

			 res = snprintf(param->clipStart, 16, 
					 "%dT000000", ibuff);
			 if (res < 0)
			   Show_All_Options(stdout, prog_master);
			 res = snprintf(param->clipStop, 16, 
					 "%dT000000", ibuff+1);
			 if (res < 0)
			   Show_All_Options(stdout, prog_master);
		    } else if (strlen(argv[narg+1]) == 6) {
			 int ibuff = atoi(argv[narg+1]);

			 res = snprintf(param->clipStart, 16, 
					  "%d01T000000", ibuff);
			 if (res < 0)
			   Show_All_Options(stdout, prog_master);
			 res = snprintf(param->clipStop, 16, 
					  "%d01T000000", ibuff+1);
			 if (res < 0)
			   Show_All_Options(stdout, prog_master);
		    } else {
			 Show_All_Options(stdout, prog_master);
		    }
	       } else if (strncmp(argv[narg]+2, "start", 5) == 0) {
		    param->flag_clip = PARAM_SET;
		    (void) nadc_strlcpy(param->clipStart, argv[narg+1], 16);
	       } else if (strncmp(argv[narg]+2, "stop", 4) == 0) {
		    param->flag_clip = PARAM_SET;
		    (void) nadc_strlcpy(param->clipStop, argv[narg+1], 16);
	       }
	       narg++;
	  }
     }
/*
 * User has to give the name of the input filename
 */
     if (param->num_infiles == 0 && param->flag_indir == PARAM_UNSET) 
	  Show_All_Options(stderr, prog_master);
}
