#ifndef  __DEFS_IMLM                            /* Avoid redefinitions */
#define  __DEFS_IMLM

#include <nadc_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_CORNERS   4

#ifdef __IMLM_CO_PRODUCT
#define ADAGUC_PROD_TEMPLATE   "SCIA__%s_V___CO_IMLM_L2__%s_%s_%04d.nc"

#define TITLE     "SRON Sciamachy carbon monoxide columns"

#define ABSTRACT "Global CO total columns derived with the Iterative Maximum"\
     " Likelihood Method (IMLM) retrieval algorithm from SCIAMACHY 2.3"\
     " micron spectra"

#define STATEMENT "This product is the result of the application of the SRON"\
     " IMLM retrieval algorithm as described by A. Gloudemans, H. Schrijver,"\
     " O. Hasekamp, and I. Aben: Error analysis for CO and CH4 total column"\
     " retrievals from SCIAMACHY 2.3 micron spectra [Atmos. Chem. Phys.,"\
     " 8:3999-4017, 2008] and references therein."

#define USE_LIMITATIONS "The availability of these data does not constitute "\
     "publication of the data. We rely on the ethics and integrity of the "\
     "user to assure that SRON receives fair credit for our work. If the "\
     "data are obtained for potential use in a publication or presentation, "\
     "SRON should be informed at the outset of the nature of this work. If "\
     "the SRON data are essential to the work, or if an important result or "\
     "conclusion depends on the SRON data, co-authorship would be "\
     "appropriate. This should be discussed at an early stage in the work. "\
     "Manuscripts using the SRON data should be sent to SRON for review "\
     "before they are submitted for publication so we can insure that the "\
     "quality and limitations of the data are accurately represented.\n"\
     "Note that individual SCIAMACHY carbon monoxide measurements have large "\
     "instrument-noise errors of ~10-100% and that thus some spatial and/or "\
     "temporal averaging needs to be done before you can use the data. "\
     "Monthly mean instrument-noise errors in a 3x2 degree grid box "\
     "typically have errors below 10%. When computing averages, please "\
     "follow the averaging procedure as described in de Laat et al. (2006), "\
     "using the CO data from this data set. Do not exclude negative, zero, or "\
     "very large CO total columns from the averaging procedure, since this "\
     "may lead to a positive bias in the averaged data. The averaging "\
     "procedure involves computing a weighted average using the inverse of "\
     "the CO total column error squared as the weights, and as such takes "\
     "care of extraordinary high or low CO columns, since these values have "\
     "large errors and thus get a low weight in the averaging procedure."

#define IMLM_ACRONYM "CO_IMLM"
#define IMLM_NC_VARS "CO,CO_error,CH4,CH4_error,albedo,cloudFraction,"\
     "meanElevation"
#define IMLM_UUID      "db0c794d-2d16-4c41-a088-9ebc07b864dc"
#define IMLM_META_ID   "bee6e1e3-3da4-41a3-9dfe-85054c25f61c"
#endif  /* __IMLM_CO_PRODUCT */

#ifdef __IMLM_H2O_PRODUCT
#define ADAGUC_PROD_TEMPLATE   "SCIA__%s_V__H2O_IMLM_L2__%s_%s_%04d.nc"

#define TITLE     "SRON Sciamachy water vapour columns"

#define ABSTRACT "Global H2O total columns derived with the Iterative Maximum"\
     " Likelihood Method (IMLM) retrieval algorithm from SCIAMACHY 2.3"\
     " micron spectra"

#define STATEMENT "This product is the result of the application of the SRON"\
     " IMLM retrieval algorithm as described by A. Gloudemans, H. Schrijver,"\
     " O. Hasekamp, and I. Aben: Error analysis for CO and CH4 total column"\
     " retrievals from SCIAMACHY 2.3 micron spectra [Atmos. Chem. Phys.,"\
     " 8:3999-4017, 2008], H. Schrijver, A.M.S. Gloudemans, C. Frankenberg,"\
     " and I. Aben: Water vapour total columns from SCIAMACHY in the 2.36"\
     " micrometer window [Atmos. Meas. Tech., 2, 561-571, 2009] and references"\
     " therein."

