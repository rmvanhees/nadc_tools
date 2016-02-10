DROP TABLE IF EXISTS tile_fresco;
DROP TABLE IF EXISTS tile_imap_ch4;
DROP TABLE IF EXISTS tile_imap_hdo;
DROP TABLE IF EXISTS tile_imlm_co;
DROP TABLE IF EXISTS tile_imlm_h2o;
DROP TABLE IF EXISTS tile_tosomi;
DROP TABLE IF EXISTS tile_cld_ol;
DROP TABLE IF EXISTS tile_no2_ol;
DROP TABLE IF EXISTS tile_o3_ol;

DROP TABLE IF EXISTS meta_fresco;
DROP TABLE IF EXISTS meta_imap_ch4;
DROP TABLE IF EXISTS meta_imap_hdo;
DROP TABLE IF EXISTS meta_imlm_co;
DROP TABLE IF EXISTS meta_imlm_h2o;
DROP TABLE IF EXISTS meta_tosomi;

--
-- Table structure for table "meta_fresco"
--
CREATE TABLE meta_fresco (
  pk_meta        serial      PRIMARY KEY,
  name           varchar(66) NOT NULL UNIQUE CHECK (name <> ''),
  fk_name_l1b    char(62)    REFERENCES meta__1P(name),
  fileSize       integer     NOT NULL CHECK (fileSize > 0),
  receiveDate    timestamp   NOT NULL,
  creationDate   timestamp   NOT NULL,
  dateTimeStart  timestamp   CHECK (numDataSets > 0 OR dateTimeStart IS NULL),
  muSecStart     integer     CHECK (numDataSets > 0 AND muSecStart >= 0 OR muSecStart IS NULL),
  dateTimeStop   timestamp   CHECK (numDataSets > 0 OR dateTimeStop IS NULL),
  muSecStop      integer     CHECK (numDataSets > 0 AND muSecStop >= 0 OR muSecStop IS NULL),
  softVersion    varchar(8)  ,
  fk_version_l1b varchar(10) ,
  absOrbit       integer     NOT NULL CHECK (absOrbit >= 0),
  numDataSets    smallint    NOT NULL
);
-- CREATE INDEX meta_fresco_absOrbit      ON meta_fresco (absOrbit);
-- CREATE INDEX meta_fresco_dateTimeStart ON meta_fresco (dateTimeStart);

--
-- Table structure for table "meta_imap_ch4"
--
CREATE TABLE meta_imap_ch4 (
  pk_meta          serial      PRIMARY KEY,
  name             varchar(66) NOT NULL UNIQUE CHECK (name <> ''),
  fk_name_l1b      char(62)    REFERENCES meta__1P(name),
  fileSize         integer     NOT NULL CHECK (fileSize > 0),
  receiveDate      timestamp   NOT NULL,
  creationDate     timestamp   NOT NULL,
  dateTimeStart    timestamp   CHECK (numDataSets > 0 OR dateTimeStart IS NULL),
  muSecStart       integer     CHECK (numDataSets > 0 AND muSecStart >= 0 OR muSecStart IS NULL),
  dateTimeStop     timestamp   CHECK (numDataSets > 0 OR dateTimeStop IS NULL),
  muSecStop        integer     CHECK (numDataSets > 0 AND muSecStop >= 0 OR muSecStop IS NULL),
  softVersion      varchar(8)  NOT NULL,
  absOrbit         integer     NOT NULL CHECK (absOrbit >= 0),
  numDataSets      smallint    NOT NULL
);
-- CREATE INDEX meta_imap_ch4_absOrbit      ON meta_imap_ch4 (absOrbit);
-- CREATE INDEX meta_imap_ch4_dateTimeStart ON meta_imap_ch4 (dateTimeStart);
--
-- Table structure for table "meta_imap_hdo"
--
CREATE TABLE meta_imap_hdo (
  pk_meta          serial      PRIMARY KEY,
  name             varchar(66) NOT NULL UNIQUE CHECK (name <> ''),
  fk_name_l1b      char(62)    REFERENCES meta__1P(name),
  fileSize         integer     NOT NULL CHECK (fileSize > 0),
  receiveDate      timestamp   NOT NULL,
  creationDate     timestamp   NOT NULL,
  dateTimeStart    timestamp   CHECK (numDataSets > 0 OR dateTimeStart IS NULL),
  muSecStart       integer     CHECK (numDataSets > 0 AND muSecStart >= 0 OR muSecStart IS NULL),
  dateTimeStop     timestamp   CHECK (numDataSets > 0 OR dateTimeStop IS NULL),
  muSecStop        integer     CHECK (numDataSets > 0 AND muSecStop >= 0 OR muSecStop IS NULL),
  softVersion      varchar(8)  NOT NULL,
  absOrbit         integer     NOT NULL CHECK (absOrbit >= 0),
  numDataSets      smallint    NOT NULL
);
-- CREATE INDEX meta_imap_hdo_absOrbit      ON meta_imap_hdo (absOrbit);
-- CREATE INDEX meta_imap_hdo_dateTimeStart ON meta_imap_hdo (dateTimeStart);

