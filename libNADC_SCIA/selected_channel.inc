/*+++++++++++++++++++++++++
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

.IDENTifer   SELECTED_CHANNEL
.PURPOSE     check if a band is selected by the user
.INPUT/OUTPUT
  call as   flag = SELECTED_CHANNEL( chan_mask, chan_id )
     input:  
	    unsigned char chan_mask : mask for channel selection
    output:  
	    unsigned char chan_id   : channel id

.RETURNS     TRUE if channel is selected, else FALSE
.COMMENTS    ANSI-C inline function, declare in your code as:
             #include <selected_channel.inc>
-------------------------*/
#ifndef  __SELECT_CHAN                            /* Avoid redefinitions */
#define  __SELECT_CHAN

static inline
bool SELECTED_CHANNEL( unsigned char chan_mask, unsigned char chan_id )
{
     switch ( (int) chan_id ) {
     case 1:
	  if ( (chan_mask & BAND_ONE) == UCHAR_ZERO ) return FALSE;
	  break;
     case 2:
	  if ( (chan_mask & BAND_TWO) == UCHAR_ZERO ) return FALSE;
	  break;
     case 3:
	  if ( (chan_mask & BAND_THREE) == UCHAR_ZERO ) return FALSE;
	  break;
     case 4:
	  if ( (chan_mask & BAND_FOUR) == UCHAR_ZERO ) return FALSE;
	  break;
     case 5:
	  if ( (chan_mask & BAND_FIVE) == UCHAR_ZERO ) return FALSE;
	  break;
     case 6:
	  if ( (chan_mask & BAND_SIX) == UCHAR_ZERO ) return FALSE;
	  break;
     case 7:
	  if ( (chan_mask & BAND_SEVEN) == UCHAR_ZERO ) return FALSE;
	  break;
     case 8:
	  if ( (chan_mask & BAND_EIGHT) == UCHAR_ZERO ) return FALSE;
	  break;
     default:
          NADC_ERROR( NADC_ERR_FATAL, "unknown detector band" );
          return FALSE;
     }
     return TRUE;
}
#endif
