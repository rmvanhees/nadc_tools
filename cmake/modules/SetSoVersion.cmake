#-------------------------------------------------------------------------------
MACRO (NADC_SET_LIB_OPTIONS libtarget libname libtype libversion)
  SET (LIB_OUT_NAME "${libname}")
  IF (${libtype} MATCHES "SHARED")
    SET_TARGET_PROPERTIES (${libtarget} PROPERTIES SOVERSION ${libversion})
  ENDIF (${libtype} MATCHES "SHARED")

  SET (LIB_RELEASE_NAME "${LIB_OUT_NAME}")
  SET (LIB_DEBUG_NAME "${LIB_OUT_NAME}_debug")
  SET_TARGET_PROPERTIES (${libtarget}
      PROPERTIES
      DEBUG_OUTPUT_NAME          ${LIB_DEBUG_NAME}
      RELEASE_OUTPUT_NAME        ${LIB_RELEASE_NAME}
      MINSIZEREL_OUTPUT_NAME     ${LIB_RELEASE_NAME}
      RELWITHDEBINFO_OUTPUT_NAME ${LIB_RELEASE_NAME}
  )
ENDMACRO (NADC_SET_LIB_OPTIONS)
