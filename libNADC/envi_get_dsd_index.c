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

.IDENTifer   ENVI_GET_DSD_INDEX
.AUTHOR      R.M. van Hees
.KEYWORDS    Envisat PDS 
.LANGUAGE    ANSI C
.PURPOSE     get index to a DSD given its name
.INPUT/OUTPUT
  call as   indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );

     input:  
          unsigned int num_dsd  : number of DSDs
	  struct dsd_envi dsd[] : characteristics of DSD
	  char   dsd_name[]     : name of the DSD

.RETURNS     index to DSD (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      3.0   22-Sep-2008	renamed to general Envisat module, RvH
              2.2   26-Mar-2002	keywords have to match exactly, RvH
              2.1   22-Mar-2002	check presence of DSD, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.0   04-Oct-2001	Created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdlib.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#define _ENVI_COMMON
#include <nadc_common.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int ENVI_GET_DSD_INDEX( unsigned int num_dsd, 
				 const struct dsd_envi *dsd,
				 const char descriptor[] )
{
     register unsigned int nd = 0;

     const char prognm[] = "ENVI_GET_DSD_INDEX";

     do {
	  if ( strcmp( dsd[nd].name, descriptor ) == 0 ) break;
     } while ( ++nd < num_dsd );

     if ( nd == num_dsd )
	  NADC_GOTO_ERROR( prognm, NADC_PDS_DSD_ABSENT, descriptor );
 done:
     return nd;
}
