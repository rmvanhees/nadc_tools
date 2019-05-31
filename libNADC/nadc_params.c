/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT (c) 2019 SRON (R.M.van.Hees@sron.nl)

   This is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License, version 2, as
   published by the Free Software Foundation.

   The software is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA  02111-1307, USA.

.IDENTifer   NADC_PARAMS
.AUTHOR      R.M. van Hees
.KEYWORDS    command-line parameters
.LANGUAGE    ANSI C
.PURPOSE     handle command-line parameters and default settings
.COMMENTS    None
.ENVIRONment None
.VERSION     1.0     25-May-2019   initial release
------------------------------------------------------------*/
/*
 * Define _ISOC99_SOURCE to indicate
 * that this is a ISO C99 program
 */
#define  _ISOC99_SOURCE

/*+++++ System headers +++++*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <hdf5.h>

/*+++++ Local Headers +++++*/
#include <nadc_common.h>

/* constants in this module */
#define PARAM_SET    ((unsigned char) TRUE)
#define PARAM_UNSET  ((unsigned char) FALSE)

/* structure definitions */
/* define functions for
 * - uint8 variables
 * - uint16 variables
 * - uint64 variables
 * - hid_t variables (HDF5, only)
 * - range variables (always float)
 * - string variables, require additions length parameter
 */
static struct param_uint8_rec {
     const char  *name;
     unsigned char value;
} params_uint8[] = {
     {"flag_check", PARAM_UNSET},
     {"flag_show", PARAM_UNSET},
     {"flag_version", PARAM_UNSET},
     {"flag_silent", PARAM_UNSET},
     {"flag_verbose", PARAM_UNSET},
     {"flag_cloud", PARAM_UNSET},
     {"flag_geoloc", PARAM_UNSET},
     {"flag_geomnmx", PARAM_SET},
     {"flag_period", PARAM_UNSET},
     {"flag_pselect", PARAM_UNSET},
     {"flag_subset", PARAM_UNSET},
     {"flag_sunz", PARAM_UNSET},
     {"flag_wave", PARAM_UNSET},
     {"qcheck", PARAM_SET},
     {"write_pds", PARAM_UNSET},
     {"write_ascii", PARAM_UNSET},
     {"write_meta", PARAM_UNSET},
     {"write_hdf5", PARAM_UNSET},
     {"flag_deflate", PARAM_UNSET},
     {"write_sql", PARAM_UNSET},
     {"flag_sql_remove", PARAM_UNSET},
     {"flag_sql_replace", PARAM_UNSET},
     {"write_lv1c", PARAM_UNSET},
     {"write_ads", PARAM_SET},
     {"write_gads", PARAM_SET},
     {"write_subset", PARAM_UNSET},
     {"write_blind", PARAM_UNSET},
     {"write_stray", PARAM_UNSET},
     {"write_aux0", PARAM_SET},
     {"write_pmd0", PARAM_SET},
     {"write_aux", PARAM_SET},
     {"write_det", PARAM_SET},
     {"write_pmd", PARAM_SET},
     {"write_pmd_geo", PARAM_SET},
     {"write_polV", PARAM_SET},
     {"write_limb", PARAM_SET},
     {"write_moni", PARAM_SET},
     {"write_moon", PARAM_SET},
     {"write_nadir", PARAM_SET},
     {"write_occ", PARAM_SET},
     {"write_sun", PARAM_SET},
     {"write_bias", PARAM_SET},
     {"write_cld", PARAM_SET},
     {"write_doas", PARAM_SET},
     {"chan_mask", (unsigned char) ~0U}
};

static struct param_uint16_rec {
     const char  *name;
     unsigned short value;
} params_uint16[] = {
     {"patch_scia", 0x0U},
     {"calib_earth", 0x0U},
     {"calib_limb", 0x0U},
     {"calib_moon", 0x0U},
     {"calib_sun", 0x0U},
     {"calib_pmd", 0x0U}
};

