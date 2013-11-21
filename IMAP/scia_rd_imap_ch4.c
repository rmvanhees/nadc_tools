/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2008 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_RD_IMAP
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA IMAP CH4
.LANGUAGE    ANSI C
.PURPOSE     read SRON IMAP-CH4 products
.INPUT/OUTPUT
  call as   SCIA_RD_IMAP_CH4( flname, &hdr, &rec );

     input:  
             char   flname[]       : filename of IMLM product
    output:  
             struct imap_hdr *hdr  : header of IMAP product
	     struct imap_rec **rec : structure holding IMAP tile information

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION     1.1     28-Apr-2011   differentiate between CH4 and HDO code, RvH
             1.0     20-Nov-2008   initial release by R. M. van Hees
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
#include <math.h>

#include <hdf5.h>
#include <hdf5_hl.h>

/*+++++ Local Headers +++++*/
#define __IMAP_CH4_PRODUCT
#include <nadc_imap.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static
void SET_IMAP_BS_FLAG( const float *scanRange, unsigned int numRec,
		       struct imap_rec *rec )
{
     register unsigned int nr, ni = 0, nj = 0;

     while ( ni < numRec ) {
	  unsigned char state_id = rec[nj].meta.stateID;
	  float    min_scanRange = scanRange[nj]; 

	  while ( ++nj < numRec && rec[nj].meta.stateID == state_id ) {
	       if ( min_scanRange > scanRange[nj] ) 
		    min_scanRange = scanRange[nj];
	  }
	  for ( nr = ni; nr < nj; nr++ ) {
	       rec[nr].meta.bs = 
		    (scanRange[nr] > 2 * min_scanRange) ? TRUE : FALSE;
	  }
	  ni = nj;
     }
}

static
unsigned int SELECT_IMAP_RECORDS( unsigned int numRec, struct imap_rec *rec )
{
     register unsigned int nr, num;

     struct imap_rec *rec_buff;

     if ( numRec == 0u ) return 0u;

     rec_buff = (struct imap_rec *) malloc( numRec * sizeof(struct imap_rec) );
     (void) memcpy( rec_buff, rec, numRec * sizeof(struct imap_rec) );

     for ( num = nr = 0u; nr < numRec; nr++ ) {
	  if ( ! rec_buff[nr].meta.bs 
	       && rec_buff[nr].meta.sza < 80.f
	       && rec_buff[nr].meta.resi_co2 > 0.f
	       && rec_buff[nr].meta.resi_co2 < 0.0015f
	       && rec_buff[nr].meta.resi_ch4 > 0 
	       && rec_buff[nr].meta.resi_ch4 < 0.01f
	       && rec_buff[nr].meta.bu_ch4 > 1000
	       && rec_buff[nr].meta.pixels_ch4 >= 32
	       && rec_buff[nr].co2_vcd / rec_buff[nr].co2_model > 0.9f ) {
	       (void) memcpy( rec+num, rec_buff+nr, sizeof(struct imap_rec) );
	       num++;
	  }
     }
     free( rec_buff );
     return num;
}
/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_RD_IMAP_CH4( bool qflag, const char *flname, struct imap_hdr *hdr,
		       struct imap_rec **imap_out )
{
     const char prognm[] = "SCIA_RD_IMAP_CH4";

     register unsigned int ni, nr;

     char   *cpntr, ctemp[SHORT_STRING_LENGTH];
     float  *rbuff;
     double *dbuff;

     hid_t   fid = -1;
     hsize_t dims[2];

