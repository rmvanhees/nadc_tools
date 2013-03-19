/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2003 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_CORR_LOS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 1b/c
.LANGUAGE    ANSI C
.PURPOSE     correct the line-of-sight azimuth and zenith angles (geoN)
.INPUT/OUTPUT
  call as    SCIA_LV1_CORR_LOS( state, mds_1b );
     input:
	    struct state1_scia *state  : structure with States of the product
    output:
            struct mds1_scia *mds_1b   : structure holding level 1b MDS records

.RETURNS     nothing
.COMMENTS    The values of the level 1b line-of-sight zenith angles are
             always larger than zero, and the azimuth angle jumps with 180
             degrees while scanning through nadir.
	     This function will modify these values as follows: removing 
	     the jump in the azimuth angles and returns negative zenith angles,
	     when the original azimuth angle was larger than 180 degree.
	     This makes interpolation much easier.
.ENVIRONment None
.VERSION     1.2     17-Oct-2005   pass state-record by reference, RvH
             1.1     29-Nov-2004   check state.type_mds: modify Nadir only, RvH
             1.0     18-Aug-2004   created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <string.h>
#include <stdlib.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_CORR_LOS( const struct state1_scia *state,
			struct mds1_scia *mds_1b )
{
     const char prognm[] = "SCIA_LV1_CORR_LOS";

     register unsigned int nc, nd, ng;

     if ( state->type_mds != SCIA_NADIR ) return;

     nd = 0;
     do {
	  ng = 0;
	  do {
	       for ( nc = 0; nc < 3; nc++ ) {
		    if ( mds_1b[nd].geoN[ng].los_azi_ang[nc] > 180.f ) {
			 mds_1b[nd].geoN[ng].los_azi_ang[nc] -= 180;
			 mds_1b[nd].geoN[ng].los_zen_ang[nc] *= -1;
		    }
	       }
	  } while ( ++ng < mds_1b[nd].n_aux );
     } while ( ++nd < state->num_dsr );

     NADC_ERROR( prognm, NADC_ERR_NONE, 
                 "\n\tapplied correction to line-of-sight geolocation" );
}
