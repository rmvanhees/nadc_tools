--
-- Note: GOME version (MSEC are mili-seconds)
-- September, 2009, r.m. van hees
-- Original (SQL version) August, 2007, sdecerff
-- Julian day in Postgresql counts from Januari, 4712 BC, 
--  number of days since 4712 BC = 1721022 
-- 38 more needed to compensate the definition...
-- 712223 to compensate the start in 1950
--
CREATE OR REPLACE 
FUNCTION DATE2JDAY(TIMESTAMP without time zone, integer) 
	 RETURNS double precision AS $$
DECLARE
	jday_offs CONSTANT integer := 1721022 + 38 + 712223;
	jday      CONSTANT double precision := to_char($1,'J');
	msec      CONSTANT double precision := $2 / 1e3;
BEGIN
	RETURN (jday - jday_offs
		+ (extract(HOUR from $1) / 24.)
		+ (extract(MINUTE from $1) / 1440.)
		+ ((extract(SECOND from $1) + msec)/ 86400.));
END;
$$ LANGUAGE plpgsql IMMUTABLE RETURNS NULL ON NULL INPUT;


CREATE OR REPLACE 
FUNCTION JDAY2DATE( double precision ) 
	 RETURNS char AS $$
DECLARE
	jday_offs CONSTANT integer := 1721022 + 38 + 712223;
	secnd_str char(6);	
	date_str  char(10);
	jdate     timestamp;
BEGIN
	jdate = to_timestamp((trunc($1) + jday_offs)::text, 'J');
	date_str  = to_char( jdate, 'YYYY-MM-DD');
	secnd_str = to_char( (($1 - trunc($1)) * 86400)::real, '99999');
	jdate = to_timestamp(date_str || secnd_str, 'YYYY-MM-DD SSSS');

	RETURN to_char(jdate, 'YYYY-MM-DD HH24:MI:SS') ;
END;
$$ LANGUAGE plpgsql IMMUTABLE RETURNS NULL ON NULL INPUT;

CREATE OR REPLACE 
FUNCTION JDAY2MSEC( double precision ) 
	 RETURNS integer AS $$
DECLARE
	dsec CONSTANT double precision := ($1 - trunc($1)) * 86400;
BEGIN
	RETURN round((dsec - trunc(dsec)) * 1e3);
END;
$$ LANGUAGE plpgsql IMMUTABLE RETURNS NULL ON NULL INPUT;

GRANT EXECUTE ON FUNCTION DATE2JDAY(TIMESTAMP without time zone, integer)
      TO nadc_user;
GRANT EXECUTE ON FUNCTION JDAY2DATE( double precision ) TO nadc_user;
GRANT EXECUTE ON FUNCTION JDAY2MSEC( double precision ) TO nadc_user;
