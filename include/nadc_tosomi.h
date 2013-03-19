#ifndef  __DEFS_TOSOMI                          /* Avoid redefinitions */
#define  __DEFS_TOSOMI

#include <nadc_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_CORNERS        4
#define ILON_IN_RANGE(lon) ((lon > 18000) ? (lon - 36000) : (lon))

#define Title \
   "KNMI Sciamachy Total Ozone (TOSOMI) Product"

#define PROD_REF_DOC        "Product Specification Document TOSOMI"
#define PROD_REF_VERSION    "1.1"
#define ADAGUC_REF_VERSION  "1.1"

#define ADAGUC_PROD_TEMPLATE   "SCIA__%s_V___TOSOMI__L2__%s_%s_%04d.nc"

#define Copyright "(c) TEMIS/KNMI"

#define UUID   "8f21cd96-fb4c-4c33-828c-a0eb9a8a4c8e"
#define METAID "85e4dad5-5e24-4c0d-8ed9-dc3c16b3b5ca"

#define Abstract "Global total ozone columns derived with a DOAS"\
     " algorithm from SCIAMACHY observations."

#define Statement "The total ozone column is retrieved from SCIAMACHY nadir"\
     " observations with an algorithm based on the Differential Optical"\
     " Absorption Spectroscopy (DOAS) method. The TOSOMI algorithm consists"\
     " of three steps. First, the reference differential absorption spectrum"\
     " of ozone is fitted to the measured Earth radiance spectrum and solar"\
     " irradiance spectrum, to obtain the slant column density. In the"\
     " second step the slant column density is translated into the vertical"\
     " column density using the so-called air mass factor (AMF). The third"\
     " step consists of a correction for the screening of ozone below clouds."

#define Reference "http://www.temis.nl/protocols/O3total.html"

#define TOSOMI_NC_VARS "scd,vcd,vcdError,vcdRaw,tile_properties"

#ifdef __NEED_ISO_ENTRIES
struct iso_entry_t {
     const char *attr_name;
     /*@null@*/ const char *attr_value;
};

static struct iso_entry_t meta_root_list[] = {
     { "title", Title },
     { "institution", "KNMI" },
     { "source", "SCIA" },
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
     { "acronym", "TOSOMI" },
     { "level", "L2" },
     { "variables", TOSOMI_NC_VARS },
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
#endif /* __NEED_ISO_ENTRIES */

/* TOSOMI structures */
#define MAX_NUM_TOSOMI_STATES 80
struct tosomi_hdr
{
     char product[26];
     char creation_date[UTC_STRING_LENGTH];
     char receive_date[UTC_STRING_LENGTH];
     char l1b_product[MAX_ADAGUC_INFILES * ENVI_FILENAME_SIZE];
     char validity_start[16];
     char validity_stop[16];
     char software_version[24];
     char lv1c_version[10];
     unsigned short numProd;
     unsigned short numState;
     unsigned int   numRec;
     unsigned int   file_size;
     struct state_list_rec state_list[MAX_NUM_TOSOMI_STATES];
};

 struct tosomi_meta_rec {
     unsigned char  intg_time;
     unsigned char  stateID;
     unsigned char  pixelType;
     unsigned char  radWeight;
     unsigned char  cloudFraction;
     unsigned short cloudTopPress;
     float sza;
     float vza;
     float amfSky;
     float amfCloud;
};

struct tosomi_rec
{
     double jday;
     struct tosomi_meta_rec meta;
     int    lon_corner[NUM_CORNERS];
     int    lat_corner[NUM_CORNERS];
     int    lon_center;
     int    lat_center;
     unsigned short scd;
     unsigned short vcd;
     unsigned short vcdError;
     unsigned short vcdRaw;
};

/* TOSOMI fuction prototypes */
extern unsigned int NADC_RD_TOSOMI( const char *, 
				    /*@out@*/ struct tosomi_hdr *,
				    /*@out@*/ struct tosomi_rec ** )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;

#if defined(_NETCDF_)
extern void NADC_TOSOMI_RD_NC_META( int, /*@out@*/ struct tosomi_hdr * );
extern unsigned int NADC_TOSOMI_RD_NC_REC( int, 
					   /*@out@*/ struct tosomi_rec ** );
extern void NADC_TOSOMI_WR_NC_META( int, const struct tosomi_hdr * );
extern void NADC_TOSOMI_WR_NC_REC( int, unsigned int, 
				   const struct tosomi_rec * );
#endif

#if defined(LIBPQ_FE_H)
extern int NADC_TOSOMI_WR_SQL_META( PGconn *conn, 
				    const struct tosomi_hdr * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;

extern void NADC_TOSOMI_WR_SQL_TILE( PGconn *conn, const char *, 
				     unsigned int, const struct tosomi_rec * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;

extern void NADC_TOSOMI_DEL_ENTRY( PGconn *conn, const char * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
#endif

#ifdef __cplusplus
  }
#endif
#endif   /* __DEFS_TOSOMI */
