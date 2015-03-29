#if defined(_NETCDF_)
/*+++++++++++++++++++++++++
.IDENTifer   ADAGUC_DEF_VAR
.PURPOSE     define a netCDF variable in ADAGUC format
.INPUT/OUTPUT
  call as   retval = ADAGUC_DEF_VAR( ncid, xtype, dimids, name, unit, 
                                     long_name, standard, ancillary )
     input:
            int ncid        :   netCDF file ID
            nc_type xtype   :   netCDF variable type
            int dimids      :   dimension IDs
            char *name      :   short name for variable
            char *unit      :   unit of variable (NULL if not defined)
            char *long_name :   long name (required!)
	    char *standard  :   standard name (NULL if not defined)
	    char *ancillary :   ancillary variables (NULL if not defined)

.RETURNS     Nothing
.COMMENTS    None
-------------------------*/
static
int ADAGUC_DEF_VAR( int ncid, nc_type xtype, int dimids,
		    const char *name, const char *unit, 
		    const char *long_name, const char *standard,
		    const char *ancillary )
{
     int retval;
     int var_id;

     retval = nc_def_var( ncid, name, xtype, 1, &dimids, &var_id );
     if ( retval != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_put_att_text( ncid, var_id, "long_name",
                               strlen(long_name), long_name );
     if ( retval != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_put_att_text( ncid, var_id, "standard_name",
                               strlen(long_name), long_name );
     if ( retval != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     retval = nc_put_att_text( ncid, var_id, "coordinates", 7, "lon lat" );
     if ( retval != NC_NOERR )
          NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     if ( unit != NULL ) {
          retval = nc_put_att_text( ncid, var_id, "units",
                                    strlen(unit), unit );
          if ( retval != NC_NOERR )
               NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     }
     if ( standard != NULL ) {
	  retval = nc_put_att_text( ncid, var_id, "standard_name",
                                    strlen(standard), standard );
          if ( retval != NC_NOERR )
               NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     }
     if ( ancillary != NULL ) {
	  retval = nc_put_att_text( ncid, var_id, "ancillary_variables",
                                    strlen(ancillary), ancillary );
          if ( retval != NC_NOERR )
               NADC_GOTO_ERROR( NADC_ERR_FATAL, nc_strerror(retval) );
     }
     return var_id;
 done:
     return -1;
}
#endif