static struct param_uint32_rec {
     const char  *name;
     unsigned int value;
} params_uint32[] = {
     {"calib_scia", 0x0U}
};

static struct param_hid_rec {
     const char  *name;
     hid_t value;
} params_hid[] = {
     {"hdf_file_id", -1}
};

static struct param_range_rec {
     const char  *name;
     float value[2];
} params_range[] = {
     {"cloud_fraction", {0, 1}},
     {"latitude", {-90, 90}},
     {"longitude", {-180, 180}},
     {"sun_zenith", {-90, 90}},
     {"wavelength", {0, 9999}}
};

static struct param_string_rec {
     const char  *name;
     /*@null@*/ char  *str;
} params_string[] = {
     {"bgn_date", NULL},
     {"end_date", NULL},
     {"program", NULL},
     {"infile", NULL},
     {"outfile", NULL}
};

/*+++++++++++++++++++++++++ Exported Functions +++++++++++++++++++++*/
/*+++++++++++++++++++++++++
.IDENTifer   nadc_set_param
.PURPOSE     set a parameter to a given value
.INPUT/OUTPUT
  call as   res = nadc_set_param(param_name, value);

	 char param_name[] :     set given parameter
	 void *value       :     value for parameter

.RETURNS     zero if successful; otherwise returns a negative value
.COMMENTS    none
-------------------------*/
int nadc_set_param_uint8(const char *param_name, const unsigned char value)
{
     register size_t ii = 0;

     const size_t nkeys = sizeof params_uint8 / sizeof(struct param_uint8_rec);

     do {
	  if (strcmp(params_uint8[ii].name, param_name) == 0) {
	       params_uint8[ii].value = value;
	       return 0;
	  }
     } while (++ii < nkeys);
     
     NADC_ERROR(NADC_ERR_FATAL, "unknown parameter name");
     return -1;
}

int nadc_set_param_uint16(const char *param_name, const unsigned short value)
{
     register size_t ii = 0;

     const size_t nkeys = sizeof params_uint16 / sizeof(struct param_uint16_rec);

     do {
	  if (strcmp(params_uint16[ii].name, param_name) == 0) {
	       params_uint16[ii].value = value;
	       return 0;
	  }
     } while (++ii < nkeys);
     
     NADC_ERROR(NADC_ERR_FATAL, "unknown parameter name");
     return -1;
}

int nadc_set_param_uint32(const char *param_name, const unsigned int value)
{
     register size_t ii = 0;

     const size_t nkeys = sizeof params_uint32 / sizeof(struct param_uint32_rec);

     do {
	  if (strcmp(params_uint32[ii].name, param_name) == 0) {
	       params_uint32[ii].value = value;
	       return 0;
	  }
     } while (++ii < nkeys);
     
     NADC_ERROR(NADC_ERR_FATAL, "unknown parameter name");
     return -1;
}

int nadc_set_param_hid(const char *param_name, const hid_t value)
{
     register size_t ii = 0;

     const size_t nkeys = sizeof params_hid / sizeof(struct param_hid_rec);

     do {
	  if (strcmp(params_hid[ii].name, param_name) == 0) {
	       params_hid[ii].value = value;
	       return 0;
	  }
     } while (++ii < nkeys);
     
     NADC_ERROR(NADC_ERR_FATAL, "unknown parameter name");
     return -1;
}

int nadc_set_param_range(const char *param_name, const float *value)
{
     register size_t ii = 0;

     const size_t nkeys = sizeof params_range / sizeof(struct param_range_rec);

     do {
	  if (strcmp(params_range[ii].name, param_name) == 0) {
	       params_range[ii].value[0] = value[0];
	       params_range[ii].value[1] = value[1];
	       return 0;
	  }
     } while (++ii < nkeys);
     
     NADC_ERROR(NADC_ERR_FATAL, "unknown parameter name");
     return -1;
}

