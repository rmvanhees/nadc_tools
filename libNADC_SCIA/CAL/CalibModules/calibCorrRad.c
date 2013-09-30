/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2006 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   calibRadCorr
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy - Level 1b - Calibration
.LANGUAGE    ANSI C
.PURPOSE     perform Radiance correction on Sciamachy L1b science data
.COMMENTS    Contains functions SCIA_ATBD_CAL_RAD & SCIA_SMR_CAL_RAD
.ENVIRONment None
.VERSION      4.0   11-Sep-2013 replaced SCIA_ATBD_CAL_RAD_DETWIDE and 
                                Apply_RadSensLimb_detwide by SCIA_SMR_CAL_RAD
				fixed several minor bugs, RvH
	      3.3   02-Feb-2011 fixed round-off errors
                                apply PET correction (channel 6 - 8)
                                and update documentation, RvH
              3.2   02-Jul-2008 improved implementation (speed-up), RvH
              3.1   03-Mar-2008 implemented bugfixes identified by K. Bramstedt
                                added usage of L1b keydata to calibrate 
				monitor states, RvH
              3.0   14-Feb-2008 replaced HDF5 with Ife keydata file, RvH
              2.0   30-Oct-2007 nearly complete rewrite, RvH
              1.1   24-Aug-2006 removed testing on NaN, RvH
              1.0   06-Jun-2006 initial release by R. M. van Hees
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

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_1
#include <nadc_sdmf.h>
#include <nadc_scia_cal.h>

#include "getCorrIntg.inc"

/*+++++ Macros +++++*/
        /* NONE */

/*+++++ Static Variables +++++*/
static int   light_path;
static float alpha0_asm;
static float alpha0_esm;
static char  do_pixelwise[SCIENCE_CHANNELS+1];

enum lightPaths {
     DEFAULT_PATH, MONI_NADIR_PATH, MONI_LIMB_PATH, MONI_SOLAR_PATH, 
     MONI_EXTRAMIR_PATH
};

/*+++++ Global Variables +++++*/
        /* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static inline
float getIntg( const struct Clcon_scia Clcon )
{
    return (Clcon.coaddf * Clcon.pet);
}

#ifdef DEBUG
#include <hdf5.h>
#include <hdf5_hl.h>

static
void WRITE_H5_RSPN( const char *dbname, const char *table,
		    unsigned short num_rsp, const struct rspn_scia *rspn )
{
     const char prognm[] = "WRITE_H5_RSPN";

     hid_t   file_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   rspn_type[2] = { H5T_NATIVE_FLOAT, -1 };

     const char *rspn_names[2] = { "ang_esm", "sensitivity" };
     const size_t rspn_size = sizeof( struct rspn_scia );
     const size_t rspn_offs[2] = {
	  HOFFSET( struct rspn_scia, ang_esm ),
	  HOFFSET( struct rspn_scia, sensitivity )
     };

     file_id = H5Fcreate( dbname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
     if ( file_id < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, dbname );

     adim = SCIENCE_PIXELS;
     rspn_type[1] = H5Tarray_create( H5T_NATIVE_DOUBLE, 1, &adim );

     stat = H5TBmake_table( "RSP from Ife keydata", file_id, table,
                            2, num_rsp, rspn_size, rspn_names,
                            rspn_offs, rspn_type, 1,
                            NULL, FALSE, rspn );
     if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, table );
done:
     (void) H5Tclose( rspn_type[1] );
     (void) H5Fclose( file_id );
     return;
}

static
void WRITE_H5_RSPLO( const char *dbname, const char *table,
		     unsigned short num_rsp, const struct rsplo_scia *rspm )
{
     const char prognm[] = "WRITE_H5_RSPM";

     hid_t   file_id;
     hsize_t adim;
     herr_t  stat;

     hid_t   rspm_type[3] = { H5T_NATIVE_FLOAT, H5T_NATIVE_FLOAT, -1 };

     const char *rspm_names[3] = { "ang_esm", "ang_asm", "sensitivity" };
     const size_t rspm_size = sizeof( struct rsplo_scia );
     const size_t rspm_offs[3] = {
	  HOFFSET( struct rsplo_scia, ang_esm ),
	  HOFFSET( struct rsplo_scia, ang_asm ),
	  HOFFSET( struct rsplo_scia, sensitivity )
     };

     file_id = H5Fcreate( dbname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
     if ( file_id < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_FILE, dbname );

     adim = SCIENCE_PIXELS;
     rspm_type[2] = H5Tarray_create( H5T_NATIVE_DOUBLE, 1, &adim );

     stat = H5TBmake_table( "RSP from Ife keydata", file_id, table,
                            3, num_rsp, rspm_size, rspm_names,
                            rspm_offs, rspm_type, 1,
                            NULL, FALSE, rspm );
     if ( stat < 0 ) NADC_GOTO_ERROR( prognm, NADC_ERR_HDF_DATA, table );
done:
     (void) H5Tclose( rspm_type[2] );
     (void) H5Fclose( file_id );
     return;
}
#endif

static 
void Interp_Akima_Per_Channel( const float* wvlen_in, const float* data_in,
			       const float* wvlen_out, double* data_out )
{
    register unsigned short wl, n_ch;
/*
 *  BSDF - startpix_6+ - 795 start pixel of detector channel 6+ [-]
 *  BSDF - startpix_8+ - 220 start pixel of detector channel 8+ [-]
 *                        1    2    3    4    5   
 *                        6    6+   7    8    8+
 *  The (virtual) channel boundaries
 */
    const unsigned short pix0[] = {
	 0   , 1024, 2048, 3072, 4096, 5120, 5915, 6144, 7168, 7388
    };
    const unsigned short pixlen[] = {
	 1024, 1024, 1024, 1024, 1024,  795,  229, 1024,  220,  804 
    };
    /* pixel_wise, const at first */
    const bool p_wise[]={
	 FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE
    };

    for ( n_ch = 0; n_ch < 10; n_ch++ ) {
	 if ( p_wise[n_ch] ) {
	      for ( wl = pix0[n_ch]; wl < pix0[n_ch] + pixlen[n_ch]; wl++ ) {
		   data_out[wl] = (double) data_in[wl];
	      }
	 } else {
	      FIT_GRID_AKIMA( FLT32_T, FLT32_T, pixlen[n_ch], 
			      wvlen_in + pix0[n_ch], data_in + pix0[n_ch],
			      FLT32_T, FLT64_T, pixlen[n_ch],
			      wvlen_out + pix0[n_ch], data_out + pix0[n_ch] );
	 }
    }
}

