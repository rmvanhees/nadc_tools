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

.IDENTifer   NADC_CHECK_FOR_SAA
.AUTHOR      R.M. van Hees
.KEYWORDS    
.LANGUAGE    ANSI C
.PURPOSE     obtain saaFlag
.INPUT/OUTPUT
  call as   saaFlag = NADC_CHECK_FOR_SAA( latitude, longitude );
     input:  
             double *latitude   :   latitude of instrument
	     double *longitude  :   longitude of instrument

.RETURNS     TRUE if above SAA region
.COMMENTS    ToDo - improve SAA definition
.ENVIRONment None
.VERSION     1.0     05-Nov-2008   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
	/* NONE */

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
bool NADC_CHECK_FOR_SAA( const double latitude, const double longitude )
{
     const double SAA_def[]={ -45, 0, 270, 360 };

     if ( (latitude > SAA_def[0]) && (latitude < SAA_def[1]) 
	  && (longitude > SAA_def[2]) && (longitude < SAA_def[3]) )
	  return TRUE;
     else
	  return FALSE;
}

