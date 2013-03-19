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

.IDENTifer   SCIA_LV1_PDS_PSP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Polarisation Sensitivity Parameters 
             (nadir, limb or occultation)
.COMMENTS    contains SCIA_LV1_RD_PSPN, SCIA_LV1_RD_PSPL, SCIA_LV1_RD_PSPO
             SCIA_LV1_WR_PSPN, SCIA_LV1_WR_PSPL, SCIA_LV1_WR_PSPO
.ENVIRONment None
.VERSION      4.1   17-Oct-2005 fixed brownpaperbag address-of-pointer bug, RvH
              4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   15-Apr-2005 added routine to write PSP?-struct to file, RvH
              2.3   04-Mar-2003	separate modules for Limb/Occultation, RvH
              2.2   22-Mar-2002	test number of DSD; can be zero, RvH
              2.1   16-Jan-2002	use of global Use_Extern_Alloc, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.2   04-Oct-2001	changed input/output, RvH 
              1.1   03-Jan-2001 separate functions for nadir 
                                 & limb/occultation parameter sets, RvH
              1.0   10-Nov-1999 created by R. M. van Hees
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
#endif /* _SWAP_TO_LITTLE_ENDIAN */

static
unsigned int SCIA_LV1_RD_PSPLO( FILE *fd, const struct dsd_envi dsd,
				struct psplo_scia *psplo )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *psplo@*/
{
     const char prognm[] = "SCIA_LV1_RD_PSPLO";

     register unsigned short ni;

     char         *psplo_pntr, *psplo_char = NULL;
     float        mu2_buff[SCIENCE_PIXELS], mu3_buff[SCIENCE_PIXELS];

     unsigned int nr_dsr = 0;

     const size_t nr_byte = SCIENCE_PIXELS * ENVI_FLOAT;
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (psplo_char = (char *) malloc( (size_t) dsd.dsr_size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "psplo_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd.offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( psplo_char, (size_t) dsd.dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	  psplo_pntr = psplo_char;
/*
 * read data buffer to PSPLO structure
 */
	  (void) memcpy( &psplo->ang_esm, psplo_pntr, ENVI_FLOAT );
	  psplo_pntr += ENVI_FLOAT;
	  (void) memcpy( &psplo->ang_asm, psplo_pntr, ENVI_FLOAT );
	  psplo_pntr += ENVI_FLOAT;
	  (void) memcpy( mu2_buff, psplo_pntr, nr_byte );
	  psplo_pntr += nr_byte;
	  (void) memcpy( mu3_buff, psplo_pntr, nr_byte );
	  psplo_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
	  if ( (psplo_pntr - psplo_char) != dsd.dsr_size )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd.name );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
          IEEE_Swap__FLT( &psplo->ang_esm );
          IEEE_Swap__FLT( &psplo->ang_asm );

          ni = 0;
          do {
               IEEE_Swap__FLT( mu2_buff+ni );
               IEEE_Swap__FLT( mu3_buff+ni );
          } while ( ++ni < SCIENCE_PIXELS );
#endif
          ni = 0;
          do {
               psplo->mu2[ni] = (double) mu2_buff[ni];
               psplo->mu3[ni] = (double) mu3_buff[ni];
          } while ( ++ni < SCIENCE_PIXELS );

	  psplo++;
     } while ( ++nr_dsr < dsd.num_dsr );
/*
 * set return values
 */
 done:
     if ( psplo_char != NULL ) free( psplo_char );
     return nr_dsr;
}

