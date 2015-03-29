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

.IDENTifer   SCIA_OL2_RD_LFIT
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA offline level 2 data
.LANGUAGE    ANSI C
.PURPOSE     read Limb Fitting Window Application Data sets
.INPUT/OUTPUT
  call as   nbyte = SCIA_OL2_RD_LFIT( fd, num_dsd, dsd, &lfit );
     input:
            FILE *fd                :   stream pointer
	    unsigned int num_dsd    :   number of DSDs
	    struct dsd_envi *dsd    :   structure for the DSDs
    output:
            struct lfit_scia **lfit :   Limb Fitting Window Data sets

.RETURNS     number of data set records read (unsigned int)
.COMMENTS    None
.ENVIRONment None
.VERSION      1.3   21-Jan-2003 bugfix, presence of intg_time 
                                not documented, RvH
              1.2   21-Jan-2003 added check for presence of this dataset, RvH
              1.1   16-Aug-2002	updated to ENV-ID-DLR-SCI-2200-4, RvH
              1.0  04-June-2002	created by R. M. van Hees 
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
void Sun2Intel_LFIT( struct lfit_scia *lfit )
{
     register unsigned short ni, nj, np, nr;

     lfit->mjd.days = byte_swap_32( lfit->mjd.days );
     lfit->mjd.secnd = byte_swap_u32( lfit->mjd.secnd );
     lfit->mjd.musec = byte_swap_u32( lfit->mjd.musec );
     lfit->dsrlen = byte_swap_u32( lfit->dsrlen );
     lfit->intg_time = byte_swap_u16( lfit->intg_time );
     IEEE_Swap__FLT( &lfit->refh );
     IEEE_Swap__FLT( &lfit->refp );
     for ( ni = nj = np = 0; np < lfit->num_rlevel; np++ ) {
	  IEEE_Swap__FLT( &lfit->tangh[np] );
	  IEEE_Swap__FLT( &lfit->tangp[np] );
	  IEEE_Swap__FLT( &lfit->tangt[np] );
	  for ( nr = 0; nr < lfit->num_species; nr++, ni++ ) {
	       IEEE_Swap__FLT( &lfit->mainrec[ni].tangvmr );
	       IEEE_Swap__FLT( &lfit->mainrec[ni].errtangvmr );
	       IEEE_Swap__FLT( &lfit->mainrec[ni].vertcol );
	       IEEE_Swap__FLT( &lfit->mainrec[ni].errvertcol );
	  }
	  for ( nr = 0; nr < lfit->num_scale; nr++, nj++ ) {
	       IEEE_Swap__FLT( &lfit->scaledrec[nj].tangvmr );
	       IEEE_Swap__FLT( &lfit->scaledrec[nj].errtangvmr );
	       IEEE_Swap__FLT( &lfit->scaledrec[nj].vertcol );
	       IEEE_Swap__FLT( &lfit->scaledrec[nj].errvertcol );
	  }
     }
/* struct meas_grid */
     for ( np = 0; np < lfit->num_mlevel; np++ ) {
	  lfit->mgrid[np].mjd.days = 
	       byte_swap_32( lfit->mgrid[np].mjd.days );
	  lfit->mgrid[np].mjd.secnd = 
	       byte_swap_u32( lfit->mgrid[np].mjd.secnd );
	  lfit->mgrid[np].mjd.musec = 
	       byte_swap_u32( lfit->mgrid[np].mjd.musec );
	  IEEE_Swap__FLT( &lfit->mgrid[np].tangh );
	  IEEE_Swap__FLT( &lfit->mgrid[np].tangp );
	  IEEE_Swap__FLT( &lfit->mgrid[np].tangt );
	  IEEE_Swap__FLT( &lfit->mgrid[np].win_limits[0] );
	  IEEE_Swap__FLT( &lfit->mgrid[np].win_limits[1] );
     }
/* struct state_vec */
     for ( np = 0; np < lfit->stvec_size; np++ ) {
	  IEEE_Swap__FLT( &lfit->statevec[np].value );
	  IEEE_Swap__FLT( &lfit->statevec[np].error );
     }
     for ( np = 0; np < lfit->cmatrixsize; np++ )
	  IEEE_Swap__FLT( &lfit->corrmatrix[np] );
     IEEE_Swap__FLT( &lfit->rms );
     IEEE_Swap__FLT( &lfit->chi2 );
     IEEE_Swap__FLT( &lfit->goodness );
     lfit->numiter = byte_swap_u16( lfit->numiter );
     lfit->summary[0] = byte_swap_u16( lfit->summary[0] );
     lfit->summary[1] = byte_swap_u16( lfit->summary[1] );
     for ( np = 0; np < lfit->ressize; np++ )
	  IEEE_Swap__FLT( &lfit->residuals[np] );
     for ( np = 0; np < lfit->num_adddiag; np++ )
	  IEEE_Swap__FLT( &lfit->adddiag[np] );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned 
int SCIA_OL2_RD_LFIT( FILE *fd, const char lfit_name[],
		      unsigned int num_dsd, const struct dsd_envi *dsd, 
		      struct lfit_scia **lfit_out )
{
     register unsigned short nl;

     char         *lfit_pntr, *lfit_char = NULL;
     size_t       dsd_size;
     unsigned int indx_dsd, n_rec;

     unsigned int nr_dsr = 0;

     struct lfit_scia *lfit;
/*
 * get index to data set descriptor
 */
     NADC_ERR_SAVE();
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, lfit_name );
     if ( IS_ERR_STAT_FATAL || IS_ERR_STAT_ABSENT )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, lfit_name );
     if ( IS_ERR_STAT_ABSENT || dsd[indx_dsd].num_dsr == 0 ) {
          NADC_ERR_RESTORE();
          lfit_out[0] = NULL;
          return 0u;
     }
     dsd_size = (size_t) dsd[indx_dsd].size;
