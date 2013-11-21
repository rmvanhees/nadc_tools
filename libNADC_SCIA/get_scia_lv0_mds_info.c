/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2009 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GET_SCIA_LV0_MDS_INFO
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 0
.LANGUAGE    ANSI C
.PURPOSE     query the contents a SCIAMACHY level 0 product
.INPUT/OUTPUT
  call as   nr_info = GET_SCIA_LV0_MDS_INFO( fd, mph, dsd, &info );
     input:  
            FILE   *fd             : (open) stream pointer
	    struct mph_envi mph    : Envisat main product header
            struct dsd_envi *dsd   : structure for the DSD records
    output:  
            struct mds0_info *info : structure holding info about MDS records

.RETURNS     number of MDS's read (unsigned int), 
             error status passed by global variable ``nadc_stat'
.COMMENTS    None
.ENVIRONment None
.EXTERNALs   ENVI_GET_DSD_INDEX 
.VERSION     3.0     28-OCT-2013   re-write, no cluster-info, RvH
             2.0     13-NOV-2012   added cluster checking, RvH
             1.1     16-FEB-2010   added stricter validity checking, RvH
             1.0     28-OCT-2009   initial release by R. M. van Hees
                                   rewrite of GET_LV0_MDS_INFO
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
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>
#endif

/*+++++ Macros +++++*/
#define CMP_MJD(a,b) memcmp( &(a), &(b), sizeof(struct mjd_envi))

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
static struct mjd_envi mjd_first, mjd_last;

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static inline
bool MJD_BEFORE_SENSING_START( const struct mjd_envi mjd )
{
     if ( mjd.days < mjd_first.days ) return TRUE;
     if ( mjd.days == mjd_first.days ) {
	  if ( mjd.secnd < mjd_first.secnd ) return TRUE;
	  if ( mjd.secnd == mjd_first.secnd 
	       && mjd.musec < mjd_first.musec ) return TRUE;
     }
     return FALSE;
}
 
static inline
bool MJD_AFTER_SENSING_STOP( const struct mjd_envi mjd )
{
     if ( mjd.days > mjd_last.days ) return TRUE;
     if ( mjd.days == mjd_last.days ) {
	  if ( mjd.secnd > mjd_last.secnd ) return TRUE;
	  if ( mjd.secnd == mjd_last.secnd 
	       && mjd.musec > mjd_last.musec ) return TRUE;
     }
     return FALSE;
}
 
