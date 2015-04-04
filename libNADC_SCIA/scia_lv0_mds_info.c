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
.VERSION      4.0   23-Mar-2015	partly re-wite, many improvements, RvH
              3.0   19-Dec-2013	new implementation, complete rewrite, RvH
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
static bool clusdef_db_exists;
static unsigned short absOrbit;

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*++++++++++ function to display info-record information ++++++++++*/
static
void _SHOW_INFO_RECORDS( const char product[],
			 unsigned int num_info, const struct mds0_info *info )
{
     unsigned int ni = 0;

     FILE *outfl;
     char flname[SHORT_STRING_LENGTH];

     /* handle special cases gracefully */
     if ( num_info == 0 ) return;

     (void) snprintf( flname, SHORT_STRING_LENGTH, "%s.info", product );
     if ( (outfl = fopen( flname, "w" )) == NULL )
          NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, flname );
     do {
	  (void) fprintf( outfl,
			  "%9u %02hhu %02hhu %10u %4hu %5hu %3hhu %4hu %4hu\n",
			  info->offset, info->packet_id, info->state_id,
			  info->on_board_time, info->bcps,
			  info->packet_length, info->q.value,
			  info->crc_errors, info->rs_errors );
     } while ( info++, ++ni < num_info );
     (void) fclose( outfl );
}

/*++++++++++ function to display states-record information ++++++++++*/
static
void _SHOW_STATE_RECORDS( const char product[],
			  size_t num_state, const struct mds0_states *states )
{
     size_t ns = 0;

     FILE *outfl;
     char flname[SHORT_STRING_LENGTH];

     /* handle special cases gracefully */
     if ( num_state == 0 ) return;

     (void) snprintf( flname, SHORT_STRING_LENGTH, "%s.states", product );
     if ( (outfl = fopen( flname, "w" )) == NULL )
          NADC_RETURN_ERROR( NADC_ERR_FILE_CRE, flname );
     do {
	  (void) fprintf( outfl,
		       "%9u %02hhu %9u %5hu %5hu %3hhu %5hu %3hhu %5hu %3hhu\n",
			  states->offset, states->state_id, 
			  states->on_board_time, states->orbit,
			  states->num_aux, states->q_aux.value,
			  states->num_det, states->q_det.value,
			  states->num_pmd, states->q_pmd.value );
     } while ( states++, ++ns < num_state );
     (void) fclose( outfl );
}

