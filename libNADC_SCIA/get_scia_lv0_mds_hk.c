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

.IDENTifer   GET_SCIA_LV0_MDS_HK
.AUTHOR      R.M. van Hees
.KEYWORDS    Sciamachy L0 
.LANGUAGE    ANSI C
.PURPOSE     Extract house keeping data from L0 MDS records
.COMMENTS    contains: GET_SCIA_LV0_DET_PET,
                       GET_SCIA_LV0_MDS_ANGLES, GET_SCIA_LV0_MDS_TEMP, 
		       GET_SCIA_LV0_MDS_TIME, GET_SCIA_LV0_STATE_ANGLE, 
		       GET_SCIA_LV0_STATE_DETtemp, GET_SCIA_LV0_STATE_OBMtemp,
		       GET_SCIA_LV0_STATE_PMDtemp
.ENVIRONment None
.VERSION     1.4     04-May-2010   GET_SCIA_LV0_STATE_OBMtemp, returns OBM 
                                   temperature accoding to SOST or SDMF pre-v3.1
             1.3     03-Mar-2010   fixed long standing bug: 
                                     mixing radTemp and azTemp, RvH
             1.2     01-Feb-2010   calculate correct OBM temperature, RvH
             1.1.1   11-Dec-2007   fixed false virtual channel indication, RvH
             1.1     14-Nov-2007   fixed a bug in GET_SCIA_LV0_DET_PET, RvH
             1.0     12-Nov-2006   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*+++++ Local Headers +++++*/
#define _SCIA_LEVEL_0
#include <nadc_scia.h>

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static
double _InterPol( unsigned short adim, const unsigned short Xarr[],
		  const double Yarr[], unsigned short Xval )
{
     register unsigned short nd;

     for ( nd = 0; nd < adim; nd++ )
	  if ( Xval < Xarr[nd] ) break;

     if ( nd == 0 ) 
	  return Yarr[0];
     else if ( nd == adim ) 
	  return Yarr[adim-1];
     else
	  return (Yarr[nd] 
		  - (Yarr[nd]-Yarr[nd-1]) 
		  * (Xarr[nd] - Xval) / (Xarr[nd] - Xarr[nd-1]));
}

static
double GET_SCIA_LV0_AUX_radTemp( const union bench_cntrl bench_rad )
{
     const double CO = 74423.0912;
     const double QP = 8. / 65536;
     const double FF = 0.183497965;

     const double aa = 9.3809e-4;
     const double bb = 2.2099e-4;
     const double cc = 1.2655e-7;

     double R_l = CO * ( 2. * bench_rad.field.temp * QP - FF );
     double R_t = (1e6 * R_l) / (1e6 - R_l);

     if ( bench_rad.field.stat != 0 || R_t < 1e-12 ) return NAN;
     
     return (1. / (aa + bb * log(R_t) + cc * pow(log(R_t), 3)));
}

static
double GET_SCIA_LV0_AUX_elvTemp( const union bench_cntrl bench_elv )
{
     const double CO = 74439.72096;
     const double QP = 8. / 65536;
     const double FF = 0.184288384;

     const double aa = 9.2998e-4;
     const double bb = 2.2188e-4;
     const double cc = 1.2568e-7;

     double R_l = CO * ( 2. * bench_elv.field.temp * QP - FF );
     double R_t = (1e6 * R_l) / (1e6 - R_l);

     if ( bench_elv.field.stat != 0 || R_t < 1e-12 ) return NAN;
     
     return (1. / (aa + bb * log(R_t) + cc * pow(log(R_t), 3)));
}

static
double GET_SCIA_LV0_AUX_azTemp( const union bench_cntrl bench_az )
{
     const double CO = 74419.32288;
     const double QP = 8. / 65536;
     const double FF = 0.184583046;

     const double aa = 9.3590e-4;
     const double bb = 2.2119e-4;
     const double cc = 1.2683e-7;

     double R_l = CO * ( 2. * bench_az.field.temp * QP - FF );
     double R_t = (1e6 * R_l) / (1e6 - R_l);

     if ( bench_az.field.stat != 0 || R_t < 1e-12 ) return NAN;
     
     return (1. / (aa + bb * log(R_t) + cc * pow(log(R_t), 3)));
}

