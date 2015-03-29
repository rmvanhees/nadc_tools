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

.IDENTifer   SCIA_OL2_RD_NFIT
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA offline level 2 data
.LANGUAGE    ANSI C
.PURPOSE     read Nadir Fitting Window Application Data sets
.INPUT/OUTPUT
  call as   nbyte = SCIA_OL2_RD_NFIT( fd, num_dsd, dsd, &nfit );
     input:
            FILE *fd                :   stream pointer
	    unsigned int num_dsd    :   number of DSDs
	    struct dsd_envi *dsd    :   structure for the DSDs
    output:
            struct nfit_scia **nfit :   Nadir Fitting Window Data sets

.RETURNS     number of data set records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      1.2   21-Jan-2003 added check for presence of this dataset, RvH
              1.1   16-Aug-2002	updated to ENV-ID-DLR-SCI-2200-4, RvH
              1.0   16-May-2002	created by R. M. van Hees 
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
#define _SCIA_LEVEL_2
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#ifdef _SWAP_TO_LITTLE_ENDIAN
#include <swap_bytes.h>

static
void Sun2Intel_NFIT( struct nfit_scia *nfit )
{
     register unsigned short np;

     size_t n_corr;

     nfit->mjd.days = byte_swap_32( nfit->mjd.days );
     nfit->mjd.secnd = byte_swap_u32( nfit->mjd.secnd );
     nfit->mjd.musec = byte_swap_u32( nfit->mjd.musec );
     nfit->dsrlen = byte_swap_u32( nfit->dsrlen );
     nfit->intg_time = byte_swap_u16( nfit->intg_time );
     for ( np = 0; np < nfit->numvcd; np++ ) {
	  IEEE_Swap__FLT( &nfit->vcd[np] );
	  IEEE_Swap__FLT( &nfit->errvcd[np] );
     }
     nfit->vcdflag = byte_swap_u16( nfit->vcdflag );
     IEEE_Swap__FLT( &nfit->esc );
     IEEE_Swap__FLT( &nfit->erresc );
     for ( np = 0; np < nfit->num_fitp; np++ ) {
	  IEEE_Swap__FLT( &nfit->linpars[np] );
	  IEEE_Swap__FLT( &nfit->errlinpars[np] );
     }
     n_corr = (size_t) (nfit->num_fitp * (nfit->num_fitp-1)) / 2;
     for ( np = 0; np < n_corr; np++ )
	  IEEE_Swap__FLT( &nfit->lincorrm[np] );
     for ( np = 0; np < nfit->num_nfitp; np++ ) {
	  IEEE_Swap__FLT( &nfit->nlinpars[np] );
	  IEEE_Swap__FLT( &nfit->errnlinpars[np] );
     }
     n_corr = (size_t) (nfit->num_nfitp * (nfit->num_nfitp-1)) / 2;
     for ( np = 0; np < n_corr; np++ )
	  IEEE_Swap__FLT( &nfit->nlincorrm[np] );
     IEEE_Swap__FLT( &nfit->rms );
     IEEE_Swap__FLT( &nfit->chi2 );
     IEEE_Swap__FLT( &nfit->goodness );
     nfit->numiter = byte_swap_u16( nfit->numiter );
     nfit->fitflag = byte_swap_u16( nfit->fitflag );
     IEEE_Swap__FLT( &nfit->amfgrd );
     IEEE_Swap__FLT( &nfit->erramfgrd );
     IEEE_Swap__FLT( &nfit->amfcld );
     IEEE_Swap__FLT( &nfit->erramfcld );
     nfit->amfflag = byte_swap_u16( nfit->amfflag );
     IEEE_Swap__FLT( &nfit->temperature );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned 
int SCIA_OL2_RD_NFIT( FILE *fd, const char nfit_name[],
		      unsigned int num_dsd, const struct dsd_envi *dsd, 
		      struct nfit_scia **nfit_out )
{
     char         *nfit_pntr, *nfit_char = NULL;
     size_t       dsd_size, n_corr;
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct nfit_scia *nfit;
/*
 * get index to data set descriptor
 */
     NADC_ERR_SAVE();
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, nfit_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, nfit_name );
     if ( IS_ERR_STAT_ABSENT || dsd[indx_dsd].num_dsr == 0 ) {
          NADC_ERR_RESTORE();
          nfit_out[0] = NULL;
          return 0u;
     }
     dsd_size = (size_t) dsd[indx_dsd].size;
/*
 * allocate memory to store output structures
 */
     if ( ! Use_Extern_Alloc ) {
	  nfit_out[0] = (struct nfit_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct nfit_scia));
     }
     if ( (nfit = nfit_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "nfit" );
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (nfit_char = (char *) malloc( dsd_size )) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "nfit_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
     if ( fread( nfit_char, dsd_size, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to NFIT structure
 */
     nfit_pntr = nfit_char;
     do { 
	  (void) memcpy( &nfit->mjd.days, nfit_pntr, ENVI_INT );
	  nfit_pntr += ENVI_INT;
	  (void) memcpy( &nfit->mjd.secnd, nfit_pntr, ENVI_UINT );
	  nfit_pntr += ENVI_UINT;
	  (void) memcpy( &nfit->mjd.musec, nfit_pntr, ENVI_UINT );
	  nfit_pntr += ENVI_UINT;
	  (void) memcpy( &nfit->dsrlen, nfit_pntr, ENVI_UINT );
	  nfit_pntr += ENVI_UINT;
	  (void) memcpy( &nfit->quality, nfit_pntr, ENVI_CHAR );
	  nfit_pntr += ENVI_CHAR;
	  (void) memcpy( &nfit->intg_time, nfit_pntr, ENVI_USHRT );
	  nfit_pntr += ENVI_USHRT;
	  (void) memcpy( &nfit->numvcd, nfit_pntr, ENVI_USHRT );
	  nfit_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  nfit->numvcd = byte_swap_u16( nfit->numvcd );
#endif
	  if ( nfit->numvcd > 0u ) {
	       nfit->vcd = (float *) malloc( nfit->numvcd * sizeof(float) );
	       if ( nfit->vcd == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "vcd" );
	       (void) memcpy( nfit->vcd, nfit_pntr, 
			      nfit->numvcd * ENVI_FLOAT );
	       nfit_pntr += nfit->numvcd * ENVI_FLOAT;
	       nfit->errvcd = (float *) malloc( nfit->numvcd * sizeof(float) );
	       if ( nfit->errvcd == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "errvcd" );
	       (void) memcpy( nfit->errvcd, nfit_pntr, 
			      nfit->numvcd * ENVI_FLOAT );
	       nfit_pntr += nfit->numvcd * ENVI_FLOAT;
	  }
	  (void) memcpy( &nfit->vcdflag, nfit_pntr, ENVI_USHRT );
	  nfit_pntr += ENVI_USHRT;
	  (void) memcpy( &nfit->esc, nfit_pntr, ENVI_FLOAT );
	  nfit_pntr += ENVI_FLOAT;
	  (void) memcpy( &nfit->erresc, nfit_pntr, ENVI_FLOAT );
	  nfit_pntr += ENVI_FLOAT;
	  (void) memcpy( &nfit->num_fitp, nfit_pntr, ENVI_USHRT );
	  nfit_pntr += ENVI_USHRT;
	  (void) memcpy( &nfit->num_nfitp, nfit_pntr, ENVI_USHRT );
	  nfit_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  nfit->num_fitp = byte_swap_u16( nfit->num_fitp );
	  nfit->num_nfitp = byte_swap_u16( nfit->num_nfitp );
#endif
	  if ( nfit->num_fitp > 0u ) {
	       nfit->linpars = (float *) 
		    malloc( nfit->num_fitp * sizeof(float) );
	       if ( nfit->linpars == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "linpars" );
	       (void) memcpy( nfit->linpars, nfit_pntr, 
				nfit->num_fitp * ENVI_FLOAT );
	       nfit_pntr += nfit->num_fitp * ENVI_FLOAT;
	       nfit->errlinpars = (float *)
		    malloc((size_t) nfit->num_fitp * sizeof( float ));
	       if ( nfit->errlinpars == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "errlinpars" );
	       (void) memcpy( nfit->errlinpars, nfit_pntr, 
				nfit->num_fitp * ENVI_FLOAT );
	       nfit_pntr += nfit->num_fitp * ENVI_FLOAT;
	  }
	  n_corr = (size_t) (nfit->num_fitp * (nfit->num_fitp-1)) / 2;
	  if ( n_corr > 0u ) {
	       nfit->lincorrm = (float *) malloc( n_corr * sizeof(float) );
	       if ( nfit->lincorrm == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "lincorrm" );
	       (void) memcpy( nfit->lincorrm, nfit_pntr, n_corr * ENVI_FLOAT );
	       nfit_pntr += n_corr * ENVI_FLOAT;
	  }
	  if ( nfit->num_nfitp > 0u ) {
	       nfit->nlinpars = (float *)
		    malloc( nfit->num_nfitp * sizeof(float) );
	       if ( nfit->nlinpars == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "nlinpars" );
	       (void) memcpy( nfit->nlinpars, nfit_pntr, 
				nfit->num_nfitp * ENVI_FLOAT );
	       nfit_pntr += nfit->num_nfitp * ENVI_FLOAT;
	       nfit->errnlinpars = (float *)
		    malloc( nfit->num_nfitp * sizeof(float) );
	       if ( nfit->errnlinpars == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "errnlinpars" );
	       (void) memcpy( nfit->errnlinpars, nfit_pntr, 
				nfit->num_nfitp * ENVI_FLOAT );
	       nfit_pntr += nfit->num_nfitp * ENVI_FLOAT;
	  }
	  n_corr = (size_t) (nfit->num_nfitp * (nfit->num_nfitp-1)) / 2;
	  if ( n_corr > 0u ) {
	       nfit->nlincorrm = (float *)
		    malloc((size_t) n_corr * sizeof( float ));
	       if ( nfit->nlincorrm == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "nlincorrm" );
	       (void) memcpy( nfit->nlincorrm, nfit_pntr, 
				n_corr * ENVI_FLOAT );
	       nfit_pntr += n_corr * ENVI_FLOAT;
	  }
	  (void) memcpy( &nfit->rms, nfit_pntr, ENVI_FLOAT );
	  nfit_pntr += ENVI_FLOAT;
	  (void) memcpy( &nfit->chi2, nfit_pntr, ENVI_FLOAT );
	  nfit_pntr += ENVI_FLOAT;
	  (void) memcpy( &nfit->goodness, nfit_pntr, ENVI_FLOAT );
	  nfit_pntr += ENVI_FLOAT;
	  (void) memcpy( &nfit->numiter, nfit_pntr, ENVI_USHRT );
	  nfit_pntr += ENVI_USHRT;
	  (void) memcpy( &nfit->fitflag, nfit_pntr, ENVI_USHRT );
	  nfit_pntr += ENVI_USHRT;
	  (void) memcpy( &nfit->amfgrd, nfit_pntr, ENVI_FLOAT );
	  nfit_pntr += ENVI_FLOAT;
	  (void) memcpy( &nfit->erramfgrd, nfit_pntr, ENVI_FLOAT );
	  nfit_pntr += ENVI_FLOAT;
	  (void) memcpy( &nfit->amfcld, nfit_pntr, ENVI_FLOAT );
	  nfit_pntr += ENVI_FLOAT;
	  (void) memcpy( &nfit->erramfcld, nfit_pntr, ENVI_FLOAT );
	  nfit_pntr += ENVI_FLOAT;
	  (void) memcpy( &nfit->amfflag, nfit_pntr, ENVI_USHRT );
	  nfit_pntr += ENVI_USHRT;
	  (void) memcpy( &nfit->temperature, nfit_pntr, ENVI_FLOAT );
	  nfit_pntr += ENVI_FLOAT;

#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_NFIT( nfit );
#endif
	  nfit++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * check if we read the whole DSR
 */
     if ( (unsigned int)(nfit_pntr - nfit_char) != dsd[indx_dsd].size )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, nfit_name );
     nfit_pntr = NULL;
/*
 * set return values
 */
 done:
     if ( nfit_char != NULL ) free( nfit_char );

     return nr_dsr;
}

