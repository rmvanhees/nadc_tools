#ifndef  __DEFS_IMAP                            /* Avoid redefinitions */
#define  __DEFS_IMAP

#include <nadc_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_CORNERS   4

#ifdef __IMAP_CH4_PRODUCT
#define ADAGUC_PROD_TEMPLATE   "SCIA__%s_V__CH4_IMAP_L2__%s_%s_%04d.nc"

#define TITLE     "SCIA IMAP Methane columns"
#define REFERENCE "http://www.sron.nl/CH4columns"

#define ABSTRACT "The product contains vertical column densities of"	\
     " atmospheric methane as retrieved from SCIAMACHY near infrared"	\
     " spectrausing the  Iterative Maximum A Posteriori (IMAP) DOAS algorithm."	\
     " The coverage is global and all available data from 2003"		\
     " have been processed. Concurrent carbon dioxide retrievals are used" \
     " to convert vertical column densities to column averaged mixing"	\
     " ratios of methane."

#define STATEMENT "This product is the result of the application of the" \
     " Iterative Maximum A Posteriori (IMAP) DOAS algorithm described by" \
     " C. Frankenberg, U. Platt, and T. Wagner: Iterative maximum a"	\
     " posteriori (IMAP)-DOAS for retrieval of strongly absorbing trace" \
     " gases: model studies for CH4 and CO2 retrieval from near infrared" \
     " spectra of SCIAMACHY onboard ENVISAT [Atmos. Chem. Phys., 5:9-22, 2005]."

#define ACCESS_CONSTRAINTS "These data are made freely available to the" \
     " public and the scientific community in the belief that their wide" \
     " dissemination will lead to greater understanding and new scientific" \
     " insights."

#define USE_LIMITATIONS "The availability of these data does not constitute" \
     " publication of the data. We rely on the ethics and integrity of the" \
     " user to assure that SRON/JPL receives fair credit for our work. If" \
     " the data are obtained for potential use in a publication or"	\
     " presentation, SRON/JPL should be informed at the outset of the nature" \
     " of this work. If the SRON/JPL data are essential to the work, or if" \
     " an important result or conclusion depends on the SRON/JPL data,"	\
     " co-authorship may be appropriate. This should be discussed at an" \
     " early stage in the work. Manuscripts using the SRON/JPL data should" \
     " be sent to SRON and JPL for review before they are submitted for" \
     " publication so we can insure that the quality and limitations of" \
     " the data are accurately represented." 

#define ADAGUC_REF_DOC      "ADAGUC Data Products Standard"
#define ADAGUC_REF_VERSION  "1.1"
#define IMAP_NC_VARS        "CH4,CH4_error,CH4_model,CO2,CO2_error,CO2_model," \
     "xVMR_CH4"
#define ADAGUC_ACRONYM      "CH4IMAP"
#define IMAP_UUID           "15c0e99a-ab94-46f6-a043-c2af27762b91"
#define IMAP_META_ID        "32c6b129-3921-4a2e-9954-b491136cdf0f"

struct imap_meta_rec {
     unsigned char  stateID;
     unsigned char  bs;
     unsigned short pixels_ch4;
     unsigned short pixels_co2;
     float  intg_time;
     float  sza;           /* sza [deg] (referenced to the ground) */
     float  lza;           /* line of sight zenith angle [deg] */
     float  elev;
     float  amf;
     float  scanRange;
     float  bu_ch4;
     float  resi_ch4;
     float  bu_co2;
     float  resi_co2;
};

struct imap_rec {
     double jday;
     float  lon_corner[NUM_CORNERS];
     float  lat_corner[NUM_CORNERS];
     float  lon_center;
     float  lat_center;
     float  ch4_vcd;
     float  ch4_error;
     float  ch4_model;
     float  ch4_vmr;
     float  co2_vcd;
     float  co2_error;
     float  co2_model;
     struct imap_meta_rec meta;
};
#endif /* __IMAP_CH4_PRODUCT */