/*+++++++++++++++++++++++++
.IDENTifer  Calc_RadSensStatic
.PURPOSE    calculate ELEV_i_alpha0, bas_rad and obm_s_p
.INPUT/OUTPUT
  call as    Get_RadSensCal( NDF, wvlen, key, elev_a0, abs_rad, obm_s_p );
     input:
	    bool NDF_flag               :  flag for using NDF correction
            float *wvlen                :  wavelength grid 
	    const struct rspd_key *key  :  keydata structure
    output:
            double elev_a0[SCIENCE_PIXELS]  : elev_a0 calculated here
	    double abs_rad[SCIENCE_PIXELS]  : abs_rad
	    double obm_s_p[SCIENCE_PIXELS]  : obm_s_p

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void Calc_RadSensStatic( bool NDF_flag, const float *wvlen, 
			 const struct rspd_key *key, /*@out@*/ double *elev_a0,
			 /*@out@*/ double *abs_rad, /*@out@*/ double *obm_s_p )
{
     register unsigned short n_ch, wl;

     float abs_rad_tmp[SCIENCE_PIXELS];

     double elev_s_a0[SCIENCE_PIXELS];
     double elev_p_a0[SCIENCE_PIXELS];
    
     /* Conversion factor absrad W/(sr*cm3) -> photons/(s*cm2*sr*nm) */
     const double absrad_conv = 5.035e8;
  
     /* set up elev_s/elev_p on full grid */
     const int n_alpha0 = 2; 		/* dirty, should be searched in data */

     /* always interpolate keydata to science wavelength grid */
     FIT_GRID_AKIMA( FLT32_T, FLT32_T, 
		     key->elev_p[n_alpha0].n_wl, key->elev_p[n_alpha0].wl,
		     key->elev_p[n_alpha0].sensitivity,
		     FLT32_T, FLT64_T, 
		     SCIENCE_PIXELS, wvlen, elev_p_a0 );

     FIT_GRID_AKIMA( FLT32_T, FLT32_T, 
		     key->elev_s[n_alpha0].n_wl, key->elev_s[n_alpha0].wl,
		     key->elev_s[n_alpha0].sensitivity,
		     FLT32_T, FLT64_T, 
		     SCIENCE_PIXELS, wvlen, elev_s_a0 );

     /* OBM_s_p to wavelength grid */
     Interp_Akima_Per_Channel( key->key_fix.wl, key->key_fix.OBM_s_p,
			       wvlen, obm_s_p );

     for ( wl = 0; wl < SCIENCE_PIXELS; wl++ )
	 elev_a0[wl] = obm_s_p[wl] * elev_s_a0[wl] + elev_p_a0[wl];

     /*  PPG correction is applied here if do_pixelwise = 't' */
     for ( n_ch = 0; n_ch < SCIENCE_CHANNELS; n_ch++ ) {
	  const unsigned short offs = n_ch * CHANNEL_SIZE;

	  if ( do_pixelwise[n_ch] == 'f' ) 
	       (void) memcpy ( abs_rad_tmp+offs , key->key_fix.ABS_RAD+offs, 
			       CHANNEL_SIZE * sizeof(float) );
	  else {
	       for ( wl = offs; wl < offs+CHANNEL_SIZE; wl++ )
		    abs_rad_tmp[wl] = 
			 key->key_fix.ABS_RAD[wl] / key->key_fix.PPG0[wl];
	  }
     }
     /* absrad to wavelength grid */
     Interp_Akima_Per_Channel( key->key_fix.wl, abs_rad_tmp, wvlen, abs_rad );

     /* recalculate absrad W/(sr.cm^3) to photons/(s.cm^2.sr.nm) */
     for ( wl = 0; wl < SCIENCE_PIXELS; wl++ )
	 abs_rad[wl] /= (absrad_conv * (double) wvlen[wl]);

     /* Apply neutral density filter if necessary */
     if ( NDF_flag ) {
	  double ndf[SCIENCE_PIXELS];
	  double ndf_s_p[SCIENCE_PIXELS];

	  Interp_Akima_Per_Channel( key->key_fix.wl, key->key_fix.NDF,
				    wvlen, ndf );

	  Interp_Akima_Per_Channel( key->key_fix.wl, key->key_fix.NDF_s_p,
				    wvlen, ndf_s_p );
	 
	  /* Apply NDF factor to overall sensitivity */
	  for ( wl = 0; wl < SCIENCE_PIXELS; wl++ ) {
	       abs_rad[wl] *= 2.0 * ndf[wl] / ( 1.0 + ndf_s_p[wl] ) ;
	       obm_s_p[wl] *= ndf_s_p[wl] ;
	  }
     }
}

/*+++++++++++++++++++++++++ Static Functions (RSPN) +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   Get_RadSensNadir
.PURPOSE     Obtain Radiation Sensitivity Parameters for wavelength grid
.INPUT/OUTPUT
  call as    Get_RadSensNadir( fileParam, wvlen, &rspn );
     input:
            struct file_rec *fileParam: file/calibration parameters
	    struct wvlen_rec wvlen    : Solar/Science wavelength grid
    output:
            struct rspn_scia **rspn   : radiance sensitivity parameters (Nadir)
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
unsigned short Get_RadSensNadir( const struct file_rec *fileParam, 
				 const struct wvlen_rec wvlen, 
				 struct rspn_scia **rspn_out )
      /*@globals  errno, nadc_stat, nadc_err_stack;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp, *rspn_out@*/
{
     const char prognm[] = "Get_RadSensNadir";

     register unsigned short n_ch, nr;

     unsigned short num_rsp = 0;

     double rbuff[CHANNEL_SIZE];

     struct rspn_scia *rspn;

     rspn_out[0] = NULL;
/*
 * read radianace sensitivity parameters (nadir)
 */
     num_rsp = (unsigned short) SCIA_LV1_RD_RSPN( fileParam->fp, 
						  fileParam->num_dsd, 
						  fileParam->dsd, &rspn );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "RSPN" );
#ifdef DEBUG
     WRITE_H5_RSPN( "scia_l1b_rspn.h5", "rspn", num_rsp, rspn );
#endif
/*
 * interpolate sensitivities to wavelength grid (when do_pixelwise equals 'f')
 */
     for ( n_ch = 0; n_ch < SCIENCE_CHANNELS; n_ch++ ) {
	  if ( do_pixelwise[n_ch] == 'f' ) {
	       unsigned short offs = n_ch * CHANNEL_SIZE;

	       for ( nr = 0; nr < num_rsp; nr++ ) {
		    (void) memcpy( rbuff, &rspn[nr].sensitivity[offs],
				   CHANNEL_SIZE * sizeof(double) );

		    FIT_GRID_AKIMA( FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.solar + offs, rbuff,
				    FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.science + offs,
				    &rspn[nr].sensitivity[offs] );
	       }
	  }
     }
/*
 * everything went fine, so return the data...
 */
     rspn_out[0] = rspn;
 done:
     return num_rsp;
}

