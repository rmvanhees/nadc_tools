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

.IDENTifer   SCIA_PDS_LADS
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Geolocation of the state records
.COMMENTS    contains SCIA_RD_LADS and SCIA_WR_LADS
.ENVIRONment None
.EXTERNALs   ENVI_GET_DSD_INDEX 
.VERSION      3.1   11-Oct-2005 add usage of SCIA_LV1_ADD_DSD; 
                                removed check on number of written bytes, RvH
              3.0   20-Apr-2005 added routine to write LADS-struct to file, RvH
              2.1   16-Jan-2002	use of global Use_Extern_Alloc, RvH
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.2   10-Oct-2001	added list of external (NADC) function, RvH 
              1.1   04-Oct-2001 changed input/output, RvH
              1.0   09-Nov-1999 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_LADS( struct lads_scia *lads )
{
     register int ni;

     lads->mjd.days = byte_swap_32( lads->mjd.days );
     lads->mjd.secnd = byte_swap_u32( lads->mjd.secnd );
     lads->mjd.musec = byte_swap_u32( lads->mjd.musec );
     for ( ni = 0; ni < NUM_CORNERS; ni++ ) {
	  lads->corner[ni].lat = byte_swap_32( lads->corner[ni].lat );
	  lads->corner[ni].lon = byte_swap_32( lads->corner[ni].lon );
     }
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_RD_LADS
.PURPOSE     read Geolocation of the state records
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_RD_LADS( fd, num_dsd, dsd, &lads );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi dsd   :   structure for the DSDs
    output:
            struct lads_scia **lads :  geolocation of states

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_RD_LADS( FILE *fd, unsigned int num_dsd, 
			   const struct dsd_envi *dsd,
			   struct lads_scia **lads_out )
{
     register int ni;

     char         *lads_pntr, *lads_char = NULL;
     size_t       dsr_size;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct lads_scia  *lads;

     const char dsd_name_lv1[] = "GEOLOCATION";
     const char dsd_name_lv2[] = "STATE_GEOLOCATION";
/*
 * get index to data set descriptor
 */
     NADC_ERR_SAVE();
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name_lv2 );
     if ( IS_ERR_STAT_ABSENT ) {
	  NADC_ERR_RESTORE();
          indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name_lv1 );
	  if ( IS_ERR_STAT_ABSENT )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, dsd_name_lv1 );
     }
     if ( dsd[indx_dsd].num_dsr == 0 ) return 0u;
     if ( ! Use_Extern_Alloc ) {
	  lads_out[0] = (struct lads_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct lads_scia));
     }
     if ( (lads = lads_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "lads" );
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (lads_char = (char *) malloc( dsr_size )) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "lads_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( lads_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to LADS structure
 */
	  (void) memcpy( &lads->mjd.days, lads_char, ENVI_INT );
	  lads_pntr = lads_char + ENVI_INT;
	  (void) memcpy( &lads->mjd.secnd, lads_pntr, ENVI_UINT );
	  lads_pntr += ENVI_UINT;
	  (void) memcpy( &lads->mjd.musec, lads_pntr, ENVI_UINT );
	  lads_pntr += ENVI_UINT;
	  (void) memcpy( &lads->flag_mds, lads_pntr, ENVI_UCHAR );
	  lads_pntr += ENVI_UCHAR;
	  for ( ni = 0; ni < NUM_CORNERS; ni++ ) {
	       (void) memcpy( &lads->corner[ni].lat, lads_pntr, ENVI_INT );
	       lads_pntr += ENVI_INT;
	       (void) memcpy( &lads->corner[ni].lon, lads_pntr, ENVI_INT );
	       lads_pntr += ENVI_INT;
	  }
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(lads_pntr - lads_char) != dsr_size )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, dsd_name_lv1 );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_LADS( lads );
#endif
	  lads++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
     lads_pntr = NULL;
/*
 * set return values
 */
 done:
     if ( lads_char != NULL ) free( lads_char );

     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_LADS
.PURPOSE     write Geolocation of the state records
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_LADS( fd, num_lads, &lads );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_lads :   number of LADS records
            struct lads_scia *lads :  geolocation of states records

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_LADS( FILE *fd, unsigned int num_lads, 
		       const struct lads_scia *lads_in )
{
     register int ni;

     struct lads_scia  lads;

     struct dsd_envi dsd = {
          "GEOLOCATION", "A",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_lads == 0u ) return;
/*
 * write data set records
 */
     do {
	  (void) memcpy( &lads, lads_in, sizeof( struct lads_scia ));
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_LADS( &lads );
#endif
/*
 * write LADS structure to data buffer
 */
	  if ( fwrite( &lads.mjd.days, ENVI_INT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_INT;
	  if ( fwrite( &lads.mjd.secnd, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &lads.mjd.musec, ENVI_UINT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UINT;
	  if ( fwrite( &lads.flag_mds, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_UCHAR;
	  for ( ni = 0; ni < NUM_CORNERS; ni++ ) {
	       if ( fwrite( &lads.corner[ni].lat, ENVI_INT, 1, fd ) != 1 )
		    NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	       dsd.size += ENVI_INT;
	       if ( fwrite( &lads.corner[ni].lon, ENVI_INT, 1, fd ) != 1 )
		    NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
	       dsd.size += ENVI_INT;
	  }
	  lads_in++;
     } while ( ++dsd.num_dsr < num_lads );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}
