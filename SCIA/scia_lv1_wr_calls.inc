/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2005 - 2019 SRON (R.M.van.Hees@sron.nl)

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
static void SCIA_WRITE_MPH(const struct mph_envi *mph)
{
     char *cpntr;
     
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  cpntr = nadc_get_param_string("outfile");
	  if ((fp_out = fopen(cpntr, "wb+")) == NULL)
	       NADC_RETURN_ERROR(NADC_ERR_FILE_CRE, cpntr);
	  free(cpntr);

	  ENVI_WR_MPH(fp_out, *mph);
	  if (IS_ERR_STAT_FATAL)
	       NADC_RETURN_ERROR(NADC_ERR_PDS_WR, "MPH");
     }
     if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	  ENVI_WR_ASCII_MPH(mph);
	  if (IS_ERR_STAT_FATAL)
	       NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "MPH");
     }
     if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	  SCIA_WR_H5_MPH(mph);
	  if (IS_ERR_STAT_FATAL)
	       NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "MPH");
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SPH
 */
static void SCIA_WRITE_SPH(const struct mph_envi mph,
			   const struct sph1_scia *sph)
{
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_SPH(fp_out, mph, *sph);
	  if (IS_ERR_STAT_FATAL)
	       NADC_RETURN_ERROR(NADC_ERR_PDS_WR, "SPH");
     }
     if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	  SCIA_LV1_WR_ASCII_SPH(sph);
	  if (IS_ERR_STAT_FATAL)
	       NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "SPH");
     }
     if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	  SCIA_LV1_WR_H5_SPH(sph);
	  if (IS_ERR_STAT_FATAL)
	       NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "SPH");
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write DSD
 */
static void SCIA_WRITE_DSD(unsigned int num_dsd,
			   const struct dsd_envi *dsd)
{
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_DSD_INIT(fp_out, num_dsd, dsd);
	  if (IS_ERR_STAT_FATAL)
	       NADC_RETURN_ERROR(NADC_ERR_PDS_WR, "DSD");
     }
     if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	  ENVI_WR_ASCII_DSD(num_dsd, dsd);
	  if (IS_ERR_STAT_FATAL)
	       NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "DSD");
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SQADS
 */
static void SCIA_WRITE_SQADS(unsigned int num_sqads,
			     const struct sqads1_scia *sqads)
{
     if (num_sqads == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_SQADS(fp_out, num_sqads, sqads);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_SQADS(num_sqads, sqads);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "SQADS");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_SQADS(num_sqads, sqads);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "SQADS");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write LADS
 */
static void SCIA_WRITE_LADS(unsigned int num_lads,
			    const struct lads_scia *lads)
{
     if (num_lads == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_LADS(fp_out, num_lads, lads);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_WR_ASCII_LADS(num_lads, lads);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "LADS");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_WR_H5_LADS(num_lads, lads);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "LADS");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SIP
 */
static void SCIA_WRITE_SIP(unsigned int num_sip,
			   const struct sip_scia sip)
{
     if (num_sip == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_SIP(fp_out, num_sip, sip);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_SIP(&sip);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "SIP");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_SIP(&sip);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "SIP");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write CLCP
 */
static void SCIA_WRITE_CLCP(unsigned int num_clcp,
			    const struct clcp_scia clcp)
{
     if (num_clcp == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_CLCP(fp_out, num_clcp, clcp);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_CLCP(&clcp);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "CLCP");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_CLCP(&clcp);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "CLCP");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write VLCP
 */
static void SCIA_WRITE_VLCP(unsigned int num_vlcp,
			    const struct vlcp_scia *vlcp)
{
     if (num_vlcp == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_VLCP(fp_out, num_vlcp, vlcp);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_VLCP(num_vlcp, vlcp);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "VLCP");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_VLCP(num_vlcp, vlcp);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "VLCP");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write PPG
 */
static void SCIA_WRITE_PPG(unsigned int num_ppg,
			   const struct ppg_scia ppg)
{
     if (num_ppg == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_PPG(fp_out, num_ppg, ppg);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_PPG(&ppg);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "PPG");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_PPG(&ppg);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "PPG");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write BASE
 */
static void SCIA_WRITE_BASE(const struct base_scia *base)
{
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_BASE(fp_out, base);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_BASE(base);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "BASE");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_BASE(base);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "BASE");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SCP
 */
static void SCIA_WRITE_SCP(unsigned int num_scp,
			   const struct scp_scia *scp)
{
     if (num_scp == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_SCP(fp_out, num_scp, scp);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_SCP(num_scp, scp);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "SCP");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_SCP(num_scp, scp);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "SCP");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SRS
 */
