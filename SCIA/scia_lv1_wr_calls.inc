/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2005 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SCIA_LV1_WR_CALLS.INC
.AUTHOR      R.M. van Hees
.PURPOSE     provides one master write-routine for all SCIA L1 datasets
.DEFINES     33 static functions:
              - SCIA_WRITE_MPH, SCIA_WRITE_SPH, SCIA_WRITE_DSD,
	      SCIA_WRITE_SQADS, SCIA_WRITE_LADS, SCIA_WRITE_SIP,
	      SCIA_WRITE_CLCP, SCIA_WRITE_VLCP, SCIA_WRITE_PPG,
	      SCIA_WRITE_BASE, SCIA_WRITE_SCP, SCIA_WRITE_SRS,
	      SCIA_WRITE_PSPN, SCIA_WRITE_PSPL, SCIA_WRITE_PSPO,
	      SCIA_WRITE_RSPN, SCIA_WRITE_RSPL, SCIA_WRITE_RSPO,
	      SCIA_WRITE_EKD, SCIA_WRITE_SFP, SCIA_WRITE_ASFP,
	      SCIA_WRITE_STATE, SCIA_WRITE_PMD, SCIA_WRITE_AUX,
	      SCIA_WRITE_LCPN, SCIA_WRITE_DARK, SCIA_WRITE_PPGN,
	      SCIA_WRITE_SCPN, SCIA_WRITE_SRSN, SCIA_WRITE_MDS_1B,
	      SCIA_WRITE_MDS_1C, SCIA_WRITE_MDS_PMD, SCIA_WRITE_MDS_POLV

.COMMENTS    ANSI-C inline function, declare in your code as:
             #include <scia_lv1_wr_calls.inc>
.VERSION     1.0     11-Oct-2005   initial release by R. M. van Hees
------------------------------------------------------------*/
/*+++++ System headers +++++*/
	/* NONE */

/*+++++ Local Headers +++++*/
	/* NONE */

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
	/* NONE */

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
				/* NONE */
/*+++++++++++++++++++++++++++++++++++++++++++++
 * write MPH
 */
