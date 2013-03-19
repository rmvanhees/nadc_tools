/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2012 - 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   SDMF_get_clusConf
.AUTHOR      R.M. van Hees
.KEYWORDS    SDMF - SCIA cluster configuration
.LANGUAGE    ANSI C
.PURPOSE     obtain cluster configuration parameters
.COMMENTS    contains SDMF_get_clusParam, SDMF_get_statePET, 
               SDMF_get_stateCoadd, SDMF_get_stateCount
	       SDMF_PET2StateID
.ENVIRONment None
.VERSION     1.0     17-May-2012   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <limits.h>
#include <math.h>

/*+++++ Local Headers +++++*/
#include <nadc_sdmf.h>

/*+++++ Macros +++++*/
#define NCLUSDEF  40

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
/* struct clusdef_rec { */
/*      unsigned char  chanID; */
/*      unsigned char  clusID; */
/*      unsigned short start; */
/*      unsigned short length; */
/* }; */

struct clusConf_rec {
     int absOrbit;
     unsigned short count;
     unsigned char coaddf[NCLUSDEF];
     float pet[SCIENCE_CHANNELS];
};

static
const struct clusConf_rec clusConf_65[] = {      /* ADC Cal */
     {1572, 40,
      {16, 16, 16, 16, 16, 16, 8, 8, 8, 16, 16, 16,
       8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
       8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
       8, 8, 8, 8, 8, 8, 8, 8},
      {0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f}},
     {INT_MAX, 0,
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}}
};

static
const struct clusConf_rec clusConf_08[] = {      /* Dark Current 5 */
     {4151, 40,
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       1, 1, 1, 1, 1, 1, 1, 1},
      {5.f, 1.f, 1.f, 1.f, 1.f, 2.f, 1.f, 1.f}},
     {7268, 40,
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       1, 1, 1, 1, 1, 1, 1, 1}, 
      {5.f, 1.f, 1.f, 1.f, 1.f, 5.f, 1.f, 1.f}},
     {43362, 80,
      {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 
       4, 4, 4, 4, 4, 4, 4, 4},
      {0.25f, 0.25f, 1.f, 1.f, 0.25f, 0.5f, 0.125f, 0.125f}},
     {INT_MAX, 0,
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}}
};

static
const struct clusConf_rec clusConf_26[] = {      /* Dark Current 4 */
     {4151, 60,
      {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
       16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 
       2, 2, 2, 2, 2, 16, 16, 16, 16, 16,
       16, 16, 16, 16, 16, 4, 4, 4},
      {0.25f, 0.25f, 0.03125f, 0.03125f, 0.25f, 0.03125f, 0.03125f, 0.125f}},
     {43362, 60,
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       1, 1, 1, 1, 1, 1, 1, 1}, 
      {1.f, 1.5f, 0.75f, 0.75f, 1.5f, 1.5f, 0.5f, 0.5f}},
     {INT_MAX, 0,
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}}
};

static
const struct clusConf_rec clusConf_46[] = {      /* Dark Current 1 */
     {1572, 20,
      {16, 16, 16, 16, 16, 16, 8, 8, 8, 16, 16, 16,
       8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
       8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
       8, 8, 8, 8, 8, 8, 8, 8},
      {0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f,0.0625f, 0.0625f}},
     {4144, 20,
      {16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
       8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
       4, 4, 4, 4, 4, 2, 2, 2, 2, 2,
       8, 8, 8, 8, 8, 8, 8, 8}, 
      {0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.125f, 0.25f, 0.0625f, 0.0625f}},
     {43362, 20,
      {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
       8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
       4, 4, 4, 4, 4, 8, 8, 8, 8, 8,
       16, 16, 16, 16, 16, 8, 8, 8}, 
      {0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.125f, 0.0625f, 0.0625f, 0.0625f}},
     {INT_MAX, 0,
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}}
};

static
const struct clusConf_rec clusConf_63[] = {      /* Dark Current 2 */
     {1990, 30,
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       2, 2, 2, 2, 2, 2, 2, 2}, 
      {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 0.5f, 0.5f}},
     {4144, 60,
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       1, 1, 1, 1, 1, 1, 1, 1}, 
      {1.f, 0.5f, 0.25f, 0.25f, 0.5f, 0.5f, 0.5f, 0.5f}},
     {43362, 80,
      {2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       2, 2, 2, 2, 2, 1, 1, 1}, 
      {0.375f, 0.75f, 0.375f, 0.375f, 0.375f, 0.375f, 0.375f, 0.375f}},
     {INT_MAX, 0,
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}}
};