static void SCIA_WRITE_SRS(unsigned int num_srs,
			   const struct srs_scia *srs)
{
     if (num_srs == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_SRS(fp_out, num_srs, srs);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_SRS(num_srs, srs);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "SRS");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_SRS(num_srs, srs);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "SRS");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write PSPN
 */
static void SCIA_WRITE_PSPN(unsigned int num_pspn,
			    const struct pspn_scia *pspn)
{
     if (num_pspn == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_PSPN(fp_out, num_pspn, pspn);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_PSPN(num_pspn, pspn);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "PSPN");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_PSPN(num_pspn, pspn);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "PSPN");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write PSPL
 */
static void SCIA_WRITE_PSPL(unsigned int num_pspl,
			    const struct psplo_scia *pspl)
{
     if (num_pspl == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_PSPL(fp_out, num_pspl, pspl);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_PSPL(num_pspl, pspl);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "PSPL");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_PSPL(num_pspl, pspl);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "PSPL");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write PSPO
 */
static void SCIA_WRITE_PSPO(unsigned int num_pspo,
			    const struct psplo_scia *pspo)
{
     if (num_pspo == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_PSPO(fp_out, num_pspo, pspo);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_PSPO(num_pspo, pspo);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "PSPO");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_PSPO(num_pspo, pspo);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "PSPO");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write RSPN
 */
static void SCIA_WRITE_RSPN(unsigned int num_rspn,
			    const struct rspn_scia *rspn)
{
     if (num_rspn == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_RSPN(fp_out, num_rspn, rspn);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_RSPN(num_rspn, rspn);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "RSPN");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_RSPN(num_rspn, rspn);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "RSPN");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write RSPL
 */
static void SCIA_WRITE_RSPL(unsigned int num_rspl,
			    const struct rsplo_scia *rspl)
{
     if (num_rspl == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_RSPL(fp_out, num_rspl, rspl);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_RSPL(num_rspl, rspl);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "RSPL");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_RSPL(num_rspl, rspl);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "RSPL");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write RSPO
 */
static void SCIA_WRITE_RSPO(unsigned int num_rspo,
			    const struct rsplo_scia *rspo)
{
     if (num_rspo == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_RSPO(fp_out, num_rspo, rspo);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_RSPO(num_rspo, rspo);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "RSPO");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_RSPO(num_rspo, rspo);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "RSPO");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write EKD
 */
static void SCIA_WRITE_EKD(unsigned int num_ekd,
			   const struct ekd_scia ekd)
{
     if (num_ekd == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_EKD(fp_out, num_ekd, ekd);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_EKD(&ekd);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "EKD");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_EKD(&ekd);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "EKD");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SFP
 */
static void SCIA_WRITE_SFP(unsigned int num_sfp,
			   const struct sfp_scia *sfp)
{
     if (num_sfp == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_SFP(fp_out, num_sfp, sfp);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_SFP(num_sfp, sfp);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "SFP");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_SFP(num_sfp, sfp);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "SFP");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write ASFP
 */
static void SCIA_WRITE_ASFP(unsigned int num_asfp,
			    const struct asfp_scia *asfp)
{
     if (num_asfp == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_ASFP(fp_out, num_asfp, asfp);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_ASFP(num_asfp, asfp);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "ASFP");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_ASFP(num_asfp, asfp);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "ASFP");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write STATE
 */
static void SCIA_WRITE_STATE(unsigned int num_state,
			     const struct state1_scia *state)
{
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_STATE(fp_out, num_state, state);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_STATE(num_state, state);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "STATE");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_STATE(num_state, state);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "STATE");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write PMD
 */
static void SCIA_WRITE_PMD(unsigned int num_pmd,
			   const struct mds1_pmd *pmd)
{
     if (num_pmd == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_PMD(fp_out, num_pmd, pmd);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_PMD(num_pmd, pmd);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "PMD");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_PMD(num_pmd, pmd);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "PMD");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write AUX
 */
static void SCIA_WRITE_AUX(unsigned int num_aux,
			   const struct mds1_aux *aux)
{
     if (num_aux == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_AUX(fp_out, num_aux, aux);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_AUX(num_aux, aux);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "AUX");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_AUX(num_aux, aux);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "AUX");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write LCPN
 */
static void SCIA_WRITE_LCPN(unsigned int num_lcpn,
			    const struct lcpn_scia *lcpn)
{
     if (num_lcpn == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_LCPN(fp_out, num_lcpn, lcpn);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_LCPN(num_lcpn, lcpn);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "LCPN");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_LCPN(num_lcpn, lcpn);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "LCPN");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write DARK
 */
