/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2015 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV0_MDS_INFO
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0 data
.LANGUAGE    ANSI C
.PURPOSE     obtain inventarisation of Sciamachy LV0 product
.INPUT/OUTPUT
  call as   num_state = SCIA_LV0_RD_MDS_INFO( fd, num_dsd, dsd, states );
     input:
             FILE *fd                    : (open) stream pointer to scia-file
             unsigned int num_dsd        : number of DSD records
	     struct dsd_envi *dsd        : structure for the DSD records
    output:
             struct mds0_states **states : info on states in product

.RETURNS     number of states records found (size_t)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
.ENVIRONment None
.VERSION      3.0   19-Dec-2013	new implementation, complete rewrite, RvH
              2.5.1 20-Apr-2006	minor bug-fix to compile without HDF5, RvH
	      2.5   16-Jan-2005	update documentation, RvH
              2.4   10-Jan-2005	store/read info-records in database, RvH
              2.3   01-Apr-2003	modified function parameters, RvH
              2.2   20-Dec-2001	renamed module, RvH 
              2.1   12-Dec-2001	updated documentation, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.0   01-Nov-2001	Created by R. M. van Hees 
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
#include <limits.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*++++++++++ function to display info-record information ++++++++++*/
static
void _SHOW_INFO_RECORDS( unsigned int num_info, const struct mds0_info *info )
{
     unsigned int ni = 0;

     /* handle special cases gracefully */
     if ( num_info == 0 ) return;

     do {
	  (void) fprintf( stdout,
			  "%9u %02hhu %02hhu %10u %4hu %5hu %5hu %3hhu %4hu %4hu\n",
			  info->offset, info->packet_id, info->state_id,
			  info->on_board_time, info->bcps,
			  info->isp_length, info->packet_length,
			  info->q.value,
			  info->crc_errors, info->rs_errors );
     } while ( info++, ++ni < num_info );
}

/*++++++++++ function to display states-record information ++++++++++*/
static
void _SHOW_STATE_RECORDS( size_t num_state, const struct mds0_states *states )
{
     size_t ns = 0;

     /* handle special cases gracefully */
     if ( num_state == 0 ) return;

     do {
	  (void) fprintf( stdout,
			  "%9u %02hhu %10u %5u %03hhu %5u %03hhu %5u %03hhu\n",
			  states->offset, states->state_id, 
			  states->on_board_time,
			  states->num_aux, states->q_aux.value,
			  states->num_det, states->q_det.value,
			  states->num_pmd, states->q_pmd.value );
     } while ( states++, ++ns < num_state );
}

