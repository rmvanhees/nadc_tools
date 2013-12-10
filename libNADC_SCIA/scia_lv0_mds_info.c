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
.VERSION      3.0   xx-Nov-2013	new implementation, complete rewrite, RvH
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
#define CMP_MJD(a,b) memcmp( &(a), &(b), sizeof(struct mjd_envi))

#define SCIA_DET_WRONG_DSR_SIZE  ((unsigned char) 0x1U)
#define SCIA_DET_WRONG_DSR_NUM   ((unsigned char) 0x2U)
#define SCIA_DET_CORRUPT_MJD     ((unsigned char) 0x4U)
#define SCIA_DET_WRONG_DURATION  ((unsigned char) 0x8U)

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   _FLAG_INCOMPLETE_STATE
.PURPOSE     identify MDS packages of the same state
.INPUT/OUTPUT
  call as   _FLAG_INCOMPLETE_STATE( num_info, info, indx );
     input:  
            unsigned int num_info  : number of info records
	    unsigned short indx    : index of incomplete state
 in/output:  
	    struct mds0_info *info : structure holding info about MDS records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static inline
void _FLAG_INCOMPLETE_STATE( unsigned int num_info, struct mds0_info *info,
			     unsigned short state_index )
{
     register unsigned int ni = 0;

     do {
	  if ( info[ni].state_index == state_index )
	       info[ni].quality |= SCIA_DET_WRONG_DSR_NUM;
     } while ( ++ni < num_info );
}

/*+++++++++++++++++++++++++
.IDENTifer   _SET_INFO_STATEINDEX
.PURPOSE     identify MDS packages of the same state
.INPUT/OUTPUT
  call as   _SET_INFO_STATEINDEX( num_info, info );
     input:  
            unsigned int num_info  : number of info records
 in/output:  
	    struct mds0_info *info : structure holding info about MDS records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static inline
void _SET_INFO_STATEINDEX( unsigned int num_info, struct mds0_info *info )
{
     const char prognm[] = "_SET_INFO_STATEINDEX";

     register unsigned int ni;
     register unsigned int ni_last;

     char msg[SHORT_STRING_LENGTH];

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
     ni_last = 0;
     do {
	  unsigned int ni_first = ni_last;

	  for ( ni = ni_first; ni < num_info; ni++ ) {
	       if ( info[ni].on_board_time == info[ni_first].on_board_time
		    || CMP_MJD( info[ni].mjd, info[ni_first].mjd ) == 0 ) {
		    info[ni].state_index = state_index;
		    ni_last = ni;
	       }
	  }

	  if ( ni_last == ni_first )         /* corrupted time tag !*/
	       info[ni_first].state_index = USHRT_MAX;
	  else
	       state_index++;

	  /* find next record with state_index equal to zero */
	  if ( ++ni_last < num_info && info[ni_last].state_index == 0 )
	       continue;

	  ni_last = 0;
	  while ( ++ni_last < num_info && info[ni_last].state_index != 0 );
     } while ( ni_last < num_info );