static void SCIA_WRITE_DARK(unsigned int num_dark,
			    const struct dark_scia *dark)
{
     if (num_dark == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_DARK(fp_out, num_dark, dark);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_DARK(num_dark, dark);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "DARK");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_DARK(num_dark, dark);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "DARK");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write PPGN
 */
static void SCIA_WRITE_PPGN(unsigned int num_ppgn,
			    const struct ppgn_scia *ppgn)
{
     if (num_ppgn == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_PPGN(fp_out, num_ppgn, ppgn);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_PPGN(num_ppgn, ppgn);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "PPGN");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_PPGN(num_ppgn, ppgn);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "PPGN");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SCPN
 */
static void SCIA_WRITE_SCPN(unsigned int num_scpn,
			    const struct scpn_scia *scpn)
{
     if (num_scpn == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_SCPN(fp_out, num_scpn, scpn);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_SCPN(num_scpn, scpn);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "SCPN");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_SCPN(num_scpn, scpn);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "SCPN");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write SRSN
 */
static void SCIA_WRITE_SRSN(unsigned int num_srsn,
			    const struct srsn_scia *srsn)
{
     if (num_srsn == 0) return;
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_SRSN(fp_out, num_srsn, srsn);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_SRSN(num_srsn, srsn);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, "SRSN");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_SRSN(num_srsn, srsn);
	       if (IS_ERR_STAT_FATAL) 
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, "SRSN");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write MDS level 1b
 */
static void SCIA_WRITE_MDS_1B(unsigned int num_mds,
			      const struct mds1_scia *mds)
{
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1_WR_MDS(fp_out, num_mds, mds);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1_WR_ASCII_MDS(num_mds, mds);
	       if (IS_ERR_STAT_FATAL)
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, 
				      "SCIA_LV1_WR_ASCII_MDS");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1_WR_H5_MDS(num_mds, mds);
	       if (IS_ERR_STAT_FATAL)
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, 
				      "SCIA_LV1_WR_H5_MDS");
	  }
     }
}


/*+++++++++++++++++++++++++++++++++++++++++++++
 * write MDS level 1c
 */
static void SCIA_WRITE_MDS_1C(unsigned int num_mds,
			      const struct mds1c_scia *mds)
{
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1C_WR_MDS(fp_out, num_mds, mds);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1C_WR_ASCII_MDS(num_mds, mds);
	       if (IS_ERR_STAT_FATAL)
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, 
				      "SCIA_LV1C_WR_ASCII_MDS");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1C_WR_H5_MDS(num_mds, mds);
	       if (IS_ERR_STAT_FATAL)
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, 
				      "SCIA_LV1C_WR_H5_MDS");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write MDS PMD level 1c
 */
static void SCIA_WRITE_MDS_PMD(const struct mds1c_pmd *mds_pmd)
{
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1C_WR_MDS_PMD(fp_out, mds_pmd);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1C_WR_ASCII_MDS_PMD(mds_pmd);
	       if (IS_ERR_STAT_FATAL)
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, 
				      "SCIA_LV1_WR_ASCII_MDS_PMD");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1C_WR_H5_MDS_PMD(mds_pmd);
	       if (IS_ERR_STAT_FATAL)
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, 
				      "SCIA_LV1_WR_H5_MDS_PMD");
	  }
     }
}

/*+++++++++++++++++++++++++++++++++++++++++++++
 * write MDS POLV level 1c
 */
static void SCIA_WRITE_MDS_POLV(const struct mds1c_polV *mds_polV)
{
     if (nadc_get_param_uint8("write_pds") == PARAM_SET) {
	  SCIA_LV1C_WR_MDS_POLV(fp_out, mds_polV);
     } else {
	  if (nadc_get_param_uint8("write_ascii") == PARAM_SET) {
	       SCIA_LV1C_WR_ASCII_MDS_POLV(mds_polV);
	       if (IS_ERR_STAT_FATAL)
		    NADC_RETURN_ERROR(NADC_ERR_FILE_WR, 
				      "SCIA_LV1_WR_ASCII_MDS_POLV");
	  }
	  if (nadc_get_param_uint8("write_hdf5") == PARAM_SET) {
	       SCIA_LV1C_WR_H5_MDS_POLV(mds_polV);
	       if (IS_ERR_STAT_FATAL)
		    NADC_RETURN_ERROR(NADC_ERR_HDF_WR, 
				      "SCIA_LV1_WR_H5_MDS_POLV");
	  }
     }
}
