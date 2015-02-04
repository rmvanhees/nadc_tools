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

.IDENTifer   SCIA_LV0_MDS_INFO
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0 data
.LANGUAGE    ANSI C
.PURPOSE     obtain inventarisation of Sciamachy LV0 product
.INPUT/OUTPUT
  call as   nr_info = SCIA_LV0_RD_MDS_INFO( fd, num_dsd, dsd, info );
     input:
             FILE *fd                  : (open) stream pointer to scia-file
             unsigned int num_dsd      : number of DSD records
	     struct dsd_envi *dsd      : structure for the DSD records
    output:
             struct mds0_info **info   : structure for "info" records

.RETURNS     number of info records found (unsigned int)
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
#define CMP_MJD(a,b) memcmp(&(a), &(b), sizeof(struct mjd_envi))

#define REPAIR_INFO_STATE_ID     ((unsigned char) 0x1U)
#define REPAIR_INFO_SORT_BCPS    ((unsigned char) 0x2U)
#define REPAIR_INFO_NUM_DSR      ((unsigned char) 0x4U)
#define REPAIR_INFO_SIZE_DSR     ((unsigned char) 0x8U)
#define REPAIR_INFO_DURATION     ((unsigned char) 0x10U)
#define REPAIR_MISSING_CLCON     ((unsigned char) 0x20U)
#define REPAIR_INFO_UNIQUE       ((unsigned char) 0x40U)

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
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
            struct mds0_states **states : info-recored organized per state

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
     states->q.value = 0;
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
	  switch ( info[ni].packet_type ) {
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
     (void) fprintf( stderr,
		     "_FILL_STATES[%-hhu]: (%u - %u) (%u - %u) (%u - %u)\n",
		     info->state_id, na, states->num_aux, nd, states->num_det,
		     np, states->num_pmd );
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
     
     /* allocate output array */
     states = (struct mds0_states *) malloc( num * sizeof(struct mds0_states) );
     if ( (states_out[0] = states) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mds0_states" );
     num_state = num;
     
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
	  switch ( info[ni].packet_type ) {
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
	       (void) fprintf( stderr, "no valid packet ID: %u\n", ni );
	  }
	  num++;
     } while ( ++ni < num_info );
     
     if ( num > 0 ) {
	  _FILL_STATES( num, &info[num_info-num], &states[ns] );
	  ns++;
     }
     (void) fprintf( stderr, "%u %zd %zd %zd\n", num_info, num, ns, num_state );
     num_state = ns;
done:
     return num_state;
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

static
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
                if (jj < ii) break;
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
            struct mds0_info *info  :   info records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void _REPAIR_INFO_SORTED( unsigned int num_info, struct mds0_info *info )
{
     const char prognm[] = "_REPAIR_INFO_SORTED";

     register unsigned int ni = 0;
     
     double *on_board_time;

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
     const char prognm[] = "_CHECK_INFO_SORTED";
     
     register unsigned int ni = 1;

     bool monotonic = TRUE;
     
     unsigned int on_board_time = info->on_board_time;
/*
 * handle special cases gracefully
 */
     if ( num_info < 2 ) return;
/*
 * check if onboard time is increasing monotonically
 */
     do {
	  if ( info[ni].on_board_time == 0 ) continue;
	  
          if ( on_board_time > info[ni].on_board_time )
               monotonic = FALSE;
          on_board_time = info[ni].on_board_time;
     } while ( monotonic && ++ni < num_info );

     if ( ! monotonic ) {
	  NADC_ERROR( prognm, NADC_ERR_NONE, "info-records not sorted" );
	  if ( correct_info_rec )
	       _REPAIR_INFO_SORTED( num_info, info );
     }
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
	    struct mds0_info *info   :   state-info records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void _CHECK_INFO_STATE_ID( bool correct_info_rec,
			   unsigned int num_info, struct mds0_info *info )
     /*@modifies info->q.value, info->state_id; @*/
{
     const char prognm[] = "_CHECK_INFO_STATE_ID";

     register size_t ns;

     register unsigned int ni, nj;

     char msg[SHORT_STRING_LENGTH];

     unsigned short counted_state_id[MAX_NUM_STATE];
     
     unsigned int on_board_time = info->on_board_time;

     (void) memset( counted_state_id, 0, MAX_NUM_STATE * sizeof(short) );
     
     for ( ni = 0; ni < num_info; ni++ ) {
	  if ( info[ni].on_board_time == 0 ) continue;
	  if ( info[ni].state_id > MAX_NUM_STATE ) continue;
	  
	  if ( on_board_time == info[ni].on_board_time ) {
	       counted_state_id[info[ni].state_id-1]++;
	  } else {
	       unsigned short id_found = 0;
	       unsigned short num = 0;
	       unsigned char state_id = 0;
	       
	       for ( ns = 0; ns < MAX_NUM_STATE; ns++ ) {
		    if ( counted_state_id[ns] > 0 ) {
			 id_found++;
			 if ( counted_state_id[ns] > num ) {
			      num = counted_state_id[ns];
			      state_id = ns + (unsigned char) 1;
			 }
		    }
	       }
	       if ( id_found > 1 ) {
		    for ( nj = 0; nj < num_info; nj++ ) {
			 if ( info[nj].on_board_time == on_board_time
			      && info[nj].state_id != state_id ) {
			      (void) snprintf( msg, SHORT_STRING_LENGTH,
				       "Invalid State ID [%-u/%02hhu] - %hhu", 
				       nj, state_id, info[nj].state_id );
			      NADC_ERROR( prognm, NADC_ERR_NONE, msg );

			      if ( correct_info_rec ) {
				   info[nj].state_id = state_id;
				   info[nj].q.flag.state_id = 1;
			      }
			 }
		    }
	       }
	       (void) memset( counted_state_id, 0,
			      MAX_NUM_STATE * sizeof(short) );
	       on_board_time = info[ni].on_board_time;
	       counted_state_id[info[ni].state_id-1] = 1;
	  }
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   _CHECK_INFO_PACKET_ID
.PURPOSE     consistency check of value of packet_type in info-records
.INPUT/OUTPUT
  call as   _CHECK_INFO_PACKET_ID( correct_info_rec, num_info, info );
     input:  
            bool correct_info_rec    :  allow correction of corrupted data?
            unsigned int num_info    :  number of info-records
 in/output:  
	    struct mds0_info *info   :   state-info records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void _CHECK_INFO_PACKET_ID( bool correct_info_rec,
			   unsigned int num_info, struct mds0_info *info )
     /*@modifies info->q.value, info->state_id; @*/
{
     const char prognm[] = "_CHECK_INFO_PACKET_ID";
     
     register unsigned int ni = 0;

     char msg[SHORT_STRING_LENGTH];

     do { 
	  if ( info[ni].packet_type > (unsigned char) 3 ) {
	       (void) snprintf( msg, SHORT_STRING_LENGTH,
				"Invalid Packet ID [%-u/%02hhu] - %hhu", 
				ni, info[ni].state_id, info[ni].packet_type );
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );

	       if ( correct_info_rec ) {
		    info[ni].packet_type &= (unsigned char) 3;
		    info[ni].q.flag.packet_id = 1;
	       }
	  }
     } while ( ++ni < num_info );
}

/*+++++++++++++++++++++++++
.IDENTifer   _SET_INFO_QUALITY
.PURPOSE     check quality of info-records using state definition database
.INPUT/OUTPUT
  call as   _SET_INFO_QUALITY( absOrbit, num_info, info );
     input:  
            unsigned short absOrbit : orbit number
            unsigned int   num_info : number of info records
 in/output:  
	    struct mds0_info *info  : structure holding info about MDS records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void _SET_INFO_QUALITY( unsigned short absOrbit, unsigned int num_info, 
			struct mds0_info *info )
     /*@modifies nadc_stat, nadc_err_stack, info->quality; @*/
     /*@globals  nadc_stat, nadc_err_stack@*/
{
     const char prognm[] = "_SET_INFO_QUALITY";

     register unsigned int   ni;
     register unsigned short num_det = 0;

     register struct mds0_info *info_pntr = info;

     char msg[SHORT_STRING_LENGTH];

     bool limb_flag = FALSE;

     unsigned char  state_id = 0;

     unsigned short limb_scan = 0;
     unsigned short bcps = 0;
     unsigned short duration = 0;
     unsigned short bcps_effective = 0;
     unsigned short size_ref;

     num_det = 1;
     state_id = info_pntr->state_id;
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
     
     for ( ni = 0; ni < num_info; ni++, ++info_pntr ) {
//	  if ( info_pntr->packet_type != SCIA_DET_PACKET ) continue;

//	  (void) fprintf( stderr, "%6u %2hhu %5hu\n", 
//			  ni, info_pntr->state_id, absOrbit );
	  if ( ! CLUSDEF_MTBL_VALID( info_pntr->state_id, absOrbit ) ) 
	       continue;

	  num_det++;
	  if ( limb_flag ) {
	       bcps_effective = info_pntr->bcps 
		    - (3 * (info_pntr->bcps / limb_scan) + 2);
	  } else
	       bcps_effective = info_pntr->bcps;

//	  if ( state_id == 24 )
//	       (void) fprintf( stderr, "%5u: %3hhu %3hhu %4hu %4hu\n", ni, 
//			       info_pntr->state_id, info_pntr->category, 
//			       bcps_effective, info_pntr->bcps );

	  /* perform check on size of data-package */
	  size_ref = CLUSDEF_DSR_SIZE( state_id, absOrbit, bcps_effective );
	  if ( size_ref > 0 && info_pntr->packet_length != size_ref ) {
	       (void) snprintf( msg, SHORT_STRING_LENGTH,
		 "wrong DSR size [DSR/state]: %-u/%02hhu; size DSR %hu != %hu",
			     ni, state_id, info_pntr->packet_length, size_ref );
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );
//	       info_pntr->quality |= REPAIR_INFO_SIZE_DSR;
	  }
	  bcps = info_pntr->bcps;
     }

     /* check state */
     if ( state_id != 0 ) {
	  unsigned short num_det_ref = CLUSDEF_NUM_DET( state_id, absOrbit );

	  if ( bcps != duration ) {
	       (void) snprintf( msg, SHORT_STRING_LENGTH,
		    "too short state [DSR/state]: %-u/%02hhu; BCPS %hu != %hu", 
				ni, state_id, bcps, duration );
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );
//	       _SET_QFLAG_STATE( REPAIR_INFO_NUM_DSR, indx, num_info, info );
	  } else if ( num_det != num_det_ref ) {
	       (void) snprintf( msg, SHORT_STRING_LENGTH,
	       "incomplete state [DSR/state]: %-u/%02hhu; num DSR: %hu != %hu", 
				ni, state_id, num_det, num_det_ref );
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );
//	       _SET_QFLAG_STATE( REPAIR_INFO_NUM_DSR, indx, num_info, info );
	  } 
     }
}