static
const struct clusConf_rec clusConf_67[] = {      /* Dark Current 3 */
     {1990, 40,
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       1, 1, 1, 1, 1, 1, 1, 1}, 
      {5.f, 5.f, 5.f, 5.f, 5.f, 5.f, 2.f, 2.f}},
     {4144, 40,
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       1, 1, 1, 1, 1, 1, 1, 1}, 
      {10.f, 10.f, 2.f, 2.f, 10.f, 5.f, 2.f, 2.f}},
     {7268, 640,
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
       1, 1, 1, 1, 1, 1, 1, 1}, 
      {10.f, 10.f, 0.125f, 0.125f, 10.f, 0.125f, 2.f, 2.f}},
     {43362, 320,
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
       4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
       1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 
       1, 1, 1, 1, 1, 1, 1, 1},
      {0.5f, 0.5f, 0.125f, 0.125f, 0.5f, 0.125f, 1.f, 1.f}},
     {INT_MAX, 0,
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}}
};

static
const struct clusConf_rec clusConf_16[] = {      /* NDF monitoring */
     {1572, 3,
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
       32, 32, 32, 32, 32, 64, 64, 64, 64, 64, 
       64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
       64, 64, 64, 64, 64, 64, 64, 64}, 
      {4.0, 4.0, 0.125f, 0.03125f, 0.03125f, 0.0144f, 0.03125f, 0.0625f}},
     {7268, 3,
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
       32, 32, 32, 32, 32, 64, 64, 64, 64, 64, 
       64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
       64, 64, 64, 64, 64, 64, 64, 64}, 
      {4.0, 4.0, 0.125f, 0.03125f, 0.03125f, 0.0072f, 0.03125f, 0.0625f}},
     {INT_MAX, 0,
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}}
};

static
const struct clusConf_rec clusConf_48[] = {      /* NDF monitoring */
     {1990, 3,
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
       32, 32, 32, 32, 32, 64, 64, 64, 64, 64, 
       64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
       64, 64, 64, 64, 64, 64, 64, 64}, 
      {4.0, 4.0, 0.125f, 0.03125f, 0.03125f, 0.0144f, 0.03125f, 0.0625f}},
     {7268, 3,
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
       32, 32, 32, 32, 32, 64, 64, 64, 64, 64, 
       64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 
       64, 64, 64, 64, 64, 64, 64, 64}, 
      {4.f, 4.f, 0.125f, 0.03125f, 0.03125f, 0.0072f, 0.03125f, 0.0625f}},
     {INT_MAX, 0,
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}}
};

static
const struct clusConf_rec clusConf_59[] = {      /* SLS */
     {1572, 3,
      {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2,
       32, 32, 32, 32, 32, 64, 64, 64, 64, 64, 
       16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 
       4, 4, 4, 4, 4, 4, 4, 4}, 
      {4.f, 2.f, 0.125f, 0.03125f, 0.25f, 0.25f, 1.f, 1.f}},
     {INT_MAX, 0,
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}}
};

static
const struct clusConf_rec clusConf_69[] = {      /* SLS Diffuser */
     {1572, 2,
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
       2, 2, 2, 2, 2, 4, 4, 4, 4, 4,
       1, 1, 1, 1, 1, 4, 4, 4, 4, 4,
       20, 20, 20, 20, 20, 20, 20, 20},
      {40.f, 40.f, 20.f, 10.f, 40.f, 10.f, 2.f, 2.f}},
     {INT_MAX, 0,
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}}
};

static
const struct clusConf_rec clusConf_52[] = {    /* Sun ESM Diffuser (NDF off)*/
     {1572, 240,
      {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
       2, 2, 2, 2, 2, 1, 1, 1}, 
      {0.0625f, 0.0625f, 0.03125f, 0.03125f, 
       0.03125f, 0.03125f, 0.0625f, 0.125f}},
     {INT_MAX, 0,
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}}
};

