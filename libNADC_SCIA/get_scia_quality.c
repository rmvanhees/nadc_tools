/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2009 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   GET_SCIA_QUALITY
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA - Data Quality
.LANGUAGE    ANSI C
.PURPOSE     obtain data quality
.INPUT/OUTPUT
  call as   res = GET_SCIA_QUALITY( orbit, period );
     input:  
            int orbit   :  absolute orbit number
    output:  
            int *period :  first and last orbit affected by event

.RETURNS     data quality: (unsigned short) 
                       0   = SCIA_Q_OK
                       1   = SCIA_Q_DECON
                       2   = SCIA_Q_RECOVER
                       3   = SCIA_Q_UNAVAIL
		       255 = SCIA_Q_UNKOWN
.COMMENTS    info taken from the SOST pages 
             http://atmos.caf.dlr.de/projects/scops/
.ENVIRONment None
.VERSION     1.2.5   08-May-2012    update, according to SOST tables, RvH
             1.2.4   28-Feb-2012    update, according to SOST tables, RvH
             1.2.3   25-Jan-2012    update, according to SOST tables, RvH
             1.2.2   16-Jan-2012    update, according to SOST tables, RvH
             1.2.1   24-Sep-2011    update, according to SOST tables, RvH
             1.2     13-Sep-2011    introduced usage of enum values, RvHa
             1.1     12-Sep-2011    first check on data availability, RvH
             1.0.13  09-Sep-2011    update, according to SOST tables, RvH
             1.0.12  29-Aug-2011    update, according to SOST tables, RvH
             1.0.11  25-Aug-2011    update, according to SOST tables, RvH
             1.0.10  30-May-2011    update, according to SOST tables, RvH
             1.0.9   06-Apr-2011    update, according to SOST tables, RvH
             1.0.8   28-Mar-2011    update, according to SOST tables, RvH
             1.0.7   24-Nov-2010    update, according to SOST tables, RvH
             1.0.6   11-Nov-2010    update, according to SOST tables, RvH
             1.0.5   12-Oct-2010    update, according to SOST tables, RvH
             1.0.4   07-Sep-2010    update, according to SOST tables, RvH
             1.0.3   16-Aug-2010    update, according to SOST tables, RvH
             1.0.2   16-Jun-2010    update, according to SOST tables, RvH
             1.0.1   24-Jan-2010    update, according to SOST tables, RvH
             1.0     24-Sep-2009    initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _POSIX_SOURCE to indicate
 * that this is a POSIX program
 */
#define  _POSIX_SOURCE 2

/*+++++ System headers +++++*/
	/* NONE */

/*+++++ Local Headers +++++*/
#define _SCIA_COMMON
#include <nadc_scia.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
struct period_rec {
     int start;
     int end;
};

static const struct period_rec decon_arr[] = {
     {35574, 35848},     /* 19-DEC-2008 - 07-JAN-2009 */
     {14676, 14897},     /* 20-DEC-2004 - 05-JAN-2005 */
     {12031, 12208},     /* 18-JUN-2004 - 30-JUN-2004 */
     { 9407,  9673},     /* 18-DEC-2003 - 05-JAN-2004 */
     { 7574,  7827},     /* 12-AUG-2003 - 29-AUG-2003 */
     { 6384,  6449},     /* 21-MAY-2003 - 25-MAY-2003 */
     { 5718,  5766},     /* 04-APR-2003 - 07-APR-2003 */
     { 4204,  4428}      /* ??-DEC-2002 - 04-JAN-2003 */
};