int nadc_set_param_string(const char *param_name, const char *str)
{
     register size_t ii = 0;

     const size_t nkeys = \
	  sizeof params_string / sizeof(struct param_string_rec);

     do {
	  if (strcmp(params_string[ii].name, param_name) == 0) {
	       if (params_string[ii].str != NULL)
		    free(params_string[ii].str);
	       params_string[ii].str = (char *) malloc(strlen(str) + 1);
	       if (params_string[ii].str == NULL)
		    NADC_GOTO_ERROR(NADC_ERR_ALLOC, "params_string[ii].str");
	       (void) strcpy(params_string[ii].str, str);
	       return 0;
	  }
     } while (++ii < nkeys);
     
     NADC_ERROR(NADC_ERR_FATAL, "unknown parameter name");
done:
     return -1;
}

int nadc_set_param_add_ext(const char *param_name, const char *ext)
{
     register size_t ii = 0;

     const size_t nkeys = \
	  sizeof params_string / sizeof(struct param_string_rec);

     do {
	  if (strcmp(params_string[ii].name, param_name) == 0) {
	       size_t length;
	       
	       if (params_string[ii].str == NULL)
		    NADC_GOTO_ERROR(NADC_ERR_FATAL, "parameter not defined");

	       length = strlen(params_string[ii].str) + strlen(ext);
	       params_string[ii].str = (char *) realloc(
		    params_string[ii].str, length + 1);
	       if (params_string[ii].str == NULL)
		    NADC_GOTO_ERROR(NADC_ERR_ALLOC, "params_string[ii].str");
	       (void) strcat(params_string[ii].str, ext);
	       return 0;
	  }
     } while (++ii < nkeys);
     
     NADC_ERROR(NADC_ERR_FATAL, "unknown parameter name");
done:
     return -1;
}

/*+++++++++++++++++++++++++
.IDENTifer   nadc_get_param
.PURPOSE     return value of a given parameter
.INPUT/OUTPUT
  call as   res = nadc_get_param(param_name, size_t length, value);

	 char param_name[] :  name of parameter
	 size_t *length    :  identifier of the datatype
	 void *value       :  value of parameter

.RETURNS     zero if successful; otherwise returns a negative value
.COMMENTS    none
-------------------------*/
unsigned char nadc_get_param_uint8(const char *param_name)
{
     register size_t ii = 0;
     
     const size_t nkeys = sizeof params_uint8 / sizeof(struct param_uint8_rec);

     do {
	  if (strcmp(params_uint8[ii].name, param_name) == 0)
	       return params_uint8[ii].value;
     } while (++ii < nkeys);

     NADC_ERROR(NADC_ERR_FATAL, "unknown parameter name");
     return (unsigned char) ~0x0U;
}

unsigned short nadc_get_param_uint16(const char *param_name)
{
     register size_t ii = 0;
     
     const size_t nkeys = sizeof params_uint16 / sizeof(struct param_uint16_rec);

     do {
	  if (strcmp(params_uint16[ii].name, param_name) == 0)
	       return params_uint16[ii].value;
     } while (++ii < nkeys);

     NADC_ERROR(NADC_ERR_FATAL, "unknown parameter name");
     return (unsigned short) ~0x0U;
}

unsigned int nadc_get_param_uint32(const char *param_name)
{
     register size_t ii = 0;
     
     const size_t nkeys = sizeof params_uint32 / sizeof(struct param_uint32_rec);

     do {
	  if (strcmp(params_uint32[ii].name, param_name) == 0)
	       return params_uint32[ii].value;
     } while (++ii < nkeys);

     NADC_ERROR(NADC_ERR_FATAL, "unknown parameter name");
     return ~0x0U;
}

hid_t nadc_get_param_hid(const char *param_name)
{
     register size_t ii = 0;
     
     const size_t nkeys = sizeof params_hid / sizeof(struct param_hid_rec);

     do {
	  if (strcmp(params_hid[ii].name, param_name) == 0)
	       return params_hid[ii].value;
     } while (++ii < nkeys);

     NADC_ERROR(NADC_ERR_FATAL, "unknown parameter name");
     return -1;
}