/*+++++++++++++++++++++++++
.IDENTifer   GET_SCIA_LV0_MDS_TEMP
.PURPOSE     calculates the temperature (K) of parts of Sciamachy
.INPUT/OUTPUT
  call as   GET_SCIA_LV0_MDS_TEMP( source, mds_lv0, temp )
     input:  
            int  source    : type of L0 MDS record
	    void *mds_lv0  : L0 MDS records (AUX, DET of PMD)
    output:  
            float *temp    : AUX -> optical bench temperature (array[])
                             DET -> detector arrays temperatures (array[8])
                             PMD -> temperature of PMD block (array[1])
.RETURNS     nothing
.COMMENTS    static function
-------------------------*/
static
void GET_SCIA_LV0_MDS_TEMP( int source, const void *mds_lv0, float *temp )
{
     if ( source == SCIA_AUX_PACKET ) {
	  register unsigned short nf;

	  const struct mds0_aux *aux = (const struct mds0_aux *) mds_lv0;

	  double radTemp;

	  for ( nf = 0; nf < NUM_LV0_AUX_PMTC_FRAME; nf++ ) {
	       radTemp = 
		    GET_SCIA_LV0_AUX_radTemp(aux->data_src[nf].bench_rad);
	       temp[nf] = isnormal(radTemp) ? (float)(0.7 + radTemp) : NAN;
	  }
     } else if ( source == SCIA_DET_PACKET ) {
	  register unsigned short n_ch;

	  int  chan_indx;

	  unsigned short ustemp;

	  const struct mds0_det *det = (const struct mds0_det *) mds_lv0;
	  
	  const unsigned short tab_tm[SCIENCE_CHANNELS][16] = {
	       {0, 17876, 18312, 18741, 19161, 19574, 19980, 20379, 
		20771, 21157, 21908, 22636, 24684, 26550, 28259, 65535},
	       {0, 18018, 18456, 18886, 19309, 19724, 20131, 20532,
		20926, 21313, 22068, 22798, 24852, 26724, 28436, 65535},
	       {0, 20601, 20996, 21384, 21765, 22140, 22509, 22872,
		23229, 23581, 23927, 24932, 26201, 27396, 28523, 65535},
	       {0, 20333, 20725, 21110, 21490, 21863, 22230, 22591,
		22946, 23295, 23640, 24640, 25905, 27097, 28222, 65535},
	       {0, 20548, 20942, 21330, 21711, 22086, 22454, 22817,
		23174, 23525, 23871, 24875, 26144, 27339, 28466, 65535},
	       {0, 17893, 18329, 18758, 19179, 19593, 20000, 20399,
		20792, 21178, 21931, 22659, 24709, 26578, 28289, 65535},
	       {0, 12994, 13526, 14046, 14555, 15054, 15543, 16022,
		16492, 17850, 20352, 22609, 24656, 26523, 28232, 65535},
	       {0, 13129, 13664, 14188, 14702, 15204, 15697, 16180,
		16653, 18019, 20536, 22804, 24860, 26733, 28447, 65535},
	  };
	  const double tab_temp[SCIENCE_CHANNELS][16] = {
	       {179., 180., 185., 190., 195., 200., 205., 210., 
		215., 220., 230., 240., 270., 300., 330., 331.},
	       {179., 180., 185., 190., 195., 200., 205., 210.,
		215., 220., 230., 240., 270., 300., 330., 331.},
	       {209., 210., 215., 220., 225., 230., 235., 240.,
		245., 250., 255., 270., 290., 310., 330., 331.},
	       {209., 210., 215., 220., 225., 230., 235., 240.,
		245., 250., 255., 270., 290., 310., 330., 331.},
	       {209., 210., 215., 220., 225., 230., 235., 240.,
		245., 250., 255., 270., 290., 310., 330., 331.},
	       {179., 180., 185., 190., 195., 200., 205., 210.,
		215., 220., 230., 240., 270., 300., 330., 331.},
	       {129., 130., 135., 140., 145., 150., 155., 160.,
		165., 180., 210., 240., 270., 300., 330., 331.},
	       {129., 130., 135., 140., 145., 150., 155., 160.,
		165., 180., 210., 240., 270., 300., 330., 331.},
	  };
	  for ( n_ch = 0; n_ch < SCIENCE_CHANNELS; n_ch++ ) temp[n_ch] = NAN;

	  for ( n_ch = 0; n_ch < SCIENCE_CHANNELS; n_ch++ ) {
	       if ( n_ch == det->num_chan ) break;
	       chan_indx = (int) det->data_src[n_ch].hdr.channel.field.id - 1;
	       ustemp = det->data_src[n_ch].hdr.temp;

	       temp[chan_indx] = (float) 
		    _InterPol( 16, tab_tm[chan_indx], tab_temp[chan_indx], 
			       ustemp );
	  }
     } else {            /* SCIA_PMD_PACKET */
	  const struct mds0_pmd *pmd = (const struct mds0_pmd *) mds_lv0;
	  
	  const unsigned short ustemp = pmd->data_src.temp;

	  const unsigned short tab_tm[16] = {
	       1008, 1762, 2167, 3448, 5446, 8971, 13940, 14871, 15828,
	       16809, 17812, 20761, 23208, 25944, 27960, 30884 };
	  const double tab_temp[16] = {
	       60., 45., 41., 27., 13., -2., -16., -18., -20., -22.,
	       -24., -30., -35., -41., -46., -55. };

	  *temp = (float)(273.15 + _InterPol( 16, tab_tm, tab_temp, ustemp ));
     }
}

