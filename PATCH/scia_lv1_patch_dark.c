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

.IDENTifer   SCIA_LV1_PATCH_DARK
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY
.LANGUAGE    ANSI C
.PURPOSE     copy/patch PDS CLCP/VLCP GADS between files
.INPUT/OUTPUT
  call as   SCIA_LV1_PATCH_DARK( patch_scia, num_dsd, dsd, fp_in, fp_out );
     input:
            unsigned short  patch_scia :  flag defining which patches to apply
            unsigned int num_dsd       :  number of DSDs
            struct dsd_envi *dsd       :  structure for the DSDs
	    FILE            *fp_in     :  file-pointer to input file
	    FILE            *fp_out    :  file-pointer to output file

.RETURNS     nothing
             error status passed by global variable ``nadc_stat''
.COMMENTS    None
.ENVIRONment None
.VERSION     2.0     06-Oct-2008   renamed modules and update to SDMF v3, RvH
             1.2     12-Dec-2005   bugfix reading OrbitDark array, RvH
             1.1     07-Dec-2005   fixed memory leakage, RvH
             1.0     24-Apr-2005   initial release by R. M. van Hees
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

/*+++++ Local Headers +++++*/
#define _SCIA_PATCH_1
#include <nadc_sdmf.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static inline
void __Inverse_Chan2( float *rbuff )
{
     register unsigned short nr;
     register float rtemp;

     rbuff += CHANNEL_SIZE;                   /* move to channel 2 data */
     for ( nr = 0; nr <  CHANNEL_SIZE / 2; nr++ ) {
	  rtemp = rbuff[nr];
	  rbuff[nr] = rbuff[(CHANNEL_SIZE-1)-nr];
	  rbuff[(CHANNEL_SIZE-1)-nr] = rtemp;
     }
}

static inline
bool checkQuality( int channel, struct mtbl_dark_rec *mtbl )
{
     if ( ! mtbl->saaFlag ) {
	  switch ( channel ) {
	  case 1:
	  case 2:
	  case 3:
	  case 4:
	  case 5:
	       if ( mtbl->obmTemp >= 252.18 && mtbl->obmTemp <= 252.22 )
		    return TRUE;
	       break;
	  case 6:
	       if ( mtbl->obmTemp >= 252.18 && mtbl->obmTemp <= 252.22 
		    && mtbl->detectorTemp[5] >= 198.f 
		    && mtbl->detectorTemp[5] <= 203.f )
		    return TRUE;
	       break;
	  case 7:
	       if ( mtbl->obmTemp >= 252.18 && mtbl->obmTemp <= 252.22 
		    && mtbl->detectorTemp[6] >= 130.f 
		    && mtbl->detectorTemp[6] <= 160.f )
		    return TRUE;
	       break;
	  case 8:
	       if ( mtbl->obmTemp >= 252.18 && mtbl->obmTemp <= 252.22 
		    && mtbl->detectorTemp[7] >= 140.f 
		    && mtbl->detectorTemp[7] <= 150.f )
		    return TRUE;
	       break;
	  }
     }
     return FALSE;
}

static
int getMetaIndx( hid_t fid, int channel, int orbit, /*@out@*/ int *fnd_orbit )
{
     const char prognm[] = "getMetaIndx";

     const short MIN_QualityNumber   = 40;
     const short GOOD_QualityNumber  = 70;

     const bool Save_Extern_Alloc = Use_Extern_Alloc;

     register int delta = 0;

     int   metaIndx     = -1;
     int   fnd_metaIndx = -1;
     short fnd_quality  = MIN_QualityNumber;

     struct mtbl_dark_rec *mtbl;

     Use_Extern_Alloc = FALSE;
     *fnd_orbit = -1;
     do {
	  int numIndx = 1;

	  (void) SDMF_get_metaIndex( fid, orbit + delta, &numIndx, &metaIndx );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR(prognm, NADC_ERR_FATAL, "SDMF_get_metaIndex");
	  if ( metaIndx > 0 ) {
	       SDMF_rd_darkTable( fid, &numIndx, &metaIndx, &mtbl );
	       if ( checkQuality( channel, mtbl ) 
		    && mtbl->quality >= fnd_quality ) {
		    fnd_quality  = mtbl->quality;
		    *fnd_orbit   = mtbl->absOrbit;
		    fnd_metaIndx = metaIndx;
	       }
	       free( mtbl );
	       if ( fnd_quality >=  GOOD_QualityNumber ) break;
	  } 
	  delta = (delta > 0) ? (-delta) : (1 - delta);
     } while ( abs( delta ) <= MAX_DiffOrbitNumber );
 done:
     Use_Extern_Alloc = Save_Extern_Alloc;
     return fnd_metaIndx;
}

