set (GOME_LV1_SRCS gome_lv1.c)
set (GOME_LV2_SRCS gome_lv2.c)

## define pre-compiler flags
if (PGSQL_FOUND)
   add_definitions (-D_WITH_SQL)

   list (APPEND GOME_LV1_SRCS
        gome_lv1_del_entry.c
        gome_lv1_wr_sql_meta.c
        gome_lv1_wr_sql_tile.c
	)
   list (APPEND GOME_LV2_SRCS
        gome_lv2_del_entry.c
        gome_lv2_wr_sql_meta.c
        gome_lv2_wr_sql_tile.c
	)
endif ()

add_executable(gome_lv1 ${GOME_LV1_SRCS})
if (PGSQL_FOUND)
   target_link_libraries(gome_lv1 nadc_gome ${PGSQL_LIBRARY})
else ()
   target_link_libraries(gome_lv1 nadc_gome)
endif ()

add_executable(gome_lv2 ${GOME_LV2_SRCS})
if (PGSQL_FOUND)
   target_link_libraries(gome_lv2 nadc_gome ${PGSQL_LIBRARY})
else ()
   target_link_libraries(gome_lv2 nadc_gome)
endif ()

## Installation instructions
install (
  TARGETS gome_lv1 gome_lv2
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
