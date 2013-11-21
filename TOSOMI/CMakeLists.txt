add_definitions (-D_WITH_NC4)
set (TOSOMI_SRCS
    nadc_rd_tosomi.c
    nadc_rd_nc_tosomi.c
    nadc_wr_nc_tosomi.c
)
set (TOSOMI_LIBS nadc ${NETCDF_LIBRARIES} ${HDF5_C_LIBRARIES})

set (TOSOMI_SQL_SRCS "")
if (PGSQL_FOUND)
   add_definitions (-D_WITH_SQL)

   set (TOSOMI_SQL_SRCS
   	nadc_tosomi_del_entry.c
	nadc_tosomi_wr_sql_meta.c
	nadc_tosomi_wr_sql_tile.c
   )
   list (APPEND IMAP_LIBS ${PGSQL_LIBRARY})
endif ()

add_executable(nadc_tosomi nadc_tosomi.c ${TOSOMI_SRCS} ${TOSOMI_SQL_SRCS})
if (PGSQL_FOUND)
   target_link_libraries(nadc_tosomi ${TOSOMI_LIBS} ${PGSQL_LIBRARY})
else ()
   target_link_libraries(nadc_tosomi ${TOSOMI_LIBS})
endif ()

add_executable(adaguc_tosomi adaguc_tosomi.c  ${TOSOMI_SRCS})

target_link_libraries(adaguc_tosomi ${TOSOMI_LIBS})

## Installation instructions
install (
  TARGETS adaguc_tosomi nadc_tosomi
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
