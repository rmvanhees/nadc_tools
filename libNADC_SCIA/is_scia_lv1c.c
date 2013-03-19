/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2000 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   IS_SCIA_LV1C
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1c data
.LANGUAGE    ANSI C
.PURPOSE     test level of the product: 1b/1c
.INPUT/OUTPUT
  call as    is_level_1c = IS_SCIA_LV1C( num_dsd, dsd );
     input:
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
            
.RETURNS     TRUE if level 1c, else FALSE
.COMMENTS    static function
.ENVIRONment None
.VERSION      1.0   18-Sep-2002 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
bool IS_SCIA_LV1C( unsigned int num_dsd, const struct dsd_envi *dsd )
{
     bool is_level_1c = TRUE;

     NADC_ERR_SAVE();
     (void) ENVI_GET_DSD_INDEX( num_dsd, dsd, "CAL_OPTIONS" );
     if ( IS_ERR_STAT_ABSENT ) is_level_1c = FALSE;
     NADC_ERR_RESTORE();

     return is_level_1c;
}
