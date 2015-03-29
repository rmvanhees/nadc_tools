/*++++++++++++++++++++++++
.IDENTifer   SCIA_CHECK_SQL_LV1B_NAME
.PURPOSE     check if SCIA level 1b product is stored in database
.INPUT/OUTPUT
  call as   SCIA_CHECK_SQL_LV1B_NAME( conn, l1b_product );
     input:  
             PGconn *conn      :  PostgreSQL connection handle
	     char *l1b_product :  name of Sciamachy level 1b product

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
------------------------*/
#ifdef LIBPQ_FE_H
static
void SCIA_CHECK_SQL_LV1B_NAME( PGconn *conn, char *l1b_product )
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, conn@*/
{
     PGresult *res;

     int   nrow;
     char  cbuff[ENVI_FILENAME_SIZE];
     char  sql_query[SQL_STR_SIZE];
/*
 * Check if the creator has given the L1c product name
 */
     if ( l1b_product[6] != '_' ) l1b_product[6] = '_';
     if ( l1b_product[9] != 'P' ) l1b_product[9] = 'P';
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
/*
 * check for presence with different magic number
 */
     (void) strcpy( cbuff, l1b_product );
     cbuff[55] = '%'; cbuff[56] = '\0';
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "SELECT name FROM meta__1P WHERE name LIKE \'%s\' %s", 
		      cbuff, "ORDER BY procStage,softVersion DESC" );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
          NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     if ( (nrow = PQntuples( res )) == 0 ) goto done;
     (void) nadc_strlcpy( l1b_product, 
                          PQgetvalue( res, 0, 0 ), ENVI_FILENAME_SIZE );
     (void) snprintf( sql_query, SQL_STR_SIZE, 
		      "changed input L1b product to: %s", l1b_product );
     NADC_ERROR( NADC_ERR_NONE, sql_query );
 done:
     PQclear( res );
}
#endif /* LIBPQ_FE_H */