/*+++++++++++++++++++++++++
.IDENTifer   Get_H5_RadSensNadir
.PURPOSE     Obtain Radiation Sensitivity Parameters for wavelength grid
.INPUT/OUTPUT
  call as    Get_H5_RadSensNadir( NDF, wvlen, &rspn );
     input:
            bool NDF                  : neutral density filter flag
	    float *wvlen              : wavelength grid
    output:
            struct rspn_scia **rspn   : radiance sensitivity parameters (Nadir)
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
unsigned short Get_H5_RadSensNadir( bool NDF, const float *wvlen, 
				    struct rspn_scia **rspn_out )
      /*@globals  errno, nadc_stat, nadc_err_stack;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack, *rspn_out@*/
{
     const char prognm[] = "Get_H5_RadSensNadir";

     register unsigned short np, nr, offs;

     unsigned short num_rsp = 0;

     double elev_a0[SCIENCE_PIXELS], abs_rad[SCIENCE_PIXELS], 
	  obm_s_p[SCIENCE_PIXELS]; 
     double elev_p[SCIENCE_PIXELS], elev_s[SCIENCE_PIXELS];

     struct rspd_key   key;
     struct rspn_scia  *rspn;
/*
 * initialize return values
 */
     key.elev_p  = NULL;
     key.elev_s  = NULL;
     key.el_az_p = NULL;
     key.el_az_s = NULL;
     key.brdf_p  = NULL;
     key.brdf_s  = NULL;
     rspn_out[0] = NULL;
/*
 * read radiance calibration keydata from HDF5 file (origin Ife)
 */
     SCIA_RD_H5_RSPD( &key );
/*
 * calculate elev_i_alpha0, abs_rad and obm_s_p
 */
     Calc_RadSensStatic( NDF, wvlen, &key, elev_a0, abs_rad, obm_s_p );
/*
 * allocate memory for the RSPN records
 */
     rspn = (struct rspn_scia *) 
	  malloc( key.n_elev * sizeof(struct rspn_scia) );
     if ( rspn == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rspn" );
     num_rsp = key.n_elev;
/*
 * interpolate sensitivities to wavelength grid 
 * also: rearrange data [elev] decreasing
 */
     offs = key.n_elev;
     nr = 0;
     do {
	  --offs;
	  FIT_GRID_AKIMA( FLT32_T, FLT32_T, 
			  key.elev_p[offs].n_wl, key.elev_p[offs].wl,
			  key.elev_p[offs].sensitivity,
			  FLT32_T, FLT64_T, 
			  SCIENCE_PIXELS, wvlen, elev_p );

	  FIT_GRID_AKIMA( FLT32_T, FLT32_T, 
			  key.elev_s[offs].n_wl, key.elev_s[offs].wl,
			  key.elev_s[offs].sensitivity,
			  FLT32_T, FLT64_T, 
			  SCIENCE_PIXELS, wvlen, elev_s );

	  rspn[nr].ang_esm = key.elev_p[offs].elevat_angle;
	  for ( np = 0; np < SCIENCE_PIXELS; np++ )
	       rspn[nr].sensitivity[np] = 
		    (abs_rad[np] 
		     * (obm_s_p[np] * elev_s[np] + elev_p[np]) / elev_a0[np]);
     } while( ++nr < num_rsp );
#ifdef DEBUG
     WRITE_H5_RSPN( "scia_key_rspn.h5", "rspn", num_rsp, rspn );
#endif
/*
 * everything went fine, so return the data...
 */
     rspn_out[0] = rspn;
 done:
     if ( key.elev_p  != NULL ) free( key.elev_p );
     if ( key.elev_s  != NULL ) free( key.elev_s );
     if ( key.el_az_p != NULL ) free( key.el_az_p );
     if ( key.el_az_s != NULL ) free( key.el_az_s );
     if ( key.brdf_p  != NULL ) free( key.brdf_p );
     if ( key.brdf_s  != NULL ) free( key.brdf_s );
     return num_rsp;
}

/*+++++++++++++++++++++++++
.IDENTifer   InterpolRSPN
.PURPOSE     calculate radiance sensitivity (nadir) for give pixel & esm-angle
.INPUT/OUTPUT
  call as    radsens = InterpolRSPN( id, ang_esm, rspn );
     input:
	    unsigned short   id       : pixel ID [0:8191]
	    float            ang_esm : mirror elevation angle
	    struct rspn_scia *rspn    : radiance sensitivity parameters (Nadir)
                                        pointer to rspn record with angl_esm
                                        smaller than parameter ang_esm
            
.RETURNS     radiance sensitivity [pixelID,ang_esm]
.COMMENTS    static function
-------------------------*/
static inline
double InterpolRSPN( unsigned short id, double ang_esm,
		     const struct rspn_scia *rspn )
{
     double frac = (ang_esm - (double) rspn->ang_esm)
	  / ((double) rspn[1].ang_esm - (double) rspn->ang_esm);

     return 
	  (1 - frac) * rspn->sensitivity[id] + frac * rspn[1].sensitivity[id];
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_RadSensNadir
.PURPOSE     apply radiance sensitivity correction (Nadir)
.INPUT/OUTPUT
  call as    Apply_RadSensNadir( intg, num_rsp, rspn, mds_1c );
     input:
            float             intg    : integration time (seconds)
            unsigned short    num_rsp : number of RSPN records
	    struct rspn_scia  *rspn   : radiance sensitivity parameters (Nadir)
 in/output:
            struct mds1c_scia *mds_1c : level 1c MDS record

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void Apply_RadSensNadir( float intg, unsigned short num_rsp, 
			 const struct rspn_scia  *rspn,
			 struct mds1c_scia *mds_1c )
{
     register unsigned short nobs = 0;

     register double angEsm;
     register float  *signal = mds_1c->pixel_val;

     do {
	  register unsigned short nr = 0;
	  register unsigned short np = 0;

	  switch( light_path ) {
	  case MONI_SOLAR_PATH:
	       angEsm = 3 * alpha0_esm - 0.5 * mds_1c->geoC[nobs].pos_esm;
	       break;
	  case MONI_NADIR_PATH:
	       angEsm = -1 * (alpha0_esm + 0.5 * mds_1c->geoC[nobs].pos_esm);
	       break;
	  default:
	       angEsm = alpha0_esm + 0.5 * mds_1c->geoN[nobs].pos_esm;
	       break;
	  }
	  /* find rspn record with ang_esm just smaller than angEsm */
	  do {
	       if ( angEsm >= (double) rspn[nr].ang_esm ) break;
	  } while ( ++nr < num_rsp );
	  if ( nr == num_rsp ) 
	       nr -= 2;
	  else if ( nr > 0 ) 
	       nr--;

	  do {
	       register unsigned short id  = mds_1c->pixel_ids[np];
	       register double         val = (double) (*signal) / intg;

	       *signal = (float) (val / InterpolRSPN( id, angEsm, rspn+nr ));
	  } while ( ++signal, ++np < mds_1c->num_pixels );
     } while ( ++nobs < mds_1c->num_obs );
}

/*+++++++++++++++++++++++++ Static Functions (RSPLO) +++++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   Get_RadSensLimb
.PURPOSE     Obtain Radiation Sensitivity Parameters for wavelength grid
.INPUT/OUTPUT
  call as    Get_RadSensLimb( fileParam, wvlen, &rspl );
     input:
            struct file_rec *fileParam: file/calibration parameters
	    struct wvlen_rec wvlen    : Solar/Science wavelength grid
    output:
            struct rsplo_scia **rspl  : radiance sensitivity parameters (Limb)
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
unsigned short Get_RadSensLimb( const struct file_rec *fileParam, 
				const struct wvlen_rec wvlen, 
				struct rsplo_scia **rspl_out )
      /*@globals  errno, nadc_stat, nadc_err_stack;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp, *rspl_out@*/
{
     const char prognm[] = "Get_RadSensLimb";

     register unsigned short n_ch, nr;

     unsigned short num_rsp = 0;

     double rbuff[CHANNEL_SIZE];

     struct rsplo_scia  *rspl;

     rspl_out[0] = NULL;

     num_rsp = (unsigned short) SCIA_LV1_RD_RSPL( fileParam->fp, 
						  fileParam->num_dsd, 
						  fileParam->dsd, &rspl );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "RSPL" );
/*
 * interpolate sensitivities to wavelength grid (when do_pixelwise equals 'f')
 */
     for ( n_ch = 0; n_ch < SCIENCE_CHANNELS; n_ch++ ) {
	  if ( do_pixelwise[n_ch] == 'f' ) {
	       unsigned short offs = n_ch * CHANNEL_SIZE;

	       for ( nr = 0; nr < num_rsp; nr++ ) {
		    (void) memcpy( rbuff, &rspl[nr].sensitivity[offs],
				   CHANNEL_SIZE * sizeof(double) );

		    FIT_GRID_AKIMA( FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.solar + offs, rbuff,
				    FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.science + offs,
				    &rspl[nr].sensitivity[offs] );
	       }
	  }
     }
/*
 * everything went fine, so return the data...
 */
     rspl_out[0] = rspl;
 done:
     return num_rsp;
}

/*+++++++++++++++++++++++++
.IDENTifer   Get_RadSensOccul
.PURPOSE     Obtain Radiation Sensitivity Parameters for wavelength grid
.INPUT/OUTPUT
  call as    Get_RadSensOccul( fileParam, wvlen, &rspo );
     input:
            struct file_rec *fileParam: file/calibration parameters
	    struct wvlen_rec wvlen    : Solar/Science wavelength grid
    output:
            struct rsplo_scia **rspo  : radiance sensitivity parameters (Occul)
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
unsigned short Get_RadSensOccul( const struct file_rec *fileParam, 
				 const struct wvlen_rec wvlen, 
				 struct rsplo_scia **rspo_out )
      /*@globals  errno, nadc_stat, nadc_err_stack;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack, fileParam->fp, *rspo_out@*/
{
     const char prognm[] = "Get_RadSensOccul";

     register unsigned short n_ch, nr;

     unsigned short num_rsp = 0;

     struct rsplo_scia  *rspo;

     rspo_out[0] = NULL;

     num_rsp = (unsigned short) SCIA_LV1_RD_RSPO( fileParam->fp, 
						  fileParam->num_dsd, 
						  fileParam->dsd, &rspo );
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_PDS_RD, "RSPO" );
/*
 * interpolate sensitivities to wavelength grid (when do_pixelwise equals 'f')
 */
     for ( n_ch = 0; n_ch < SCIENCE_CHANNELS; n_ch++ ) {
	  if ( do_pixelwise[n_ch] == 'f' ) {
	       unsigned short offs = n_ch * CHANNEL_SIZE;
	       double rbuff[CHANNEL_SIZE];

	       for ( nr = 0; nr < num_rsp; nr++ ) {
		    (void) memcpy( rbuff, &rspo[nr].sensitivity[offs],
				   CHANNEL_SIZE * sizeof(double) );

		    FIT_GRID_AKIMA( FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.solar + offs, rbuff,
				    FLT32_T, FLT64_T, CHANNEL_SIZE,
				    wvlen.science + offs,
				    &rspo[nr].sensitivity[offs] );
	       }
	  }
     }
/*
 * everything went fine, so return the data...
 */
     rspo_out[0] = rspo;
 done:
     return num_rsp;
}