/*+++++++++++++++++++++++++
.IDENTifer   _ASSIGN_INFO_STATES
.PURPOSE     identify DSR packages of the same state
.INPUT/OUTPUT
  call as   num_states = _ASSIGN_INFO_STATES( correct_info_rec, num_info, info,
			                      &states )
     input:  
            bool correct_info_rec   : allow correction of corrupted data?
            unsigned int num_info   : number of info records
	    struct mds0_info *info  : structure holding info about MDS records
    output:  
            struct mds0_states **states : info-records organized per state

.RETURNS     number of states in product (size_t)
             error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static inline
void _FILL_STATES( unsigned int num_info, struct mds0_info *info,
		   struct mds0_states *states )
{
     register unsigned int ni = 0;
     
     unsigned int na = 0;
     unsigned int nd = 0;
     unsigned int np = 0;

     (void) memcpy( &states->mjd, &info->mjd, sizeof(struct mjd_envi) );
     states->category = info->category;
     states->state_id = info->state_id;
     states->q_aux.value = 0;
     states->q_det.value = 0;
     states->q_pmd.value = 0;
     states->on_board_time = info->on_board_time;
     states->offset = info->offset;
     if ( states->num_aux > 0 ) {
	  states->info_aux = (struct mds0_info *)
	       malloc( states->num_aux * sizeof(struct mds0_info) );
     }
     if ( states->num_det > 0 ) {
	  states->info_det = (struct mds0_info *)
	       malloc( states->num_det * sizeof(struct mds0_info) );
     }
     if ( states->num_pmd > 0 ) {
	  states->info_pmd = (struct mds0_info *)
	       malloc( states->num_pmd * sizeof(struct mds0_info) );
     }
     do {
	  switch ( (int) info[ni].packet_id ) {
	  case SCIA_DET_PACKET: 
	       (void) memcpy( states->info_det+nd, info+ni, 
			      sizeof(struct mds0_info) );
	       nd++;
	       break;
	  case SCIA_AUX_PACKET:
	       (void) memcpy( states->info_aux+na, info+ni, 
			      sizeof(struct mds0_info) );
	       na++;
	       break;
	  case SCIA_PMD_PACKET:
	       (void) memcpy( states->info_pmd+np, info+ni, 
			      sizeof(struct mds0_info) );
	       np++;
	       break;
	  }
     } while ( ++ni < num_info );
}

static
size_t _ASSIGN_INFO_STATES( unsigned int num_info, struct mds0_info *info,
			    /*@out@*/ struct mds0_states **states_out )
     /*@modifies nadc_stat, nadc_err_stack, *states; @*/
     /*@globals  nadc_stat, nadc_err_stack@*/
{
     const char prognm[] = "_ASSIGN_INFO_STATES";

     register unsigned int ni  = 0;

     register size_t ns  = 0;
     register size_t num = 0;

     size_t num_state = 0;
     unsigned int on_board_time = info->on_board_time;
     struct mjd_envi mjd;
     struct mds0_states *states = NULL;
     
     /* handle special cases gracefully */
     if ( num_info == 0u ) goto done;

     /* find number of states in product */
     for ( ni = 0; ni < num_info; ni++ ) {
	  if ( info[ni].on_board_time > 0
	       && on_board_time != info[ni].on_board_time ) {
	       on_board_time = info[ni].on_board_time;
	       num++;
	  }
     }
     num++;

     /* handle special cases gracefully */
     if ( num == 0 ) return 0;
     num_state = num;

     /* allocate output array */
     states = (struct mds0_states *) malloc( num * sizeof(struct mds0_states) );
     if ( (states_out[0] = states) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mds0_states" );
     
     /* fill output array */
     ni = 0u;
     num = 0;
     on_board_time = info->on_board_time;
     (void) memcpy(&mjd, &info->mjd, sizeof(struct mjd_envi));
     states->num_aux = 0;
     states->num_det = 0;
     states->num_pmd = 0;

     do {
	  if ( on_board_time != info[ni].on_board_time ) {
	       if ( num > 3 ) {
		    _FILL_STATES( num, &info[ni-num], states+ns );
		    ns++;
	       }
	       num = 0;
	       on_board_time = info[ni].on_board_time;
	       (void) memcpy(&mjd, &info[ni].mjd, sizeof(struct mjd_envi));
	       states[ns].num_aux = 0;
	       states[ns].num_det = 0;
	       states[ns].num_pmd = 0;
	  }
	  switch ( (int) info[ni].packet_id ) {
	  case ( SCIA_DET_PACKET ):
	       states[ns].num_det++;
	       break;
	  case ( SCIA_AUX_PACKET ):
	       states[ns].num_aux++;
	       break;
	  case ( SCIA_PMD_PACKET ):
	       states[ns].num_pmd++;
	       break;
	  default:
	       (void) fprintf( stdout, "no valid packet ID[%u]: %hhu\n",
			       ni, info[ni].packet_id );
	  }
	  num++;
     } while ( ++ni < num_info );
     
     if ( num > 0 ) {
	  _FILL_STATES( num, &info[num_info-num], &states[ns] );
	  ns++;
     }
     num_state = ns;
done:
     return num_state;
}

/*+++++++++++++++++++++++++
.IDENTifer   _CHECK_INFO_PACKET_ID
.PURPOSE     consistency check of value of packet_id in info-records
.INPUT/OUTPUT
  call as   _CHECK_INFO_PACKET_ID( correct_info_rec, num_info, info );
     input:  
            bool correct_info_rec    :  allow correction of corrupted data?
            unsigned int num_info    :  number of info-records
 in/output:  
	    struct mds0_info *info   :  info records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void _CHECK_INFO_PACKET_ID( bool correct_info_rec,
			   unsigned int num_info, struct mds0_info *info )
     /*@modifies info->q.value, info->packet_id; @*/
{
     //const char prognm[] = "_CHECK_INFO_PACKET_ID";

     register unsigned int ni = 0;

     /* handle special cases gracefully */
     if ( num_info < 2 ) return;

     if ( ! correct_info_rec ) return;
     
     do { 
	  if ( info->packet_id > (unsigned char) 3 ) {
	       info->packet_id &= (unsigned char) 3;
	       info->q.flag.packet_id = 1;
	  }
	  if ( info->packet_id == (unsigned char) 0 ) {
	       if ( info->packet_length == 1659 ) {
		    info->packet_id = SCIA_AUX_PACKET;
	       } else if ( info->packet_length == 6813 ) {
		    info->packet_id = SCIA_PMD_PACKET;
	       } else if ( info->packet_length > 0 ) {
		    info->packet_id = SCIA_DET_PACKET;
	       }
	       info->q.flag.packet_id = 1;
	  }
     } while ( info++, ++ni < num_info );
}

/*+++++++++++++++++++++++++
.IDENTifer   _CHECK_INFO_ON_BOARD_TIME
.PURPOSE     consistency check of value of on_board_time in info-records
.INPUT/OUTPUT
  call as   _CHECK_INFO_ON_BOARD_TIME( correct_info_rec, num_info, info );
     input:  
            bool correct_info_rec    :  allow correction of corrupted data?
            unsigned int num_info    :  number of info-records
 in/output:  
	    struct mds0_info *info   :  info records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void _CHECK_INFO_ON_BOARD_TIME( bool correct_info_rec,
				unsigned int num_info, struct mds0_info *info )
     /*@modifies info->q.value, info->on_board_time; @*/
{
     //const char prognm[] = "_CHECK_INFO_ON_BOARD_TIME";

     register unsigned int ni = 0;

     unsigned int   on_board_time = info->on_board_time;
     
     /* handle special cases gracefully */
     if ( num_info < 2 ) return;
     
     do {
	  if ( on_board_time != info->on_board_time ) {
	       if ( (ni+1) == num_info ) break;

	       if ( info->on_board_time == info[1].on_board_time ) 
		    on_board_time = info->on_board_time;
	       else if ( on_board_time == info[1].on_board_time ) {
		    info->q.flag.on_board_time = 1;
		    if ( correct_info_rec )
			 info->on_board_time = on_board_time;
	       }
	  }
     } while ( info++, ++ni < num_info );
}

/*+++++++++++++++++++++++++
.IDENTifer   _CHECK_INFO_BCPS
.PURPOSE     consistency check of value of BCPS in info-records
.INPUT/OUTPUT
  call as   _CHECK_INFO_BCPS( correct_info_rec, num_info, info );
     input:  
            bool correct_info_rec    :  allow correction of corrupted data?
            unsigned int num_info    :  number of info-records
 in/output:  
	    struct mds0_info *info   :  info records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void _CHECK_INFO_BCPS( bool correct_info_rec, unsigned char packet_id,
		       unsigned int num_info, struct mds0_info *info )
     /*@modifies info->q.value, info->bcps; @*/
{
     //const char prognm[] = "_CHECK_INFO_BCPS";

     register unsigned int ni = 0;

     register unsigned int nj, on_board_time;

     struct mds0_info *prev, *next;
     
     /* handle special cases gracefully */
     if ( num_info < 2 ) return;

     /* find first detector DSR */
     while ( ni < num_info && info[ni].packet_id != packet_id ) ni++;
     while ( ++ni < num_info && info[ni].packet_id != packet_id );

     do {
	  on_board_time = info[ni].on_board_time;

	  /* find previous info-record in state */
	  nj = ni-1;
	  while ( nj > 0 && info[nj].packet_id != packet_id ) nj--;
	  if ( info[nj].on_board_time != on_board_time ) {
	       while ( ++ni < num_info && info[ni].packet_id != packet_id );
	       continue;
	  }
	  prev = info+nj;

	  /* find next info-record in state */
	  nj = ni;
	  while ( ++nj < num_info && info[nj].packet_id != packet_id );

	  /* no next info record in file */
	  if ( (nj+1) == num_info ) break;
	  if ( info[nj].on_board_time != on_board_time ) {
	       while ( ++ni < num_info && info[ni].packet_id != packet_id );
	       continue;
	  }
	  next = info+nj;

	  (void) fprintf( stderr, "%u %u %hu %u %hu %u %hu\n", ni,
			  prev->on_board_time, prev->bcps, 
			  info[ni].on_board_time, info[ni].bcps, 
			  next->on_board_time, next->bcps ); 
	  
	  if ( next->bcps > prev->bcps && next->bcps < info[ni].bcps ) {
	       unsigned short ii = 15;
	       unsigned short ii_max = 32 - __builtin_clz(next->bcps);
	       unsigned short bcps = info[ni].bcps;

	       while ( ii > ii_max && bcps > next->bcps ) {
		    if ( (((unsigned short) 1) << ii) < bcps )
			 bcps -= ((unsigned short) 1) << ii;
		    ii--;
	       }
	       if ( prev->bcps < bcps && bcps < next->bcps ) {
		    (void) fprintf( stderr,
				    "__builtin_clz(0) : %u %hu %hu %hu %hu\n",
				    info[ni].on_board_time,
				    ii_max, prev->bcps, bcps, next->bcps );
		    info[ni].q.flag.bcps = 1;
		    if ( correct_info_rec ) info[ni].bcps = bcps;
	       }
	  }
	  ni = nj;
     } while ( ni < num_info );
}

/*+++++++++++++++++++++++++
.IDENTifer   _INFO_QSORT
.PURPOSE     sort an array into assending order using the Heapsort algorithm
.INPUT/OUTPUT
  call as   HPSORT( dim, jday, info );
     input:
            unsigned int dim        :   dimension of the array to be sorted
 in/output:
            double       *ref_arr   :   array to be sorted
            struct mds0_info *info  :   array of records to be sorted as jday

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
#define PIX_STACK_SIZE 50
#define PIX_SWAP(a,b) { double temp=(a);(a)=(b);(b)=temp; }
#define INFO_SWAP(a,b) { struct mds0_info c; memcpy(&c, &(a), type_size); memcpy(&(a), &(b), type_size); memcpy(&(b), &c, type_size); }

static inline
void _INFO_QSORT( unsigned int dim, double *ref_arr, struct mds0_info *info )
{
    register int  ii, ir, jj, kk, ll;
    
    int     *i_stack;
    int     j_stack;
    double  ref_tmp;
    
    struct mds0_info info_tmp;

    const size_t type_size = sizeof(struct mds0_info);

    ir = dim ;
    ll = 1 ;
    j_stack = 0 ;
    i_stack = (int *) malloc( PIX_STACK_SIZE * sizeof(int) ) ;
    for (;;) {
        if ( ir-ll < 7 ) {
            for ( jj = ll+1; jj <= ir; jj++ ) {
                ref_tmp = ref_arr[jj-1];
		(void) memcpy( &info_tmp, &info[jj-1], type_size );
                for ( ii = jj-1 ; ii >= 1 ; ii-- ) {
                    if (ref_arr[ii-1] <= ref_tmp) break;
                    ref_arr[ii] = ref_arr[ii-1];
		    (void) memcpy( &info[ii], &info[ii-1], type_size );
                }
                ref_arr[ii] = ref_tmp;
		(void) memcpy( &info[ii], &info_tmp, type_size );
            }
            if (j_stack == 0) break;
            ir = i_stack[j_stack-- -1];
            ll  = i_stack[j_stack-- -1];
        } else {
            kk = (ll+ir) >> 1;
            PIX_SWAP(ref_arr[kk-1], ref_arr[ll])
	    INFO_SWAP(info[kk-1], info[ll])
            if (ref_arr[ll] > ref_arr[ir-1]) {
                PIX_SWAP(ref_arr[ll], ref_arr[ir-1])
		INFO_SWAP(info[ll], info[ir-1])
            }
            if (ref_arr[ll-1] > ref_arr[ir-1]) {
                PIX_SWAP(ref_arr[ll-1], ref_arr[ir-1])
		INFO_SWAP(info[ll-1], info[ir-1])
            }
            if (ref_arr[ll] > ref_arr[ll-1]) {
                PIX_SWAP(ref_arr[ll], ref_arr[ll-1])
		INFO_SWAP(info[ll], info[ll-1])
            }
            ii = ll+1;
            jj = ir;
            ref_tmp = ref_arr[ll-1];
	    (void) memcpy( &info_tmp, &info[ll-1], type_size );
            for (;;) {
                do ii++; while (ref_arr[ii-1] < ref_tmp);
                do jj--; while (ref_arr[jj-1] > ref_tmp);
                if (jj <= ii) break;
                PIX_SWAP(ref_arr[ii-1], ref_arr[jj-1]);
		INFO_SWAP(info[ii-1], info[jj-1])
            }
            ref_arr[ll-1] = ref_arr[jj-1];
            ref_arr[jj-1] = ref_tmp;
	    (void) memcpy( &info[ll-1], &info[jj-1], type_size );
	    (void) memcpy( &info[jj-1], &info_tmp, type_size );
            j_stack += 2;
            if (j_stack > PIX_STACK_SIZE) {
                printf("stack too small in pixel_qsort: aborting");
                exit(-2001) ;
            }
            if (ir-ii+1 >= jj-ll) {
                i_stack[j_stack-1] = ir;
                i_stack[j_stack-2] = ii;
                ir = jj-1;
            } else {
                i_stack[j_stack-1] = jj-1;
                i_stack[j_stack-2] = ll;
                ll = ii;
            }
        }
    }
    free(i_stack);
}
#undef PIX_STACK_SIZE
#undef PIX_SWAP
#undef INFO_SWAP

/*+++++++++++++++++++++++++
.IDENTifer   _REPAIR_INFO_SORTED
.PURPOSE     sort DSRs with ICU onboard time monotonically increasing
.INPUT/OUTPUT
  call as   _REPAIR_INFO_SORTED( num_info, info );
     input:  
            unsigned int  num_info  :  number of info-records
 in/output:  
            struct mds0_info *info  :  info records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void _REPAIR_INFO_SORTED( unsigned int num_info, struct mds0_info *info )
{
     const char prognm[] = "_REPAIR_INFO_SORTED";

     register unsigned int ni = 0;
     
     double *on_board_time;

     /* handle special cases gracefully */
     if ( num_info < 2 ) return;

     /* allocate memory */
     on_board_time = (double *) malloc( num_info * sizeof(double) );
     if ( on_board_time == NULL )
          NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "on_board_time" );

     /* initialize array on_board_time */
     do { 
          on_board_time[ni] = info[ni].on_board_time
	       + (double) info[ni].bcps / 1e4;
     } while ( ++ni < num_info );

     _INFO_QSORT( num_info, on_board_time, info );
     free( on_board_time );
}

