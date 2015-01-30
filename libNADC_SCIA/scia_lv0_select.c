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
.PURPOSE     obtain indices to state-records of selected MDS records
.INPUT/OUTPUT
  call as   nr_mds = SCIA_LV0_SELECT_MDS( source, param, nr_states, states, 
                                          &states_out );
     input: 
            int source                 : data source (Detector, Auxiliary, PMD)
	    struct param_record param  : struct holding user-defined settings
	    size_t nr_states           : number of state-records
	    struct mds0_states *states : structure for state-records
    output:  
	    struct mds0_states **states_out : array with selected states-records

.RETURNS     number of selected records (size_t)
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
.IDENTifer   SCIA_LV0_SELECT_MDS_UNIQ
.PURPOSE     reject duplicated states
.INPUT/OUTPUT
  call as   nr_indx = SCIA_LV0_SELECT_MDS_UNIQ( states, nr_indx, indx );
     input:  
	    struct mds0_states *states : structure for mds0_states
	    size_t nr_indx             : (at first call) number of states
    output:  
            size_t *indx               : (at first call) array of size nr_indx
                                         initialized as [0,1,2,...,nr_indx-1]

.RETURNS     number of selected states
.COMMENTS    static function
-------------------------*/
static
size_t SCIA_LV0_SELECT_MDS_UNIQ( const struct mds0_states *states, 
				       size_t nr_indx, size_t *indx )
       /*@modifies indx@*/
{
     const char prognm[] = "SCIA_LV0_SELECT_MDS_UNIQ";

     char msg[64];

     register size_t ni, nj;
     register size_t nr = 0u;

     register const struct mds0_states *states_ptr;

     /* check if on_board_time is strictly increasing */
     for ( ni = 1; ni < nr_indx; ni++ ) {
	  if ( states[indx[ni-1]].on_board_time
	       > states[indx[ni]].on_board_time ) 
	       break;
     }
     if ( ni == nr_indx ) return nr_indx;

     /* identify duplicated records */
     for ( ni = 0; ni < nr_indx; ni++ ) {
	  if ( indx[ni] == UINT_MAX ) continue;

	  states_ptr = states + indx[ni];
	  for ( nj = ni+1;  nj < nr_indx; nj++ ) {
	       if ( indx[nj] == UINT_MAX ) continue;

	       if ( states_ptr->on_board_time == states[indx[nj]].on_board_time
		    && states_ptr->num_aux == states[indx[nj]].num_aux
		    && states_ptr->num_det == states[indx[nj]].num_det
		    && states_ptr->num_pmd == states[indx[nj]].num_pmd )
		    indx[nj] = UINT_MAX;
	  }
     }
     /* remove indices to duplicated records */
     for ( ni = 0; ni < nr_indx; ni++ ) {
	  if ( indx[ni] != UINT_MAX ) indx[nr++] = indx[ni];
     }
     if ( nr_indx == nr ) return nr_indx;

     /* compose message to user */
     (void) snprintf( msg, 64, 
		      "rejected %-zd duplicated states", nr_indx - nr );
     NADC_ERROR( prognm, NADC_ERR_NONE, msg );
     return nr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_SELECT_MDS_PERIOD
.PURPOSE     select states on given time window
.INPUT/OUTPUT
  call as   nr_indx = SCIA_LV0_SELECT_MDS_PERIOD( bgn_date, end_date, states, 
                                                  nr_indx, indx );
     input:  
            char *bgn_date             : start of time window
            char *end_date             : end of time window
	    struct mds0_states *states : structure for state-records
	    size_t nr_indx             : (at first call) number of state-records
    output:  
            size_t *indx               : (at first call) array of size nr_indx
	                                 initialized as [0,1,2,...,nr_indx-1]

.RETURNS     number of selected state-records
.COMMENTS    static function
-------------------------*/
static
size_t SCIA_LV0_SELECT_MDS_PERIOD( const char *bgn_date, const char *end_date,
				   const struct mds0_states *states, 
				   size_t nr_indx, size_t *indx )
       /*@globals  errno;@*/
       /*@modifies errno, indx@*/
{
     register size_t ni;
     register size_t nr = 0u;

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
	  register double mjd_date = states[indx[ni]].mjd.days 
	       + (states[indx[ni]].mjd.secnd 
		  + states[indx[ni]].mjd.musec / 1000000.) / SecPerDay;

	  if ( mjd_date >= bgn_jdate && mjd_date <= end_jdate )
	       indx[nr++] = indx[ni];
     }
     return nr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_SELECT_MDS_STATE
.PURPOSE     select state-records on state ID
.INPUT/OUTPUT
  call as   nr_indx = SCIA_LV0_SELECT_MDS_STATE( stateID_nr, stateID, states, 
                                                 nr_indx, indx );
     input:  
            unsigned char stateID_nr   : number of selected stateIDs
	    unsigned char *stateID     : array with stateIDs
	    struct mds0_states *states : structure for state-records
	    size_t nr_indx             : (at first call) number of state-records
    output:  
            size_t *indx               : (at first call) array of size nr_indx
                                         initialized as [0,1,2,...,nr_indx-1]

.RETURNS     number of selected state-records
.COMMENTS    static function
-------------------------*/
static
size_t SCIA_LV0_SELECT_MDS_STATE( unsigned char stateID_nr,
				  const unsigned char *stateID,
				  const struct mds0_states *states, 
				  size_t nr_indx, size_t *indx )
       /*@modifies indx@*/
{
     register size_t ni;
     register size_t nr = 0u;

     for ( ni = 0; ni < nr_indx; ni++ ) {
	  register short ns = 0;

	  register bool found = FALSE;
	  do {
	       if ( stateID[ns] == states[indx[ni]].state_id )
		    found = TRUE;
	  } while( ! found && ++ns < (short) stateID_nr );
	  if ( found ) indx[nr++] = indx[ni];

     }
     return nr;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
size_t SCIA_LV0_SELECT_MDS( const struct param_record param,
			    size_t num_states, const struct mds0_states *states,
			    struct mds0_states **states_out )
{
     const char prognm[] = "SCIA_LV0_SELECT_MDS";

     register size_t ni;
     register size_t nr_indx = 0;

     size_t *indx_states = NULL;
/*
 * initialize output array
 */
     *states_out = NULL;
     if ( num_states == 0 ) return 0;
/*
 * first check type of selected MDS
 */
     if ( param.write_aux == PARAM_UNSET
	  && param.write_det == PARAM_UNSET
	  && param.write_pmd == PARAM_UNSET )
	  return 0u;
/*
 * allocate memory to store indices to selected MDS records
 */
     (void) fprintf( stderr, "num_states(1): %zd\n", num_states );
     indx_states = (size_t *) malloc( num_states * sizeof(size_t) );
     if ( indx_states == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "indx_states" );
     for ( ni = 0; ni < num_states; ni++ ) indx_states[ni] = ni;
/*
 * apply selection criteria
 */
     nr_indx = SCIA_LV0_SELECT_MDS_UNIQ( states, num_states, indx_states );
     (void) fprintf( stderr, "num_states(2): %zd\n", nr_indx );

     if ( param.flag_period != PARAM_UNSET ) {
	  nr_indx = SCIA_LV0_SELECT_MDS_PERIOD( param.bgn_date, param.end_date,
						states, nr_indx, indx_states );
     }
     if ( param.stateID_nr != PARAM_UNSET ) {
	  nr_indx = SCIA_LV0_SELECT_MDS_STATE( param.stateID_nr, param.stateID,
					       states, nr_indx, indx_states );
     }
     (void) fprintf( stderr, "num_states(3): %zd\n", nr_indx );
     if ( nr_indx == 0 ) goto done;
/*
 * copy selected state-records to output array
 */
     *states_out = (struct mds0_states *) 
	  malloc( nr_indx * sizeof( struct mds0_states ));
     if ( *states_out == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mds0_states" );

     for ( ni = 0; ni < nr_indx; ni++ ) {
	  (void) memcpy( &(*states_out)[ni].mjd, &states[indx_states[ni]].mjd,
			 sizeof(struct mjd_envi) );
	  (*states_out)[ni].category = states[indx_states[ni]].category;
	  (*states_out)[ni].state_id = states[indx_states[ni]].state_id;
	  (*states_out)[ni].q.value = states[indx_states[ni]].q.value ;
	  (*states_out)[ni].num_aux = states[indx_states[ni]].num_aux;
	  (*states_out)[ni].num_det = states[indx_states[ni]].num_det;
	  (*states_out)[ni].num_pmd = states[indx_states[ni]].num_pmd;
	  (*states_out)[ni].on_board_time =
	       states[indx_states[ni]].on_board_time;
	  (*states_out)[ni].offset = states[indx_states[ni]].offset;

	  if ( param.write_aux == PARAM_UNSET ) {
	       (*states_out)[ni].num_aux = 0;
	       (*states_out)[ni].info_aux = NULL;
	  } else if ( (*states_out)[ni].num_aux > 0 ) {
	       size_t num_aux = (*states_out)[ni].num_aux;
	       
	       (*states_out)[ni].info_aux = (struct mds0_info *)
		    malloc( num_aux * sizeof(struct mds0_info));
	       (void) memcpy( &(*states_out)[ni].info_aux,
			      &states[indx_states[ni]].info_aux,
			      num_aux * sizeof(struct mds0_info) );
	  }
	  if ( param.write_det == PARAM_UNSET ) {
	       (*states_out)[ni].num_det = 0;
	       (*states_out)[ni].info_det = NULL;
	  } else if ( (*states_out)[ni].num_det > 0 ) {
	       size_t num_det = (*states_out)[ni].num_det;
	       
	       (*states_out)[ni].info_det = (struct mds0_info *)
		    malloc( num_det * sizeof(struct mds0_info));
	       (void) memcpy( &(*states_out)[ni].info_det,
			      &states[indx_states[ni]].info_det,
			      num_det * sizeof(struct mds0_info) );
	  }
	  if ( param.write_pmd == PARAM_UNSET ) {
	       (*states_out)[ni].num_pmd = 0;
	       (*states_out)[ni].info_pmd = NULL;
	  } else if ( (*states_out)[ni].num_pmd > 0 ) {
	       size_t num_pmd = (*states_out)[ni].num_pmd;
	       
	       (*states_out)[ni].info_pmd = (struct mds0_info *)
		    malloc( num_pmd * sizeof(struct mds0_info));
	       (void) memcpy( &(*states_out)[ni].info_pmd,
			      &states[indx_states[ni]].info_pmd,
			      num_pmd * sizeof(struct mds0_info) );
	  }
     }
done:
     (void) fprintf( stderr, "num_states(4): %zd\n", nr_indx );
     if ( indx_states != NULL ) free( indx_states );
     return nr_indx;
}
