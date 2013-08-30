/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2001 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_IDL
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIAMACHY level 1b, IDL
.LANGUAGE    ANSI C
.PURPOSE     IDL wrappers for reading SCIAMACHY level 1b data
.COMMENTS    contains 
.ENVIRONment None
.VERSION      2.6   14-Mar-2011 adopted new method to patch level 1b MDS, RvH
	      2.5   07-Dec-2005 renamed pixel_val_err to pixel_err, RvH
	      2.4   19-Jan-2005 forgot to copy type_mds in LV1C PMD/PolV, RvH
              2.3   06-Oct-2004 modified structure mds1c_scia, RvH
              2.2   12-Oct-2002	consistently return, in case of error, -1, RvH 
              2.1   02-Jul-2002	added more error checking, RvH
              2.0   27-Mar-2002	added routines to read level 1c DSD's, RvH 
              1.0   15-Jan-2002	Created by R. M. van Hees 
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
#define _SCIA_CALIB
#include <nadc_idl.h>

/*+++++ Global Variables +++++*/
extern FILE *fd_nadc;

/*+++++ Static Variables +++++*/
static const char err_msg[] = "invalid number of function arguments";

/*+++++++++++++++++++++++++ Main Program or Functions +++++++++++++++*/
int IDL_STDCALL _SCIA_LV1_RD_ASFP ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_ASFP";

     int nr_asfp;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct asfp_scia *asfp;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     asfp = (struct asfp_scia *) argv[2];

     nr_asfp = (int) SCIA_LV1_RD_ASFP( fd_nadc, num_dsd, dsd, &asfp );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_asfp;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_AUX ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_AUX";

     int nr_aux;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct aux_scia *aux;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd = (struct dsd_envi *) argv[1];
     aux = (struct aux_scia *) argv[2];

     nr_aux = (int) SCIA_LV1_RD_AUX( fd_nadc, num_dsd, dsd, &aux );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_aux;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_BASE ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_BASE";

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct base_scia *base;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     base = (struct base_scia *) argv[2];
     SCIA_LV1_RD_BASE( fd_nadc, num_dsd, dsd, base );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return 1;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_CLCP ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_CLCP";

     int nr_clcp;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct clcp_scia *clcp;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     clcp = (struct clcp_scia *) argv[2];

     nr_clcp = (int) SCIA_LV1_RD_CLCP( fd_nadc, num_dsd, dsd, clcp );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_clcp;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_DARK ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_DARK";

     int nr_dark;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct dark_scia *dark;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     dark = (struct dark_scia *) argv[2];

     nr_dark = (int) SCIA_LV1_RD_DARK( fd_nadc, num_dsd, dsd, &dark );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_dark;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_EKD ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_EKD";

     int nr_ekd;

     unsigned int num_dsd;

     struct dsd_envi *dsd;
     struct ekd_scia *ekd;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     ekd = (struct ekd_scia *) argv[2];

     nr_ekd = (int) SCIA_LV1_RD_EKD( fd_nadc, num_dsd, dsd, ekd );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_ekd;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_LCPN ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_LCPN";

     int nr_lcpn;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct lcpn_scia *lcpn;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     lcpn = (struct lcpn_scia *) argv[2];

     nr_lcpn = (int) SCIA_LV1_RD_LCPN( fd_nadc, num_dsd, dsd, &lcpn );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_lcpn;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_PMD ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_PMD";

     int nr_pmd;

     unsigned int num_dsd;

     struct dsd_envi *dsd;
     struct pmd_scia *pmd;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd = (struct dsd_envi *) argv[1];
     pmd = (struct pmd_scia *) argv[2];

     nr_pmd = (int) SCIA_LV1_RD_PMD( fd_nadc, num_dsd, dsd, &pmd );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_pmd;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_PPG ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_PPG";

     int nr_ppg;

     unsigned int num_dsd;

     struct dsd_envi *dsd;
     struct ppg_scia *ppg;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     ppg = (struct ppg_scia *) argv[2];

     nr_ppg = (int) SCIA_LV1_RD_PPG( fd_nadc, num_dsd, dsd, ppg );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_ppg;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_PPGN ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_PPGN";

     int nr_ppgn;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct ppgn_scia *ppgn;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     ppgn = (struct ppgn_scia *) argv[2];

     nr_ppgn = (int) SCIA_LV1_RD_PPGN( fd_nadc, num_dsd, dsd, &ppgn );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_ppgn;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_PSPN ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_PSPN";

     int nr_pspn;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct pspn_scia *pspn;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     pspn = (struct pspn_scia *) argv[2];

     nr_pspn = (int) SCIA_LV1_RD_PSPN( fd_nadc, num_dsd, dsd, &pspn );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_pspn;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_PSPL ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_PSPL";

     int nr_pspl;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct psplo_scia *pspl;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     pspl = (struct psplo_scia *) argv[2];

     nr_pspl = (int) SCIA_LV1_RD_PSPL( fd_nadc, num_dsd, dsd, &pspl );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_pspl;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_PSPO ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_PSPO";

     int nr_pspo;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct psplo_scia *pspo;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     pspo = (struct psplo_scia *) argv[2];

     nr_pspo = (int) SCIA_LV1_RD_PSPO( fd_nadc, num_dsd, dsd, &pspo );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_pspo;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_RSPN ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_RSPN";

     int nr_rspn;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct rspn_scia *rspn;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     rspn = (struct rspn_scia *) argv[2];

     nr_rspn = (int) SCIA_LV1_RD_RSPN( fd_nadc, num_dsd, dsd, &rspn );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_rspn;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_RSPL ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_RSPL";

     int nr_rspl;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct rsplo_scia *rspl;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     rspl = (struct rsplo_scia *) argv[2];

     nr_rspl = (int) SCIA_LV1_RD_RSPL( fd_nadc, num_dsd, dsd, &rspl );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_rspl;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_RSPO ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_RSPO";

     int nr_rspo;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct rsplo_scia *rspo;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     rspo = (struct rsplo_scia *) argv[2];

     nr_rspo = (int) SCIA_LV1_RD_RSPO( fd_nadc, num_dsd, dsd, &rspo );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_rspo;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_SCP ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_SCP";

     int nr_scp;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct scp_scia *scp;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     scp = (struct scp_scia *) argv[2];

     nr_scp = (int) SCIA_LV1_RD_SCP( fd_nadc, num_dsd, dsd, &scp );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_scp;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_SCPN ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_SCPN";

     int nr_scpn;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct scpn_scia *scpn;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     scpn = (struct scpn_scia *) argv[2];

     nr_scpn = (int) SCIA_LV1_RD_SCPN( fd_nadc, num_dsd, dsd, &scpn );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_scpn;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_SFP ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_SFP";

     int nr_sfp;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct sfp_scia *sfp;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     sfp = (struct sfp_scia *) argv[2];

     nr_sfp = (int) SCIA_LV1_RD_SFP( fd_nadc, num_dsd, dsd, &sfp );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_sfp;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_SIP ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_SIP";

     int nr_sip;

     unsigned int num_dsd;

     struct dsd_envi *dsd;
     struct sip_scia *sip;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     sip = (struct sip_scia *) argv[2];
     nr_sip = SCIA_LV1_RD_SIP( fd_nadc, num_dsd, dsd, sip );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_sip;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_SPH ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_SPH";

     struct mph_envi  mph;
     struct sph1_scia *sph;

     if ( argc != 2 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     mph = *(struct mph_envi *) argv[0];
     sph = (struct sph1_scia *) argv[1];

     if ( IS_ERR_STAT_FATAL ) return -999;
     SCIA_LV1_RD_SPH( fd_nadc, mph, sph );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return 1;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_SQADS ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_SQADS";

     int nr_sqads;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct sqads1_scia *sqads;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd   = (struct dsd_envi *) argv[1];
     sqads = (struct sqads1_scia *) argv[2];
     nr_sqads = (int) SCIA_LV1_RD_SQADS( fd_nadc, num_dsd, dsd, &sqads );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_sqads;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_SRS ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_SRS";

     int nr_srs;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct srs_scia *srs;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     srs = (struct srs_scia *) argv[2];

     nr_srs = (int) SCIA_LV1_RD_SRS( fd_nadc, num_dsd, dsd, &srs );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_srs;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_SRSN ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_SRSN";

     int nr_srsn;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct srsn_scia *srsn;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     srsn = (struct srsn_scia *) argv[2];

     nr_srsn = (int) SCIA_LV1_RD_SRSN( fd_nadc, num_dsd, dsd, &srsn );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_srsn;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_STATE ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_STATE";

     int nr_state;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct state1_scia *state;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     state = (struct state1_scia *) argv[2];
     nr_state = (int) SCIA_LV1_RD_STATE( fd_nadc, num_dsd, dsd, &state );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_state;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_VLCP ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_VLCP";

     int nr_vlcp;

     unsigned int num_dsd;

     struct dsd_envi  *dsd;
     struct vlcp_scia *vlcp;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     vlcp = (struct vlcp_scia *) argv[2];

     nr_vlcp = (int) SCIA_LV1_RD_VLCP( fd_nadc, num_dsd, dsd, &vlcp );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_vlcp;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_MDS ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_MDS";

     register int nr;

     char *env_str = getenv( "SCIA_CORR_LOS" );

     unsigned short     patch_mask = SCIA_PATCH_NONE;

     int    nr_mds, nr_mds1b;
     size_t nr_byte;

     unsigned short     *pixel_ids;
     unsigned int       calib_mask;
     unsigned long long clus_mask;
     float              *pixel_wv, *pixel_wv_err, *pixel_val, *pixel_err;

     struct state1_scia state;
     struct geoC_scia   *geoC;
     struct geoL_scia   *geoL;
     struct geoN_scia   *geoN;
     struct mds1_scia   *C_mds1b;
     struct mds1c_scia  *C_mds;

     struct IDL_mds1c_scia
     {
	  struct mjd_envi  mjd;
	  signed char      rad_units_flag;
	  signed char      quality_flag;
	  unsigned char    type_mds;
	  unsigned char    coaddf;
	  unsigned char    category;
	  unsigned char    state_id;
	  unsigned char    state_index;
	  unsigned char    chan_id;
	  unsigned char    clus_id;
	  unsigned short   dur_scan;
	  unsigned short   num_obs;
	  unsigned short   num_pixels;
	  unsigned int     dsr_length;
	  float            orbit_phase;
	  float            pet;
	  IDL_ULONG        pntr_pixel_ids;     /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_pixel_wv;      /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_pixel_wv_err;  /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_pixel_val;     /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_pixel_err;     /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_geoC;          /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_geoL;          /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_geoN;          /* IDL uses 32-bit addresses */
     } *mds;

     if ( argc != 12 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     state = *(struct state1_scia *) argv[0];
     clus_mask = *(unsigned long long *) argv[1];
     calib_mask = *(unsigned int *) argv[2];
     mds = (struct IDL_mds1c_scia *) argv[3];
     pixel_ids = (unsigned short *) argv[4];
     pixel_wv = (float *) argv[5];
     pixel_wv_err = (float *) argv[6];
     pixel_val = (float *) argv[7];
     pixel_err = (float *) argv[8];
     geoC = (struct geoC_scia *) argv[9];
     geoL = (struct geoL_scia *) argv[10];
     geoN = (struct geoN_scia *) argv[11];
/*
 * read de Measurement Data Sets of one state
 */
     nadc_stat = NADC_STAT_SUCCESS;
     nr_mds1b = (int) SCIA_LV1_RD_MDS( fd_nadc, ~0ULL, &state, &C_mds1b );
     if ( IS_ERR_STAT_FATAL ) return -1;
/*
 * correct the line-of-sight azimuth and zenith angles (geoN)
 */
     if ( env_str != NULL && strcmp( env_str, "1" ) == 0 )
	  SCIA_LV1_CORR_LOS( &state, C_mds1b );
/*
 * set mask to pach the level 1b structures
 */
     if ( (calib_mask & DO_CORR_VIS_MEM) != UINT_ZERO 
	  && (calib_mask & DO_SRON_MEM_NLIN) != UINT_ZERO )
	  patch_mask |= SCIA_PATCH_MEM;
     if ( (calib_mask & DO_CORR_IR_NLIN) != UINT_ZERO 
	  && (calib_mask & DO_SRON_MEM_NLIN) != UINT_ZERO )
	  patch_mask |= SCIA_PATCH_NLIN;
     if ( (calib_mask & DO_CORR_STRAY) != UINT_ZERO 
	  && (calib_mask & DO_SRON_STRAY) != UINT_ZERO )
	  patch_mask |= SCIA_PATCH_STRAY;
     SCIA_LV1_PATCH_MDS( fd_nadc, patch_mask, &state, C_mds1b );
/*
 * copy the data in the level 1b records to level 1c records
 */
     C_mds = (struct mds1c_scia *)
	  malloc( state.num_clus * sizeof( struct mds1c_scia ));
     nr_mds = (int) GET_SCIA_LV1C_MDS( clus_mask, &state, C_mds1b, C_mds );
/*
 * calibrate detector read-outs
 */
     SCIA_LV1_CAL( fd_nadc, calib_mask, &state, C_mds1b, C_mds );
/*
 * release the level 1b C-structures
 */
     SCIA_LV1_FREE_MDS( (int) state.type_mds, nr_mds1b, C_mds1b );
     if ( IS_ERR_STAT_FATAL ) {
	  SCIA_LV1C_FREE_MDS( (int) state.type_mds, nr_mds, C_mds );
	  return -1;
     }
/*
 * copy C-structures to IDL-structures
 */
     for ( nr = 0; nr < nr_mds; nr++ ) {
	  (void) memcpy( &mds[nr].mjd, &C_mds[nr].mjd, 
			 sizeof( struct mjd_envi ) );
	  mds[nr].rad_units_flag = C_mds[nr].rad_units_flag;
	  mds[nr].quality_flag = C_mds[nr].quality_flag;
	  mds[nr].type_mds = C_mds[nr].type_mds;
	  mds[nr].coaddf   = C_mds[nr].coaddf;
	  mds[nr].category = C_mds[nr].category;
	  mds[nr].state_id = C_mds[nr].state_id;
	  mds[nr].state_index = C_mds[nr].state_index;
	  mds[nr].chan_id  = C_mds[nr].chan_id;
	  mds[nr].clus_id  = C_mds[nr].clus_id;
	  mds[nr].dur_scan = C_mds[nr].dur_scan;
	  mds[nr].num_obs  = C_mds[nr].num_obs;
	  mds[nr].num_pixels = C_mds[nr].num_pixels;
	  mds[nr].dsr_length = C_mds[nr].dsr_length;
	  mds[nr].orbit_phase = C_mds[nr].orbit_phase;
	  mds[nr].pet      = C_mds[nr].pet;

	  nr_byte = C_mds[nr].num_pixels * ENVI_USHRT;
	  (void) memcpy( pixel_ids, C_mds[nr].pixel_ids, nr_byte );
	  pixel_ids += C_mds[nr].num_pixels;
	  nr_byte = C_mds[nr].num_pixels * ENVI_FLOAT;
	  (void) memcpy( pixel_wv, C_mds[nr].pixel_wv, nr_byte );
	  pixel_wv += C_mds[nr].num_pixels;
	  (void) memcpy( pixel_wv_err, C_mds[nr].pixel_wv_err, nr_byte );
	  pixel_wv_err += C_mds[nr].num_pixels;

	  nr_byte = C_mds[nr].num_obs * C_mds[nr].num_pixels * ENVI_FLOAT;
	  (void) memcpy( pixel_val, C_mds[nr].pixel_val, nr_byte );
	  pixel_val += (C_mds[nr].num_obs * C_mds[nr].num_pixels);
	  (void) memcpy( pixel_err, C_mds[nr].pixel_err, nr_byte );
	  pixel_err += (C_mds[nr].num_obs * C_mds[nr].num_pixels);
	  switch ( (int) state.type_mds ) {
	  case SCIA_NADIR:
	       nr_byte = C_mds[nr].num_obs * sizeof( struct geoN_scia );
	       (void) memcpy( geoN, C_mds[nr].geoN, nr_byte );
	       geoN += C_mds[nr].num_obs;
	       break;
	  case SCIA_LIMB:
	  case SCIA_OCCULT:
	       nr_byte = C_mds[nr].num_obs * sizeof( struct geoL_scia );
	       (void) memcpy( geoL, C_mds[nr].geoL, nr_byte );
	       geoL += C_mds[nr].num_obs;
	       break;
	  case SCIA_MONITOR:
	       nr_byte = C_mds[nr].num_obs * sizeof( struct geoC_scia );
	       (void) memcpy( geoC, C_mds[nr].geoC, nr_byte );
	       geoC += C_mds[nr].num_obs;
	       break;
	  }
     }
/*
 * release the level 1c C-structures
 */
     SCIA_LV1C_FREE_MDS( (int) state.type_mds, nr_mds, C_mds );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return nr_mds;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1_RD_MDS_PMD ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_MDS_PMD";

     int    nr_mds = 0;
     int    nr_mds1b;
     size_t nr_byte;

     char *env_str = getenv( "SCIA_CORR_LOS" );

     struct mds1_scia   *C_mds1b;

     float              *int_pmd;
     struct state1_scia state;
     struct geoL_scia   *geoL;
     struct geoN_scia   *geoN;
     struct mds1c_pmd   *C_mds_pmd;

     struct IDL_mds1c_pmd
     {
	  struct mjd_envi  mjd;
	  signed char      quality_flag;
	  unsigned char    type_mds;
	  unsigned char    category;
	  unsigned char    state_id;
	  unsigned char    state_index;
	  unsigned short   dur_scan;
	  unsigned short   num_pmd;
	  unsigned short   num_geo;
	  unsigned int     dsr_length;
	  float            orbit_phase;
	  IDL_ULONG        pntr_int_pmd;       /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_geoN;          /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_geoL;          /* IDL uses 32-bit addresses */
     } *mds_pmd;

     const unsigned long long clus_mask  = ~0ULL;

     if ( argc != 5 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     state = *(struct state1_scia *) argv[0];
     mds_pmd = (struct IDL_mds1c_pmd *) argv[1];
     int_pmd = (float *) argv[2];
     geoL = (struct geoL_scia *) argv[3];
     geoN = (struct geoN_scia *) argv[4];
/*
 * read de Measurement Data Sets of one state
 */
     nadc_stat = NADC_STAT_SUCCESS;
     nr_mds1b = (int) SCIA_LV1_RD_MDS( fd_nadc, clus_mask, &state, &C_mds1b );
     if ( IS_ERR_STAT_FATAL ) return -1;
/*
 * correct the line-of-sight azimuth and zenith angles (geoN)
 */
     if ( env_str != NULL && strcmp( env_str, "1" ) == 0 )
	  SCIA_LV1_CORR_LOS( &state, C_mds1b );
/*
 * copy the data in the level 1b PMD records to level 1c records
 */
     C_mds_pmd = (struct mds1c_pmd *) malloc( sizeof( struct mds1c_pmd ));
     nr_mds = (int) GET_SCIA_LV1C_PMD( &state, C_mds1b, C_mds_pmd );
/*
 * release the level 1b C-structures
 */
     SCIA_LV1_FREE_MDS( (int) state.type_mds, nr_mds1b, C_mds1b );
     if ( IS_ERR_STAT_FATAL ) {
	  SCIA_LV1C_FREE_MDS_PMD( (int) state.type_mds, C_mds_pmd );
	  return -1;
     }
/*
 * copy C-struct to IDL-struct
 */
     (void) memcpy( &mds_pmd->mjd, &C_mds_pmd->mjd, 
		    sizeof( struct mjd_envi ) );
     mds_pmd->quality_flag = C_mds_pmd->quality_flag;
     mds_pmd->category = C_mds_pmd->category;
     mds_pmd->state_id = C_mds_pmd->state_id;
     mds_pmd->dur_scan = C_mds_pmd->dur_scan;
     mds_pmd->num_pmd = C_mds_pmd->num_pmd;
     mds_pmd->num_geo = C_mds_pmd->num_geo;
     mds_pmd->dsr_length = C_mds_pmd->dsr_length;
     mds_pmd->orbit_phase = C_mds_pmd->orbit_phase;
     nr_byte = mds_pmd->num_pmd * ENVI_FLOAT;
     (void) memcpy( int_pmd, C_mds_pmd->int_pmd, nr_byte );
     switch ( (int) state.type_mds ) {
     case SCIA_NADIR:
	  nr_byte = mds_pmd->num_geo * sizeof( struct geoN_scia );
	  (void) memcpy( geoN, C_mds_pmd->geoN, nr_byte );
	  break;
     case SCIA_LIMB:
     case SCIA_OCCULT:
	  nr_byte = mds_pmd->num_geo * sizeof( struct geoL_scia );
	  (void) memcpy( geoL, C_mds_pmd->geoL, nr_byte );
	  break;
     }
/*
 * release memory
 */
     SCIA_LV1C_FREE_MDS_PMD( (int) state.type_mds, C_mds_pmd );
 done:
     return nr_mds;
}

int IDL_STDCALL _SCIA_LV1_RD_MDS_POLV ( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_LV1_RD_MDS_POLV";

     int    nr_mds = 0;
     int    nr_mds1b;
     size_t nr_byte;

     char *env_str = getenv( "SCIA_CORR_LOS" );

     struct mds1_scia   *C_mds1b;

     struct state1_scia state;
     struct geoL_scia   *geoL;
     struct geoN_scia   *geoN;
     struct polV_scia   *polV;
     struct mds1c_polV  *C_mds_polV;

     struct IDL_mds1c_polV
     {
	  struct mjd_envi  mjd;
	  signed char      quality_flag;
	  unsigned char    type_mds;
	  unsigned char    category;
	  unsigned char    state_id;
	  unsigned char    state_index;
	  unsigned short   dur_scan;
	  unsigned short   total_polV;
	  unsigned short   num_diff_intg;
	  unsigned short   num_geo;
	  unsigned int     dsr_length;
	  float            orbit_phase;
	  unsigned short   intg_times[MAX_CLUSTER];
	  unsigned short   num_polar[MAX_CLUSTER];
	  IDL_ULONG        pntr_polV;          /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_geoN;          /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_geoL;          /* IDL uses 32-bit addresses */
     } *mds_polV;

     const unsigned long long clus_mask  = ~0ULL;

     if ( argc != 5 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     state = *(struct state1_scia *) argv[0];
     mds_polV = (struct IDL_mds1c_polV *) argv[1];
     polV = (struct polV_scia *) argv[2];
     geoL = (struct geoL_scia *) argv[3];
     geoN = (struct geoN_scia *) argv[4];
/*
 * read de Measurement Data Sets of one state
 */
     nadc_stat = NADC_STAT_SUCCESS;
     nr_mds1b = (int) SCIA_LV1_RD_MDS( fd_nadc, clus_mask, &state, &C_mds1b );
     if ( IS_ERR_STAT_FATAL ) return -1;
/*
 * correct the line-of-sight azimuth and zenith angles (geoN)
 */
     if ( env_str != NULL && strcmp( env_str, "1" ) == 0 )
	  SCIA_LV1_CORR_LOS( &state, C_mds1b );
/*
 * copy the data in the level 1b polV records to level 1c records
 */
     C_mds_polV = (struct mds1c_polV *) malloc( sizeof( struct mds1c_polV ));
     nr_mds = (int) GET_SCIA_LV1C_POLV( &state, C_mds1b, C_mds_polV );
/*
 * release the level 1b C-structures
 */
     SCIA_LV1_FREE_MDS( (int) state.type_mds, nr_mds1b, C_mds1b );
     if ( IS_ERR_STAT_FATAL ) {
	  SCIA_LV1C_FREE_MDS_POLV( (int) state.type_mds, C_mds_polV );
	  return -1;
     }
/*
 * copy C-struct to IDL-struct
 */
     (void) memcpy( &mds_polV->mjd, &C_mds_polV->mjd, 
		    sizeof( struct mjd_envi ) );
     mds_polV->quality_flag = C_mds_polV->quality_flag;
     mds_polV->category = C_mds_polV->category;
     mds_polV->state_id = C_mds_polV->state_id;
     mds_polV->dur_scan = C_mds_polV->dur_scan;
     mds_polV->num_geo = C_mds_polV->num_geo;
     mds_polV->total_polV = C_mds_polV->total_polV;
     mds_polV->num_diff_intg = C_mds_polV->num_diff_intg;
     mds_polV->dsr_length = C_mds_polV->dsr_length;
     mds_polV->orbit_phase = C_mds_polV->orbit_phase;
     nr_byte = state.num_intg * ENVI_USHRT;
     (void) memcpy( mds_polV->intg_times, 
		    C_mds_polV->intg_times, nr_byte );
     (void) memcpy( mds_polV->num_polar, 
		    C_mds_polV->num_polar, nr_byte );
     nr_byte = mds_polV->total_polV * sizeof( struct polV_scia );
     (void) memcpy( polV, C_mds_polV->polV, nr_byte );
     switch ( (int) state.type_mds ) {
     case SCIA_NADIR:
	  nr_byte = mds_polV->num_geo * sizeof( struct geoN_scia );
	  (void) memcpy( geoN, C_mds_polV->geoN, nr_byte );
	  break;
     case SCIA_LIMB:
     case SCIA_OCCULT:
	  nr_byte = mds_polV->num_geo * sizeof( struct geoL_scia );
	  (void) memcpy( geoL, C_mds_polV->geoL, nr_byte );
	  break;
     }
/*
 * release memory
 */
     SCIA_LV1C_FREE_MDS_POLV( (int) state.type_mds, C_mds_polV );
 done:
     return nr_mds;
}

int IDL_STDCALL _SCIA_LV1C_RD_CALOPT ( int argc, void *argv[] )
{
     const char prognm[] = "LV1C_RD_CALOPT";

     unsigned int num_dsd;

     struct dsd_envi    *dsd;
     struct cal_options *calopt;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     num_dsd = *(unsigned int *) argv[0];
     dsd  = (struct dsd_envi *) argv[1];
     calopt = (struct cal_options *) argv[2];
     SCIA_LV1C_RD_CALOPT( fd_nadc, num_dsd, dsd, calopt );
     if ( IS_ERR_STAT_FATAL ) return -1;

     return 1;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1C_RD_MDS ( int argc, void *argv[] )
{
     const char prognm[] = "LV1C_RD_MDS";

     register int nr;

     int    nr_mds;
     size_t nr_byte;

     unsigned short     *pixel_ids;
     unsigned long long clus_mask;
     float              *pixel_wv, *pixel_wv_err, *pixel_val, *pixel_err;
     struct state1_scia state;
     struct geoC_scia   *geoC;
     struct geoL_scia   *geoL;
     struct geoN_scia   *geoN;
     struct mds1c_scia  *C_mds;

     struct IDL_mds1c_scia
     {
	  struct mjd_envi  mjd;
	  signed char      rad_units_flag;
	  signed char      quality_flag;
	  unsigned char    type_mds;
	  unsigned char    coaddf;
	  unsigned char    category;
	  unsigned char    state_id;
	  unsigned char    state_index;
	  unsigned char    chan_id;
	  unsigned char    clus_id;
	  unsigned short   dur_scan;
	  unsigned short   num_obs;
	  unsigned short   num_pixels;
	  unsigned int     dsr_length;
	  float            orbit_phase;
	  float            pet;
	  IDL_ULONG        pntr_pixel_ids;     /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_pixel_wv;      /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_pixel_wv_err;  /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_pixel_val;     /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_pixel_err;     /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_geoN;          /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_geoL;          /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_geoC;          /* IDL uses 32-bit addresses */
     } *mds;

     if ( argc != 11 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     clus_mask = *(unsigned long long *) argv[0];
     state = *(struct state1_scia *) argv[1];
     mds = (struct IDL_mds1c_scia *) argv[2];
     pixel_ids = (unsigned short *) argv[3];
     pixel_wv = (float *) argv[4];
     pixel_wv_err = (float *) argv[5];
     pixel_val = (float *) argv[6];
     pixel_err = (float *) argv[7];
     geoC = (struct geoC_scia *) argv[8];
     geoL = (struct geoL_scia *) argv[9];
     geoN = (struct geoN_scia *) argv[10];
/*
 * read de Measurement Data Sets of one state
 */
     nr_mds = (int) SCIA_LV1C_RD_MDS( fd_nadc, clus_mask, &state, &C_mds );
     if ( IS_ERR_STAT_FATAL ) {
	  SCIA_LV1C_FREE_MDS( (int) state.type_mds, nr_mds, C_mds );
	  return -1;
     }
/*
 * copy C-struct to IDL-struct
 */
     for ( nr = 0; nr < nr_mds; nr++ ) {
	  (void) memcpy( &mds[nr].mjd, &C_mds[nr].mjd, 
			 sizeof( struct mjd_envi ) );
	  mds[nr].rad_units_flag = C_mds[nr].rad_units_flag;
	  mds[nr].quality_flag = C_mds[nr].quality_flag;
	  mds[nr].type_mds = C_mds[nr].type_mds;
	  mds[nr].coaddf   = C_mds[nr].coaddf;
	  mds[nr].category = C_mds[nr].category;
	  mds[nr].state_id = C_mds[nr].state_id;
	  mds[nr].chan_id  = C_mds[nr].chan_id;
	  mds[nr].clus_id  = C_mds[nr].clus_id;
	  mds[nr].dur_scan = C_mds[nr].dur_scan;
	  mds[nr].num_obs  = C_mds[nr].num_obs;
	  mds[nr].num_pixels = C_mds[nr].num_pixels;
	  mds[nr].dsr_length = C_mds[nr].dsr_length;
	  mds[nr].orbit_phase = C_mds[nr].orbit_phase;
	  mds[nr].pet      = C_mds[nr].pet;

	  nr_byte = mds[nr].num_pixels * ENVI_USHRT;
	  (void) memcpy( pixel_ids, C_mds[nr].pixel_ids, nr_byte );
	  pixel_ids += mds[nr].num_pixels;

	  nr_byte = mds[nr].num_pixels * ENVI_FLOAT;
	  (void) memcpy( pixel_wv, C_mds[nr].pixel_wv, nr_byte );
	  pixel_wv += mds[nr].num_pixels;
	  (void) memcpy( pixel_wv_err, C_mds[nr].pixel_wv_err, nr_byte );
	  pixel_wv_err += mds[nr].num_pixels;

	  nr_byte = mds[nr].num_obs * mds[nr].num_pixels * ENVI_FLOAT;
	  (void) memcpy( pixel_val, C_mds[nr].pixel_val, nr_byte );
	  pixel_val += (mds[nr].num_obs * mds[nr].num_pixels);
	  (void) memcpy( pixel_err, C_mds[nr].pixel_err, nr_byte );
	  pixel_err += (mds[nr].num_obs * mds[nr].num_pixels);

	  switch ( (int) state.type_mds ) {
	  case SCIA_NADIR:
	       nr_byte = mds[nr].num_obs * sizeof( struct geoN_scia );
	       (void) memcpy( geoN, C_mds[nr].geoN, nr_byte );
	       geoN += mds[nr].num_obs;
	       break;
	  case SCIA_LIMB:
	  case SCIA_OCCULT:
	       nr_byte = mds[nr].num_obs * sizeof( struct geoL_scia );
	       (void) memcpy( geoL, C_mds[nr].geoL, nr_byte );
	       geoL += mds[nr].num_obs;
	       break;
	  case SCIA_MONITOR:
	       nr_byte = mds[nr].num_obs * sizeof( struct geoC_scia );
	       (void) memcpy( geoC, C_mds[nr].geoC, nr_byte );
	       geoC += mds[nr].num_obs;
	       break;
	  }
     }
/*
 * release memory
 */
     SCIA_LV1C_FREE_MDS( (int) state.type_mds, nr_mds, C_mds );

     return nr_mds;
 done:
     return -1;
}

int IDL_STDCALL _SCIA_LV1C_RD_MDS_PMD ( int argc, void *argv[] )
{
     const char prognm[] = "LV1C_RD_MDS_PMD";

     int    nr_mds = 0;
     size_t nr_byte;

     float              *int_pmd;
     struct state1_scia state;
     struct geoL_scia   *geoL;
     struct geoN_scia   *geoN;
     struct mds1c_pmd   *C_mds_pmd;

     struct IDL_mds1c_pmd
     {
	  struct mjd_envi  mjd;
	  signed char      quality_flag;
	  unsigned char    type_mds;
	  unsigned char    category;
	  unsigned char    state_id;
	  unsigned char    state_index;
	  unsigned short   dur_scan;
	  unsigned short   num_pmd;
	  unsigned short   num_geo;
	  unsigned int     dsr_length;
	  float            orbit_phase;
	  IDL_ULONG        pntr_int_pmd;       /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_geoN;          /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_geoL;          /* IDL uses 32-bit addresses */
     } *mds_pmd;

     if ( argc != 5 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     state = *(struct state1_scia *) argv[0];
     mds_pmd = (struct IDL_mds1c_pmd *) argv[1];
     int_pmd = (float *) argv[2];
     geoL = (struct geoL_scia *) argv[3];
     geoN = (struct geoN_scia *) argv[4];
/*
 * read de Measurement Data Sets of one state
 */
     C_mds_pmd = (struct mds1c_pmd *) malloc( sizeof( struct mds1c_pmd ));
     if ( C_mds_pmd == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "C_mds_pmd" );
     nr_mds = (int) SCIA_LV1C_RD_MDS_PMD( fd_nadc, &state, &C_mds_pmd );
     if ( IS_ERR_STAT_FATAL ) {
	  SCIA_LV1C_FREE_MDS_PMD( (int) state.type_mds, C_mds_pmd );
	  return -1;
     }
     if ( nr_mds == 0 ) return 0;
/*
 * copy C-struct to IDL-struct
 */
     (void) memcpy( &mds_pmd->mjd, &C_mds_pmd->mjd, 
		    sizeof( struct mjd_envi ) );
     mds_pmd->quality_flag = C_mds_pmd->quality_flag;
     mds_pmd->type_mds = C_mds_pmd->type_mds;
     mds_pmd->category = C_mds_pmd->category;
     mds_pmd->state_id = C_mds_pmd->state_id;
     mds_pmd->dur_scan = C_mds_pmd->dur_scan;
     mds_pmd->num_pmd = C_mds_pmd->num_pmd;
     mds_pmd->num_geo = C_mds_pmd->num_geo;
     mds_pmd->dsr_length = C_mds_pmd->dsr_length;
     mds_pmd->orbit_phase = C_mds_pmd->orbit_phase;
     nr_byte = mds_pmd->num_pmd * ENVI_FLOAT;
     (void) memcpy( int_pmd, C_mds_pmd->int_pmd, nr_byte );
     switch ( (int) state.type_mds ) {
     case SCIA_NADIR:
	  nr_byte = mds_pmd->num_geo * sizeof( struct geoN_scia );
	  (void) memcpy( geoN, C_mds_pmd->geoN, nr_byte );
	  break;
     case SCIA_LIMB:
     case SCIA_OCCULT:
	  nr_byte = mds_pmd->num_geo * sizeof( struct geoL_scia );
	  (void) memcpy( geoL, C_mds_pmd->geoL, nr_byte );
	  break;
     }
/*
 * release memory
 */
     SCIA_LV1C_FREE_MDS_PMD( (int) state.type_mds, C_mds_pmd );
 done:
     return nr_mds;
}

int IDL_STDCALL _SCIA_LV1C_RD_MDS_POLV ( int argc, void *argv[] )
{
     const char prognm[] = "LV1C_RD_MDS_POLV";

     int    nr_mds = 0;
     size_t nr_byte;

     struct state1_scia state;
     struct geoL_scia   *geoL;
     struct geoN_scia   *geoN;
     struct polV_scia   *polV;
     struct mds1c_polV  *C_mds_polV;

     struct IDL_mds1c_polV
     {
	  struct mjd_envi  mjd;
	  signed char      quality_flag;
	  unsigned char    type_mds;
	  unsigned char    category;
	  unsigned char    state_id;
	  unsigned char    state_index;
	  unsigned short   dur_scan;
	  unsigned short   total_polV;
	  unsigned short   num_diff_intg;
	  unsigned short   num_geo;
	  unsigned int     dsr_length;
	  float            orbit_phase;
	  unsigned short   intg_times[MAX_CLUSTER];
	  unsigned short   num_polar[MAX_CLUSTER];
	  IDL_ULONG        pntr_polV;          /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_geoN;          /* IDL uses 32-bit addresses */
	  IDL_ULONG        pntr_geoL;          /* IDL uses 32-bit addresses */
     } *mds_polV;

     if ( argc != 5 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );

     state = *(struct state1_scia *) argv[0];
     mds_polV = (struct IDL_mds1c_polV *) argv[1];
     polV = (struct polV_scia *) argv[2];
     geoL = (struct geoL_scia *) argv[3];
     geoN = (struct geoN_scia *) argv[4];
/*
 * read de Measurement Data Sets of one state
 */
     C_mds_polV = (struct mds1c_polV *) malloc( sizeof( struct mds1c_polV ));
     if ( C_mds_polV == NULL )
	  NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "C_mds_polV" );
     nr_mds = (int) SCIA_LV1C_RD_MDS_POLV( fd_nadc, &state, &C_mds_polV );
     if ( IS_ERR_STAT_FATAL ) {
	  SCIA_LV1C_FREE_MDS_POLV( (int) state.type_mds, C_mds_polV );
	  return -1;
     }
     if ( nr_mds == 0 ) return 0;
/*
 * copy C-struct to IDL-struct
 */
     (void) memcpy( &mds_polV->mjd, &C_mds_polV->mjd, 
		    sizeof( struct mjd_envi ) );
     mds_polV->quality_flag = C_mds_polV->quality_flag;
     mds_polV->type_mds = C_mds_polV->type_mds;
     mds_polV->category = C_mds_polV->category;
     mds_polV->state_id = C_mds_polV->state_id;
     mds_polV->dur_scan = C_mds_polV->dur_scan;
     mds_polV->total_polV = C_mds_polV->total_polV;
     mds_polV->num_diff_intg = C_mds_polV->num_diff_intg;
     mds_polV->num_geo = C_mds_polV->num_geo;
     mds_polV->dsr_length = C_mds_polV->dsr_length;
     mds_polV->orbit_phase = C_mds_polV->orbit_phase;
     nr_byte = state.num_intg * ENVI_USHRT;
     (void) memcpy( mds_polV->intg_times, 
		    C_mds_polV->intg_times, nr_byte );
     (void) memcpy( mds_polV->num_polar, 
		    C_mds_polV->num_polar, nr_byte );
     nr_byte = mds_polV->total_polV * sizeof( struct polV_scia );
     (void) memcpy( polV, C_mds_polV->polV, nr_byte );
     switch ( (int) state.type_mds ) {
     case SCIA_NADIR:
	  nr_byte = mds_polV->num_geo * sizeof( struct geoN_scia );
	  (void) memcpy( geoN, C_mds_polV->geoN, nr_byte );
	  break;
     case SCIA_LIMB:
     case SCIA_OCCULT:
	  nr_byte = mds_polV->num_geo * sizeof( struct geoL_scia );
	  (void) memcpy( geoL, C_mds_polV->geoL, nr_byte );
	  break;
     }
/*
 * release memory
 */
     SCIA_LV1C_FREE_MDS_POLV( (int) state.type_mds, C_mds_polV );
 done:
     return nr_mds;
}

#define TO_JDAY(mjd) (mjd.days + \
		      (mjd.secnd + mjd.musec / 1e6) / (60. * 60 * 24))

int IDL_STDCALL _GET_SCIA_MDS1_DATA ( int argc, void *argv[] )
{
     const char prognm[] = "_GET_SCIA_MDS1_DATA";

     register int ni, nr;

     int                dim_Y    = 0;
     int                nr_mds1b = 0;
     int                nr_mds1c = 0;

     bool               is_level_1c, pmdScaling;
     unsigned int       calib_mask;
     unsigned long long clus_mask;
     float              *sign;

     struct scia_geoC_rec {
	  double jday;
	  struct geoC_scia geoC;
     } *sign_geoC;

     struct scia_geoL_rec {
	  double jday;
	  struct geoL_scia geoL;
     } *sign_geoL;

     struct scia_geoN_rec {
	  double jday;
	  struct geoN_scia geoN;
     } *sign_geoN;

     struct state1_scia state;
     struct mds1_scia   *C_mds1b = NULL;
     struct mds1c_pmd   *C_pmd1c = NULL;
     struct mds1c_scia  *C_mds1c = NULL;

     const double SecPerDay = 60 * 60 * 24;
/*
 * check input parameters
 */
     if ( argc != 7 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     if ( fileno( fd_nadc ) == -1 ) 
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FILE, "No open stream" );
/*
 * set pointers
 */
     is_level_1c = *(bool *) argv[0];
     pmdScaling = *(bool *) argv[1];
     state = *(struct state1_scia *) argv[2];
     clus_mask = *(unsigned long long *) argv[3];
     calib_mask = *(unsigned int *) argv[4];
     sign = (float *) argv[5];
     switch ( (int) (state.type_mds) ) {
     case SCIA_NADIR:
	  sign_geoC = NULL;
	  sign_geoL = NULL;
	  sign_geoN = (struct scia_geoN_rec *) argv[6];
	  break;
     case SCIA_LIMB:
     case SCIA_OCCULT:
	  sign_geoC = NULL;
	  sign_geoL = (struct scia_geoL_rec *) argv[6];
	  sign_geoN = NULL;
	  break;
     case SCIA_MONITOR:
	  sign_geoC = (struct scia_geoC_rec *) argv[6];
	  sign_geoL = NULL;
	  sign_geoN = NULL;
	  break;
     default:
	  sign_geoC = NULL;
	  sign_geoL = NULL;
	  sign_geoN = NULL;
	  break;
     }			 
/*
 * read de Measurement Data Sets of one state, 
 * in case of a Lv1c product we also need to read the PMD data
 */
     if ( is_level_1c ) {
	  C_pmd1c = (struct mds1c_pmd *) malloc( sizeof( struct mds1c_pmd ));
	  if ( C_pmd1c == NULL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "C_pmd1c" );
	  if ( SCIA_LV1C_RD_MDS_PMD( fd_nadc, &state, &C_pmd1c ) == 0 ) { 
	       free( C_pmd1c ); 
	       C_pmd1c = NULL; 
	  }
	  if ( IS_ERR_STAT_FATAL ) goto done;

	  C_mds1c = (struct mds1c_scia *)
	       malloc( state.num_clus * sizeof( struct mds1c_scia ));
	  if ( C_mds1c == NULL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "C_mds1c" );
	  nr_mds1c = SCIA_LV1C_RD_MDS( fd_nadc, clus_mask, &state, &C_mds1c );
	  if ( IS_ERR_STAT_FATAL ) goto done;
/*
 * extract science data from MDS 1c records
 */
	  dim_Y = (int) GET_SCIA_MDS1_DATA( pmdScaling, C_mds1c->chan_id, 
					    &state, NULL, C_pmd1c, C_mds1c, 
					    &sign );
	  if ( IS_ERR_STAT_FATAL ) goto done;
     } else {
	  nr_mds1b = (int) SCIA_LV1_RD_MDS( fd_nadc, ~0ULL, &state, &C_mds1b );
	  if ( IS_ERR_STAT_FATAL ) goto done;
/*
 * calibrate and copy the data in the level 1b records to level 1c records
 */
	  C_mds1c = (struct mds1c_scia *)
	       malloc( state.num_clus * sizeof( struct mds1c_scia ));
	  if ( C_mds1c == NULL ) 
	       NADC_GOTO_ERROR( prognm, NADC_ERR_ALLOC, "C_mds1c" );
	  nr_mds1c = (int) 
	       GET_SCIA_LV1C_MDS( clus_mask, &state, C_mds1b, C_mds1c );
	  if ( IS_ERR_STAT_FATAL ) goto done;
	  SCIA_LV1_CAL( fd_nadc, calib_mask, &state, C_mds1b, C_mds1c );
	  if ( IS_ERR_STAT_FATAL ) goto done;
/*
 * extract science data from MDS 1c records
 */
	  dim_Y = (int) GET_SCIA_MDS1_DATA( pmdScaling, C_mds1c->chan_id, 
					    &state, C_mds1b, NULL, C_mds1c, 
					    &sign );
	  if ( IS_ERR_STAT_FATAL ) goto done;
     }
/*
 * collect geolocation data
 */
     for ( nr = 0; nr < nr_mds1c; nr++ ) {
	  if ( C_mds1c[nr].num_obs == dim_Y ) {
	       double jday = TO_JDAY( C_mds1c[nr].mjd );
	       double intg = C_mds1c[nr].coaddf * C_mds1c[nr].pet / SecPerDay;

	       switch ( (int) (state.type_mds) ) {
	       case SCIA_NADIR:
		    for ( ni = 0; ni < dim_Y; ni++ ) {
			 sign_geoN[ni].jday = jday + ni * intg;
			 (void) memcpy( &sign_geoN[ni].geoN, 
					&C_mds1c[nr].geoN[ni],
					sizeof( struct geoN_scia ) );
		    }
		    break;
	       case SCIA_LIMB:
	       case SCIA_OCCULT:
		    for ( ni = 0; ni < dim_Y; ni++ ) {
			 sign_geoL[ni].jday = jday + ni * intg;
			 (void) memcpy( &sign_geoL[ni].geoL, 
					&C_mds1c[nr].geoL[ni],
					sizeof( struct geoL_scia ) );
		    }
		    break;
	       case SCIA_MONITOR:
		    for ( ni = 0; ni < dim_Y; ni++ ) {
			 sign_geoC[ni].jday = jday + ni * intg;
			 (void) memcpy( &sign_geoC[ni].geoC, 
					&C_mds1c[nr].geoC[ni],
					sizeof( struct geoC_scia ) );
		    }
		    break;
	       }
	       break;
	  }
     }
/*
 * release the level 1b and 1c C-structures
 */
 done:
     if ( C_mds1b != NULL )
	  SCIA_LV1_FREE_MDS( (int) state.type_mds, nr_mds1b, C_mds1b );
     if ( C_pmd1c != NULL )
	  SCIA_LV1C_FREE_MDS_PMD( (int) state.type_mds, C_pmd1c );
     if ( C_mds1c != NULL )
	  SCIA_LV1C_FREE_MDS( (int) state.type_mds, nr_mds1c, C_mds1c );

     if ( IS_ERR_STAT_FATAL ) 
	  return -1;
     else
	  return dim_Y;
}

int IDL_STDCALL _SCIA_RD_MFACTOR( int argc, void *argv[] )
{
     const char prognm[] = "_SCIA_RD_MFACTOR";

     const unsigned int calibFlag = 0U;

     enum mf_type mftype;

     IDL_STRING   *mf_str;
     IDL_STRING   *sensing_start;
     float        *mfactor;

     if ( argc != 3 ) NADC_GOTO_ERROR( prognm, NADC_ERR_PARAM, err_msg );
     mf_str        = (IDL_STRING *) argv[0];
     sensing_start = (IDL_STRING *) argv[1];
     mfactor       = (float *) argv[2];

     if ( strcmp( mf_str[0].s, "M_CAL" ) == 0 )
	  mftype = M_CAL;
     else if ( strcmp( mf_str[0].s, "M_DL" ) == 0 )
	  mftype = M_DL;
     else if ( strcmp( mf_str[0].s, "M_DN" ) == 0 )
	  mftype = M_DN;
     else
	  goto done;

     SCIA_RD_MFACTOR( mftype, sensing_start[0].s, calibFlag, mfactor );
     if ( IS_ERR_STAT_FATAL ) goto done;

     return 0;
done:
     return -1;
     
}