void nadc_get_param_range(const char *param_name, float *buff)
{
     register size_t ii = 0;
     
     const size_t nkeys = sizeof params_range / sizeof(struct param_range_rec);

     do {
	  if (strcmp(params_range[ii].name, param_name) == 0) {
	       (void) memcpy(buff, params_range[ii].value, 2 * ENVI_FLOAT);
	       return;
	  }
     } while (++ii < nkeys);

     NADC_RETURN_ERROR(NADC_ERR_FATAL, "unknown parameter name");
}

char * nadc_get_param_string(const char *param_name)
{
     register size_t ii = 0;

     char *str = NULL;

     const size_t nkeys = \
	  sizeof params_string / sizeof(struct param_string_rec);

     do {
	  if (strcmp(params_string[ii].name, param_name) != 0)
	       continue;

	  if (params_string[ii].str == NULL)
	       goto done;

	  if ((str = malloc(strlen(params_string[ii].str)+1)) == NULL)
	       NADC_GOTO_ERROR(NADC_ERR_ALLOC, "str");
	  (void) strcpy(str, params_string[ii].str);
	  return str;
     } while (++ii < nkeys);

     NADC_ERROR(NADC_ERR_FATAL, "unknown parameter name");
done:
     return NULL;
}

/*--------------------------------------------------*/
static unsigned char chan_mask = (unsigned char) ~0U;

int nadc_set_param_chan(const unsigned char *chan_list, int length)
{
     register int ii = 0;

     if (length < 0) {
	  chan_mask = (unsigned char) ~0U;
	  return 0;
     }
     chan_mask = 0U;
     if (length == 0)
	  return 0;

     do {
	  Set_Bit_uc(&chan_mask, (unsigned char)(chan_list[ii]-1));
     } while (++ii < length);

     return 0;
}

bool nadc_get_param_chan(int id)
{
     return (Get_Bit_uc(chan_mask, id - 1) == (unsigned char) 1);
}

void nadc_repr_param_chan(size_t length, char *str)
{
     register int id;

     char tmp_str[5];
     
     if (chan_mask == (unsigned char) ~0U) {
	  (void) strcpy(str, "All");
	  return;
     }
     if (chan_mask == (unsigned char) 0U) {
	  (void) strcpy(str, "None");
	  return;
     }
     *str = '\0';
     for (id = 1; id <= 8; id++) {
	  if (nadc_get_param_chan(id)) {
	       if (strlen(str) > 0)
		    (void) snprintf(tmp_str, 4, ",%-d", id);
	       else
		    (void) snprintf(tmp_str, 4, "%-d", id);
	       (void) nadc_strlcat(str, tmp_str, length);
	  }
     }
}

/*--------------------------------------------------*/
static unsigned long long clus_mask = ~0ULL;

int nadc_set_param_clus(const unsigned char *clus_list, int length)
{
     register int ii = 0;

     if (length < 0) {
	  clus_mask = ~0ULL;
	  return 0;
     }
     clus_mask = 0ULL;
     if (length == 0)
	  return 0;

     do {
	  Set_Bit_LL(&clus_mask, (unsigned char)(clus_list[ii]-1));
     } while (++ii < length);

     return 0;
}

bool nadc_get_param_clus(int id)
{
     return (Get_Bit_LL(clus_mask, id - 1) == 1ULL);
}

void nadc_repr_param_clus(size_t length, char *str)
{
     register int id;
     
     char tmp_str[5];

     if (clus_mask == ~0ULL) {
	  (void) strcpy(str, "All");
	  return;
     }
     if (clus_mask == 0ULL) {
	  (void) strcpy(str, "None");
	  return;
     }
     *str = '\0';
     for (id = 1; id <= MAX_NUM_CLUS; id++) {
	  if (nadc_get_param_clus(id)) {
	       if (strlen(str) > 0)
		    (void) snprintf(tmp_str, 4, ",%-d", id);
	       else
		    (void) snprintf(tmp_str, 4, "%-d", id);
	       (void) nadc_strlcat(str, tmp_str, length);
	  }
     }
}

