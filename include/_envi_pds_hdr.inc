#ifndef __PDS_RD_ONLY
static
void NADC_WR_PDS_ITEM( NADC_pds_hdr_t *pds_item, const void *buff )
{
     char format[SHORT_STRING_LENGTH];

     switch ( pds_item->type ) {
     case PDS_String: 
	  (void) snprintf( format, SHORT_STRING_LENGTH, 
			   "\"%%%-us\"", pds_item->length-1 );
	  (void) snprintf( pds_item->value, PDS_KEYVAL_LENGTH,
			   format, (const char *) buff );
	  break;
     case PDS_Plain: 
	  (void) snprintf( format, SHORT_STRING_LENGTH, 
			   "%%%-us", pds_item->length );
	  (void) snprintf( pds_item->value, PDS_KEYVAL_LENGTH,
			   format, (const char *) buff );
	  break;
     case PDS_Short: 
	  if ( pds_item->unit != NULL )
	       (void) snprintf( format, SHORT_STRING_LENGTH, "%%+0%-uhd%s", 
				pds_item->length, pds_item->unit );
	  else
	       (void) snprintf( format, SHORT_STRING_LENGTH, 
				"%%+0%-uhd", pds_item->length );
	  (void) snprintf( pds_item->value, PDS_KEYVAL_LENGTH,
			  format, *((const short *) buff) );
	  break;
     case PDS_uShort: 
	  if ( pds_item->unit != NULL )
	       (void) snprintf( format, SHORT_STRING_LENGTH, "+%%0%-uhu%s", 
				pds_item->length-1, pds_item->unit );
	  else
	       (void) snprintf( format, SHORT_STRING_LENGTH, 
				"+%%0%-uhu", pds_item->length-1 );
	  (void) snprintf( pds_item->value, PDS_KEYVAL_LENGTH,
			  format, *((const unsigned short *) buff) );
	  break;
     case PDS_Long: 
	  if ( pds_item->unit != NULL )
	       (void) snprintf( format, SHORT_STRING_LENGTH, "%%+0%-ud%s", 
				pds_item->length, pds_item->unit );
	  else
	       (void) snprintf( format, SHORT_STRING_LENGTH, 
				"%%+0%-ud", pds_item->length );
	  (void) snprintf( pds_item->value, PDS_KEYVAL_LENGTH,
			  format, *((const int *) buff) );
	  break;
     case PDS_uLong: 
	  if ( pds_item->unit != NULL )
	       (void) snprintf( format, SHORT_STRING_LENGTH, "+%%0%-uu%s", 
				pds_item->length-1, pds_item->unit );
	  else
	       (void) snprintf( format, SHORT_STRING_LENGTH, 
				"+%%0%-uu", pds_item->length-1 );
	  (void) snprintf( pds_item->value, PDS_KEYVAL_LENGTH,
			  format, *((const unsigned int *) buff) );
	  break;
     case PDS_Ado06: 
	  if ( pds_item->unit != NULL )
	       (void) snprintf( format, SHORT_STRING_LENGTH, "%%+8.6f%s", 
				pds_item->unit );
	  else
	       (void) snprintf( format, SHORT_STRING_LENGTH, "%%+8.6f" );
	  (void) snprintf( pds_item->value, PDS_KEYVAL_LENGTH,
			  format, *((const double *) buff) );
	  if ( pds_item->value[1] == '0' ) {
	       size_t nbyte = strlen( pds_item->value );
	       (void) memmove( pds_item->value+1, pds_item->value+2, nbyte-2 );
	       pds_item->value[nbyte-1] = '\0';
	  }
	  break;
     case PDS_Ado46: 
	  if ( pds_item->unit != NULL )
	       (void) snprintf( format, SHORT_STRING_LENGTH, "%%+012.6f%s", 
				pds_item->unit );
	  else
	       (void) snprintf( format, SHORT_STRING_LENGTH, "%%+012.6f" );
	  (void) snprintf( pds_item->value, PDS_KEYVAL_LENGTH,
			  format, *((const double *) buff) );
	  break;
     case PDS_Ado73: 
	  if ( pds_item->unit != NULL )
	       (void) snprintf( format, SHORT_STRING_LENGTH, "%%+012.3f%s", 
				pds_item->unit );
	  else
	       (void) snprintf( format, SHORT_STRING_LENGTH, "%%+012.3f" );
	  (void) snprintf( pds_item->value, PDS_KEYVAL_LENGTH,
			  format, *((const double *) buff) );
	  break;
     case PDS_Ado18e:
	  if ( pds_item->unit != NULL )
	       (void) snprintf( format, SHORT_STRING_LENGTH, "%%+15.8E%s", 
				pds_item->unit );
	  else
	       (void) snprintf( format, SHORT_STRING_LENGTH, "%%+15.8E" );
	  (void) snprintf( pds_item->value, PDS_KEYVAL_LENGTH,
			  format, *((const double *) buff) );
	  break;
     }
}

