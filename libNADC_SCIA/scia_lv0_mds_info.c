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
.IDENTifer   _SET_QFLAG_STATE
.PURPOSE     set State quality of all DSR which belong to one state execution
.INPUT/OUTPUT
  call as   _SET_QFLAG_STATE( qflag, index, num_info, info );
     input:  
	    unsigned char  qflag   : value of quality
	    unsigned short indx    : index of state
            unsigned int num_info  : number of info records
 in/output:  
	    struct mds0_info *info : structure holding info about MDS records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static inline
void _SET_QFLAG_STATE( unsigned char qflag, unsigned short state_index,
		       unsigned int num_info, struct mds0_info *info )
{
     register unsigned int ni = 0;

     do {
	  if ( info[ni].state_index == state_index )
	       info[ni].quality |= qflag;
     } while ( ++ni < num_info );
}

/*+++++++++++++++++++++++++
.IDENTifer   _REPAIR_INFO_MJD
.PURPOSE     sort DSRs with ICU onboard time monotonically increasing
.INPUT/OUTPUT
  call as   _REPAIR_INFO_MJD( num_info, info );
     input:  
            unsigned int   num_info    :  number of info-records
 in/output:  
	    struct mds0_info *info  :   info records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static inline
void shellsort_uint32( unsigned int length, unsigned int array[] )
{
     register unsigned int ii, jj, kol, tmp;

     for ( kol = length/3; ; kol /= 3 ) {
	  if ( kol == 0 ) kol = 1;

	  for ( ii = kol; ii < length; ii++ ) {
	       jj = 0;

	       while( ii-jj >= kol && array[ii-jj] < array[ii-kol-jj] ) {
		    tmp = array[ii-jj];
		    array[ii-jj] = array[ii-kol-jj];
		    array[ii-kol-jj] = tmp;
		    jj += kol;
	       }
	  }
	  if ( kol == 1 ) return;
     }
}

static inline
void _REPAIR_INFO_MJD( unsigned int num_info, struct mds0_info *info )
     /*@modifies nadc_stat, nadc_err_stack, info; @*/
     /*@globals  nadc_stat, nadc_err_stack@*/
{
     const char prognm[] = "_REPAIR_INFO_MJD";

     register unsigned int ni, no;
     register unsigned int val;
     
     unsigned int num_obt = 0;
     unsigned int *on_board_time;

     struct mds0_info *info_tmp;

     if ( num_info == 0 ) return;

     /* allocate memory */
     on_board_time = (unsigned int *) malloc( num_info * sizeof(int) );
     if ( on_board_time == NULL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "on_board_time" );

     /* initialize arrays */
     ni = 0;
     do { 
	  on_board_time[ni] = info[ni].on_board_time;
     } while ( ++ni < num_info );

     shellsort_uint32( num_info, on_board_time );

     /* obtain unique values of onboard time */
     val = *on_board_time;
     for ( ni = 0; ni < num_info; ni++ ) {
	  if ( on_board_time[ni] != val ) {
	       val = (on_board_time[++num_obt] = on_board_time[ni]);
	  }
     }
     num_obt++;

     /* 
      * Sequence of the info-records with the same onboard time should 
      * not be changed, therefore a temparary buffer is used for the sorting
      */
     info_tmp = (struct mds0_info *)
	  malloc( num_info * sizeof(struct mds0_info) );
     if ( info_tmp == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "info_tmp" );
     (void) memcpy( info_tmp, info, num_info * sizeof(struct mds0_info) );

     /* sort info-records */
     for ( no = 0; no < num_obt; no++ ) {
	  for ( ni = 0; ni < num_info; ni++ ) {
	       if ( info_tmp[ni].on_board_time == on_board_time[no] ) {
		    (void) memcpy(info, info_tmp+ni, sizeof(struct mds0_info));
		    info++;
	       }
	  }
     }
     free( info_tmp );
done:
     free( on_board_time );
}