--
-- Table structure for table "meta_imlm_co"
--
CREATE TABLE meta_imlm_co (
  pk_meta          serial      PRIMARY KEY,
  name             varchar(66) NOT NULL UNIQUE CHECK (name <> ''),
  fk_name_l1b      char(62)    REFERENCES meta__1P(name),
  fileSize         integer     NOT NULL CHECK (fileSize > 0),
  receiveDate      timestamp   NOT NULL,
  creationDate     timestamp   NOT NULL,
  dateTimeStart    timestamp   CHECK (numDataSets > 0 OR dateTimeStart IS NULL),
  muSecStart       integer     CHECK (numDataSets > 0 AND muSecStart >= 0 OR muSecStart IS NULL),
  dateTimeStop     timestamp   CHECK (numDataSets > 0 OR dateTimeStop IS NULL),
  muSecStop        integer     CHECK (numDataSets > 0 AND muSecStop >= 0 OR muSecStop IS NULL),
  softVersion      varchar(8)  NOT NULL,
  pixelMaskVersion varchar(16) NOT NULL,
  cloudMaskVersion varchar(16) NOT NULL,
  absOrbit         integer     NOT NULL CHECK (absOrbit >= 0),
  windowPixel      integer[2]  NOT NULL,
  windowWave       real[2]     NOT NULL,
  numDataSets      smallint    NOT NULL
);
-- CREATE INDEX meta_imlm_co_absOrbit      ON meta_imlm_co (absOrbit);
-- CREATE INDEX meta_imlm_co_dateTimeStart ON meta_imlm_co (dateTimeStart);

--
-- Table structure for table "meta_imlm_h2o"
--
CREATE TABLE meta_imlm_h2o (
  pk_meta          serial      PRIMARY KEY,
  name             varchar(66) NOT NULL UNIQUE CHECK (name <> ''),
  fk_name_l1b      char(62)    REFERENCES meta__1P(name),
  fileSize         integer     NOT NULL CHECK (fileSize > 0),
  receiveDate      timestamp   NOT NULL,
  creationDate     timestamp   NOT NULL,
  dateTimeStart    timestamp   CHECK (numDataSets > 0 OR dateTimeStart IS NULL),
  muSecStart       integer     CHECK (numDataSets > 0 AND muSecStart >= 0 OR muSecStart IS NULL),
  dateTimeStop     timestamp   CHECK (numDataSets > 0 OR dateTimeStop IS NULL),
  muSecStop        integer     CHECK (numDataSets > 0 AND muSecStop >= 0 OR muSecStop IS NULL),
  softVersion      varchar(8)  NOT NULL,
  pixelMaskVersion varchar(16) NOT NULL,
  cloudMaskVersion varchar(16) NOT NULL,
  absOrbit         integer     NOT NULL CHECK (absOrbit >= 0),
  windowPixel      integer[2]  NOT NULL,
  windowWave       real[2]     NOT NULL,
  numDataSets      smallint    NOT NULL
);
-- CREATE INDEX meta_imlm_h2o_absOrbit      ON meta_imlm_h2o (absOrbit);
-- CREATE INDEX meta_imlm_h2o_dateTimeStart ON meta_imlm_h2o (dateTimeStart);