/* taken from SOST "Data Quality History" */
static const struct period_rec atctc_arr[] = {
     {52219, 52225},     /* recovery from HTR/RF */
     {51788, 51802},     /* recovery from STANDBY */
     {51687, 51693},     /* recovery from HTR/RF */
     /* {50318, 50323},     recovery from HTR/RF */
     {50021, 50027},     /* recovery from HTR/RF */
     {49820, 49826},     /* recovery from HTR/RF */
     {49633, 49639},     /* recovery from HTR/RF */
     {49072, 49080},     /* recovery from HTR/RF */
     {48325, 48336},     /* recovery from STANDBY */
     {47567, 47589},     /* recovery from OFF-SAFE */
     {47393, 47404},     /* recovery from STANDBY */
     {45641, 45653},     /* recovery from R/W WAIT */
     {45396, 45408},     /* recovery from R/W WAIT */
     /* {45261, 45352},      platform in YSM */ 
     {44968, 44979},     /* recovery from STANDBY */
     {44351, 44357},     /* recovery from HTR/RF */
     {44148, 44154},     /* recovery from HTR/RF */
     {42476, 42488},     /* recovery from R/W WAIT */
     {41887, 41899},     /* recovery from R/W WAIT */
     {41733, 41738},     /* recovery from HTR/RF */
     {41430, 41442},     /* recovery from R/W WAIT */
     {41201, 41212},     /* recovery from STANDBY */
     {40371, 40383},     /* recovery from R/W WAIT */
     {38938, 38944},     /* recovery from HTR/RF */
     {38230, 38236},     /* recovery from HTR/RF */
     {38153, 38165},     /* recovery from STANDBY */
     {37980, 37992},     /* recovery from STANDBY */
     {36664, 36678},     /* recovery from STANDBY */
     {34906, 34911},     /* recovery from HTR/RF */
     /*  {34643, 34645}, ATC readjustment */
     {33890, 33903},     /* recovery from STANDBY */
     {30264, 30278},     /* recovery from STANDBY */
     {30136, 30150},     /* recovery from STANDBY */
     {29165, 29189},     /* recovery from OFF-SAFE */
     {28318, 28329},     /* recovery from HTR/RF */
     {27873, 27874},     /* recovery from HTR/RF */
     /* {26945, 26955}, TC readjustment */
     {25073, 25081},     /* recovery from OFF-SAFE */
     {24844, 24857},     /* recovery from OFF-SAFE */
     {24754, 24760},     /* recovery from HTR/RF */
     {23698, 23717},     /* recovery from OFF-SAFE */
     {22163, 22170},     /* recovery from HTR/RF */
     {21634, 21640},     /* recovery from HTR/RF */
     {21547, 21552},     /* recovery from HTR/RF */
     {21477, 21498},     /* recovery from OFF-SAFE */
     {20605, 20619},     /* recovery from HTR/RF */
     {20588, 20600},     /* recovery from R/W WAIT */
     {16739, 16755},     /* recovery from R/W WAIT */
     {16686, 16690},     /* recovery from HTR/RF */
     {14898, 14912},     /* recovery from STANDBY & DECON */
     {14217, 14232},     /* recovery from HTR/RF */
     {13546, 13559},     /* recovery from R/W WAIT */
     {13430, 13443},     /* recovery from HTR/RF */
     {12286, 12303},     /* recovery from R/W WAIT */
     {11471, 11482},     /* recovery from HTR/RF */
     { 9883,  9899},     /* recovery from HTR/RF */
     { 9685,  9698},     /* recovery from R/W WAIT */
     { 9239,  9253},     /* recovery from OFF-SAFE */
     { 7954,  7964},     /* recovery from OFF-SAFE */
     { 5516,  5527},     /* recovery from R/W WAIT */
     { 5477,  5491},     /* recovery from STANDBY */
     { 5136,  5152},     /* recovery from STANDBY */
     { 5073,  5086},     /* recovery from R/W WAIT */
     { 4457,  4479},     /* recovery from HTR/RF */
     { 4187,  4199},     /* recovery from HTR */
     { 4110,  4122},     /* recovery from HTR/RF */
     { 3990,  4002},     /* recovery from HTR/RF */
     { 3958,  3970},     /* recovery from HTR/RF */
     { 3788,  3809},     /* recovery from OFF-SAFE */
     { 2781,  2793},     /* recovery from HTR */
     { 2596,  2608},     /* recovery from HTR/RF */
     { 1969,  1981},     /* recovery from R/W WAIT */
     { 1564,  1576},     /* recovery from R/W WAIT */
     { 1511,  1524},     /* recovery from R/W WAIT */
     { 1455,  1476},     /* recovery from OFF-SAFE */
     { 1285,  1306},     /* recovery from OFF-SAFE */
     { 1195,  1207},     /* recovery from R/W WAIT */
     { 1080,  1092},     /* recovery from OFF-SAFE */
     {  868,   880},     /* recovery from HTR/RF */
     {  634,   646}      /* recovery from STB/RF */
};

