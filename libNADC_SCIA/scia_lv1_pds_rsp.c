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

.IDENTifer   SCIA_LV1_PDS_RSP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write Radiance Sensitivity Parameters
             (nadir, limb or occultation)
.COMMENTS    contains SCIA_LV1_RD_RSPN, SCIA_LV1_RD_RSPL, SCIA_LV1_RD_RSPO
             SCIA_LV1_WR_RSPN, SCIA_LV1_WR_RSPL, SCIA_LV1_WR_RSPO
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
              3.0   15-Apr-2005 added routine to write RSP?-struct to file, RvH
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
unsigned int SCIA_LV1_RD_RSPLO( FILE *fd, const struct dsd_envi dsd,
				struct rsplo_scia *rsplo )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd, *rsplo@*/
{
     const char prognm[] = "SCIA_LV1_RD_RSPLO";

     register unsigned short ni;

     char         *rsplo_pntr, *rsplo_char = NULL;
     float        sensitivity[SCIENCE_PIXELS];

     unsigned int nr_dsr = 0;

     const size_t nr_byte = SCIENCE_PIXELS * ENVI_FLOAT;
/*
 * allocate memory to temporary store data for output structure
 */
     if ( (rsplo_char = (char *) malloc( (size_t) dsd.dsr_size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rsplo_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd.offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( rsplo_char, dsd.dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	  rsplo_pntr = rsplo_char;
/*
 * read data buffer to RSPLO structure
 */
	  (void) memcpy( &rsplo->ang_esm, rsplo_pntr, ENVI_FLOAT );
	  rsplo_pntr += ENVI_FLOAT;
	  (void) memcpy( &rsplo->ang_asm, rsplo_pntr, ENVI_FLOAT );
	  rsplo_pntr += ENVI_FLOAT;
	  (void) memcpy( sensitivity, rsplo_pntr, nr_byte );
	  rsplo_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
	  if ( (rsplo_pntr - rsplo_char) != dsd.dsr_size )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd.name );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
          IEEE_Swap__FLT( &rsplo->ang_esm );
          IEEE_Swap__FLT( &rsplo->ang_asm );

          ni = 0;
          do {
               IEEE_Swap__FLT( sensitivity+ni );
          } while ( ++ni < SCIENCE_PIXELS );
#endif
          ni = 0;
          do {
               rsplo->sensitivity[ni] = (double) sensitivity[ni];
          } while ( ++ni < SCIENCE_PIXELS );

	  rsplo++;
     } while ( ++nr_dsr < dsd.num_dsr );
/*
 * set return values
 */
 done:
     if ( rsplo_char != NULL ) free( rsplo_char );
     return nr_dsr;
}

static 
void SCIA_LV1_WR_RSPLO( FILE *fd, struct dsd_envi dsd, unsigned int num_rsplo,
			const struct rsplo_scia *rsplo_in )
       /*@globals  errno;@*/
       /*@modifies errno, fd@*/
{
     const char prognm[] = "SCIA_LV1_WR_RSPLO";

     register unsigned short ni;

     float sensitivity[SCIENCE_PIXELS];

     struct rsplo_scia rsplo;

     const size_t nr_byte = SCIENCE_PIXELS * ENVI_FLOAT;
/*
 * write data set records
 */
     do {
	  (void) memcpy( &rsplo, rsplo_in, sizeof( struct rsplo_scia ));

          ni = 0;
          do {
               sensitivity[ni] = (float) rsplo.sensitivity[ni];
          } while ( ++ni < SCIENCE_PIXELS );
#ifdef _SWAP_TO_LITTLE_ENDIAN
          IEEE_Swap__FLT( &rsplo.ang_esm );
          IEEE_Swap__FLT( &rsplo.ang_asm );

          ni = 0;
          do {
               IEEE_Swap__FLT( sensitivity+ni );
          } while ( ++ni < SCIENCE_PIXELS );
#endif
/*
 * write RSPLO structure to data buffer
 */
	  if ( fwrite( &rsplo.ang_esm, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  if ( fwrite( &rsplo.ang_asm, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  if ( fwrite( sensitivity, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  rsplo_in++;
     } while ( ++dsd.num_dsr < num_rsplo );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_RSPN
.PURPOSE     read Radiance Sensitivity Parameters (nadir)
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_RSPN( fd, num_dsd, dsd, &rspn );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct rspn_scia **rspn :  Radiation Sensitivity Parameters
	                                (nadir)
.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_RSPN( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi *dsd,
			       struct rspn_scia **rspn_out )
{
     const char prognm[] = "SCIA_LV1_RD_RSPN";

     register unsigned short ni;

     char         *rspn_pntr, *rspn_char = NULL;
     size_t       dsr_size;
     unsigned int indx_dsd;
     float        sensitivity[SCIENCE_PIXELS];

     unsigned int nr_dsr = 0;

     struct rspn_scia *rspn;

     const char dsd_name[] = "RAD_SENS_NADIR";
     const size_t nr_byte  = SCIENCE_PIXELS * ENVI_FLOAT;
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( dsd[indx_dsd].num_dsr == 0 ) {
          rspn_out[0] = NULL;
          return 0u;
     }
     if ( ! Use_Extern_Alloc ) {
	  rspn_out[0] = (struct rspn_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct rspn_scia));
     }
     if ( (rspn = rspn_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rspn" );
/*
 * allocate memory to temporary store data for output structure
 */
     dsr_size = (size_t) dsd[indx_dsd].dsr_size;
     if ( (rspn_char = (char *) malloc( dsr_size )) == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rspn_char" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( rspn_char, dsr_size, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "" );
	  rspn_pntr = rspn_char;
/*
 * read data buffer to RSPN structure
 */
	  (void) memcpy( &rspn->ang_esm, rspn_pntr, ENVI_FLOAT );
	  rspn_pntr += ENVI_FLOAT;
	  (void) memcpy( sensitivity, rspn_pntr, nr_byte );
	  rspn_pntr += nr_byte;
/*
 * check if we read the whole DSR
 */
	  if ( (size_t)(rspn_pntr - rspn_char) != dsr_size )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_SIZE, dsd_name );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  IEEE_Swap__FLT( &rspn->ang_esm );

	  ni = 0;
	  do {
	       IEEE_Swap__FLT( sensitivity+ni );
	  } while ( ++ni < SCIENCE_PIXELS );
#endif
          ni = 0;
          do {
               rspn->sensitivity[ni] = (double) sensitivity[ni];
          } while ( ++ni < SCIENCE_PIXELS );

	  rspn++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
     rspn_pntr = NULL;
/*
 * set return values
 */
 done:
     if ( rspn_char != NULL ) free( rspn_char );
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_RSPL
.PURPOSE     read Radiance Sensitivity Parameters (limb)
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_RSPL( fd, num_dsd, dsd, &rspl );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct rsplo_scia **rspl:  Radiation Sensitivity Parameters
	                                (limb)
.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_RSPL( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi *dsd,
			       struct rsplo_scia **rspl_out )
{
     const char prognm[] = "SCIA_LV1_RD_RSPL";

     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     const char dsd_name[] = "RAD_SENS_LIMB";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( dsd[indx_dsd].num_dsr == 0 ) {
          rspl_out[0] = NULL;
          return 0u;
     }
     if ( ! Use_Extern_Alloc ) {
	  rspl_out[0] = (struct rsplo_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct rsplo_scia));
     }
     if ( rspl_out[0] == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rspl" );
/*
 * read radiance sensitivity parameters Limb/Occultation
 */
     nr_dsr = SCIA_LV1_RD_RSPLO( fd, dsd[indx_dsd], rspl_out[0] );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SCIA_LV1_RD_RSPLO" );
/*
 * set return values
 */
 done:
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_RSPO
.PURPOSE     read Radiance Sensitivity Parameters (occultation)
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_RSPO( fd, num_dsd, dsd, &rspo );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_dsd  :   number of DSDs
	    struct dsd_envi *dsd  :   structure for the DSDs
    output:
            struct rsplo_scia **rspo:  Radiation Sensitivity Parameters
	                                (occultation)
.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_RSPO( FILE *fd, unsigned int num_dsd, 
			       const struct dsd_envi *dsd,
			       struct rsplo_scia **rspo_out )
{
     const char prognm[] = "SCIA_LV1_RD_RSPO";

     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     const char dsd_name[] = "RAD_SENS_OCC";
/*
 * get index to data set descriptor
 */
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, dsd_name );
     if ( dsd[indx_dsd].num_dsr == 0 ) {
          rspo_out[0] = NULL;
          return 0u;
     }
     if ( ! Use_Extern_Alloc ) {
	  rspo_out[0] = (struct rsplo_scia *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct rsplo_scia));
     }
     if ( rspo_out[0] == NULL ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rspo" );
/*
 * read radiance sensitivity parameters Limb/Occultation
 */
     nr_dsr = SCIA_LV1_RD_RSPLO( fd, dsd[indx_dsd], rspo_out[0] );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "SCIA_LV1_RD_RSPLO" );
/*
 * set return values
 */
 done:
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_RSPN
.PURPOSE     write Radiance Sensitivity Parameters (nadir)
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_RSPN( fd, num_rspn, rspn );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_rspn :   number of RSPN records
            struct rspn_scia *rspn :  Radiation Sensitivity Parameters
	                                (nadir)
.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_RSPN( FILE *fd, unsigned int num_rspn, 
		       const struct rspn_scia *rspn_in )
{
     const char prognm[] = "SCIA_LV1_WR_RSPN";

     register unsigned short ni;

     float  sensitivity[SCIENCE_PIXELS];

     struct rspn_scia rspn;

     struct dsd_envi dsd = {
          "RAD_SENS_NADIR", "G",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     const size_t nr_byte  = SCIENCE_PIXELS * ENVI_FLOAT;

     if ( num_rspn == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records to file
 */
     do {
	  (void) memcpy( &rspn, rspn_in, sizeof( struct rspn_scia ));

	  ni = 0;
	  do {
	       sensitivity[ni] = (float) rspn.sensitivity[ni];
	  } while ( ++ni < SCIENCE_PIXELS );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  IEEE_Swap__FLT( &rspn.ang_esm );

	  ni = 0;
	  do {
	       IEEE_Swap__FLT( sensitivity+ni );
	  } while ( ++ni < SCIENCE_PIXELS );
#endif
	  if ( fwrite( &rspn.ang_esm, ENVI_FLOAT, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += ENVI_FLOAT;
	  if ( fwrite( sensitivity, nr_byte, 1, fd ) != 1 )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "" );
	  dsd.size += nr_byte;

	  rspn_in++;
     } while ( ++dsd.num_dsr < num_rspn );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_RSPL
.PURPOSE     write Radiance Sensitivity Parameters (limb)
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_RSPL( fd, num_rspl, rspl );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_rspl :   number of RSPL records
            struct rsplo_scia *rspl:  Radiation Sensitivity Parameters
	                                (limb)
.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_RSPL( FILE *fd, unsigned int num_rspl, 
		       const struct rsplo_scia *rspl_in )
{
     struct dsd_envi dsd = {
          "RAD_SENS_LIMB", "G",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_rspl == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write radiance sensitivity parameters Limb/Occultation
 */
     SCIA_LV1_WR_RSPLO( fd, dsd, num_rspl, rspl_in );
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_RSPO
.PURPOSE     write Radiance Sensitivity Parameters (occultation)
.INPUT/OUTPUT
  call as   SCIA_LV1_WR_RSPO( fd, num_rspo, rspo );
     input:
            FILE *fd              :   stream pointer
	    unsigned int num_rspo :   number of RSPO records
            struct rsplo_scia *rspo:  Radiation Sensitivity Parameters
	                                (occultation)
.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_RSPO( FILE *fd, unsigned int num_rspo, 
		       const struct rsplo_scia *rspo_in )
{
     struct dsd_envi dsd = {
          "RAD_SENS_OCC", "G",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_rspo == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write radiance sensitivity parameters Limb/Occultation
 */
     SCIA_LV1_WR_RSPLO( fd, dsd, num_rspo, rspo_in );
}