#define USE_LIMITATIONS "The availability of these data does not constitute "\
     "publication of the data. We rely on the ethics and integrity of the "\
     "user to assure that SRON receives fair credit for our work. If the "\
     "data are obtained for potential use in a publication or presentation, "\
     "SRON should be informed at the outset of the nature of this work. If "\
     "the SRON data are essential to the work, or if an important result or "\
     "conclusion depends on the SRON data, co-authorship would be "\
     "appropriate. This should be discussed at an early stage in the work. "\
     "Manuscripts using the SRON data should be sent to SRON for review "\
     "before they are submitted for publication so we can insure that the "\
     "quality and limitations of the data are accurately represented.\n"\
     "Note that individual SCIAMACHY water vapour measurements can have large "\
     "instrument-noise errors in the case of low ground albedo, in particular "\
     "over cloud-free ocean scenes. A reasonable error for such scenes "\
     "usually implies undetected cloudiness (outside sunglint or sea ice "\
     "regions). In regions with large erros some spatial and/or temporal "\
     "averaging may be needed before you can use the data. Additiona cloud "\
     "filtering can be performed using the methane total columns provided. "\
     "When computing averages, please "\
     "follow the averaging procedure as described in de Laat et al. (2006), "\
     "for CO data. Do not exclude negative, zero, or "\
     "very large total columns from the averaging procedure, since this "\
     "may lead to a positive bias in the averaged data. The averaging "\
     "procedure involves computing a weighted average using the inverse of "\
     "the total column error squared as the weights, and as such takes "\
     "care of extraordinary high or low columns, since these values have "\
     "large errors and thus get a low weight in the averaging procedure."

#define IMLM_ACRONYM "H2O_IMLM"
#define IMLM_NC_VARS "CH4,CH4_error,H2O,H2O_error,albedo,cloudFraction,"\
     "meanElevation"
#define IMLM_UUID      "d3d3e920-7169-11e0-a6aa-0002a5d5c51b"
#define IMLM_META_ID   "1e84b300-716a-11e0-a468-0002a5d5c51b"
#endif /* __IMLM_H2O_PRODUCT */

#define REFERENCE           "http://www.sron.nl/COcolumns"

#define PROD_REF_DOC        "IMLM v6.3 algorithm description"
#define PROD_REF_VERSION    "1.0"
#define ADAGUC_REF_VERSION  "1.1"