static
const struct clusConf_rec clusConf_62[] = {    /* Sun ESM Diffuser (NDF) */
     {1572, 240,
      {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
       2, 2, 2, 2, 2, 1, 1, 1}, 
      {0.0625f, 0.0625f, 0.03125f, 0.03125f, 
       0.0625f, 0.03125f, 0.0625f, 0.125f}},
     {INT_MAX, 0,
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}}
};

static
const struct clusConf_rec clusConf_39[] = {      /* WLS */
     {1990, 6,
      {1, 1, 1, 1, 1, 1, 8, 8, 8, 8, 8, 8,
       16, 16, 16, 16, 16, 32, 32, 32, 32, 32,
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
       32, 32, 32, 32, 32, 32, 32, 32},
      {2.f, 0.25f, 0.125f, 0.03125f, 0.03125f, 0.0072f, 0.0036f, 0.0072f}},
     {INT_MAX, 0,
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}}
};

static
const struct clusConf_rec clusConf_61[] = {      /* WLS */
     {1572, 6,
      {1, 1, 1, 1, 1, 1, 8, 8, 8, 8, 8, 8,
       16, 16, 16, 16, 16, 32, 32, 32, 32, 32,
       32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
       32, 32, 32, 32, 32, 32, 32, 32},
      {2.f, 0.25f, 0.125f, 0.03125f, 0.03125f, 0.0072f, 0.0036f, 0.0072f}},
     {INT_MAX, 0,
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}}
};

static
const struct clusConf_rec clusConf_70[] = {      /* WLS Diffuser */
     {1990, 2,
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
       4, 4, 4, 4, 4, 10, 10, 10, 10, 10,
       10, 10, 10, 10, 10, 40, 40, 40, 40, 40,
       40, 40, 40, 40, 40, 20, 20, 20},
      {40.f, 40.f, 10.f, 4.f, 4.f, 1.f, 1.f, 2.f}},
     {INT_MAX, 0,
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}}
};

/* static */
/* const unsigned short chan2clus[] = {0, 6, 12, 17, 22, 27, 32, 37}; */

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_stateParam
.PURPOSE     obtain number of readouts for given orbit
.INPUT/OUTPUT
  call as    SDMF_get_stateParam( stateID, orbit, channel, &ipet, orbit_range );
    input:
            unsigned char  stateID   :  state ID [1..70]
	    unsigned short orbit     :  orbit number
	    unsigned short channel   :  channel ID [1..8]
   output:
            unsigned short *int_pet  :  pixel exposure time * 32
            int *orbit_range : orbit range with same state definition as orbit

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void SDMF_get_stateParam( unsigned char stateID, unsigned short absOrbit, 
			  unsigned short channel,
			  /*@null@*/ unsigned short *int_pet,
			  /*@null@*/ int *orbit_range )
{
     const char prognm[] = "SDMF_get_stateParam";

     register unsigned short nc = 0;

     const struct clusConf_rec *clusConf;

     switch ( (int) stateID ) {
     case 8:
	  clusConf = clusConf_08;
	  break;
     case 26:
	  clusConf = clusConf_26;
	  break;
     case 46:
	  clusConf = clusConf_46;
	  break;
     case 63:
	  clusConf = clusConf_63;
	  break;
     case 67:
	  clusConf = clusConf_67;
	  break;
     case 16:
	  clusConf = clusConf_16;
	  break;
     case 39:
	  clusConf = clusConf_39;
	  break;
     case 48:
	  clusConf = clusConf_48;
	  break;
     case 52:
	  clusConf = clusConf_52;
	  break;
     case 59:
	  clusConf = clusConf_59;
	  break;
     case 61:
	  clusConf = clusConf_61;
	  break;
     case 62:
	  clusConf = clusConf_62;
	  break;
     case 65:
	  clusConf = clusConf_65;
	  break;
     case 69:
	  clusConf = clusConf_69;
	  break;
     case 70:
	  clusConf = clusConf_70;
	  break;
     default: {
	  char msg[64];

	  (void) snprintf( msg, 64, "undefined clusConf for state: %02hhu\n", 
			   stateID );
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, msg );
	  break;
     }}
     while( absOrbit >= clusConf[nc+1].absOrbit ) nc++;
     if ( absOrbit < clusConf[nc].absOrbit ) goto done;
     if ( orbit_range != NULL ) {
          if ( orbit_range[0] < clusConf[nc].absOrbit )
               orbit_range[0] = clusConf[nc].absOrbit;
          if ( orbit_range[1] >= clusConf[nc+1].absOrbit )
               orbit_range[1] = clusConf[nc+1].absOrbit-1;
     }
     if ( int_pet != NULL ) 
	  *int_pet = __ROUNDf_us( 32 * clusConf[nc].pet[channel-1] );
     return;