/*+++++++++++++++++++++++++
.IDENTifer   _CHECK_INFO_SORTED
.PURPOSE     ICU on_board_time should be increasing monotonically
.INPUT/OUTPUT
  call as   _CHECK_INFO_SORTED( num_info, info );
     input:  
            bool correct_info_rec  : allow correction of corrupted data?
            insigned int num_info  : number of info-records
 in/output:
            struct mds0_info *info : info records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void _CHECK_INFO_SORTED( bool correct_info_rec,
			 unsigned int num_info, struct mds0_info *info )
{
//     const char prognm[] = "_CHECK_INFO_SORTED";
     
     /* handle special cases gracefully */
     if ( num_info < 2 ) return;

     /* sort auxiliary, detector and PMD DSR */
     if ( correct_info_rec )
	  _REPAIR_INFO_SORTED( num_info, info );
}

/*+++++++++++++++++++++++++
.IDENTifer   _CHECK_INFO_STATE_ID
.PURPOSE     consistency check of value of state_id in info-records
.INPUT/OUTPUT
  call as   _CHECK_INFO_STATE_ID( correct_info_rec, num_info, info );
     input:  
            bool correct_info_rec    :  allow correction of corrupted data?
            unsigned int num_info    :  number of info-records
 in/output:  
	    struct mds0_info *info   :  info records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void _CHECK_INFO_STATE_ID( bool correct_info_rec,
			   unsigned int num_info, struct mds0_info *info )
     /*@modifies info->q.value, info->state_id; @*/
{
     //const char prognm[] = "_CHECK_INFO_STATE_ID";

     register size_t ns;

     register unsigned int ni = 0;

     unsigned short counted_state_id[MAX_NUM_STATE];
     
     unsigned int on_board_time = info->on_board_time;

     /* handle special cases gracefully */
     if ( num_info < 2 ) return;

     /* allocate memory for number of records per State ID */
     (void) memset( counted_state_id, 0, MAX_NUM_STATE * sizeof(short) );
     
     do {
	  if ( on_board_time == info->on_board_time ) {
	       if ( info->state_id < MAX_NUM_STATE )
		    counted_state_id[info->state_id-1]++;
	  } else {
	       unsigned short id_found = 0;
	       unsigned short num = 0;
	       unsigned char state_id = 0;
	       
	       for ( ns = 0; ns < MAX_NUM_STATE; ns++ ) {
		    if ( counted_state_id[ns] > 0 ) {
			 id_found++;
			 if ( counted_state_id[ns] > num ) {
			      num = counted_state_id[ns];
			      state_id = (unsigned char) (ns + 1);
			 }
		    }
	       }
	       if ( id_found > 1 ) {
		    register unsigned int nj = 0;

		    do {
			 if ( info[nj].on_board_time == on_board_time
			      && info[nj].state_id != state_id ) {
			      if ( correct_info_rec ) {
				   info[nj].state_id = state_id;
				   info[nj].q.flag.state_id = 1;
			      }
			 }
		    } while ( ++nj < num_info );
	       }
	       (void) memset( counted_state_id, 0,
			      MAX_NUM_STATE * sizeof(short) );
	       on_board_time = info->on_board_time;
	       counted_state_id[info->state_id-1] = 1;
	  }
     } while ( info++, ++ni < num_info );
}