/*+++++++++++++++++++++++++
.IDENTifer   _CHECK_INFO_MJD
.PURPOSE     ICU on_board_time should be increasing monotonically
.INPUT/OUTPUT
  call as   _CHECK_INFO_MJD( num_info, info );
     input:  
            insigned int num_info  :   number of info-records
 in/output:  
	    struct mds0_info *info :   info records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void _CHECK_INFO_MJD( unsigned int num_info, struct mds0_info *info )
     /*@modifies nadc_stat, nadc_err_stack, info; @*/
     /*@globals  nadc_stat, nadc_err_stack@*/
{
     const char prognm[] = "_CHECK_INFO_MJD";

     register unsigned int ni = 0;

     unsigned int on_board_time = 0;
/*
 * handle special cases gracefully
 */
     if ( num_info < 2 ) return;
/*
 * check if onboard time is increasing monotonically
 */
     for ( ni = 0; ni < num_info; ni++ ) {
	  if ( on_board_time > info[ni].on_board_time ) {
	       char msg[SHORT_STRING_LENGTH];

      	       (void) snprintf( msg, SHORT_STRING_LENGTH,
      				"MJD of DSRs not monotonic increasing");
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );

	       _REPAIR_INFO_MJD( num_info, info );
	       break;
	  }
	  on_board_time = info[ni].on_board_time;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   _SET_INFO_STATEINDEX
.PURPOSE     identify DSR packages of the same state
.INPUT/OUTPUT
  call as   _SET_INFO_STATEINDEX( num_info, info );
     input:  
            unsigned int num_info  : number of info records
 in/output:  
	    struct mds0_info *info : structure holding info about MDS records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void _SET_INFO_STATEINDEX( unsigned int num_info, struct mds0_info *info )
     /*@modifies info->quality, info->state_index @*/
{
//     const char prognm[] = "_SET_INFO_STATEINDEX";

     register unsigned int ni, num;
     register unsigned int val;

     unsigned short state_index = 1;
/*
 * handle special cases gracefully
 */
     if ( num_info == 0 ) return;
     if ( num_info == 1 ) {
	  info[0].state_index = 1;
	  return;
     }
/*
 * collect all readouts belonging to one state
 */
     ni = 0;
     while ( ni < num_info && info[0].on_board_time == 0 ) {
	  info[ni].state_index = USHRT_MAX;
	  ni++;
     }
     val = info[ni].on_board_time;

     for ( num = ni = 0; ni < num_info; ni++ ) {
	  if ( info[ni].on_board_time != val ) {
	       val = info[ni].on_board_time;
	       if ( num == 1 ) 
		    info[ni-1].state_index = USHRT_MAX;
	       else
		    state_index++;
	       num = 0;
	  }
	  info[ni].state_index = state_index;
	  num++;
     }
/*
 * repair State Counter (ToDo check implementation/move to separate function)
 */
     /* for ( ni = 1; ni < (num_info-1); ni++ ) { */
     /* 	  if ( info[ni].state_index == USHRT_MAX ) { */
     /* 	       info[ni].quality |= SCIA_DET_CORRUPT_MJD; */
     /* 	       (void) snprintf( msg, SHORT_STRING_LENGTH, */
     /* 				"correct state_index record/state: %-u/%02hhu",  */
     /* 				ni, info[ni].state_id ); */

     /* 	       if ( info[ni-1].state_id == info[ni+1].state_id ) { */
     /* 		    info[ni].state_index = info[ni+1].state_index; */
     /* 		    NADC_ERROR( prognm, NADC_ERR_NONE, msg ); */
     /* 	       } */
     /* 	  } */
     /* } */
}

/*+++++++++++++++++++++++++
.IDENTifer   _REPAIR_INFO_STATE_ID
.PURPOSE     consistency check of value of state_id in info-records
.INPUT/OUTPUT
  call as   _REPAIR_INFO_STATE_ID( state_index, num_info, info );
     input:  
            unsigned short state_index :  state index
            unsigned int   num_info    :  number of info-records
 in/output:  
	    struct mds0_info *info  :   info records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static inline
void _REPAIR_INFO_STATE_ID( unsigned short state_index,
			    unsigned int num_info, struct mds0_info *info )
     /*@modifies nadc_stat, nadc_err_stack, info->quality, info->state_id; @*/
     /*@globals  nadc_stat, nadc_err_stack@*/
{
     const char prognm[] = "_REPAIR_INFO_STATE_ID";

     char msg[SHORT_STRING_LENGTH];

     register unsigned int ni;
     register unsigned short num = 0;

     unsigned char state_id;
     unsigned char *state_array;

     size_t num_index = 0;

     if ( num_info == 0 ) return;

     ni = 0;
     do { 
	  if ( info[ni].state_index == state_index ) num++;
     } while ( ++ni < num_info );
     state_array = (unsigned char *) malloc( (num_index = (size_t) num) );
     if ( state_array == NULL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "state_array" );
     num = 0; ni = 0;
     do {
	  if ( info[ni].state_index == state_index )
	       state_array[num++] = info[ni].state_id;
     } while ( ++ni < num_info );
     state_id = SELECTuc( (num_index+1)/2, num_index, state_array );

     ni = 0;
     do {
	  if ( info[ni].state_index == state_index 
	       && info[ni].state_id != state_id ) {
	       (void) snprintf( msg, SHORT_STRING_LENGTH,
				"correct state_id [DSR/state]: %-u/%02hhu", 
				ni, state_id );
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       info[ni].state_id = state_id;
	  }
     } while ( ++ni < num_info );
     free( state_array );
}

/*+++++++++++++++++++++++++
.IDENTifer   _CHECK_INFO_STATE_ID
.PURPOSE     consistency check of value of state_id in info-records
.INPUT/OUTPUT
  call as   _CHECK_INFO_STATE_ID( num_info, info );
     input:  
            insigned int num_info  :   number of info-records
 in/output:  
	    struct mds0_info *info :   info records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
void _CHECK_INFO_STATE_ID( unsigned int num_info, struct mds0_info *info )
     /*@modifies nadc_stat, nadc_err_stack, info->quality, info->state_id; @*/
     /*@globals  nadc_stat, nadc_err_stack@*/
{
     const char prognm[] = "_CHECK_INFO_STATE_ID";

     register unsigned int ni;

     unsigned char  state_id;
     unsigned short indx_max = 0;
     unsigned short indx = 0;
/*
 * handle special cases gracefully
 */
     if ( num_info < 2 ) return;
/*
 * obtain largest (valid) value for state_index
 */
     ni = num_info - 1;
     while ( ni > 0 && info[ni].state_index == USHRT_MAX ) ni--;
     indx_max = info[ni].state_index;
/*
 * check for unique state ID
 */
     for ( indx = 1; indx <= indx_max; indx++ ) {
	  ni = 0;
	  while ( ni < num_info && info[ni].state_index != indx ) ni++;
	  state_id = info[ni].state_id;

	  while ( ++ni < num_info ) {
	       if ( info[ni].state_index == indx ) {
		    if ( state_id != info[ni].state_id ) {
			 char msg[SHORT_STRING_LENGTH];

			 (void) snprintf( msg, SHORT_STRING_LENGTH,
				  "correct State ID [index/state]: %-u/%02hhu", 
					  indx, state_id );
			 NADC_ERROR( prognm, NADC_ERR_NONE, msg );

			 _SET_QFLAG_STATE( REPAIR_INFO_STATE_ID,
					   indx, num_info, info );
			 _REPAIR_INFO_STATE_ID( indx, num_info, info );
			 break;
		    }
	       }
	  }
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   _SORT_INFO_PACKET_ID
.PURPOSE     sort info-records on packet_id (DET, AUX, PMD) 
.INPUT/OUTPUT
  call as   num_info_det = _SORT_INFO_PACKET_ID( num_info, info );
     input:  
            unsigned int   num_info    :  number of info-records
 in/output:  
	    struct mds0_info *info     :   info records

.RETURNS     number of detector info-records, 
             error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static
unsigned int _SORT_INFO_PACKET_ID( unsigned int num_info, 
				   struct mds0_info *info )
     /*@modifies nadc_stat, nadc_err_stack, info; @*/
     /*@globals  nadc_stat, nadc_err_stack@*/
{
     const char prognm[] = "_SORT_INFO_PACKET_ID";

     register unsigned char packet_id;
     register unsigned int  ni;

     unsigned int num_info_aux   = 0;
     unsigned int num_info_det   = 0;
     unsigned int num_info_pmd   = 0;
     unsigned int num_info_undef = 0;

     struct mds0_info *info_tmp;

     if ( num_info == 0 ) return 0;

     ni = 0;
     do {
	  if ( info[ni].packet_type == SCIA_DET_PACKET )
	       num_info_det++;
	  else if ( info[ni].packet_type == SCIA_AUX_PACKET )
	       num_info_aux++;
	  else if ( info[ni].packet_type == SCIA_PMD_PACKET )
	       num_info_pmd++;
	  else
	       num_info_undef++;
     } while ( ++ni < num_info );
     if ( num_info_undef != 0 ) {
	  char msg[SHORT_STRING_LENGTH];

	  (void) snprintf( msg, SHORT_STRING_LENGTH,
			   "%-u DSRs with undefined packet ID found", 
			   num_info_undef );
	  NADC_ERROR( prognm, NADC_ERR_NONE, msg );
     }

     info_tmp = (struct mds0_info *) 
	  malloc( num_info * sizeof(struct mds0_info) );
     if ( info_tmp == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "info_tmp" );
     (void) memcpy( info_tmp, info, num_info * sizeof(struct mds0_info) );

     /* sort info-records */
     for ( packet_id = 1; packet_id <= 3; packet_id++ ) {
	  for ( ni = 0; ni < num_info; ni++ ) {
	       if ( info_tmp[ni].packet_type == packet_id ) {
		    (void) memcpy(info, info_tmp+ni, sizeof(struct mds0_info));
		    info++;
	       }
	  }
     }
     free( info_tmp );
done:
     return num_info_det;
}

/*+++++++++++++++++++++++++
.IDENTifer   _REPAIR_INFO_BCPS
.PURPOSE     sort info-records of a state with bcps monotonically increasing
             remove info-records with duplicate bcps values
.INPUT/OUTPUT
  call as   _REPAIR_INFO_BCPS( state_index, num_info, info );
     input:  
            unsigned short state_index :  state index
            unsigned int   num_info    :  number of info-records
 in/output:  
	    struct mds0_info *info     :   info records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static inline
void shellsort_bcps( unsigned int length, struct mds0_info *info )
{
     register unsigned int ii, jj, kol;

     struct mds0_info tmp;

     for ( kol = length/3; ; kol /= 3 ) {
	  if ( kol == 0 ) kol = 1;

	  for ( ii = kol; ii < length; ii++ ) {
	       jj = 0;

	       while( ii-jj >= kol 
		      && info[ii-jj].bcps < info[ii-kol-jj].bcps ) {
		    (void) memcpy( &tmp, &info[ii-jj], 
				   sizeof(struct mds0_info) );
		    (void) memcpy( &info[ii-jj], &info[ii-kol-jj],
				   sizeof(struct mds0_info) );
		    (void) memcpy( &info[ii-kol-jj], &tmp, 
				   sizeof(struct mds0_info) );
		    jj += kol;
	       }
	  }
	  if ( kol == 1 ) return;
     }
}

static inline
void _REPAIR_INFO_BCPS( unsigned short state_index,
			unsigned int num_info, struct mds0_info *info )
     /*@modifies info->quality, info->bcps, info->state_index @*/
{
     const char prognm[] = "_REPAIR_INFO_BCPS";

     register unsigned int ni;

     char msg[SHORT_STRING_LENGTH];

     unsigned short found_double = 0;

     unsigned int offs, bcps;
     unsigned int num_index;

     /* determine offset to info-records */
     ni = 0;
     while ( ni < num_info && info[ni].state_index != state_index ) ni++;
     offs = ni;

     /* determine number of info-records */
     while ( ni < num_info && info[ni].state_index == state_index ) ni++;
     num_index = ni - offs;

     /* sort info records according to bcps */
     shellsort_bcps( num_index, info+offs );

     /* identify info-records with non-unique bcps*/
     ni = 0;
     while ( ni < num_info && info[ni].state_index != state_index ) ni++;
     offs = ni;

     bcps = info[ni].bcps;
     while ( ++ni < num_info ) {
	  if ( info[ni].state_index != state_index ) break;
	  if ( bcps == info[ni].bcps ) {
	       found_double++;
	       info[ni].state_index = USHRT_MAX;
	  } else {
	       bcps = info[ni].bcps;
	  }
     }
     if ( found_double > 0 ) {
	  (void) snprintf( msg, SHORT_STRING_LENGTH,
	     "removed %-hu double DSR(s) from state [index/state]: %-u/%02hhu", 
			   found_double, state_index, info[offs].state_id );

	  _SET_QFLAG_STATE( (REPAIR_INFO_SORT_BCPS & REPAIR_INFO_UNIQUE),
			    state_index, num_info, info );
     } else {
	  (void) snprintf( msg, SHORT_STRING_LENGTH,
		          "sorted DSR(s) from state [index/state]: %-u/%02hhu", 
			   state_index, info[offs].state_id );

	  _SET_QFLAG_STATE( REPAIR_INFO_SORT_BCPS, 
			    state_index, num_info, info );
     }
     NADC_ERROR( prognm, NADC_ERR_NONE, msg );
}

/*+++++++++++++++++++++++++
.IDENTifer   _CHECK_INFO_BCPS
.PURPOSE     BCPS should be increasing monotonically
.INPUT/OUTPUT
  call as   _CHECK_INFO_BCPS( num_info, info );
     input:  
            insigned int num_info  :   number of info-records
 in/output:  
	    struct mds0_info *info :   info records

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void _CHECK_INFO_BCPS( unsigned int num_info, struct mds0_info *info )
     /*@modifies info->quality, info->bcps, info->state_index @*/
{
     register unsigned int ni;

     unsigned short indx_max = 0;
     unsigned short indx = 0;
/*
 * handle special cases gracefully
 */
     if ( num_info < 2 ) return;
/*
 * obtain largest (valid) value for state_index
 */
     ni = num_info - 1;
     while ( ni > 0 && info[ni].state_index == USHRT_MAX ) ni--;
     indx_max = info[ni].state_index;
/*
 * check if bcps is increasing monotonically
 */
     for ( indx = 1; indx <= indx_max; indx++ ) {
	  register unsigned short bcps = 0;

	  for ( ni = 0; ni < num_info; ni++ ) {
	       if ( info[ni].state_index == indx ) {
		    if ( bcps >= info[ni].bcps ) {
			 _REPAIR_INFO_BCPS( indx, num_info, info );
			 break;
		    }
		    bcps = info[ni].bcps;
	       }
	  }
     }
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
     unsigned short indx = 0;
     unsigned short bcps = 0;
     unsigned short duration = 0;
     unsigned short bcps_effective = 0;
     unsigned short size_ref;

     for ( ni = 0; ni < num_info; ni++, ++info_pntr ) {
//	  if ( info_pntr->packet_type != SCIA_DET_PACKET ) continue;
	  if ( info_pntr->state_index == USHRT_MAX ) continue; 

//	  (void) fprintf( stderr, "%6u %2hhu %5hu\n", 
//			  ni, info_pntr->state_id, absOrbit );
	  if ( ! CLUSDEF_MTBL_VALID( info_pntr->state_id, absOrbit ) ) 
	       continue;

	  if ( info_pntr->state_index != indx ) {
	       if ( state_id != 0 ) {
		    unsigned short num_det_ref = 
			 CLUSDEF_NUM_DET( state_id, absOrbit );

		    if ( bcps != duration ) {
			 (void) snprintf( msg, SHORT_STRING_LENGTH,
	            "too short state [DSR/state]: %-u/%02hhu; BCPS %hu != %hu", 
					  ni, state_id, bcps, duration );
			 NADC_ERROR( prognm, NADC_ERR_NONE, msg );
			 _SET_QFLAG_STATE( REPAIR_INFO_NUM_DSR,
					   indx, num_info, info );
		    } else if ( num_det != num_det_ref ) {
			 (void) snprintf( msg, SHORT_STRING_LENGTH,
	       "incomplete state [DSR/state]: %-u/%02hhu; num DSR: %hu != %hu", 
					  ni, state_id, num_det, num_det_ref );
			 NADC_ERROR( prognm, NADC_ERR_NONE, msg );
			 _SET_QFLAG_STATE( REPAIR_INFO_NUM_DSR,
					   indx, num_info, info );
		    } 
	       }
	       num_det = 1;
	       indx = info_pntr->state_index;
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
	  } else {
	       num_det++;
	  }
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
	       info_pntr->quality |= REPAIR_INFO_SIZE_DSR;
	  }
	  bcps = info_pntr->bcps;
     }

     /* check last state */
     if ( state_id != 0 ) {
	  unsigned short num_det_ref = CLUSDEF_NUM_DET( state_id, absOrbit );

	  if ( bcps != duration ) {
	       (void) snprintf( msg, SHORT_STRING_LENGTH,
		    "too short state [DSR/state]: %-u/%02hhu; BCPS %hu != %hu", 
				ni, state_id, bcps, duration );
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       _SET_QFLAG_STATE( REPAIR_INFO_NUM_DSR, indx, num_info, info );
	  } else if ( num_det != num_det_ref ) {
	       (void) snprintf( msg, SHORT_STRING_LENGTH,
	       "incomplete state [DSR/state]: %-u/%02hhu; num DSR: %hu != %hu", 
				ni, state_id, num_det, num_det_ref );
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       _SET_QFLAG_STATE( REPAIR_INFO_NUM_DSR, indx, num_info, info );
	  } 
     }
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int SCIA_LV0_RD_MDS_INFO( FILE *fd, unsigned int num_dsd,
				   const struct dsd_envi *dsd, 
				   struct mds0_info **info_out )
{
     const char prognm[] = "SCIA_LV0_RD_MDS_INFO";

     unsigned short absOrbit;
     unsigned int   indx_dsd;
     unsigned int   num_info, num_info_det;

     struct mph_envi  mph;
     struct mds0_info *info;

     const char dsd_name[] = "SCIAMACHY_SOURCE_PACKETS";

     char *env_str = getenv( "NO_INFO_CORRECTION" );

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
     if ( ! Use_Extern_Alloc ) {
	  info_out[0] = (struct mds0_info *) 
	       malloc( (size_t) num_info * sizeof(struct mds0_info) );
     }
     if ( (info = info_out[0]) == NULL ) {
	  NADC_ERROR( prognm, NADC_ERR_ALLOC, "mds0_info" );
	  return 0u;
     }

     /* extract info-records from input file */
     num_info = GET_SCIA_LV0_MDS_INFO( fd, &dsd[indx_dsd], info );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "GET_SCIA_LV0_MDS_INFO" );

     /* skip checks when environment variable is set */
     if ( env_str != NULL && *env_str == '1' ) goto done;

     /* set State counter */
     _CHECK_INFO_MJD( num_info, info );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "_CHECK_INFO_MJD" );
     _SET_INFO_STATEINDEX( num_info, info );
     _CHECK_INFO_STATE_ID( num_info, info );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "_CHECK_INFO_STATE_ID" );

     num_info_det = _SORT_INFO_PACKET_ID( num_info, info );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "_SORT_INFO_PACKET_ID" );

     /* perform quality check on detector info-records */
     _CHECK_INFO_BCPS( num_info_det, info );
     if ( CLUSDEF_DB_EXISTS() ) {
	  _SET_INFO_QUALITY( absOrbit, num_info_det, info );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "_SET_INFO_QUALITY" );
     }
done:
     return num_info;
}
