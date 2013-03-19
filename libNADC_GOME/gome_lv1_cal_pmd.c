/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 1999 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GOME_LV1_CAL_PMD
.AUTHOR      R.M. van Hees
.KEYWORDS    GOME level 1
.LANGUAGE    ANSI C
.PURPOSE     calibrate EARTH or Moon/SUN PMD data
.INPUT/OUTPUT
  call as   CALIB_PCD_PMD( param, fcd, pcd, nr_pmd, pmd );

     input:  
            struct param_record param :  struct holding user-defined settings
	    struct fcd_gome   *fcd  :  Fixed Calibration Data Record
	    struct pcd_gome   *pcd  :  Pixel Specific Calibration Records
	    size_t nr_pmd             :  number of selected PMD structures
 in/output:  
	    struct pmd_gome   *pmd  :  structure with PMD data

  call as   CALIB_SMCD_PMD( param, fcd, smcd, nr_pmd, pmd );
     input:  
            struct param_record param :  struct holding user-defined settings
	    struct fcd_gome   *fcd  :  Fixed Calibration Data Record
	    struct smcd_gome  *smcd :  Sun/Moon Specific Calibration Records
	    size_t nr_pmd             :  number of selected PMD structures
 in/output:  
	    struct pmd_gome   *pmd  :  structure with PMD data

.RETURNS     Nothing
.COMMENTS    None
.ENVIRONment None
.VERSION     2.2     02-Oct-2006   bugfix: possible divide by zero 
                                   (polarization), RvH
             2.1     19-Jul-2001   pass structures using pointers, RvH
             2.0     10-Nov-2000   Store PMD data in struct, RvH
             1.0     10-Mar-2000   Created by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_C_SOURCE 2

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

/*+++++ Local Headers +++++*/
#define _GOME_COMMON
#include <nadc_gome.h>

/*+++++ Macros +++++*/
#define PMD_UNDEFINED    (NAN)

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static
const double Watt2Photon = 1e-9 / (1e+7 * (2.997925e+8 * 6.62618e-34));

/*+++++++++++++++++++++++++
.IDENTifer   CONVERT_PMD_INTENS
.PURPOSE     perform absolute radiance calibration
.INPUT/OUTPUT
  call as    CONVERT_PMD_INTENS( sun_pmd_wv[], sun_pmd[], pmd_wv, pmd );

     input:
	    float sun_pmd_wv[] :  wavelength of the PMD (Sun reference spectra)
	    float sun_pmd[]    :  PMD mean values (Sun reference spectra)
            float pmd_wv       :  wavelength of the PMD mean values
            
.RETURNS     factor to perform absolute radiance calibration
.COMMENTS    static function
-------------------------*/
static inline
float CONVERT_PMD_INTENS( const float sun_pmd_wv[], const float sun_pmd[], 
			  short PMDindx, float pmd_wv )
{
     float ext_pmd[PMD_NUMBER + 2];
     float pmd_fitted, ext_pmd_wv[PMD_NUMBER + 2];

     const float Epsilon_PMD_wv = 10.f;
/*
 * check wavelength of the PMD
 */
     if ( pmd_wv <= 0.f ) return sun_pmd[PMDindx];
/*
 * extent Sun PMD mean wavelength by 2 points
 */
     ext_pmd_wv[0] = sun_pmd_wv[0];
     ext_pmd_wv[1] = sun_pmd_wv[0] + Epsilon_PMD_wv;
     ext_pmd_wv[2] = sun_pmd_wv[1];
     ext_pmd_wv[3] = sun_pmd_wv[2] - Epsilon_PMD_wv;
     ext_pmd_wv[4] = sun_pmd_wv[2];
/*
 * extent Sun PMD mean values by 2 points (linear interpolation)
 */
     ext_pmd[0] = sun_pmd[0];
     ext_pmd[1] = sun_pmd[0] 
	  + (float) (Epsilon_PMD_wv * (sun_pmd[1] - sun_pmd[0]) 
		     / (sun_pmd_wv[1] - sun_pmd_wv[0]));
     ext_pmd[2] = sun_pmd[1];
     ext_pmd[3] = sun_pmd[2]
	  - (float) (Epsilon_PMD_wv * (sun_pmd[2] - sun_pmd[1]) 
		     / (sun_pmd_wv[2] - sun_pmd_wv[1]));
     ext_pmd[4] = sun_pmd[2];
/*
 * do Akima interpolation
 */
     FIT_GRID_AKIMA( FLT32_T, FLT32_T, (size_t) (PMD_NUMBER+2), 
		     ext_pmd_wv, ext_pmd,
		     FLT32_T, FLT32_T, 1, &pmd_wv, &pmd_fitted );

     return pmd_fitted;
}


/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
void CALIB_PCD_PMD( unsigned char write_pmd_geo, unsigned short calib_flag,
		    const struct fcd_gome *fcd, short nr_pcd, 
		    const short *indx_pcd, struct pcd_gome *pcd )
{
     const char prognm[] = "CALIB_PCD_PMD";

     register short  indx, nr, pg, pn;

     register struct pmd_gome *pmd_pntr;
/*
 * check number of records
 */
     if ( nr_pcd == 0 ) return;
/*
 * copy PMD data to structure, includin geolocation data
 */
     GOME_LV1_PMD_GEO( write_pmd_geo, nr_pcd, indx_pcd, pcd );
     if ( IS_ERR_STAT_FATAL ) 
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, "PMD" );
/*
 * check calibration flag
 */
     if ( calib_flag == CALIB_NONE ) return;
