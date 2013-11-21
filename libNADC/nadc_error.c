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

.IDENTifer   NADC_ERROR
.AUTHOR      R.M. van Hees
.KEYWORDS    error handling
.LANGUAGE    ANSI C
.PURPOSE     error handling and display routines
.COMMENTS    contains NADC_Err_Push, NADC_Err_Clear, NADC_Err_Keep, 
             NADC_Err_Trace
             used global variables: nadc_stat and nadc_err_stack
.ENVIRONment None
.VERSION      1.5   30-Jul-2007 added counter to repeated errors, KB
              1.4   25-Feb-2003 added 2 functions to save and restore
                                error status en messages, RvH
              1.3   25-Feb-2003 did some code clean-up, RvH
              1.2   22-Mar-2002 added flag to indicate absence of DSD's, RvH
              1.1   07-Dec-2001	added possibility to give warnings, RvH
              1.0   31-Oct-2001	Created by R. M. van Hees 
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

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++ Global Variables +++++*/
unsigned char nadc_stat = NADC_STAT_SUCCESS;

NADC_E_t nadc_err_stack;

/*+++++ Local Variables +++++*/
static const NADC_mesg_t NADC_Err_mesg[] = {
     {NADC_ERR_NONE,       "No error, issuing a notice or debug message" },
     {NADC_ERR_WARN,       "No error, issuing a warning"},
     {NADC_ERR_FATAL,      "Fatal, see below for description of error(s)"},

     {NADC_ERR_ALLOC,      "Fatal, memory allocation problems" },
     {NADC_ERR_MXDIMS,     "Fatal, exceed maximum number of dimensions" },
     {NADC_ERR_STRLEN,     "Fatal, string too short" },

     {NADC_ERR_PARAM,      "Fatal, invalid function parameters" },

     {NADC_ERR_CALIB,      "Fatal, problems during calibration" },

     {NADC_ERR_FILE,       "Fatal, unable to open file" },
     {NADC_ERR_FILE_CRE,   "Fatal, unable to create file" },
     {NADC_ERR_FILE_RD,    "Fatal, unable to read from file" },
     {NADC_ERR_FILE_WR,    "Fatal, unable to write to file (disk full?)" },

     {NADC_PDS_DSD_ABSENT, "Fatal, the requested PDS DSD is not found" },
     {NADC_SDMF_ABSENT,    "Fatal, no data found for this orbit in SDMF"},

     {NADC_ERR_SQL,        "Fatal, problems reported by the PostgreSQL C-API"},
     {NADC_ERR_SQL_TWICE,  "Warning, entry is already inserted in database"},

     {NADC_WARN_PDS_RD,    "Warning, corrupted data packet found, skipping..."},
     {NADC_ERR_PDS_RD,     "Fatal, problems while reading a PDS data set" },
     {NADC_ERR_PDS_WR,     "Fatal, problems while writing a PDS data set" },
     {NADC_ERR_PDS_KEY,    "Fatal, can not find keyword in PDS data set" },
     {NADC_ERR_PDS_DSD,    "Fatal, invalid/unkown name for PDS DSD set" },
     {NADC_ERR_PDS_SIZE,   "Fatal, PDS data set differs from definition" },

     {NADC_ERR_HDF_FILE,   "Fatal, problems with HDF5 file interface" },
     {NADC_ERR_HDF_CRE,    "Fatal, problems creating a HDF5 file" },
     {NADC_ERR_HDF_RD,     "Fatal, problems reading from a HDF5 file" },
     {NADC_ERR_HDF_WR,     "Fatal, problems writing to a HDF5 file" },
     {NADC_ERR_HDF_GRP,    "Fatal, problems with HDF5 group interface" },
     {NADC_ERR_HDF_DATA,   "Fatal, problems with HDF5 data set interface" },
     {NADC_ERR_HDF_ATTR,   "Fatal, problems with HDF5 attribute interface" },
     {NADC_ERR_HDF_SPACE,  "Fatal, problems releasing HDF5 data-space" },
     {NADC_ERR_HDF_DTYPE,  "Fatal, problems with HDF5 data-type" },
     {NADC_ERR_HDF_PLIST,  "Fatal, problems with HDF5 data-properties" },
     {NADC_ERR_DORIS,      "Warning or Fatal error reported by Envisat CFI s/w"}
};

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static
short NADCget_err_mesg( NADC_err_t mesg_num )
{
     register short ni;

     const short NumMesg = 
	  (short) (sizeof( NADC_Err_mesg ) / sizeof( NADC_mesg_t ));

     for ( ni = 0; ni < NumMesg; ni++ ) {
	  if ( NADC_Err_mesg[ni].error_code == mesg_num ) return ni;
     }
     return (short)(-1);
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   NADC_Err_Push
.PURPOSE     push error message on stack
.INPUT/OUTPUT
  call as    NADC_Err_Push( mesg_num, file_name, func_name, line, desc );
            
     input:  
           NADC_err_t mesg_num   :  id of error message
	   char       *file_name :  name of the module,
                                    where the error was issued
	   char       *func_name :  name of the function,
                                    where the error was issued
	   int        line       :  line where the error was issued
	   char       *desc      :  string with additional info

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void NADC_Err_Push( NADC_err_t mesg_num, const char *file_name, 
		    const char *func_name, int line, const char *desc )
{
     register unsigned short ns;

     const unsigned short nused = nadc_err_stack.nused;
/*
 * check number of messages already stored
 */
     if ( nused >= NADC_E_NSLOTS ) return;
/*
 * do not repeat messages, instead count them
 */
     for ( ns = 0; ns < nused; ns++ ) {
	  if ( nadc_err_stack.slots[ns].mesg_num == mesg_num
	       && nadc_err_stack.slots[ns].line == line
	       && strncmp( nadc_err_stack.slots[ns].file_name, 
			   file_name, SHORT_STRING_LENGTH ) == 0
	       && strncmp( nadc_err_stack.slots[ns].desc, 
			   desc, MAX_STRING_LENGTH ) == 0 ) {
	       nadc_err_stack.slots[ns].count++;
	       return;
	  }
     }
/*
 * store new message
 */
     nadc_err_stack.slots[nused].mesg_num = mesg_num;
     (void) nadc_strlcpy( nadc_err_stack.slots[nused].file_name,
			  file_name, SHORT_STRING_LENGTH );
     (void) nadc_strlcpy( nadc_err_stack.slots[nused].func_name,
			  func_name, SHORT_STRING_LENGTH );
     nadc_err_stack.slots[nused].line = line;
     (void) nadc_strlcpy( nadc_err_stack.slots[nused].desc,
			  desc, MAX_STRING_LENGTH );
     nadc_err_stack.nused++;

     switch ( mesg_num ) {
     case NADC_ERR_NONE:
	  nadc_stat |= NADC_STAT_INFO;
	  break;
     case NADC_PDS_DSD_ABSENT:
     case NADC_SDMF_ABSENT:
	  nadc_stat |= NADC_STAT_ABSENT;
	  break;
     case NADC_ERR_WARN:
     case NADC_WARN_PDS_RD:
     case NADC_ERR_SQL_TWICE:
	  nadc_stat |= NADC_STAT_WARN;
	  break;
     default: 
	  nadc_stat |= NADC_STAT_FATAL;
	  break;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_Err_Clear
.PURPOSE     clear error stack
.INPUT/OUTPUT
  call as    NADC_Err_Clear();

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void NADC_Err_Clear( void )
{
     nadc_stat = NADC_STAT_SUCCESS;
     nadc_err_stack.nused = 0u;
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_Err_Keep
.PURPOSE     save error status
.INPUT/OUTPUT
  call as    NADC_Err_Keep( do_save );
     input:  
           bool  do_save    :  save current error status

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void NADC_Err_Keep( bool do_save )
{
     static unsigned char  nadc_stat_save = NADC_STAT_SUCCESS;
     static unsigned short nused_save = 0u;

     if ( do_save ) {
	  nadc_stat_save = nadc_stat;
	  nused_save = nadc_err_stack.nused;
     } else {
	  nadc_stat = nadc_stat_save;
	  nadc_err_stack.nused = nused_save;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer  NADC_Err_Trace 
.PURPOSE     write error messages to stream
.INPUT/OUTPUT
  call as    NADC_Err_Trace( stream );
     input:  
           FILE *stream    :  open stream to write messages

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void NADC_Err_Trace( FILE *stream )
{
     register short nm;
     register unsigned short nr = 0;
     register unsigned short nused = nadc_err_stack.nused;

     if ( nadc_stat == UCHAR_ZERO ) return;

     while ( nused-- > 0u ) {
	  NADC_err_t mesg_num = nadc_err_stack.slots[nused].mesg_num;

	  (void) fprintf( stream, "#%03hu: %s line %-d in %s(): %s\n",
			  nr++, 
			  nadc_err_stack.slots[nused].file_name, 
			  nadc_err_stack.slots[nused].line,
			  nadc_err_stack.slots[nused].func_name,
			  nadc_err_stack.slots[nused].desc );

	  if ( (nm = NADCget_err_mesg( mesg_num )) == (short)(-1) )
	       (void) fprintf( stream, "message(%-d): %s\n",
			  mesg_num, "Invalid error message number" );
	  else
	       (void) fprintf( stream, "message(%-d): %s\n",
			       mesg_num, NADC_Err_mesg[nm].str );
	  if ( nadc_err_stack.slots[nused].count > 1)
	       (void) fprintf( stream, "message repeated %-d times.\n",
			       nadc_err_stack.slots[nused].count );
     }
     
}