--
-- Table structure for table "meta_tosomi"
--
CREATE TABLE meta_tosomi (
  pk_meta        serial      PRIMARY KEY,
  name           varchar(66) NOT NULL UNIQUE CHECK (name <> ''),
  fk_name_l1b    char(62)    REFERENCES meta__1P(name),
  fileSize       integer     NOT NULL CHECK (fileSize > 0),
  receiveDate    timestamp   NOT NULL,
  creationDate   timestamp   NOT NULL,
  dateTimeStart  timestamp   CHECK (numDataSets > 0 OR dateTimeStart IS NULL),
  muSecStart     integer     CHECK (numDataSets > 0 AND muSecStart >= 0 OR muSecStart IS NULL),
  dateTimeStop   timestamp   CHECK (numDataSets > 0 OR dateTimeStop IS NULL),
  muSecStop      integer     CHECK (numDataSets > 0 AND muSecStop >= 0 OR muSecStop IS NULL),
  softVersion    varchar(8)  ,
  fk_version_l1b varchar(10) ,
  absOrbit       integer     NOT NULL CHECK (absOrbit >= 0),
  numDataSets    smallint    NOT NULL
);
-- CREATE INDEX meta_tosomi_absOrbit      ON meta_tosomi (absOrbit);
-- CREATE INDEX meta_tosomi_dateTimeStart ON meta_tosomi (dateTimeStart);

--
-- Table structure for table "tile_fresco"
--
CREATE TABLE tile_fresco (
  pk_tile          bigserial PRIMARY KEY,
  fk_meta          integer   REFERENCES meta_fresco(pk_meta) ON DELETE CASCADE,
  julianDay        double precision NOT NULL UNIQUE,
  integrationTime  smallint  NOT NULL,
  errorFlag        smallint  NOT NULL,
  cloudFraction    real      NOT NULL,
  cloudTopHeight   real      NOT NULL,
  cloudTopPressure real      NOT NULL,
  cloudAlbedo      real      NOT NULL,
  surfaceHeight    real      NOT NULL,
  surfacePressure  real      NOT NULL,
  surfaceAlbedo    real      NOT NULL
);
-- SRID voor WGS84 is 4326
SELECT AddGeometryColumn( 'tile_fresco', 'tile', 4326, 'POLYGON', 2 );
CREATE INDEX tile_fresco_tile ON tile_fresco USING GIST (tile);

--
-- Table structure for table "tile_imap_ch4"
--
CREATE TABLE tile_imap_ch4 (
  pk_tile          bigserial PRIMARY KEY,
  fk_meta          integer  REFERENCES meta_imap_ch4(pk_meta) ON DELETE CASCADE,
  julianDay        double precision NOT NULL UNIQUE,
  integrationTime  smallint  NOT NULL,
  meanElevation    real      NOT NULL,
  VCD_CH4          real      NOT NULL,
  VCD_CH4_ERROR    real      NOT NULL,
  VCD_CO2          real      NOT NULL,
  VCD_CO2_ERROR    real      NOT NULL,
  xVMR_CH4         real      NOT NULL
);
-- SRID voor WGS84 is 4326
SELECT AddGeometryColumn( 'tile_imap_ch4', 'tile', 4326, 'POLYGON', 2 );
CREATE INDEX tile_imap_ch4_tile ON tile_imap_ch4 USING GIST (tile);