/*+++++++++++++++++++++++++
.IDENTifer   _INFO_QSORT
.PURPOSE     sort an array into assending order using the Quicksort algorithm
.INPUT/OUTPUT
  call as   _INFO_QSORT( dim, ref_arr, info );
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

    ir = dim;
    ll = 1;
    j_stack = 0;
    i_stack = (int *) malloc( PIX_STACK_SIZE * sizeof(int) );
    if ( i_stack == NULL )
	 NADC_RETURN_ERROR( NADC_ERR_ALLOC, "i_stack" );
    for (;;) {
        if ( ir-ll < 7 ) {
            for ( jj = ll+1; jj <= ir; jj++ ) {
                ref_tmp = ref_arr[jj-1];
		(void) memcpy( &info_tmp, &info[jj-1], type_size );
                for ( ii = jj-1; ii >= 1; ii-- ) {
                    if ( ref_arr[ii-1] <= ref_tmp ) break;
                    ref_arr[ii] = ref_arr[ii-1];
		    (void) memcpy( &info[ii], &info[ii-1], type_size );
                }
                ref_arr[ii] = ref_tmp;
		(void) memcpy( &info[ii], &info_tmp, type_size );
            }
            if ( j_stack == 0 ) break;
            ir = i_stack[j_stack-- -1];
            ll = i_stack[j_stack-- -1];
        } else {
            kk = (ll+ir) >> 1;
            PIX_SWAP(ref_arr[kk-1], ref_arr[ll])
	    INFO_SWAP(info[kk-1], info[ll])
            if ( ref_arr[ll] > ref_arr[ir-1] ) {
                PIX_SWAP(ref_arr[ll], ref_arr[ir-1])
		INFO_SWAP(info[ll], info[ir-1])
            }
            if ( ref_arr[ll-1] > ref_arr[ir-1] ) {
                PIX_SWAP(ref_arr[ll-1], ref_arr[ir-1])
		INFO_SWAP(info[ll-1], info[ir-1])
            }
            if ( ref_arr[ll] > ref_arr[ll-1] ) {
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

                PIX_SWAP(ref_arr[ii-1], ref_arr[jj-1])
		INFO_SWAP(info[ii-1], info[jj-1])
            }
            ref_arr[ll-1] = ref_arr[jj-1];
            ref_arr[jj-1] = ref_tmp;
	    (void) memcpy( &info[ll-1], &info[jj-1], type_size );
	    (void) memcpy( &info[jj-1], &info_tmp, type_size );
            j_stack += 2;
            if ( j_stack > PIX_STACK_SIZE )
		 NADC_GOTO_ERROR( NADC_ERR_FATAL,
				  "stack too small in pixel_qsort: aborting" );

            if ( (ir - ii + 1) >= (jj - ll) ) {
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
done:
    free( i_stack );
}
#undef INFO_SWAP
#undef PIX_SWAP
#undef PIX_STACK_SIZE

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
     
     unsigned short na = 0;
     unsigned short nd = 0;
     unsigned short np = 0;

     (void) memcpy( &states->mjd, &info->mjd, sizeof(struct mjd_envi) );
     states->category = info->category;
     states->state_id = info->state_id;
     states->orbit    = absOrbit;
     states->on_board_time = info->on_board_time;
     states->offset   = info->offset;
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

     if ( states->num_det > 0 ) {
	  bool  flag;
	  int   ibuff;
	  float rbuff;

	  size_t indx = states->num_det / 2;
	  double jday = states->info_det[indx].mjd.days
	       + states->info_det[indx].mjd.secnd / 86400.;

	  GET_SCIA_ROE_INFO( FALSE, jday, &ibuff, &flag, &rbuff );
	  if ( ibuff > 0 ) states->orbit = (unsigned short) ibuff;
     }
}

static
size_t _ASSIGN_INFO_STATES( unsigned int num_info, struct mds0_info *info,
			    /*@out@*/ struct mds0_states **states_out )
     /*@modifies nadc_stat, nadc_err_stack, *states; @*/
     /*@globals  nadc_stat, nadc_err_stack@*/
{
     register unsigned int ni  = 0;

     register size_t ns;
     register size_t num = 0;

     size_t num_state = 0;
     unsigned int offs_aux = 0;
     unsigned int offs_det = 0;
     unsigned int offs_pmd = 0;
     unsigned int on_board_time = 0;

     struct mds0_states *states = NULL;
     
     /* handle special cases gracefully */
     if ( num_info == 0 ) goto done;

     /* find number of states in product */
     for ( ni = 0; ni < num_info; ni++ ) {
	  if ( info[ni].on_board_time > 0
	       && on_board_time != info[ni].on_board_time ) {
	       on_board_time = info[ni].on_board_time;
	       num++;
	  }
     }
     num_state = num;

     /* allocate output array */
     states = (struct mds0_states *) 
	  malloc( num_state * sizeof(struct mds0_states) );
     if ( (states_out[0] = states) == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mds0_states" );
     
     /* fill output array */
     ns = 0;
     num = 0;
     on_board_time = info->on_board_time;
     (void) memset( states, 0, sizeof(struct mds0_states) );

     for ( ni = 0; ni < num_info; ni++, info++ ) {
	  if ( info->on_board_time > 0 ) {
	       if ( on_board_time != info->on_board_time ) {
		    if ( num > 2 ) {
			 _FILL_STATES( num, info-num, states+ns );
			 ns++;
		    }
		    num = 0;
		    on_board_time = info->on_board_time;
		    (void) memset( states+ns, 0, sizeof(struct mds0_states) );
	       }
	       switch ( (int) info->packet_id ) {
	       case SCIA_DET_PACKET:
		    states[ns].num_det++;
		    if ( info->q.flag.sync == 1 )
			 states[ns].q_det.flag.sync = 1;
		    if ( offs_det > info->offset )
			 states[ns].q_det.flag.sorted = 1;
		    offs_det = info->offset;
		    break;
	       case SCIA_AUX_PACKET:
		    states[ns].num_aux++;
		    if ( info->q.flag.sync == 1 )
			 states[ns].q_aux.flag.sync = 1;
		    if ( offs_aux > info->offset )
			 states[ns].q_aux.flag.sorted = 1;
		    offs_aux = info->offset;
		    break;
	       case SCIA_PMD_PACKET:
		    states[ns].num_pmd++;
		    if ( info->q.flag.sync == 1 )
			 states[ns].q_pmd.flag.sync = 1;
		    if ( offs_pmd > info->offset )
			 states[ns].q_pmd.flag.sorted = 1;
		    offs_pmd = info->offset;
		    break;
	       }
	       num++;
	  }
     }
     
     if ( num > 2 && on_board_time != 0 ) {
	  _FILL_STATES( num, info-num, &states[ns] );
	  ns++;
     }
     return ns;
done:
     return num_state;
}

/*+++++++++++++++++++++++++
.IDENTifer   _QCHECK_INFO_PARAMS
.PURPOSE     quick check of state_id values
.INPUT/OUTPUT
  call as   _QCHECK_INFO_PARAMS( correct_info_rec, num_info, info );
     input:  
            bool correct_info_rec    :  allow correction of corrupted data?
            unsigned int num_info    :  number of info-records
 in/output:  
	    struct mds0_info *info   :  info records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void _QCHECK_INFO_PARAMS( bool correct_info_rec,
			  unsigned int num_info, struct mds0_info *info )
     /*@modifies info->q.value, info->state_id; @*/
{
     register unsigned int ni = 0;

     unsigned char state_id;

     /* handle special cases gracefully */
     if ( num_info < 2 ) return;

     /* check value of State ID [1..70] */
     do {
	  if ( (state_id = info->state_id) > MAX_NUM_STATE ) {
	       unsigned short ii = 7;

	       while ( ii >= 1 && state_id > MAX_NUM_STATE ) {
	  	    if ( (unsigned char) (1 << ii) < state_id )
	  		 state_id -= (unsigned char) (1 << ii);
	  	    ii--;
	       }
	       if ( state_id <= MAX_NUM_STATE ) {
	  	    info->q.flag.state_id = 1;
	  	    if ( correct_info_rec ) info->state_id = state_id;
	       }
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
     /*@modifies info->q.value, info->state_id, info->on_board_time; @*/
{
     register int nb;
     
     register unsigned int ii, ni, nk;

     unsigned int num_key;

     struct key_rec {
	  unsigned char  state_id;
	  unsigned short num;
	  unsigned short num_dsr;
	  unsigned int   on_board_time;
	  unsigned int   i_mn;
	  unsigned int   i_mx;
     } *key;
     
     /* handle special cases gracefully */
     if ( num_info < 2 ) return;

     key = (struct key_rec *) malloc( num_info * sizeof(struct key_rec) );
     
     /*
      * create a list of unique combinations of on_board_time and state_id
      * - key->num:      number of DSR occurrences found
      * - key->num_dsr:  number of DSR expected
      * - key->i_mn:     smallest index to array info
      * - key->i_mx:     largest index to array info
      */
     num_key = 0;
     for ( ni = 0; ni < num_info; ni++ ) {
	  bool fnd_key = FALSE;
	  
	  for ( nk = 0; nk < num_key; nk++ ) {
	       if ( info[ni].state_id == key[nk].state_id
		    && info[ni].on_board_time == key[nk].on_board_time ) {
		    key[nk].num++;
		    key[nk].i_mx = ni;
		    fnd_key = TRUE;
		    break;
	       }
	  }
	  if ( ! fnd_key ) {
	       key[num_key].state_id = info[ni].state_id;
	       key[num_key].on_board_time = info[ni].on_board_time;
	       key[num_key].num = 1;
	       if ( info[ni].state_id > 0 )
		    key[num_key].num_dsr =
			 CLUSDEF_NUM_DSR( info[ni].state_id, absOrbit );
	       else
		    key[num_key].num_dsr = 1;
	       key[num_key].i_mn = key[num_key].i_mx = ni;
	       num_key++;
	  }
     }

     /*
      * try to correct state_id values
      */
     for ( nk = 0; nk < num_key; nk++ ) {
	  unsigned short num_thres = 
	       key[nk].num_dsr > 20 ? key[nk].num_dsr / 10 : 2;

	  /* skip small sub-sets or complete sets */
	  if ( key[nk].num >= key[nk].num_dsr || key[nk].num <= num_thres ) 
	       continue;
	  
	  for ( ni = 0; ni < num_key; ni++ ) {
	       if ( ni == nk || key[ni].num == 0
		    || key[ni].on_board_time != key[nk].on_board_time )
		    continue;
	       
	       for ( ii = key[ni].i_mn; ii <= key[ni].i_mx; ii++ ) {
		    if ( info[ii].state_id == key[ni].state_id
			 && info[ii].on_board_time == key[ni].on_board_time ) {
			 info[ii].q.flag.state_id = 1;
			 if ( correct_info_rec )
			      info[ii].state_id = key[nk].state_id;
		    }
	       }
	       if ( key[ni].i_mn < key[nk].i_mn )
		    key[nk].i_mn = key[ni].i_mn;
	       if ( key[ni].i_mx > key[nk].i_mx )
		    key[nk].i_mx = key[ni].i_mx;
	       key[nk].num += key[ni].num;
	       key[ni].num = 0;
	  }
     }

     /*
      * try to correct on_board_time values
      */
     for ( nb = 1; nb <= 3; nb++ ) {
	  for ( nk = 0; nk < num_key; nk++ ) {
	       unsigned short num_thres = 
		    key[nk].num_dsr > 20 ? key[nk].num_dsr / 10 : 2;

	       /* skip small sub-sets or complete sets */
	       if ( key[nk].num >= key[nk].num_dsr || key[nk].num <= num_thres )
		    continue;
	       
	       for ( ni = 0; ni < num_key; ni++ ) {
		    unsigned int diff =
			 key[ni].on_board_time > key[nk].on_board_time ?
			 key[ni].on_board_time - key[nk].on_board_time :
			 key[nk].on_board_time - key[ni].on_board_time;

		    if ( ni == nk || key[ni].num == 0
			 || key[ni].state_id != key[nk].state_id
			 || __builtin_popcount(diff) > nb )
			 continue;

		    for ( ii = key[ni].i_mn; ii <= key[ni].i_mx; ii++ ) {
			 if ( info[ii].on_board_time == key[ni].on_board_time ) {
			      info[ii].q.flag.on_board_time = 1;
			      if ( correct_info_rec )
				   info[ii].on_board_time = key[nk].on_board_time;
			 }
		    }
		    if ( key[ni].i_mn < key[nk].i_mn )
			 key[nk].i_mn = key[ni].i_mn;
		    if ( key[ni].i_mx > key[nk].i_mx )
			 key[nk].i_mx = key[ni].i_mx;
		    key[nk].num += key[ni].num;
		    key[ni].num = 0;
	       }
	  }
     }

     /* release allocated memory */
     free( key );
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
	  if ( nj == num_info ) break;
	  if ( info[nj].on_board_time != on_board_time ) {
	       while ( ++ni < num_info && info[ni].packet_id != packet_id );
	       continue;
	  }
	  next = info+nj;

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
		    info[ni].q.flag.bcps = 1;
		    if ( correct_info_rec ) info[ni].bcps = bcps;
	       }
	  }
	  ni = nj;
     } while ( ni < num_info );
}

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
static inline
void _REPAIR_INFO_SORTED( unsigned int num_info, struct mds0_info *info )
{
     register unsigned int ni = 0;
     
     double *on_board_time;

     /* allocate memory */
     on_board_time = (double *) malloc( num_info * sizeof(double) );
     if ( on_board_time == NULL )
          NADC_RETURN_ERROR( NADC_ERR_ALLOC, "on_board_time" );

     /* initialize array on_board_time */
     do { 
          on_board_time[ni] = info[ni].on_board_time
	       + (double) info[ni].bcps / 16.;
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
     /* handle special cases gracefully */
     if ( num_info < 2 ) return;

     /* sort auxiliary, detector and PMD DSR */
     if ( correct_info_rec )
	  _REPAIR_INFO_SORTED( num_info, info );
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
     const size_t sz_mds0_info = sizeof( struct mds0_info );

     register size_t         ns = 0;
     register unsigned short ni, num;

     struct mds0_info *info;

     /* handle special cases gracefully */
     if ( num_state == 0 ) return;

     do {
	  /* check Auxiliary DSRs */
	  info = states[ns].info_aux;
	  for ( ni = 1; ni < states[ns].num_aux; ni++ ) {
	       if ( info[ni].bcps == info[ni-1].bcps ) {
		    info[ni].q.flag.duplicate = 1;
	       }
	  }
	  if ( correct_info_rec ) {
	       for ( num = ni = 0; ni < states[ns].num_aux; ni++ ) {
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
		    info[ni].q.flag.duplicate = 1;
	       }
	  }
	  if ( correct_info_rec ) {
	       for ( num = ni = 0; ni < states[ns].num_det; ni++ ) {
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
		    info[ni].q.flag.duplicate = 1;
	       }
	  }
	  if ( correct_info_rec ) {
	       for ( num = ni = 0; ni < states[ns].num_pmd; ni++ ) {
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

/*+++++++++++++++++++++++++
.IDENTifer   _SET_DET_INFO_QUALITY
.PURPOSE     check quality of info-records using state definition database
.INPUT/OUTPUT
  call as   _SET_DET_DSR_QUALITY( states );
 in/output:  
	    struct mds0_states *states : info-records organized per states

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void _SET_DET_DSR_QUALITY( struct mds0_states *states )
     /*@modifies nadc_stat, nadc_err_stack, states->q_det, 
       states->info_det.quality; @*/
     /*@globals  nadc_stat, nadc_err_stack@*/
{
     register unsigned short nr;

     bool limb_flag = FALSE;

     unsigned short limb_scan = 0;
     unsigned short duration = 0;
     unsigned short bcps_effective = 0;
     unsigned short num_duplicate = 0;
     unsigned short num_det_ref;
     unsigned short size_ref;

     unsigned short num_det = states->num_det;
     struct mds0_info *info_pntr = states->info_det;

     /* handle special cases gracefully */
     if ( num_det < 2 ) return;

     /* use nadc_clusDef database as reference */
     if ( ! CLUSDEF_MTBL_VALID( states->state_id, states->orbit ) ) return;
     duration = CLUSDEF_DURATION( states->state_id, states->orbit );
     num_det_ref = CLUSDEF_NUM_DET( states->state_id, states->orbit );

     /* limb profiles */
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
     
     for ( nr = 0; nr < num_det; nr++, info_pntr++ ) {
	  bcps_effective = info_pntr->bcps;
	  if ( limb_flag ) {
	       bcps_effective -= 
		    (3 * (info_pntr->bcps / limb_scan) + 2);
	  }
	  
	  /* perform check on size of data-package */
	  size_ref = CLUSDEF_DSR_SIZE( states->state_id, states->orbit,
				       bcps_effective );
	  if ( size_ref > 0 && info_pntr->packet_length != size_ref )
	       info_pntr->q.flag.dsr_size = 1;

	  if ( info_pntr->q.flag.duplicate == 1 ) num_duplicate++;
     }

     /* check state */
     if ( states->info_det[num_det-1].bcps != duration ) {
	  states->q_det.flag.too_short = 1;
     } else if ( (num_det - num_duplicate) != num_det_ref ) {
	  states->q_det.flag.dsr_missing = 1;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   _SET_AUX_INFO_QUALITY
.PURPOSE     check quality of info-records using state definition database
.INPUT/OUTPUT
  call as   _SET_AUX_DSR_QUALITY( states );
 in/output:  
	    struct mds0_states *states : info-records organized per states

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void _SET_AUX_DSR_QUALITY( struct mds0_states *states )
     /*@modifies nadc_stat, nadc_err_stack, states->q_aux; @*/
     /*@globals  nadc_stat, nadc_err_stack@*/
{
     /* handle special cases gracefully */
     if ( states->num_aux == 0 ) return;

     /* use nadc_clusDef database as reference */
     if ( ! CLUSDEF_MTBL_VALID( states->state_id, states->orbit ) ) return;

     /* check state */
     if ( states->num_aux < CLUSDEF_NUM_AUX(states->state_id, states->orbit) ) {
	  states->q_aux.flag.dsr_missing = 1;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   _SET_PMD_INFO_QUALITY
.PURPOSE     check quality of info-records using state definition database
.INPUT/OUTPUT
  call as   _SET_PMD_DSR_QUALITY( states );
 in/output:  
	    struct mds0_states *states : info-records organized per states

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void _SET_PMD_DSR_QUALITY( struct mds0_states *states )
     /*@modifies nadc_stat, nadc_err_stack, states->q_pmd; @*/
     /*@globals  nadc_stat, nadc_err_stack@*/
{
     /* handle special cases gracefully */
     if ( states->num_pmd == 0 ) return;

     /* use nadc_clusDef database as reference */
     if ( ! CLUSDEF_MTBL_VALID( states->state_id, states->orbit ) ) return;

     /* check state */
     if ( states->num_pmd < CLUSDEF_NUM_PMD(states->state_id, states->orbit) ) {
	  states->q_pmd.flag.dsr_missing = 1;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   _MDS_INFO_WARNINGS
.PURPOSE     add warning to message error stack
.INPUT/OUTPUT
  call as   _MDS_INFO_WARNINGS( num_state, states );
     input:  
            size_t        num_state : number of info records
 in/output:  
	    struct mds0_states *states : info-records organized per state

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void _MDS_INFO_WARNINGS( size_t num_state, struct mds0_states *states )
{
     register size_t         ns = 0;
     register unsigned short ni;

     struct mds0_info *info;

     char msg[MAX_STRING_LENGTH];

     const char msg_packet[]
	  = "\n#\tfixed %s DSR [%zd,%u] (state_id=%02hhu) - packet_id to %hhu";
     const char msg_state[]
	  = "\n#\tfixed %s DSR [%zd,%u] (on_board_time=%u) - state_id to %hhu";
     const char msg_time[]
	  = "\n#\tfixed %s DSR [%zd,%u] (state_id=%02hhu) - on_board_time to %u";
     const char msg_bcps[]
	  = "\n#\tfixed %s DSR [%zd,%u] (state_id=%02hhu) - bcps to %hu";
     const char msg_sync[]
	  = "\n#\tcorrupted sync values in %s DSR [%zd,%u] (state_id=%02hhu)";
     const char msg_duplicate[]
	  = "\n#\tduplicate %s DSR [%zd,%u] (state_id=%02hhu)";
     const char msg_dsr_sz[]
	  = "\n#\tinvalid %s DSR [%zd,%u] (state_id=%02hhu) - size = %u bytes";
     const char msg_duration[]
	  = "\n#\tstate [%zd,%02hhu] (on_board_time=%u) - too short duration (%hu != %hu) in %s DSRs";
     const char msg_missing[]
	  = "\n#\tstate [%zd,%02hhu] (on_board_time=%u) - incorrect number (%hu != %hu) of %s DSRs";
     const char msg_unique[]
	  = "\n#\tstate [%zd,%02hhu] (on_board_time=%u) - removed duplicated %s DSRs";
     const char msg_sorted[]
	  = "\n#\tstate [%zd,%02hhu] (on_board_time=%u) - non-chronological %s DSRs";

     /* handle special cases gracefully */
     if ( num_state == 0 ) return;

     do {
	  /* check Auxiliary DSRs */
	  info = states[ns].info_aux;
	  for ( ni = 0; ni < states[ns].num_aux; ni++ ) {
	       // fixed value(s) of packet_id
	       if ( info[ni].q.flag.packet_id == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_packet, "auxiliary", ns, ni, 
				     info[ni].state_id, 
				     info[ni].packet_id );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // fixed value(s) of state_id
	       if ( info[ni].q.flag.state_id == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_state, "auxiliary", ns, ni, 
				     info[ni].on_board_time,
				     info[ni].state_id );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // fixed value(s) of_board_time
	       if ( info[ni].q.flag.on_board_time == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_time, "auxiliary", ns, ni, 
				     info[ni].state_id, 
				     info[ni].on_board_time );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // fixed value(s) of bcps
	       if ( info[ni].q.flag.bcps == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_bcps, "auxiliary", ns, ni, 
				     info[ni].state_id, 
				     info[ni].bcps );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // corrupted sync value(s) found
	       if ( info[ni].q.flag.sync == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_sync, "auxiliary", ns, ni, 
				     info[ni].state_id );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // identified duplicated DSR
	       if ( info[ni].q.flag.duplicate == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_duplicate, "auxiliary", ns, ni, 
				     info[ni].state_id );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }
	  }
	  if ( states[ns].q_aux.flag.dsr_missing == 1 ) {
	       (void) snprintf( msg, MAX_STRING_LENGTH,
				msg_missing, ns, 
				states[ns].state_id, states[ns].on_board_time,
				states[ns].num_aux,
				CLUSDEF_NUM_AUX(states[ns].state_id,
						states[ns].orbit),
				"Auxiliary" );
	       NADC_ERROR( NADC_ERR_NONE, msg );
	  }
	  if ( states[ns].q_aux.flag.duplicates == 1 ) {
	       (void) snprintf( msg, MAX_STRING_LENGTH,
				msg_unique, ns, 
				states[ns].state_id, states[ns].on_board_time,
				"auxiliary" );
	       NADC_ERROR( NADC_ERR_NONE, msg );
	  }
	  if ( states[ns].q_aux.flag.sorted == 1 ) {
	       (void) snprintf( msg, MAX_STRING_LENGTH,
				msg_sorted, ns, 
				states[ns].state_id, states[ns].on_board_time,
				"auxiliary" );
	       NADC_ERROR( NADC_ERR_NONE, msg );
	  }

	  /* check Detector DSRs */
	  info = states[ns].info_det;
	  for ( ni = 0; ni < states[ns].num_det; ni++ ) {
	       // fixed value(s) of packet_id
	       if ( info[ni].q.flag.packet_id == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_packet, "detector", ns, ni, 
				     info[ni].state_id, 
				     info[ni].packet_id );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // fixed value(s) of state_id
	       if ( info[ni].q.flag.state_id == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_state, "detector", ns, ni, 
				     info[ni].on_board_time,
				     info[ni].state_id );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // fixed value(s) of_board_time
	       if ( info[ni].q.flag.on_board_time == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_time, "detector", ns, ni, 
				     info[ni].state_id, 
				     info[ni].on_board_time );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // fixed value(s) of bcps
	       if ( info[ni].q.flag.bcps == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_bcps, "detector", ns, ni, 
				     info[ni].state_id, 
				     info[ni].bcps );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // corrupted sync value(s) found
	       if ( info[ni].q.flag.sync == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_sync, "detector", ns, ni, 
				     info[ni].state_id );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // identified duplicated DSR
	       if ( info[ni].q.flag.duplicate == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_duplicate, "detector", ns, ni, 
				     info[ni].state_id );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // invalid DSR size
	       if ( info[ni].q.flag.dsr_size == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_dsr_sz, "detector", ns, ni, 
				     info[ni].state_id, 
				     info[ni].packet_length );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }
	  }
	  if ( states[ns].q_det.flag.too_short == 1 ) {
	       (void) snprintf( msg, MAX_STRING_LENGTH,
				msg_duration, ns, 
				states[ns].state_id, states[ns].on_board_time,
				states[ns].info_det[states[ns].num_det-1].bcps,
				CLUSDEF_DURATION( states[ns].state_id,
						  states[ns].orbit ),
				"detector" );
	       NADC_ERROR( NADC_ERR_NONE, msg );
	  }
	  if ( states[ns].q_det.flag.dsr_missing == 1 ) {
	       (void) snprintf( msg, MAX_STRING_LENGTH,
				msg_missing, ns, 
				states[ns].state_id, states[ns].on_board_time,
				states[ns].num_det,
				CLUSDEF_NUM_DET( states[ns].state_id,
						 states[ns].orbit ),
				"detector" );
	       NADC_ERROR( NADC_ERR_NONE, msg );
	  }
	  if ( states[ns].q_det.flag.duplicates == 1 ) {
	       (void) snprintf( msg, MAX_STRING_LENGTH,
				msg_unique, ns, 
				states[ns].state_id, states[ns].on_board_time,
				"detector" );
	       NADC_ERROR( NADC_ERR_NONE, msg );
	  }
	  if ( states[ns].q_det.flag.sorted == 1 ) {
	       (void) snprintf( msg, MAX_STRING_LENGTH,
				msg_sorted, ns, 
				states[ns].state_id, states[ns].on_board_time,
				"detector" );
	       NADC_ERROR( NADC_ERR_NONE, msg );
	  }

	  /* check PMD DSRs */
	  info = states[ns].info_pmd;
	  for ( ni = 0; ni < states[ns].num_pmd; ni++ ) {
	       // fixed value(s) of packet_id
	       if ( info[ni].q.flag.packet_id == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_packet, "PMD", ns, ni, 
				     info[ni].state_id, 
				     info[ni].packet_id );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // fixed value(s) of state_id
	       if ( info[ni].q.flag.state_id == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_state, "PMD", ns, ni, 
				     info[ni].on_board_time,
				     info[ni].state_id );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // fixed value(s) of_board_time
	       if ( info[ni].q.flag.on_board_time == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_time, "PMD", ns, ni, 
				     info[ni].state_id, 
				     info[ni].on_board_time );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // fixed value(s) of bcps
	       if ( info[ni].q.flag.bcps == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_bcps, "PMD", ns, ni, 
				     info[ni].state_id, 
				     info[ni].bcps );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // corrupted sync value(s) found
	       if ( info[ni].q.flag.sync == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_sync, "PMD", ns, ni, 
				     info[ni].state_id );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }

	       // identified duplicated DSR
	       if ( info[ni].q.flag.duplicate == 1 ) {
		    (void) snprintf( msg, MAX_STRING_LENGTH,
				     msg_duplicate, "PMD", ns, ni, 
				     info[ni].state_id );
		    NADC_ERROR( NADC_ERR_NONE, msg );
	       }
	  }
	  if ( states[ns].q_pmd.flag.dsr_missing == 1 ) {
	       (void) snprintf( msg, MAX_STRING_LENGTH,
				msg_missing, ns, 
				states[ns].state_id, states[ns].on_board_time,
				states[ns].num_pmd,
				CLUSDEF_NUM_PMD( states[ns].state_id,
						 states[ns].orbit),
				"PMD" );
	       NADC_ERROR( NADC_ERR_NONE, msg );
	  }
	  if ( states[ns].q_pmd.flag.duplicates == 1 ) {
	       (void) snprintf( msg, MAX_STRING_LENGTH,
				msg_unique, ns, 
				states[ns].state_id, states[ns].on_board_time,
				"PMD" );
	       NADC_ERROR( NADC_ERR_NONE, msg );
	  }
	  if ( states[ns].q_pmd.flag.sorted == 1 ) {
	       (void) snprintf( msg, MAX_STRING_LENGTH,
				msg_sorted, ns, 
				states[ns].state_id, states[ns].on_board_time,
				"PMD" );
	       NADC_ERROR( NADC_ERR_NONE, msg );
	  }
     } while ( ++ns < num_state );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
size_t SCIA_LV0_RD_MDS_INFO( FILE *fd, unsigned int num_dsd,
			     const struct dsd_envi *dsd, 
			     struct mds0_states **states_out )
{
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

     /* initialize return values */
     states_out[0] = NULL;

     /* read Main Product Header */
     ENVI_RD_MPH( fd, &mph );

     /* set static variables */
     absOrbit = (unsigned short) mph.abs_orbit;
     clusdef_db_exists = CLUSDEF_DB_EXISTS();

     /* get index to data set descriptor */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_ABSENT ) {
	  NADC_ERROR( NADC_ERR_PDS_RD, dsd_name );
	  return 0u;
     }

     /* allocate memory to store info-records */
     num_info = dsd[indx_dsd].num_dsr;
     info = (struct mds0_info *) 
	  malloc( (size_t) num_info * sizeof(struct mds0_info) );
     if ( info == NULL )
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mds0_info" );

     /* extract info-records from input file */
     num_info = GET_SCIA_LV0_MDS_INFO( fd, &dsd[indx_dsd], info );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, "GET_SCIA_LV0_MDS_INFO" );
     if ( num_info != dsd[indx_dsd].num_dsr ) {
	  char msg[SHORT_STRING_LENGTH];
	  (void) snprintf( msg, SHORT_STRING_LENGTH,
			   "read error at DSR %u/%u",
			   num_info+1, dsd[indx_dsd].num_dsr );
	  NADC_ERROR( NADC_ERR_NONE, msg );
     }
     if ( show_info_rec ) _SHOW_INFO_RECORDS( mph.product, num_info, info );

     /*
      * - There is no need to check the size of DSRs, because the read should
      *   fail in GET_SCIA_LV0_MDS_INFO when the packet_length is corrupted
      * - Complicating factor is that DSR's are not always sorted correctly
      *
      * - ToDo: 
      *     * majority of products are fine: implement quick-check to 
      *       skip unnecessary tests
      *     * assumed is that values are corrupted by to bit-flipping, however,
      *       this is not checked in all cases
      *
      * 1a) Check and fix 'Pack ID' (quick):
      *    - value of the Packet ID can be checked against the packet_length
      *    - only packet_id = 0 or packet_id > 3 are assumed to be corrupted
      * 1b) Check and fix 'State ID' (quick):
      *    - only state_id = 0 or state_id > 70 are assumed to be corrupted
      *      state_id > 70: subtrackt 128
      * 1c) Check and fix 'bcps' (requires clusdef_db_exist):
      *    - check/fix if bcps is out-of-range for given (state_id, orbit)
      * 2) Check and fix combination 'On Board Time' and 'State ID':
      *    - each on_board_time value should match only one state_id
      *      and each state_id can be matched with several on_board_time values
      *    - each combination on state_id and on_board_time should occur many 
      *      times in a product
      *    - unique on_board_time values may indicate data corruption, can be 
      *      fixed when 3 successive packages (around corrupted on_board_time) 
      *      have same state_id and/or incremental bcps values, alternatively 
      *      one could check if state_id (ofcorrupted on_board_time) occures 
      *      more often and one of these is missing one occurence
      *    - 
      */
     /* quick check on Packet ID, State ID and BCPS */
     _QCHECK_INFO_PARAMS( correct_info_rec, num_info, info );

     /* check on_board_time */
     _CHECK_INFO_ON_BOARD_TIME( correct_info_rec, num_info, info );

     /* check BCPS */
     _CHECK_INFO_BCPS( correct_info_rec, SCIA_AUX_PACKET, num_info, info );
     _CHECK_INFO_BCPS( correct_info_rec, SCIA_DET_PACKET, num_info, info );
     _CHECK_INFO_BCPS( correct_info_rec, SCIA_PMD_PACKET, num_info, info );

     /* sort info-records */
     _CHECK_INFO_SORTED( TRUE, num_info, info );
//     if ( show_info_rec ) _SHOW_INFO_RECORDS( mph.product, num_info, info );
     
     /* combine info-records to states  */
     num_state = _ASSIGN_INFO_STATES( num_info, info, &states );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_FATAL, "_ASSIGN_INFO_STATES(" );
     states_out[0] = states;

     /* check for repeated DSR records in product */
     _CHECK_INFO_UNIQUE( correct_info_rec, num_state, states );

     /* perform quality check on detector info-records */
     if ( clusdef_db_exists ) {
	  register size_t ns = 0;

	  do {
	       _SET_AUX_DSR_QUALITY( states+ns );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_FATAL, "_SET_AUX_DSR_QUALITY" );
	       _SET_DET_DSR_QUALITY( states+ns );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_FATAL, "_SET_DET_DSR_QUALITY" );
	       _SET_PMD_DSR_QUALITY( states+ns );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_FATAL, "_SET_PMD_DSR_QUALITY" );
	  } while ( ++ns < num_state );
     }
     if ( show_info_rec ) _SHOW_STATE_RECORDS( mph.product, num_state, states );
     _MDS_INFO_WARNINGS( num_state, states );
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
