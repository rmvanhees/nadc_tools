add_definitions (-D_WITH_NC4)

set (IMLM_CO_SRCS
    scia_rd_imlm.c
    scia_rd_nc_co.c
    scia_wr_nc_co.c
)

set (IMLM_H2O_SRCS
    scia_rd_imlm.c
    scia_rd_nc_h2o.c
    scia_wr_nc_h2o.c
)
set (IMLM_LIBS nadc ${NETCDF_LIBRARIES} ${HDF5_C_LIBRARIES})

set (IMLM_CO_SQL_SRCS "")
set (IMLM_H2O_SQL_SRCS "")
if (PGSQL_FOUND)
   add_definitions (-D_WITH_SQL)

   set (IMLM_CO_SQL_SRCS
   	scia_del_entry_imlm_co.c
	scia_wr_sql_co_meta.c
	scia_wr_sql_co_tile.c
   )
   set (IMLM_H2O_SQL_SRCS
   	scia_del_entry_imlm_h2o.c
	scia_wr_sql_h2o_meta.c
	scia_wr_sql_h2o_tile.c
   )
   list (APPEND IMLM_LIBS ${PGSQL_LIBRARY})
endif ()

add_executable(scia_imlm_co scia_imlm_co.c 
			     ${IMLM_CO_SRCS} ${IMLM_CO_SQL_SRCS})

target_link_libraries(scia_imlm_co ${IMLM_LIBS} -lm)

add_executable(scia_imlm_h2o scia_imlm_h2o.c 
			     ${IMLM_H2O_SRCS} ${IMLM_H2O_SQL_SRCS})

target_link_libraries(scia_imlm_h2o ${IMLM_LIBS} -lm)

add_executable(adaguc_imlm_co adaguc_imlm_co.c ${IMLM_CO_SRCS})

target_link_libraries(adaguc_imlm_co ${IMLM_LIBS})

add_executable(adaguc_imlm_h2o adaguc_imlm_h2o.c ${IMLM_H2O_SRCS})

target_link_libraries(adaguc_imlm_h2o ${IMLM_LIBS})

## Installation instructions
install (
  TARGETS adaguc_imlm_co adaguc_imlm_h2o scia_imlm_co scia_imlm_h2o
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
