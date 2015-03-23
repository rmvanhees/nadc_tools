set (FRESCO_SRCS nadc_rd_fresco.c)
set (FRESCO_LIBS nadc)

if (NETCDF_FOUND)
   add_definitions (-D_WITH_NC4)

   list (APPEND FRESCO_SRCS
        nadc_rd_nc_fresco.c
        nadc_wr_nc_fresco.c
	)
   list (APPEND FRESCO_LIBS ${NETCDF_LIBRARIES})
endif ()
list (APPEND FRESCO_LIBS ${HDF5_C_LIBRARIES})

set (FRESCO_SQL_SRCS "")
if (PGSQL_FOUND)
   add_definitions (-D_WITH_SQL)

   set (FRESCO_SQL_SRCS
   	nadc_fresco_del_entry.c
	nadc_fresco_wr_sql_meta.c
	nadc_fresco_wr_sql_tile.c
	)
endif ()

add_executable(nadc_fresco nadc_fresco.c ${FRESCO_SRCS} ${FRESCO_SQL_SRCS})
if (PGSQL_FOUND)
   target_link_libraries(nadc_fresco ${FRESCO_LIBS} ${PGSQL_LIBRARY})
else ()
   target_link_libraries(nadc_fresco ${FRESCO_LIBS})
endif ()

if (NETCDF_FOUND)
   add_executable(adaguc_fresco adaguc_fresco.c ${FRESCO_SRCS})

   target_link_libraries(adaguc_fresco ${FRESCO_LIBS})
endif ()

## Installation instructions
if (NETCDF_FOUND)
install (
  TARGETS adaguc_fresco nadc_fresco
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
else ()
install (
  TARGETS nadc_fresco
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
endif ()