/*+++++++++++++++++++++++++
.IDENTifer   _CHECK_INFO_UNIQUE
.PURPOSE     check if info-records are unique
.INPUT/OUTPUT
  call as   _CHECK_INFO_UNIQUE( correct_info_rec, num_state, states );
     input:  
            unsigned int num_state     :  number of info-records
 in/output:  
	    struct mds0_states *states :  info-records organized per state

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void _CHECK_INFO_UNIQUE( bool correct_info_rec,
			 size_t num_state, struct mds0_states *states )
     /*@modifies states->q.value, states->info->q.value; @*/
{
     // const char prognm[] = "_CHECK_INFO_UNIQUE";

     const size_t sz_mds0_info = sizeof( struct mds0_info );

     register size_t       ns = 0;
     register unsigned int ni;

     struct mds0_info *info;

     /* handle special cases gracefully */
     if ( num_state == 0 ) return;

     do {
	  /* check Auxiliary DSRs */
	  info = states[ns].info_aux;
	  for ( ni = 1; ni < states[ns].num_aux; ni++ ) {
	       if ( info[ni].bcps == info[ni-1].bcps ) {
		    if ( info[ni-1].crc_errors > info[ni].crc_errors
			 || info[ni-1].rs_errors > info[ni].rs_errors
			 || info[ni-1].q.value > info[ni].q.value ) {
			 info[ni-1].q.flag.duplicate = 1;
		    } else {
			 info[ni].q.flag.duplicate = 1;
		    }
	       }
	  }
	  if ( correct_info_rec ) {
	       register unsigned int num = 0;

	       for ( ni = 0; ni < states[ns].num_aux; ni++ ) {
		    if ( info[ni].q.flag.duplicate == 0 ) {
			 if ( ni > num )
			      (void) memcpy( info+num, info+ni, sz_mds0_info );
			 num++;
		    }
	       }
	       if ( states[ns].num_aux != num ) {
		    states[ns].q_aux.flag.duplicates = 1;
		    states[ns].num_aux = num;
	       }
	  }
	  
	  /* check Detector DSRs */
	  info = states[ns].info_det;
	  for ( ni = 1; ni < states[ns].num_det; ni++ ) {
	       if ( info[ni].bcps == info[ni-1].bcps ) {
		    if ( info[ni-1].crc_errors > info[ni].crc_errors
			 || info[ni-1].rs_errors > info[ni].rs_errors
			 || info[ni-1].q.value > info[ni].q.value ) {
			 info[ni-1].q.flag.duplicate = 1;
		    } else {
			 info[ni].q.flag.duplicate = 1;
		    }
	       }
	  }
	  if ( correct_info_rec ) {
	       register unsigned int num = 0;
	  
	       for ( ni = 0; ni < states[ns].num_det; ni++ ) {
	  	    if ( info[ni].q.flag.duplicate == 0 ) {
	  		 if ( ni > num )
	  		      (void) memcpy( info+num, info+ni, sz_mds0_info );
	  		 num++;
	  	    }
	       }
	       if ( states[ns].num_det != num ) {
	  	    states[ns].q_det.flag.duplicates = 1;
	  	    states[ns].num_det = num;
	       }
	  }

	  /* check PMD DSRs */
	  info = states[ns].info_pmd;
	  for ( ni = 1; ni < states[ns].num_pmd; ni++ ) {
	       if ( info[ni].bcps == info[ni-1].bcps ) {
		    if ( info[ni-1].crc_errors > info[ni].crc_errors
			 || info[ni-1].rs_errors > info[ni].rs_errors
			 || info[ni-1].q.value > info[ni].q.value ) {
			 info[ni-1].q.flag.duplicate = 1;
		    } else {
			 info[ni].q.flag.duplicate = 1;
		    }
	       }
	  }
	  if ( correct_info_rec ) {
	       register unsigned int num = 0;
	  
	       for ( ni = 0; ni < states[ns].num_pmd; ni++ ) {
		    if ( info[ni].q.flag.duplicate == 0 ) {
			 if ( ni > num )
			      (void) memcpy( info+num, info+ni, sz_mds0_info );
			 num++;
		    }
	       }
	       if ( states[ns].num_pmd != num ) {
		    states[ns].q_pmd.flag.duplicates = 1;
		    states[ns].num_pmd = num;
	       }
	  }
     } while ( ++ns < num_state );
}