static
void SCIA_LV1_WR_PSPLO( FILE *fd, struct dsd_envi dsd, unsigned int num_psplo,
			const struct psplo_scia *psplo_in )
       /*@globals  errno;@*/
       /*@modifies errno, fd@*/
{
     const char prognm[] = "SCIA_LV1_WR_PSPLO";

     register unsigned short ni;

     float mu2_buff[SCIENCE_PIXELS], mu3_buff[SCIENCE_PIXELS];

     struct psplo_scia psplo;

     const size_t nr_byte = SCIENCE_PIXELS * ENVI_FLOAT;
/*
 * write data set records
 */
     do {
	  (void) memcpy( &psplo, psplo_in, sizeof( struct psplo_scia ));

          ni = 0;
          do {
               mu2_buff[ni] = (float) psplo.mu2[ni];
               mu3_buff[ni] = (float) psplo.mu3[ni];
          } while ( ++ni < SCIENCE_PIXELS );
#ifdef _SWAP_TO_LITTLE_ENDIAN
          IEEE_Swap__FLT( &psplo.ang_esm );
          IEEE_Swap__FLT( &psplo.ang_asm );

          ni = 0;
          do {
               IEEE_Swap__FLT( mu2_buff+ni );
               IEEE_Swap__FLT( mu3_buff+ni );
          } while ( ++ni < SCIENCE_PIXELS );
#endif
/*
 * write PSPLO structure to data buffer
 */
	  if ( fwrite( &psplo.ang_esm, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  if ( fwrite( &psplo.ang_asm, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  if ( fwrite( mu2_buff, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( mu3_buff, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  psplo_in++;
     } while ( ++dsd.num_dsr < num_psplo );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_PSPN
.PURPOSE     read Polarisation Sensitivity Parameters (nadir)
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_PSPN( fd, num_dsd, dsd, &pspn );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct pspn_scia **pspn :  Polarisation Sensitivity Parameters
	                                (nadir)
.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_PSPN( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi *dsd,
			       struct pspn_scia **pspn_out )
{
     const char prognm[] = "SCIA_LV1_RD_PSPN";

     register unsigned short ni;

     char         *pspn_pntr, *pspn_char = NULL;
     size_t       dsr_size;
     unsigned int indx_dsd;
     float        mu2_buff[SCIENCE_PIXELS], mu3_buff[SCIENCE_PIXELS];

     unsigned int nr_dsr = 0;

     struct pspn_scia *pspn;

     const char dsd_name[] = "POL_SENS_NADIR";
     const size_t nr_byte  = SCIENCE_PIXELS * ENVI_FLOAT;
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( dsd[indx_dsd].num_dsr == 0 ) {
          pspn_out[0] = NULL;
          return 0u;
     }
     if ( ! Use_Extern_Alloc ) {
	  pspn_out[0] = (struct pspn_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct pspn_scia));
     } 
     if ( (pspn = pspn_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "pspn" );
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (pspn_char = (char *) malloc( dsr_size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "pspn_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( pspn_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	  pspn_pntr = pspn_char;
/*
 * read data buffer to PSPN structure
 */
	  (void) memcpy( &pspn->ang_esm, pspn_pntr, ENVI_FLOAT );
	  pspn_pntr += ENVI_FLOAT;
	  (void) memcpy( mu2_buff, pspn_pntr, nr_byte );
	  pspn_pntr += nr_byte;
	  (void) memcpy( mu3_buff, pspn_pntr, nr_byte );
	  pspn_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(pspn_pntr - pspn_char) != dsr_size )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  IEEE_Swap__FLT( &pspn->ang_esm );

	  ni = 0;
	  do {
	       IEEE_Swap__FLT( mu2_buff+ni );
	       IEEE_Swap__FLT( mu3_buff+ni );
	  } while ( ++ni < SCIENCE_PIXELS );
#endif
          ni = 0;
          do {
               pspn->mu2[ni] = (double) mu2_buff[ni];
               pspn->mu3[ni] = (double) mu3_buff[ni];
          } while ( ++ni < SCIENCE_PIXELS );

	  pspn++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * set return values
 */
     pspn_pntr = NULL;
 done:
     if ( pspn_char != NULL ) free( pspn_char );
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_PSPL
.PURPOSE     read Polarisation Sensitivity Parameters (limb)
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_PSPL( fd, num_dsd, dsd, &pspl );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct psplo_scia **pspl:  Polarisation Sensitivity Parameters
	                                (limb)
.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_PSPL( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi *dsd,
			       struct psplo_scia **pspl_out )
{
     const char prognm[] = "SCIA_LV1_RD_PSPL";

     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     const char dsd_name[] = "POL_SENS_LIMB";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( dsd[indx_dsd].num_dsr == 0 ) {
          pspl_out[0] = NULL;
          return 0u;
     }
     if ( ! Use_Extern_Alloc ) {
	  pspl_out[0] = (struct psplo_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct psplo_scia));
     }
     if ( pspl_out[0] == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "pspl" );
/*
 * read polarisation sensitivity parameters Limb/Occultation
 */
     nr_dsr = SCIA_LV1_RD_PSPLO( fd, dsd[indx_dsd], pspl_out[0] );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SCIA_LV1_RD_PSPLO" );
/*
 * set return values
 */
 done:
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_PSPO
.PURPOSE     read Polarisation Sensitivity Parameters (occultation)
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_PSPO( fd, num_dsd, dsd, &pspo );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct psplo_scia **pspo:  Polarisation Sensitivity Parameters
	                                (occultation)
.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_PSPO( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi *dsd,
			       struct psplo_scia **pspo_out )
{
     const char prognm[] = "SCIA_LV1_RD_PSPO";

     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     const char dsd_name[] = "POL_SENS_OCC";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( dsd[indx_dsd].num_dsr == 0 ) {
          pspo_out[0] = NULL;
          return 0u;
     }
     if ( ! Use_Extern_Alloc ) {
	  pspo_out[0] = (struct psplo_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct psplo_scia));
     }
     if ( pspo_out[0] == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "pspo" );
/*
 * read polarisation sensitivity parameters Limb/Occultation
 */
     nr_dsr = SCIA_LV1_RD_PSPLO( fd, dsd[indx_dsd], pspo_out[0] );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SCIA_LV1_RD_PSPLO" );
/*
 * set return values
 */
 done:
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_PSPN
.PURPOSE     write Polarisation Sensitivity Parameters (nadir)
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_PSPN( fd, num_dsd, dsd, pspn );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_pspn :   number of PSPN records
            struct pspn_scia *pspn :  Polarisation Sensitivity Parameters
	                                (nadir)
.RETURNS     nnothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_PSPN( FILE *fd, unsigned int num_pspn, 
		       const struct pspn_scia *pspn_in )
{
     const char prognm[] = "SCIA_LV1_WR_PSPN";

     register unsigned short ni;

     float  mu2_buff[SCIENCE_PIXELS], mu3_buff[SCIENCE_PIXELS];

     struct pspn_scia pspn;

     struct dsd_envi dsd = {
          "POL_SENS_NADIR", "G",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     const size_t nr_byte  = SCIENCE_PIXELS * ENVI_FLOAT;

     if ( num_pspn == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     do {
	  (void) memcpy( &pspn, pspn_in, sizeof( struct pspn_scia ));

	  ni = 0;
	  do {
	       mu2_buff[ni] = (float) pspn.mu2[ni];
	       mu3_buff[ni] = (float) pspn.mu3[ni];
	  } while ( ++ni < SCIENCE_PIXELS );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  IEEE_Swap__FLT( &pspn.ang_esm );

	  ni = 0;
	  do {
	       IEEE_Swap__FLT( mu2_buff+ni );
	       IEEE_Swap__FLT( mu3_buff+ni );
	  } while ( ++ni < SCIENCE_PIXELS );
#endif
/*
 * write PSPN structure to data buffer
 */
	  if ( fwrite( &pspn.ang_esm, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  if ( fwrite( mu2_buff, nr_byte,1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;
	  if ( fwrite( mu3_buff, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  pspn_in++;
     } while ( ++dsd.num_dsr < num_pspn );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_PSPL
.PURPOSE     write Polarisation Sensitivity Parameters (limb)
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_PSPL( fd, num_pspl, pspl );
     input:
            FILE *fd               :  stream pointer
	    unsigned int num_pspl  :  number of PSPL records
            struct psplo_scia *pspl:  Polarisation Sensitivity Parameters
	                                (limb)
.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_PSPL( FILE *fd, unsigned int num_pspl, 
		       const struct psplo_scia *pspl_in )
{
     struct dsd_envi dsd = {
          "POL_SENS_LIMB", "G",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_pspl == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write polarisation sensitivity parameters Limb/Occultation
 */
     SCIA_LV1_WR_PSPLO( fd, dsd, num_pspl, pspl_in );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_PSPO
.PURPOSE     write Polarisation Sensitivity Parameters (occultation)
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_PSPO( fd, num_pspo, pspo );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_pspo :   number of PSPO records
            struct psplo_scia *pspo:  Polarisation Sensitivity Parameters
	                                (occultation)
.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_PSPO( FILE *fd, unsigned int num_pspo, 
		       const struct psplo_scia *pspo_in )
{
     struct dsd_envi dsd = {
          "POL_SENS_OCC", "G",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_pspo == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write polarisation sensitivity parameters Limb/Occultation
 */
     SCIA_LV1_WR_PSPLO( fd, dsd, num_pspo, pspo_in );
}
