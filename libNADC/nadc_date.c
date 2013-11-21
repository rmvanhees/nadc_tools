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

.IDENTifer   NADC_DATE
.AUTHOR      R.M. van Hees
.KEYWORDS    date convertion
.LANGUAGE    ANSI C
.COMMENTS    contains ASCII_2_UTC, UTC_2_ASCII, ASCII_2_MJD and MJD_2_ASCII
                      UTC_2_DATETIME, MJD_2_DATETIME, MJD_2_YMD,
		      MJD_2_Julian and Julian_2_MJD, SciaJDAY2adaguc, 
		      Adaguc2sciaJDAY, GomeJDAY2adaguc, Adaguc2gomeJDAY
.ENVIRONment none
.VERSION      1.8.1 23-Oct-2008	added GomeJDAY2adaguc, Adaguc2gomeJDAY, RvH
              1.8   22-Oct-2008	added SciaJDAY2adaguc, Adaguc2sciaJDAY, RvH
              1.7   30-Jul-2007	added MJD_2_YMD, KB
              1.6   29-Jan-2007	added UTC_2_DATETIME, MJD_2_DATETIME, RvH
              1.5   22-Aug-2005	bugfixes and some rewriting after reading 
                                "Julian dates from Meeus (Python)"
                                exported Julian_2_MJD and MJD_2_Julian, RvH
              1.4   13-Jan-2003	renamed module 
                                removed unnecessary use of unsigned
                                sscanf needed more checks for Time-string, RvH
              1.3   21-Dec-2001	added ASCII_2_MJD, RvH
              1.2   04-Jan-2001	distinction between GOME and SCIA date 
                                   format and use of snprintf, RvH
              1.1   08-Aug-2000 do not use fraction of second, RvH
              1.0   04-Aug-1999 created by R. M. van Hees
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
#include <ctype.h>
#include <math.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/*
 * Gregorian Calander was adopted on Oct. 15, 1582
 *
 * static const int IGREG = 15 + 31 * (10 + 12 * 1582);
 */

/*
 * Julian date at 01/01/1950 and 01/01/2000
 */
static const double jday_01011950 = 2433282.5;
static const double jday_01012000 = 2451544.5;

/*
 * array for month names (in capitals)
 */
#define MONTHinYEAR       12
static const char *Month_Names[MONTHinYEAR] = {
     "JAN", "FEB", "MAR", "APR", "MAY", "JUN", 
     "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" 
};