/*--------------------------------------------------*/
static unsigned long long cat_mask = ~0ULL;

int nadc_set_param_cat(const unsigned char *cat_list, int length)
{
     register int ii = 0;

     if (length < 0) {
	  cat_mask = ~0ULL;
	  return 0;
     }
     cat_mask = 0ULL;
     if (length == 0)
	  return 0;

     do {
	  Set_Bit_LL(&cat_mask, (unsigned char)(cat_list[ii]-1));
     } while (++ii < length);

     return 0;
}

bool nadc_get_param_cat(int id)
{
     return (Get_Bit_LL(cat_mask, id - 1) == 1ULL);
}

void nadc_repr_param_cat(size_t length, char *str)
{
     register int id;
     
     char tmp_str[5];

     if (cat_mask == ~0ULL) {
	  (void) strcpy(str, "All");
	  return;
     }
     if (cat_mask == 0ULL) {
	  (void) strcpy(str, "None");
	  return;
     }
     *str = '\0';
     for (id = 1; id <= MAX_NUM_CLUS; id++) {
	  if (nadc_get_param_cat(id)) {
	       if (strlen(str) > 0)
		    (void) snprintf(tmp_str, 4, ",%-d", id);
	       else
		    (void) snprintf(tmp_str, 4, "%-d", id);
	       (void) nadc_strlcat(str, tmp_str, length);
	  }
     }
}

/*--------------------------------------------------*/
typedef struct bitfield {
     unsigned char x0:1, x1:1, x2:1, x3:1, x4:1, x5:1,
	  x6:1, x7:1, x8:1, x9:1, xA:1, xB:1, xC:1, xD:1, xE:1, xF:1;
} Bitfield;

static struct param_state_rec {
     union {
          Bitfield field;
          unsigned short two_byte;
     } block1;
     union {
          Bitfield field;
          unsigned short two_byte;
     } block2;
     union {
          Bitfield field;
          unsigned short two_byte;
     } block3;
     union {
          Bitfield field;
          unsigned short two_byte;
     } block4;
     union {
          Bitfield field;
          unsigned short two_byte;
     } block5;
} param_state = {
     .block1.two_byte = (unsigned short) ~0U,
     .block2.two_byte = (unsigned short) ~0U,
     .block3.two_byte = (unsigned short) ~0U,
     .block4.two_byte = (unsigned short) ~0U,
     .block5.two_byte = (unsigned short) ~0U
};