/*
 * allocate memory to store output structures
 */
     if ( ! Use_Extern_Alloc ) {
	  lfit_out[0] = (struct lfit_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct lfit_scia));
     }
     if ( (lfit = lfit_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "lfit" );
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (lfit_char = (char *) malloc( dsd_size )) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "lfit_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
     if ( fread( lfit_char, dsd_size, 1, fd ) != 1 )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
/*
 * read data buffer to LFIT structure
 */
     lfit_pntr = lfit_char;
     do { 
	  (void) memcpy( &lfit->mjd.days, lfit_pntr, ENVI_INT );
	  lfit_pntr += ENVI_INT;
	  (void) memcpy( &lfit->mjd.secnd, lfit_pntr, ENVI_UINT );
	  lfit_pntr += ENVI_UINT;
	  (void) memcpy( &lfit->mjd.musec, lfit_pntr, ENVI_UINT );
	  lfit_pntr += ENVI_UINT;
	  (void) memcpy( &lfit->dsrlen, lfit_pntr, ENVI_UINT );
	  lfit_pntr += ENVI_UINT;
	  (void) memcpy( &lfit->quality, lfit_pntr, ENVI_CHAR );
	  lfit_pntr += ENVI_CHAR;
	  (void) memcpy( &lfit->intg_time, lfit_pntr, ENVI_USHRT );
	  lfit_pntr += ENVI_USHRT;
	  (void) memcpy( &lfit->method, lfit_pntr, ENVI_UCHAR );
	  lfit_pntr += ENVI_UCHAR;
	  (void) memcpy( &lfit->refh, lfit_pntr, ENVI_FLOAT );
	  lfit_pntr += ENVI_FLOAT;
	  (void) memcpy( &lfit->refp, lfit_pntr, ENVI_FLOAT );
	  lfit_pntr += ENVI_FLOAT;
	  (void) memcpy( &lfit->refpsrc, lfit_pntr, ENVI_UCHAR );
	  lfit_pntr += ENVI_UCHAR;
	  (void) memcpy( &lfit->num_rlevel, lfit_pntr, ENVI_UCHAR );
	  lfit_pntr += ENVI_UCHAR;
	  (void) memcpy( &lfit->num_mlevel, lfit_pntr, ENVI_UCHAR );
	  lfit_pntr += ENVI_UCHAR;
	  (void) memcpy( &lfit->num_species, lfit_pntr, ENVI_UCHAR );
	  lfit_pntr += ENVI_UCHAR;
	  (void) memcpy( &lfit->num_closure, lfit_pntr, ENVI_UCHAR );
	  lfit_pntr += ENVI_UCHAR;
	  (void) memcpy( &lfit->num_other, lfit_pntr, ENVI_UCHAR );
	  lfit_pntr += ENVI_UCHAR;
	  (void) memcpy( &lfit->num_scale, lfit_pntr, ENVI_UCHAR );
	  lfit_pntr += ENVI_UCHAR;
	  if ( lfit->num_rlevel > 0u ) {
	       lfit->tangh = (float *) 
		    malloc( lfit->num_rlevel * sizeof(float) );
	       if ( lfit->tangh == NULL )
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "tangh" );
	       (void) memcpy( lfit->tangh, lfit_pntr, 
			      lfit->num_rlevel * ENVI_FLOAT );
	       lfit_pntr += lfit->num_rlevel * ENVI_FLOAT;
	       lfit->tangp = (float *) 
		    malloc( lfit->num_rlevel * sizeof(float) );
	       if ( lfit->tangp == NULL )
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "tangp" );
	       (void) memcpy( lfit->tangp, lfit_pntr, 
			      lfit->num_rlevel * ENVI_FLOAT );
	       lfit_pntr += lfit->num_rlevel * ENVI_FLOAT;
	       lfit->tangt = (float *) 
		    malloc( lfit->num_rlevel * sizeof(float) );
	       if ( lfit->tangt == NULL )
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "tangt" );
	       (void) memcpy( lfit->tangt, lfit_pntr, 
			      lfit->num_rlevel * ENVI_FLOAT );
	       lfit_pntr += lfit->num_rlevel * ENVI_FLOAT;
	  }
	  n_rec = lfit->num_rlevel * lfit->num_species;
	  if ( n_rec > 0u ) {
	       lfit->mainrec = (struct layer_rec *)
		    malloc( n_rec * sizeof( struct layer_rec ));
	       if ( lfit->mainrec == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mainrec" );
	       (void) memcpy( lfit->mainrec, lfit_pntr, 
			      n_rec * sizeof(struct layer_rec) );
	       lfit_pntr += n_rec * sizeof(struct layer_rec);
	  }
	  n_rec = lfit->num_rlevel * lfit->num_scale;
	  if ( n_rec > 0u ) {
	       lfit->scaledrec = (struct layer_rec *)
		    malloc( n_rec * sizeof( struct layer_rec ));
	       if ( lfit->scaledrec == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "scaledrec" );
	       (void) memcpy( lfit->scaledrec, lfit_pntr, 
			      n_rec * sizeof(struct layer_rec) );
	       lfit_pntr += n_rec * sizeof(struct layer_rec);
	  }
/*
 * note that meas_grid is not aligned
 */
	  if ( lfit->num_mlevel > 0u ) {
	       lfit->mgrid = (struct meas_grid *)
		    malloc( lfit->num_mlevel * sizeof( struct meas_grid ));
	       if ( lfit->mgrid == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "mgrid" );
	       nl = 0;
	       do {
		    (void) memcpy( &lfit->mgrid[nl].mjd.days, lfit_pntr, 
				   ENVI_INT );
		    lfit_pntr += ENVI_INT;
		    (void) memcpy( &lfit->mgrid[nl].mjd.secnd, lfit_pntr, 
				   ENVI_UINT );
		    lfit_pntr += ENVI_UINT;
		    (void) memcpy( &lfit->mgrid[nl].mjd.musec, lfit_pntr, 
				   ENVI_UINT );
		    lfit_pntr += ENVI_UINT;
		    (void) memcpy( &lfit->mgrid[nl].tangh, lfit_pntr, 
				   ENVI_FLOAT );
		    lfit_pntr += ENVI_FLOAT;
		    (void) memcpy( &lfit->mgrid[nl].tangp, lfit_pntr, 
				   ENVI_FLOAT );
		    lfit_pntr += ENVI_FLOAT;
		    (void) memcpy( &lfit->mgrid[nl].tangt, lfit_pntr, 
				   ENVI_FLOAT );
		    lfit_pntr += ENVI_FLOAT;
		    (void) memcpy( &lfit->mgrid[nl].num_win, lfit_pntr, 
				   ENVI_UCHAR );
		    lfit_pntr += ENVI_UCHAR;
		    (void) memcpy( lfit->mgrid[nl].win_limits, lfit_pntr, 
				   2 * ENVI_FLOAT );
		    lfit_pntr += 2 * ENVI_FLOAT;
	       } while ( ++nl < lfit->num_mlevel );
	  }
	  (void) memcpy( &lfit->stvec_size, lfit_pntr, ENVI_USHRT );
	  lfit_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  lfit->stvec_size = byte_swap_u16( lfit->stvec_size );
#endif
	  if ( lfit->stvec_size > 0u ) {
	       lfit->statevec = (struct state_vec *)
		    malloc( lfit->stvec_size * sizeof(struct state_vec) );
	       if ( lfit->statevec == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "statevec" );
	       (void) memcpy( lfit->statevec, lfit_pntr, 
			      lfit->stvec_size * sizeof( struct state_vec ));
	       lfit_pntr += lfit->stvec_size * sizeof( struct state_vec );
	  }
	  (void) memcpy( &lfit->cmatrixsize, lfit_pntr, ENVI_USHRT );
	  lfit_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  lfit->cmatrixsize = byte_swap_u16( lfit->cmatrixsize );
#endif
	  if ( lfit->cmatrixsize > 0u ) {
	       lfit->corrmatrix = (float *)
		    malloc((size_t) lfit->cmatrixsize * ENVI_FLOAT );
	       if ( lfit->corrmatrix == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "corrmatrix" );
	       (void) memcpy( lfit->corrmatrix, lfit_pntr, 
			      lfit->cmatrixsize * ENVI_FLOAT );
	       lfit_pntr += lfit->cmatrixsize * ENVI_FLOAT ;
	  }
	  (void) memcpy( &lfit->rms, lfit_pntr, ENVI_FLOAT );
	  lfit_pntr += ENVI_FLOAT;
	  (void) memcpy( &lfit->chi2, lfit_pntr, ENVI_FLOAT );
	  lfit_pntr += ENVI_FLOAT;
	  (void) memcpy( &lfit->goodness, lfit_pntr, ENVI_FLOAT );
	  lfit_pntr += ENVI_FLOAT;
	  (void) memcpy( &lfit->numiter, lfit_pntr, ENVI_USHRT );
	  lfit_pntr += ENVI_USHRT;
	  (void) memcpy( lfit->summary, lfit_pntr, 2 * ENVI_USHRT );
	  lfit_pntr += 2 * ENVI_USHRT;
	  (void) memcpy( &lfit->criteria, lfit_pntr, ENVI_UCHAR );
	  lfit_pntr += ENVI_UCHAR;
	  (void) memcpy( &lfit->ressize, lfit_pntr, ENVI_USHRT );
	  lfit_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  lfit->ressize = byte_swap_u16( lfit->ressize );
#endif
	  if ( lfit->ressize > 0u ) {
	       lfit->residuals = (float *)
		    malloc((size_t) lfit->ressize * ENVI_FLOAT );
	       if ( lfit->residuals == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "residuals" );
	       (void) memcpy( lfit->residuals, lfit_pntr, 
			      lfit->ressize * ENVI_FLOAT );
	       lfit_pntr += lfit->ressize * ENVI_FLOAT ;
	  }
	  (void) memcpy( &lfit->num_adddiag, lfit_pntr, ENVI_USHRT );
	  lfit_pntr += ENVI_USHRT;
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  lfit->num_adddiag = byte_swap_u16( lfit->num_adddiag );
#endif
	  if ( lfit->num_adddiag > 0u ) {
	       lfit->adddiag = (float *)
		    malloc((size_t) lfit->num_adddiag * ENVI_FLOAT );
	       if ( lfit->adddiag == NULL ) 
		    NADC_GOTO_ERROR( NADC_ERR_ALLOC, "adddiag" );
	       (void) memcpy( lfit->adddiag, lfit_pntr, 
				lfit->num_adddiag * ENVI_FLOAT );
	       lfit_pntr += lfit->num_adddiag * ENVI_FLOAT ;
	  }
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_LFIT( lfit );
#endif
	  lfit++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * check if we read the whole DSR
 */
     if ( (unsigned int)(lfit_pntr - lfit_char) != dsd[indx_dsd].size )
	  NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, lfit_name );
     lfit_pntr = NULL;
/*
 * set return values
 */
 done:
     if ( lfit_char != NULL ) free( lfit_char );

     return nr_dsr;
}