     struct imap_rec *rec = NULL;
/*
 * strip path of file-name
 */
     if ( (cpntr = strrchr( flname, '/' )) != NULL ) {
          (void) nadc_strlcpy( ctemp, ++cpntr, SHORT_STRING_LENGTH );
     } else {
          (void) nadc_strlcpy( ctemp, flname, SHORT_STRING_LENGTH );
     }
/*
 * initialize IMAP header structure
 */
     (void) nadc_strlcpy( hdr->product, ctemp, ENVI_FILENAME_SIZE );
     NADC_RECEIVEDATE( flname, hdr->receive_date );
     (void) strcpy( hdr->creation_date, hdr->receive_date );
     (void) strcpy( hdr->l1b_product, "" );
     (void) strcpy( hdr->validity_start, "" );
     (void) strcpy( hdr->validity_stop, "" );
     (void) nadc_strlcpy( hdr->software_version, ctemp+9, 4 );
     if ( (cpntr = strrchr( ctemp, '_' )) != NULL ) {
	  char str_magic[5], str_orbit[6];

	  (void) nadc_strlcpy( str_magic, cpntr+1, 5 );
	  while ( --cpntr > ctemp ) if ( *cpntr == '_' ) break;
	  (void) nadc_strlcpy( str_orbit, cpntr+1, 6 );
	  hdr->counter[0] = 
	       (unsigned short) strtoul( str_magic, (char **) NULL, 10 );
	  hdr->orbit[0]   = 
	       (unsigned int) strtoul( str_orbit, (char **) NULL, 10 );
     } else {
	  hdr->counter[0] = 0u;
	  hdr->orbit[0]   = 0u;
     }
     hdr->numProd   = 1u;
     hdr->numRec    = 0u;
     hdr->file_size = nadc_file_size( flname );
     imap_out[0] = NULL;
/*
 * open IMAP product in original HDF5 format
 */
     H5E_BEGIN_TRY {
	  fid = H5Fopen( flname, H5F_ACC_RDONLY, H5P_DEFAULT );
     } H5E_END_TRY;
     if ( fid < 0 ) NADC_RETURN_ERROR(prognm, NADC_ERR_HDF_FILE, flname );

