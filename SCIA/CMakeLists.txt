set (SCIA_LV0_SRCS scia_nl0.c)
set (SCIA_LV1_SRCS scia_nl1.c)
set (SCIA_LV2_SRCS scia_ol2.c)
set (INSTALL_TARGETS scia_dmop scia_nl0 scia_nl1 scia_ol2)

## define pre-compiler flags
if (PGSQL_FOUND)
   add_definitions (-D_WITH_SQL)

   list (APPEND SCIA_LV0_SRCS
        scia_lv0_del_entry.c
	scia_lv0_match_state.c
       	scia_lv0_wr_sql_meta.c
   )
   list (APPEND SCIA_LV1_SRCS
        scia_lv1_del_entry.c
	scia_lv1_match_state.c
        scia_lv1_wr_sql_aux.c
        scia_lv1_wr_sql_meta.c
   )
   list (APPEND SCIA_LV2_SRCS
        scia_ol2_del_entry.c
        scia_ol2_match_state.c
        scia_ol2_wr_sql_cld.c
        scia_ol2_wr_sql_meta.c
        scia_ol2_wr_sql_nfit.c
   )
endif ()

add_executable(scia_dmop scia_dmop.c)
if (PGSQL_FOUND)
   target_link_libraries(scia_dmop nadc ${PGSQL_LIBRARY})
else ()
   target_link_libraries(scia_dmop nadc)
endif ()

add_executable(scia_nl0 ${SCIA_LV0_SRCS})
if (PGSQL_FOUND)
   target_link_libraries(scia_nl0 nadc_scia ${PGSQL_LIBRARY})
else ()
   target_link_libraries(scia_nl0 nadc_scia)
endif ()

add_executable(scia_nl1 ${SCIA_LV1_SRCS})
if (PGSQL_FOUND)
   target_link_libraries(scia_nl1 nadc_scia_cal ${PGSQL_LIBRARY})
else ()
   target_link_libraries(scia_nl1 nadc_scia_cal)
endif ()

if (HDF5_VERSION VERSION_GREATER 1.10)
   add_executable(scia_lv1_nc scia_lv1_nc.c)
   target_link_libraries(scia_lv1_nc nadc_scia)
   list (APPEND INSTALL_TARGETS scia_lv1_nc)
endif ()

add_executable(scia_ol2 ${SCIA_LV2_SRCS})
if (PGSQL_FOUND)
   target_link_libraries(scia_ol2 nadc_scia ${PGSQL_LIBRARY})
else ()
   target_link_libraries(scia_ol2 nadc_scia)
endif ()

## Installation instructions
install (
  TARGETS ${INSTALL_TARGETS}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
