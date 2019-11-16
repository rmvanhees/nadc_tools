/*++++++++++++++++++++++++
.IDENTifer   CONNECT_NADC_DB
.PURPOSE     connect to meta/pixel database (reading XML configuration file)
.INPUT/OUTPUT
  call as   CONNECT_NADC_DB(conn, database);
     input:  
             PGconn *conn   :  PostgreSQL connection handle
	     char *database :  name of database to be used

.RETURNS     Nothing, error status passed by global variable ``nadc_stat''
------------------------*/
#ifdef LIBPQ_FE_H
static
void CONNECT_NADC_DB(PGconn **conn, const char *database)
       /*@globals  nadc_stat, nadc_err_stack;@*/
       /*@modifies nadc_stat, nadc_err_stack, conn@*/
{
     size_t res;
     
     char conninfo[MAX_STRING_LENGTH];

     char xml_db_host[SHORT_STRING_LENGTH];
     char xml_db_user[SHORT_STRING_LENGTH];
     char xml_db_pass[SHORT_STRING_LENGTH];
     char xml_db_port[SHORT_STRING_LENGTH];

     char *user_home = getenv("HOME");
     char *db_host = getenv("NADC_DB_HOST");
     char *db_user = getenv("NADC_DB_USER");
     char *db_pass = getenv("NADC_DB_PASSWD");
     char *db_port = getenv("NADC_DB_PORT");
/*
 * obtain host, user and password from NADC XML configuration file
 */
     *conn = NULL;
     if (db_host == NULL || db_user == NULL) {
	  FILE *fp = NULL;

	  char config_file[MAX_STRING_LENGTH];

	  (void) strcpy(xml_db_host, "");
	  (void) strcpy(xml_db_user, "");
	  (void) strcpy(xml_db_pass, "");
	  (void) strcpy(xml_db_port, "");

	  (void) strcpy(config_file, "./nadc.config.xml");
	  if ((fp = fopen(config_file, "r")) == NULL) {
	       res = snprintf(config_file, MAX_STRING_LENGTH,
			      "%s/nadc.config.xml", user_home);
	       if (res > MAX_STRING_LENGTH)
		    NADC_ERROR(NADC_ERR_WARN, "config_file truncated");
	       if ((fp = fopen(config_file, "r")) == NULL)
		    NADC_RETURN_ERROR(NADC_ERR_FILE, config_file);
	  }
	  NADC_GET_XML_METADB(fp, xml_db_host, xml_db_port, 
			       xml_db_user, xml_db_pass);
	  if (db_host == NULL && strlen(xml_db_host) > 0) 
	       db_host = xml_db_host;
	  if (db_user == NULL && strlen(xml_db_user) > 0) 
	       db_user = xml_db_user;
	  if (db_pass == NULL && strlen(xml_db_pass) > 0) 
	       db_pass = xml_db_pass;
	  if (db_port == NULL && strlen(xml_db_port) > 0) 
	       db_port = xml_db_port;

	  (void) fclose(fp);
     }
/*
 * make connection to the database
 */
     if (db_pass == NULL && db_port == NULL)
	  res = snprintf(conninfo, MAX_STRING_LENGTH,
			 "host=%s user=%s dbname=%s", 
			 db_host, db_user, database);
     else if (db_port == NULL)
	  res = snprintf(conninfo, MAX_STRING_LENGTH,
			 "host=%s user=%s password=%s dbname=%s", 
			 db_host, db_user, db_pass, database);
     else if (db_pass == NULL)
	  res = snprintf(conninfo, MAX_STRING_LENGTH,
			 "host=%s port=%s user=%s dbname=%s", 
			 db_host, db_port, db_user, database);
     else
	  res = snprintf(conninfo, MAX_STRING_LENGTH,
			 "host=%s port=%s user=%s password=%s dbname=%s", 
			 db_host, db_port, db_user, db_pass, database);
     if (res > MAX_STRING_LENGTH)
	  NADC_ERROR(NADC_ERR_WARN, "conninfo truncated");

     *conn = PQconnectdb(conninfo);
     if (PQstatus(*conn) != CONNECTION_OK) 
	  NADC_RETURN_ERROR(NADC_ERR_SQL, PQerrorMessage(*conn));
}
#endif /* LIBPQ_FE_H */