#ifdef __IMAP_HDO_PRODUCT
#define ADAGUC_PROD_TEMPLATE   "SCIA__%s_V__HDO_IMAP_L2__%s_%s_%04d.nc"

#define TITLE     "SCIA IMAP HDO/H2O ratio"
#define REFERENCE "http://www.sron.nl/HDO"

#define ABSTRACT "This product contains the ratio between the vertical"\
     " column densities of the heavy water vapor isotopologue HDO and H2O"\
     " as retrieved from SCIAMACHY near-infrared specra (channel 8). The"\
     " coverage is global and for the years 2003 through 2005. The retrieval"\
     " algorithm used is the SRON/JPL Iterative Maximum A Posteriori (IMAP)"\
     " DOAS algorithm."

#define STATEMENT "This product is the result of the application of the"\
     " Iterative Maximum A Posteriori (IMAP) DOAS algorithm. The algorithm"\
     " was first described by C. Frankenberg, U. Platt, and T. Wagner:"\
     " Iterative maximum a posteriori (IMAP)-DOAS for retrieval of"\
     " strongly absorbing trace gases: model studies for CH4 and CO2"\
     " retrieval from near infrared spectra of SCIAMACHY onboard ENVISAT"\
     " [Atmos. Chem. Phys., 5:9-22, 2005]. The algorithm was adapted to the"\
     " retrieval of the HDO/H2O product, first described in C. Frankenberg"\
     " et al.: Dynamic processes governing lower-tropospheric HDO/H2O ratios"\
     " as observed from space and ground, [Science, 325:1374-1377, 2009]."

#define ACCESS_CONSTRAINTS "These data are made freely available to the"\
     " public and the scientific community in the belief that their wide"\
     " dissemination will lead to greater understanding and new scientific"\
     " insights."

#define USE_LIMITATIONS "The availability of these data does not constitute"\
     " publication of the data. We rely on the ethics and integrity of the"\
     " user to assure that SRON/JPL receives fair credit for our work. If"\
     " the data are obtained for potential use in a publication or"\
     " presentation, SRON/JPL should be informed at the outset of the nature"\
     " of this work. If the SRON/JPL data are essential to the work, or if"\
     " an important result or conclusion depends on the SRON/JPL data,"\
     " co-authorship may be appropriate. This should be discussed at an"\
     " early stage in the work. Manuscripts using the SRON/JPL data should"\
     " be sent to SRON and JPL for review before they are submitted for"\
     " publication so we can insure that the quality and limitations of"\
     " the data are accurately represented." 

#define ADAGUC_REF_DOC      "ADAGUC Data Products Standard"
#define ADAGUC_REF_VERSION  "1.1"
#define IMAP_NC_VARS        "HDO,HDO_error,H2O,H2O_error,H2O_model,delta_d,delta_d_error"
#define ADAGUC_ACRONYM      "HDO_IMAP"
#define IMAP_UUID           "c020ce90-7176-11e0-a1f0-0800200c9a66"
#define IMAP_META_ID        "c020ce91-7176-11e0-a1f0-0800200c9a66"

struct imap_meta_rec {
     unsigned char  stateID;
     unsigned char  bs;
     unsigned short pixels_fresco;
     float  intg_time;
     float  sza;           /* sza [deg] (referenced to the ground) */
     float  lza;           /* line of sight zenith angle [deg] */
     float  elev;          /* elevation, meters */
     float  rms_res;       /* 1-sigma of the fit residuals */
     float  rms_res_wght;  /* same, weighted by SQRT(estimated pixel noise) */
     float  bu;            /* detector readout at first pixel, BU */
     float  cl_frac;       /* Fresco cloud fraction */
     float  cl_press;      /* Fresco cloud top pressure */
     float  ocean_frac;    /* Fraction pixel covered by ocean */
};