int nadc_set_param_state(const unsigned char *state_list, int length)
{
     register int ii = 0;

     if (length < 0) {
	  param_state.block1.two_byte = (unsigned short) ~0U;
	  param_state.block2.two_byte = (unsigned short) ~0U;
	  param_state.block3.two_byte = (unsigned short) ~0U;
	  param_state.block4.two_byte = (unsigned short) ~0U;
	  param_state.block5.two_byte = (unsigned short) ~0U;
	  return 0;
     }
     param_state.block1.two_byte = 0U;
     param_state.block2.two_byte = 0U;
     param_state.block3.two_byte = 0U;
     param_state.block4.two_byte = 0U;
     param_state.block5.two_byte = 0U;
     if (length == 0)
	  return 0;
     
     do {
	  switch (state_list[ii]) {
	  case 1:
	       param_state.block1.field.x0 = 1;
	       break;
	  case 2:
	       param_state.block1.field.x1 = 1;
	       break;
	  case 3:
	       param_state.block1.field.x2 = 1;
	       break;
	  case 4:
	       param_state.block1.field.x3 = 1;
	       break;
	  case 5:
	       param_state.block1.field.x4 = 1;
	       break;
	  case 6:
	       param_state.block1.field.x5 = 1;
	       break;
	  case 7:
	       param_state.block1.field.x6 = 1;
	       break;
	  case 8:
	       param_state.block1.field.x7 = 1;
	       break;
	  case 9:
	       param_state.block1.field.x8 = 1;
	       break;
	  case 10:
	       param_state.block1.field.x9 = 1;
	       break;
	  case 11:
	       param_state.block1.field.xA = 1;
	       break;
	  case 12:
	       param_state.block1.field.xB = 1;
	       break;
	  case 13:
	       param_state.block1.field.xC = 1;
	       break;
	  case 14:
	       param_state.block1.field.xD = 1;
	       break;
	  case 15:
	       param_state.block1.field.xE = 1;
	       break;
	  case 16:
	       param_state.block1.field.xF = 1;
	       break;
	  case 17:
	       param_state.block2.field.x0 = 1;
	       break;
	  case 18:
	       param_state.block2.field.x1 = 1;
	       break;
	  case 19:
	       param_state.block2.field.x2 = 1;
	       break;
	  case 20:
	       param_state.block2.field.x3 = 1;
	       break;
	  case 21:
	       param_state.block2.field.x4 = 1;
	       break;
	  case 22:
	       param_state.block2.field.x5 = 1;
	       break;
	  case 23:
	       param_state.block2.field.x6 = 1;
	       break;
	  case 24:
	       param_state.block2.field.x7 = 1;
	       break;
	  case 25:
	       param_state.block2.field.x8 = 1;
	       break;
	  case 26:
	       param_state.block2.field.x9 = 1;
	       break;
	  case 27:
	       param_state.block2.field.xA = 1;
	       break;
	  case 28:
	       param_state.block2.field.xB = 1;
	       break;
	  case 29:
	       param_state.block2.field.xC = 1;
	       break;
	  case 30:
	       param_state.block2.field.xD = 1;
	       break;
	  case 31:
	       param_state.block2.field.xE = 1;
	       break;
	  case 32:
	       param_state.block2.field.xF = 1;
	       break;
	  case 33:
	       param_state.block3.field.x0 = 1;
	       break;
	  case 34:
	       param_state.block3.field.x1 = 1;
	       break;
	  case 35:
	       param_state.block3.field.x2 = 1;
	       break;
	  case 36:
	       param_state.block3.field.x3 = 1;
	       break;
	  case 37:
	       param_state.block3.field.x4 = 1;
	       break;
	  case 38:
	       param_state.block3.field.x5 = 1;
	       break;
	  case 39:
	       param_state.block3.field.x6 = 1;
	       break;
	  case 40:
	       param_state.block3.field.x7 = 1;
	       break;
	  case 41:
	       param_state.block3.field.x8 = 1;
	       break;
	  case 42:
	       param_state.block3.field.x9 = 1;
	       break;
	  case 43:
	       param_state.block3.field.xA = 1;
	       break;
	  case 44:
	       param_state.block3.field.xB = 1;
	       break;
	  case 45:
	       param_state.block3.field.xC = 1;
	       break;
	  case 46:
	       param_state.block3.field.xD = 1;
	       break;
	  case 47:
	       param_state.block3.field.xE = 1;
	       break;
	  case 48:
	       param_state.block3.field.xF = 1;
	       break;
	  case 49:
	       param_state.block4.field.x0 = 1;
	       break;
	  case 50:
	       param_state.block4.field.x1 = 1;
	       break;
	  case 51:
	       param_state.block4.field.x2 = 1;
	       break;
	  case 52:
	       param_state.block4.field.x3 = 1;
	       break;
	  case 53:
	       param_state.block4.field.x4 = 1;
	       break;
	  case 54:
	       param_state.block4.field.x5 = 1;
	       break;
	  case 55:
	       param_state.block4.field.x6 = 1;
	       break;
	  case 56:
	       param_state.block4.field.x7 = 1;
	       break;
	  case 57:
	       param_state.block4.field.x8 = 1;
	       break;
	  case 58:
	       param_state.block4.field.x9 = 1;
	       break;
	  case 59:
	       param_state.block4.field.xA = 1;
	       break;
	  case 60:
	       param_state.block4.field.xB = 1;
	       break;
	  case 61:
	       param_state.block4.field.xC = 1;
	       break;
	  case 62:
	       param_state.block4.field.xD = 1;
	       break;
	  case 63:
	       param_state.block4.field.xE = 1;
	       break;
	  case 64:
	       param_state.block4.field.xF = 1;
	       break;
	  case 65:
	       param_state.block5.field.x0 = 1;
	       break;
	  case 66:
	       param_state.block5.field.x1 = 1;
	       break;
	  case 67:
	       param_state.block5.field.x2 = 1;
	       break;
	  case 68:
	       param_state.block5.field.x3 = 1;
	       break;
	  case 69:
	       param_state.block5.field.x4 = 1;
	       break;
	  case 70:
	       param_state.block5.field.x5 = 1;
	       break;
	  default:
	       NADC_ERROR(NADC_ERR_FATAL, "state ID out-of-range");
	       return -1;
	  }
     } while (++ii < length);
     
     return 0;
}