//
// ToDo
// * Checks voor AUX en PMD toevoegen (completeness, DSR size)
// * mds0_states uitbreiden met CLcon informatie per state voor gebruik by lezen DET DSR
//

/*+++++++++++++++++++++++++
.IDENTifer   _SET_INFO_QUALITY
.PURPOSE     check quality of info-records using state definition database
.INPUT/OUTPUT
  call as   _SET_DET_DSR_QUALITY( absOrbit, q_det, num_info, info );
     input:  
            unsigned short absOrbit : orbit number
	    union qstate_re   d_det : DET quality
            unsigned int   num_info : number of info records
 in/output:  
	    struct mds0_info *info  : structure holding info about MDS records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void _SET_DET_DSR_QUALITY( unsigned short absOrbit, union qstate_rec *q_det,
			   unsigned int num_info, struct mds0_info *info )
     /*@modifies nadc_stat, nadc_err_stack, info->quality; @*/
     /*@globals  nadc_stat, nadc_err_stack@*/
{
     // const char prognm[] = "_SET_DET_DSR_QUALITY";

     register unsigned short nr;

     register struct mds0_info *info_pntr = info;

     bool limb_flag = FALSE;

     unsigned char  state_id = 0;

     unsigned short limb_scan = 0;
     unsigned short bcps = 0;
     unsigned short duration = 0;
     unsigned short bcps_effective = 0;
     unsigned short size_ref;
     unsigned short num_dsr_ref;

     const unsigned short num_dsr = (unsigned short) num_info;

     /* handle special cases gracefully */
     if ( num_info < 2 ) return;

     /* check if entry in nadc_clusDef database is valid */
     state_id = info_pntr->state_id;
     if ( ! CLUSDEF_MTBL_VALID( state_id, absOrbit ) ) return;

     duration = CLUSDEF_DURATION( state_id, absOrbit );
     limb_flag = ( info_pntr->category == 2
		   || info_pntr->category == 26 
		   || info_pntr->category == 27
		   || (info_pntr->category == 4
		       && ((duration + 1) % 27) == 0)
		   || (info_pntr->category == 5
		       && ((duration + 1) % 27) == 0)
		   || (info_pntr->category == 7
		       && ((duration + 1) % 27) == 0)
		   || (info_pntr->category == 10
		       && ((duration + 1) % 27) == 0)
		   || (info_pntr->category == 31 
		       && ((duration + 1) % 27) == 0)
		   || (info_pntr->category == 11
		       && ((duration + 1) % 67) == 0)
		   || (info_pntr->category == 28
		       && ((duration + 1) % 67) == 0)
		   || (info_pntr->category == 29
		       && ((duration + 1) % 67) == 0) );
     if ( limb_flag ) {
	  limb_scan = (((duration + 1) % 27) == 0) ? 27 : 67;
     } else
	  limb_scan = 0;
     num_dsr_ref = CLUSDEF_NUM_DET( state_id, absOrbit );
     
     for ( nr = 0; nr < num_dsr; nr++, info_pntr++ ) {
	  bcps_effective = info_pntr->bcps;
	  if ( limb_flag ) {
	       bcps_effective -= 
		    (3 * (info_pntr->bcps / limb_scan) + 2);
	  }

	  /* perform check on size of data-package */
	  size_ref = CLUSDEF_DSR_SIZE( state_id, absOrbit, bcps_effective );
	  if ( size_ref > 0 && info_pntr->packet_length != size_ref ) {
	       info_pntr->q.flag.dsr_size = 1;
	  }
     }

     /* check state */
     bcps = info[num_dsr-1].bcps;
     if ( bcps != duration ) {
	  q_det->flag.too_short = 1;
     } else if ( num_dsr != num_dsr_ref ) {
	  q_det->flag.dsr_missing = 1;
     } 
}