/*++++++++++++++++++++++++ Main Program or Functions +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   GET_SCIA_LV0_MDS_TIME
.PURPOSE     calculate Julian day as fraction number of days since 01-01-2000
.INPUT/OUTPUT
  call as   jday = GET_SCIA_LV0_MDS_TIME( source, mds_lv0 );
     input:  
            int  source    : type of L0 MDS record
	    void *mds_lv0  : L0 MDS records (AUX, DET of PMD)
            
.RETURNS     Julian day (double) of the first valid packet
.COMMENTS    none
-------------------------*/
double GET_SCIA_LV0_MDS_TIME( int source, const void *mds_lv0 )
{
     register unsigned short ni;

     bool found = FALSE;

     double dsec;
     double delay = 0.;
     double jday = NAN;

     const double sec2day = 24. * 60 * 60;

     const unsigned short ri[MAX_NUM_STATE] = {
	  86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 
	  86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 
	  86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 
	  86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 
	  86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 
	  86, 86, 86, 86, 86, 86, 86, 86, 111, 86, 
	  303, 86, 86, 86, 86, 86, 86, 86, 111, 303
     };

     if ( source == SCIA_AUX_PACKET ) {
	  const struct mds0_aux *aux = (const struct mds0_aux *) mds_lv0;

	  const unsigned short AUX_SYNC = 0xDDDD;

          ni = 0;
          do {
               register unsigned short nj = 0;

               do {
                    if ( aux->data_src[ni].bcp[nj].sync == AUX_SYNC ) {
                         found = TRUE;
                         delay = aux->data_src[ni].bcp[nj].bcps / 16.;
                    }
               } while ( ! found && ++nj < NUM_LV0_AUX_BCP );
          } while ( ! found && ++ni < NUM_LV0_AUX_PMTC_FRAME );
          if ( found ) {
	       dsec  = aux->isp.secnd + aux->isp.musec / 1e6
		    + ri[aux->data_hdr.state_id-1] / 256.
		    + delay;
	       jday = aux->isp.days + dsec / sec2day;
	  }
     } else if ( source == SCIA_DET_PACKET ) {
	  const struct mds0_det *det = (const struct mds0_det *) mds_lv0;

	  dsec = det->isp.secnd + det->isp.musec / 1e6
	       + ri[det->data_hdr.state_id-1] / 256.
	       + det->data_src[0].hdr.bcps / 16.;
	  jday = det->isp.days + dsec / sec2day;
     } else {            /* SCIA_PMD_PACKET */
	  const struct mds0_pmd *pmd = (const struct mds0_pmd *) mds_lv0;

	  const unsigned short PMD_SYNC = 0xEEEE;

          ni = 0;
          do {
               if ( pmd->data_src.packet[ni].sync == PMD_SYNC ) {
                    found = TRUE;

                    delay = pmd->data_src.packet[ni].time.field.delta / 500.;
                    delay -= 12.5;
		    delay /= 1e3;
                    delay += pmd->data_src.packet[ni].bcps / 16.;
               }
          } while ( ! found && ++ni < NUM_LV0_PMD_PACKET );
          if ( found ) {
	       dsec = pmd->isp.secnd + pmd->isp.musec / 1e6
		    + ri[pmd->data_hdr.state_id-1] / 256.
		    + delay;
	       jday = pmd->isp.days + dsec / sec2day;
	  }
     }
     return jday;
}