void _SHOW_INFO_RECORDS( unsigned int num_info, const struct mds0_info *info )
{
     unsigned int ni = 0;

     do {
	  (void) printf( "%02hhu %02hhu %8u %5hu %6hu %5hu %5hu\n",
			 info[ni].packet_type, info[ni].state_id,
			 info[ni].on_board_time, info[ni].bcps,
			 info[ni].packet_length, 
			 info[ni].crc_errors, info[ni].rs_errors );
     } while ( ++ni < num_info );
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

     size_t num_states = 0;

     struct mph_envi    mph;
     struct mds0_info   *info = NULL;
     struct mds0_states *states = NULL;

     const char dsd_name[] = "SCIAMACHY_SOURCE_PACKETS";

     char *env_str1 = getenv( "NO_INFO_CORRECTION" );
     char *env_str2 = getenv( "SHOW_INFO_RECORDS" );

     bool correct_info_rec =
	  (env_str1 != NULL && *env_str1 == '1') ? FALSE : TRUE;
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
     (void) printf( "GET_SCIA_LV0_MDS_INFO: %u\n", num_info );

     /* sort info-records */
     _CHECK_INFO_SORTED( correct_info_rec, num_info, info );
     if ( show_info_rec ) _SHOW_INFO_RECORDS( num_info, info );
     
     /* check State ID */
     _CHECK_INFO_STATE_ID( correct_info_rec, num_info, info );

     /* check Packet type */
     _CHECK_INFO_PACKET_ID( correct_info_rec, num_info, info );

     /* combine info-records to states  */
     num_states = _ASSIGN_INFO_STATES( num_info, info, &states );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "_ASSIGN_INFO_STATES(" );
     states_out[0] = states;
     (void) printf( "_ASSIGN_INFO_STATES: %zd\n", num_states );
     

//     num_info_det = _SORT_INFO_PACKET_ID( num_info, info );
//     if ( IS_ERR_STAT_FATAL )
//	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "_SORT_INFO_PACKET_ID" );

     /* perform quality check on detector info-records */
//     if ( clusdef_db_exist ) {
//	  _SET_INFO_QUALITY( absOrbit, num_info_det, info );
//	  if ( IS_ERR_STAT_FATAL )
//	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "_SET_INFO_QUALITY" );
//     }
done:
     return num_states;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV0_FREE_MDS_INFO
.PURPOSE     free memory allocated by SCIA_LV0_MDS_INFO
.INPUT/OUTPUT
  call as   SCIA_LV0_FREE_MDS_INFO( num_states, states );
     input:  
            size_t num_states          : number of MDS info records (per state)
    output:  
            struct mds0_states *states : MDS info-records

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
