/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 1999 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_INIT_PARAM
.AUTHOR      R.M. van Hees
.KEYWORDS    command-line parameter handling
.LANGUAGE    ANSI C
.PURPOSE     initializes param-structure with command-line parameters
.CONTAINS    NADC_INIT_PARAM
.RETURNS     Nothing (check global error status)
.COMMENTS    None
.ENVIRONment None
.VERSION      6.1.1 09-Jan-2013	added SDMF_SELECT_NRT, RvH
              6.1   20-May-2012	added SCIA_TRANS_RANGE, RvH
              6.0   10-Apr-2012	separated modules NADC_INIT_PARAM
                                and NADC_SET_PARAM, RvH
              5.9   20-Jun-2008	removed HDF4 support, RvH
              5.8   23-Nov-2006	added options for SDMF software, RvH
              5.7   23-Oct-2006	bugfix hdf4/5 request without HDF support, RvH
              5.6   17-Nov-2005	bugfix option no_pmd_geo and -meta (gome_lv1)
              5.5   11-Oct-2005	added options -pds and -pds_1b for SCIA L1, RvH
              5.4   14-Jul-2005	added option -meta for all processors, RvH
              5.3   23-Dec-2003	debugged MDS selection, RvH
              5.2   25-Aug-2003	added instrument equals SCIA_PATCH_1, RvH
              5.1   10-Feb-2003	bugfixes and added more parameters, RvH
              5.0   06-Feb-2003	changes to optional parameters (!), 
                                major rewrite of the code with 
                                improved parameter checking, RvH
              4.1   23-Jan-2003	more robuust against invalid input, RvH 
              4.0   12-Dec-2002	added option to calculate reflectance, RvH
              3.9   10-Nov-2002	implemented new Scia calib options, RvH 
              3.8   04-Oct-2002	new options for scia_ol2, RvH 
              3.7   18-Sep-2002	added geolocation selection to scia_nl1, RvH 
              3.6   10-Sep-2002	modified/added options to SCIA-progs, RvH
              3.5   04-Sep-2002	added option for alternative calibration 
                                algorithms (--altcal), RvH
              3.4   02-Aug-2002	added calibration option "B" 
                                (SCIA level 1), RvH
              3.3   02-Aug-2002	added selection on MDS (SCIA level 2), RvH 
              3.2   02-Aug-2002	added option "--info", RvH 
              3.1   02-Aug-2002	added category selection (SCIA level 1), RvH 
              3.0   02-Aug-2002	added clusters selection (SCIA level 1), RvH 
              2.9   17-Jun-2002	added calib options for SCIA data, RvH 
              2.8   21-Dec-2001	added more options for scia_lv0, RvH 
              2.7   21-Dec-2001	Added parameters for scia_lv0, RvH 
              2.6   28-Oct-2001	bugs in geolocation, RvH 
              2.5   25-Oct-2001 rename modules to NADC_..., RvH
              2.4   20-Sep-2001 changed options for output generation, RvH
              2.3   05-Sep-2001 compiles without HDF4/5 library, RvH
              2.2   07-Aug-2001 sequence of reading PMD options is important
              2.1   26-Jul-2001 improved help output, RvH
              2.0   01-May-2001 new outfile naming scheme, RvH
              1.5   21-Apr-2001 added HDF-4 options, RvH
              1.4   16-Nov-2000 improved help displayed with GIVE_HELP, RvH
              1.3   09-Nov-2000 set default to NO calibration, RvH
              1.2   11-Feb-2000 changed calibration selection conform 
                                  the DLR Extractor, RvH
              1.1   08-Feb-2000 handles various date-strings correctly, RvH
              1.0   20-May-1999 Created by R. M. van Hees
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
#include <ctype.h>
#include <math.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++ Static Variables +++++*/

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   NADC_INIT_PARAM
.PURPOSE     initializes param-structure
.INPUT/OUTPUT
  call as   NADC_INIT_PARAM( param );
	     struct param_record 
	            *param   :     struct holding user-defined settings