/* taken from SOST "Data Unavailability History" */
static const struct period_rec unavail_arr[] = {
     {52313, 52318},     /* transfer to MEASUREMENT/IDLE */
     {52204, 52219},     /* transfer to HTR/RF */
     {51772, 51788},     /* transfer to STANDBY */
     {51670, 51687},     /* transfer to HTR/RF */
     {50003, 50021},     /* transfer to HTR/RF */
     {49800, 49820},     /* transfer to HTR/RF */
     {49800, 49820},     /* transfer to HTR/RF */
     {49620, 49633},     /* transfer to HTR/RF */
     {49034, 49072},     /* transfer to HTR/RF */
     {48302, 48325},     /* transfer to STANDBY */
     {47540, 47567},     /* transfer to OFF-SAFE */
     {47370, 47393},     /* transfer to STANDBY */
     {45619, 45641},     /* transfer to R/W WAIT */
     {45379, 45396},     /* transfer to R/W WAIT */
     {45188, 45261},     /* transfer to MEASUREMENT/IDLE */
     {44953, 44968},     /* transfer to STANDBY */
     {44364, 44368},     /* transfer to MEASUREMENT/IDLE */
     {44340, 44351},     /* transfer to HTR/RF */
     {44135, 44148},     /* transfer to HTR/RF */
     {43643, 43647},     /* transfer to MEASUREMENT/IDLE */
     {42640, 42645},     /* transfer to MEASUREMENT/IDLE */
     {42462, 42476},     /* transfer to R/W WAIT */
     /* {41929, 41929},     corrupt data */
     {41867, 41887},     /* transfer to R/W WAIT */
     {41722, 41733},     /* transfer to HTR/RF */
     {41638, 41643},     /* transfer to MEASUREMENT/IDLE */
     {41409, 41430},     /* transfer to R/W WAIT */
     {41186, 41201},     /* transfer to STANDBY */
     {40636, 40641},     /* transfer to MEASUREMENT/IDLE */
     {40355, 40371},     /* transfer to R/W WAIT */
     {39634, 39639},     /* transfer to MEASUREMENT/IDLE */
     /* {39456, 39456},     corrupt data */
     /* {39380, 39380},     corrupt data */
     {38911, 38938},     /* transfer to HTR/RF */
     {38632, 38637},     /* transfer to MEASUREMENT/IDLE */
     {38216, 38230},     /* transfer to HTR/RF */
     {38131, 38153},     /* transfer to STANDBY */
     {37959, 37980},     /* transfer to STANDBY */
     /* {37672, 37672},     corrupt data */
     /* {37438, 37451},     corrupt data (HSM) */
     {37130, 37137},     /* transfer to MEASUREMENT/IDLE */
     /* {36716, 36716},     corrupt data */
     {36647, 36664},     /* transfer to STANDBY */
     /* {36603, 36603},     corrupt data */
     {36126, 36134},     /* transfer to MEASUREMENT/IDLE */
     {35819, 35834},     /* transfer to STANDBY */
     {35125, 35133},     /* transfer to MEASUREMENT/IDLE */
     {34894, 34906},     /* transfer to HTR/RF */
     {34123, 34131},     /* transfer to MEASUREMENT/IDLE */
     {33870, 33890},     /* transfer to STANDBY */
     {33121, 33129},     /* transfer to MEASUREMENT/IDLE */
     {32119, 32127},     /* transfer to MEASUREMENT/IDLE */
     {31117, 31125},     /* transfer to MEASUREMENT/IDLE */
     /* {30741, 30753},     corrupt data (HSM) */
     /* {30463, 30463},     corrupt data */
     {30249, 30264},     /* transfer to STANDBY */
     {30076, 30136},     /* transfer to STANDBY */
     {29107, 29165},     /* transfer to OFF-SAFE */
     {28304, 28318},     /* transfer to HTR/RF */
     {28111, 28119},     /* transfer to MEASUREMENT/IDLE */
     {27856, 27873},     /* transfer to HTR/RF */
     /* {27421, 27722},     Ka-band antenna blocking NCWM */
     {27297, 27299},     /* transfer to MEASUREMENT/IDLE */
     {26609, 26616},     /* transfer to MEASUREMENT/IDLE */
     {25607, 25614},     /* transfer to MEASUREMENT/IDLE */
     {25182, 25183},     /* transfer to MEASUREMENT/IDLE */
     {25016, 25073},     /* transfer to OFF-SAFE */
     {24810, 24844},     /* transfer to OFF-SAFE */
     {24740, 24754},     /* transfer to HTR/RF */
     /* {23916, 23978},     APC anomaly (no Ka-band downlink) */
     {23717, 23725},     /* transfer to MEASUREMENT/IDLE */
     {23641, 23697},     /* transfer to OFF-SAFE */
     /* {22594, 22594},     corrupt data */
     {22177, 22181},     /* transfer to MEASUREMENT/IDLE */
     {22167, 22172},     /* transfer to MEASUREMENT/IDLE */
     {22139, 22163},     /* transfer to HTR/RF */
     {21584, 21634},     /* transfer to HTR/RF */
     {21534, 21547},     /* transfer to HTR/RF */
     {21428, 21479},     /* transfer to OFF-SAFE */
     {21298, 21306},     /* transfer to MEASUREMENT/IDLE */
     {20590, 20606},     /* transfer to HTR/RF */
     {20570, 20588},     /* transfer to R/W WAIT */
     {20196, 20204},     /* transfer to MEASUREMENT/IDLE */
     {18409, 18415},     /* transfer to MEASUREMENT/IDLE */
     {16716, 16739},     /* transfer to R/W WAIT */
     {16675, 16686},     /* transfer to HTR/RF */
     /* {16468, 16471},     X Down Converter anomaly at Kiruna */
     /* {16443, 16444},     X Down Converter anomaly at Kiruna */
     {15916, 15924},     /* transfer to MEASUREMENT/IDLE */
     {14930, 14936},     /* transfer to MEASUREMENT/IDLE */
     {14882, 14898},     /* transfer to STANDBY */
     {14198, 14217},     /* transfer to HTR/RF */
     {13526, 13546},     /* transfer to R/W WAIT */
     {13410, 13430},     /* transfer to HTR/RF */
     {13384, 13393},     /* transfer to MEASUREMENT/IDLE */
     /* {12820, 12820},     corrupt data */
     {12269, 12286},     /* transfer to R/W WAIT */
     {11449, 11471},     /* transfer to HTR/RF */
     {11094, 11100},     /* transfer to MEASUREMENT/IDLE */
     {10092, 10098},     /* transfer to MEASUREMENT/IDLE */
     { 9867,  9883},     /* transfer to HTR/RF */
     /* { 9865,  9866},     no tracking of Kiruna X-band antennas */
     { 9667,  9685},     /* transfer to R/W WAIT */
     { 9439,  9482},     /* transfer to HTR/RF */
     { 9412,  9426},     /* transfer to HTR/RF */
     { 9193,  9239},     /* transfer to OFF-SAFE */
     { 8675,  8684},     /* transfer to MEASUREMENT/IDLE */
     { 8082,  8083},     /* FEC problem */
     { 7793,  7794},     /* FEC problem */
     { 7634,  7664},     /* transfer to HTR/RF */
     { 7309,  7363},     /* transfer to HTR/RF */
     /* { 7236,  7237},     intermittent TLM */
     /* { 7222,  7223},     intermittent TLM */
     /* { 7196,  7197},     intermittent TLM */
     { 6344,  6382},     /* transfer to HTR */
     /* { 5764,  5773},      anomalies (HSM) */
     { 5502,  5516},     /* transfer to R/W WAIT */
     { 5426,  5477},     /* transfer to STANDBY */
     { 5099,  5136},     /* transfer to STANDBY */
     { 5034,  5073},     /* transfer to R/W WAIT */
     { 4429,  4457},     /* transfer to HTR/RF */
     { 4180,  4187},     /* transfer to HTR */
     { 4169,  4178},     /* transfer to MEASUREMENT/IDLE */
     { 4093,  4110},     /* transfer to HTR/RF */
     { 3982,  3990},     /* transfer to HTR/RF */
     { 3925,  3958},     /* transfer to HTR/RF */
     { 3752,  3788},     /* transfer to OFF-SAFE */
     { 3643,  3696},     /* corrupt data */
     /* { 3399,  3399},     transfer to MEASUREMENT/IDLE (partly unavailable) */
     { 2737,  2781},     /* transfer to HTR */
     { 2586,  2596},     /* transfer to HTR/RF */
     { 1952,  1969},     /* transfer to R/W WAIT */
     { 1550,  1564},     /* transfer to R/W WAIT */
     { 1488,  1511},     /* transfer to R/W WAIT */
     { 1483,  1488},     /* transfer to HTR/RF */
     { 1385,  1455},     /* transfer to OFF-SAFE */
     { 1245,  1285},     /* transfer to OFF-SAFE */
     { 1238,  1245},     /* transfer to R/W WAIT */
     { 1113,  1195},     /* transfer to R/W WAIT */
     { 1022,  1080},     /* transfer to OFF-SAFE */
     {  842,   868},     /* transfer to HTR/RF */
     {  627,   634}      /* transfer to STB/RF */
};

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
unsigned short GET_SCIA_QUALITY( int orbit, int *period )
{
     register unsigned short nr;

     const unsigned short NumDecon  = (unsigned short) 
	  (sizeof(decon_arr) / sizeof(struct period_rec));
     const unsigned short NumATC_TC = (unsigned short) 
	  (sizeof(atctc_arr) / sizeof(struct period_rec));
     const unsigned short NumUnAvail = (unsigned short) 
	  (sizeof(unavail_arr) / sizeof(struct period_rec));

     if ( period != NULL ) {
	  period[0] = period[1] = 0;
     }

     for ( nr = 0; nr < NumUnAvail; nr++ ) {  /* data unavailable */
	  if ( unavail_arr[nr].start <= orbit 
	       && unavail_arr[nr].end >= orbit ) {
	       if ( period != NULL ) {
		    period[0] = unavail_arr[nr].start;
		    period[1] = unavail_arr[nr].end;
	       } 
	       return SCIA_Q_UNAVAIL;
	  }
     }
     for ( nr = 0; nr < NumDecon; nr++ ) {   /* decontamination */
	  if ( decon_arr[nr].start <= orbit 
	       && decon_arr[nr].end >= orbit ) {
	       if ( period != NULL ) {
		    period[0] = decon_arr[nr].start;
		    period[1] = decon_arr[nr].end;
	       } 
	       return SCIA_Q_DECON;
	  }
     }
     for ( nr = 0; nr < NumATC_TC; nr++ ) {  /* recovery ATC/TC affected */
	  if ( atctc_arr[nr].start <= orbit 
	       && atctc_arr[nr].end >= orbit ) {
	       if ( period != NULL ) {
		    period[0] = atctc_arr[nr].start;
		    period[1] = atctc_arr[nr].end;
	       } 
	       return SCIA_Q_RECOVER;
	  }
     }
     return SCIA_Q_OK;
}
