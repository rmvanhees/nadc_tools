/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2006 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_INFO
.AUTHOR      R.M. van Hees
.KEYWORDS    
.LANGUAGE    ANSI C
.PURPOSE     display information on the callers display
.COMMENTS    NADC_Info_Proc, NADC_Info_Update, NADC_Info_Finish
.ENVIRONment None
.VERSION     1.1     12-Apr-2008   routine also works with stdout, RvH
             1.0     26-Jan-2006   initial release by R. M. van Hees
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

/*+++++ Static Variables +++++*/
static bool nadc_info_init = FALSE;

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   NADC_Info_Proc
.PURPOSE     display information about the progress of a certain routine
.INPUT/OUTPUT
  call as    NADC_Info_Proc( stream, proc_name, counter );
            
     input:  
           FILE *stream          :  pointer to open straem
	   char *proc_name       :  name of the routine
	   unsigned int counter  :  total number of actions performed
           

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void NADC_Info_Proc( FILE *stream, const char proc_name[], 
		     unsigned int counter )
{
     (void) fprintf( stream, "Processing %s [%-u]\t-\t", proc_name, counter );
     (void) fflush( stdout );
     nadc_info_init = TRUE;
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_Info_Update
.PURPOSE     update message to user, to be called after every call of routine
.INPUT/OUTPUT
  call as    NADC_Info_Update( stream, digits, counter );
            
     input:  
           FILE *stream          :  pointer to open straem
	   unsigned short digits :  specify number of digits to be displayed
	   unsigned int counter  :  total number of actions performed

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void NADC_Info_Update( FILE *stream, unsigned short digits,
		       unsigned int counter )
{
     unsigned short ni;

     char str_fmt[8];

     if ( ! nadc_info_init ) {
	  char *sbuff = (char *) malloc( digits + 1 );

	  (void) strcpy( sbuff, "" );
	  for ( ni = 0; ni < digits; ni++ ) (void) strcat( sbuff, "\b" );
	  (void) fprintf( stream, "%s", sbuff );
	  (void) fflush( stdout );
	  (void) free( sbuff );
     } else
	  nadc_info_init = FALSE;
     (void) snprintf( str_fmt, 8, "%%0%-huu", digits );
     (void) fprintf( stream, str_fmt, counter );
     (void) fflush( stdout );
}

/*+++++++++++++++++++++++++
.IDENTifer   NADC_Info_Finish
.PURPOSE     finalise message after routine has finished
.INPUT/OUTPUT
  call as    NADC_Info_Finish( stream, digits, counter );
            
     input:  
           FILE *stream          :  pointer to open straem
	   unsigned short digits :  specify number of digits to be displayed
	   unsigned int counter  :  total number of actions performed

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void NADC_Info_Finish( FILE *stream, unsigned short digits,
		       unsigned int counter )
{
     unsigned short ni;

     char str_fmt[8];

     if ( nadc_info_init )
	  (void) fprintf( stream, "finished\n" );
     else {
	  char *sbuff = (char *) malloc( digits + 1 );

	  (void) strcpy( sbuff, "" );
	  for ( ni = 0; ni < digits; ni++ ) (void) strcat( sbuff, "\b" );
	  (void) fprintf( stream, "%s", sbuff );
	  (void) free( sbuff );
	  (void) snprintf( str_fmt, 8, "%%0%-huu\n", digits );
	  (void) fprintf( stream, str_fmt, counter );
     }
     (void) fflush( stdout );
}