static
unsigned int NADC_WR_PDS_HDR( unsigned int num_items,
			      const NADC_pds_hdr_t *pds_items, FILE *fd )
       /*@modifies fd@*/
{
     register unsigned int ni;

     unsigned int nbyte = 0u;

     nbyte = 0u;
     for ( ni = 0; ni < num_items; ni++ ) {
	  if ( pds_items[ni].type == PDS_Spare ) {
	       char cbuff[PDS_KEYVAL_LENGTH];
	       size_t  nj, ibuff;
/*
 * if keyword > 0 then add one for the equal sign
 */
	       ibuff = strlen( pds_items[ni].key );
	       ibuff += (ibuff > 0 ? 1 : 0) + pds_items[ni].length;

	       strcpy( cbuff, "" );
	       for ( nj = 0; nj < ibuff; nj++ ) strcat( cbuff, " " );

	       nbyte += (unsigned int) fprintf( fd, "%s\n", cbuff );
	  } else {
	       nbyte += (unsigned int)
		    fprintf( fd, "%s=%s\n", pds_items[ni].key,
			     pds_items[ni].value );
	  }
     }
     return nbyte;
}
#endif

static
unsigned int NADC_RD_PDS_HDR( FILE *fd, unsigned int num_items,
			      /*@partial@*/ NADC_pds_hdr_t *pds_items )
       /*@globals  errno, nadc_stat, nadc_err_stack;@*/
       /*@modifies errno, nadc_stat, nadc_err_stack, fd@*/
       /*@modifies pds_items->values@*/
{
     register unsigned int ni;

     unsigned int nbyte = 0u;

     char   keyword[PDS_KEYWORD_LENGTH], keyvalue[PDS_KEYVAL_LENGTH];

     const char err_rd_pds[] = "error reading PDS header";

     for ( ni = 0; ni < num_items; ni++ ) {
	  if ( pds_items[ni].type == PDS_Spare ) {
	       nbyte += pds_items[ni].length + 1;    /* add one for newline */
	       if ( strlen( pds_items[ni].key ) > 0 )
		    nbyte += (unsigned int) strlen( pds_items[ni].key ) + 1;
	  } else if ( pds_items[ni].type == PDS_Optional ) {
	       nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
	       if ( IS_ERR_STAT_FATAL )
		    NADC_GOTO_ERROR( NADC_ERR_FILE, err_rd_pds );
	       if ( strcmp( keyword, pds_items[ni].key ) == 0 ) {
		    if ( strncmp( "INIT_VERSION", keyword, 12 ) == 0 ) {
			 pds_items[ni].type = PDS_Plain;
		    } else {
			 pds_items[ni].type = PDS_String;
		    }
	       } else {
		    if ( strncmp(pds_items[ni].key,"NAD_FIT_WINDOW",14) != 0 ) {
			 pds_items[ni].type = PDS_Spare;
			 nbyte += pds_items[ni].length + 1;
			 if ( strlen( pds_items[ni].key ) > 0 )
			      nbyte += 
				   (unsigned int) strlen( pds_items[ni].key )+1;
		    }
		    while ( ++ni < num_items
			    && strcmp( keyword, pds_items[ni].key ) != 0 );
		    if ( ni == num_items )
			 NADC_GOTO_ERROR( NADC_ERR_PDS_KEY, keyword );
	       }
	       (void) nadc_strlcpy( pds_items[ni].value, 
			   	    keyvalue, PDS_KEYVAL_LENGTH );
	  } else {
	       nbyte += ENVI_RD_PDS_INFO( fd, keyword, keyvalue );
	       if ( IS_ERR_STAT_FATAL ) 
		    NADC_GOTO_ERROR( NADC_ERR_FILE, err_rd_pds );
	       if ( strcmp( keyword, pds_items[ni].key ) != 0 )
		    NADC_GOTO_ERROR( NADC_ERR_PDS_KEY, keyword );
	       (void) nadc_strlcpy( pds_items[ni].value, 
			            keyvalue, PDS_KEYVAL_LENGTH );
	  }
     }
done:
     return nbyte;
}