bool nadc_get_param_state(int id)
{
     unsigned char res = 0;

     switch (id) {
     case 1:
	  res = param_state.block1.field.x0;
	  break;
     case 2:
	  res = param_state.block1.field.x1;
	  break;
     case 3:
	  res = param_state.block1.field.x2;
	  break;
     case 4:
	  res = param_state.block1.field.x3;
	  break;
     case 5:
	  res = param_state.block1.field.x4;
	  break;
     case 6:
	  res = param_state.block1.field.x5;
	  break;
     case 7:
	  res = param_state.block1.field.x6;
	  break;
     case 8:
	  res = param_state.block1.field.x7;
	  break;
     case 9:
	  res = param_state.block1.field.x8;
	  break;
     case 10:
	  res = param_state.block1.field.x9;
	  break;
     case 11:
	  res = param_state.block1.field.xA;
	  break;
     case 12:
	  res = param_state.block1.field.xB;
	  break;
     case 13:
	  res = param_state.block1.field.xC;
	  break;
     case 14:
	  res = param_state.block1.field.xD;
	  break;
     case 15:
	  res = param_state.block1.field.xE;
	  break;
     case 16:
	  res = param_state.block1.field.xF;
	  break;
     case 17:
	  res = param_state.block2.field.x0;
	  break;
     case 18:
	  res = param_state.block2.field.x1;
	  break;
     case 19:
	  res = param_state.block2.field.x2;
	  break;
     case 20:
	  res = param_state.block2.field.x3;
	  break;
     case 21:
	  res = param_state.block2.field.x4;
	  break;
     case 22:
	  res = param_state.block2.field.x5;
	  break;
     case 23:
	  res = param_state.block2.field.x6;
	  break;
     case 24:
	  res = param_state.block2.field.x7;
	  break;
     case 25:
	  res = param_state.block2.field.x8;
	  break;
     case 26:
	  res = param_state.block2.field.x9;
	  break;
     case 27:
	  res = param_state.block2.field.xA;
	  break;
     case 28:
	  res = param_state.block2.field.xB;
	  break;
     case 29:
	  res = param_state.block2.field.xC;
	  break;
     case 30:
	  res = param_state.block2.field.xD;
	  break;
     case 31:
	  res = param_state.block2.field.xE;
	  break;
     case 32:
	  res = param_state.block2.field.xF;
	  break;
     case 33:
	  res = param_state.block3.field.x0;
	  break;
     case 34:
	  res = param_state.block3.field.x1;
	  break;
     case 35:
	  res = param_state.block3.field.x2;
	  break;
     case 36:
	  res = param_state.block3.field.x3;
	  break;
     case 37:
	  res = param_state.block3.field.x4;
	  break;
     case 38:
	  res = param_state.block3.field.x5;
	  break;
     case 39:
	  res = param_state.block3.field.x6;
	  break;
     case 40:
	  res = param_state.block3.field.x7;
	  break;
     case 41:
	  res = param_state.block3.field.x8;
	  break;
     case 42:
	  res = param_state.block3.field.x9;
	  break;
     case 43:
	  res = param_state.block3.field.xA;
	  break;
     case 44:
	  res = param_state.block3.field.xB;
	  break;
     case 45:
	  res = param_state.block3.field.xC;
	  break;
     case 46:
	  res = param_state.block3.field.xD;
	  break;
     case 47:
	  res = param_state.block3.field.xE;
	  break;
     case 48:
	  res = param_state.block3.field.xF;
	  break;
     case 49:
	  res = param_state.block4.field.x0;
	  break;
     case 50:
	  res = param_state.block4.field.x1;
	  break;
     case 51:
	  res = param_state.block4.field.x2;
	  break;
     case 52:
	  res = param_state.block4.field.x3;
	  break;
     case 53:
	  res = param_state.block4.field.x4;
	  break;
     case 54:
	  res = param_state.block4.field.x5;
	  break;
     case 55:
	  res = param_state.block4.field.x6;
	  break;
     case 56:
	  res = param_state.block4.field.x7;
	  break;
     case 57:
	  res = param_state.block4.field.x8;
	  break;
     case 58:
	  res = param_state.block4.field.x9;
	  break;
     case 59:
	  res = param_state.block4.field.xA;
	  break;
     case 60:
	  res = param_state.block4.field.xB;
	  break;
     case 61:
	  res = param_state.block4.field.xC;
	  break;
     case 62:
	  res = param_state.block4.field.xD;
	  break;
     case 63:
	  res = param_state.block4.field.xE;
	  break;
     case 64:
	  res = param_state.block4.field.xF;
	  break;
     case 65:
	  res = param_state.block5.field.x0;
	  break;
     case 66:
	  res = param_state.block5.field.x1;
	  break;
     case 67:
	  res = param_state.block5.field.x2;
	  break;
     case 68:
	  res = param_state.block5.field.x3;
	  break;
     case 69:
	  res = param_state.block5.field.x4;
	  break;
     case 70:
	  res = param_state.block5.field.x5;
	  break;
     }
     return (res == (unsigned short) 1);
}