static const unsigned short Month_Days[MONTHinYEAR] = {
     31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
static
unsigned int GET_MONTH_INDX( const char *month_str )
{
     register unsigned short month_idx = 0;

     char str_buff[4];
/*
 * make sure that the first 3 characters in the input string are in upper-case
 */
     str_buff[0] = (char) toupper( (int) month_str[0] );
     str_buff[1] = (char) toupper( (int) month_str[1] );
     str_buff[2] = (char) toupper( (int) month_str[2] );
     str_buff[3] = '\0';
/*
 * compare the first 3 characters with the reference array
 */
     while ( month_idx < MONTHinYEAR 
	     && strncmp(Month_Names[month_idx], str_buff, 3) != 0 ) 
	  month_idx++;

     return (month_idx + 1);
}

/*+++++++++++++++++++++++++ Exported Functions +++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   Julian_2_MJD
.PURPOSE     return the calendar date for given julian date
.INPUT/OUTPUT
  call as    Julian_2_MJD( jday, &day, &imon, &iyear );

     input:
            double       jday   :  Julian day number (starts at noon)
    output:
            double       *day   :  number of day of the month.
            unsigned int *imon  :  number of the month (1 = january, ... )
            unsigned int *iyear :  number of the year

.RETURNS     nothing
.COMMENTS    static function
             Taken from "Numerical Recipies in C", by William H. Press,
	     Brian P. Flannery, Saul A. Teukolsky, and William T. Vetterling.
	     Cambridge University Press, 1988 (second printing).
-------------------------*/
void Julian_2_MJD( double jday, 
		   double *day, unsigned int *imon, unsigned int *iyear )
{
     int    ja, jalpha, jb, jc, jd, je;
     double frac_day;

     jd = (int) (jday += 0.5);
     frac_day = jday - jd;
     if ( jd >= 2299161 ) {
	  jalpha = (int) ((jd - 1867216.26)/ 36254.25);
	  ja = jd + 1 + jalpha - (int) (jalpha / 4);
     } else {
	  ja = jd;
     }
     jb = ja + 1524;
     jc = (int)((jb - 122.1) / 365.25);
     jd = (int)(365.25 * jc);
     je = (int)((jb - jd) / 30.6001);
     *day = jb - jd - (int) (30.6001 * je) + frac_day;

     *imon = (unsigned int) (je - 1u);
     if ( *imon > MONTHinYEAR ) *imon -= MONTHinYEAR;
     *iyear = (unsigned int) (jc - 4715U);
     if ( *imon > 2 ) --(*iyear);
}

/*+++++++++++++++++++++++++
.IDENTifer   MJD_2_Julian
.PURPOSE     return julian day for given the calendar date
.INPUT/OUTPUT
  call as    MJD_2_Julian( day, imon, iyear, &jday );

     input:
            double       day   :  number of day of the month.
            unsigned int imon  :  number of the month (1 = january, ... )
            unsigned int iyear :  number of the year
    output:
            double       *jday :  Julian day number (starts at noon)

.RETURNS     nothing
.COMMENTS    static function
             Taken from "Numerical Recipies in C", by William H. Press,
	     Brian P. Flannery, Saul A. Teukolsky, and William T. Vetterling.
	     Cambridge University Press, 1988 (second printing).
-------------------------*/
void MJD_2_Julian( double day, unsigned int imon, unsigned int iyear, 
		   double *jday )
{
     int jy, jm;

     jy = (int) iyear;
     if ( imon > 2 ) {
	  jm = imon + 1;
     } else {
	  jy--;
	  jm = imon + 13;
     }
     *jday = floor(365.25 * jy) + floor(30.6001 * jm) + day + 1720994.5;
/*
 * test whether to change to Gregorian calendar
 */
     if ( (jy + jm / 100. + day / 10000.) >= 1582.1015 ) {
	  int ja = jy / 100;
	  *jday += (2 - ja + (ja / 4));
     }
}

/*+++++++++++++++++++++++++
.IDENTifer   ASCII_2_UTC
.PURPOSE     Converts ASCII time into ESA UTC time 
.INPUT/OUTPUT
  call as    ASCII_2_UTC( ASCII_Time, &utc_day, &utc_sec );

     input:
            char *ASCII_Time      :  given as DD-MMM-YYYY HH:MM:SS.SSS
    output:
            unsigned int utc_day  :  ESA UTC days since 01.01.1950
            unsigned int utc_msec :  ESA UTC milli-seconds since midnight

.RETURNS     error status
.COMMENTS    none
-------------------------*/
void ASCII_2_UTC( const char ASCII_DateTime[], 
		  unsigned int *utc_day, unsigned int *utc_msec )
{
     char   ASCII_mon[4];                   /* input month (string) */
     char   ASCII_Date[12];                 /* date part of input string */
     char   ASCII_Time[13];                 /* time part of input string */

     unsigned int  imin, ihour, iday, imon, iyear;
     double        jday, day, sec;

     const double SecPerDay = 24. * 60. * 60.;
/*
 * initialisation
 */
     *utc_day  = 0U;
     *utc_msec = 0U;
/*
 * decomposition of input string into its date and time part
 */
     (void) nadc_strlcpy( ASCII_Date, ASCII_DateTime, 12 );
     (void) nadc_strlcpy( ASCII_Time, ASCII_DateTime+12, 13 );
/*
 * decomposition of date and time string into numbers into the tm struct
 */
     (void) sscanf( ASCII_Date, " %2u %*c %3s %*c %4u ", 
		    &iday, ASCII_mon, &iyear );
     if ( strlen( ASCII_Time ) < 6 ) {
	  (void) sscanf( ASCII_Time, " %2u %*c %2u ", &ihour, &imin );
	  sec = 0.;
     } else {
	  (void) sscanf( ASCII_Time, " %2u %*c %2u %*c %lf ", 
			 &ihour, &imin, &sec );
     }
     imon = GET_MONTH_INDX( ASCII_mon );
     day = iday + (ihour + ((imin + (sec / 60.)) / 60.)) / 24.;
/*
 * calculate output parameters
 */
     MJD_2_Julian( day, imon, iyear, &jday );
     jday -= (jday_01011950);
     *utc_day = (unsigned int) jday;
     if ( jday < 0 ) (*utc_day)--;
     *utc_msec = (unsigned int) (1000U * SecPerDay * (jday - (*utc_day)));
}

/*+++++++++++++++++++++++++
.IDENTifer   ASCII_2_MJD
.PURPOSE     Converts ASCII time into SCIAMACHY MJD
.INPUT/OUTPUT
  call as    ASCII_2_MJD( ASCII_Time, &mjd2000, &second, &mu_sec );

     input:
            char *ASCII_Time      :  given as DD-MMM-YYYY HH:MM:SS.SSSSSS
    output:
            signed int   *mjd2000 :  number of days elapsed since 1.1.2000
            unsigned int *second  :  seconds elepsed since midnight
            unsigned int *mu_sec  :  micro-seconds since last second

.RETURNS     error status
.COMMENTS    none
-------------------------*/
void ASCII_2_MJD( const char ASCII_DateTime[], int *mjd2000, 
		  unsigned int *second, unsigned int *mu_sec )
{
     char   ASCII_mon[4];                   /* input month (string) */
     char   ASCII_Date[12];                 /* date part of input string */
     char   ASCII_Time[16];                 /* time part of input string */

     unsigned int imin, ihour, iday, imon, iyear;
     double       day, jday, sec, frac_sec;

     const double SecPerDay = 24. * 60. * 60.;
/*
 * initialisation
 */
     *mjd2000 = 0;
     *second  = 0U;
     *mu_sec  = 0U;
/*
 * decomposition of input string into its date and time part
 */
     (void) nadc_strlcpy( ASCII_Date, ASCII_DateTime, 12 );
     (void) nadc_strlcpy( ASCII_Time, ASCII_DateTime+12, 16 );
/*
 * decomposition of date and time string into numbers into the tm struct
 */
     (void) sscanf( ASCII_Date, " %2u %*c %3s %*c %4u ", 
		    &iday, ASCII_mon, &iyear );
     if ( strlen( ASCII_Time ) < 6 ) {
	  (void) sscanf( ASCII_Time, " %2u %*c %2u ", &ihour, &imin );
	  sec = 0.;
     } else {
	  (void) sscanf( ASCII_Time, "%2u %*c %2u %*c %lf", 
			 &ihour, &imin, &sec );
     }
     imon = GET_MONTH_INDX( ASCII_mon );
     day = iday + (ihour + ((imin + (sec / 60.)) / 60.)) / 24.;
     frac_sec = sec - floor(sec);
/*
 * calculate output values
 */
     MJD_2_Julian( day, imon, iyear, &jday );
     jday -= (jday_01012000);
     *mjd2000 = (int) (jday);
     if ( jday < 0 ) (*mjd2000)--;
     *second = (unsigned int) (SecPerDay * (jday - (*mjd2000)));
     *mu_sec = (unsigned int) (1000000. * frac_sec + 0.5);
}

/*+++++++++++++++++++++++++
.IDENTifer   UTC_2_ASCII
.PURPOSE     Converts ESA UTC time into a ASCII time
.INPUT/OUTPUT
  call as    UTC_2_ASCII( utc_day, utc_msec, ASCII_Time );

     input:
            unsigned int utc_day  :  ESA UTC days since 01.01.1950
            unsigned int utc_msec :  ESA UTC milli-seconds since midnight
    output:
            char *ASCII_Time      :  returned as DD-MMM-YYYY HH:MM:SS.SSS

.RETURNS     nothing
.COMMENTS    see GOME date specification
-------------------------*/
void UTC_2_ASCII( unsigned int utc_day, unsigned int utc_msec, 
		  char ASCII_DateTime[] )
{
     unsigned int ihour, imin, imon, imsec, isec, iyear;
     double       mday, jday;
/*
 * convert to julian day
 */
     jday = utc_day + jday_01011950;
     Julian_2_MJD( jday, &mday, &imon, &iyear );

     isec = (unsigned int) (utc_msec / 1000U);
     imsec = (unsigned int) (utc_msec - 1000 * isec);
     ihour = isec / 3600; 
     isec -= 3600 * ihour;
     imin  = isec / 60;
     isec -= 60 * imin;
     (void) snprintf( ASCII_DateTime, DATE_STRING_LENGTH,
		      "%.2d-%3s-%.4u %.2u:%.2u:%.2u.%.3u", (int) mday,
		      Month_Names[imon-1], iyear, ihour, imin, isec, imsec );
}

/*+++++++++++++++++++++++++
.IDENTifer   MJD_2_ASCII
.PURPOSE     Converts SCIAMACHY MJD into a ASCII time
.INPUT/OUTPUT
  call as    MJD_2_ASCII( mjd2000, second, mu_sec, ASCII_Time );

     input:
            signed int mjd2000  :  number of days elapsed since 1.1.2000
            unsigned int second :  seconds elepsed since midnight
            unsigned int mu_sec :  micro-seconds since last second
    output:
            char *ASCII_Time    :  returned as DD-MMM-YYYY HH:MM:SS.SSSSSS

.RETURNS     nothing
.COMMENTS    see SCIAMACHY date specification
-------------------------*/
void MJD_2_ASCII( int mjd2000, unsigned int second, unsigned int mu_sec, 
		  char *ASCII_DateTime )
{
     unsigned int ihour, imin, imon, isec, iyear;
     double       mday, jday;
/*
 * convert to julian day
 */
     jday = mjd2000 + jday_01012000;
     Julian_2_MJD( jday, &mday, &imon, &iyear );

     ihour = (unsigned int)(second / 3600);
     isec  = (unsigned int) second - (3600 * ihour);
     imin  = isec / 60;
     isec -= 60 * imin;
     (void) snprintf( ASCII_DateTime, UTC_STRING_LENGTH,
		      "%.2d-%3s-%.4u %.2u:%.2u:%.2u.%.6u", (int) mday, 
		      Month_Names[imon-1], iyear, ihour, imin, isec, mu_sec );
}

/*+++++++++++++++++++++++++
.IDENTifer   UTC_2_DATETIME
.PURPOSE     Converts ESA UTC time into a MySQL dateTime string
.INPUT/OUTPUT
  call as    UTC_2_DATETIME( utc_day, utc_msec, dateTime );

     input:
            unsigned int utc_day  :  ESA UTC days since 01.01.1950
            unsigned int utc_msec :  ESA UTC milli-seconds since midnight
    output:
            char *dateTime        :  returned as YYYY-MM-DD HH:MM:SS.SSS

.RETURNS     nothing
.COMMENTS    see GOME date specification
-------------------------*/
void UTC_2_DATETIME( unsigned int utc_day, unsigned int utc_msec, 
		     char dateTime[] )
{
     unsigned int  ihour, imin, imon, imsec, isec, iyear;
     double        mday, jday;
/*
 * convert to julian day
 */
     jday = utc_day + jday_01011950;
     Julian_2_MJD( jday, &mday, &imon, &iyear );

     isec = (unsigned int) (utc_msec / 1000U);
     imsec = (unsigned int) (utc_msec - 1000 * isec);
     ihour = isec / 3600; 
     isec -= 3600 * ihour;
     imin  = isec / 60;
     isec -= 60 * imin;
     (void) snprintf( dateTime, DATE_STRING_LENGTH,
		      "%.4u-%.2u-%.2d %.2u:%.2u:%.2u.%.3u", iyear,
		      imon, (int) mday, ihour, imin, isec, imsec );
}

/*+++++++++++++++++++++++++
.IDENTifer   DATETIME_2_JULIAN
.PURPOSE     Converts MySQL dateTime string to Julian day
.INPUT/OUTPUT
  call as    jday = DATETIME_2_MJD( dateTime, muSeconds );

     input:
            char *dateTime        :  given as YYYY-MM-DD HH:MM:SS
	    int  muSeconds        :  micro-seconds since last second

.RETURNS     Julian day (double)
.COMMENTS    none
-------------------------*/
double DATETIME_2_JULIAN( const char dateTime[], unsigned int muSecond )
{
     char   ASCII_Date[12];                 /* date part of input string */
     char   ASCII_Time[16];                 /* time part of input string */

     unsigned int imin, ihour, iday, imon, iyear;
     double       day, jday, sec;
/*
 * decomposition of input string into its date and time part
 */
     (void) nadc_strlcpy( ASCII_Date, dateTime, 11 );
     (void) nadc_strlcpy( ASCII_Time, dateTime+11, 16 );
/*
 * decomposition of date and time string into numbers into the tm struct
 */
     (void) sscanf( ASCII_Date, " %4u %*c %2u %*c %2u ", 
		    &iyear, &imon, &iday );
     if ( strlen( ASCII_Time ) < 6 ) {
	  (void) sscanf( ASCII_Time, " %2u %*c %2u ", &ihour, &imin );
	  sec = muSecond / 1e6;
     } else {
	  (void) sscanf( ASCII_Time, "%2u %*c %2u %*c %lf", 
			 &ihour, &imin, &sec );
	  sec += (muSecond / 1e6);
     }
     day = iday + (ihour + (imin + (sec / 60.)) / 60.) / 24.;
/*
 * calculate output values
 */
     MJD_2_Julian( day, imon, iyear, &jday );

     return jday - jday_01012000;
}

/*+++++++++++++++++++++++++
.IDENTifer   MJD_2_DATETIME
.PURPOSE     Converts SCIAMACHY MJD into a MySQL dateTime string
.INPUT/OUTPUT
  call as    MJD_2_DATETIME( mjd2000, second, mu_sec, dateTime );

     input:
            signed int mjd2000  :  number of days elapsed since 1.1.2000
            unsigned int second :  seconds elepsed since midnight
            unsigned int mu_sec :  micro-seconds since last second
    output:
            char *dateTime      :  returned as YYYY-MM-DD HH:MM:SS.SSSSSS

.RETURNS     nothing
.COMMENTS    see SCIAMACHY date specification
-------------------------*/
void MJD_2_DATETIME( int mjd2000, unsigned int second, unsigned int mu_sec, 
		     char *dateTime )
{
     unsigned int ihour, imin, imon, isec, iyear;
     double       mday, jday;
/*
 * convert to julian day
 */
     jday = mjd2000 + jday_01012000;
     Julian_2_MJD( jday, &mday, &imon, &iyear );

     ihour = (unsigned int)(second / 3600);
     isec  = (unsigned int) second - (3600 * ihour);
     imin  = isec / 60;
     isec -= 60 * imin;
     (void) snprintf( dateTime, UTC_STRING_LENGTH-1,
		      "%.4u-%.2u-%.2d %.2u:%.2u:%.2u.%.6u", iyear, 
		      imon, (int) mday, ihour, imin, isec, mu_sec );
}

/*+++++++++++++++++++++++++ KB
.IDENTifer   MJD_2_YMD
.PURPOSE     Converts SCIAMACHY MJD into a dateTime string, 
             as used in aux. files
.INPUT/OUTPUT
  call as    MJD_2_YMD( mjd2000, second, dateTime );
     input:
            signed int mjd2000  :  number of days elapsed since 1.1.2000
            unsigned int second :  seconds elepsed since midnight
            unsigned int mu_sec :  micro-seconds since last second
    output:
            char *dateTime      :  returned as YYYYMMDD_HH:MM:SS

.RETURNS     nothing
.COMMENTS    see SCIAMACHY date specification
-------------------------*/
void MJD_2_YMD( int mjd2000, unsigned int second, char *dateTime )
{
     unsigned int ihour, imin, imon, isec, iyear;
     double       mday, jday;
/*
 * convert to julian day
 */
     jday = mjd2000 + jday_01012000;
     Julian_2_MJD( jday, &mday, &imon, &iyear );

     ihour = (unsigned int)(second / 3600);
     isec  = (unsigned int) second - (3600 * ihour);
     imin  = isec / 60;
     isec -= 60 * imin;
     (void) snprintf( dateTime, UTC_STRING_LENGTH-1,
		      "%.4u%.2u%.2d_%.2u%.2u%.2u", iyear, 
		      imon, (int) mday, ihour, imin, isec);
}

/*+++++++++++++++++++++++++ 
.IDENTifer   SciaJDAY2adaguc
.PURPOSE     Converts julian (2000) decimal day into a dateTime string 
.INPUT/OUTPUT
  call as    SciaJDAY2adaguc( jday, dateTime );
     input:
            double jday         :  decimal days elapsed since 1.1.2000
    output:
            char *dateTime      :  returned as yyyyMMddThhmmss

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void SciaJDAY2adaguc( double jday, char *dateTime )
{
     unsigned int imin, ihour, iday, imon, iyear;
     double       day, sec;

     jday += jday_01012000;
     Julian_2_MJD( jday, &day, &imon, &iyear );
     iday = (unsigned int) day;
     day  -= iday;
     ihour = (unsigned int) (day *= 24);
     day  -= ihour; 
     imin  = (unsigned int) (day *= 60);
     day  -= imin;
     sec = (day * 60);

     if ( sec >= 59.5 ) {
	  sec = 0.;
	  if ( ++imin == 60 ) {
	       imin = 0;
	       if ( ++ihour == 24 ) {
		    ihour = 0;
		    if ( ++iday > Month_Days[imon-1] ) {
			 iday = 1;
			 if ( ++imon > MONTHinYEAR ) {
			      imon = 1;
			      iyear++;
			 }
		    }
	       }
	  }
     }
/*
 * write date time
 */
     (void) sprintf( dateTime, "%04u%02u%02uT%02u%02u%02.0f",
                     iyear, imon, iday, ihour, imin, sec );
}

