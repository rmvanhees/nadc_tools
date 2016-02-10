--
-- Table structure for table `stateinfo`
--
DROP TABLE IF EXISTS stateinfo;

DROP TYPE proc_stage;
CREATE TYPE proc_stage AS 
	ENUM ('B', 'S', 'T', 'N', 'O', 'P', 'R', 'U', 'W', 'Y', 'Z');
DROP TYPE quality;
CREATE TYPE quality AS ENUM ('BAD', 'FAIR', 'GOOD');

CREATE TABLE stateinfo (
  pk_stateinfo  serial     PRIMARY KEY,
  dateTimeStart timestamp  NOT NULL,
  muSecStart    integer    NOT NULL CHECK (muSecStart >= 0),
  dateTimeStop  timestamp  NOT NULL,
  muSecStop     integer    NOT NULL CHECK (muSecStop >= 0),
  timeLine      char(8)    NOT NULL,
  stateID       smallint   NOT NULL CHECK (stateID > 0 AND stateID <= 70),
  absOrbit      integer    NOT NULL CHECK (absOrbit > 0),
  orbitPhase    real       CHECK (orbitPhase >= 0 AND orbitPhase <= 1),
  softVersion   char(3)    default '000',
  saaFlag       boolean    ,
  obmTemp       real       default -999,
  detTemp       real[8]    default '{-999,-999,-999,-999,-999,-999,-999,-999}',
  pmdTemp       real       default -999,
  CONSTRAINT stateDateStart UNIQUE (dateTimeStart, muSecStart)
);
CREATE INDEX stateinfo_absOrbit ON stateinfo (absOrbit);
CREATE INDEX stateinfo_stateID  ON stateinfo (stateID);
CREATE INDEX stateinfo_dateTime ON stateinfo (dateTimeStart);

-- SRID voor WGS84 is 4326
SELECT AddGeometryColumn( 'stateinfo', 'tile', 4326, 'POLYGON', 2 );
CREATE INDEX stateinfo_tile ON stateinfo USING GIST (tile);