--
-- Table structure for table "tile_imap_hd0"
--
CREATE TABLE tile_imap_hdo (
  pk_tile          bigserial PRIMARY KEY,
  fk_meta          integer  REFERENCES meta_imap_hdo(pk_meta) ON DELETE CASCADE,
  julianDay        double precision NOT NULL UNIQUE,
  integrationTime  smallint  NOT NULL,
  meanElevation    real      NOT NULL,
  VCD_HDO          real      NOT NULL,
  VCD_HDO_ERROR    real      NOT NULL,
  VCD_H2O          real      NOT NULL,
  VCD_H2O_ERROR    real      NOT NULL,
  VCD_H2O_MODEL    real      NOT NULL
);
-- SRID voor WGS84 is 4326
SELECT AddGeometryColumn( 'tile_imap_hdo', 'tile', 4326, 'POLYGON', 2 );
CREATE INDEX tile_imap_hdo_tile ON tile_imap_hdo USING GIST (tile);

--
-- Table structure for table "tile_imlm_co"
--
CREATE TABLE tile_imlm_co (
  pk_tile          bigserial PRIMARY KEY,
  fk_meta          integer   REFERENCES meta_imlm_co(pk_meta) ON DELETE CASCADE,
  julianDay        double precision NOT NULL UNIQUE,
  integrationTime  smallint  NOT NULL,
  errorFlag        smallint  NOT NULL,
  meanElevation    real      NOT NULL,
  cloudFraction    real      ,
  surfaceAlbedo    real      ,
  VCD_CO           real      ,
  VCD_CO_ERROR     real      ,
  VCD_CH4          real      ,
  VCD_CH4_ERROR    real      
);
-- SRID voor WGS84 is 4326
SELECT AddGeometryColumn( 'tile_imlm_co', 'tile', 4326, 'POLYGON', 2 );
CREATE INDEX tile_imlm_co_tile ON tile_imlm_co USING GIST (tile);

--
-- Table structure for table "tile_imlm_h2o"
--
CREATE TABLE tile_imlm_h2o (
  pk_tile          bigserial PRIMARY KEY,
  fk_meta          integer  REFERENCES meta_imlm_h2o(pk_meta) ON DELETE CASCADE,
  julianDay        double precision NOT NULL UNIQUE,
  integrationTime  smallint  NOT NULL,
  errorFlag        smallint  NOT NULL,
  meanElevation    real      NOT NULL,
  cloudFraction    real      ,
  surfaceAlbedo    real      ,
  VCD_H2O          real      ,
  VCD_H2O_ERROR    real      ,
  VCD_CH4          real      ,
  VCD_CH4_ERROR    real      
);
-- SRID voor WGS84 is 4326
SELECT AddGeometryColumn( 'tile_imlm_h2o', 'tile', 4326, 'POLYGON', 2 );
CREATE INDEX tile_imlm_h2o_tile ON tile_imlm_h2o USING GIST (tile);

--
-- Table structure for table "tile_tosomi"
--
CREATE TABLE tile_tosomi (
  pk_tile          bigserial PRIMARY KEY,
  fk_meta          integer   REFERENCES meta_tosomi(pk_meta) ON DELETE CASCADE,
  julianDay        double precision NOT NULL UNIQUE,
  integrationTime  smallint  NOT NULL,
  cloudFraction    smallint  NOT NULL,
  cloudTopPress    integer   NOT NULL,
  amf              real      NOT NULL,
  amfCloud         real      NOT NULL,
  ozone            real      NOT NULL,
  ozoneSlant       real      NOT NULL
);
-- SRID voor WGS84 is 4326
SELECT AddGeometryColumn( 'tile_tosomi', 'tile', 4326, 'POLYGON', 2 );
CREATE INDEX tile_tosomi_tile ON tile_tosomi USING GIST (tile);

