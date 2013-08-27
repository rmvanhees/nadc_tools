/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1C_SELECT
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1c data selection
.LANGUAGE    ANSI C
.PURPOSE     select level 1c MDS records on cluster ID
.INPUT/OUTPUT
  call as   nr_mds = SCIA_LV1C_SELECT_MDS( clus_mask, state, &mds_state );
     input: 
            int source                 : data source (Nadir, Limb, ...)
	    struct param_record param  : struct holding user-defined settings
	    unsigned int num_dsd       : number of DSD records
	    struct dsd_envi *dsd       : structure with DSD records
	    unsigned int num_state     : number of State records
    output:  
	    struct state1_scia *state  : structure with States of the product
	    struct mds1c_scia *mds     : structure with Level 1c MDS records

.RETURNS     number of selected records (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      1.0   27-Aug-2013	created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int SCIA_LV1C_SELECT_MDS( unsigned long long clus_mask,
				   struct state1_scia *state, 
				   struct mds1c_scia *mds )
{
     /* const char prognm[] = "SCIA_LV1C_SELECT_MDS"; */

     register unsigned short nc, ncc;

     unsigned int num_mds = state->num_clus;

     if ( clus_mask == ~0ULL ) return num_mds;
/*
 * update state-record to reflect the actual cluster stored in the MDS record
 */
     nc = ncc = 0;
     do {
	  if ( Get_Bit_LL( clus_mask, (unsigned char) nc ) == 1ULL ) {
	       if ( ncc < nc ) {
		    (void) memmove( &state->Clcon[ncc], &state->Clcon[nc],
				    sizeof( struct Clcon_scia ) );
		    (void) memmove( mds+ncc, mds+nc, 
				    sizeof( struct mds1c_scia ) );
	       }
	       ncc++;
	  }
     } while ( ++nc < state->num_clus );
     state->num_clus = ncc;

     return (num_mds = (unsigned int) ncc);
}
