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

.IDENTifer   ENVI_PDS_DSD
.AUTHOR      R.M. van Hees
.KEYWORDS    Envisat - PDS data format
.LANGUAGE    ANSI C
.PURPOSE     read/write Data Set Description of the Envisat PDS product
.COMMENTS    contains ENVI_RD_DSD and ENVI_WR_DSD
.ENVIRONment None
.EXTERNALs   ENVI_RD_PDS_INFO
.VERSION      8.0   18-Sep-2008	renamed to general Envisat module, RvH
              7.1   11-Oct-2005	do not use lseek when writing(!);
                                fixed several bugs; 
				add usage of SCIA_LV1_ADD_DSD, RvH
              7.0   20-Apr-2005	complete rewrite & added write routine, RvH
              6.7   04-Feb-2005 move file-pointer to start first DSD, 
                                directly without searching for it, RvH
              6.6   12-Nov-2002	check file pointer status for EOF, RvH
              6.5   26-Mar-2002	remove whitespace after DSD name, RvH
              6.4   19-Jan-2002	removed SCIA_RD_ONE_DSD, RvH 
              6.3   17-Jan-2002	use of global Use_Extern_Alloc, RvH
              6.2   12-Dec-2001	updated documentation, RvH 
              6.1   03-Dec-2001	added SCIA_RD_ONE_DSD, RvH 
              6.0   08-Nov-2001	moved to the new Error handling routines, RvH
              5.0   29-Oct-2001	moved to new Error handling routines, RvH 
              4.0   24-Oct-2001 removed bytes_read parameter, RvH
              3.1   10-Oct-2001 added list of external (NADC) function, RvH
              3.0   23-Aug-2001 moved to separate module, RvH
              2.0   13-Oct-1999 combined in one module and rewritten 
                                  SCIA_LV1_RD_MPH, READ_SPH and READ_DSD, RvH
              1.0   13-Aug-1999 created by R. M. van Hees
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

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

#define LENGTH_DSD_SP 279
#define NUM_DSD_ITEMS 8

static NADC_pds_hdr_t dsd_items[NUM_DSD_ITEMS] = {
     {"DS_NAME", PDS_String, 29, "", NULL},
     {"DS_TYPE", PDS_Plain, 1, "", NULL},
     {"FILENAME", PDS_String, ENVI_FILENAME_SIZE, "", NULL},
     {"DS_OFFSET", PDS_uLong, 21, "", "<bytes>"},
     {"DS_SIZE", PDS_uLong, 21, "", "<bytes>"},
     {"NUM_DSR", PDS_uLong, 11, "", NULL},
     {"DSR_SIZE", PDS_Long, 11, "", "<bytes>"},
     {"", PDS_Spare, 32, "", NULL}
};
/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include <_envi_pds_hdr.inc>

