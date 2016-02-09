#ifndef  __DEFS_SCIA_SDMF                      /* Avoid redefinitions */
#define  __DEFS_SCIA_SDMF

#ifdef __cplusplus
extern "C" {
#endif

#include <nadc_scia_cal.h>

enum sdmf24_db {
     SDMF24_STATE, SDMF24_FITTED, SDMF24_ORBITAL,
     SDMF24_BDPM, SDMF24_PPG, SDMF24_WLSTRANS, SDMF24_TRANS
};

/* length of entryDate string YYYY:MM:DDThh:mm:ss */
#define STR_SZ_DATE       20

/* maximum number of states in a level 0 product */
#define MAX_STATES_IN_FILE   256       

/* structure definitions (all databases) */
#define DIM_FTBL   4
struct ftbl_rec 
{
     int    absOrbit;
     char   prodName[ENVI_FILENAME_SIZE];
     char   entryDate[STR_SZ_DATE];
     char   stateList[MAX_NUM_STATE];
     char   dummy[3];
};
/*@unused@*/ 
static const size_t ftbl_sizes[DIM_FTBL] = {
     sizeof(int), ENVI_FILENAME_SIZE, STR_SZ_DATE, MAX_NUM_STATE
};

#define DIM_FTBL2   5
struct ftbl2_rec 
{
     char            prodName[ENVI_FILENAME_SIZE];
     char            dummy;
     char            entryDate[STR_SZ_DATE];
     char            stateList[MAX_NUM_STATE];
     unsigned short  absOrbit;
};
/*@unused@*/ 
static const size_t ftbl2_sizes[DIM_FTBL2] = {
     ENVI_FILENAME_SIZE, sizeof(char), STR_SZ_DATE, MAX_NUM_STATE, sizeof(short)
};

/* structure definition of SDMF v2.4 MonitorList */
struct monitor_rec {
     int  Orbit;
     int  MagicNumber;
     int  StateCount[16];
     int  QualityNumber;
     int  QualitySmoothMask;
     int  Consolidated;
     int  Transmission;
     int  WLSTransmission;
     int  PixelGain;
     int  Orbital;
     int  OrbitalData;
     int  OrbitalFit;
     int  SMR;
     char FileName[70];
};

/* structure definition of histograms (SDMF v3.0)*/
#define DIM_SDMF_HIST1    5
#define MAX_NUM_HIST1     120
struct sdmf_hist1_rec 
{
     unsigned int   offset;
     unsigned char  binsize;
     unsigned char  coaddf;
     unsigned short location[MAX_NUM_HIST1];
     unsigned short count[MAX_NUM_HIST1];
};
/*@unused@*/ 
static const size_t sdmf_hist1_sizes[DIM_SDMF_HIST1] = {
     sizeof(int), sizeof(char), sizeof(char), 
     MAX_NUM_HIST1 * sizeof(short), MAX_NUM_HIST1 * sizeof(short)
};

/* structure definition of histograms (SDMF v3.1)*/
#define DIM_SDMF_HIST2   6
#define MAX_NUM_HIST2    80
struct sdmf_hist2_rec 
{
     unsigned int   offset;
     unsigned char  binsize;
     unsigned char  coaddf;
     unsigned short count[MAX_NUM_HIST2];
     unsigned short location[MAX_NUM_HIST2];
     unsigned short remainder[MAX_NUM_HIST2];
};
/*@unused@*/ 
static const size_t sdmf_hist2_sizes[DIM_SDMF_HIST2] = {
     sizeof(int), sizeof(char), sizeof(char), 
     MAX_NUM_HIST2 * sizeof(short), 
     MAX_NUM_HIST2 * sizeof(short), 
     MAX_NUM_HIST2 * sizeof(short)
};

/* structure calibration-state database (SDMF v3.0) */
#define DIM_MTBL_CALIB   20
struct mtbl_calib_rec 
{
     double julianDay;                        /* taken from MDS_DET */
     int    duration;                         /* 1/16 sec from MDS_DET */
     int    absOrbit;                         /* taken from MPH */
     char   entryDate[STR_SZ_DATE];
     char   procStage[2];                     /* taken from MPH */
     char   softVersion[15];                  /* taken from MPH */
     unsigned char saaFlag;                   /* taken from ROE info */
     unsigned char rtsEnhFlag;                /* bit 0: ch 6+, bit 1: ch 8*/
     unsigned char vorporFlag;                /* 000000GV G: geo/pnt presence, V: VOR presence */
     unsigned short crc_state;                /* CRC errors in state */
     unsigned short solomon_state;            /* reed-solomon errors in state */
     float  orbitPhase;                       /* taken from ROE info*/
     float  sunSemiDiam;                      /* taken from DORIS */
     float  moonAreaSunlit;                   /* taken from DORIS */
     float  longitude;                        /* taken from DORIS */
     float  latitude;                         /* taken from DORIS */
     float  asmAngle;                         /* taken from MDS_AUX */
     float  esmAngle;                         /* taken from MDS_AUX */
     float  obmTemp;                          /* taken from MDS_AUX */
     float  detectorTemp[SCIENCE_CHANNELS];   /* taken from MDS_DET */
};
/*@unused@*/ 
static const size_t mtbl_calib_sizes[DIM_MTBL_CALIB] = {
     sizeof(double), sizeof(int), sizeof(int), 
     STR_SZ_DATE * sizeof(char), 2 * sizeof(char), 15 * sizeof(char), 
     sizeof(char), sizeof(char), sizeof(char), 
     sizeof(float), sizeof(float), sizeof(float), sizeof(float), 
     sizeof(float), sizeof(float), sizeof(float), sizeof(float),
     SCIENCE_CHANNELS * sizeof(float)
};

/* structure calibration-state database (SDMF v3.1) */
#define DIM_MTBL_CALIB2   19
struct mtbl_calib2_rec 
{
     double julianDay;                        /* taken from MDS_DET */
     char   entryDate[STR_SZ_DATE];
     char   softVersion[15];                  /* taken from MPH */
     char   procStage;                        /* taken from MPH */
     unsigned char vorporFlag;                /* quality Doris prediction */
     bool   saaFlag;                          /* taken from ROE info */
     unsigned short crc_errs;                 /* CRC errors in state */
     unsigned short solomon_errs;             /* reed-solomon errors in state */
     int    absOrbit;                         /* taken from MPH */
     int    duration;                         /* 1/16 sec from MDS_DET */
     float  orbitPhase;                       /* taken from ROE info*/
     float  sunSemiDiam;                      /* taken from DORIS */
     float  moonAreaSunlit;                   /* taken from DORIS */
     float  longitude;                        /* taken from DORIS */
     float  latitude;                         /* taken from DORIS */
     float  asmAngle;                         /* taken from MDS_AUX */
     float  esmAngle;                         /* taken from MDS_AUX */
     float  obmTemp;                          /* taken from MDS_AUX */
     float  detectorTemp[SCIENCE_CHANNELS];   /* taken from MDS_DET */
};
/*@unused@*/ 
static const size_t mtbl_calib2_sizes[DIM_MTBL_CALIB2] = {
     sizeof(double), STR_SZ_DATE * sizeof(char), 15 * sizeof(char), 
     sizeof(char), sizeof(char), sizeof(bool), 
     sizeof(short), sizeof(short), sizeof(int), sizeof(int), 
     sizeof(float), sizeof(float), sizeof(float), sizeof(float), 
     sizeof(float), sizeof(float), sizeof(float), sizeof(float),
     SCIENCE_CHANNELS * sizeof(float)
};

/* structure definitions (Sun/Last-Limb databases) */
struct mtbl_pt_rec {
     double julianDay;                      /* + taken from MDS_DET */
     int    duration;                       /* + 1/16 sec from MDS_DET */
     int    absOrbit;                       /* + taken from MPH */
     char   entryDate[STR_SZ_DATE];         /* + */
     char   procStage;                      /* + taken from MPH */
     bool   saaFlag;                        /* + taken from DORIS / AUX_FRA */
     bool   porFlag;                        /* TRUE when predicted is used */
     char   dummy;
     float  orbitPhase;                     /* + taken from ROE info */
     float  asmAngle;                       /* + taken from DORIS / AUX_FRA */
     float  esmAngle;                       /* + taken from DORIS / AUX_FRA */
     float  sunAzim;                        /* + taken from DORIS / AUX_FRA */
     float  sunElev;                        /* + taken from DORIS / AUX_FRA */
     float  longitude;                      /* + taken from DORIS / AUX_FRA */
     float  latitude;                       /* + taken from DORIS / AUX_FRA */
     float  obmTemp;                        /* + taken from MDS_AUX */
     float  detTemp[SCIENCE_CHANNELS];      /* + taken from MDS_AUX */
};

struct geo_pt_rec {
     double julianDay;
     float  asmAngle;
     float  esmAngle;
     float  sunAzim;
     float  sunElev;
};

/* structure State-Dark database (SDMF v3.0) */
#define DIM_MTBL_DARK   8
struct mtbl_dark_rec 
{
     double julianDay;
     int    absOrbit;
     char   entryDate[STR_SZ_DATE];
     bool   saaFlag;
     short  quality;
     float  orbitPhase;
     float  obmTemp;
     float  detectorTemp[SCIENCE_CHANNELS];
};
/*@unused@*/ 
static const size_t mtbl_dark_sizes[DIM_MTBL_DARK] = {
     sizeof(double), sizeof(int), STR_SZ_DATE * sizeof(char), 
     sizeof(bool), sizeof(short), sizeof(float), sizeof(float), 
     SCIENCE_CHANNELS * sizeof(float)
};

/* structure definitions Dark database (SDMF 3.1) */
#define DIM_MTBL_DARK2    9
struct mtbl_dark2_rec 
{
     double julianDay;                     /* start of orbit ROE file (SOST) */
     double entryDate;
     unsigned short absOrbit;                 
     unsigned short quality;    /* 0: Failed, 1: Local, 2: Degrad, 4: Decont,
				   8: GOOD */
     unsigned short stateCount; /* State: always 0 */
                                /* Fit: number of unique states used for fit */
     unsigned short statesSelected;             /* number of states selected */
     unsigned short orbitRange[SCIENCE_CHANNELS][2];
     float  obmTemp;                                /* from Extract database */
     float  detTemp[SCIENCE_CHANNELS];              /* from Extract database */
};
/*@unused@*/ 
static const size_t mtbl_dark2_sizes[DIM_MTBL_DARK2] = {
     sizeof(double), sizeof(double),  
     sizeof(short), sizeof(short), sizeof(short), sizeof(short),
     2 * SCIENCE_CHANNELS * sizeof(short), 
     sizeof(float), SCIENCE_CHANNELS * sizeof(float)
};

/* structure definitions Simu-Dark database (SDMF v3.0) */
#define DIM_MTBL_SIMUDARK 13
struct mtbl_simudark_rec
{
     double julianDate;
     int    absOrbit;
     char   entryDate[STR_SZ_DATE];
     bool   saaFlag;
     float  obmTemp;
     float  detTemp;
     float  quality;
     float  orbitPhase;
     float  sig_phase;
     float  phase2;
     float  sig_phase2;
     float  amp2;
     float  sig_amp2;
};
/*@unused@*/ 
static const size_t mtbl_simudark_sizes[DIM_MTBL_SIMUDARK] = {
     sizeof(double), sizeof(int), STR_SZ_DATE * sizeof( char), 
     sizeof(bool), sizeof(float), sizeof(float), sizeof(float), 
     sizeof(float), sizeof(float), sizeof(float), sizeof(float), 
     sizeof(float), sizeof(float)
};

/* structure definitions SMR database (SDMF v3.1) */
#define DIM_MTBL_SMR2    13
struct mtbl_smr2_rec 
{
     double julianDay;
     char   entryDate[STR_SZ_DATE];
     unsigned short absOrbit;                 
     unsigned short quality;
     float  orbitPhase;
     float  longitude;
     float  latitude;
     float  asmAngle;
     float  esmAngle;
     float  sunAzim;
     float  sunElev;
     float  obmTemp;
     float  detTemp[SCIENCE_CHANNELS];
};
/*@unused@*/ 
static const size_t mtbl_smr2_sizes[DIM_MTBL_SMR2] = {
     sizeof(double), STR_SZ_DATE * sizeof(char),  
     sizeof(short), sizeof(short), sizeof(float), sizeof(float), 
     sizeof(float), sizeof(float), SCIENCE_CHANNELS * sizeof(float)
};

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * function prototypes
 */
extern char *SDMF_PATH( const char * );

extern void SDMF_get_stateParam( unsigned char, unsigned short, 
				 unsigned short, 
				 /*@null@*/ unsigned short *int_pet,
				 /*@null@*/ int *orbit_range )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, int_pet, orbit_range@*/;
extern float SDMF_get_statePET( unsigned char, unsigned short, unsigned short )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/;
extern unsigned char SDMF_get_stateCoadd( unsigned char, unsigned short, 
					  unsigned short )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/;
extern unsigned short SDMF_get_stateCount( unsigned char, unsigned short )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/;
extern unsigned char SDMF_PET2StateID( unsigned short, unsigned short,
				       float ) __attribute__ ((const));
extern float SDMF_orbitPhaseDiff( int orbit )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack@*/;

extern bool SDMF_get_fileEntry( enum sdmf24_db sdmfDB, int, 
				/*@out@*/ char *fileEntry )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, fileEntry@*/;

extern bool SDMF_get_BDPM_24( unsigned short, /*@out@*/ unsigned char *bdpm )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, bdpm@*/;
extern bool SDMF_get_BDPM_30( unsigned short, /*@out@*/ unsigned char *bdpm )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, bdpm@*/;

extern bool SDMF_get_PPG_24( unsigned short, unsigned short, 
			     /*@out@*/ float *ppg )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, ppg@*/;
extern bool SDMF_get_PPG_30( unsigned short, unsigned short, 
			     /*@out@*/ float *ppg )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, ppg@*/;

extern bool SDMF_get_StateDark_24( unsigned char, unsigned short, 
				   unsigned short, /*@out@*/ float *pet, 
				   /*@out@*/ float *darkSignal, 
				   /*@out@*/ float *darkNoise )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, pet, darkSignal, darkNoise@*/;
extern bool SDMF_get_StateDark_30( unsigned char, unsigned short, 
				   unsigned short, /*@out@*/ float *pet, 
				   /*@out@*/ float *darkSignal, 
				   /*@out@*/ float *darkNoise )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, pet, darkSignal, darkNoise@*/;
extern bool SDMF_get_StateDark( unsigned char, unsigned short, unsigned short, 
				/*@out@*/ float *pet, 
				/*@out@*/ float *darkSignal, 
				/*@out@*/ float *darkNoise )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, pet, darkSignal, darkNoise@*/;

extern bool SDMF_get_FittedDark_24( unsigned short, unsigned short,
				    /*@out@*/ float *ao, /*@out@*/ float *lc, 
				    /*@null@*/ /*@out@*/ float *,
				    /*@null@*/ /*@out@*/ float *,
				    /*@null@*/ /*@out@*/ float * )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, ao, lc@*/;
extern bool SDMF_get_FittedDark_30( unsigned short, unsigned short,
				    /*@out@*/ float *ao, /*@out@*/ float *lc, 
				    /*@null@*/ /*@out@*/ float *,
				    /*@null@*/ /*@out@*/ float *,
				    /*@null@*/ /*@out@*/ float * )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, ao, lc@*/;
extern bool SDMF_get_FittedDark( unsigned short, unsigned short,
				 /*@out@*/ float *ao, /*@out@*/ float *lc, 
				 /*@null@*/ /*@out@*/ float *analogOffsError,
				 /*@null@*/ /*@out@*/ float *darkCurrrentError,
				 /*@null@*/ /*@out@*/ float *meanNoise,
				 /*@null@*/ /*@out@*/ float *chiSquareFit,
				 /*@null@*/ /*@out@*/ float *probabilityFit )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, ao, lc@*/;

extern bool SDMF_get_OrbitalDark_24( unsigned short, float, 
				     /*@out@*/ float *ao, /*@out@*/ float *lc, 
				     /*@out@*/ float *ao_err, 
				     /*@out@*/ float *lc_err )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, ao, lc, ao_err, lc_err@*/;
extern bool SDMF_get_OrbitalDark_30( unsigned short, float, 
				     /*@out@*/ float *ao, /*@out@*/ float *lc, 
				     /*@out@*/ float *ao_err, 
				     /*@out@*/ float *lc_err )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, ao, lc, ao_err, lc_err@*/;
extern bool SDMF_get_OrbitalDark( unsigned short, float, 
				  /*@out@*/ float *ao, /*@out@*/ float *lc, 
				  /*@out@*/ float *ao_err, 
				  /*@out@*/ float *lc_err )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, ao, lc, ao_err, lc_err@*/;

extern bool SDMF_get_SMR_30( bool, unsigned short, unsigned short,
			     /*@null@*/ const float *,
			     /*@out@*/ float *solarMean )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, solarMean@*/;

extern bool SDMF_get_SMR_31( bool, unsigned short, unsigned short,
			     /*@null@*/ const float *,
			     /*@out@*/ float *solarMean )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, solarMean@*/;

extern bool SDMF_get_Transmission_24( bool, unsigned short, unsigned short,
				      /*@out@*/ float *transmission )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, transmission@*/;
extern bool SDMF_get_Transmission_30( bool, unsigned short, unsigned short, 
				      /*@out@*/ float *transmission )
     /*@globals  nadc_stat, nadc_err_stack;@*/
     /*@modifies nadc_stat, nadc_err_stack, transmission@*/;

#ifdef _HDF5_H
extern int SDMF_get_metaIndex( hid_t, int, int *numIndex, int *metaIndex)
	  /*@modifies numIndex, metaIndex@*/;
extern int SDMF_get_metaIndex_range( hid_t, const int *, int *numIndex, 
				     int *metaIndex, int use_neighbours )
      /*@modifies numIndex, metaIndex@*/;
extern void SDMF30_rd_metaTable( hid_t, /*@out@*/ int *numIndx, const int *,
				 /*@out@*/ struct mtbl_calib_rec **mtbl )
          /*@modifies numIndx, *mtbl@*/;
extern void SDMF31_rd_metaTable( hid_t, /*@out@*/ int *numIndx, const int *,
				 /*@out@*/ struct mtbl_calib2_rec **mtbl )
          /*@modifies numIndx, *mtbl@*/;
extern void SDMF30_rd_histTable( hid_t, int, const int *, const int *,
                                 /*@out@*/ struct sdmf_hist1_rec *sdmf_hist )
          /*@modifies numIndx, *sdmf_hist@*/;
extern void SDMF31_rd_histTable( hid_t, int, const int *, const int *,
                                 /*@out@*/ struct sdmf_hist2_rec *sdmf_hist )
          /*@modifies numIndx, *sdmf_hist@*/;
extern void SDMF_get_pt_orbitIndex( hid_t, int, 
				    size_t *numIndex, size_t *metaIndex )
	  /*@modifies numIndex, metaIndex@*/;
extern void SDMF_get_pt_jdayIndex( hid_t, const double *, 
				   size_t *numIndex, size_t *metaIndex )
	  /*@modifies numIndex, metaIndex@*/;
extern void SDMF_rd_pt_metaTable( hid_t, /*@out@*/ size_t *numIndx, 
				  size_t *indices, struct mtbl_pt_rec **mtbl )
          /*@modifies numIndx, indices, *mtbl@*/;
extern void SDMF_rd_pt_pointing( hid_t, /*@out@*/ size_t *numIndx, 
				   size_t *indices, struct geo_pt_rec *geo )
          /*@modifies numIndx, indices, geo@*/;
extern void SDMF_rd_pt_cluster( hid_t, unsigned char, size_t *numIndx, 
				size_t *metaIndx, float *pixel_val )
          /*@modifies numIndx, metaIndx, pixel_val@*/;

extern void SDMF_rd_darkTable( hid_t, /*@out@*/ int *numIndx, int *,
			       /*@out@*/ struct mtbl_dark_rec **mtbl )
          /*@modifies numIndx, *mtbl@*/;
extern void SDMF_rd_simudarkTable( hid_t, /*@out@*/ int *numIndx, int *,
				   /*@out@*/ struct mtbl_simudark_rec **mtbl )
       /*@modifies numIndx, *mtbl@*/;

extern void SDMF_rd_string_Array( const hid_t, const char *, const int, 
				  /*@out@*/ char *);
extern void SDMF_rd_uint8_Array( hid_t, const char *, int, 
				 const int *, /*@NULL@*/ const int *, 
				 /*@out@*/ unsigned char * );
extern void SDMF_rd_uint8_Matrix( const hid_t, const char *, const int, 
				    const int *, const int, 
				    /*@out@*/ unsigned char * );
extern void SDMF_rd_int16_Array( hid_t, const char *, int, const int *, 
				   /*@NULL@*/ const int *, /*@out@*/ short * );
extern void SDMF_rd_int16_Matrix( hid_t, const char *, const int *,
				  const int *, const int, /*@out@*/ short *);

extern void SDMF_rd_uint16_Array( hid_t, const char *, int, const int *, 
				  /*@NULL@*/ const int *, 
				  /*@out@*/ unsigned short * );
extern void SDMF_rd_uint16_Matrix( hid_t, const char *, const int *,
				   const int *, const int, 
				   /*@out@*/ unsigned short *);
extern void SDMF_rd_float_Array( hid_t, const char *, int, const int *, 
				 /*@NULL@*/ const int *, /*@out@*/ float * );
extern void SDMF_rd_float_Matrix( hid_t, const char *, const int *,
				  const int *, const int, /*@out@*/ float * );
#endif /* _HDF5_H */

#ifdef __cplusplus
}
#endif
#endif /* __DEFS_SCIA_SDMF */