/*
 * repair State Counter 
 */
     for ( ni = 1; ni < (num_info-1); ni++ ) {
	  if ( info[ni].state_index == USHRT_MAX ) {
	       info[ni].quality |= SCIA_DET_CORRUPT_MJD;
	       (void) snprintf( msg, SHORT_STRING_LENGTH,
				"correct state_index record/state: %-u/%02hhu", 
				ni, info[ni].state_id );

	       if ( info[ni-1].state_id == info[ni+1].state_id ) {
		    info[ni].state_index = info[ni+1].state_index;
		    NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       }
	  }
     }
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
static inline
void _CHECK_INFO_STATE_ID( unsigned int num_info, struct mds0_info *info )
{
     const char prognm[] = "_CHECK_INFO_STATE_ID";

     register unsigned int ni;

     char msg[SHORT_STRING_LENGTH];

     unsigned char  state_id = 0;
     unsigned short indx = 0;
     size_t         num = 0;
     unsigned char  *state_array;
/*
 * handle special cases gracefully
 */
     if ( num_info < 2 ) return;

     state_array = (unsigned char *) malloc( num_info * sizeof(char) );
     
     for ( ni = 0; ni < num_info; ni++ ) {
	  if ( info[ni].state_index == USHRT_MAX ) continue;
	  if ( info[ni].state_index != indx ) {
	       if ( ni > 0u ) {
		    register unsigned short nj = ni - 1u;

		    state_id = SELECTuc( (num+1)/2, num, state_array );

		    while ( info[nj].state_index == indx ) {
			 if ( info[nj].state_id != state_id ) {
			      (void) snprintf( msg, SHORT_STRING_LENGTH,
				"correct state_id record/state: %-u/%02hhu", 
					       nj, state_id );
			      NADC_ERROR( prognm, NADC_ERR_NONE, msg );
			      info[nj].state_id = state_id;
			 }
			 if ( nj-- == 0 ) break;
		    }
	       }
	       indx = info[ni].state_index;
	       num = 0;
	  } 
	  state_array[num++] = info[ni].state_id;
     }
     free( state_array );
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
     unsigned short bcps_in_scan = 0;
     unsigned short size_ref;

     for ( ni = 0; ni < num_info; ni++, ++info_pntr ) {
	  if ( info_pntr->packet_type != SCIA_DET_PACKET ) continue;

//	  (void) fprintf( stderr, "%6u %2hhu %5hu\n", 
//			  ni, info_pntr->state_id, absOrbit );
	  if ( CLUSDEF_INVALID( info_pntr->state_id, absOrbit ) ) continue;

	  if ( info_pntr->state_index != indx ) {
	       if ( state_id != 0 ) {
		    unsigned short num_det_ref = 
			 CLUSDEF_NUM_DET( state_id, absOrbit );

		    if ( bcps != duration ) {
			 (void) snprintf( msg, SHORT_STRING_LENGTH,
	     "too short state record/state: %-u/%02hhu; BCPS %hu != %hu", 
					  ni, state_id, bcps, duration );
			 NADC_ERROR( prognm, NADC_ERR_NONE, msg );
			 _FLAG_INCOMPLETE_STATE( num_info, info, indx );
		    } else if ( num_det != num_det_ref ) {
			 (void) snprintf( msg, SHORT_STRING_LENGTH,
	     "incomplete state record/state: %-u/%02hhu; num DSR: %hu != %hu", 
					  ni, state_id, num_det, num_det_ref );
			 NADC_ERROR( prognm, NADC_ERR_NONE, msg );
			 _FLAG_INCOMPLETE_STATE( num_info, info, indx );
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
			     || (info_pntr->category == 11
				 && ((duration + 1) % 67) == 0)
			     || (info_pntr->category == 28
				 && ((duration + 1) % 67) == 0)
			     || (info_pntr->category == 29
				 && ((duration + 1) % 67) == 0)
			     || (info_pntr->category == 31 
				 && ((duration + 1) % 27) == 0) );
	       if ( limb_flag ) {
		    limb_scan = (((duration + 1) % 27) == 0) ? 27 : 67;
	       } else
		    limb_scan = 0;
	  } else {
	       num_det++;
	  }
	  if ( limb_flag ) {
	       bcps_in_scan = info_pntr->bcps 
		    - ((info_pntr->bcps / limb_scan) * limb_scan) - 2;
	  } else
	       bcps_in_scan = info_pntr->bcps;

//	  if ( state_id == 28 )
//	       (void) fprintf( stderr, "%5u: %3hhu %3hhu %4hu %4hu\n", ni, 
//			       info_pntr->state_id, info_pntr->category, 
//			       bcps_in_scan, info_pntr->bcps );

	  /* perform check on size of data-package */
	  size_ref = CLUSDEF_DSR_SIZE( state_id, absOrbit, bcps_in_scan );
	  if ( info_pntr->packet_length != size_ref ) {
	       (void) snprintf( msg, SHORT_STRING_LENGTH,
		"wrong DSR size record/state: %-u/%02hhu; size DSR %hu != %hu", 
			     ni, state_id, info_pntr->packet_length, size_ref );
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       info_pntr->quality |= SCIA_DET_WRONG_DSR_SIZE;
	  }
	  bcps = info_pntr->bcps;
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
     unsigned int   num_info;

     struct mph_envi  mph;
     struct mds0_info *info;

     const char dsd_name[] = "SCIAMACHY_SOURCE_PACKETS";
/*
 * read Main Product Header
 */
     ENVI_RD_MPH( fd, &mph );
     absOrbit = (unsigned short) mph.abs_orbit;
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_ABSENT ) {
	  NADC_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
	  return 0u;
     }
/*
 * allocate memory to store info-records
 */
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

     /* set State counter */
     _SET_INFO_STATEINDEX( num_info, info );
     _CHECK_INFO_STATE_ID( num_info, info );

     /* perform quality check on info-records */
     _SET_INFO_QUALITY( absOrbit, num_info, info );

     return num_info;
}
