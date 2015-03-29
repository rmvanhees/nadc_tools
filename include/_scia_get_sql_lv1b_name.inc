/*+++++++++++++++++++++++++
.IDENTifer   SCIA_GET_SQL_LV1_NAME
.AUTHOR      R.M. van Hees
.KEYWORDS    SCIA SQL
.LANGUAGE    ANSI C
.PURPOSE     obtain Sciamachy level 1b product name from database
.INPUT/OUTPUT
  call as   SCIA_GET_SQL_LV1B_NAME( absOrbit, fileCounter, l1b_product );
     input:  
             unsigned int absOrbit      :   absoluut orbit number
             unsigned short fileCounter :   file counter
    output:  
             char *l1b_product          :   Sciamachy level 1b product name

.RETURNS     Nothing
------------------------*/
#ifdef LIBPQ_FE_H
#define SQL_STR_SIZE   128

#define SQL_GET_L1B_NAME \
"SELECT name FROM meta__1P WHERE name LIKE '%%_%05u_%04hu.N1' order by procStage DESC"

#define SQL_GET_L1B_NAME2 \
"SELECT name FROM meta__1P WHERE name ~ 'SCI_NL__1P[RUW].{37}_%05u_.{4}.N1'"

/*+++++++++++++++++++++++++ Static Functions +++++++++++++++++++++++*/
#include <_connect_nadc_db.inc>

static
void SCIA_GET_SQL_LV1B_NAME( unsigned int absOrbit, unsigned short fileCounter,
			    /*@out@*/ char *l1b_product )
{
     PGconn *conn;
     PGresult *res;

     int   nrow;

     char  sql_query[SQL_STR_SIZE];

     (void) strcpy( l1b_product, "unknown" );
/*
 * connect to PostgreSQL database
 */
     CONNECT_NADC_DB( &conn, "scia" );
     if ( IS_ERR_STAT_FATAL )
	  NADC_RETURN_ERROR( NADC_ERR_SQL, "failed to connect" );

     (void) snprintf( sql_query, SQL_STR_SIZE, SQL_GET_L1B_NAME,
		      absOrbit, fileCounter );
     res = PQexec( conn, sql_query );
     if ( PQresultStatus( res ) != PGRES_TUPLES_OK ) {
	  NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );
     }
     if ( (nrow = PQntuples( res )) != 0 ) {
	  (void) nadc_strlcpy( l1b_product, PQgetvalue(res,0,0), 
		 	       ENVI_FILENAME_SIZE );
     } else {
	  PQclear( res );
	  (void) snprintf( sql_query, SQL_STR_SIZE, 
			   SQL_GET_L1B_NAME2, absOrbit );
	  res = PQexec( conn, sql_query );
	  if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
	       NADC_GOTO_ERROR( NADC_ERR_SQL, PQresultErrorMessage(res) );

	  if ( (nrow = PQntuples( res )) != 0 )
	       (void) nadc_strlcpy( l1b_product, PQgetvalue(res,0,0), 
			            ENVI_FILENAME_SIZE );
     }
 done:
     PQclear( res );
     PQfinish( conn );
}
#endif /* LIBPQ_FE_H */
