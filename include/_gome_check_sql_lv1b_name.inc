/*++++++++++++++++++++++++
.IDENTifer   GOME_CHECK_SQL_LV1B_NAME
.PURPOSE     check if GOME level 1b product is stored in database
.INPUT/OUTPUT
  call as   GOME_CHECK_SQL_LV1B_NAME( conn, l1b_product );
     input:  
             PGconn *conn      :  PostgreSQL connection handle
	     char *l1b_product :  name of GOME level 1b products

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
------------------------*/
#ifdef LIBPQ_FE_H

#define SELECT_PRODUCT \
"SELECT name FROM meta__1P WHERE dateTimeStart <= \'%19s\' \
 AND dateTimeStop >= \'%19s\' ORDER BY softVersion DESC"

static
void GOME_CHECK_SQL_LV1B_NAME( PGconn *conn, const char *l1b_product_in,
			       const char *datetime_start, 
			       const char *datetime_stop,
			       /*@out@*/ char *l1b_product )
{
     PGresult *res;

     int   nrow;
     char  sql_query[SQL_STR_SIZE];
/*
 * initialize return value with original name of level 1b product
 */
     (void) nadc_strlcpy( l1b_product, l1b_product_in, ERS2_FILENAME_SIZE );
/*
 * check for presence in database
 */
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "SELECT pk_meta FROM meta__1P WHERE name=\'%s\'", 
		      l1b_product );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     if ( (nrow = PQntuples( res )) != 0 ) return;
     PQclear( res );

     (void) snprintf( sql_query, SQL_STR_SIZE, SELECT_PRODUCT,
		      datetime_start, datetime_stop );
/*      (void) fprintf( stderr, "%s\n", sql_query ); */
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     if ( (nrow = PQntuples( res )) == 0 ) goto done;
     (void) nadc_strlcpy( l1b_product, 
                          PQgetvalue( res, 0, 0 ), ERS2_FILENAME_SIZE );
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "changed input L1b product to: %s", l1b_product );
     NADC_ERROR( NADC_ERR_NONE, sql_query );
 done:
     PQclear( res );
}
#endif /* LIBPQ_FE_H */