/*+++++++++++++++++++++++++
.IDENTifer   GET_INFO_STATEINDEX
.PURPOSE     identify MDS packages of the same state
.INPUT/OUTPUT
  call as   GET_INFO_STATEINDEX( num_info, info );
     input:  
            unsigned int num_info  : number of info records
 in/output:  
	    struct mds0_info *info : structure holding info about MDS records

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    static function
-------------------------*/
static inline
void GET_INFO_STATEINDEX( unsigned int num_info, struct mds0_info *info )
{
     const char prognm[] = "GET_INFO_STATEINDEX";

     register unsigned int ni;
     register unsigned int ni_last = 0;

     unsigned short stateIndex = 1;

     struct mjd_envi mjd_current;
/*
 * handle special cases gracefully
 */
     if ( num_info == 0 ) return;
     if ( num_info == 1 ) {
	  info[0].stateIndex = 1;
	  return;
     }
/*
 * repair corrupted mjd-records
 */
     for ( ni = 0; ni < num_info; ni++ ) {
	  if ( info[ni].mjd.days == 0 ) {
	       char msg[SHORT_STRING_LENGTH];

	       if ( ni > 0 )
		    (void) memcpy( &info[ni].mjd, &info[ni-1].mjd,
				   sizeof( struct mjd_envi ) );
	       else
		    (void) memcpy( &info[ni].mjd, &info[ni+1].mjd,
				   sizeof( struct mjd_envi ) );

	       (void) snprintf( msg, SHORT_STRING_LENGTH,
				"correct MJD of info record: %-u", ni );
	  }
     }
/*
 * collect all readouts belonging to one state
 */
     do {
	  register unsigned int ni_first = ni_last;

	  (void) memcpy( &mjd_current, &info[ni_last].mjd, 
			 sizeof(struct mjd_envi));
	  for ( ni = ni_last; ni < num_info; ni++ ) {
	       if ( CMP_MJD( info[ni].mjd, mjd_current ) == 0 ) {
		    info[ni].stateIndex = stateIndex;
		    ni_last = ni;
	       }
	  }
	  if ( ni_last - ni_first == 1u )         /* corrupted time tag !*/
	       info[ni_first].stateIndex = USHRT_MAX;
	  else
	       stateIndex++;

	  /* find next record with stateIndex equal to zero */
	  if ( ++ni_last < num_info && info[ni_last].stateIndex == 0 )
	       continue;

	  ni_last = 0;
	  while ( ++ni_last < num_info && info[ni_last].stateIndex != 0 );
     } while ( ni_last < num_info );
/*
 * repair state IDs
 */
     for ( ni = 0; ni < num_info; ni++ ) {
	  if ( info[ni].stateIndex == USHRT_MAX ) {
	       char msg[SHORT_STRING_LENGTH];

	       (void) snprintf( msg, SHORT_STRING_LENGTH,
				"correct stateID of info record: %-u", ni );
	       NADC_ERROR( prognm, NADC_ERR_NONE, msg );
	       if ( ni == 0 && info[ni].stateID == info[ni+1].stateID )
		    info[ni].stateIndex = info[ni+1].stateIndex;
	       else if ( ni == num_info-1 
			 && info[ni].stateID == info[ni-1].stateID )
		    info[ni].stateIndex = info[ni-1].stateIndex;
	       else if ( info[ni-1].stateID == info[ni+1].stateID )
		    info[ni].stateIndex = info[ni+1].stateIndex;
	  }
     }
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned int GET_SCIA_LV0_MDS_INFO( FILE *fd, struct mph_envi mph,
				    const struct dsd_envi *dsd, 
				    struct mds0_info *info_out )
{
     const char prognm[] = "GET_SCIA_LV0_MDS_INFO";

     register char *cpntr;
     register struct mds0_info *info = info_out;

     char           *mds_start, *mds_char = NULL;
     unsigned int   num_info = 0;

     struct lv0_hdr_rec {
	  struct mjd_envi isp;
	  struct mjd_envi mjd;                 /* FEP elements */
	  unsigned short isp_length;
	  unsigned short num_crc_error;
	  unsigned short num_rs_error;
	  unsigned short spare;
	  unsigned short packet_id;            /* packet header */
	  unsigned short packet_control;
	  unsigned short packet_length;
	  unsigned short data_length;          /* data-field header */
	  unsigned char  category;
	  unsigned char  state_id;
	  unsigned int   on_board_time;
	  unsigned short rdv;
	  unsigned char  packet_type;
	  unsigned char  overflow;
     } lv0_hdr;


     /* set range for the start time of the data packages */
     ASCII_2_MJD( mph.sensing_start, 
		  &mjd_first.days, &mjd_first.secnd, &mjd_first.musec );
     ASCII_2_MJD( mph.sensing_stop, 
		  &mjd_last.days, &mjd_last.secnd, &mjd_last.musec );

     /* allocate memory to buffer source packages */
     if ( (mds_char = (char *) malloc( (size_t) dsd->size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mds_char" );

     /* read all Sciamachy source packages in product */
     (void) fseek( fd, (long) dsd->offset, SEEK_SET );
     if ( fread( mds_char, (size_t) dsd->size, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd->name );

     /* examine whole source data section */
     cpntr = mds_char;
     do {
	  mds_start = cpntr;
	  (void) memcpy( &lv0_hdr, cpntr, sizeof( struct lv0_hdr_rec ) );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  lv0_hdr.isp.days       = byte_swap_32( lv0_hdr.isp.days );
	  lv0_hdr.isp.secnd      = byte_swap_u32( lv0_hdr.isp.secnd );
	  lv0_hdr.isp.musec      = byte_swap_u32( lv0_hdr.isp.musec );
	  lv0_hdr.mjd.days       = byte_swap_32( lv0_hdr.mjd.days );
	  lv0_hdr.mjd.secnd      = byte_swap_u32( lv0_hdr.mjd.secnd );
	  lv0_hdr.mjd.musec      = byte_swap_u32( lv0_hdr.mjd.musec );
	  lv0_hdr.isp_length     = byte_swap_u16( lv0_hdr.isp_length );
	  lv0_hdr.num_crc_error  = byte_swap_u16( lv0_hdr.num_crc_error );
	  lv0_hdr.num_rs_error   = byte_swap_u16( lv0_hdr.num_rs_error );
	  lv0_hdr.packet_id      = byte_swap_u16( lv0_hdr.packet_id );
	  lv0_hdr.packet_control = byte_swap_u16( lv0_hdr.packet_control );
	  lv0_hdr.data_length    = byte_swap_u16( lv0_hdr.data_length );
	  lv0_hdr.on_board_time  = byte_swap_u32( lv0_hdr.on_board_time );
	  lv0_hdr.rdv            = byte_swap_u16( lv0_hdr.rdv );
#endif
	  (void) memcpy( &info->mjd, &lv0_hdr.isp, sizeof(struct mjd_envi) );
	  info->packetID   = lv0_hdr.packet_id >> 4;
	  info->category   = lv0_hdr.category;
	  info->stateID    = lv0_hdr.state_id;
	  info->sizeCheck  = 255;
	  info->crc_errors = lv0_hdr.num_crc_error;
	  info->rs_errors  = lv0_hdr.num_rs_error;

	  /* store byte offset w.r.t. begin of file */
	  info->offset = (unsigned int) (dsd->offset + (mds_start - mds_char));

	  /* move to the next MDS */
	  cpntr += lv0_hdr.isp_length - 11;

	  info++;
     } while ( ++num_info < dsd->num_dsr );

     GET_INFO_STATEINDEX( num_info, info );
done:
     if ( mds_char != NULL ) free( mds_char );
     return num_info;
}
