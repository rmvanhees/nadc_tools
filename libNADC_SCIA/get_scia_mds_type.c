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

.IDENTifer   GET_SCIA_MDS_TYPE
.AUTHOR      R.M. van Hees
.LANGUAGE    ANSI C
.PURPOSE     get MDS type identifier: Nadir, Limb, Occultation, or Monitor
.INPUT/OUTPUT
  call as   mdsType = GET_SCIA_MDS_TYPE( unsigned char stateID )

     input:  
             unsigned char stateID : state ID

.RETURNS     unsigned char
.COMMENTS    None
.VERSION      1.0     10-Apr-2008   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/

/*+++++ Local Headers +++++*/
#define _SCIA_COMMON
#include <nadc_scia.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
static
const unsigned char SciaTypeMDS[MAX_NUM_STATE] = {
     SCIA_NADIR, SCIA_NADIR, SCIA_NADIR, SCIA_NADIR, SCIA_NADIR, 
     SCIA_NADIR, SCIA_NADIR, SCIA_MONITOR, SCIA_NADIR, SCIA_NADIR, 
     SCIA_NADIR, SCIA_NADIR, SCIA_NADIR, SCIA_NADIR, SCIA_NADIR, 
     SCIA_MONITOR, SCIA_MONITOR, SCIA_MONITOR, SCIA_MONITOR, SCIA_MONITOR,
     SCIA_MONITOR, SCIA_MONITOR, SCIA_NADIR, SCIA_NADIR, SCIA_NADIR, 
     SCIA_MONITOR, SCIA_LIMB, SCIA_LIMB, SCIA_LIMB, SCIA_LIMB,
     SCIA_LIMB, SCIA_LIMB, SCIA_LIMB, SCIA_LIMB, SCIA_LIMB, 
     SCIA_LIMB, SCIA_LIMB, SCIA_MONITOR, SCIA_MONITOR, SCIA_LIMB, 
     SCIA_LIMB, SCIA_NADIR, SCIA_NADIR, SCIA_NADIR, SCIA_NADIR, 
     SCIA_MONITOR, SCIA_OCCULT, SCIA_MONITOR, SCIA_OCCULT, SCIA_MONITOR, 
     SCIA_OCCULT, SCIA_MONITOR, SCIA_MONITOR, SCIA_MONITOR, SCIA_OCCULT,
     SCIA_OCCULT, SCIA_OCCULT, SCIA_MONITOR, SCIA_MONITOR, SCIA_MONITOR, 
     SCIA_MONITOR, SCIA_MONITOR, SCIA_MONITOR, SCIA_MONITOR, SCIA_MONITOR,
     SCIA_MONITOR, SCIA_MONITOR, SCIA_MONITOR, SCIA_MONITOR, SCIA_MONITOR
};

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
unsigned char GET_SCIA_MDS_TYPE( unsigned char stateID )
{
     const char prognm[] = "GET_SCIA_MDS_TYPE";

     if ( stateID >= (unsigned char) 1 
	  && stateID <= (unsigned char) MAX_NUM_STATE )
	  return SciaTypeMDS[stateID-1];

     NADC_ERROR( prognm, NADC_ERR_FATAL, "state ID out-of-range" );
     return UCHAR_ZERO;
}

