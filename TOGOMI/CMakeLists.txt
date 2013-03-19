set (TOGOMI_SRCS nadc_rd_togomi.c)
set (TOGOMI_LIBS nadc)

if (NETCDF_FOUND)
   add_definitions (-D_WITH_NC4)

   list (APPEND TOGOMI_SRCS
        nadc_rd_nc_togomi.c
        nadc_wr_nc_togomi.c
   )
   list (APPEND TOGOMI_LIBS ${NETCDF_LIBRARIES})
endif ()
list (APPEND TOGOMI_LIBS ${HDF5_C_LIBRARIES})

set (TOGOMI_SQL_SRCS "")
if (PGSQL_FOUND)
   add_definitions (-D_WITH_SQL)

   set (TOGOMI_SQL_SRCS
   	nadc_togomi_del_entry.c
	nadc_togomi_wr_sql_meta.c
	nadc_togomi_wr_sql_tile.c
   )
   list (APPEND IMAP_LIBS ${PGSQL_LIBRARY})
endif ()

add_executable(nadc_togomi nadc_togomi.c ${TOGOMI_SRCS} ${TOGOMI_SQL_SRCS})
if (PGSQL_FOUND)
   target_link_libraries(nadc_togomi ${TOGOMI_LIBS} ${PGSQL_LIBRARY})
else ()
   target_link_libraries(nadc_togomi ${TOGOMI_LIBS})
endif ()

if (NETCDF_FOUND)
   add_executable(adaguc_togomi adaguc_togomi.c  ${TOGOMI_SRCS})

   target_link_libraries(adaguc_togomi ${TOGOMI_LIBS})
endif ()

## Installation instructions
install (
  TARGETS adaguc_togomi nadc_togomi
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