/*+++++++++++++++++++++++++ 
.IDENTifer   Adaguc2sciaJDAY
.PURPOSE     Converts dateTime string to julian (2000) decimal day
.INPUT/OUTPUT
  call as    jday = Adaguc2sciaJDAY2( dateTime );
     input:
            char *dateTime   :  dateTime given as yyyyMMddThhmmss

.RETURNS     decimal days elapsed since 1.1.2000 (double)
.COMMENTS    none
-------------------------*/
double Adaguc2sciaJDAY( const char *dateTime )
{
     unsigned int isec, imin, ihour, iday, imon, iyear;
     double       day, jday;

     (void) sscanf( dateTime, "%4u%2u%2uT%2u%2u%2u", 
		    &iyear, &imon, &iday, &ihour, &imin, &isec );

     day = iday + (ihour + ((imin + (isec / 60.)) / 60.)) / 24.;
     MJD_2_Julian( day, imon, iyear, &jday );

     return (jday - jday_01012000);
}

/*+++++++++++++++++++++++++ 
.IDENTifer   GomeJDAY2adaguc
.PURPOSE     Converts julian (1950) decimal day into a dateTime string 
.INPUT/OUTPUT
  call as    GomeJDAY2adaguc( jday, dateTime );
     input:
            double jday         :  decimal days elapsed since 1.1.1950
    output:
            char *dateTime      :  returned as yyyyMMddThhmmss

.RETURNS     nothing
.COMMENTS    none
-------------------------*/
void GomeJDAY2adaguc( double jday, char *dateTime )
{
     unsigned int imin, ihour, iday, imon, iyear;
     double       day, sec;

     jday += jday_01011950;
     Julian_2_MJD( jday, &day, &imon, &iyear );
     iday = (unsigned int) day;
     day  -= iday;
     ihour = (unsigned int) (day *= 24);
     day  -= ihour; 
     imin  = (unsigned int) (day *= 60);
     day  -= imin;
     sec = (day * 60);

     if ( sec >= 59.5 ) {
	  sec = 0.;
	  if ( ++imin == 60 ) {
	       imin = 0;
	       if ( ++ihour == 24 ) {
		    ihour = 0;
		    if ( ++iday > Month_Days[imon-1] ) {
			 iday = 1;
			 if ( ++imon > MONTHinYEAR ) {
			      imon = 1;
			      iyear++;
			 }
		    }
	       }
	  }
     }
/*
 * write date time
 */
     (void) sprintf( dateTime, "%04u%02u%02uT%02u%02u%02.0f",
                     iyear, imon, iday, ihour, imin, sec );
}

/*+++++++++++++++++++++++++ 
.IDENTifer   Adaguc2gomeJDAY
.PURPOSE     Converts dateTime string to julian (1950) decimal day
.INPUT/OUTPUT
  call as    jday = Adaguc2gomeJDAY2( dateTime );
     input:
            char *dateTime   :  dateTime given as yyyyMMddThhmmss

.RETURNS     decimal days elapsed since 1.1.1950 (double)
.COMMENTS    see GOME date specification
-------------------------*/
double Adaguc2gomeJDAY( const char *dateTime )
{
     unsigned int isec, imin, ihour, iday, imon, iyear;
     double       day, jday;

     (void) sscanf( dateTime, "%4u%2u%2uT%2u%2u%2u", 
		    &iyear, &imon, &iday, &ihour, &imin, &isec );

     day = iday + (ihour + ((imin + (isec / 60.)) / 60.)) / 24.;
     MJD_2_Julian( day, imon, iyear, &jday );

     return (jday - jday_01011950);
}
