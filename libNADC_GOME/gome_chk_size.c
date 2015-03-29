/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2007 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GOME_CHK_SIZE
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME
.LANGUAGE    ANSI C
.PURPOSE     check expected product size against size on disk
.INPUT/OUTPUT
  call as   GOME_LV1_CHK_SIZE( const struct fsr1_gome fsr, const char *gomefl )

     input: 
              struct fsr1_gome  : structure for level 1 FSR record
	      char *gomefl      : file name of GOME level 1 product

  call as   GOME_LV2_CHK_SIZE( const struct fsr2_gome fsr, const char *gomefl )

     input: 
              struct fsr2_gome  : structure for level 2 FSR record
	      char *gomefl      : file name of GOME level 1 product

.RETURNS     Nothing, check nadc_err_stat
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     20-Dec-2006   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void GOME_LV1_CHK_SIZE( const struct fsr1_gome fsr, const char *gomefl )
{
     register short nb;

     struct stat filestat;

     size_t prod_sz = LVL1_PIR_LENGTH + LVL1_FSR_LENGTH;

     prod_sz += fsr.nr_sph * fsr.sz_sph;
     prod_sz += fsr.nr_fcd * fsr.sz_fcd;
     prod_sz += fsr.nr_pcd * fsr.sz_pcd;
     prod_sz += fsr.nr_scd * fsr.sz_scd;
     prod_sz += fsr.nr_mcd * fsr.sz_mcd;
     for ( nb = 0; nb < NUM_SPEC_BANDS; nb++ )
	  prod_sz += fsr.nr_band[nb] * fsr.sz_band[nb];

     (void) stat( gomefl, &filestat );
     if ( prod_sz != (size_t) filestat.st_size )
	  NADC_RETURN_ERROR( NADC_ERR_FILE, "file corrupt" );
}

void GOME_LV2_CHK_SIZE( const struct fsr2_gome fsr, const char *gomefl )
{
     struct stat filestat;

     size_t prod_sz = LVL1_PIR_LENGTH + LVL2_FSR_LENGTH;

     prod_sz += fsr.nr_sph * fsr.sz_sph;
     prod_sz += fsr.nr_ddr * fsr.sz_ddr;

     (void) stat( gomefl, &filestat );
     if ( prod_sz != (size_t) filestat.st_size )
	  NADC_RETURN_ERROR( NADC_ERR_FILE, "file corrupt" );
}