/*+++++++++++++++++++++++++
.IDENTifer   readConstDarkSDMF
.PURPOSE     Read darkcurrent correction values from Monitoring database
.INPUT/OUTPUT
  call as   readConstDarkSDMF( orbit, &clcp );
     input:
           int orbit                :  absolute orbitnumber
    output:
	   struct clcp_scia *clcp   :  struct for constant leakage parameters

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
static
void readConstDarkSDMF( int orbit, struct clcp_scia *clcp )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, clcp@*/
{
     const char prognm[] = "readConstDarkSDMF";

     const size_t sz_ds_byte = SCIENCE_PIXELS * ENVI_FLOAT;

     const char sdmf_dark_db[] = "/SCIA/share/SDMF/3.0/sdmf_dark.h5";

     register int nch;

     char str_tmp[12];
     char str_msg[94] = "patched CLCP channel:";

     int fnd_orbit;

     hid_t  fid = -1;
/*
 * initialise output array
 */
     (void) memset( clcp->fpn, 0, sz_ds_byte );
     (void) memset( clcp->fpn_error, 0, sz_ds_byte );
     (void) memset( clcp->lc, 0, sz_ds_byte );
     (void) memset( clcp->lc_error, 0, sz_ds_byte );

/*
 * open output HDF5-file
 */
     if ( (fid = H5Fopen( sdmf_dark_db, H5F_ACC_RDONLY, H5P_DEFAULT )) < 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_FILE, sdmf_dark_db );
/*
 * read dark parameters per channel (NOT channel 8!)
 */
     for ( nch = 1; nch < SCIENCE_CHANNELS; nch++ ) {
	  int pixelRange[] = {(nch-1) * CHANNEL_SIZE, 
			      nch * CHANNEL_SIZE - 1};
/*
 * get indices to metaTable records or this state
 */
	  int metaIndx = getMetaIndx( fid, nch, orbit, &fnd_orbit );
/*
 * check if a solution was found
 */
	  if ( metaIndx == -1 ) continue;
	  if ( orbit != fnd_orbit )
	       (void) snprintf( str_tmp, 12, "%2d[%-d]", nch, fnd_orbit );
	  else
	       (void) snprintf( str_tmp, 12, "%2d", nch );
	  (void) strlcat( str_msg, str_tmp, 94 );
/*
 * read (constant) Dark correction parameters
 */
	  SDMF_rd_float_Array( fid, "analogOffset", 
				 1, &metaIndx, pixelRange,
				 (clcp->fpn)+pixelRange[0] );

	  SDMF_rd_float_Array( fid, "analogOffsetError", 
				 1, &metaIndx, pixelRange,
				 (clcp->fpn_error)+pixelRange[0] );

	  SDMF_rd_float_Array( fid, "darkCurrent", 
				 1, &metaIndx, pixelRange,
				 (clcp->lc)+pixelRange[0] );

	  SDMF_rd_float_Array( fid, "darkCurrentError", 
				 1, &metaIndx, pixelRange,
				 (clcp->lc_error)+pixelRange[0] );
     }
     NADC_ERROR( prognm, NADC_ERR_NONE, str_msg );

/* disabled until properly tested.. */
#if 0
/*
 * replace channel six with darks that account for RTS behaviour in 6+. 
 */
     {
     int ch6s = 5*1024;
     float stateDark[CHANNEL_SIZE];
/*
 * we just assume PET = 0.5 s just to have a single safe choice.
 */
     SDMF_CAL_IR_DARK(orbit, 6, .5f, clcp->fpn+ch6s, clcp->fpn_error+ch6s, clcp->lc+ch6s, clcp->lc_error+ch6s, stateDark);
     }
#endif

/*
 * invert channel 2 dark parameters
 */
     __Inverse_Chan2( clcp->fpn );
     __Inverse_Chan2( clcp->fpn_error );
     __Inverse_Chan2( clcp->lc );
     __Inverse_Chan2( clcp->lc_error );
/*
 * close SDMF Dark database
 */
     if ( fid != -1 ) (void) H5Fclose( fid );
}

