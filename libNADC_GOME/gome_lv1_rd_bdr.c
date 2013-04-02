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

.IDENTifer   GOME_LV1_RD_BDR
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1b
.LANGUAGE    ANSI C
.PURPOSE     read data of a BCR including sub-channel data (for split-channels)
.INPUT/OUTPUT
  call as   nr_rec = GOME_LV1_RD_BDR( infl, nband, fsr, fcd, &rec_out );

     input:  
	    FILE  *infl           : (open) file descriptor
            short nband           : number of the spectral band [1a=0,1b,2a..]
	    struct fsr1_gome *fsr : structure for the FSR record
	    struct fcd_gome *fcd  : Fixed Calibration Data Record
    output: 
            struct rec_gome *rec  : structure for a spectral band data record

.RETURNS     nothing
.COMMENTS    None
.ENVIRONment None
.VERSION      3.2   06-Mar-2003 use global "Use_Extern_Alloc", RvH
              3.1   20-Nov-2001	read sz_band[], not nr_band[] (line 137), RvH
              3.1   20-Nov-2001	check number of records to be written, RvH
              3.0   11-Nov-2001	moved to the new Error handling routines, RvH 
              2.0   08-Aug-2001	new name convention, include READ_REC, RvH 
              1.0   24-JUL-2001 created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>
#endif

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   GOME_LV1_RD_REC
.PURPOSE     Read a Spectral Band Record
.INPUT/OUTPUT
  call as    num = GOME_LV1_RD_REC( infl, nband, bcr_start, bcr_count, &rec );

     input:
            FILE   *infl          : (open) file descriptor
            short  nband          : number of the spectral band [1a=0,1b,2a..]
	    short  bcr_start      : first pixel of spectral band
	    short  bcr_count      : number of pixels per spectral band
    output:  
            struct rec_gome *rec  : structure for a REC

