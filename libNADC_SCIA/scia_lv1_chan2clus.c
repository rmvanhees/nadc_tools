/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2002 - 2019 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_CHAN2CLUS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 1 data selection
.LANGUAGE    ANSI C
.PURPOSE     combine channel and cluster selection into a mask (0/1)
.INPUT/OUTPUT
  call as   clus_mask = SCIA_LV1_CHAN2CLUS(state);

     input: 
	     struct state1_scia *state :  structure with States of the product

.RETURNS     mask with clusters to be selected (unsigned long long)
.COMMENTS    None
.ENVIRONment None
.VERSION     1.1     17-Oct-2005  pass state-record by reference, RvH
             1.0     29-Jul-2002  created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
	/* NONE */

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include "selected_channel.inc"

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned
long long SCIA_LV1_CHAN2CLUS(const struct state1_scia *state)
{
     register unsigned short nc = 0;

     unsigned long long clus_mask = 0ULL;

     do {
	  int ichan = state->Clcon[nc].channel - 1;

	  if (nadc_get_param_chan(ichan) == (unsigned char) 0)
	       continue;

	  if (nadc_get_param_clus(nc) == 0ULL)
	       continue;
	  
	  Set_Bit_LL(&clus_mask, (unsigned char) nc);
     } while(++nc < state->num_clus);

     return clus_mask;
}