/*+++++++++++++++++++++++++
.IDENTifer   Get_H5_RadSensLimb
.PURPOSE     Obtain Radiation Sensitivity Parameters for wavelength grid
.INPUT/OUTPUT
  call as    Get_H5_RadSensLimb( NDF, wvlen, &rspl );
     input:
            bool NDF                   : neutral density filter flag
	    float *wvlen               : wavelength grid
    output:
            struct rsplo_scia **rspl   : radiance sensitivity parameters (Limb)
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
unsigned short Get_H5_RadSensLimb( bool NDF, const float *wvlen, 
				   struct rsplo_scia **rspl_out )
      /*@globals  errno, nadc_stat, nadc_err_stack;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack, *rspl_out@*/
{
     const char prognm[] = "Get_H5_RadSensLimb";

     register unsigned short np, nr;

     unsigned short n_azi, n_elev;
     unsigned short num_rsp = 0;

     float  tmpAzi;
     double elev_a0[SCIENCE_PIXELS], abs_rad[SCIENCE_PIXELS], 
	  obm_s_p[SCIENCE_PIXELS];
     double el_az_p[SCIENCE_PIXELS], el_az_s[SCIENCE_PIXELS];

     struct rspd_key   key;
     struct rsplo_scia *rspl;
/*
 * initialize return values
 */
     key.elev_p  = NULL;
     key.elev_s  = NULL;
     key.el_az_p = NULL;
     key.el_az_s = NULL;
     key.brdf_p  = NULL;
     key.brdf_s  = NULL;
     rspl_out[0] = NULL;
/*
 * read radiance calibration keydata from HDF5 file (origin Ife)
 */
     SCIA_RD_H5_RSPD( &key );
/*
 * calculate elev_i_alpha0, abs_rad and obm_s_p
 */
     Calc_RadSensStatic( NDF, wvlen, &key, elev_a0, abs_rad, obm_s_p );
/*
 * allocate memory for the RSPL records
 */
     if ( key.n_el_az == 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rspl" );
     rspl = (struct rsplo_scia *) 
	  malloc( key.n_el_az * sizeof(struct rsplo_scia) );
     if ( rspl == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rspl" );
     num_rsp = key.n_el_az;
/*
 * interpolate sensitivities to wavelength grid 
 * also: rearrange data [zen,azi] -> [azi,zen], both azi,elev decreasing
 */
     n_elev = 1;
     tmpAzi = key.el_az_p->azimuth_angle;
     for ( nr = 1; nr < num_rsp; nr++ )
	  if ( key.el_az_p[nr].azimuth_angle == tmpAzi ) n_elev++;
     n_azi = key.n_el_az / n_elev;

     nr = 0;
     do {
	  register unsigned short nrr  = (nr+1) % n_azi;
	  register unsigned short indx;

	  /* elevation angle is already in decreasing order */
	  indx = nr / n_elev  + nrr * n_elev;

	  FIT_GRID_AKIMA( FLT32_T, FLT32_T, 
			  key.el_az_p[indx].n_wl, key.el_az_p[indx].wl,
			  key.el_az_p[indx].sensitivity,
			  FLT32_T, FLT64_T, 
			  SCIENCE_PIXELS, wvlen, el_az_p );

	  FIT_GRID_AKIMA( FLT32_T, FLT32_T, 
			  key.el_az_s[indx].n_wl, key.el_az_s[indx].wl,
			  key.el_az_s[indx].sensitivity,
			  FLT32_T, FLT64_T, 
			  SCIENCE_PIXELS, wvlen, el_az_s );

	  rspl[nr].ang_esm = key.el_az_p[indx].elevat_angle;
	  rspl[nr].ang_asm  = key.el_az_p[indx].azimuth_angle;
	  for ( np = 0; np < SCIENCE_PIXELS; np++ )
	       rspl[nr].sensitivity[np] = 
		    (abs_rad[np] * 
		     (obm_s_p[np] * el_az_s[np] + el_az_p[np]) / elev_a0[np]);
     } while( ++nr < num_rsp );
#ifdef DEBUG
     WRITE_H5_RSPLO( "scia_key_rspl.h5", "rspl", num_rsp, rspl );
#endif
/*
 * everything went fine, so return the data...
 */
     rspl_out[0] = rspl;
 done:
     if ( key.elev_p  != NULL ) free( key.elev_p );
     if ( key.elev_s  != NULL ) free( key.elev_s );
     if ( key.el_az_p != NULL ) free( key.el_az_p );
     if ( key.el_az_s != NULL ) free( key.el_az_s );
     if ( key.brdf_p  != NULL ) free( key.brdf_p );
     if ( key.brdf_s  != NULL ) free( key.brdf_s );
     return num_rsp;
}

/*+++++++++++++++++++++++++
.IDENTifer   Get_H5_RadSensMoni
.PURPOSE     Obtain Radiation Sensitivity Parameters for wavelength grid
.INPUT/OUTPUT
  call as    Get_H5_RadSensMoni( NDF, wvlen, &rspm );
     input:
            bool NDF                   : neutral density filter flag
	    float *wvlen               : wavelength grid
    output:
            struct rsplo_scia **rspm   : radiance sensitivity parameters (Moni)
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
unsigned short Get_H5_RadSensMoni( bool NDF, /*@null@*/ const float *wvlen_in, 
				   struct rsplo_scia **rspm_out )
      /*@globals  errno, nadc_stat, nadc_err_stack;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack, *rspm_out@*/
{
     const char prognm[] = "Get_H5_RadSensMoni";

     register unsigned short np, nr, offs;

     unsigned short n_azi, n_elev;
     unsigned short num_rsp = 0;

     float  tmpAzi;
     float wvlen[SCIENCE_PIXELS];
     double elev_a0[SCIENCE_PIXELS], abs_rad[SCIENCE_PIXELS], 
	  obm_s_p[SCIENCE_PIXELS], brdf_p[SCIENCE_PIXELS], 
	  brdf_s[SCIENCE_PIXELS];

     struct rspd_key   key;
     struct rsplo_scia *rspm;
/*
 * initialize return values
 */
     key.elev_p  = NULL;
     key.elev_s  = NULL;
     key.el_az_p = NULL;
     key.el_az_s = NULL;
     key.brdf_p  = NULL;
     key.brdf_s  = NULL;
     rspm_out[0] = NULL;
/*
 * read radiance calibration keydata from HDF5 file (origin Ife)
 */
     SCIA_RD_H5_RSPD( &key );
/*
 * check wavelength grid
 */
     if ( wvlen_in == NULL )
	  (void) memcpy( wvlen, key.key_fix.wl, SCIENCE_PIXELS * sizeof(float));
     else
	  (void) memcpy( wvlen, wvlen_in, SCIENCE_PIXELS * sizeof(float) );
/*
 * calculate elev_i_alpha0, abs_rad and obm_s_p
 */
     Calc_RadSensStatic( NDF, wvlen, &key, elev_a0, abs_rad, obm_s_p );
/*
 * allocate memory for the RSPM records
 */
     if ( key.n_brdf == 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rspm" );
     rspm = (struct rsplo_scia *) 
	  malloc( key.n_brdf * sizeof(struct rsplo_scia) );
     if ( rspm == NULL ) NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "rspm" );
     num_rsp = key.n_brdf;
/*
 * interpolate sensitivities to wavelength grid 
 * also: rearrange data [elev,azi] -> [azi,elev], both azi,elev decreasing
 */
     n_elev = 1;
     tmpAzi = key.brdf_p->asm_angle;
     for ( nr = 1; nr < num_rsp; nr++ )
	  if ( key.brdf_p[nr].asm_angle == tmpAzi ) n_elev++;
     n_azi = key.n_brdf / n_elev;

     offs = n_elev;
     nr = 0;
     do {
	  register unsigned short nrr  = nr % n_azi;
	  register unsigned short indx;

	  if ( nrr == 0 ) --offs;
	  indx = offs + nrr * n_elev;

	  FIT_GRID_AKIMA( FLT32_T, FLT32_T, 
			  key.brdf_p[indx].n_wl, key.brdf_p[indx].wl,
			  key.brdf_p[indx].sensitivity,
			  FLT32_T, FLT64_T, 
			  SCIENCE_PIXELS, wvlen, brdf_p );

	  FIT_GRID_AKIMA( FLT32_T, FLT32_T, 
			  key.brdf_s[indx].n_wl, key.brdf_s[indx].wl,
			  key.brdf_s[indx].sensitivity,
			  FLT32_T, FLT64_T, 
			  SCIENCE_PIXELS, wvlen, brdf_s );

	  rspm[nr].ang_esm = key.brdf_p[indx].elevat_angle;
	  rspm[nr].ang_asm  = key.brdf_p[indx].asm_angle;
	  for ( np = 0; np < SCIENCE_PIXELS; np++ )
	       rspm[nr].sensitivity[np] = 
		    (abs_rad[np] 
		     * (obm_s_p[np] * brdf_s[np] + brdf_p[np]) / elev_a0[np]);
     } while( ++nr < num_rsp );
#ifdef DEBUG
     WRITE_H5_RSPLO( "scia_key_rspm.h5", "rspm", num_rsp, rspm );
#endif
/*
 * everything went fine, so return the data...
 */
     rspm_out[0] = rspm;
 done:
     if ( key.elev_p  != NULL ) free( key.elev_p );
     if ( key.elev_s  != NULL ) free( key.elev_s );
     if ( key.el_az_p != NULL ) free( key.el_az_p );
     if ( key.el_az_s != NULL ) free( key.el_az_s );
     if ( key.brdf_p  != NULL ) free( key.brdf_p );
     if ( key.brdf_s  != NULL ) free( key.brdf_s );
     return num_rsp;
}

/*+++++++++++++++++++++++++
.IDENTifer   Get_Matrix_RSPLO
.PURPOSE     create a 2d-matrix to a rsplo structure
.INPUT/OUTPUT
  call as   mtx = Get_Matrix_RSPLO( num_dsr, rsplo, &n_asm, &n_esm );
     input:
            unsigned short num_dsr   : number of RSPLO records
            struct rsplo_scia *rsplo : RSPLO structures
    output:
            unsigned short *n_asm    : number of distinct azimuth angles
            unsigned short *n_esm    : number of distinct elevation angles

.RETURNS     pointer to struct rsplo_scia
.COMMENTS    static function
-------------------------*/
static /*@null@*/ /*@out@*/ /*@only@*/ const
struct rsplo_scia **Get_Matrix_RSPLO( unsigned short num_dsr,
                                      const struct rsplo_scia *rsplo,
                                      /*@out@*/ unsigned short *n_asm,
                                      /*@out@*/ unsigned short *n_esm )
     /*@globals  nadc_stat, nadc_err_stack;@*/
{
     const char prognm[] = "Get_Matrix_RSPLO";

     register unsigned short nr, offs;

     const float tmpAzi  = rsplo->ang_asm;
     const float tmpElev = rsplo->ang_esm;

     const struct rsplo_scia **mtx_rsplo;

     *n_asm = *n_esm = 0;
     for ( nr = 0; nr < num_dsr; nr++ ) {
          if ( rsplo[nr].ang_asm == tmpAzi  ) (*n_esm)++;
          if ( rsplo[nr].ang_esm == tmpElev ) (*n_asm)++;
     }
     if ( (*n_esm) == 0 || (*n_asm) == 0 )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mtx_rsplo" );
/*
 * allocate pointers to rows
 */
     mtx_rsplo = (const struct rsplo_scia **)
          malloc( (*n_esm) * sizeof( const struct rsplo_scia * ) );
     if ( mtx_rsplo == NULL )
          NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "mtx_rsplo" );
/*
 * set pointes to rows
 */
     nr = offs = 0u;
     do {
          mtx_rsplo[nr] = rsplo + offs;
          offs += (*n_asm);
     } while ( ++nr < (*n_esm) );
     return mtx_rsplo;
 done:
     return NULL;
}