.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void NADC_INIT_PARAM( struct param_record *param )
{
     param->flag_infile  = PARAM_UNSET;
     param->flag_outfile = PARAM_UNSET;

     param->flag_check   = PARAM_UNSET;
     param->flag_show    = PARAM_UNSET;
     param->flag_version = PARAM_UNSET;
     param->flag_silent  = PARAM_UNSET;
     param->flag_verbose = PARAM_UNSET;

     param->flag_cloud   = PARAM_UNSET;
     param->flag_geoloc  = PARAM_UNSET;
     param->flag_geomnmx = PARAM_SET;
     param->flag_period  = PARAM_UNSET;
     param->flag_pselect = PARAM_UNSET;
     param->flag_subset  = PARAM_UNSET;
     param->flag_sunz    = PARAM_UNSET;
     param->flag_wave    = PARAM_UNSET;

     param->qcheck       = PARAM_SET;

     param->write_pds    = PARAM_UNSET;
     param->write_ascii  = PARAM_UNSET;
     param->write_meta   = PARAM_UNSET;
     param->write_hdf5   = PARAM_UNSET;
     param->flag_deflate = PARAM_UNSET;

     param->write_sql    = PARAM_UNSET;
     param->flag_sql_remove  = PARAM_UNSET;
     param->flag_sql_replace = PARAM_UNSET;

     param->write_lv1c    = PARAM_UNSET;

     param->write_ads     = PARAM_SET;
     param->write_gads    = PARAM_SET;

     param->write_subset  = 0x0U;  /* default: write all ground pixels */
     param->write_blind   = PARAM_UNSET;     /* default: no blind pixel data */
     param->write_stray   = PARAM_UNSET;      /* default: no straylight data */
     param->write_aux0    = PARAM_SET;
     param->write_pmd0    = PARAM_SET;
     param->write_aux     = PARAM_SET;
     param->write_det     = PARAM_SET;
     param->write_pmd     = PARAM_SET;
     param->write_pmd_geo = PARAM_SET;
     param->write_polV    = PARAM_SET;

     param->write_limb    = PARAM_SET;
     param->write_moni    = PARAM_SET;
     param->write_moon    = PARAM_SET;
     param->write_nadir   = PARAM_SET;
     param->write_occ     = PARAM_SET;
     param->write_sun     = PARAM_SET;

     param->write_bias    = PARAM_SET;
     param->write_cld     = PARAM_SET;
     param->write_doas    = PARAM_SET;

     param->catID_nr      = PARAM_UNSET;    /* default: write all categories */
     param->stateID_nr    = PARAM_UNSET;        /* default: write all states */
     param->clusID_nr     = PARAM_UNSET;      /* default: write all clusters */

     param->chan_mask    = (unsigned char) ~0U;

     param->patch_scia   = 0x0U;
     param->calib_earth  = 0x0U;
     param->calib_limb   = 0x0U;
     param->calib_moon   = 0x0U;
     param->calib_sun    = 0x0U;
     param->calib_pmd    = 0x0U;
     param->calib_scia   = 0x0U;

     param->hdf_file_id = -1;

     param->clus_mask  = ~0ULL;             /* default: extract all clusters */

     (void) memset( param->catID, 0u, MAX_NUM_STATE );
     (void) memset( param->stateID, 0u, MAX_NUM_STATE );

     (void) memset( param->bgn_date, 0u, DATE_STRING_LENGTH );
     (void) memset( param->end_date, 0u, DATE_STRING_LENGTH );

     (void) memset( param->pselect, 0u, MAX_STRING_LENGTH );
     (void) strcpy( param->pselect, "ALL" );

     (void) memset( param->program  , 0u, MAX_STRING_LENGTH );
     (void) memset( param->infile   , 0u, MAX_STRING_LENGTH );
     (void) memset( param->outfile  , 0u, MAX_STRING_LENGTH );
     (void) memset( param->hdf5_name, 0u, MAX_STRING_LENGTH );

     param->cloud[0] = 0.f;
     param->cloud[1] = 1.f;
     param->geo_lat[0] =  -90.f;
     param->geo_lat[1] =   90.f;
     param->geo_lon[0] = -180.f;
     param->geo_lon[1] =  180.f;
     param->sunz[0] =  -90.f;
     param->sunz[1] =   90.f;
     param->wave[0] =    0.f;
     param->wave[1] = 5000.f;
}