/*+++++++++++++++++++++++++
.IDENTifer   GET_SCIA_LV0_MDS_TIME_ARR
.PURPOSE     calculate Julian day as fraction number of days since 01-01-2000
             Detector MDS:  time when a packet was assembled
              $T = T_{ICU} + bcps / 16.$
             Auxiliary MDS: time when a BCP was assembled
              $T[0:79] = T_{ICU} + bcps[0:79] / 16.$
             PMD MDS:       time when a PMD data packet was assembled
              $T[0:199] = T_{ICU} + bcps[0:199]/16. + (2e-3 * delta_T-12.5)/1e3$
.INPUT/OUTPUT
  call as   GET_SCIA_LV0_MDS_TIME_ARR( source, mds_lv0, jday );
     input:  
            int  source    : type of L0 MDS record
	    void *mds_lv0  : L0 MDS records (AUX, DET of PMD)
    output:
            double *jday   : Julian day
.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void GET_SCIA_LV0_MDS_TIME_ARR( int source, const void *mds_lv0, 
				/*@ou@*/ double *jday )
{
     register unsigned short ni;
     register double delay;

     double dsec;

     const double sec2day = 24. * 60 * 60;

     const unsigned short ri[MAX_NUM_STATE] = {
	  86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 
	  86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 
	  86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 
	  86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 
	  86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 
	  86, 86, 86, 86, 86, 86, 86, 86, 111, 86, 
	  303, 86, 86, 86, 86, 86, 86, 86, 111, 303
     };

     if ( source == SCIA_AUX_PACKET ) {
	  const struct mds0_aux *aux = (const struct mds0_aux *) mds_lv0;

	  const unsigned short AUX_SYNC = 0xDDDD;

	  dsec = aux->isp.secnd + aux->isp.musec / 1e6
	       + ri[aux->data_hdr.state_id-1] / 256.;
	  ni = 0;
	  do { 
	       register unsigned short nj = 0;

	       do {
		    if ( aux->data_src[ni].bcp[nj].sync == AUX_SYNC ) {
			 delay = aux->data_src[ni].bcp[nj].bcps / 16.;

			 *jday++ = aux->isp.days + (dsec + delay) / sec2day;
		    } else
			 *jday++ = NAN;
	       } while ( ++nj < NUM_LV0_AUX_BCP );
	  } while ( ++ni < NUM_LV0_AUX_PMTC_FRAME );
     } else if ( source == SCIA_PMD_PACKET ) {
	  const struct mds0_pmd *pmd = (const struct mds0_pmd *) mds_lv0;

	  const unsigned short PMD_SYNC = 0xEEEE;

	  dsec = pmd->isp.secnd + pmd->isp.musec / 1e6
	       + ri[pmd->data_hdr.state_id-1] / 256.;
	  ni = 0;
	  do {
	       if ( pmd->data_src.packet[ni].sync == PMD_SYNC ) {
		    delay = pmd->data_src.packet[ni].time.field.delta / 500.;
		    delay -= 12.5;
		    delay /= 1e3;
		    delay += pmd->data_src.packet[ni].bcps / 16.;

		    *jday++ = pmd->isp.days + (dsec + delay) / sec2day;
	       } else
		    *jday = NAN;
	  } while ( ++ni < NUM_LV0_PMD_PACKET );
     } else {            /* SCIA_DET_PACKET */
	  const struct mds0_det *det = (const struct mds0_det *) mds_lv0;

	  dsec = det->isp.secnd + det->isp.musec / 1e6
	       + ri[det->data_hdr.state_id-1] / 256. 
	       + det->data_src->hdr.bcps / 16.;
	  *jday = det->isp.days + dsec / sec2day;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   GET_SCIA_LV0_MDS_ANGLES
.PURPOSE     calculates the ASM and ASM mirrors
.INPUT/OUTPUT
  call as   GET_SCIA_LV0_MDS_ANGLES( aux, asmAngle, esmAngle );
     input:  
            struct mds0_aux *aux : L0 Auxiliary MDS record
    output:  
            float *asmAngle      : azimuth scan angles
            float *esmAngle      : elevation scan angles

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void GET_SCIA_LV0_MDS_ANGLES( const struct mds0_aux *aux, 
			      float *asmAngle, float *esmAngle )
{
     register unsigned short nbcp, nframe;

     unsigned int aziCntr, eleCntr;

     double aziOff, eleOff;

     const unsigned short PMTC_SYNC = 0xDDDD;
     const double scaleFactor = 360. / 640000.;

     if ( (aux->data_hdr.rdv.field.config_id % 2) == 1 ) {
	  aziOff = -108.18143;
	  eleOff = -19.2340;
     } else {
	  aziOff = -18.18943;
	  eleOff = -109.2425;
     }

     for ( nframe = 0; nframe < NUM_LV0_AUX_PMTC_FRAME; nframe++ ) {
	  for ( nbcp = 0; nbcp < NUM_LV0_AUX_BCP; nbcp++ ) {
	       *asmAngle = *esmAngle = NAN;
	       if ( aux->data_src[nframe].bcp[nbcp].sync == PMTC_SYNC ) {
		    aziCntr = 
			 aux->data_src[nframe].bcp[nbcp].azi_encode_cntr;
		    eleCntr = 
			 aux->data_src[nframe].bcp[nbcp].ele_encode_cntr;

		    if ( aziCntr != 0u )
			 *asmAngle = (float) (aziOff + scaleFactor * aziCntr);
		    if ( eleCntr != 0u )
			 *esmAngle = (float) (eleOff + scaleFactor * eleCntr);
	       }
	       asmAngle++; esmAngle++;
	  }
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   GET_SCIA_LV0_STATE_ANGLE
.PURPOSE     average scan angles during a number of L0 auxiliary packages
.INPUT/OUTPUT
  call as   GET_SCIA_LV0_STATE_ANGLE( num_aux, aux, &asmAngle, &esmAngle );
     input:  
            unsigned short num_aux : number of auxiliary packages
	    struct mds0_aux *aux   : L0 Auxiliary MDS records
    output:  
            float  *asmAngle       : average azimuth scan angle
	    float  *esmAngle       : average elevation scan angle

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void GET_SCIA_LV0_STATE_ANGLE( unsigned short num_aux, 
			       const struct mds0_aux *aux,
			       float *asmAngle, float *esmAngle )
{
     register unsigned short na, nframe, nbcp;

     register unsigned short numAngle = 0;
     register double asmSum = 0.;
     register double esmSum = 0.;

     float asmBuff[NUM_LV0_AUX_PMTC_FRAME * NUM_LV0_AUX_BCP];
     float esmBuff[NUM_LV0_AUX_PMTC_FRAME * NUM_LV0_AUX_BCP];

     for ( na = 0; na < num_aux; na++ ) {
	  register unsigned short indx = 0;

	  GET_SCIA_LV0_MDS_ANGLES( aux+na, asmBuff, esmBuff );
	  for ( nframe = 0; nframe < NUM_LV0_AUX_PMTC_FRAME; nframe++ ) {
	       for ( nbcp = 0; nbcp < NUM_LV0_AUX_BCP; nbcp++ ) {
		    if ( isnormal(asmBuff[indx]) && isnormal(esmBuff[indx]) ) {
			 numAngle++;
			 asmSum += asmBuff[indx];
			 esmSum += esmBuff[indx];
		    }
		    indx++;
	       }
	  }
     }
     if ( numAngle > 0 ) {
	  *asmAngle = (float)(asmSum / numAngle);
	  *esmAngle = (float)(esmSum / numAngle);
     } else
	  *asmAngle = *esmAngle = NAN;
}

/*+++++++++++++++++++++++++
.IDENTifer   GET_SCIA_LV0_STATE_OBMtemp
.PURPOSE     average OBM temperature during a number of L0 auxiliary packages
.INPUT/OUTPUT
  call as   GET_SCIA_LV0_STATE_OBMtemp( sost_t_obm, num_aux, aux, &obmTemp );
     input:
            bool  sost_obm         : return OBM temperature according to SOST
                                     otherwise return (wrong) SDMF pre-v3.1
            unsigned short num_aux : number of auxiliary packages
	    struct mds0_aux *aux   : L0 Auxiliary MDS records
    output:  
            float *obmTemp         : temperature of the optical bench [K]

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void GET_SCIA_LV0_STATE_OBMtemp( bool sost_obm, unsigned short num_aux, 
				 const struct mds0_aux *aux, 
				 /*@out@*/ float *obmTemp )
{
     register unsigned short na = 0;
     register unsigned short numTemp = 0;

     register double obmSum = 0.;

     register const struct pmtc_frame *pmtc_ptr;
/*
 * initialize return value
 */
     *obmTemp = NAN;
     if ( num_aux == 0 ) return;
/*
 * calculate average OBM temperature during this state
 */ 
     if ( sost_obm ) {
	  do {
	       register unsigned short nf = 0;
	       register double az_buff, elv_buff;

	       do {
		    pmtc_ptr = &aux->data_src[nf];
		    az_buff = GET_SCIA_LV0_AUX_azTemp( pmtc_ptr->bench_az );
		    elv_buff = GET_SCIA_LV0_AUX_elvTemp( pmtc_ptr->bench_elv );

		    if ( isnormal( az_buff ) && isnormal( elv_buff ) ) {
			 numTemp++;
			 obmSum += (az_buff + elv_buff ) / 2;
		    }
	       } while ( ++nf < NUM_LV0_AUX_PMTC_FRAME );
	       aux++;
	  } while ( ++na < num_aux );

	  if ( numTemp > 0 ) *obmTemp = (float) (obmSum / numTemp - 2.2);
     } else {
	  do {
	       register unsigned short nf = 0;
	       register double rad_buff;

	       do {
		    pmtc_ptr = &aux->data_src[nf];
		    rad_buff = GET_SCIA_LV0_AUX_azTemp( pmtc_ptr->bench_rad );
		    
		    if ( isnormal( rad_buff ) ) {
			 numTemp++;
			 obmSum += rad_buff;
		    }
	       } while ( ++nf < NUM_LV0_AUX_PMTC_FRAME );
	       aux++;
	  } while ( ++na < num_aux );

	  if ( numTemp > 0 ) *obmTemp = (float) (0.7 + obmSum / numTemp);
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   GET_SCIA_LV0_STATE_DETtemp
.PURPOSE     average Detector block temperature during a number of L0 packages
.INPUT/OUTPUT
  call as   GET_SCIA_LV0_STATE_DETtemp( num_mds, det, detTemp );
     input:  
            unsigned short num_mds : number of MDS records
            struct mds0_det *det   : L0 Detector MDS records
    output:  
            float  *detTemp        : temperature of the detector blocks [K]
.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void GET_SCIA_LV0_STATE_DETtemp( unsigned short num_mds, 
				 const struct mds0_det *det,
				 float *detTemp )
{
     register unsigned short nd, nchan;

     unsigned short numTemp[SCIENCE_CHANNELS] = {
	  0, 0, 0, 0, 0, 0, 0, 0
     };
     double detSum [SCIENCE_CHANNELS] = {
	  0., 0., 0., 0., 0., 0., 0., 0.
     };
     float detBuff[SCIENCE_CHANNELS];

     for ( nd = 0; nd < num_mds; nd++ ) {
	  GET_SCIA_LV0_MDS_TEMP( SCIA_DET_PACKET, det+nd, detBuff );
	  for ( nchan = 0; nchan < SCIENCE_CHANNELS; nchan++ ) {
	       if ( isnormal( detBuff[nchan] ) ) {
		    numTemp[nchan] += 1;
		    detSum[nchan]  += detBuff[nchan];
	       }
	  }
     }
     for ( nchan = 0; nchan < SCIENCE_CHANNELS; nchan++ ) {
	  if ( numTemp[nchan] > 0 )
	       detTemp[nchan] = (float)(detSum[nchan] / numTemp[nchan]);
	  else
	       detTemp[nchan] = NAN;
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   GET_SCIA_LV0_STATE_PMDtemp
.PURPOSE     average PMD temperature during a number of L0 PMD packages
.INPUT/OUTPUT
  call as   GET_SCIA_LV0_STATE_PMDtemp( num_pmd, pmd, &pmdTemp );
     input:  
            unsigned short num_pmd : number of PMD packages
	    struct mds0_pmd *pmd   : L0 PMD MDS records
    output:  
            float *pmdTemp         : temperature of the PMD block [K]

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void GET_SCIA_LV0_STATE_PMDtemp( unsigned short num_pmd, 
				 const struct mds0_pmd *pmd, 
				 /*@out@*/ float *pmdTemp )
{
     register unsigned short np;

     register unsigned short numTemp = 0;

     register double pmdSum = 0.;

     float pmdBuff;

     for ( np = 0; np < num_pmd; np++ ) {
	  GET_SCIA_LV0_MDS_TEMP( SCIA_PMD_PACKET, pmd+np, &pmdBuff );
	  if ( isnormal( pmdBuff ) ) {
	       numTemp++;
	       pmdSum += pmdBuff;
	  }
     }
     if ( numTemp > 0 )
	  *pmdTemp = (float)(pmdSum / numTemp);
     else
	  *pmdTemp = NAN;
}

/*+++++++++++++++++++++++++
.IDENTifer   GET_SCIA_LV0_DET_PET
.PURPOSE     get pixel exposure time
.INPUT/OUTPUT
  call as   GET_SCIA_LV0_DET_PET( chan_hdr, pet, &vir_chan_b );
     input:  
            struct chan_hdr hdr     : L0 MDS channel header
    output:  
            float *pet              : pixel exposure time [sec]
                                      if (*vir_chan_b != 0) pet = fltarr(2)
	    unsigned short *vir_chan_b : virtual channel indicator

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void GET_SCIA_LV0_DET_PET( struct chan_hdr hdr, float *pet, 
			   unsigned short *vir_chan_b )
{
     const char prognm[] = "GET_SCIA_LV0_DET_PET";

     const unsigned short CHANNEL_SYNC = 0xAAAA;

/* initialise return values */
     *vir_chan_b = 0;
     pet[0] = pet[1] = NAN;

/* check validity of channel header */
     if ( hdr.sync != CHANNEL_SYNC ) {
	  NADC_RETURN_ERROR( prognm, NADC_WARN_PDS_RD, 
			     "incorrect Channel Sync value(s) found" );
     }
/* handle visual channels different from infra-red channel */
     if ( hdr.channel.field.id <= VIS_CHANNELS ) {         /* Channels 1 - 5 */
	  if ( hdr.command_vis.field.etf == UCHAR_ZERO )
	       pet[0] = 31.25e-3;
	  else
	       pet[0] = 62.5e-3 * hdr.command_vis.field.etf;

	  if ( hdr.command_vis.field.sec > UCHAR_ONE ) { 
	       pet[1] = pet[0];
	       *vir_chan_b = (unsigned short) (2 * hdr.command_vis.field.sec);
	       if ( hdr.command_vis.field.ratio > UCHAR_ONE )
		    pet[0] *= hdr.command_vis.field.ratio;
	  }
     } else {                                               /* Channel 6 - 8 */
	  if ( hdr.command_ir.field.mode == UCHAR_ZERO ) {     /*normal mode */
	       if ( hdr.command_ir.field.etf == UCHAR_ZERO )
		    pet[0] = 31.25e-3;
	       else
		    pet[0] = 62.5e-3 * hdr.command_ir.field.etf;

	       /* pet[0] -= 1.18125e-3; */
	  } else if ( hdr.command_ir.field.mode == UCHAR_ONE )   /*hot mode */
	       pet[0] = 28.125e-6 * ldexp(1., (int) hdr.command_ir.field.pet);
     }
}