/*+++++++++++++++++++++++++
.IDENTifer   InterpolRSPLO
.PURPOSE     calculate radiance sensitivity for give pixel & esm/asm-angle
.INPUT/OUTPUT
  call as    radsens = InterpolRSPLO( id, ang_asm, ang_esm,
                                      psplo_el_mn, psplo_el_mx );
     input:
	    unsigned short   id       : pixel ID [0:8191]
	    double           ang_asm  : mirror azimuth angle
	    double           ang_esm  : mirror elevation angle
            struct rsplo_scia *rsplo_mn : RSPLO structure for elev_mn
            struct rsplo_scia *rsplo_mx : RSPLO structure for elev_mx
            
.RETURNS     radiance sensitivity [pixelID,ang_asm,ang_esm]
.COMMENTS    static function
-------------------------*/
static inline
double InterpolRSPLO( unsigned short id, double ang_asm, double ang_esm,
		      const struct rsplo_scia *rsplo_el_mn, 
		      const struct rsplo_scia *rsplo_el_mx )
{
     double frac, radsensAzi1, radsensAzi2;

     frac = (double) (ang_asm - rsplo_el_mn->ang_asm) 
	  / (double) (rsplo_el_mn[1].ang_asm - rsplo_el_mn->ang_asm);
     radsensAzi1 = (1 - frac) * rsplo_el_mn->sensitivity[id] 
	  + frac * rsplo_el_mn[1].sensitivity[id];

     frac = (double) (ang_asm - rsplo_el_mx->ang_asm) 
	  / (double) (rsplo_el_mx[1].ang_asm - rsplo_el_mx->ang_asm);
     radsensAzi2 = (1 - frac) * rsplo_el_mx->sensitivity[id] 
	  + frac * rsplo_el_mx[1].sensitivity[id];

     frac = (double) (ang_esm - rsplo_el_mn->ang_esm) 
	  / (double) (rsplo_el_mx->ang_esm - rsplo_el_mn->ang_esm);
     return ((1 - frac) * radsensAzi1 + frac * radsensAzi2);
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_RadSensLimb
.PURPOSE     apply radiance sensitivity correction (Limb)
.INPUT/OUTPUT
  call as    Apply_RadSensLimb( intg, num_rsp, rspl, mds_1c );
     input:
            float             intg    : integration time (seconds)
            unsigned short    num_rsp : number of RSPL records
	    struct rsplo_scia  *rspl  : radiance sensitivity parameters (Limb)
 in/output:
            struct mds1c_scia *mds_1c : level 1c MDS record

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void Apply_RadSensLimb( float intg, unsigned short num_rsp, 
			const struct rsplo_scia  *rspl,
			struct mds1c_scia *mds_1c )
{
     register unsigned short na, ne, nobs = 0;

     register double angAsm, angEsm;
     register float  *signal = mds_1c->pixel_val;

     unsigned short n_asm, n_esm;

     const struct rsplo_scia 
	  **mtx_rspl = Get_Matrix_RSPLO( num_rsp, rspl, &n_asm, &n_esm );

     do {
	  register unsigned short np = 0;

	  switch( light_path ) {
	  case MONI_SOLAR_PATH:
	       angAsm = alpha0_asm - 0.5 * mds_1c->geoC[nobs].pos_asm;
	       angEsm = mds_1c->geoC[nobs].sun_zen_ang - 90.;
	       break;
	  case MONI_LIMB_PATH:
	       angAsm = alpha0_asm - 0.5 * mds_1c->geoC[nobs].pos_asm;
	       angEsm = alpha0_esm + 0.5 * mds_1c->geoC[nobs].pos_esm;
	       break;
	  case MONI_EXTRAMIR_PATH:
	       angAsm = alpha0_asm - 0.5 * mds_1c->geoC[nobs].pos_asm;
	       angEsm = -12.7;
	       break;
	  default:
	       angAsm = alpha0_asm - 0.5 * mds_1c->geoL[nobs].pos_asm;
	       angEsm = alpha0_esm + 0.5 * mds_1c->geoL[nobs].pos_esm;
	       break;
	  }
          /* find rsplo record with ang_esm just smaller than angEsm */
	  ne = 0;
          do {
               if ( angEsm >= (double) mtx_rspl[ne][0].ang_esm ) break;
          } while ( ++ne < n_esm );
          if ( ne == n_esm ) 
	       ne -= 2;
          else if ( ne > 0 ) 
	       ne--;

          /* find rsplo record with ang_asm just smaller than angAsm */
	  na = 0;
          do {
               if ( angAsm >= (double) mtx_rspl[ne][na].ang_asm ) break;
          } while ( ++na < n_asm );
          if ( na == n_asm ) 
	       na -= 2;
          else if ( na > 0 ) 
	       na--;
	  do {
	       register unsigned short id  = mds_1c->pixel_ids[np];
	       register double         val = (double) (*signal) / intg;

	       *signal = (float) 
		    (val / InterpolRSPLO( id, angAsm, angEsm,
					  mtx_rspl[ne]+na, mtx_rspl[ne+1]+na ));
	  } while ( ++signal, ++np < mds_1c->num_pixels );
     } while ( ++nobs < mds_1c->num_obs );

     free( mtx_rspl );
}

/*+++++++++++++++++++++++++
.IDENTifer   Apply_RadSensOccul
.PURPOSE     apply radiance sensitivity correction (Occul)
.INPUT/OUTPUT
  call as    Apply_RadSensOccul( intg, num_rsp, rspo, mds_1c );
     input:
            float             intg    : integration time (seconds)
            unsigned short    num_rsp : number of RSPO records
	    struct rsplo_scia  *rspo  : radiance sensitivity parameters (Occul)
 in/output:
            struct mds1c_scia *mds_1c : level 1c MDS record

.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void Apply_RadSensOccul( float intg, unsigned short num_rsp, 
			 const struct rsplo_scia  *rspo,
			 struct mds1c_scia *mds_1c )
{
     register unsigned short na, ne, nobs = 0;

     register float *signal = mds_1c->pixel_val;

     unsigned short n_esm, n_asm;

     const struct rsplo_scia 
	  **mtx_rspo = Get_Matrix_RSPLO( num_rsp, rspo, &n_asm, &n_esm );

     do {
	  register unsigned short np = 0;

	  double angAsm = alpha0_asm - 0.5 * mds_1c->geoL[nobs].pos_asm;
	  double angEsm = alpha0_esm + 0.5 * mds_1c->geoL[nobs].pos_esm;

          /* find rsplo record with ang_esm just smaller than angEsm */
	  ne = 0;
          do {
               if ( angEsm >= (double) mtx_rspo[ne][0].ang_esm ) break;
          } while ( ++ne < n_esm );
          if ( ne == n_esm ) 
	       ne -= 2;
          else if ( ne > 0 ) 
	       ne--;

          /* find rsplo record with ang_asm just smaller than angAsm */
	  na = 0;
          do {
               if ( angAsm >= (double) mtx_rspo[ne][na].ang_asm ) break;
          } while ( ++na < n_asm );
          if ( na == n_asm ) 
	       na -= 2;
          else if ( na > 0 ) 
	       na--;

	  do {
	       register unsigned short id  = mds_1c->pixel_ids[np];
	       register double         val = (double) (*signal) / intg;

	       *signal = (float)
		    (val / InterpolRSPLO( id, angAsm, angEsm,
					  mtx_rspo[ne]+na, mtx_rspo[ne+1]+na ));
	  } while ( ++signal, ++np < mds_1c->num_pixels );
     } while ( ++nobs < mds_1c->num_obs );

     free( mtx_rspo );
}

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   SCIA_ATBD_CAL_RAD
.PURPOSE     perform Radiance correction on Sciamachy L1b science data
.INPUT/OUTPUT
  call as   SCIA_ATBD_CAL_RAD( fileParam, wvlen, state, mds_1c );
     input:  
             struct file_rec *fileParam : file/calibration parameters
	     struct wvlen_rec wvlen     : Solar and science wavelength grid
	     struct state1_scia *state  : structure with States of the product
 in/output:  
             struct mds1c_scia *mds_1c  : level 1c MDS records

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    None
-------------------------*/
void SCIA_ATBD_CAL_RAD( const struct file_rec *fileParam,
			const struct wvlen_rec wvlen,
			const struct state1_scia *state,
			struct mds1c_scia *mds_1c )
      /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
{
     const char prognm[] = "SCIA_ATBD_CAL_RAD";

     register unsigned short nr, np, num;

     unsigned short num_rsp = 0;

     bool NDF;

     struct rspn_scia  *rspn = NULL;
     struct rsplo_scia *rspm = NULL;
     struct rsplo_scia *rspl = NULL;
     struct rsplo_scia *rspo = NULL;

     const bool Save_Extern_Alloc = Use_Extern_Alloc;
/*
 * set some global variables
 */
     light_path = DEFAULT_PATH;
     alpha0_asm = fileParam->alpha0_asm - 360.f;
     alpha0_esm = fileParam->alpha0_esm - 360.f;
     (void) memcpy( do_pixelwise, fileParam->do_pixelwise, SCIENCE_CHANNELS+1);
/*
 * read/interpolate radiance sensitivity parameters
 */
     Use_Extern_Alloc = FALSE;
     switch ( (int) state->type_mds ) {
     case SCIA_NADIR:
	  if ( (fileParam->calibFlag & DO_KEYDATA_RAD) != UINT_ZERO )
	       num_rsp = Get_H5_RadSensNadir( FALSE, wvlen.science, &rspn );
	  else 
	       num_rsp = Get_RadSensNadir( fileParam, wvlen, &rspn );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "Get_RadSensNadir" );

          /* apply m-factor to radiance sensitivity */
	  if ( (fileParam->calibFlag & DO_MFACTOR_RAD) != UINT_ZERO ) {
	       float mfactor[SCIENCE_PIXELS];

	       SCIA_RD_MFACTOR( M_DN, fileParam->sensing_start, 
				fileParam->calibFlag, mfactor );

	       for ( nr = 0; nr < num_rsp; nr++ ) {
		    for ( np = 0; np < SCIENCE_PIXELS; np++ )
			 rspn[nr].sensitivity[np] /= mfactor[np];
	       }
	  }

	  /* apply radiance sensitivity correction */
	  num = 0;
	  do {
	       Apply_RadSensNadir( getCorrIntg( state->Clcon[num] ), 
				   num_rsp, rspn, mds_1c );
	  } while ( ++mds_1c, ++num < state->num_clus );

	  if ( rspn != NULL ) free( rspn );
	  break;
     case SCIA_LIMB:
	  if ( (fileParam->calibFlag & DO_KEYDATA_RAD) != UINT_ZERO )
	       num_rsp = Get_H5_RadSensLimb( FALSE, wvlen.science, &rspl );
	  else
	       num_rsp = Get_RadSensLimb( fileParam, wvlen, &rspl );
	  if ( IS_ERR_STAT_FATAL )
	    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "Get_RadSensLimb" );

          /* apply m-factor to radiance sensitivity */
	  if ( (fileParam->calibFlag & DO_MFACTOR_RAD) != UINT_ZERO ) {
	       float mfactor[SCIENCE_PIXELS];

	       SCIA_RD_MFACTOR( M_DL, fileParam->sensing_start, 
				fileParam->calibFlag, mfactor );

	       for ( nr = 0; nr < num_rsp; nr++ ) {
		    for ( np = 0; np < SCIENCE_PIXELS; np++ )
			 rspl[nr].sensitivity[np] /= mfactor[np];
	       }
	  }

          /* apply radiance sensitivity correction */
	  num = 0;
	  do {
	       Apply_RadSensLimb( getCorrIntg( state->Clcon[num] ), 
				  num_rsp, rspl, mds_1c );
	  } while ( ++mds_1c, ++num < state->num_clus );
 
	  if ( rspl != NULL ) free( rspl );
	  break;
     case SCIA_OCCULT:
	  if ( (fileParam->calibFlag & DO_KEYDATA_RAD) != UINT_ZERO )
	       num_rsp = Get_H5_RadSensLimb( TRUE, wvlen.science, &rspo );
	  else
	       num_rsp = Get_RadSensOccul( fileParam, wvlen, &rspo );
	  if ( IS_ERR_STAT_FATAL )
	    NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "Get_RadSensOccul" );

          /* apply m-factor to radiance sensitivity */
	  if ( (fileParam->calibFlag & DO_MFACTOR_RAD) != UINT_ZERO ) {
	       float mfactor[SCIENCE_PIXELS];

	       SCIA_RD_MFACTOR( M_DL, fileParam->sensing_start, 
				fileParam->calibFlag, mfactor );

	       for ( nr = 0; nr < num_rsp; nr++ ) {
		    for ( np = 0; np < SCIENCE_PIXELS; np++ )
			 rspl[nr].sensitivity[np] /= mfactor[np];
	       }
	  }

          /* apply radiance sensitivity correction */
	  num = 0;
	  do {
	       Apply_RadSensOccul( getCorrIntg( state->Clcon[num] ), 
				   num_rsp, rspo, mds_1c );
	  } while ( ++mds_1c, ++num < state->num_clus );
 
	  if ( rspo != NULL ) free( rspo );
	  break;
     case SCIA_MONITOR:
	  switch ( state->state_id ) {
	      /* ASM-mirror - ESM-mirror - ExtraMirror - ESM-Mirror 
                  only ASM-mirror is moving, ESM fixed, 
		  treating as limb is the best way to do it */
	  case 64:
	  case 66:
	  case 68:
	       /* Limb light path */
	  case 54:		/* lunar occultation calibration */
	  case 55: 		/* lunar occultation */
	  case 56:              /* lunar occultation */
	  case 57:              /* lunar occultation */
	  case 8 :		/* Dark Current */
	  case 26:		/* Dark Current */
	  case 46:		/* Dark Current */
	  case 63:		/* Dark Current */
	  case 67:		/* Dark Current */
	       if ( state->state_id == 64 || state->state_id == 66
		    || state->state_id == 68 ) {
		    light_path = MONI_EXTRAMIR_PATH;
		    NDF = TRUE;
	       } else {
		    light_path = MONI_LIMB_PATH;
		    NDF = FALSE;
	       }
	       if ( (fileParam->calibFlag & DO_KEYDATA_RAD) != UINT_ZERO )
		    num_rsp = Get_H5_RadSensLimb( NDF, wvlen.science, &rspl );
	       else
		    num_rsp = Get_RadSensLimb( fileParam, wvlen, &rspl );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR(prognm, NADC_ERR_FATAL, "Get_RadSensLimb");

               /* apply m-factor to radiance sensitivity */
	       if ( (fileParam->calibFlag & DO_MFACTOR_RAD) != UINT_ZERO ) {
		    float mfactor[SCIENCE_PIXELS];

		    SCIA_RD_MFACTOR( M_DL, fileParam->sensing_start, 
				     fileParam->calibFlag, mfactor );

		    for ( nr = 0; nr < num_rsp; nr++ ) {
			 for ( np = 0; np < SCIENCE_PIXELS; np++ )
			      rspl[nr].sensitivity[np] /= mfactor[np];
		    }
	       }

	       /* apply radiance sensitivity correction */
	       num = 0;
	       do {
		    Apply_RadSensLimb( getIntg( state->Clcon[num] ), 
				       num_rsp, rspl, mds_1c );
	       } while ( ++mds_1c, ++num < state->num_clus );
 
	       if ( rspl != NULL ) free( rspl );
	       break;
	       /* Nadir light path */
	  case 60:		/* sub-solar port  */
	  case 58:		/* sub-solar port  */
	  case 53:		/* sub-solar port  */
	       light_path = MONI_SOLAR_PATH;
	       if ( (fileParam->calibFlag & DO_KEYDATA_RAD) != UINT_ZERO ) {
		    num_rsp = 
			 Get_H5_RadSensNadir( TRUE, wvlen.science, &rspn );
	       } else
		    num_rsp = Get_RadSensNadir( fileParam, wvlen, &rspn );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR(prognm,NADC_ERR_FATAL,"Get_RadSensNadir");

               /* apply m-factor to radiance sensitivity */
	       if ( (fileParam->calibFlag & DO_MFACTOR_RAD) != UINT_ZERO ) {
		    float mfactor[SCIENCE_PIXELS];

		    SCIA_RD_MFACTOR( M_DN, fileParam->sensing_start, 
				     fileParam->calibFlag, mfactor );

		    for ( nr = 0; nr < num_rsp; nr++ ) {
			 for ( np = 0; np < SCIENCE_PIXELS; np++ )
			      rspn[nr].sensitivity[np] /= mfactor[np];
		    }
	       }

	       /* apply radiance sensitivity correction */
	       num = 0;
	       do {
		    Apply_RadSensNadir( getIntg( state->Clcon[num] ), 
					num_rsp, rspn, mds_1c );
	       } while ( ++mds_1c, ++num < state->num_clus );

	       if ( rspn != NULL ) free( rspn );
	       break;	       
	  case 48:		/* WLS */
	  case 16:		/* WLS */
	  case 59:		/* SLS */
	  case 39:              /* Dark current WLS */
	  case 61:		/* WLS */
	       light_path = MONI_NADIR_PATH;
	       if ( (fileParam->calibFlag & DO_KEYDATA_RAD) != UINT_ZERO ) {
		    NDF = (state->state_id == 48) ? TRUE : FALSE;
		    num_rsp = 
			 Get_H5_RadSensNadir( NDF, wvlen.science, &rspn );
	       } else
		    num_rsp = Get_RadSensNadir( fileParam, wvlen, &rspn );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR(prognm,NADC_ERR_FATAL,"Get_RadSensNadir");

               /* apply m-factor to radiance sensitivity */
	       if ( (fileParam->calibFlag & DO_MFACTOR_RAD) != UINT_ZERO ) {
		    float mfactor[SCIENCE_PIXELS];

		    SCIA_RD_MFACTOR( M_DN, fileParam->sensing_start, 
				     fileParam->calibFlag, mfactor );

		    for ( nr = 0; nr < num_rsp; nr++ ) {
			 for ( np = 0; np < SCIENCE_PIXELS; np++ )
			      rspn[nr].sensitivity[np] /= mfactor[np];
		    }
	       }

	       /* apply radiance sensitivity correction */
	       num = 0;
	       do {
		    Apply_RadSensNadir( getIntg( state->Clcon[num] ), 
					num_rsp, rspn, mds_1c );
	       } while ( ++mds_1c, ++num < state->num_clus );

	       if ( rspn != NULL ) free( rspn );
	       break;

	       /* calibration light path */
	       /* Only implemented from keydata, therefore use it always */
	  case 62:		/* ESM diffuser */
	  case 52:		/* ESM diffuser */
	       light_path = MONI_SOLAR_PATH;
	       NDF = (state->state_id == 62) ? TRUE : FALSE;
	       num_rsp = Get_H5_RadSensMoni( NDF, wvlen.science, &rspm );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR(prognm, NADC_ERR_FATAL, "Get_RadSensMoni");

               /* apply m-factor to radiance sensitivity */
	       if ( (fileParam->calibFlag & DO_MFACTOR_RAD) != UINT_ZERO ) {
		    float mfactor[SCIENCE_PIXELS];

		    SCIA_RD_MFACTOR( M_CAL, fileParam->sensing_start, 
				     fileParam->calibFlag, mfactor );

		    for ( nr = 0; nr < num_rsp; nr++ ) {
			 for ( np = 0; np < SCIENCE_PIXELS; np++ )
			      rspm[nr].sensitivity[np] /= mfactor[np];
		    }
	       }

	       /* apply radiance sensitivity correction */
	       num = 0;
	       do {
		    Apply_RadSensLimb( getIntg( state->Clcon[num] ), 
				       num_rsp, rspm, mds_1c );
	       } while ( ++mds_1c, ++num < state->num_clus );

	       if ( rspm != NULL ) free( rspm );
	       break;
	  default:		 
	       /* generate a message for states not defined yet */
	       NADC_ERROR( prognm, NADC_ERR_FATAL,
			   "RadSens correction for this Monitoring state"
			   " not implemented" );
	  }
     }
 done:
     Use_Extern_Alloc = Save_Extern_Alloc;
}

