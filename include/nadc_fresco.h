#ifndef  __DEFS_FRESCO                          /* Avoid redefinitions */
#define  __DEFS_FRESCO

#include <nadc_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_CORNERS   4

#define Title \
   "KNMI Fast Retrieval Scheme for Cloud Observables (FRESCO+) method"

#define PROD_REF_DOC       "Product Specification Document FRESCO and FRESCO+"
#define PROD_REF_VERSION   "2.1"
#define ADAGUC_REF_VERSION "1.1"

#define ADAGUC_PROD_TEMPLATE  "%s__%s_V___FRESCO__L2__%s_%s_%04d.nc"

#define Copyright "(c) TEMIS/KNMI"

#define GOME_UUID   "08950d90-ddab-4363-bcbd-0db2e3d5ffc6"
#define GOME_METAID "2defed6d-66b2-472d-a5fe-8aaa454b74fc"

#define SCIA_UUID   "637fa615-1245-443f-811e-c00c88e74b73"
#define SCIA_METAID "3b01e24e-c5d7-4ff7-b751-422539663a0a"

#define GOME_Abstract "Global effective cloudfraction and cloud pressure"\
     " derived with the GOME instrument"
#define SCIA_Abstract "Global effective cloudfraction and cloud pressure"\
     " derived with the SCIAMACHY instrument"

#define Statement "For the retrieval of tropospheric trace gas columns it is"\
     " essential to have information on the cloud cover conditions. Both the"\
     " cloud fraction and the cloud pressure are needed as input for the"\
     " trace gas retrieval algorithms. With the FRESCO+ cloud algorithm, the"\
     " (effective) cloud fraction and cloud pressure are derived from GOME"\
     " and SCIAMACHY measurements."

#define Reference "http://www.temis.nl/fresco/fresco.html"

#define FRESCO_NC_VARS "cloudFraction,cloudFractionError,cloudTopHeight,"\
                       "cloudTopPress,cloudTopPressError,cloudAlbedo,"\
                       "cloudAlbedoError,surfaceAlbedo,surfaceHeight,"\
                       "groundPress,tile"

#ifdef __NEED_ISO_ENTRIES
struct iso_entry_t {
     const char *attr_name;
     /*@null@*/ const char *attr_value;
};

static struct iso_entry_t meta_root_list[] = {
     { "title", Title },
     { "institution", "KNMI" },
     { "source", NULL },
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
     { "acronym", "Fresco" },
     { "level", "L2" },
     { "variables", FRESCO_NC_VARS },
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
     { "abstract", NULL },
     { "status", "completed" },
     { "type", "dataset" },
     { "uid", NULL },
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
     { "metadata-id", NULL },
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
#endif     /* __NEED_ISO_ENTRIES */

/* FRESCO structures */
#define MAX_NUM_FRESCO_STATES 80
struct fresco_hdr
{
     char source[5];
     char product[26];
     char l1b_product[MAX_ADAGUC_INFILES * SHORT_STRING_LENGTH];
     char creation_date[UTC_STRING_LENGTH];
     char receive_date[UTC_STRING_LENGTH];
     char validity_start[16];
     char validity_stop[16];
     char software_version[8];
     char lv1c_version[10];
     unsigned short numProd;
     unsigned short numState;
     unsigned int   numRec;
     unsigned int   file_size;
     struct state_list_rec state_list[MAX_NUM_FRESCO_STATES];
};

struct fresco_meta_rec {
     unsigned char intg_time;
     unsigned char stateID;
     unsigned char pixelType;
     unsigned char errorFlag;
     float lza;
     float sza;
     float raa;
     float chisq;
};

struct fresco_rec
{
     double jday;
     float lon_corner[NUM_CORNERS];
     float lat_corner[NUM_CORNERS];
     float lon_center;
     float lat_center;
     float cloudFraction;
     float cloudFractionError;
     float cloudTopHeight;
     float cloudTopPress;
     float cloudTopPressError;
     float cloudAlbedo;
     float cloudAlbedoError;
     float surfaceAlbedo;
     float surfaceHeight;
     float groundPress;
     struct fresco_meta_rec meta;
};

extern unsigned int NADC_RD_FRESCO( const char *, 
				    /*@out@*/ struct fresco_hdr *,
				    /*@out@*/ struct fresco_rec ** )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack@*/;

#if defined(_NETCDF_)
extern void NADC_FRESCO_RD_NC_META( int , /*@out@*/ struct fresco_hdr * );
extern unsigned int NADC_FRESCO_RD_NC_REC( int, 
					   /*@out@*/ struct fresco_rec ** );
extern void NADC_FRESCO_WR_NC_META( int, const struct fresco_hdr * );
extern void NADC_FRESCO_WR_NC_REC( int, const char *, unsigned int,
				   const struct fresco_rec * );
#endif

#if defined(LIBPQ_FE_H)
extern int NADC_FRESCO_WR_SQL_META( PGconn *conn, const struct fresco_hdr * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;

extern void NADC_FRESCO_WR_SQL_GOME_TILE( PGconn *conn, int, unsigned int, 
					  const struct fresco_rec * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void NADC_FRESCO_WR_SQL_SCIA_TILE( PGconn *conn, const char *, 
				     unsigned int, const struct fresco_rec * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;

extern void NADC_FRESCO_DEL_GOME_ENTRY( PGconn *conn, const char * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
extern void NADC_FRESCO_DEL_SCIA_ENTRY( PGconn *conn, const char * )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, conn@*/;
#endif

#ifdef __cplusplus
  }
#endif
#endif   /* __DEFS_FRESCO */
