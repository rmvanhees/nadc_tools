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

.IDENTifer   SCIA_LV0_SELECT
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0 data selection
.LANGUAGE    ANSI C
.PURPOSE     obtain indices to info-records of selected MDS records
.INPUT/OUTPUT
  call as   nr_mds = SCIA_LV0_SELECT_MDS( source, param, nr_info, info, 
                                          &info_out );
     input: 
            int source                 : data source (Detector, Auxiliary, PMD)
	    struct param_record param  : struct holding user-defined settings
	    unsigned int nr_info       : number of info-records
	    struct mds0_info *info     : structure for info-records
    output:  
	    struct mds0_info **info_out: array with selected info-records

.RETURNS     number of selected records (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION      3.1   09-Dec-2009	add unique filter, RvH
              3.0   11-Oct-2005	rewrite to one selection per module, RvH
              2.5   13-Oct-2003	changes to due modification of mds0_info, RvH
              2.4   19-Feb-2002	returns array with selected info-records, RvH
              2.3   01-Feb-2002	added selection in detector channel(s), RvH 
              2.2   21-Dec-2001	implemented selection on states (scia_lv0), RvH
              2.1   12-Dec-2001	updated documentation, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.0   01-Nov-2001	Created by R. M. van Hees 
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include "selected_channel.inc"

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_SELECT_MDS_SOURCE
.PURPOSE     select info-records on data source
.INPUT/OUTPUT
  call as   nr_indx = SCIA_LV0_SELECT_MDS_SOURCE( source, info, 
                                                  nr_indx, indx_info );
     input:  
            int source              : data source (Detector, Auxiliary, PMD)
	    struct mds0_info *info  : structure for info-records
	    unsigned int nr_indx    : (at first call) number of info-records
    output:  
            unsigned int *indx_info : (at first call) array of size nr_indx
                                      initialized as [0,1,2,...,nr_indx-1]

.RETURNS     number of selected info-records
.COMMENTS    static function
-------------------------*/
static
unsigned int SCIA_LV0_SELECT_MDS_SOURCE( int source, 
					 const struct mds0_info *info, 
					 unsigned int nr_indx, 
					 unsigned int *indx_info )
       /*@modifies indx_info@*/
{
     register unsigned int ni;
     register unsigned int nr = 0u;

     for ( ni = 0; ni < nr_indx; ni++ ) {
	  if ( source == (int) info[indx_info[ni]].packet_type ) 
	       indx_info[nr++] = indx_info[ni];
     }
     return nr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_SELECT_MDS_UNIQ
.PURPOSE     reject duplicated info-records
.INPUT/OUTPUT
  call as   nr_indx = SCIA_LV0_SELECT_MDS_UNIQ( info, nr_indx, indx_info );
     input:  
	    struct mds0_info *info  : structure for info-records
	    unsigned int nr_indx    : (at first call) number of info-records
    output:  
            unsigned int *indx_info : (at first call) array of size nr_indx
                                      initialized as [0,1,2,...,nr_indx-1]

.RETURNS     number of selected info-records
.COMMENTS    static function
-------------------------*/
#define CMP_MJD(a,b) memcmp( &(a), &(b), sizeof(struct mjd_envi))

static
unsigned int SCIA_LV0_SELECT_MDS_UNIQ( const struct mds0_info *info, 
				       unsigned int nr_indx, 
				       unsigned int *indx_info )
       /*@modifies indx_info@*/
{
     const char prognm[] = "SCIA_LV0_SELECT_MDS_UNIQ";

     char msg[64];

     register unsigned int ni, nj;
     register unsigned int nr = 0u;

     register const struct mds0_info *info_ptr;

     /* check if state_index is strictly increasing (duplicates are allowed) */
     for ( ni = 1; ni < nr_indx; ni++ ) {
	  if (info[indx_info[ni-1]].state_index > info[indx_info[ni]].state_index) 
	       break;
     }
     if ( ni == nr_indx ) return nr_indx;

     /* identify duplicated records */
     for ( ni = 0; ni < nr_indx; ni++ ) {
	  if ( indx_info[ni] == UINT_MAX ) continue;

	  info_ptr = info + indx_info[ni];
	  for ( nj = ni+1;  nj < nr_indx; nj++ ) {
	       if ( indx_info[nj] == UINT_MAX ) continue;

	       if ( info_ptr->packet_type == info[indx_info[nj]].packet_type
/* 		    && CMP_MJD(info_ptr->mjd, info[indx_info[nj]].mjd) == 0 */
		    && info_ptr->state_index == info[indx_info[nj]].state_index
		    && info_ptr->bcps == info[indx_info[nj]].bcps )
		    indx_info[nj] = UINT_MAX;
	  }
     }
     /* remove indices to duplicated records */
     for ( ni = 0; ni < nr_indx; ni++ ) {
	  if ( indx_info[ni] != UINT_MAX ) indx_info[nr++] = indx_info[ni];
     }
     if ( nr_indx == nr ) return nr_indx;

     /* compose message to user */
     if ( (int) info[indx_info[0]].packet_type == SCIA_AUX_PACKET ) {
	  (void) snprintf( msg, 64, 
			   "rejected %-u duplicated Auxiliary packages", 
			   nr_indx - nr );
     } else if ( (int) info[indx_info[0]].packet_type == SCIA_DET_PACKET ) {
	  (void) snprintf( msg, 64, 
			   "rejected %-u duplicated Detector packages", 
			   nr_indx - nr );
     } else {
	  (void) snprintf( msg, 64, "rejected %-u duplicated PMD packages", 
			   nr_indx - nr );
     }
     NADC_ERROR( prognm, NADC_ERR_NONE, msg );
     return nr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_SELECT_MDS_PERIOD
.PURPOSE     select info-records on given time window
.INPUT/OUTPUT
  call as   nr_indx = SCIA_LV0_SELECT_MDS_PERIOD( bgn_date, end_date, info, 
                                                  nr_indx, indx_info );
     input:  
            char *bgn_date          : start of time window
            char *end_date          : end of time window
	    struct mds0_info *info  : structure for info-records
	    unsigned int nr_indx    : (at first call) number of info-records
    output:  
            unsigned int *indx_info : (at first call) array of size nr_indx
                                      initialized as [0,1,2,...,nr_indx-1]

.RETURNS     number of selected info-records
.COMMENTS    static function
-------------------------*/
static
unsigned int SCIA_LV0_SELECT_MDS_PERIOD( const char *bgn_date,
					 const char *end_date,
					 const struct mds0_info *info, 
					 unsigned int nr_indx, 
					 unsigned int *indx_info )
       /*@globals  errno;@*/
       /*@modifies errno, indx_info@*/
{
     register unsigned int ni;
     register unsigned int nr = 0u;

     int          mjd2000;
     unsigned int secnd, mu_sec;
     double       bgn_jdate = 0.;
     double       end_jdate = 0.;

     const double SecPerDay = 24. * 60. * 60.;
/*
 * initialize begin and end julian date of time window
 */
     ASCII_2_MJD( bgn_date, &mjd2000, &secnd, &mu_sec );
     bgn_jdate = mjd2000 + (secnd + mu_sec / 1000000.) / SecPerDay;
     ASCII_2_MJD( end_date, &mjd2000, &secnd, &mu_sec );
     end_jdate = mjd2000 + (secnd + mu_sec / 1000000.) / SecPerDay;

     for ( ni = 0; ni < nr_indx; ni++ ) {
	  register double mjd_date = info[indx_info[ni]].mjd.days 
	       + (info[indx_info[ni]].mjd.secnd 
		  + info[indx_info[ni]].mjd.musec / 1000000.) / SecPerDay;

	  if ( mjd_date >= bgn_jdate && mjd_date <= end_jdate )
	       indx_info[nr++] = indx_info[ni];
     }
     return nr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_SELECT_MDS_STATE
.PURPOSE     select info-records on state ID
.INPUT/OUTPUT
  call as   nr_indx = SCIA_LV0_SELECT_MDS_STATE( stateID_nr, stateID, info, 
                                                 nr_indx, indx_info );
     input:  
            unsigned char stateID_nr: number of selected stateIDs
	    unsigned char *stateID  : array with stateIDs
	    struct mds0_info *info  : structure for info-records
	    unsigned int nr_indx    : (at first call) number of info-records
    output:  
            unsigned int *indx_info : (at first call) array of size nr_indx
                                      initialized as [0,1,2,...,nr_indx-1]

.RETURNS     number of selected info-records
.COMMENTS    static function
-------------------------*/
static
unsigned int SCIA_LV0_SELECT_MDS_STATE( unsigned char stateID_nr,
					const unsigned char *stateID,
					const struct mds0_info *info, 
					unsigned int nr_indx, 
					unsigned int *indx_info )
       /*@modifies indx_info@*/
{
     register unsigned int ni;
     register unsigned int nr = 0u;

     for ( ni = 0; ni < nr_indx; ni++ ) {
	  register short ns = 0;

	  register bool found = FALSE;
	  do {
	       if ( stateID[ns] == info[indx_info[ni]].state_id )
		    found = TRUE;
	  } while( ! found && ++ns < (short) stateID_nr );
	  if ( found ) indx_info[nr++] = indx_info[ni];

     }
     return nr;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int SCIA_LV0_SELECT_MDS( int source, struct param_record param,
				  unsigned int nr_info, 
				  const struct mds0_info *info,
				  struct mds0_info **info_out )
{
     const char prognm[] = "SCIA_LV0_SELECT_MDS";

     register unsigned int ni;
     register unsigned int nr_indx = 0;

     unsigned int *indx_info = NULL;
/*
 * initialize output array
 */
     *info_out = NULL;
     if ( nr_info == 0u ) return 0u;
/*
 * first check type of selected MDS
 */
     if ( source == SCIA_AUX_PACKET && param.write_aux == PARAM_UNSET )
	  return 0u;
     if ( source == SCIA_DET_PACKET && param.write_det == PARAM_UNSET )
	  return 0u;
     if ( source == SCIA_PMD_PACKET && param.write_pmd == PARAM_UNSET )
	  return 0u;
/*
 * allocate memory to store indices to selected MDS records
 */
     indx_info = (unsigned int *) malloc( nr_info * sizeof( unsigned int ));
     if ( indx_info == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "indx_info" );
     for ( ni = 0; ni < nr_info; ni++ ) indx_info[ni] = ni;
/*
 * apply selection criteria
 */
     nr_indx = SCIA_LV0_SELECT_MDS_SOURCE( source, info, nr_info, indx_info );
     nr_indx = SCIA_LV0_SELECT_MDS_UNIQ( info, nr_indx, indx_info );

     if ( param.flag_period != PARAM_UNSET ) {
	  nr_indx = SCIA_LV0_SELECT_MDS_PERIOD( param.bgn_date, param.end_date,
						info, nr_indx, indx_info );
     }
     if ( param.stateID_nr != PARAM_UNSET ) {
	  nr_indx = SCIA_LV0_SELECT_MDS_STATE( param.stateID_nr, param.stateID,
					       info, nr_indx, indx_info );
     }
     if ( nr_indx == 0 ) goto done;
/*
 * copy selected info-records to output array
 */
     *info_out = (struct mds0_info *) 
	  malloc( (size_t) nr_indx * sizeof( struct mds0_info ));
     if ( *info_out == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mds0_info" );

     for ( ni = 0; ni < nr_indx; ni++ ) {
	  (void) memcpy( &(*info_out)[ni], &info[indx_info[ni]],
			 sizeof( struct mds0_info ) );
     }
done:
     if ( indx_info != NULL ) free( indx_info );
     return nr_indx;
}