static void SCIA_WRITE_MPH( const struct param_record param, 
			    const struct mph_envi *mph )
{
     const char prognm[] = "SCIA_WRITE_MPH";

     if ( param.write_pds == PARAM_SET ) {
	  if ( (fp_out = fopen( param.outfile, "wb+")) == NULL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_CRE, param.outfile);

	  ENVI_WR_MPH( fp_out, *mph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "MPH" );
     }
     if ( param.write_ascii == PARAM_SET ) {
	  ENVI_WR_ASCII_MPH( param, mph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "MPH" );
     }
     if ( param.write_hdf5 == PARAM_SET ) {
	  SCIA_WR_H5_MPH( param, mph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "MPH" );
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SPH
 */
static void SCIA_WRITE_SPH( const struct param_record param, 
			    const struct mph_envi mph,
			    const struct sph1_scia *sph )
{
     const char prognm[] = "SCIA_WRITE_SPH";

     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_SPH( fp_out, mph, *sph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "SPH" );
     }
     if ( param.write_ascii == PARAM_SET ) {
	  SCIA_LV1_WR_ASCII_SPH( param, sph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "SPH" );
     }
     if ( param.write_hdf5 == PARAM_SET ) {
	  SCIA_LV1_WR_H5_SPH( param, sph );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "SPH" );
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write DSD
 */
static void SCIA_WRITE_DSD( const struct param_record param, 
			    unsigned int num_dsd,
			    const struct dsd_envi *dsd )
{
     const char prognm[] = "SCIA_WRITE_DSD";

     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_DSD_INIT( param, fp_out, num_dsd, dsd );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_PDS_WR, "DSD" );
     }
     if ( param.write_ascii == PARAM_SET ) {
	  ENVI_WR_ASCII_DSD( param, num_dsd, dsd );
	  if ( IS_ERR_STAT_FATAL )
	       NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "DSD" );
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SQADS
 */
static void SCIA_WRITE_SQADS( const struct param_record param, 
			      unsigned int num_sqads,
			      const struct sqads1_scia *sqads )
{
     const char prognm[] = "SCIA_WRITE_SQADS";

     if ( num_sqads == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_SQADS( fp_out, num_sqads, sqads );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_SQADS( param, num_sqads, sqads );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "SQADS" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_SQADS( param, num_sqads, sqads );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "SQADS" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write LADS
 */
static void SCIA_WRITE_LADS( const struct param_record param, 
			      unsigned int num_lads,
			      const struct lads_scia *lads )
{
     const char prognm[] = "SCIA_WRITE_LADS";

     if ( num_lads == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_LADS( fp_out, num_lads, lads );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_WR_ASCII_LADS( param, num_lads, lads );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "LADS" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_WR_H5_LADS( param, num_lads, lads );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "LADS" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SIP
 */
static void SCIA_WRITE_SIP( const struct param_record param, 
			    unsigned int num_sip,
			    const struct sip_scia sip )
{
     const char prognm[] = "SCIA_WRITE_SIP";

     if ( num_sip == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_SIP( fp_out, num_sip, sip );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_SIP( param, &sip );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "SIP" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_SIP( param, &sip );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "SIP" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write CLCP
 */
static void SCIA_WRITE_CLCP( const struct param_record param, 
			     unsigned int num_clcp,
			     const struct clcp_scia clcp )
{
     const char prognm[] = "SCIA_WRITE_CLCP";

     if ( num_clcp == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_CLCP( fp_out, num_clcp, clcp );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_CLCP( param, &clcp );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "CLCP" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_CLCP( param, &clcp );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "CLCP" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write VLCP
 */
static void SCIA_WRITE_VLCP( const struct param_record param, 
			     unsigned int num_vlcp,
			     const struct vlcp_scia *vlcp )
{
     const char prognm[] = "SCIA_WRITE_VLCP";

     if ( num_vlcp == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_VLCP( fp_out, num_vlcp, vlcp );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_VLCP( param, num_vlcp, vlcp );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "VLCP" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_VLCP( param, num_vlcp, vlcp );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "VLCP" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write PPG
 */
static void SCIA_WRITE_PPG( const struct param_record param, 
			    unsigned int num_ppg,
			    const struct ppg_scia ppg )
{
     const char prognm[] = "SCIA_WRITE_PPG";

     if ( num_ppg == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_PPG( fp_out, num_ppg, ppg );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_PPG( param, &ppg );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "PPG" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_PPG( param, &ppg );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "PPG" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write BASE
 */
static void SCIA_WRITE_BASE( const struct param_record param, 
			     const struct base_scia *base )
{
     const char prognm[] = "SCIA_WRITE_BASE";

     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_BASE( fp_out, base );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_BASE( param, base );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "BASE" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_BASE( param, base );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "BASE" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SCP
 */
static void SCIA_WRITE_SCP( const struct param_record param, 
			    unsigned int num_scp,
			    const struct scp_scia *scp )
{
     const char prognm[] = "SCIA_WRITE_SCP";

     if ( num_scp == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_SCP( fp_out, num_scp, scp );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_SCP( param, num_scp, scp );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "SCP" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_SCP( param, num_scp, scp );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "SCP" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SRS
 */
static void SCIA_WRITE_SRS( const struct param_record param, 
			    unsigned int num_srs,
			    const struct srs_scia *srs )
{
     const char prognm[] = "SCIA_WRITE_SRS";

     if ( num_srs == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_SRS( fp_out, num_srs, srs );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_SRS( param, num_srs, srs );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "SRS" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_SRS( param, num_srs, srs );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "SRS" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write PSPN
 */
static void SCIA_WRITE_PSPN( const struct param_record param, 
			     unsigned int num_pspn,
			     const struct pspn_scia *pspn )
{
     const char prognm[] = "SCIA_WRITE_PSPN";

     if ( num_pspn == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_PSPN( fp_out, num_pspn, pspn );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_PSPN( param, num_pspn, pspn );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "PSPN" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_PSPN( param, num_pspn, pspn );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "PSPN" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write PSPL
 */
static void SCIA_WRITE_PSPL( const struct param_record param, 
			     unsigned int num_pspl,
			     const struct psplo_scia *pspl )
{
     const char prognm[] = "SCIA_WRITE_PSPL";

     if ( num_pspl == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_PSPL( fp_out, num_pspl, pspl );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_PSPL( param, num_pspl, pspl );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "PSPL" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_PSPL( param, num_pspl, pspl );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "PSPL" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write PSPO
 */
static void SCIA_WRITE_PSPO( const struct param_record param, 
			     unsigned int num_pspo,
			     const struct psplo_scia *pspo )
{
     const char prognm[] = "SCIA_WRITE_PSPO";

     if ( num_pspo == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_PSPO( fp_out, num_pspo, pspo );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_PSPO( param, num_pspo, pspo );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "PSPO" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_PSPO( param, num_pspo, pspo );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "PSPO" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write RSPN
 */
static void SCIA_WRITE_RSPN( const struct param_record param, 
			     unsigned int num_rspn,
			     const struct rspn_scia *rspn )
{
     const char prognm[] = "SCIA_WRITE_RSPN";

     if ( num_rspn == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_RSPN( fp_out, num_rspn, rspn );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_RSPN( param, num_rspn, rspn );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "RSPN" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_RSPN( param, num_rspn, rspn );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "RSPN" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write RSPL
 */
static void SCIA_WRITE_RSPL( const struct param_record param, 
			     unsigned int num_rspl,
			     const struct rsplo_scia *rspl )
{
     const char prognm[] = "SCIA_WRITE_RSPL";

     if ( num_rspl == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_RSPL( fp_out, num_rspl, rspl );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_RSPL( param, num_rspl, rspl );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "RSPL" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_RSPL( param, num_rspl, rspl );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "RSPL" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write RSPO
 */
static void SCIA_WRITE_RSPO( const struct param_record param, 
			     unsigned int num_rspo,
			     const struct rsplo_scia *rspo )
{
     const char prognm[] = "SCIA_WRITE_RSPO";

     if ( num_rspo == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_RSPO( fp_out, num_rspo, rspo );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_RSPO( param, num_rspo, rspo );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "RSPO" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_RSPO( param, num_rspo, rspo );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "RSPO" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write EKD
 */
static void SCIA_WRITE_EKD( const struct param_record param, 
			    unsigned int num_ekd,
			    const struct ekd_scia ekd )
{
     const char prognm[] = "SCIA_WRITE_EKD";

     if ( num_ekd == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_EKD( fp_out, num_ekd, ekd );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_EKD( param, &ekd );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "EKD" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_EKD( param, &ekd );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "EKD" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SFP
 */
static void SCIA_WRITE_SFP( const struct param_record param, 
			    unsigned int num_sfp,
			    const struct sfp_scia *sfp )
{
     const char prognm[] = "SCIA_WRITE_SFP";

     if ( num_sfp == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_SFP( fp_out, num_sfp, sfp );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_SFP( param, num_sfp, sfp );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "SFP" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_SFP( param, num_sfp, sfp );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "SFP" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write ASFP
 */
static void SCIA_WRITE_ASFP( const struct param_record param, 
			     unsigned int num_asfp,
			     const struct asfp_scia *asfp )
{
     const char prognm[] = "SCIA_WRITE_ASFP";

     if ( num_asfp == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_ASFP( fp_out, num_asfp, asfp );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_ASFP( param, num_asfp, asfp );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "ASFP" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_ASFP( param, num_asfp, asfp );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "ASFP" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write STATE
 */
static void SCIA_WRITE_STATE( const struct param_record param, 
			      unsigned int num_state,
			      const struct state1_scia *state )
{
     const char prognm[] = "SCIA_WRITE_STATE";

     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_STATE( fp_out, num_state, state );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_STATE( param, num_state, state );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "STATE" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_STATE( param, num_state, state );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "STATE" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write PMD
 */
static void SCIA_WRITE_PMD( const struct param_record param, 
			    unsigned int num_pmd,
			    const struct mds1_pmd *pmd )
{
     const char prognm[] = "SCIA_WRITE_PMD";

     if ( num_pmd == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_PMD( fp_out, num_pmd, pmd );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_PMD( param, num_pmd, pmd );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "PMD" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_PMD( param, num_pmd, pmd );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "PMD" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write AUX
 */
static void SCIA_WRITE_AUX( const struct param_record param, 
			    unsigned int num_aux,
			    const struct mds1_aux *aux )
{
     const char prognm[] = "SCIA_WRITE_AUX";

     if ( num_aux == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_AUX( fp_out, num_aux, aux );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_AUX( param, num_aux, aux );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "AUX" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_AUX( param, num_aux, aux );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "AUX" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write LCPN
 */
static void SCIA_WRITE_LCPN( const struct param_record param, 
			     unsigned int num_lcpn,
			     const struct lcpn_scia *lcpn )
{
     const char prognm[] = "SCIA_WRITE_LCPN";

     if ( num_lcpn == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_LCPN( fp_out, num_lcpn, lcpn );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_LCPN( param, num_lcpn, lcpn );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "LCPN" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_LCPN( param, num_lcpn, lcpn );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "LCPN" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write DARK
 */
static void SCIA_WRITE_DARK( const struct param_record param, 
			     unsigned int num_dark,
			     const struct dark_scia *dark )
{
     const char prognm[] = "SCIA_WRITE_DARK";

     if ( num_dark == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_DARK( fp_out, num_dark, dark );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_DARK( param, num_dark, dark );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "DARK" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_DARK( param, num_dark, dark );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "DARK" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write PPGN
 */
static void SCIA_WRITE_PPGN( const struct param_record param, 
			     unsigned int num_ppgn,
			     const struct ppgn_scia *ppgn )
{
     const char prognm[] = "SCIA_WRITE_PPGN";

     if ( num_ppgn == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_PPGN( fp_out, num_ppgn, ppgn );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_PPGN( param, num_ppgn, ppgn );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "PPGN" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_PPGN( param, num_ppgn, ppgn );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "PPGN" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SCPN
 */
static void SCIA_WRITE_SCPN( const struct param_record param, 
			     unsigned int num_scpn,
			     const struct scpn_scia *scpn )
{
     const char prognm[] = "SCIA_WRITE_SCPN";

     if ( num_scpn == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_SCPN( fp_out, num_scpn, scpn );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_SCPN( param, num_scpn, scpn );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "SCPN" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_SCPN( param, num_scpn, scpn );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "SCPN" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SRSN
 */
static void SCIA_WRITE_SRSN( const struct param_record param, 
			     unsigned int num_srsn,
			     const struct srsn_scia *srsn )
{
     const char prognm[] = "SCIA_WRITE_SRSN";

     if ( num_srsn == 0 ) return;
     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_SRSN( fp_out, num_srsn, srsn );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_SRSN( param, num_srsn, srsn );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, "SRSN" );
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_SRSN( param, num_srsn, srsn );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, "SRSN" );
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write MDS level 1b
 */
static void SCIA_WRITE_MDS_1B( const struct param_record param,
			       unsigned int num_mds,
			       const struct mds1_scia *mds )
{
     const char prognm[] = "SCIA_WRITE_MDS_1B";

     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1_WR_MDS( fp_out, num_mds, mds );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1_WR_ASCII_MDS( param, num_mds, mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, 
				       "SCIA_LV1_WR_ASCII_MDS");
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1_WR_H5_MDS( param, num_mds, mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, 
				       "SCIA_LV1_WR_H5_MDS");
	  }
     }
}


/*+++++++++++++++++++++++++++++++++++++++++++++
 * write MDS level 1c
 */
static void SCIA_WRITE_MDS_1C( const struct param_record param,
			       unsigned int num_mds,
			       const struct mds1c_scia *mds )
{
     const char prognm[] = "SCIA_WRITE_MDS_1C";

     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1C_WR_MDS( fp_out, num_mds, mds );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1C_WR_ASCII_MDS( param, num_mds, mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, 
				       "SCIA_LV1C_WR_ASCII_MDS");
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1C_WR_H5_MDS( param, num_mds, mds );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, 
				       "SCIA_LV1C_WR_H5_MDS");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write MDS PMD level 1c
 */
static void SCIA_WRITE_MDS_PMD( const struct param_record param,
				const struct mds1c_pmd *mds_pmd )
{
     const char prognm[] = "SCIA_WRITE_MDS_PMD";

     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1C_WR_MDS_PMD( fp_out, mds_pmd );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1C_WR_ASCII_MDS_PMD( param, mds_pmd );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, 
				       "SCIA_LV1_WR_ASCII_MDS_PMD");
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1C_WR_H5_MDS_PMD( param, mds_pmd );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, 
				       "SCIA_LV1_WR_H5_MDS_PMD");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write MDS POLV level 1c
 */
static void SCIA_WRITE_MDS_POLV( const struct param_record param,
				 const struct mds1c_polV *mds_polV )
{
     const char prognm[] = "SCIA_WRITE_MDS_POLV";

     if ( param.write_pds == PARAM_SET ) {
	  SCIA_LV1C_WR_MDS_POLV( fp_out, mds_polV );
     } else {
	  if ( param.write_ascii == PARAM_SET ) {
	       SCIA_LV1C_WR_ASCII_MDS_POLV( param, mds_polV );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_RETURN_ERROR( prognm, NADC_ERR_FILE_WR, 
				       "SCIA_LV1_WR_ASCII_MDS_POLV");
	  }
	  if ( param.write_hdf5 == PARAM_SET ) {
	       SCIA_LV1C_WR_H5_MDS_POLV( param, mds_polV );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_RETURN_ERROR( prognm, NADC_ERR_HDF_WR, 
				       "SCIA_LV1_WR_H5_MDS_POLV");
	  }
     }
}