done:
     *int_pet = 0;
     orbit_range[0] = orbit_range[1] = 0;
     return;
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_statePET
.PURPOSE     obtain PET for given state, orbit and channel
.INPUT/OUTPUT
  call as    SDMF_get_statePET( stateID, orbit, channel, pet );
    input:
            unsigned char  stateID :  state ID
            unsigned short orbit   :  orbit number
	    unsigned short channel :  channel ID or zero for all channels
   output:
            float                  :  pixel exposure time (s)

.RETURNS     nothing, error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
void SDMF_get_statePET( unsigned char stateID, unsigned short absOrbit,
			unsigned short channel, float *pet )
{
     const char prognm[] = "SDMF_get_statePET";

     const struct clusConf_rec *clusConf;

     register unsigned short nc = 0;

     /* initialize return value */
     *pet = -1.f;

     switch ( (int) stateID ) {
     case 8:
	  clusConf = clusConf_08;
	  break;
     case 26:
	  clusConf = clusConf_26;
	  break;
     case 46:
	  clusConf = clusConf_46;
	  break;
     case 63:
	  clusConf = clusConf_63;
	  break;
     case 67:
	  clusConf = clusConf_67;
	  break;
     case 16:
	  clusConf = clusConf_16;
	  break;
     case 39:
	  clusConf = clusConf_39;
	  break;
     case 48:
	  clusConf = clusConf_48;
	  break;
     case 52:
	  clusConf = clusConf_52;
	  break;
     case 59:
	  clusConf = clusConf_59;
	  break;
     case 61:
	  clusConf = clusConf_61;
	  break;
     case 62:
	  clusConf = clusConf_62;
	  break;
     case 65:
	  clusConf = clusConf_65;
	  break;
     case 69:
	  clusConf = clusConf_69;
	  break;
     case 70:
	  clusConf = clusConf_70;
	  break;
     default: {
	  char msg[64];

	  (void) snprintf( msg, 64, "undefined clusConf for state: %02hhu\n", 
			   stateID );
	  NADC_RETURN_ERROR( prognm, NADC_ERR_FATAL, msg );
	  break;
     }}
     while( absOrbit >= clusConf[nc+1].absOrbit ) nc++;
     if ( absOrbit < clusConf[nc].absOrbit ) return;
     if ( channel == 0 ) {
	  pet[0] = clusConf[nc].pet[0];
	  pet[1] = clusConf[nc].pet[1];
	  pet[2] = clusConf[nc].pet[2];
	  pet[3] = clusConf[nc].pet[3];
	  pet[4] = clusConf[nc].pet[4];
	  pet[5] = clusConf[nc].pet[5];
	  pet[6] = clusConf[nc].pet[6];
	  pet[7] = clusConf[nc].pet[7];
     } else
	  *pet = clusConf[nc].pet[channel-1];
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_stateCoadd
.PURPOSE     obtain co-add factor for given state, orbit and cluster
.INPUT/OUTPUT
  call as    coaddf = SDMF_get_stateCoadd( stateID, orbit, clusID );
    input:
            unsigned char  stateID :  state ID
            unsigned short orbit   :  orbit number
	    unsigned short clusID  :  cluster ID

.RETURNS     co-adding factor (unsigned char),
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned char SDMF_get_stateCoadd( unsigned char stateID, 
				   unsigned short absOrbit,
				   unsigned short clusID )
{
     const char prognm[] = "SDMF_get_stateCoadd";

     const struct clusConf_rec *clusConf;

     register unsigned short nc = 0;

     switch ( (int) stateID ) {
     case 8:
	  clusConf = clusConf_08;
	  break;
     case 26:
	  clusConf = clusConf_26;
	  break;
     case 46:
	  clusConf = clusConf_46;
	  break;
     case 63:
	  clusConf = clusConf_63;
	  break;
     case 67:
	  clusConf = clusConf_67;
	  break;
     case 16:
	  clusConf = clusConf_16;
	  break;
     case 39:
	  clusConf = clusConf_39;
	  break;
     case 48:
	  clusConf = clusConf_48;
	  break;
     case 52:
	  clusConf = clusConf_52;
	  break;
     case 59:
	  clusConf = clusConf_59;
	  break;
     case 61:
	  clusConf = clusConf_61;
	  break;
     case 62:
	  clusConf = clusConf_62;
	  break;
     case 65:
	  clusConf = clusConf_65;
	  break;
     case 69:
	  clusConf = clusConf_69;
	  break;
     case 70:
	  clusConf = clusConf_70;
	  break;
     default: {
	  char msg[64];

	  (void) snprintf( msg, 64, "undefined clusConf for state: %02hhu\n", 
			   stateID );
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, msg );
	  break;
     }}
     while( absOrbit >= clusConf[nc+1].absOrbit ) nc++;
     if ( absOrbit >= clusConf[nc].absOrbit )
	  return clusConf[nc].coaddf[clusID-1];
done:
     return (unsigned char) UCHAR_MAX;
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_get_stateCount
.PURPOSE     obtain number of complete readouts for given state and orbit
.INPUT/OUTPUT
  call as    count = SDMF_get_stateCount( stateID, orbit );
    input:
            unsigned char  stateID :  state ID
            unsigned short orbit   :  orbit number

.RETURNS     number of complete readouts (unsigned short), 
             error status passed by global variable ``nadc_stat''
.COMMENTS    none
-------------------------*/
unsigned short SDMF_get_stateCount( unsigned char stateID, 
				    unsigned short absOrbit )
{
     const char prognm[] = "SDMF_get_stateCount";

     const struct clusConf_rec *clusConf;

     register unsigned short nc = 0;

     switch ( (int) stateID ) {
     case 8:
	  clusConf = clusConf_08;
	  break;
     case 26:
	  clusConf = clusConf_26;
	  break;
     case 46:
	  clusConf = clusConf_46;
	  break;
     case 63:
	  clusConf = clusConf_63;
	  break;
     case 67:
	  clusConf = clusConf_67;
	  break;
     case 16:
	  clusConf = clusConf_16;
	  break;
     case 39:
	  clusConf = clusConf_39;
	  break;
     case 48:
	  clusConf = clusConf_48;
	  break;
     case 52:
	  clusConf = clusConf_52;
	  break;
     case 59:
	  clusConf = clusConf_59;
	  break;
     case 61:
	  clusConf = clusConf_61;
	  break;
     case 62:
	  clusConf = clusConf_62;
	  break;
     case 65:
	  clusConf = clusConf_65;
	  break;
     case 69:
	  clusConf = clusConf_69;
	  break;
     case 70:
	  clusConf = clusConf_70;
	  break;
     default: {
	  char msg[64];

	  (void) snprintf( msg, 64, "undefined clusConf for state: %02hhu\n", 
			   stateID );
	  NADC_GOTO_ERROR( prognm, NADC_ERR_FATAL, msg );
	  break;
     }}
     while( absOrbit >= clusConf[nc+1].absOrbit ) nc++;
     if ( absOrbit >= clusConf[nc].absOrbit )
	  return clusConf[nc].count;
done:
     return 0;
}

/*+++++++++++++++++++++++++
.IDENTifer   SDMF_PET2StateID
.PURPOSE     obtain state ID for given orbit, channel and PET
.INPUT/OUTPUT
  call as    stateID = SDMF_PET2StateID( orbit, channel, pet );
    input:
            unsigned short orbit   :  orbit number
	    unsigned short channel :  channel ID [1..8]
	    float  pet             :  pixel exposure time (s)

.RETURNS     state ID (unsigned char), or UCHAR_MAX on failure
.COMMENTS    none
-------------------------*/
unsigned char SDMF_PET2StateID( unsigned short absOrbit, 
				unsigned short channel,
				float pet )
{
     register unsigned short nc;

     nc = 0;
     while( absOrbit >= clusConf_08[nc+1].absOrbit ) nc++;
     if ( absOrbit >= clusConf_08[nc].absOrbit
	  && fabs( pet - clusConf_08[nc].pet[channel-1] ) < 1e-3 )
	  return (unsigned char) 8;
     nc = 0;
     while( absOrbit >= clusConf_26[nc+1].absOrbit ) nc++;
     if ( absOrbit >= clusConf_26[nc].absOrbit
	  && fabs( pet - clusConf_26[nc].pet[channel-1] ) < 1e-3 )
	  return (unsigned char) 26;
     nc = 0;
     while( absOrbit >= clusConf_46[nc+1].absOrbit ) nc++;
     if ( absOrbit >= clusConf_46[nc].absOrbit
	  && fabs( pet - clusConf_46[nc].pet[channel-1] ) < 1e-3 )
	  return (unsigned char) 46;
     nc = 0;
     while( absOrbit >= clusConf_63[nc+1].absOrbit ) nc++;
     if ( absOrbit >= clusConf_63[nc].absOrbit
	  && fabs( pet - clusConf_63[nc].pet[channel-1] ) < 1e-3 )
	  return (unsigned char) 63;
     nc = 0;
     while( absOrbit >= clusConf_67[nc+1].absOrbit ) nc++;
     if ( absOrbit >= clusConf_67[nc].absOrbit
	  && fabs( pet - clusConf_67[nc].pet[channel-1] ) < 1e-3 )
	  return (unsigned char) 67;

     return UCHAR_MAX;
}

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
#ifdef TEST_PROG
bool Use_Extern_Alloc = FALSE;

int main( int argc, char *argv[] )
{
     /* const char prognm[] = "sdmf_clusConf"; */

     unsigned char  coaddf;
     unsigned char  stateID;
     unsigned short orbit;
     unsigned short count;
     unsigned short int_pet;
     unsigned short channel = 0;
     unsigned short cluster = 1;

     int   orbit_range[2];
     float pet;
     
/*
 * initialization of command-line parameters
 */
     if ( argc <= 2 || (argc > 1 && strncmp( argv[1], "-h", 2 ) == 0) ) {
          (void) fprintf( stderr, 
			  "Usage: %s orbit state [channel] [cluster]\n", 
			  argv[0] );
          exit( EXIT_FAILURE );
     }
     orbit = (unsigned short) atoi( argv[1] );
     stateID = (unsigned char) atoi( argv[2] );
     if ( argc >= 4 ) channel = (unsigned short) atoi( argv[3] );
     if ( argc == 5 ) cluster = (unsigned short) atoi( argv[4] );

     orbit_range[0] = orbit - 100;
     orbit_range[1] = orbit + 100;
     SDMF_get_stateParam( stateID, orbit, channel, &int_pet, orbit_range );
     (void) printf( "# SDMF_get_stateParam(%2hhu, %-hu, %2hu, %s, %s)\n", 
		    stateID, orbit, channel, "&int_pet", "orbit_range" );
     (void) printf( " %3hu    %5d %5d\n", int_pet, orbit_range[0],
		    orbit_range[1] );

     SDMF_get_statePET( stateID, orbit, channel, &pet );
     (void) printf( "# SDMF_get_statePET(%2hhu, %-hu, %2hu, %s)\n", 
		    stateID, orbit, channel, "&pet" );
     (void) printf( " %8.5g\n", pet );

     coaddf = SDMF_get_stateCoadd( stateID, orbit, cluster );
     (void) printf( "# coaddf = SDMF_get_stateCoadd(%2hhu, %-hu, %2hu)\n", 
		    stateID, orbit, cluster );
     (void) printf( " %3hhu\n", coaddf );

     count = SDMF_get_stateCount( stateID, orbit );
     (void) printf( "# count = SDMF_get_stateCount(%2hhu, %-hu)\n", 
		    stateID, orbit );
     (void) printf( " %3hu\n", count );

     stateID = SDMF_PET2StateID( orbit, channel, pet );
     (void) printf( "# stateID = SDMF_PET2StateID(%-hu, %2hu, %.5g)\n", 
		    orbit, channel, pet );
     (void) printf( " %3hhu\n", stateID );

     NADC_Err_Trace( stderr );
     exit( EXIT_SUCCESS );
}
#endif /* TEST_PROG */