/*+++++++++++++++++++++++++
.IDENTifer   _MDS_INFO_WARNINGS
.PURPOSE     add warning to message error stack
.INPUT/OUTPUT
  call as   _MDS_INFO_WARNINGS( absOrbit, num_state, states );
     input:  
            unsigned short absOrbit : orbit number
            size_t        num_state : number of info records
 in/output:  
	    struct mds0_states *states : info-records organized per state

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void _MDS_INFO_WARNINGS( unsigned short absOrbit,
			 size_t num_state, struct mds0_states *states )
{
     const char prognm[] = "_MDS_INFO_WARNINGS";

     register size_t       ns = 0;
     register unsigned int ni;

     struct mds0_info *info = states[ns].info_det;

     char msg[MAX_STRING_LENGTH];

     const char msg_state[]
	  = "\n#\tfixed %s DSR [%zd,%u] (on_board_time=%u) - state_id to %hhu";
     const char msg_packet[]
	  = "\n#\tfixed %s DSR [%zd,%u] (state_id=%02hhu) - packet_id to %hhu";
     const char msg_duplicate[]
	  = "\n#\tduplicate %s DSR [%zd,%u] (state_id=%02hhu)";
     const char msg_dsr_sz[]
	  = "\n#\tinvalid %s DSR [%zd,%u] (state_id=%02hhu) - size = %u bytes";
     const char msg_duration[]
	  = "\n#\tstate [%zd,%02hhu] (on_board_time=%u) - too short duration (%hu != %hu) in %s DSRs";
     const char msg_missing[]
	  = "\n#\tstate [%zd,%02hhu] (on_board_time=%u) - incorrect number (%u != %u) of %s DSRs";
     const char msg_unique[]
	  = "\n#\tstate [%zd,%02hhu] (on_board_time=%u) - removed duplicated %s DSRs";

     /* handle special cases gracefully */
     if ( num_state == 0 ) return;

     do {
	  /* check Auxiliary DSRs */
	  info = states[ns].info_aux;
	  for ( ni = 0; ni < states[ns].num_aux; ni++ ) {
	       // fixed value state_id
	       if ( info[ni].q.flag.state_id == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_state, "auxiliary", ns, ni, 
				     info[ni].on_board_time,
				     info[ni].state_id );
		    NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       }

	       // fixed value packet_id
	       if ( info[ni].q.flag.packet_id == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_packet, "auxiliary", ns, ni, 
				     info[ni].state_id, 
				     info[ni].packet_id );
		    NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       }

	       // identified duplicated DSR
	       if ( info[ni].q.flag.duplicate == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_duplicate, "auxiliary", ns, ni, 
				     info[ni].state_id );
		    NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       }
	  }
	  if ( states[ns].q_aux.flag.duplicates == 1 ) {
	       (void) snprintf( msg, MAX_STRING_LENGTH,
				msg_unique, ns, 
				states[ns].state_id, states[ns].on_board_time,
				"auxiliary" );
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	  }

	  /* check Detector DSRs */
	  info = states[ns].info_det;
	  for ( ni = 0; ni < states[ns].num_det; ni++ ) {
	       // fixed value state_id
	       if ( info[ni].q.flag.state_id == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_state, "detector", ns, ni, 
				     info[ni].on_board_time,
				     info[ni].state_id );
		    NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       }

	       // fixed value packet_id
	       if ( info[ni].q.flag.packet_id == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_packet, "detector", ns, ni, 
				     info[ni].state_id, 
				     info[ni].packet_id );
		    NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       }

	       // identified duplicated DSR
	       if ( info[ni].q.flag.duplicate == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_duplicate, "detector", ns, ni, 
				     info[ni].state_id );
		    NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       }

	       // invalid DSR size
	       if ( info[ni].q.flag.dsr_size == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_dsr_sz, "detector", ns, ni, 
				     info[ni].state_id, 
				     info[ni].packet_length );
		    NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       }
	  }
	  if ( states[ns].q_det.flag.too_short == 1 ) {
	       (void) snprintf( msg, MAX_STRING_LENGTH,
				msg_duration, ns, 
				states[ns].state_id, states[ns].on_board_time,
				states[ns].info_det[states[ns].num_det-1].bcps,
				CLUSDEF_DURATION( states[ns].state_id, absOrbit ),
				"detector" );
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	  }
	  if ( states[ns].q_det.flag.dsr_missing == 1 ) {
	       (void) snprintf( msg, MAX_STRING_LENGTH,
				msg_missing, ns, 
				states[ns].state_id, states[ns].on_board_time,
				states[ns].num_det,
				CLUSDEF_NUM_DET(states[ns].state_id, absOrbit),
				"detector" );
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	  }
	  if ( states[ns].q_det.flag.duplicates == 1 ) {
	       (void) snprintf( msg, MAX_STRING_LENGTH,
				msg_unique, ns, 
				states[ns].state_id, states[ns].on_board_time,
				"detector" );
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	  }
	  
	  /* check PMD DSRs */
	  info = states[ns].info_pmd;
	  for ( ni = 0; ni < states[ns].num_pmd; ni++ ) {
	       // fixed value state_id
	       if ( info[ni].q.flag.state_id == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_state, "PMD", ns, ni, 
				     info[ni].on_board_time,
				     info[ni].state_id );
		    NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       }

	       // fixed value packet_id
	       if ( info[ni].q.flag.packet_id == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_packet, "PMD", ns, ni, 
				     info[ni].state_id, 
				     info[ni].packet_id );
		    NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       }

	       // identified duplicated DSR
	       if ( info[ni].q.flag.duplicate == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_duplicate, "PMD", ns, ni, 
				     info[ni].state_id );
		    NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       }
	  }
	  if ( states[ns].q_pmd.flag.duplicates == 1 ) {
	       (void) snprintf( msg, MAX_STRING_LENGTH,
				msg_unique, ns, 
				states[ns].state_id, states[ns].on_board_time,
				"PMD" );
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	  }
     } while ( ++ns < num_state );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
