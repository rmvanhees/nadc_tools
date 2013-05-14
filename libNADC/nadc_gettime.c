/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2013 SRON (R.M.van.Hees@sron.nl)

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

.IDENTifer   NADC_GETTIME
.AUTHOR      R.M. van Hees
.KEYWORDS    
.LANGUAGE    ANSI C
.PURPOSE     obtain precise system time from OS for code timing
.COMMENTS    contains nadc_set_start_time, nadc_set_stop_time,
                      nadc_get_epoch_time
.ENVIRONment None
.VERSION     1.0     14-May-2013   initial release by R. M. van Hees
------------------------------------------------------------*/
/*
 * Define _GNU_SOURCE to indicate
 * that this is a GNU program
 */
#define _GNU_SOURCE

/*+++++ System headers +++++*/
#ifdef __MACH__
#include <mach/mach_time.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#endif

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*+++++ Macros +++++*/
	/* NONE */

/*+++++ Global Variables +++++*/
	/* NONE */

/*+++++ Static Variables +++++*/
#ifdef __MACH__
static uint64_t  t1, t2;
#else
static clockid_t clk_id = CLOCK_REALTIME;

static struct timespec t1, t2;
#endif

static double  conversion_factor = 1.;

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
				/* NONE */

/*+++++++++++++++++++++++++ Main Program or Function +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   nadc_set_start_time
.PURPOSE     retrieve the time of system-wide clock that measures real time
.INPUT/OUTPUT
  call as    nadc_set_start_time();
.RETURNS     nothing
.COMMENTS    none
-------------------------*/
#ifdef __MACH__
void nadc_set_start_time( void )
{
     if ( conversion_factor < 0. ) {
	  mach_timebase_info_data_t timebase;

	  mach_timebase_info( &timebase );
	  conversion_factor = (double)timebase.numer / (double)timebase.denom;
     }
     t1 = mach_absolute_time();
}
#else
void nadc_set_start_time( void )
{
     if ( conversion_factor < 0. ) {
        struct timespec res;

        (void) clock_getres( clk_id, &res);
        conversion_factor = 1. / (1000. * res.tv_nsec);
     }
     (void) clock_gettime( clk_id, &t1 );
}
#endif

/*+++++++++++++++++++++++++
.IDENTifer   nadc_set_stop_time
.PURPOSE     retrieve the time of system-wide clock that measures real time
.INPUT/OUTPUT
  call as    nadc_set_stop_time();
.RETURNS     nothing
.COMMENTS    none
-------------------------*/
#ifdef __MACH__
void nadc_set_stop_time( void )
{
     t2 = mach_absolute_time();
}
#else
void nadc_set_stop_time( void )
{
     (void) clock_gettime( clk_id, &t2 );
}
#endif

/*+++++++++++++++++++++++++
.IDENTifer   nadc_get_epoch_time
.PURPOSE     retrieve elapse time between time stop and start
.INPUT/OUTPUT
  call as    epoch_time = nadc_get_epoch_time();
.RETURNS     seconds, precision in nano-seconds (double)
.COMMENTS    none
-------------------------*/
#ifdef __MACH__
double nadc_get_epoch_time( void )
{
     return (double)(t2 - t1) * conversion_factor;
}

#else
double nadc_get_epoch_time( void )
{
     return (double) (t2.tv_sec - t1.tv_sec)
     + (double) (t2.tv_nsec - t1.tv_nsec) * conversion_factor;
}
#endif
