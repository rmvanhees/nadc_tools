/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2003 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GOME_LV1_IDL
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1b, IDL
.LANGUAGE    ANSI C
.PURPOSE     IDL wrappers for reading GOME level 1b data
.COMMENTS    contains 
.ENVIRONment None
.VERSION      1.0   05-Mar-2003	created by R. M. van Hees 
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
#include <nadc_idl.h>

/*+++++ Global Variables +++++*/
extern FILE *fd_nadc;

/*+++++ Static Variables +++++*/
static const char err_msg[] = "invalid number of function arguments";

/*+++++++++++++++++++++++++ Main Program or Functions +++++++++++++++*/
int IDL_STDCALL _GOME_LV1_RD_FSR ( int argc, void *argv[] )
{
     struct fsr1_gome *fsr;

     const char prognm[] = "LV1_RD_FSR";

     if ( argc != 1 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     fsr = (struct fsr1_gome *) argv[0];
     GOME_LV1_RD_FSR( fd_nadc, fsr );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return 1;
 done:
     return -1;
}

int IDL_STDCALL _GOME_LV1_RD_SPH ( int argc, void *argv[] )
{
     const char prognm[] = "LV1_RD_SPH";

     struct fsr1_gome *fsr;
     struct sph1_gome *sph;

     if ( argc != 2 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     fsr = (struct fsr1_gome *) argv[0];
     sph = (struct sph1_gome *) argv[1];
     GOME_LV1_RD_SPH( fd_nadc, fsr, sph );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return 1;
 done:
     return -1;
}

int IDL_STDCALL _GOME_LV1_RD_FCD ( int argc, void *argv[] )
{
     const char prognm[] = "LV1_RD_FCD";

     struct fsr1_gome *fsr;
     struct fcd_gome  C_fcd;
     struct lv1_leak  *leak;
     struct lv1_hot   *hot;
     struct lv1_spec  *spec;
     struct lv1_calib *calib;

     struct IDL_fcd_gome
     {
	  unsigned short flag_bitfield;
	  short  npeltier;
	  short  nleak;
	  short  nhot;
	  short  nspec;
	  short  nang;
	  short  width_conv;
	  short  indx_spec;
	  unsigned int sun_date;
	  unsigned int sun_time;
	  float  bsdf_0;
	  float  elevation;
	  float  azimuth;
	  float  sun_pmd[PMD_NUMBER];
	  float  sun_pmd_wv[PMD_NUMBER];
	  float  stray_level[4];
	  float  scale_peltier[NUM_FPA_SCALE];
	  float  coeffs[8];
	  float  filter_peltier[NUM_FPA_COEFFS];
	  float  pixel_gain[SCIENCE_PIXELS];
	  float  intensity[SCIENCE_PIXELS];
	  float  sun_ref[SCIENCE_PIXELS];
	  float  sun_precision[SCIENCE_PIXELS];
	  struct lv1_ghost ghost;
	  struct lv1_kde kde;
	  struct lv1_bcr bcr[NUM_SPEC_BANDS];
	  IDL_ULONG  pntr_leak;                /* IDL uses 32-bit addresses */
	  IDL_ULONG  pntr_hot;                 /* IDL uses 32-bit addresses */
	  IDL_ULONG  pntr_spec;                /* IDL uses 32-bit addresses */
	  IDL_ULONG  pntr_calib;               /* IDL uses 32-bit addresses */
     } *fcd;

     if ( argc != 6 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     fsr   = (struct fsr1_gome *) argv[0];
     fcd   = (struct IDL_fcd_gome *) argv[1];
     leak  = (struct lv1_leak *) argv[2];
     hot   = (struct lv1_hot *) argv[3];
     spec  = (struct lv1_spec *) argv[4];
     calib = (struct lv1_calib *) argv[5];
     Use_Extern_Alloc = FALSE;
     GOME_LV1_RD_FCD( fd_nadc, fsr, &C_fcd );
     Use_Extern_Alloc = TRUE;
     if ( IS_ERR_STAT_FATAL ) return -1;

     fcd->flag_bitfield = C_fcd.flags.two_byte;
     fcd->npeltier = C_fcd.npeltier;
     fcd->nleak = C_fcd.nleak;
     fcd->nhot = C_fcd.nhot;
     fcd->nspec = C_fcd.nspec;
     fcd->nang = C_fcd.nang;
     fcd->width_conv = C_fcd.width_conv;
     fcd->indx_spec = C_fcd.indx_spec;
     fcd->sun_date = C_fcd.sun_date;
     fcd->sun_time = C_fcd.sun_time;
     fcd->bsdf_0 = C_fcd.bsdf_0;
     fcd->elevation = C_fcd.elevation;
     fcd->azimuth = C_fcd.azimuth;
     (void) memcpy( fcd->sun_pmd, &C_fcd.sun_pmd, 
		    PMD_NUMBER * sizeof(float) );
     (void) memcpy( fcd->sun_pmd_wv, C_fcd.sun_pmd_wv, 
		    PMD_NUMBER * sizeof(float) );
     (void) memcpy( fcd->stray_level, C_fcd.stray_level, 4 * sizeof(float) );
     (void) memcpy( fcd->scale_peltier, C_fcd.scale_peltier, 
		    NUM_FPA_SCALE * sizeof(float) );
     (void) memcpy( fcd->coeffs, C_fcd.coeffs, 8 * sizeof(float) );
     (void) memcpy( fcd->filter_peltier, C_fcd.filter_peltier, 
		    NUM_FPA_COEFFS * sizeof(float) );
     (void) memcpy( fcd->pixel_gain, C_fcd.pixel_gain, 
		    SCIENCE_PIXELS * sizeof(float) );
     (void) memcpy( fcd->intensity, C_fcd.intensity, 
		    SCIENCE_PIXELS * sizeof(float) );
     (void) memcpy( fcd->sun_ref, C_fcd.sun_ref, 
		    SCIENCE_PIXELS * sizeof(float) );
     (void) memcpy( fcd->sun_precision, C_fcd.sun_precision, 
		    SCIENCE_PIXELS * sizeof(float) );
     (void) memcpy( &fcd->ghost, &C_fcd.ghost, sizeof(struct lv1_ghost) );
     (void) memcpy( &fcd->kde, &C_fcd.kde, sizeof(struct lv1_kde) );
     (void) memcpy( fcd->bcr, C_fcd.bcr, 
		    NUM_SPEC_BANDS * sizeof(struct lv1_bcr) );
     if ( fcd->nleak > 0 )
	  (void) memcpy( leak, C_fcd.leak, 
			 fcd->nleak * sizeof(struct lv1_leak) );
     if ( fcd->nhot > 0 )
	  (void) memcpy( hot, C_fcd.hot, 
			 fcd->nhot * sizeof(struct lv1_hot) );
     if ( fcd->nspec > 0 )
	  (void) memcpy( spec, C_fcd.spec, 
			 fcd->nspec * sizeof(struct lv1_spec) );
     if ( fcd->nang > 0 )
	  (void) memcpy( calib, C_fcd.calib, 
			 fcd->nang * sizeof(struct lv1_calib) );

     if ( C_fcd.nleak > 0 ) free( C_fcd.leak );
     if ( C_fcd.nspec > 0 ) free( C_fcd.spec );
     if ( C_fcd.nhot > 0 ) free( C_fcd.hot );
     if ( C_fcd.nang > 0 ) free( C_fcd.calib );
     return 1;
 done:
     return -1;
}

int IDL_STDCALL _GOME_LV1_RD_PCD ( int argc, void *argv[] )
{
     const char prognm[] = "LV1_RD_PCD";

     int nr_pcd;

     struct fsr1_gome *fsr;
     struct sph1_gome *sph;
     struct pcd_gome  *pcd;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     fsr = (struct fsr1_gome *) argv[0];
     sph = (struct sph1_gome *) argv[1];
     pcd = (struct pcd_gome *) argv[2];
     nr_pcd = GOME_LV1_RD_PCD( fd_nadc, fsr, sph, &pcd );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_pcd;
 done:
     return -1;
}

int IDL_STDCALL _GOME_LV1_RD_SMCD ( int argc, void *argv[] )
{
     const char prognm[] = "LV1_RD_SMCD";

     unsigned char source;

     int nr_smcd;

     struct fsr1_gome *fsr;
     struct sph1_gome *sph;
     struct smcd_gome *smcd;

     if ( argc != 4 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     source = *(unsigned char *) argv[0];
     fsr = (struct fsr1_gome *) argv[1];
     sph = (struct sph1_gome *) argv[2];
     smcd = (struct smcd_gome *) argv[3];
     nr_smcd = GOME_LV1_RD_SMCD( source, fd_nadc, fsr, sph, &smcd );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_smcd;
 done:
     return -1;
}

int IDL_STDCALL _GOME_LV1_PCD_PMD ( int argc, void *argv[] )
{
     const char prognm[] = "LV1_PCD_PMD";

     short nr_pcd, *indx_pcd;

     unsigned short   calib_mask;
     struct fsr1_gome *fsr;
     struct fcd_gome  fcd;
     struct pcd_gome  *pcd;

     if ( argc != 5 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     calib_mask = *(unsigned short *) argv[0];
     fsr = (struct fsr1_gome *) argv[1];
     nr_pcd = *(short *) argv[2];
     indx_pcd = (short *) argv[3];
     pcd = (struct pcd_gome *) argv[4];
/*
 * read a local copy of the FCD record
 */
     Use_Extern_Alloc = FALSE;
     GOME_LV1_RD_FCD( fd_nadc, fsr, &fcd );
     Use_Extern_Alloc = TRUE;
     if ( IS_ERR_STAT_FATAL ) {
	  Use_Extern_Alloc = TRUE;
	  if ( fcd.nleak > 0 ) free( fcd.leak );
	  if ( fcd.nspec > 0 ) free( fcd.spec );
	  if ( fcd.nhot > 0 ) free( fcd.hot );
	  if ( fcd.nang > 0 ) free( fcd.calib );
	  return -1;
     }
/*
 * calibrate PMD data
 */ 
     CALIB_PCD_PMD( PARAM_SET, calib_mask, &fcd, nr_pcd, indx_pcd, pcd );

     if ( fcd.nleak > 0 ) free( fcd.leak );
     if ( fcd.nspec > 0 ) free( fcd.spec );
     if ( fcd.nhot > 0 ) free( fcd.hot );
     if ( fcd.nang > 0 ) free( fcd.calib );
     return nr_pcd;
 done:
     return -1;
}

int IDL_STDCALL _GOME_LV1_SMCD_PMD ( int argc, void *argv[] )
{
     const char prognm[] = "LV1_SMCD_PMD";

     short nr_smcd, *indx_smcd;

     unsigned short   calib_mask;
     struct fsr1_gome *fsr;
     struct fcd_gome  fcd;
     struct smcd_gome  *smcd;

     if ( argc != 5 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     calib_mask = *(unsigned short *) argv[0];
     fsr = (struct fsr1_gome *) argv[1];
     nr_smcd = *(short *) argv[2];
     indx_smcd = (short *) argv[3];
     smcd = (struct smcd_gome *) argv[4];
/*
 * read a local copy of the FCD record
 */
     Use_Extern_Alloc = FALSE;
     GOME_LV1_RD_FCD( fd_nadc, fsr, &fcd );
     Use_Extern_Alloc = TRUE;
     if ( IS_ERR_STAT_FATAL ) {
	  Use_Extern_Alloc = TRUE;
	  if ( fcd.nleak > 0 ) free( fcd.leak );
	  if ( fcd.nspec > 0 ) free( fcd.spec );
	  if ( fcd.nhot > 0 ) free( fcd.hot );
	  if ( fcd.nang > 0 ) free( fcd.calib );
	  return -1;
     }
/*
 * calibrate PMD data
 */ 
     CALIB_SMCD_PMD( calib_mask, &fcd, nr_smcd, indx_smcd, smcd );

     if ( fcd.nleak > 0 ) free( fcd.leak );
     if ( fcd.nspec > 0 ) free( fcd.spec );
     if ( fcd.nhot > 0 ) free( fcd.hot );
     if ( fcd.nang > 0 ) free( fcd.calib );
     return nr_smcd;
 done:
     return -1;
}

static
short SELECT_PCD_REC( short nr_pcd, const short *indx_pcd, 
		      short nr_rec, /*@unique@*/ const struct rec_gome *rec, 
		      /*@out@*/ struct rec_gome *rec_out )
{
     register short nl = 0, np = 0, nr = 0;

     do {
	  while ( np < nr_pcd && rec[nr].indx_pcd > indx_pcd[np] ) np++;
	  if ( np >= nr_pcd ) break;
	  if ( rec[nr].indx_pcd == indx_pcd[np] ) {
	       (void) memcpy( rec_out+nl, rec+nr, sizeof( struct rec_gome ));
	       nl++;
	  }
     } while ( ++nr < nr_rec );
     return nl;
}

int IDL_STDCALL _GOME_LV1_RD_BDR ( int argc, void *argv[] )
{
     const char prognm[] = "LV1_RD_BDR";

     register int nr, np;

     short channel, nband;
     short nr_pcd, *indx_pcd;
     int   nr_rec;

     unsigned short   calib_mask;
     struct fsr1_gome *fsr;
     struct fcd_gome  fcd;
     struct pcd_gome  *pcd;
     struct rec_gome  *rec, *rec_tmp;

     if ( argc != 7 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     nband = *(short *) argv[0];
     calib_mask = *(unsigned short *) argv[1];
     fsr = (struct fsr1_gome *) argv[2];
     nr_pcd = *(short *) argv[3];
     indx_pcd = (short *) argv[4];
     pcd = (struct pcd_gome *) argv[5];
     rec = (struct rec_gome *) argv[6];
/*
 * read a local copy of the FCD record
 */
     Use_Extern_Alloc = FALSE;
     GOME_LV1_RD_FCD( fd_nadc, fsr, &fcd );
     if ( IS_ERR_STAT_FATAL ) {
	  Use_Extern_Alloc = TRUE;
	  return -1;
     }
/*
 * read Spectral Band Records
 */
     nr_rec = GOME_LV1_RD_BDR( fd_nadc, nband, fsr, &fcd, &rec_tmp );
     Use_Extern_Alloc = TRUE;
     if ( IS_ERR_STAT_FATAL ) {
	  if ( fcd.nleak > 0 ) free( fcd.leak );
	  if ( fcd.nspec > 0 ) free( fcd.spec );
	  if ( fcd.nhot > 0 ) free( fcd.hot );
	  if ( fcd.nang > 0 ) free( fcd.calib );
	  return -1;
     }
/*
 * select Sun/Moon Spectral Band Records
 */
     nr_rec = SELECT_PCD_REC( nr_pcd, indx_pcd, nr_rec, rec_tmp, rec );
     free( rec_tmp );
/*
 * add wavelength of detector data
 */
     channel = fcd.bcr[nband].array_nr - 1;
     nr = 0;
     do {
	  double *coeffs = 
	       fcd.spec[pcd[rec[nr].indx_pcd].indx_spec].coeffs[channel];

	  np = 0;
	  do {
	       rec[nr].wave[np] = 
		    (float) (((coeffs[4] * np + coeffs[3]) * np 
			      + coeffs[2]) * np + coeffs[1]) * np + coeffs[0];
	  } while ( ++np < CHANNEL_SIZE );
     } while ( ++nr < nr_rec );
/*
 * calibrate the data
 */
     CALIB_PCD_BDR( calib_mask, nband, &fcd, fsr->nr_pcd, pcd, nr_rec, rec );

     if ( fcd.nleak > 0 ) free( fcd.leak );
     if ( fcd.nspec > 0 ) free( fcd.spec );
     if ( fcd.nhot > 0 ) free( fcd.hot );
     if ( fcd.nang > 0 ) free( fcd.calib );
     return nr_rec;
 done:
     return -1;
}

static
short SELECT_SMCD_REC( short nband, short nr_smcd, const short *indx_smcd,
		       const struct smcd_gome *smcd,
		       /*@unique@*/ const struct rec_gome *rec,
		       /*@out@*/ struct rec_gome *rec_out )
{
     register short ni, nl = 0, nr = 0;

     do {
	  if ( (ni = smcd[indx_smcd[nr]].indx_bands[nband]) != -1 ) {
	       (void) memcpy( rec_out+nl, rec+ni, sizeof( struct rec_gome ));
	       nl++;
	  }
     } while ( ++nr < nr_smcd );

     return nl;
}

int IDL_STDCALL _GOME_LV1_RD_SMBDR ( int argc, void *argv[] )
{
     const char prognm[] = "LV1_RD_SMBDR";

     register int nr, np;

     short channel, nband;
     short nr_smcd, *indx_smcd;
     int   nr_rec;

     unsigned short   calib_mask;
     struct fsr1_gome *fsr;
     struct fcd_gome  fcd;
     struct smcd_gome *smcd;
     struct rec_gome  *rec, *rec_tmp;

     if ( argc != 7 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
          NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     nband = *(short *) argv[0];
     calib_mask = *(unsigned short *) argv[1];
     fsr = (struct fsr1_gome *) argv[2];
     nr_smcd = *(short *) argv[3];
     indx_smcd = (short *) argv[4];
     smcd = (struct smcd_gome *) argv[5];
     rec = (struct rec_gome *) argv[6];
/*
 * read a local copy of FCD record
 */
     (void) fprintf( stderr, "nr_smcd: %hd\n", nr_smcd );
     Use_Extern_Alloc = FALSE;
     GOME_LV1_RD_FCD( fd_nadc, fsr, &fcd );
     if ( IS_ERR_STAT_FATAL ) {
	  Use_Extern_Alloc = TRUE;
	  return -1;
     }
/*
 * read Spectral Band Records
 */
     nr_rec = GOME_LV1_RD_BDR( fd_nadc, nband, fsr, &fcd, &rec_tmp );
     Use_Extern_Alloc = TRUE;
     if ( IS_ERR_STAT_FATAL ) goto done;
     (void) fprintf( stderr, "nr_rec: %d\n", nr_rec );
/*
 * select Earth Spectral Band Records
 */
     nr_rec = SELECT_SMCD_REC( nband, nr_smcd, indx_smcd, smcd, rec_tmp, rec );
     nr = 0;
     do { rec[nr].indx_pcd -= fsr->nr_pcd; } while( ++nr < nr_rec );
     free( rec_tmp );
     (void) fprintf( stderr, "nr_rec: %d\n", nr_rec );
/*
 * add wavelength of detector data
 */
     channel = fcd.bcr[nband].array_nr - 1;
     nr = 0;
     do {
	  double *coeffs = 
	       fcd.spec[smcd[indx_smcd[nr]].indx_spec].coeffs[channel];

	  np = 0;
	  do {
	       rec[nr].wave[np] = 
		    (float) (((coeffs[4] * np + coeffs[3]) * np 
			      + coeffs[2]) * np + coeffs[1]) * np + coeffs[0];
	  } while ( ++np < CHANNEL_SIZE );
     } while ( ++nr < nr_smcd );
/*
 * calibrate the data
 */
     (void) fprintf( stderr, "Before calibration\n" );
     CALIB_SMCD_BDR( calib_mask, channel, &fcd, smcd, nr_smcd, rec );
     (void) fprintf( stderr, "Finished calibration\n" );

     if ( fcd.nleak > 0 ) free( fcd.leak );
     if ( fcd.nspec > 0 ) free( fcd.spec );
     if ( fcd.nhot > 0 ) free( fcd.hot );
     if ( fcd.nang > 0 ) free( fcd.calib );
     return nr_rec;

 done:
     if ( fcd.nleak > 0 ) free( fcd.leak );
     if ( fcd.nspec > 0 ) free( fcd.spec );
     if ( fcd.nhot > 0 ) free( fcd.hot );
     if ( fcd.nang > 0 ) free( fcd.calib );
     return -1;
}