size_t SCIA_LV0_RD_MDS_INFO( FILE *fd, unsigned int num_dsd,
			     const struct dsd_envi *dsd, 
			     struct mds0_states **states_out )
{
     const char prognm[] = "SCIA_LV0_RD_MDS_INFO";

     unsigned short absOrbit;
     unsigned int   indx_dsd;
     unsigned int   num_info;

     size_t num_state = 0;

     struct mph_envi    mph;
     struct mds0_info   *info = NULL;
     struct mds0_states *states = NULL;

     const char dsd_name[] = "SCIAMACHY_SOURCE_PACKETS";

     const char *env_str1 = getenv( "NO_INFO_CORRECTION" );
     bool correct_info_rec =
	  (env_str1 != NULL && *env_str1 == '1') ? FALSE : TRUE;

     const char *env_str2 = getenv( "SHOW_INFO_RECORDS" );
     bool show_info_rec =
	  (env_str2 != NULL && *env_str2 == '1') ? TRUE : FALSE;

     bool clusdef_db_exist = CLUSDEF_DB_EXISTS();

     /* initialize return values */
     states_out[0] = NULL;

     /* read Main Product Header */
     ENVI_RD_MPH( fd, &mph );
     absOrbit = (unsigned short) mph.abs_orbit;

     /* get index to data set descriptor */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_ABSENT ) {
	  NADC_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
	  return 0u;
     }

     /* allocate memory to store info-records */
     num_info = dsd[indx_dsd].num_dsr;
     info = (struct mds0_info *) 
	  malloc( (size_t) num_info * sizeof(struct mds0_info) );
     if ( info == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mds0_info" );

     /* extract info-records from input file */
     num_info = GET_SCIA_LV0_MDS_INFO( fd, &dsd[indx_dsd], info );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "GET_SCIA_LV0_MDS_INFO" );
     if ( num_info != dsd[indx_dsd].num_dsr ) {
	  char msg[SHORT_STRING_LENGTH];
	  (void) snprintf( msg, SHORT_STRING_LENGTH,
			   "read error at DSR %u/%u",
			   num_info+1, dsd[indx_dsd].num_dsr );
	  NADC_ERROR( prognm, NADC_ERR_NONE, msg );
     }
     if ( show_info_rec ) _SHOW_INFO_RECORDS( num_info, info );

     /* check Packet type */
     _CHECK_INFO_PACKET_ID( correct_info_rec, num_info, info );

     /* check on_board_time */
     _CHECK_INFO_ON_BOARD_TIME( correct_info_rec, num_info, info );

     /* check BCPS */
     _CHECK_INFO_BCPS( correct_info_rec, SCIA_AUX_PACKET, num_info, info );
     _CHECK_INFO_BCPS( correct_info_rec, SCIA_DET_PACKET, num_info, info );
     _CHECK_INFO_BCPS( correct_info_rec, SCIA_PMD_PACKET, num_info, info );

     /* sort info-records */
     _CHECK_INFO_SORTED( TRUE, num_info, info );
     
     /* check State ID */
     _CHECK_INFO_STATE_ID( correct_info_rec, num_info, info );

     /* combine info-records to states  */
     num_state = _ASSIGN_INFO_STATES( num_info, info, &states );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "_ASSIGN_INFO_STATES(" );
     states_out[0] = states;
     
     /* check for repeated DSR records in product */
     _CHECK_INFO_UNIQUE( correct_info_rec, num_state, states );
     if ( show_info_rec ) _SHOW_STATE_RECORDS( num_state, states );

     /* perform quality check on detector info-records */
     if ( clusdef_db_exist ) {
	  register size_t ns = 0;

	  do {
	       _SET_DET_DSR_QUALITY( absOrbit, &states[ns].q_det,
				     states[ns].num_det, states[ns].info_det );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL,
				     "_SET_DET_DSR_QUALITY" );
	  } while ( ++ns < num_state );
     }
     if ( show_info_rec ) _SHOW_STATE_RECORDS( num_state, states );
     _MDS_INFO_WARNINGS( absOrbit, num_state, states );
done:
     if ( info != NULL ) free( info );
     return num_state;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_FREE_MDS_INFO
.PURPOSE     free memory allocated by SCIA_LV0_MDS_INFO
.INPUT/OUTPUT
  call as   SCIA_LV0_FREE_MDS_INFO( num_states, states );
     input:  
            size_t num_states          : number of MDS info records (per state)
    output:  
            struct mds0_states *states : info-recored organized per state

.RETURNS     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV0_FREE_MDS_INFO( size_t num_states, struct mds0_states *states )
{
     register size_t ns = 0;

     if ( num_states == 0 || states == NULL ) return;

     do {
	  if ( states[ns].num_aux > 0 )
	       free( states[ns].info_aux );
	  if ( states[ns].num_det > 0 )
	       free( states[ns].info_det );
	  if ( states[ns].num_pmd > 0 )
	       free( states[ns].info_pmd );
     } while( ++ns < num_states );

     free( states );
}
