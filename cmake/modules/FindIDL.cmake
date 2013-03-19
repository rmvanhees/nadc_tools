# Exports several useful variables:
#
#   IDL_INCLUDE_DIR
#      full path to the directory containing idl_export.h, i.e.::
#
#        /usr/local/itt/idl/idl80/external/include
#
#   IDL_FOUND
#      Set to true, if all components of IDL have been found
#
if (NOT IDL_FOUND)

  if (NOT IDL_ROOT_DIR)
    set (IDL_ROOT_DIR ${CMAKE_INSTALL_PREFIX})
  endif (NOT IDL_ROOT_DIR)

  ##___________________________________________________________________________
  ## Check for the header files

  find_path(IDL_INCLUDE_DIR
    idl_export.h
    HINTS ${IDL_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES external/include
  )

  if (IDL_INCLUDE_DIR)
    set(IDL_FOUND TRUE)
  endif ()

  if (IDL_FOUND)
    if (NOT IDL_FIND_QUIETLY)
      message(STATUS "IDL include: ${IDL_INCLUDE_DIR}")
    endif ()
    mark_as_advanced (
      IDL_FOUND
      IDL_INCLUDE_DIR
    )
  else ()
    if (IDL_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find IDL")
    endif ()
  endif ()
endif (NOT IDL_FOUND)