struct imap_rec {
     double jday;                    /* decimal days since 1.1.2000 */
     float  lat_center;              /* center latitude, degrees */
     float  lon_center;              /* central longitude, degrees */
     float  lat_corner[NUM_CORNERS]; /* corners latitude, degrees */
     float  lon_corner[NUM_CORNERS]; /* corners longitude, degrees */
     float  hdo_vcd;                 /* VCD, molecules per cm^2 */
     float  hdo_error;               /* error, molecules per cm^2 */
     float  h2o_vcd;                 /* VCD, molecules per cm^2 */
     float  h2o_error;               /* error, molecules per cm^2 */
     float  h2o_model;               /* ECMWF, molecules per cm^2 */
     float  delta_d;                 /* per million */
     float  delta_d_error;           /* per million */
     struct imap_meta_rec meta;
};
#endif /* __IMAP_HDO_PRODUCT */


#ifdef __NEED_ISO_ENTRIES
struct iso_entry_t {
     const char *attr_name;
     /*@null@*/ const char *attr_value;
};

static struct iso_entry_t meta_root_list[] = {
     { "title", TITLE },
     { "institution", "SRON Netherlands Institute for Space Research" },
     { "source", "SCIA" },
     { "history", "none" },
     { "references", REFERENCE },
     { "comment", "none" },
     { "Convention", "CF-1.4"}
};

/*@unused@*/
static const unsigned short numRootKeys = (unsigned short)
     (sizeof(meta_root_list) / sizeof(struct iso_entry_t));

static struct iso_entry_t meta_prod_list[] = {
     { "ref_doc", ADAGUC_REF_DOC },
     { "ref_doc_version", ADAGUC_REF_VERSION },
     { "format_version", "1.0" },
     { "input_products", NULL },
     { "originator", "SRON Netherlands Institute for Space Research" },
     { "creation_date", NULL },
     { "software_version", NULL },
     { "file_class", "CONS" },
     { "type", "V" },
     { "acronym", ADAGUC_ACRONYM },
     { "level", "L2" },
     { "variables", IMAP_NC_VARS },
     { "validity_start", NULL },
     { "validity_stop", NULL },
     { "style", "lowercase" }
 };
/*@unused@*/
static const unsigned short numProdKeys = (unsigned short)
     (sizeof(meta_prod_list) / sizeof(struct iso_entry_t));

static struct iso_entry_t geo_prod_list[] = {
     { "projection_name", "Latitude Longitude" },
     { "EPSG_code", "EPSG:4326" },
     { "proj4_params", "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs" }
};
/*@unused@*/
static const unsigned short numGeoKeys = (unsigned short)
     (sizeof(geo_prod_list) / sizeof(struct iso_entry_t));

static const struct iso_entry_t iso_prod_list[] = {
     { "title", TITLE },
     { "abstract", ABSTRACT },
     { "status", "completed" },
     { "type", "dataset" },
     { "uid",  IMAP_UUID },
     { "topic", "atmosphere" },
     { "keyword", "Tropospheric Atmospheric Global "			\
       "[Theme=atmosphere,air quality],[Place=Global Statum=troposphere]," \
       "[Temporal=recent years]" },
     { "keyword_title", "netCDF-CF" },
     { "max-x", "180.0" },
     { "min-x", "-180.0" },
     { "max-y", "90.0" },
     { "min-y", "-90.0" },
     { "temporal_extent", "2002-09-01 2012-09-01" },
     { "date", "2009-01-24" },
     { "dateType", "revision" },
     { "statement", STATEMENT },
     { "code", "4326 (WGS84)" },
     { "codeSpace", "EPSG" },
     { "accessConstraints", ACCESS_CONSTRAINTS },
     { "useLimitation", USE_LIMITATIONS },
     { "organisationName_dataset", 
       "SRON Netherlands Institute for Space Research" },
     { "email_dataset", "R.A.Scheepmaker_at_sron.nl" },
     { "role_dataset", "pointOfContact" },
     { "metadata-id", IMAP_META_ID },
     { "organisationName_metadata", 
       "SRON Netherlands Institute for Space Research" },
     { "role_metadata", "pointOfContact" },
     { "email_metadata", "adaguc@knmi.nl" },
     { "url_metadata", "http://adaguc.knmi.nl" },
     { "datestamp", "2009-01-24" },
     { "language", "eng" },
     { "metadataStandardName", "ISO19115" },
     { "metadataStandardNameVersion", 
       "Nederlandse metadatastandaard voor geografie 1.2" }
};
/*@unused@*/
static const unsigned short numIsoKeys = (unsigned short)
     (sizeof(iso_prod_list) / sizeof(struct iso_entry_t));