/*+++++++++++++++++++++++++
.IDENTifer   SCIA_SMR_CAL_RAD
.PURPOSE     apply radiance sensitivity correction on Solar spectrum
.INPUT/OUTPUT
  call as    SCIA_SMR_CAL_RAD( orbit, channel, angAsm, angEsm, wvlen, smr );
     input:
            unsigned short orbit       : absolute orbit number
	    unsigned short channel     : channel ID [1,2,..,8]
	    float angAsm               : ASM angle
	    float angEsm               : ESM angle
	    float *wvlen               : wavelength grid [all channels]
 in/output:
            float *smr                 : Solar Mean Spectrum

.RETURNS     nothing
.COMMENTS    only works for SDMF (v3.0) SMR derived from state 62 observations
-------------------------*/
void SCIA_SMR_CAL_RAD( unsigned short absOrbit, unsigned short channel, 
		       float angAsm, float angEsm, const float *wvlen, 
		       float *smr )
      /*@globals  errno, nadc_stat, nadc_err_stack, Use_Extern_Alloc;@*/
{
     const char prognm[] = "SCIA_SMR_CAL_RAD";

     const bool NDF = TRUE;
     const bool Save_Extern_Alloc = Use_Extern_Alloc;

     register unsigned short na, ne;

     unsigned short num_rsp = 0;

     unsigned short n_asm, n_esm;

     float pet;

     struct rsplo_scia *rspm = NULL;
     const struct rsplo_scia **mtx_rspm;
/* 
 * Obtain Radiation Sensitivity Parameters for wavelength grid 
 */
     Use_Extern_Alloc = FALSE;
     num_rsp = Get_H5_RadSensMoni( NDF, wvlen, &rspm );
     Use_Extern_Alloc = Save_Extern_Alloc;
     if ( IS_ERR_STAT_FATAL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, "Get_RadSensMoni" );

     /* create 2D-matrix pointing to rspm structure */
     mtx_rspm = Get_Matrix_RSPLO( num_rsp, rspm, &n_asm, &n_esm );

     /* find rspm record with ang_esm just smaller than angEsm */
     ne = 0;
     do {
	  if ( angEsm >= mtx_rspm[ne][0].ang_esm ) break;
     } while ( ++ne < n_esm );
     if ( ne == n_esm ) 
	  ne -= 2;
     else if ( ne > 0 ) 
	  ne--;

     /* find rspm record with ang_asm just smaller than angAsm */
     na = 0;
     do {
	  if ( angAsm >= mtx_rspm[ne][na].ang_asm ) break;
     } while ( ++na < n_asm );
     if ( na == n_asm ) 
	  na -= 2;
     else if ( na > 0 ) 
	  na--;
/*
 *
 */
     if ( channel == 0 ) {
	  register unsigned short np = 0;
	  register unsigned short nch;

	  for ( nch = 1; nch <= SCIENCE_CHANNELS; nch++ ) {
	       register unsigned short ni = (nch-1) * CHANNEL_SIZE;

	       pet = SDMF_get_statePET( 62, absOrbit, nch );
	       if ( nch > 5 ) pet -= 1.18125e-3;

	       do {
		    register double val = (double) smr[np] / pet;

		    smr[np] = (float) 
			 (val / InterpolRSPLO( ni, angAsm, angEsm,
					       mtx_rspm[ne+1]+na, mtx_rspm[ne]+na ));
	       } while ( ++np, ++ni < (nch * CHANNEL_SIZE) );
	  }
     } else {
	  register unsigned short np = 0;
	  register unsigned short ni = (channel-1) * CHANNEL_SIZE;

	  pet = SDMF_get_statePET( 62, absOrbit, channel );
	  if ( channel > 5 ) pet -= 1.18125e-3;

	  do {
	       register double val = (double) smr[np] / pet;

	       smr[np] = (float) 
		    (val / InterpolRSPLO( ni, angAsm, angEsm,
					  mtx_rspm[ne+1]+na, mtx_rspm[ne]+na ));
	  } while ( ++ni, ++np < CHANNEL_SIZE );
     }
     free( mtx_rspm );
done:
     if ( rspm != NULL ) free( rspm );
}