     (void) H5LTget_dataset_info( fid, "/Data/Geolocation/time", 
				  dims, NULL, NULL );
     if ( dims[0] == 0 ) return;
     hdr->numRec = (unsigned int) dims[0];
/*
 * allocate enough space to store all records
 */
     rec = (struct imap_rec *) 
	  malloc( hdr->numRec * sizeof( struct imap_rec ));
     if ( rec == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rec" );

     if ( (dbuff = (double *) malloc( hdr->numRec * sizeof(double) )) == NULL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
     (void) H5LTread_dataset_double( fid, "/Data/Geolocation/time", dbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].jday = dbuff[ni];
     free( dbuff );

     if ( (rbuff = (float *) malloc( hdr->numRec * sizeof(float) )) == NULL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
     (void) H5LTread_dataset_float( fid, "/Data/Geolocation/Longitude", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ )
	  rec[ni].lon_center = LON_IN_RANGE( rbuff[ni] );
     (void) H5LTread_dataset_float( fid, "/Data/Geolocation/Latitude", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].lat_center = rbuff[ni];

     (void) H5LTread_dataset_float( fid, "/Data/FitResults/VCD_CH4", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].ch4_vcd = rbuff[ni];
     (void) H5LTread_dataset_float( fid, 
				    "/Data/FitResults/VCD_CH4_ERROR", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].ch4_error = rbuff[ni];
     (void) H5LTread_dataset_float( fid, 
				    "/Data/FitResults/VCD_CH4_MODEL", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].ch4_model = rbuff[ni];
     (void) H5LTread_dataset_float( fid, "/Data/FitResults/xVMR_CH4", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].ch4_vmr = rbuff[ni];
     (void) H5LTread_dataset_float( fid, "/Data/FitResults/VCD_CO2", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].co2_vcd = rbuff[ni];
     (void) H5LTread_dataset_float( fid, 
				    "/Data/FitResults/VCD_CO2_ERROR", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].co2_error = rbuff[ni];
     (void) H5LTread_dataset_float( fid, 
				    "/Data/FitResults/VCD_CO2_MODEL", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].co2_model = rbuff[ni];
/*
 * read auxiliary/geolocation data
 */
     (void) H5LTread_dataset_float( fid, "/Data/Geolocation/state_id", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) 
	  rec[ni].meta.stateID = (unsigned char) rbuff[ni];
     (void) H5LTread_dataset_float( fid, "/Data/Geolocation/IntegrationTime", 
				    rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) {
	  if ( rbuff[ni] == 0.12f ) 
	       rec[ni].meta.intg_time = 0.125f;
	  else
	  rec[ni].meta.intg_time = rbuff[ni];
     }
     (void) H5LTread_dataset_float( fid, "/Data/Auxiliary/pixels_ch4", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) 
	  rec[ni].meta.pixels_ch4 = (unsigned short) rbuff[ni];
     (void) H5LTread_dataset_float( fid, "/Data/Auxiliary/pixels_co2", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) 
	  rec[ni].meta.pixels_co2 = (unsigned short) rbuff[ni];
     (void) H5LTread_dataset_float( fid, "/Data/Geolocation/AirMassFactor", 
				    rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].meta.amf = rbuff[ni];
     (void) H5LTread_dataset_float( fid, 
				    "/Data/Geolocation/LineOfSightZenithAngle", 
				    rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].meta.lza = rbuff[ni];
     (void) H5LTread_dataset_float( fid, "/Data/Geolocation/SolarZenithAngle", 
				    rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].meta.sza = rbuff[ni];
     (void) H5LTread_dataset_float( fid, "/Data/Geolocation/ScanRange", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].meta.scanRange = rbuff[ni];
     SET_IMAP_BS_FLAG( rbuff, hdr->numRec, rec);
     (void) H5LTread_dataset_float( fid, "/Data/Geolocation/SurfaceElevation", 
				    rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].meta.elev = rbuff[ni];

     (void) H5LTread_dataset_float( fid, 
				    "/Data/Auxiliary/BU_ch4_window", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].meta.bu_ch4 = rbuff[ni];
     (void) H5LTread_dataset_float( fid, 
				    "/Data/Auxiliary/residual_ch4", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].meta.resi_ch4 = rbuff[ni];

     (void) H5LTread_dataset_float( fid, 
				    "/Data/Auxiliary/BU_co2_window", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].meta.bu_co2 = rbuff[ni];
     (void) H5LTread_dataset_float( fid, 
				    "/Data/Auxiliary/residual_co2", rbuff );
     for ( ni = 0; ni < hdr->numRec; ni++ ) rec[ni].meta.resi_co2 = rbuff[ni];
/*
 * read corners of the tiles
 */
     rbuff = (float *) realloc( rbuff, 
				NUM_CORNERS * hdr->numRec * sizeof(float) );
     if ( rbuff == NULL ) NADC_RETURN_ERROR( prognm, NADC_ERR_ALLOC, "rbuff" );
     (void) H5LTread_dataset_float( fid, "/Data/Geolocation/CornerLongitudes", 
				    rbuff );
     for ( ni = nr = 0; ni < hdr->numRec; ni++ ) {
	  register unsigned short nc = 0;

	  rec[ni].lon_corner[1] = LON_IN_RANGE( rbuff[nr] ); nr++;
	  rec[ni].lon_corner[0] = LON_IN_RANGE( rbuff[nr] ); nr++;
	  rec[ni].lon_corner[2] = LON_IN_RANGE( rbuff[nr] ); nr++;
	  rec[ni].lon_corner[3] = LON_IN_RANGE( rbuff[nr] ); nr++;
	  do {
	       if ( fabsf(rec[ni].lon_center-rec[ni].lon_corner[nc]) > 180.f ) {
		    if ( rec[ni].lon_center > 0.f )
			 rec[ni].lon_corner[nc] += 360.f;
		    else
			 rec[ni].lon_corner[nc] -= 360.f;
	       }
	  } while ( ++nc < NUM_CORNERS );
     }
     (void) H5LTread_dataset_float( fid, "/Data/Geolocation/CornerLatitudes", 
				    rbuff );
     for ( ni = nr = 0; ni < hdr->numRec; ni++ ) {
	  rec[ni].lat_corner[1] = rbuff[nr++];
	  rec[ni].lat_corner[0] = rbuff[nr++];
	  rec[ni].lat_corner[2] = rbuff[nr++];
	  rec[ni].lat_corner[3] = rbuff[nr++];
     }
     free( rbuff );
/*
 * select IMAP records
 */
     if ( qflag ) hdr->numRec = SELECT_IMAP_RECORDS( hdr->numRec, rec );

     /* obtain validity period from data records */
     if ( hdr->numRec > 0 ) {
	  SciaJDAY2adaguc( rec->jday, hdr->validity_start );
	  SciaJDAY2adaguc( rec[hdr->numRec-1].jday, hdr->validity_stop );
	  imap_out[0] = rec;
     } else
	  free( rec );
}