#endif /* __NEED_ISO_ENTRIES */

/* SCIA IMAP structures */
struct imap_hdr {
     char  product[SHORT_STRING_LENGTH];
     char  receive_date[UTC_STRING_LENGTH];
     char  creation_date[UTC_STRING_LENGTH];
     char  l1b_product[MAX_ADAGUC_INFILES * ENVI_FILENAME_SIZE];
     char  validity_start[16];
     char  validity_stop[16];
     char  software_version[8];
     unsigned short numProd;
     unsigned int   numRec;
     unsigned int   orbit[MAX_ADAGUC_INFILES];
     unsigned short counter[MAX_ADAGUC_INFILES];
     unsigned int   file_size;
};

/* SCIA IMAP fuction prototypes */
extern void SCIA_RD_IMAP_CH4( bool, const char *, 
			      /*@out@*/ struct imap_hdr *hdr, 
			      /*@null@*/ /*@out@*/ struct imap_rec **rec )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, hdr, rec@*/;
extern void SCIA_RD_IMAP_HDO( const char *, /*@out@*/ struct imap_hdr *hdr, 
			      /*@null@*/ /*@out@*/ struct imap_rec **rec )
     /*@globals  errno, nadc_stat, nadc_err_stack;@*/
     /*@modifies errno, nadc_stat, nadc_err_stack, hdr, rec@*/;

#if defined(_NETCDF_)
     extern void SCIA_RD_NC_CH4_META( int, /*@out@*/ struct imap_hdr * );
     extern unsigned int SCIA_RD_NC_CH4_REC( int, 
					     /*@out@*/ struct imap_rec ** );
     extern void SCIA_WR_NC_CH4_META( int, const struct imap_hdr * );
     extern void SCIA_WR_NC_CH4_REC( int, unsigned int, 
				     const struct imap_rec * );

     extern void SCIA_RD_NC_HDO_META( int, /*@out@*/ struct imap_hdr * );
     extern unsigned int SCIA_RD_NC_HDO_REC( int, 
					     /*@out@*/ struct imap_rec ** );
     extern void SCIA_WR_NC_HDO_META( int, const struct imap_hdr * );
     extern void SCIA_WR_NC_HDO_REC( int, unsigned int, 
				     const struct imap_rec * );
#endif

#if defined(LIBPQ_FE_H)
     extern void SCIA_DEL_ENTRY_IMAP_CH4( PGconn *conn, const char * )
          /*@globals  errno, nadc_stat, nadc_err_stack;@*/
	  /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
     extern void SCIA_WR_SQL_CH4_META( PGconn *conn, const struct imap_hdr * )
          /*@globals  errno, nadc_stat, nadc_err_stack;@*/
	  /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
     extern void SCIA_WR_SQL_CH4_TILE( PGconn *conn, const char *, unsigned int,
				       const struct imap_rec * )
          /*@globals  errno, nadc_stat, nadc_err_stack;@*/
	  /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;

     extern void SCIA_DEL_ENTRY_IMAP_HDO( PGconn *conn, const char * )
          /*@globals  errno, nadc_stat, nadc_err_stack;@*/
	  /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
     extern void SCIA_WR_SQL_HDO_META( PGconn *conn, const struct imap_hdr * )
          /*@globals  errno, nadc_stat, nadc_err_stack;@*/
	  /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
     extern void SCIA_WR_SQL_HDO_TILE( PGconn *conn, const char *, unsigned int,
				       const struct imap_rec * )
          /*@globals  errno, nadc_stat, nadc_err_stack;@*/
	  /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
#endif

#ifdef __cplusplus
  }
#endif
#endif   /* __DEFS_IMAP */