/*
 * Apply leakage correction
 */
     if ( (calib_flag & GOME_CAL_LEAK) != USHRT_ZERO ) {
	  for ( nr = 0; nr < nr_pcd; nr++ ){
	       indx = pcd[indx_pcd[nr]].indx_leak;
	       pmd_pntr = pcd[indx_pcd[nr]].pmd;
	       for ( pg = 0; pg < PMD_IN_GRID; pg++ ) {
		    for ( pn = 0; pn < PMD_NUMBER; pn++ ) {
			 pmd_pntr[pg].value[pn] -= 
			      fcd->leak[indx].pmd_offs[pn];
		    }
	       }
	  }
     }
/*
 * Apply polarization correction
 */
     if ( (calib_flag & GOME_CAL_POLAR) != USHRT_ZERO ) {
	  for ( nr = 0; nr < nr_pcd; nr++ ){
	       indx = indx_pcd[nr];
	       pmd_pntr = pcd[indx].pmd;
	       for ( pg = 0; pg < PMD_IN_GRID; pg++ ) {
		    for ( pn = 0; pn < PMD_NUMBER; pn++ ) {
			 if ( pcd[indx].polar.coeff[pn] > FLT_EPSILON
			      && pcd[indx].polar.coeff[pn] < 1.f ) 
			      pmd_pntr[pg].value[pn] /=
				   (2.f * pcd[indx].polar.coeff[pn]);
		    }
	       }
	  }
     }
/*
 * Apply absolute radiance calibration
 */
     if ( (calib_flag & GOME_CAL_INTENS) != USHRT_ZERO ) {
	  for ( pn = 0; pn < PMD_NUMBER; pn++ ) {
	       register short indx_old = -1;
	       register float abs_calib_factor = -1.f;

	       for ( nr = 0; nr < nr_pcd; nr++ ){
		    indx = indx_pcd[nr];
		    pmd_pntr = pcd[indx].pmd;
		    for ( pg = 0; pg < PMD_IN_GRID; pg++ ) {
			 if ( indx != indx_old ) {
			      abs_calib_factor = CONVERT_PMD_INTENS( 
				   fcd->sun_pmd_wv, fcd->sun_pmd, 
				   pn, pcd[indx].polar.wv[pn] );
			      indx_old = indx;
			 }
			 pmd_pntr[pg].value[pn] /= abs_calib_factor;
		    }
	       }
	  }
     }
/*
 * Apply unit conversion
 */
     if ( (calib_flag & GOME_CAL_UNIT) != USHRT_ZERO ) {
	  for ( nr = 0; nr < nr_pcd; nr++ ){
	       indx = indx_pcd[nr];
	       pmd_pntr = pcd[indx].pmd;
	       for ( pg = 0; pg < PMD_IN_GRID; pg++ ) {
		    for ( pn = 0; pn < PMD_NUMBER; pn++ ) {
			 pmd_pntr[pg].value[pn] *= (float)
			      (Watt2Photon * pcd[indx].polar.wv[pn]);
		    }
	       }
	  }
     }
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++*/
void CALIB_SMCD_PMD( unsigned short calib_flag,
		     const struct fcd_gome *fcd, 
		     short nr_smcd, const short *indx_smcd,
		     struct smcd_gome *smcd )
{
     register short  indx, nr, pg, pn;

     register struct pmd_gome *pmd_pntr;
/*
 * check number of records
 */
     if ( nr_smcd == 0 ) return;
/*
 * copy PMD data to structure
 */
     nr = 0;
     do {
	  pg = 0;
	  do {
	       pn = 0;
	       do {
		    smcd[indx_smcd[nr]].pmd[pg].value[pn] = 
			 (float) smcd[indx_smcd[nr]].ihr.pmd[pn][pg];
	       } while ( ++pn < PMD_NUMBER );
	  } while ( ++pg < PMD_IN_GRID );
     } while ( ++nr < nr_smcd );
/*
 * check calibration flag
 */
     if ( calib_flag == CALIB_NONE ) return;
/*
 * Apply leakage correction
 */
     if ( (calib_flag & GOME_CAL_LEAK) != USHRT_ZERO ) {
	  for ( nr = 0; nr < nr_smcd; nr++ ){
	       indx = smcd[indx_smcd[nr]].indx_leak;
	       pmd_pntr = smcd[indx_smcd[nr]].pmd;
	       for ( pg = 0; pg < PMD_IN_GRID; pg++ ) {
		    for ( pn = 0; pn < PMD_NUMBER; pn++ ) {
			 pmd_pntr[pg].value[pn] -= 
			      fcd->leak[indx].pmd_offs[pn];
		    }
	       }
	  }
     }
/*
 * Apply absolute radiance calibration
 */
     if ( (calib_flag & GOME_CAL_INTENS) != USHRT_ZERO ) {
	  for ( pn = 0; pn < PMD_NUMBER; pn++ ) {
	       register float abs_calib_factor = fcd->sun_pmd[pn];

	       for ( nr = 0; nr < nr_smcd; nr++ ){
		    pmd_pntr = smcd[indx_smcd[nr]].pmd;
		    for ( pg = 0; pg < PMD_IN_GRID; pg++ ) {
			 pmd_pntr[pg].value[pn] /= abs_calib_factor;
		    }
	       }
	  }
     }
/*
 * Apply unit conversion
 */
     if ( (calib_flag & GOME_CAL_UNIT) != USHRT_ZERO ) {
	  for ( nr = 0; nr < nr_smcd; nr++ ){
	       pmd_pntr = smcd[indx_smcd[nr]].pmd;
	       for ( pg = 0; pg < PMD_IN_GRID; pg++ ) {
		    for ( pn = 0; pn < PMD_NUMBER; pn++ ) {
			 pmd_pntr[pg].value[pn] *= (float)
			      (Watt2Photon * fcd->sun_pmd_wv[pn]);
		    }
	       }
	  }
     }
}