#define ACCESS_CONSTRAINTS "These data are made freely available to the "\
     "public and the scientific community in the belief that their wide "\
     "dissemination will lead to greater understanding and new scientific "\
     "insights. This is a purely scientific product and thus often requires "\
     "co-authorship of the retrieval group if the data is used for any "\
     "publication (see also the Use limitations). For any questions "\
     "regarding data use, results, and/or interpretation of the results "\
     "please contact Catharinus (Rien) Dijkstra (C.Dijkstra_at_sron.nl)."

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
     { "ref_doc", PROD_REF_DOC },
     { "ref_doc_version", PROD_REF_VERSION },
     { "format_version", ADAGUC_REF_VERSION },
     { "input_products", NULL },
     { "originator", "SRON Netherlands Institute for Space Research" },
     { "creation_date", NULL },
     { "software_version", NULL },
     { "file_class", "CONS" },
     { "type", "V" },
     { "acronym", IMLM_ACRONYM },
     { "level", "L2" },
     { "variables", IMLM_NC_VARS },
     { "validity_start", NULL },
     { "validity_stop", NULL },
     { "style", "mixed" }
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
     { "uid", IMLM_UUID },
     { "topic", "atmosphere" },
     { "keyword", "Tropospheric Atmospheric Global " \
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
     { "email_dataset", "C.Dijkstra_at_sron.nl" },
     { "role_dataset", "pointOfContact" },
     { "metadata-id", IMLM_META_ID },
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

/* SCIA IMLM structures */
struct imlm_hdr {
     char  product[SHORT_STRING_LENGTH];
     char  contact[64];
     char  creation_date[UTC_STRING_LENGTH];
     char  receive_date[UTC_STRING_LENGTH];
     char  l1b_product[MAX_ADAGUC_INFILES * ENVI_FILENAME_SIZE];
     char  validity_start[16];
     char  validity_stop[16];
     char  product_format[8];
     char  software_version[8];
     char  pixelmask_version[16];
     char  cloudmask_version[16];
     unsigned char  scia_channel;
     unsigned short numProd;
     unsigned int   numRec;
     unsigned int   orbit[MAX_ADAGUC_INFILES];
     unsigned int   file_size;
     unsigned short window_pixel[2];
     float          window_wave[2];
};

struct imlm_meta_rec {
     float  intg_time;     /* integration time [seconds] */
     float  sza;           /* sza [deg] (referenced to the ground) */
     float  lza;           /* line of sight zenith angle [deg] */
     float  chisq;         /* reduced chi-square of fit */
     unsigned short dof;   /* degrees of freedom */
     unsigned short eflag; /* retrieval error flag */
     unsigned char  st;    /* ordinal number of state (first state = 0) */
     unsigned char  bs;    /* backscan ground pixel identifyer */
     unsigned short px;    /* ground pixel number (within a state) */
};

struct imlm_rec {
     double dsr_time;      /* data set record time [days] */
     struct imlm_meta_rec meta;
     float  lon_corner[NUM_CORNERS];
     float  lat_corner[NUM_CORNERS];
     float  lon_center;
     float  lat_center;
     float  albedo;        /* albedo */
     float  cl_fr;         /* cloud fraction [0 -> 1.0] */
     float  mean_elev;     /* mean elevation [m] */
     float  signal;        /* only for internal SRON usage */
     float  pressure;      /* only for internal SRON usage */
     float  CO;            /* retrieved vertical column of CO [cm-2] */
     float  CO_err;        /* error retrieved vertical column of CO [cm-2] */
     float  CH4;           /* retrieved vertical column of CH4 [cm-2] */
     float  CH4_err;       /* error retrieved vertical column of CH4 [cm-2] */
     float  H2O;           /* retrieved vertical column of H2O [cm-2] */
     float  H2O_err;       /* error retrieved vertical column of H2O [cm-2] */
     float  HDO;           /* retrieved vertical column of HDO [cm-2] */
     float  HDO_err;       /* error retrieved vertical column of HDO [cm-2] */
     float  N2O;           /* retrieved vertical column of N2O [cm-2] */
     float  N2O_err;       /* error retrieved vertical column of N2O [cm-2] */
};

/* SCIA IMLM fuction prototypes */
extern void SCIA_RD_IMLM( const char *, /*@out@*/ struct imlm_hdr *hdr, 
			  /*@null@*/ /*@out@*/ struct imlm_rec **rec )
      /*@globals  errno, nadc_stat, nadc_err_stack;@*/
      /*@modifies errno, nadc_stat, nadc_err_stack, hdr, rec@*/;

#if defined(_NETCDF_)
extern void SCIA_RD_NC_CO_META( int, /*@out@*/ struct imlm_hdr * );
extern unsigned int SCIA_RD_NC_CO_REC( int, /*@out@*/ struct imlm_rec ** );
extern void SCIA_WR_NC_CO_META( int, const struct imlm_hdr * );
extern void SCIA_WR_NC_CO_REC( int, unsigned int, 
			       const struct imlm_rec * );
extern void SCIA_RD_NC_H2O_META( int, /*@out@*/ struct imlm_hdr * );
extern unsigned int SCIA_RD_NC_H2O_REC( int, /*@out@*/ struct imlm_rec ** );
extern void SCIA_WR_NC_H2O_META( int, const struct imlm_hdr * );
extern void SCIA_WR_NC_H2O_REC( int, unsigned int, 
				const struct imlm_rec * );
#endif

#if defined(LIBPQ_FE_H)
extern void SCIA_DEL_ENTRY_IMLM_CO( PGconn *conn, const char * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void SCIA_WR_SQL_CO_META( PGconn *conn, const struct imlm_hdr * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void SCIA_WR_SQL_CO_TILE( PGconn *conn, const char *, unsigned int,
				 const struct imlm_rec * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void SCIA_DEL_ENTRY_IMLM_H2O( PGconn *conn, const char * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void SCIA_WR_SQL_H2O_META( PGconn *conn, const struct imlm_hdr * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void SCIA_WR_SQL_H2O_TILE( PGconn *conn, const char *, unsigned int,
				  const struct imlm_rec * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
#endif

#ifdef __cplusplus
  }
#endif
#endif   /* __DEFS_IMLM */
