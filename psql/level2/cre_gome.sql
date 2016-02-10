DROP TABLE IF EXISTS tileinfo_meta__1P;
DROP TABLE IF EXISTS tileinfo_meta__2P;
DROP TABLE IF EXISTS tile_meta_fresco;
DROP TABLE IF EXISTS tile_meta_togomi;

DROP TABLE IF EXISTS tile_fresco;
DROP TABLE IF EXISTS tile_togomi;
DROP TABLE IF EXISTS tile_pmd;
DROP TABLE IF EXISTS tile_gdp;

DROP TABLE IF EXISTS meta_fresco;
DROP TABLE IF EXISTS meta_togomi;

--
-- Table structure for table "meta_fresco"
--
CREATE TABLE meta_fresco (
  pk_meta        serial      PRIMARY KEY,
  name           char(26)    NOT NULL UNIQUE CHECK (name <> ''),
  fk_name_l1b    varchar(67) NOT NULL CHECK (fk_name_l1b <> ''),
  fileSize       integer     NOT NULL CHECK (fileSize > 0),
  receiveDate    timestamp   NOT NULL,
  creationDate   timestamp   NOT NULL,
  dateTimeStart  timestamp   CHECK (numDataSets > 0 AND dateTimeStart IS NOT NULL),
  muSecStart     integer     CHECK (numDataSets > 0 AND muSecStart >= 0 OR muSecStart IS NULL),
  dateTimeStop   timestamp   CHECK (numDataSets > 0 AND dateTimeStop IS NOT NULL),
  muSecStop      integer     CHECK (numDataSets > 0 AND muSecStop >= 0 OR muSecStop IS NULL),
  softVersion    varchar(8)  ,
  fk_version_l1b varchar(10) ,
  absOrbit       integer     NOT NULL CHECK (absOrbit >= 0),
  numDataSets    smallint    NOT NULL,
  FOREIGN KEY (fk_name_l1b,fk_version_l1b) REFERENCES meta__1P (name, softVersion)
);
-- CREATE INDEX meta_fresco_absOrbit      ON meta_fresco (absOrbit);
-- CREATE INDEX meta_fresco_dateTimeStart ON meta_fresco (dateTimeStart);

--
-- Table structure for table "meta_togomi"
--
CREATE TABLE meta_togomi (
  pk_meta        serial      PRIMARY KEY,
  name           char(26)    NOT NULL UNIQUE CHECK (name <> ''),
  fk_name_l1b    varchar(67) NOT NULL CHECK (fk_name_l1b <> ''),
  fileSize       integer     NOT NULL CHECK (fileSize > 0),
  receiveDate    timestamp   NOT NULL,
  creationDate   timestamp   NOT NULL,
  dateTimeStart  timestamp   CHECK (numDataSets > 0 AND dateTimeStart IS NOT NULL),
  muSecStart     integer     CHECK (numDataSets > 0 AND muSecStart >= 0 OR muSecStart IS NULL),
  dateTimeStop   timestamp   CHECK (numDataSets > 0 AND dateTimeStop IS NOT NULL),
  muSecStop      integer     CHECK (numDataSets > 0 AND muSecStop >= 0 OR muSecStop IS NULL),
  softVersion    varchar(8)  ,
  fk_version_l1b varchar(10) ,
  absOrbit       integer     NOT NULL CHECK (absOrbit >= 0),
  numDataSets    smallint    NOT NULL,
  FOREIGN KEY (fk_name_l1b,fk_version_l1b) REFERENCES meta__1P (name, softVersion)
);
-- CREATE INDEX meta_togomi_absOrbit      ON meta_togomi (absOrbit);
-- CREATE INDEX meta_togomi_dateTimeStart ON meta_togomi (dateTimeStart);

--
-- Table structure for table "tile_pmd"
--
CREATE TABLE tile_pmd (
  julianDay    double precision PRIMARY KEY,
  fk_tileinfo  integer     REFERENCES tileinfo(pk_tileinfo) ON DELETE CASCADE ON UPDATE CASCADE,
  pmd_1        real[16]    NOT NULL,
  pmd_2        real[16]    NOT NULL,
  pmd_3        real[16]    NOT NULL
);
CREATE INDEX tile_pmd_tileinfo  ON tile_pmd (fk_tileinfo);

--
-- Table structure for table "tile_gdp"
--
CREATE TABLE tile_gdp (
  julianDay     double precision PRIMARY KEY,
  fk_tileinfo   integer   REFERENCES tileinfo(pk_tileinfo) ON DELETE CASCADE ON UPDATE CASCADE,
  o3_gdp        real      NOT NULL,
  o3_gdp_err    real      NOT NULL,
  cloudFraction real      NOT NULL,
  cloudTopPress real      NOT NULL,
  surfacePress  real      NOT NULL
);
CREATE INDEX tile_gdp_tileinfo  ON tile_gdp (fk_tileinfo);

--
-- Table structure for table "tile_fresco"
--
CREATE TABLE tile_fresco (
  julianDay        double precision PRIMARY KEY,
  fk_tileinfo      integer REFERENCES tileinfo(pk_tileinfo) ON DELETE CASCADE ON UPDATE CASCADE,
  integrationTime  smallint,
  errorFlag        smallint,
  cloudFraction    real    ,
  cloudTopHeight   real    ,
  cloudTopPressure real    ,
  cloudAlbedo      real    ,
  surfaceHeight    real    ,
  surfacePressure  real    ,
  surfaceAlbedo    real    
);

--
-- Table structure for table "tile_meta_fresco"
--
CREATE TABLE tile_meta_fresco (
  fk_meta   integer          REFERENCES meta_fresco(pk_meta) ON DELETE CASCADE,
  julianDay double precision REFERENCES tile_fresco(julianDay) ON DELETE CASCADE,
  PRIMARY KEY (fk_meta, julianDay)
);

--
-- Table structure for table "tile_togomi"
--
CREATE TABLE tile_togomi (
  julianDay        double precision PRIMARY KEY,
  fk_tileinfo      integer REFERENCES tileinfo(pk_tileinfo) ON DELETE CASCADE ON UPDATE CASCADE,
  integrationTime  smallint NOT NULL,
  cloudFraction    real     NOT NULL,
  cloudTopHeight   real     NOT NULL,
  amf              real     NOT NULL,
  amfCloud         real     NOT NULL,
  ozone            real     NOT NULL,
  ozoneSlant       real     NOT NULL
);

--
-- Table structure for table "tile_meta_togomi"
--
CREATE TABLE tile_meta_togomi (
  fk_meta   integer          REFERENCES meta_togomi(pk_meta) ON DELETE CASCADE,
  julianDay double precision REFERENCES tile_togomi(julianDay) ON DELETE CASCADE,
  PRIMARY KEY (fk_meta, julianDay)
);

GRANT USAGE ON SCHEMA nadc_admin TO nadc_user;
GRANT SELECT ON
      meta_fresco, meta_fresco_pk_meta_seq,
      meta_togomi, meta_togomi_pk_meta_seq,
      tile_gdp, tile_pmd, 
      tile_fresco, tile_meta_fresco,
      tile_togomi, tile_meta_togomi TO nadc_user;