/*+++++++++++++++++++++++++
.IDENTifer   getSimuDarkSDMF
.PURPOSE     Read darkcurrent correction values from Monitoring database
.INPUT/OUTPUT
  call as   getSimuDarkSDMF( orbit, &clcp, num_dsr, vlcp );
     input:
           int orbit                :  absolute orbitnumber
	   unsigned int num_dsr     :  number of VLCP records
    output:
           struct clcp_scia *clcp   :  struct for constant leakage parameters
	   struct vlcp_scia *vlcp   :  struct for variable leakage parameters

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
static
void getSimuDarkSDMF( int orbit, struct clcp_scia *clcp,
		      unsigned int num_dsr, struct vlcp_scia *vlcp )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, vlcp@*/
{
     const char prognm[] = "getSimuDarkSDMF";

     const size_t offs_ch8    = 7 * CHANNEL_SIZE;
     const size_t offs_ch8_ir = 2 * CHANNEL_SIZE;

     const char sdmf_dark_db[] = "/SCIA/share/SDMF/3.0/sdmf_simudark.h5";
     const int  pixelRange[]   = {0, CHANNEL_SIZE-1};

     register unsigned int nr, np;
     register float        orbvar;

     char   str_msg[SHORT_STRING_LENGTH];

     int    numIndx, metaIndx;
     float  amp1[CHANNEL_SIZE], sig_amp1[CHANNEL_SIZE];

     hid_t  fileID = -1;
     hid_t  grpID = -1;

     struct mtbl_simudark_rec *mtbl;
/*
 * open output HDF5-file
 */
     if ( (fileID = H5Fopen( sdmf_dark_db, H5F_ACC_RDONLY, H5P_DEFAULT )) < 0 )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_FILE, sdmf_dark_db );
     if ( (grpID = H5Gopen( fileID, "/ch8", H5P_DEFAULT )) < 0 ) {
          NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_GRP, "/ch8" );
     }
     numIndx = 1;
     metaIndx = -1;
     (void) SDMF_get_metaIndex( grpID, orbit, &numIndx, &metaIndx );
     if ( metaIndx == -1 ) goto done;

     SDMF_rd_simudarkTable( grpID, &numIndx, &metaIndx, &mtbl );

     SDMF_rd_float_Array( grpID, "ao", 1, &metaIndx, pixelRange,
			    clcp->fpn + offs_ch8 );
     SDMF_rd_float_Array( grpID, "sig_ao", 1, &metaIndx, pixelRange, 
			    clcp->fpn_error + offs_ch8 );
     SDMF_rd_float_Array( grpID, "lc", 1, &metaIndx, pixelRange,
			    clcp->lc + offs_ch8 );
     SDMF_rd_float_Array( grpID, "sig_lc", 1, &metaIndx, pixelRange,
			    clcp->lc_error + offs_ch8 );
     SDMF_rd_float_Array( grpID, "amp1", 1, &metaIndx, pixelRange,
			    amp1 );
     SDMF_rd_float_Array( grpID, "sig_amp1", 1, &metaIndx, pixelRange,
			    sig_amp1 );

     /* calculate orbvar for each angle in vlcp */
     for ( nr = 0; nr < num_dsr; nr++ ) {
	  orbvar = (float)
	       (cos(2 * PI * (mtbl->orbitPhase + vlcp[nr].orbit_phase))
		+ mtbl->amp2 
		* cos(4 * PI * (mtbl->phase2 + vlcp[nr].orbit_phase)));
	  for ( np = 0; np < CHANNEL_SIZE; np++ ) {
	       vlcp[nr].var_lc[np + offs_ch8_ir] = orbvar * amp1[np];
	       vlcp[nr].var_lc_error[np + offs_ch8_ir] = orbvar * sig_amp1[np];
	  }
     }
     (void) snprintf( str_msg, SHORT_STRING_LENGTH, 
		      "patched CLCP & VLCP channel: 8[%-d]", mtbl->absOrbit );
     NADC_ERROR( prognm, NADC_ERR_NONE, str_msg );
     free( mtbl );
/*
 * close SDMF Dark database
 */
 done:
     if ( grpID != -1 ) (void) H5Gclose( grpID );
     if ( fileID != -1 ) (void) H5Fclose( fileID );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void SCIA_LV1_PATCH_DARK( unsigned short patch_scia, int orbit,
			  unsigned int num_dsd, const struct dsd_envi *dsd, 
			  FILE *fp_in, FILE *fp_out )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, fp_in, fp_out@*/
{
     const char prognm[] = "SCIA_LV1_PATCH_DARK";

     unsigned int num_dsr;

     struct clcp_scia  clcp;
     struct vlcp_scia  *vlcp;
/*
 * read (G)ADS
 */
     (void) SCIA_LV1_RD_CLCP( fp_in, num_dsd, dsd, &clcp );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "CLCP" );
/*
 * read (G)ADS
 */
     num_dsr = SCIA_LV1_RD_VLCP( fp_in, num_dsd, dsd, &vlcp );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_RD, "VLCP" );
/*
 * replace Analog Offset and Leakage Current
 */
     if ( (patch_scia & SCIA_PATCH_DARK) != USHRT_ZERO ) {
          NADC_ERR_SAVE();
          readConstDarkSDMF( orbit, &clcp );
          if ( IS_ERR_STAT_ABSENT ) NADC_ERR_RESTORE();
     }
/*
 * replace Analog Offset, Leakage Current and Variable Dark (chan 8, only)
 */
     if ( (patch_scia & SCIA_PATCH_DARK) != USHRT_ZERO ) {
	  NADC_ERR_SAVE();
	  getSimuDarkSDMF( orbit, &clcp, num_dsr, vlcp );
	  if ( IS_ERR_STAT_ABSENT ) NADC_ERR_RESTORE();
     }
/*
 * write (G)ADS
 */
     (void) SCIA_LV1_WR_CLCP( fp_out, 1, clcp );
     if ( IS_ERR_STAT_FATAL )
          NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "CLCP" );
/*
 * write (G)ADS
 */
     (void) SCIA_LV1_WR_VLCP( fp_out, num_dsr, vlcp );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "VLCP" );
     if ( num_dsr > 0 ) free( vlcp );
}