void nadc_repr_param_state(size_t length, char *str)
{
     register int id;

     char tmp_str[5];
     
     if (param_state.block1.two_byte == (unsigned short) ~0U
	 && param_state.block2.two_byte== (unsigned short) ~0U
	 && param_state.block3.two_byte == (unsigned short) ~0U
	 && param_state.block4.two_byte == (unsigned short) ~0U
	 && param_state.block5.two_byte == (unsigned short) ~0U) {
	  (void) strcpy(str, "All");
	  return;
     }
     if (param_state.block1.two_byte == (unsigned short) 0U
	 && param_state.block2.two_byte == (unsigned short) 0U
	 && param_state.block3.two_byte == (unsigned short) 0U
	 && param_state.block4.two_byte == (unsigned short) 0U
	 && param_state.block5.two_byte == (unsigned short) 0U) {
	  (void) strcpy(str, "None");
	  return;
     }
     *str = '\0';
     for (id = 1; id <= MAX_NUM_STATE; id++) {
	  if (nadc_get_param_state(id)) {
	       if (strlen(str) > 0)
		    (void) snprintf(tmp_str, 4, ",%-d", id);
	       else
		    (void) snprintf(tmp_str, 4, "%-d", id);
	       (void) nadc_strlcat(str, tmp_str, length);
	  }
     }
}

void nadc_free_param_string(void)
{
     register size_t ii = 0;

     const size_t nkeys = \
	  sizeof params_string / sizeof(struct param_string_rec);

     do {
	  if (params_string[ii].str != NULL)
	       free(params_string[ii].str);
     } while (++ii < nkeys);
}   
