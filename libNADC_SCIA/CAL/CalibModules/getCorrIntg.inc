/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2002 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   getCorrIntg
.AUTHOR      R.M. van Hees
.KEYWORDS    math function
.LANGUAGE    ANSI C
.PURPOSE     return integration time, including a correction for channel 6-8

.INPUT/OUTPUT
  call as   intg = getCorrIntg( Clcon );
     input:  
             struct Clcon_scia Clcon : 

.RETURNS     corrected integration time
.COMMENTS    ANSI-C inline function, declare in your code as:
             #include <getCorrIntg.inc>
------------------------------------------------------------*/
#ifndef  __CORR_INTG                            /* Avoid redefinitions */
#define  __CORR_INTG

#include <float.h>

static inline
float getCorrIntg( const struct Clcon_scia Clcon )
{
     if ( Clcon.channel < FirstInfraChan ) {
	  return (Clcon.coaddf * Clcon.pet);
     } else {
	  char *env_str = getenv( "SCIA_CORR_PET" );
/*
 * This correction should only be applied on:
 * - Epitaxx read-outs
 * - not on hot-mode pixel exposure times
 * Check for double correction!
 */ 
	  if ( env_str == NULL || *env_str != '0' ) {
	       const float PET_Offset = 1.18125e-3;
	       const float PET_noHotMode = 0.03125f;

	       if ( Clcon.pet >= (PET_noHotMode - FLT_EPSILON)
		    && fmodf( Clcon.pet, PET_noHotMode) == 0.f )
		    return (Clcon.coaddf * (Clcon.pet - PET_Offset));
	  }
	  return (Clcon.coaddf * Clcon.pet);
     }
}
#endif
