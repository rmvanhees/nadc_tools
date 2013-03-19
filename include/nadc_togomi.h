#ifndef  __DEFS_TOGOMI                          /* Avoid redefinitions */
#define  __DEFS_TOGOMI

#include <nadc_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_CORNERS   4

#define Title \
   "KNMI GOME Total Ozone (TOGOMI) Product"

#define PROD_REF_DOC        "TOGOMI Algorithm Theoretical Basis Document"
#define PROD_REF_VERSION    "1.2"
#define ADAGUC_REF_VERSION  "1.1"

#define ADAGUC_PROD_TEMPLATE   "GOME__%s_V___TOGOMI__L2__%s_%s_%04d.nc"

#define Copyright "(c) TEMIS/KNMI"

#define UUID   "1770e104-35b5-44cc-9c09-b24ee78ade35"
#define METAID "8d360d96-15fc-4a13-9888-e5610b961f75"

#define Abstract "Global total ozone columns derived with a DOAS algorithm"\
     " from GOME observations."

#define Statement "The total ozone column is retrieved from GOME observations"\
     " with an algorithm based on the Differential Optical Absorption"\
     " Spectroscopy (DOAS) method. The TOGOMI algorithm consists of three"\
     " steps. First, the reference differential absorption spectrum of ozone"\
     " is fitted to the measured Earth radiance spectrum and solar"\
     " irradiance spectrum, to obtain the slant column density. In the"\
     " second step the slant column density is translated into the vertical"\
     " column density using the so-called air mass factor (AMF). The third"\
     " step consists of a correction for the screening of ozone below clouds."

#define Reference "http://www.temis.nl/protocols/O3total.html"

#define TOGOMI_NC_VARS "scd,scdError,vcd,vcdError,tile_properties"

#ifdef __NEED_ISO_ENTRIES
struct iso_entry_t {
     const char *attr_name;
     /*@null@*/ const char *attr_value;
};

static struct iso_entry_t meta_root_list[] = {
     { "title", Title },
     { "institution", "KNMI" },
     { "source", "GOME" },
     { "history", "none" },
     { "references", Reference },
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
     { "originator", "KNMI" },
     { "creation_date", NULL },
     { "software_version", NULL },
     { "file_class", "CONS" },
     { "type", "V" },
     { "acronym", "TOGOMI" },
     { "level", "L2" },
     { "variables", TOGOMI_NC_VARS },
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
     { "title", Title },
     { "abstract", Abstract },
     { "status", "completed" },
     { "type", "dataset" },
     { "uid", UUID },
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
     { "statement", Statement },
     { "code", "4326 (WGS84)" },
     { "codeSpace", "EPSG" },
     { "accessConstraints", Copyright },
     { "useLimitation", "inapplicable" },
     { "organisationName_dataset", 
       "Royal Netherlands Meteorological Institute (KNMI)" },
     { "email_dataset", "http://www.temis.nl/contact.php" },
     { "role_dataset", "pointOfContact" },
     { "metadata-id", METAID },
     { "organisationName_metadata", 
       "Royal Netherlands Meteorological Institute (KNMI)" },
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
#endif   /* __NEED_ISO_ENTRIES */

/* TOGOMI structures */
#define MAX_NUM_TOGOMI_STATES 1
struct togomi_hdr
{
     char product[26];
     char creation_date[UTC_STRING_LENGTH];
     char receive_date[UTC_STRING_LENGTH];
     char l1b_product[MAX_ADAGUC_INFILES * ERS2_FILENAME_SIZE];
     char validity_start[16];
     char validity_stop[16];
     char software_version[8];
     char lv1c_version[10];
     unsigned short numProd;
     unsigned short numState;
     unsigned int   numRec;
     unsigned int   file_size;
     struct state_list_rec state_list[MAX_NUM_TOGOMI_STATES];
};

struct togomi_meta_rec {
     unsigned char  intg_time;
     unsigned char  stateID;
     unsigned char  pixelType;
     unsigned char  swathType;
     float sza;
     float effTemp;
     float ghostColumn;
     float cloudFraction;
     float cloudHeight;
     float amfSky;
     float amfCloud;
};

struct togomi_rec
{
     double jday;
     struct togomi_meta_rec meta;
     float  lon_corner[NUM_CORNERS];
     float  lat_corner[NUM_CORNERS];
     float  lon_center;
     float  lat_center;
     float  scd;
     float  scdError;
     float  vcd;
     float  vcdError;
};

/* TOGOMI fuction prototypes */
extern unsigned int NADC_RD_TOGOMI( const char *, 
				    /*@out@*/ struct togomi_hdr *,
				    /*@out@*/ struct togomi_rec ** )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;

#if defined(_NETCDF_)
extern void NADC_TOGOMI_RD_NC_META( int, /*@out@*/ struct togomi_hdr * );
extern unsigned int NADC_TOGOMI_RD_NC_REC( int, 
					   /*@out@*/ struct togomi_rec ** );
extern void NADC_TOGOMI_WR_NC_META( int, const struct togomi_hdr * );
extern void NADC_TOGOMI_WR_NC_REC( int, unsigned int,
                                   const struct togomi_rec * );
#endif

#if defined(LIBPQ_FE_H)
extern int NADC_TOGOMI_WR_SQL_META( PGconn *conn, 
				    const struct togomi_hdr * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;

       extern void NADC_TOGOMI_WR_SQL_TILE( PGconn *conn, int, 
				     unsigned int, const struct togomi_rec * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;

extern void NADC_TOGOMI_DEL_ENTRY( PGconn *conn, const char * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
#endif

#ifdef __cplusplus
  }
#endif
#endif   /* __DEFS_TOGOMI */