static
void ENVI_WR_DSD_SPARE( FILE *fd )
{
     register unsigned short ni = 0u;

     const char str_space[] = " ";
     const char str_newl[] = "\n";

     do {
	  if ( fwrite( str_space, sizeof(char), 1, fd ) != 1 )
	       NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
     } while ( ++ni < LENGTH_DSD_SP );
     if ( fwrite( str_newl, sizeof(char), 1, fd ) != 1 )
	  NADC_RETURN_ERROR( NADC_ERR_PDS_WR, "" );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   ENVI_RD_DSD
.PURPOSE     read Data Set Description records of the Envisat PDS product
.INPUT/OUTPUT
  call as   nr_dsd = ENVI_RD_DSD( fd, mph, &dsd );
     input:  
            FILE   *fd            : (open) stream pointer
	    struct mph_envi mph   : main product header
    output:  
            struct dsd_envi *dsd  : structure for the DSD

.RETURNS     number of DSD read (unsigned int)
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned int ENVI_RD_DSD( FILE *fd, const struct mph_envi mph, 
			  struct dsd_envi *dsd )
{
     register size_t nr_char;

     unsigned int nr_dsd = 0u;

     const unsigned int total_dsd =
	  ((strncmp(mph.product,"MER_RR__2P",10) == 0) 
	   ? mph.num_dsd-2 : mph.num_dsd-1);    /* last DSD is empty (spare) */
     const long offset = PDS_MPH_LENGTH + 
	  (long)(mph.sph_size - mph.num_dsd * mph.dsd_size);
/*
 * rewind the file and search for the first DSD record
 */
     (void) fseek( fd, offset, SEEK_SET );
/*
 * read PDS header
 */
     do {
	  if ( NADC_RD_PDS_HDR(fd, NUM_DSD_ITEMS, dsd_items) != mph.dsd_size )
	       NADC_GOTO_ERROR( NADC_ERR_PDS_SIZE, "DSD size" );
/*
 * fill dsd_envi struct
 */
	  (void) nadc_strlcpy( dsd->name, dsd_items[0].value+1, 
			       (nr_char = dsd_items[0].length) );
	  while( --nr_char > 0 && 
		 (dsd->name[nr_char] == ' ' || dsd->name[nr_char] == '\0') );
	  dsd->name[nr_char+1] = '\0';
	  (void) nadc_strlcpy( dsd->type, dsd_items[1].value, 
			       (size_t) dsd_items[1].length+1 );
	  (void) nadc_strlcpy( dsd->flname, dsd_items[2].value+1, 
			       (nr_char = dsd_items[2].length) );
	  while ( --nr_char > 0 && 
		  (dsd->flname[nr_char] == ' ' 
		   ||  dsd->flname[nr_char] == '\0') );
	  if ( nr_char == 0 )
	       dsd->flname[0] = '\0';
	  else
	       dsd->flname[nr_char+1] = '\0';
	  (void) sscanf( dsd_items[3].value, "%u", &dsd->offset );
	  (void) sscanf( dsd_items[4].value, "%u", &dsd->size );
	  (void) sscanf( dsd_items[5].value, "%u", &dsd->num_dsr );
	  (void) sscanf( dsd_items[6].value, "%d", &dsd->dsr_size );

	  dsd++;
     } while ( ++nr_dsd < total_dsd );  
 done:
     return nr_dsd;
}

/*+++++++++++++++++++++++++
.IDENTifer   ENVI_WR_DSD
.PURPOSE     write Data Set Description records of the Envisat PDS product
.INPUT/OUTPUT
  call as   ENVI_WR_DSD( fd, num_dsd, dsd );
     input:  
            FILE   *fd            : (open) stream pointer
	    unsigned int num_dsd  : number of DSD records
            struct dsd_envi *dsd  : structure for the DSD

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void ENVI_WR_DSD( FILE *fd, unsigned int num_dsd, 
		  const struct dsd_envi *dsd )
{
     register size_t nr_char;

     unsigned int nr_dsd = 0u;

     char cbuff[SHORT_STRING_LENGTH];
/*
 * fill struct "dsd_envi"
 */
     nr_dsd = 0;
     do {
	  (void) nadc_strlcpy( cbuff, dsd[nr_dsd].name, SHORT_STRING_LENGTH );
	  nr_char = strlen( cbuff );
	  while( ++nr_char < dsd_items[0].length ) (void) strcat( cbuff, " " );
	  NADC_WR_PDS_ITEM( dsd_items, cbuff );
	  NADC_WR_PDS_ITEM( dsd_items+1, dsd[nr_dsd].type );
	  (void) nadc_strlcpy( cbuff, dsd[nr_dsd].flname, SHORT_STRING_LENGTH );
	  nr_char = strlen( cbuff );
	  while( ++nr_char < dsd_items[2].length ) (void) strcat( cbuff, " " );
	  NADC_WR_PDS_ITEM( dsd_items+2, cbuff );
	  NADC_WR_PDS_ITEM( dsd_items+3, &dsd[nr_dsd].offset );
	  NADC_WR_PDS_ITEM( dsd_items+4, &dsd[nr_dsd].size );
	  NADC_WR_PDS_ITEM( dsd_items+5, &dsd[nr_dsd].num_dsr );
	  NADC_WR_PDS_ITEM( dsd_items+6, &dsd[nr_dsd].dsr_size );

	  NADC_WR_PDS_HDR( NUM_DSD_ITEMS, dsd_items, fd );
     } while ( ++nr_dsd < num_dsd );
/*
 * last DSD is empty (spare)
 */
     ENVI_WR_DSD_SPARE( fd );
}
