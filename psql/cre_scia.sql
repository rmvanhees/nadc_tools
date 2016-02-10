DROP TABLE IF EXISTS auxiliary;

DROP TABLE IF EXISTS stateinfo_meta__2P;
DROP TABLE IF EXISTS stateinfo_meta__1P;
DROP TABLE IF EXISTS stateinfo_meta__0P;

DROP TABLE IF EXISTS meta__2P;
DROP TABLE IF EXISTS meta__1P;
DROP TABLE IF EXISTS meta__0P;

--
-- Table structure for table meta__0P
--
CREATE TABLE meta__0P (
  pk_meta        serial      PRIMARY KEY,
  name           char(62)    NOT NULL UNIQUE CHECK (name <> ''),
  fileSize       integer     NOT NULL CHECK (fileSize > 0),
  receiveDate    timestamp   NOT NULL,
  procStage      proc_stage  NOT NULL,
  procCenter     char(6)     NOT NULL,
  softVersion    varchar(14) NOT NULL,
  dateTimeStart  timestamp   NOT NULL,
  muSecStart     integer     NOT NULL CHECK (muSecStart >= 0),
  dateTimeStop   timestamp   NOT NULL,
  muSecStop      integer     NOT NULL CHECK (muSecStop >= 0),
  absOrbit       integer     NOT NULL CHECK (absOrbit >= 0),
  relOrbit       smallint    NOT NULL,
  numDataSets    smallint    NOT NULL,
  nadirStates    smallint    NOT NULL,
  limbStates     smallint    NOT NULL,
  occultStates   smallint    NOT NULL,
  monitorStates  smallint    NOT NULL,
  noEntryDMOP    smallint    DEFAULT 0,
  delayedBy      real        
);
-- CREATE INDEX meta__0P_absOrbit      ON meta__0P (absOrbit);
-- CREATE INDEX meta__0P_dateTimeStart ON meta__0P (dateTimeStart);

--
-- Table structure for table meta__1P
--
CREATE TABLE meta__1P (
  pk_meta          serial      PRIMARY KEY,
  name             char(62)    NOT NULL UNIQUE CHECK (name <> ''),
  fileSize         integer     NOT NULL CHECK (fileSize > 0),
  receiveDate      timestamp   NOT NULL,
  procStage        proc_stage  NOT NULL,
  procCenter       char(6)     NOT NULL,
  softVersion      varchar(14) NOT NULL,
  keydataVersion   varchar(8)  NOT NULL,
  mFactorVersion   varchar(8)  NOT NULL,
  spectralCal      quality     NOT NULL,
  saturatedPix     quality     NOT NULL,
  deadPixels       quality     NOT NULL,
  dateTimeStart    timestamp   NOT NULL,
  muSecStart       integer     NOT NULL CHECK (muSecStart >= 0),
  dateTimeStop     timestamp   NOT NULL,
  muSecStop        integer     NOT NULL CHECK (muSecStop >= 0),
  absOrbit         integer     NOT NULL CHECK (absOrbit >= 0),
  relOrbit         smallint    NOT NULL,
  numDataSets      smallint    NOT NULL,
  leakageFileID    integer    ,  
  ppgEtalonFileID  integer    ,  
  spectralFileID   integer    ,  
  sunRefFileID     integer    ,  
  keyDataFileID    integer    ,  
  mFactorFileID    integer    ,  
  initFileID       integer    ,  
  nadirStates      smallint    NOT NULL,
  limbStates       smallint    NOT NULL,
  occultStates     smallint    NOT NULL,
  monitorStates    smallint    NOT NULL,
  noProcStates     smallint    NOT NULL,
  noEntryDMOP      smallint    DEFAULT 0,
  delayedBy        real        
);
-- CREATE INDEX meta__1P_absOrbit      ON meta__1P (absOrbit);
-- CREATE INDEX meta__1P_dateTimeStart ON meta__1P (dateTimeStart);

--
-- Table structure for table meta__2P
--
CREATE TABLE meta__2P (
  pk_meta        serial      PRIMARY KEY,
  name           char(62)    NOT NULL UNIQUE CHECK (name <> ''),
  fk_name_l1b    char(62)    REFERENCES meta__1P(name),
  fileSize       integer     NOT NULL CHECK (fileSize > 0),
  receiveDate    timestamp   NOT NULL,
  procStage      proc_stage  NOT NULL,
  procCenter     char(6)     NOT NULL,
  softVersion    varchar(14) NOT NULL,
  fittingErrSum  quality     NOT NULL,
  dateTimeStart  timestamp   NOT NULL,
  muSecStart     integer     NOT NULL CHECK (muSecStart >= 0),
  dateTimeStop   timestamp   NOT NULL,
  muSecStop      integer     NOT NULL CHECK (muSecStop >= 0),
  absOrbit       integer     NOT NULL CHECK (absOrbit >= 0),
  relOrbit       smallint    NOT NULL,
  numDataSets    smallint    NOT NULL,
  nadirFittingWindows  varchar(192) NOT NULL,
  limbFittingWindows   varchar(192) NOT NULL,
  occultFittingWindows varchar(192) NOT NULL,
  numNadirFW     smallint    NOT NULL,
  numLimbFW      smallint    NOT NULL,
  numOccultFW    smallint    NOT NULL,
  noEntryDMOP    smallint    DEFAULT 0,
  delayedBy      real        
);
-- CREATE INDEX meta__2P_absOrbit      ON meta__2P (absOrbit);
-- CREATE INDEX meta__2P_dateTimeStart ON meta__2P (dateTimeStart);

--
-- Table structure for table "stateinfo_meta__0P"
--
CREATE TABLE stateinfo_meta__0P (
  fk_meta      integer UNIQUE REFERENCES meta__0P(pk_meta) ON DELETE CASCADE,
  fk_stateinfo integer[] NOT NULL,
  procStage    proc_stage NOT NULL
);

--
-- Table structure for table "stateinfo_meta__1P"
--
CREATE TABLE stateinfo_meta__1P (
  fk_meta      integer UNIQUE REFERENCES meta__1P(pk_meta) ON DELETE CASCADE,
  fk_stateinfo integer[] NOT NULL,
  procStage    proc_stage NOT NULL
);

--
-- Table structure for table "stateinfo_meta__2P"
--
CREATE TABLE stateinfo_meta__2P (
  fk_meta      integer UNIQUE REFERENCES meta__2P(pk_meta) ON DELETE CASCADE,
  fk_stateinfo integer[] NOT NULL,
  procStage    proc_stage NOT NULL
);

--
-- Table structure for table "auxiliary"
--
CREATE TABLE auxiliary (
  pk_id        serial    PRIMARY KEY,
  fileName     char(62)  NOT NULL UNIQUE
);

GRANT USAGE ON SCHEMA nadc_admin to nadc_user;
GRANT SELECT ON auxiliary, auxiliary_pk_id_seq, 
      meta__0p, meta__0p_pk_meta_seq,
      meta__1p, meta__1p_pk_meta_seq,
      meta__2p, meta__2p_pk_meta_seq,
      stateinfo, stateinfo_pk_stateinfo_seq,
      stateinfo_meta__0p, stateinfo_meta__1p, stateinfo_meta__2p TO nadc_user;
