set (MERIS_FR1_SRCS meris_fr1.c)
set (MERIS_RR1_SRCS meris_rr1.c)
set (MERIS_RR2_SRCS meris_rr2.c)

add_executable(meris_fr1 ${MERIS_FR1_SRCS})
target_link_libraries(meris_fr1 nadc_meris)

add_executable(meris_rr1 ${MERIS_RR1_SRCS})
target_link_libraries(meris_rr1 nadc_meris)

add_executable(meris_rr2 ${MERIS_RR2_SRCS})
target_link_libraries(meris_rr2 nadc_meris)


## Installation instructions
install (
  TARGETS meris_fr1 meris_rr1 meris_rr2
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