--
-- Table structure for table "tile_cld_ol"
--
CREATE TABLE tile_cld_ol (
  pk_tile           bigserial PRIMARY KEY,
  fk_meta           integer   REFERENCES meta__2P(pk_meta) ON DELETE CASCADE,
  julianDay         double precision NOT NULL UNIQUE,
  integrationTime   smallint  NOT NULL,
  quality           smallint  NOT NULL,
  cloudFraction     real      NOT NULL,
  cloudTopPress     real      NOT NULL,
  cloudOpticalDepth real      NOT NULL,
  cloudBRDF         real      NOT NULL,
  surfacePress      real      NOT NULL,
  surfaceRefl       real      NOT NULL,
  aerosolIndex      real      NOT NULL
);
-- SRID voor WGS84 is 4326
SELECT AddGeometryColumn( 'tile_cld_ol', 'tile', 4326, 'POLYGON', 2 );
CREATE INDEX tile_cld_ol_tile ON tile_cld_ol USING GIST (tile);

--
-- Table structure for table "tile_no2_ol"
--
CREATE TABLE tile_no2_ol (
  pk_tile           bigserial PRIMARY KEY,
  fk_meta           integer   REFERENCES meta__2P(pk_meta) ON DELETE CASCADE,
  julianDay         double precision NOT NULL UNIQUE,
  integrationTime   smallint  NOT NULL,
  quality           smallint  NOT NULL,
  verticalColumn    real      NOT NULL,
  slantColumn       real      NOT NULL,
  amfGround         real      NOT NULL,
  amfCloud          real      NOT NULL
);
-- SRID voor WGS84 is 4326
SELECT AddGeometryColumn( 'tile_no2_ol', 'tile', 4326, 'POLYGON', 2 );
CREATE INDEX tile_no2_ol_tile ON tile_no2_ol USING GIST (tile);

--
-- Table structure for table "tile_o3_ol"
--
CREATE TABLE tile_o3_ol (
  pk_tile           bigserial PRIMARY KEY,
  fk_meta           integer   REFERENCES meta__2P(pk_meta) ON DELETE CASCADE,
  julianDay         double precision NOT NULL UNIQUE,
  integrationTime   smallint  NOT NULL,
  quality           smallint  NOT NULL,
  verticalColumn    real   NOT NULL,
  slantColumn       real   NOT NULL,
  amfGround         real   NOT NULL,
  amfCloud          real   NOT NULL
);
-- SRID voor WGS84 is 4326
SELECT AddGeometryColumn( 'tile_o3_ol', 'tile', 4326, 'POLYGON', 2 );
CREATE INDEX tile_o3_ol_tile ON tile_o3_ol USING GIST (tile);

GRANT USAGE ON SCHEMA nadc_admin to nadc_user;
GRANT SELECT ON auxiliary, auxiliary_pk_id_seq, 
      meta_fresco, meta_fresco_pk_meta_seq, 
      meta_imap_ch4, meta_imap_ch4_pk_meta_seq, 
      meta_imap_hdo, meta_imap_hdo_pk_meta_seq,
      meta_imlm_co, meta_imlm_co_pk_meta_seq, 
      meta_imlm_h2o, meta_imlm_h2o_pk_meta_seq, 
      meta_tosomi, meta_tosomi_pk_meta_seq, 
      tile_cld_ol, tile_cld_ol_pk_tile_seq,
      tile_fresco, tile_fresco_pk_tile_seq,
      tile_imap_ch4, tile_imap_ch4_pk_tile_seq, 
      tile_imap_hdo, tile_imap_hdo_pk_tile_seq, 
      tile_imlm_co, tile_imlm_co_pk_tile_seq, 
      tile_imlm_h2o, tile_imlm_h2o_pk_tile_seq,  
      tile_no2_ol, tile_no2_ol_pk_tile_seq,
      tile_o3_ol, tile_o3_ol_pk_tile_seq,
      tile_tosomi, tile_tosomi_pk_tile_seq TO nadc_user;
