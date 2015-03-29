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

.IDENTifer   SCIA_LV1_PDS_PMD
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA level 1 data
.LANGUAGE    ANSI C
.PURPOSE     read/write PMD (level 0) Data Packets
.COMMENTS    contains SCIA_LV1_RD_PMD and SCIA_LV1_WR_PMD
.ENVIRONment None
.VERSION      4.0   11-Oct-2005 use direct write, add usage of SCIA_LV1_ADD_DSD
                                fixed also a bug in the write routine
              3.0   15-Apr-2005 added routine to write PMD-struct to file, RvH
              2.2   27-Nov-2002	bug fix: use variable Use_Extern_Alloc, RvH
              2.1   22-Mar-2002	test number of DSD; can be zero, RvH 
              2.0   08-Nov-2001	moved to the new Error handling routines, RvH 
              1.1   04-Oct-2001	changed input/output, RvH 
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

static
void Sun2Intel_PMD( struct mds1_pmd *pmd )
{
     pmd->mjd.days = byte_swap_32( pmd->mjd.days );
     pmd->mjd.secnd = byte_swap_u32( pmd->mjd.secnd );
     pmd->mjd.musec = byte_swap_u32( pmd->mjd.musec );
}
#endif /* _SWAP_TO_LITTLE_ENDIAN */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_RD_PMD
.PURPOSE     read PMD Data Packets
.INPUT/OUTPUT
  call as   nr_dsr = SCIA_LV1_RD_PMD( fd, num_dsd, dsd, &pmd );
     input:
            FILE *fd              :  stream pointer
	    unsigned int num_dsd  :  number of DSDs
	    struct dsd_envi *dsd  :  structure for the DSDs
    output:
            struct mds1_pmd **pmd :  structure for PMD data packets

.RETURNS     number of data set records read (unsigned int)
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int SCIA_LV1_RD_PMD( FILE *fd, unsigned int num_dsd, 
			      const struct dsd_envi *dsd,
			      struct mds1_pmd **pmd_out )
{
     unsigned int indx_dsd;

     unsigned int nr_dsr = 0;

     struct mds1_pmd *pmd;

     const char dsd_name[] = "PMD_PACKETS";
/*
 * get index to data set descriptor
 */
     NADC_ERR_SAVE();
     indx_dsd = ENVI_GET_DSD_INDEX( num_dsd, dsd, dsd_name );
     if ( IS_ERR_STAT_FATAL )
          NADC_GOTO_ERROR( NADC_ERR_PDS_RD, dsd_name );
     if ( IS_ERR_STAT_ABSENT || dsd[indx_dsd].num_dsr == 0 ) {
          NADC_ERR_RESTORE();
          pmd_out[0] = NULL;
          return 0u;
     }
     if ( ! Use_Extern_Alloc ) {
	  pmd_out[0] = (struct mds1_pmd *) 
	       malloc( dsd[indx_dsd].num_dsr * sizeof(struct mds1_pmd));
     }
     if ( (pmd = pmd_out[0]) == NULL ) 
	  NADC_GOTO_ERROR( NADC_ERR_ALLOC, "pmd" );
/*
 * rewind/read input data file
 */
     (void) fseek( fd, (long) dsd[indx_dsd].offset, SEEK_SET );
/*
 * read data set records
 */
     do {
	  if ( fread( &pmd->mjd, sizeof(struct mjd_envi), 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
	  if ( fread( &pmd->flag_mds, ENVI_UCHAR, 1, fd ) != 1 )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "" );
	  SCIA_LV0_RD_LV1_PMD( fd, pmd );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_RD, "MDS_PMD" );
/*
 * byte swap data to local representation
 */
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_PMD( pmd );
#endif
	  pmd++;
     } while ( ++nr_dsr < dsd[indx_dsd].num_dsr );
/*
 * set return values
 */
 done:
     return nr_dsr;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_LV1_WR_PMD
.PURPOSE     write PMD Data Packets
.INPUT/OUTPUT
  call as    SCIA_LV1_WR_PMD( fd, num_pmd, pmd );
     input:
            FILE *fd              :  stream pointer
	    unsigned int num_pmd  :  number of PMD records
            struct mds1_pmd *pmd :  structure for PMD data packets

.RETURNS     nothing
	     error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SCIA_LV1_WR_PMD( FILE *fd, unsigned int num_pmd,
		      const struct mds1_pmd *pmd_in )
{
     struct mds1_pmd pmd;

     struct dsd_envi dsd = {
          "PMD_PACKETS", "A",
          "                                                              ",
          0u, 0u, 0u, 0
     };

     if ( num_pmd == 0u ) {
	  (void) strcpy( dsd.flname, "NOT USED" );
	  SCIA_LV1_ADD_DSD( &dsd );
	  return;
     }
/*
 * write data set records
 */
     do {
	  (void) memcpy( &pmd, pmd_in, sizeof( struct mds1_pmd ) );
#ifdef _SWAP_TO_LITTLE_ENDIAN
	  Sun2Intel_PMD( &pmd );
#endif
	  dsd.size += SCIA_LV0_WR_LV1_PMD( fd, pmd );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( NADC_ERR_FATAL,"SCIA_LV0_WR_LV1_PMD" );
     } while ( pmd_in++, ++dsd.num_dsr < num_pmd );
/*
 * update list of written DSD records
 */
     dsd.dsr_size = (int) (dsd.size / dsd.num_dsr);
     SCIA_LV1_ADD_DSD( &dsd );
}
