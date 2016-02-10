DROP TABLE IF EXISTS tileinfo_meta__1P;
DROP TABLE IF EXISTS tileinfo_meta__2P;

DROP TABLE IF EXISTS tileinfo;

DROP TABLE IF EXISTS meta__1P;
DROP TABLE IF EXISTS meta__2P;

--
-- Table structure for table "meta__1P"
--
CREATE TABLE meta__1P (
  pk_meta        serial      PRIMARY KEY,
  name           varchar(67) NOT NULL CHECK (name <> ''),
  fileSize       integer     NOT NULL CHECK (fileSize > 0),
  receiveDate    timestamp   NOT NULL,
  procCenter     char(2)     NOT NULL,
  procDate       timestamp   NOT NULL,
  softVersion    char(5)     NOT NULL,
  keydataVersion char(5)     NOT NULL,
  dateTimeStart  timestamp   NOT NULL,
  muSecStart     integer     NOT NULL CHECK (muSecStart >= 0),
  dateTimeStop   timestamp   NOT NULL,
  muSecStop      integer     NOT NULL CHECK (muSecStop >= 0),
  absOrbit       integer     NOT NULL CHECK (absOrbit >= 0),
  numEarthMDS    smallint    NOT NULL,
  numSunMDS      smallint    NOT NULL,
  numMoonMDS     smallint    NOT NULL,
  CONSTRAINT meta__1P_name UNIQUE(name, softVersion)
);
-- CREATE INDEX meta__1P_absOrbit      ON meta__1P (absOrbit);
-- CREATE INDEX meta__1P_dateTimeStart ON meta__1P (dateTimeStart);

--
-- Table structure for table "meta__2P"
--
CREATE TABLE meta__2P (
  pk_meta        serial      PRIMARY KEY,
  name           varchar(56) NOT NULL CHECK (name <> ''),
  fileSize       integer     NOT NULL CHECK (fileSize > 0),
  receiveDate    timestamp   NOT NULL,
  procCenter     char(2)     NOT NULL,
  procDate       timestamp   NOT NULL,
  softVersion    char(5)     NOT NULL,
  keydataVersion char(5)     NOT NULL,
  formatVersion  char(5)     NOT NULL,
  dateTimeStart  timestamp   NOT NULL,
  muSecStart     integer     NOT NULL CHECK (muSecStart >= 0),
  dateTimeStop   timestamp   NOT NULL,
  muSecStop      integer     NOT NULL CHECK (muSecStop >= 0),
  absOrbit       integer     NOT NULL CHECK (absOrbit >= 0),
  numDataSets    smallint    NOT NULL,
  CONSTRAINT meta__2P_name UNIQUE(name, softVersion)
);
-- CREATE INDEX meta__2P_absOrbit      ON meta__2P (absOrbit);
-- CREATE INDEX meta__2P_dateTimeStart ON meta__2P (dateTimeStart);

--
-- Table structure for table "tileinfo"
--
CREATE TABLE tileinfo (
  pk_tileinfo     serial           PRIMARY KEY,
  julianDay       double precision NOT NULL UNIQUE,
  release         integer[2]       DEFAULT '{-1,-1}',
  pixelNumber     smallint         ,
  subSetCounter   smallint         NOT NULL,
  swathType       smallint         ,
  satZenithAngle  real             ,
  sunZenithAngle  real             ,
  relAzimuthAngle real             
);
-- SRID voor WGS84 is 4326
SELECT AddGeometryColumn( 'tileinfo', 'tile', 4326, 'POLYGON', 2 );
CREATE INDEX tileinfo_tile ON tileinfo USING GIST (tile);

--
-- Table structure for table "tileinfo_meta__1P"
--
CREATE TABLE tileinfo_meta__1P (
  fk_tileinfo integer     REFERENCES tileinfo(pk_tileinfo) ON DELETE CASCADE,
  fk_meta     integer     REFERENCES meta__1P(pk_meta) ON DELETE CASCADE,
  PRIMARY KEY (fk_tileinfo, fk_meta)
);
-- CREATE INDEX tileinfo_meta__1P_meta ON tileinfo_meta__1P (fk_meta);

--
-- Table structure for table "tileinfo_meta__2P"
--
CREATE TABLE tileinfo_meta__2P (
  fk_tileinfo integer     REFERENCES tileinfo(pk_tileinfo) ON DELETE CASCADE,
  fk_meta     integer     REFERENCES meta__2P(pk_meta) ON DELETE CASCADE,
  PRIMARY KEY (fk_tileinfo, fk_meta)
);
-- CREATE INDEX tileinfo_meta__2P_meta ON tileinfo_meta__2P (fk_meta);

GRANT USAGE ON SCHEMA nadc_admin TO nadc_user;
GRANT SELECT ON
      meta__1p, meta__1p_pk_meta_seq,
      meta__2p, meta__2p_pk_meta_seq,
      meta_fresco, meta_fresco_pk_meta_seq,
      meta_togomi, meta_togomi_pk_meta_seq,
      tileinfo, tileinfo_pk_tileinfo_seq,
      tileinfo_meta__1p, tileinfo_meta__2p TO nadc_user;
