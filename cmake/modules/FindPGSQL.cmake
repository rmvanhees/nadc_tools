# - Check for the presence of PostgreSQL
#
# If it's found it sets PGSQL_FOUND to TRUE
# and following variables are set:
# PGSQL_INCLUDE_DIR
# PGSQL_LIBRARY
# PGSQL_VERSION
#
if (UNIX)
  set (PGSQL_CONFIG_PREFER_PATH "$ENV{PGSQL_HOME}/bin" CACHE STRING "preferred path to PG (pg_config)")
  find_program (PGSQL_CONFIG pg_config
    ${PGSQL_CONFIG_PREFER_PATH}
    /usr/local/pgsql/bin/
    /usr/local/bin/
    /usr/bin/
    )
  # MESSAGE("DBG PGSQL_CONFIG ${PGSQL_CONFIG}")

  if (PGSQL_CONFIG)
    exec_program(${PGSQL_CONFIG}
      ARGS --version
      OUTPUT_VARIABLE PG_TMP)
    string (REGEX REPLACE "PostgreSQL[ \t]+([0-9]+.[0-9]+.[0-9]+).*$" "\\1" 
      MYSTR ${PG_TMP})
    set (PGSQL_VERSION ${MYSTR})

    # set INCLUDE_DIR
    exec_program(${PGSQL_CONFIG}
      ARGS --includedir
      OUTPUT_VARIABLE PG_TMP)
    set (PGSQL_INCLUDE_DIR ${PG_TMP} CACHE STRING INTERNAL)

    # set LIBRARY_DIR
    exec_program(${PGSQL_CONFIG}
      ARGS --libdir
      OUTPUT_VARIABLE PG_TMP)
    if (APPLE)
      set (PGSQL_LIBRARY ${PG_TMP}/libpq.dylib CACHE STRING INTERNAL)
    else ()
      set (PGSQL_LIBRARY ${PG_TMP}/libpq.so CACHE STRING INTERNAL)
    endif ()
  endif(PGSQL_CONFIG)
endif(UNIX)

if (PGSQL_INCLUDE_DIR AND PGSQL_LIBRARY)
  set (PGSQL_FOUND TRUE)
  if (EXISTS "${PGSQL_INCLUDE_DIR}/pg_config.h")
    set (HAVE_PGCONFIG TRUE)
  else ()
    set (HAVE_PGCONFIG FALSE)
  endif ()
endif (PGSQL_INCLUDE_DIR AND PGSQL_LIBRARY)

if (PGSQL_FOUND)
  mark_as_advanced (
    PGSQL_FOUND
    PGSQL_VERSION
    PGSQL_INCLUDE_DIR
    PGSQL_LIBRARY
  )
  if (NOT PGSQL_FIND_QUIETLY)
    message (STATUS "Found PostgreSQL: ${PGSQL_LIBRARY}")
  endif (NOT PGSQL_FIND_QUIETLY)
else ()
  if (PGSQL_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find PostgreSQL")
  else ()
    message (STATUS "Could not find PostgreSQL")
  endif ()
endif ()
