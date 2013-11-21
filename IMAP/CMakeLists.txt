add_definitions (-D_WITH_NC4)

set (IMAP_CH4_SRCS 
    scia_rd_imap_ch4.c
    scia_rd_nc_ch4.c
    scia_wr_nc_ch4.c
)
set (IMAP_HDO_SRCS 
    scia_rd_imap_hdo.c
    scia_rd_nc_hdo.c
    scia_wr_nc_hdo.c
)
set (IMAP_LIBS nadc ${NETCDF_LIBRARIES} ${HDF5_C_LIBRARIES})

set (IMAP_CH4_SQL_SRCS "")
set (IMAP_HDO_SQL_SRCS "")
if (PGSQL_FOUND)
   add_definitions (-D_WITH_SQL)

   set (IMAP_CH4_SQL_SRCS
   	scia_del_entry_imap_ch4.c
	scia_wr_sql_ch4_meta.c
	scia_wr_sql_ch4_tile.c
   )
   set (IMAP_HDO_SQL_SRCS
   	scia_del_entry_imap_hdo.c
	scia_wr_sql_hdo_meta.c
	scia_wr_sql_hdo_tile.c
   )
   list (APPEND IMAP_LIBS ${PGSQL_LIBRARY})
endif ()

add_executable(scia_imap_ch4 scia_imap_ch4.c 
			     ${IMAP_CH4_SRCS} ${IMAP_CH4_SQL_SRCS})

target_link_libraries(scia_imap_ch4 ${IMAP_LIBS})

add_executable(scia_imap_hdo scia_imap_hdo.c 
			     ${IMAP_HDO_SRCS} ${IMAP_HDO_SQL_SRCS})

target_link_libraries(scia_imap_hdo ${IMAP_LIBS})

add_executable(adaguc_imap_ch4 adaguc_imap_ch4.c ${IMAP_CH4_SRCS})

target_link_libraries(adaguc_imap_ch4 ${IMAP_LIBS})

add_executable(adaguc_imap_hdo adaguc_imap_hdo.c ${IMAP_HDO_SRCS})

target_link_libraries(adaguc_imap_hdo ${IMAP_LIBS})

## Installation instructions
install (
  TARGETS adaguc_imap_ch4 adaguc_imap_hdo scia_imap_ch4 scia_imap_hdo
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