.RETURNS     number of bytes read
.COMMENTS    static function
-------------------------*/
static
short GOME_LV1_RD_REC( FILE *infl, short nband, 
		       const struct fsr1_gome *fsr,
		       const struct fcd_gome *fcd, 
                       /*@out@*/struct rec_gome **rec_out )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, infl, *rec_out@*/
{
     const char  prognm[] = "GOME_LV1_RD_REC";

     register short nb, ni;

     char           *rec_char = NULL;
     char           *rec_pntr;
     unsigned short usbuff, raw[CHANNEL_SIZE];
     short          nr_rec = 0;
     long           byte_offs;

     struct rec_gome *rec;

     const short bcr_start = fcd->bcr[nband].start;
     const short bcr_count = 
	  (short) (fcd->bcr[nband].end - fcd->bcr[nband].start + 1);
     const float CountDuration = 0.09375f;
/*
 * check number of Band records
 */
     if ( fsr->nr_band[nband] == 0 ) {
	  rec_out[0] = NULL;
	  return 0;
     }
/*
 * allocate memory for the Spectral Band Records
 */
     rec_out[0] = (struct rec_gome *)
	  malloc( fsr->nr_band[nband] * sizeof( struct rec_gome ) );
     if ( (rec = rec_out[0]) == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rec" );
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (rec_char = (char *) malloc((size_t) fsr->sz_band[nband] )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rec_char" );
/*
 * rewind/read input data file
 */
     byte_offs = (long) (LVL1_PIR_LENGTH + LVL1_FSR_LENGTH 
			 + fsr->sz_sph + fsr->sz_fcd
			 + fsr->nr_pcd * fsr->sz_pcd
			 + fsr->nr_scd * fsr->sz_scd
			 + fsr->nr_mcd * fsr->sz_mcd);
     for ( nb = 0; nb < nband; nb++ )
	  byte_offs += (long) (fsr->nr_band[nb] * fsr->sz_band[nb]);

     (void) fseek( infl, byte_offs, SEEK_SET );
/*
 * read data buffer to REC structure
 */
     do {
	  rec_pntr = rec_char;
	  if ( fread( rec_pntr, (size_t) fsr->sz_band[nband], 1, infl ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
/*
 * (1) quality flags
 */
	  (void) memcpy( &rec->quality.two_byte, rec_pntr, GOME_SHORT );
	  rec_pntr += GOME_SHORT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  rec->quality.two_byte = byte_swap_u16( rec->quality.two_byte );
#endif
/*
 * (2) index to Polarisations Sensitivity Parameters
 */
	  (void) memcpy( &rec->indx_psp, rec_pntr, GOME_SHORT );
	  rec_pntr += GOME_SHORT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  rec->indx_psp = byte_swap_16( rec->indx_psp );
#endif
/*
 * (3) index to Pixel Specific Calibration Parameters Record (PCD)
 */
	  (void) memcpy( &rec->indx_pcd, rec_pntr, GOME_SHORT );
	  rec_pntr += GOME_SHORT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  rec->indx_pcd = byte_swap_16( rec->indx_pcd );
#endif
/*
 * (4) integration time (one count corresponds to 93.5 ms)
 */
	  (void) memcpy( &usbuff, rec_pntr, GOME_USHRT );
	  rec_pntr += GOME_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  usbuff = byte_swap_u16( usbuff );
#endif
	  rec->integration[0] = CountDuration * usbuff;
/*
 * (5) array data values
 */
	  (void) memcpy( raw, rec_pntr, bcr_count * GOME_USHRT );
	  ni = 0;
	  do {
#ifdef _SWAP_TO_LITTLE_ENDIAN
	       rec->data[ni+bcr_start] = (float) byte_swap_u16( raw[ni] );
#else
	       rec->data[ni+bcr_start] = (float) raw[ni];
#endif
	  } while ( ++ni < bcr_count );
	  rec_pntr += bcr_count * GOME_USHRT;
/*
 * check if we read the whole DSR
 */
	  if ( rec_pntr - rec_char != fsr->sz_band[nband] )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, "REC size" );
/*
 * go to next record
 */
	  rec++;
     } while ( ++nr_rec < fsr->nr_band[nband] );
 done:
     if ( rec_char != NULL ) free( rec_char );
/*
 * set return value
 */
     return nr_rec;
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
short GOME_LV1_RD_BDR( FILE *infl, short nband, 
		       const struct fsr1_gome *fsr,
		       const struct fcd_gome *fcd, 
		       struct rec_gome **rec_out )
{
     const char prognm[]  = "GOME_LV1_RD_BDR";

     register short nr;

     short nr_rec = 0;

     struct rec_gome *rec = NULL;

     const short bcr_channel = (short) (fcd->bcr[nband].array_nr - 1);
/*
 * Two possible requests:
 *  1) requested is data of spectral bands 3, 4
 *     -> simply read the data and return
 *  2) requested is data of split-channels 1 and 2
 *     -> read data from of both subchannels and combine
 */
     if ( BandChannel(bcr_channel, 1) == -1 
	  || nband == BLIND_1a || nband == STRAY_1a
	  || nband == STRAY_1b || nband == STRAY_2a ) {
	  nr_rec = GOME_LV1_RD_REC( infl, nband, fsr, fcd, &rec );
	  if ( rec == NULL || nr_rec != fsr->nr_band[nband] ) {
	       NADC_ERROR( prognm, NADC_ERR_PDS_SIZE, "REC" );
	       if ( rec != NULL ) free( rec );
	       return 0;
	  }
	  for ( nr = 0; nr < nr_rec; nr++ ) 
	       rec[nr].integration[1] = -1.f;
/*
 * +++ request for band 1a/2a
 */
     } else if ( BandChannel(bcr_channel, 0) == nband ) {
	  register short nb;

	  short nr_recB;
	  struct rec_gome *recB = NULL;

	  const short nbandB = BandChannel(bcr_channel, 1);
	  const short bcr_startB = fcd->bcr[nbandB].start;
	  const short bcr_countB = 
	       (short) (fcd->bcr[nbandB].end - fcd->bcr[nbandB].start + 1);
	  const size_t nbyteB = bcr_countB * sizeof( float );

/*
 * read data of the first part of the split-channel
 */
	  nr_rec = GOME_LV1_RD_REC( infl, nband, fsr, fcd, &rec );
	  if ( rec == NULL || nr_rec != fsr->nr_band[nband] ) {
	       NADC_ERROR( prognm, NADC_ERR_PDS_SIZE, "rec(A)" );
	       if ( rec != NULL ) free( rec );
	       return 0;
	  }
/*
 * read data of the second part of the split-channel
 */
	  nr_recB = GOME_LV1_RD_REC( infl, nbandB, fsr, fcd, &recB );
	  if ( recB == NULL || nr_recB != fsr->nr_band[nbandB] ) {
	       NADC_ERROR( prognm, NADC_ERR_PDS_SIZE, "rec(B)" );
	       free( rec );
	       if ( recB != NULL ) free( recB );
	       return 0;
	  }
/*
 * combine band data in one record
 */
	  if ( nr_recB > 0 ) {
	       for ( nb = nr = 0; nr < nr_rec; nr++ ) {
		    rec[nr].integration[1] = -1.f;
		    while ( nb < nr_recB 
			    && rec[nr].indx_pcd > recB[nb].indx_pcd )
			 nb++;

		    if ( nb < nr_recB 
			 && rec[nr].indx_pcd == recB[nb].indx_pcd ) {
			 rec[nr].integration[1] = recB[nb].integration[0];
			 (void) memcpy( &rec[nr].data[bcr_startB], 
					&recB[nb].data[bcr_startB], nbyteB );
			 nb++;
		    }
	       }
	       free( recB );
	  }
/*
 * +++ request for band 1b/2b
 */
     } else {
	  register short na;

	  short  nr_recA;
	  struct rec_gome *recA = NULL;

	  const short nbandA = BandChannel(bcr_channel, 0);
	  const short  bcr_startA = fcd->bcr[nbandA].start;
	  const short  bcr_countA = 
	       (short) (fcd->bcr[nbandA].end - fcd->bcr[nbandA].start + 1);
          const size_t nbyteA = bcr_countA * sizeof( float );
/*
 * read data of the first part of the split-channel
 */
	  nr_recA = GOME_LV1_RD_REC( infl, nbandA, fsr, fcd, &recA );
	  if ( recA == NULL || nr_recA != fsr->nr_band[nbandA] ) {
	       NADC_ERROR( prognm, NADC_ERR_PDS_SIZE, "rec(A)" );
	       if ( recA != NULL ) free( recA );
	       return 0;
	  }
/*
 * read data of the second part of the split-channel
 */
	  nr_rec = GOME_LV1_RD_REC( infl, nband, fsr, fcd, &rec );
	  if ( rec == NULL || nr_rec != fsr->nr_band[nband] ) {
	       NADC_ERROR( prognm, NADC_ERR_PDS_SIZE, "rec(B)" );
	       free( recA );
	       if ( rec != NULL ) free( rec );
	       return 0;
	  }
/*
 * combine band data in one record
 */
	  if ( nr_recA > 0 ) {
	       for ( na = nr = 0; nr < nr_rec; nr++ ) {
		    rec[nr].integration[1] = rec[nr].integration[0];
		    rec[nr].integration[0] = -1.f;

		    while ( na < nr_recA 
			    && rec[nr].indx_pcd > recA[na].indx_pcd )
			 na++;

		    if ( na < nr_recA 
			 && rec[nr].indx_pcd == recA[na].indx_pcd ) {
			 rec[nr].integration[0] = recA[na].integration[0];
			 (void) memcpy( &rec[nr].data[bcr_startA], 
					&recA[na].data[bcr_startA], nbyteA );
			 na++;
		    }
	       }
	       free( recA );
	  }
     }

     if ( ! Use_Extern_Alloc ) {
	  rec_out[0] = rec;
     } else {
	  (void) memmove( rec_out[0], rec, nr_rec * sizeof( struct rec_gome ));
	  free( rec );
     }
     return nr_rec;
}

