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

.IDENTifer   NADC_IDL
.AUTHOR      R.M. van Hees
.KEYWORDS    header file
.LANGUAGE    ANSI C
.PURPOSE     prototyping of additional IDL modules
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     15-Jul-2008   initial release by R. M. van Hees
------------------------------------------------------------*/

#ifndef  NADC_PROTO_IDL                       /* Avoid redefinitions */
#define  NADC_PROTO_IDL

#include <idl_export.h>

#if (defined _GOME_COMMON)
#include <nadc_gome.h>
#elif (defined _SCIA_CALIB)
#define _SCIA_LEVEL_1
#include <nadc_scia_cal.h>
#else
#include <nadc_scia.h>
#endif

extern int IDL_STDCALL OpenFile( int, void ** );
extern int IDL_STDCALL CloseFile( void );
extern int IDL_STDCALL Err_Clear( void );
extern int IDL_STDCALL Err_Trace( int, void ** );

#ifdef _GOME_COMMON
extern int IDL_STDCALL _GOME_LV1_RD_FSR ( int, void ** );
extern int IDL_STDCALL _GOME_LV1_RD_SPH ( int, void ** );
extern int IDL_STDCALL _GOME_LV1_RD_FCD ( int, void ** );
extern int IDL_STDCALL _GOME_LV1_RD_PCD ( int, void ** );
extern int IDL_STDCALL _GOME_LV1_RD_SMCD ( int, void ** );
extern int IDL_STDCALL _GOME_LV1_PCD_PMD ( int, void ** );
extern int IDL_STDCALL _GOME_LV1_SMCD_PMD ( int, void ** );
extern int IDL_STDCALL _GOME_LV1_RD_BDR ( int, void ** );
extern int IDL_STDCALL _GOME_LV1_RD_SMBDR ( int, void ** );
#endif

#ifdef _SCIA_COMMON
extern unsigned int IDL_STDCALL _NADC_SCIA_CalibMask( int, void ** );
extern unsigned short IDL_STDCALL _GET_SCIA_QUALITY( int, void ** );
extern int IDL_STDCALL _ENVI_RD_MPH( int, void ** );
extern int IDL_STDCALL _ENVI_RD_DSD( int, void ** );
extern int IDL_STDCALL _SCIA_RD_LADS( int, void ** );
extern int IDL_STDCALL _GET_SCIA_ROE_ORBITPHASE( int, void ** );
extern int IDL_STDCALL _GET_SCIA_ROE_INFO( int, void ** );
extern int IDL_STDCALL _SCIA_LV0_RD_SPH ( int, void ** );
extern int IDL_STDCALL _GET_LV0_MDS_INFO ( int, void ** );
extern int IDL_STDCALL _SCIA_LV0_RD_MDS_INFO ( int, void ** );
extern int IDL_STDCALL _SCIA_LV0_WR_MDS_INFO ( int, void ** );
extern int IDL_STDCALL _SCIA_LV0_RD_AUX ( int, void ** );
extern int IDL_STDCALL _SCIA_LV0_RD_DET ( int, void ** );
extern int IDL_STDCALL _SCIA_LV0_RD_PMD ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_ASFP ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_AUX ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_BASE ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_CLCP ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_DARK ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_EKD ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_LCPN ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_PMD ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_PPG ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_PPGN ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_PSPN ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_PSPL ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_PSPO ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_RSPN ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_RSPL ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_RSPO ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_SCP ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_SCPN ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_SFP ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_SIP ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_SPH ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_SQADS ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_SRS ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_SRSN ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_STATE ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_VLCP ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_MDS ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_MDS_PMD ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_RD_MDS_POLV ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1_SCALE_MDS ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1C_RD_CALOPT ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1C_RD_MDS ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1C_RD_MDS_PMD ( int, void ** );
extern int IDL_STDCALL _SCIA_LV1C_RD_MDS_POLV ( int, void ** );
extern int IDL_STDCALL _SCIA_LV2_RD_BIAS ( int, void ** );
extern int IDL_STDCALL _SCIA_LV2_RD_CLD ( int, void ** );
extern int IDL_STDCALL _SCIA_LV2_RD_DOAS ( int, void ** );
extern int IDL_STDCALL _SCIA_LV2_RD_GEO ( int, void ** );
extern int IDL_STDCALL _SCIA_LV2_RD_SPH ( int, void ** );
extern int IDL_STDCALL _SCIA_LV2_RD_SQADS ( int, void ** );
extern int IDL_STDCALL _SCIA_LV2_RD_STATE ( int, void ** );
extern int IDL_STDCALL _SCIA_OL2_RD_CLD ( int, void ** );
extern int IDL_STDCALL _SCIA_OL2_RD_LGEO ( int, void ** );
extern int IDL_STDCALL _SCIA_OL2_RD_NGEO ( int, void ** );
extern int IDL_STDCALL _SCIA_OL2_RD_LCLD ( int, void ** );
extern int IDL_STDCALL _SCIA_OL2_RD_LFIT ( int, void ** );
extern int IDL_STDCALL _SCIA_OL2_RD_NFIT ( int, void ** );
extern int IDL_STDCALL _SCIA_OL2_RD_SPH ( int, void ** );
extern int IDL_STDCALL _SCIA_OL2_RD_SQADS ( int, void ** );
extern int IDL_STDCALL _SCIA_RD_MFACTOR ( int, void ** );

#ifdef _HDF5_H
extern int IDL_STDCALL _SCIA_RD_H5_MEMCORR( int , void ** );
extern int IDL_STDCALL _SCIA_WR_H5_MEMCORR( int , void ** );
extern int IDL_STDCALL _SCIA_RD_H5_NLCORR( int , void ** );
extern int IDL_STDCALL _SCIA_WR_H5_NLCORR( int , void ** );
extern int IDL_STDCALL _SCIA_RD_H5_STRAYLIGHT( int , void ** );
extern int IDL_STDCALL _SCIA_WR_H5_STRAYLIGHT( int , void ** );
#endif
#endif

#ifdef _SCIA_COMMON
extern int IDL_STDCALL _SDMF_RD_PT_CLUSDEF( int , void ** );
extern int IDL_STDCALL _SDMF_RD_PT_GEO_ATTR( int , void ** );
extern int IDL_STDCALL _SDMF_RD_PT_CLUS_ATTR( int , void ** );
extern int IDL_STDCALL _SDMF_GET_PT_ORBITINDEX( int , void ** );
extern int IDL_STDCALL _SDMF_RD_PT_POINTING( int , void ** );
extern int IDL_STDCALL _SDMF_RD_PT_CLUSTER( int , void ** );

extern unsigned short IDL_STDCALL _SCIA_GET_ORBIT_PARAMS( int, void ** );
#endif

#endif
