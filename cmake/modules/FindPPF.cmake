# Exports several useful variables:
#
#   PPF_INCLUDE_DIR
#      full path to the directory containing idl_export.h, i.e.::
#
#        /usr/local/itt/idl/idl80/external/include
#
#   PPF_FOUND
#      Set to true, if all components of PPF have been found
#
if (NOT PPF_FOUND)

  if (NOT PPF_ROOT_DIR)
    set (PPF_ROOT_DIR ${CMAKE_INSTALL_PREFIX})
  endif (NOT PPF_ROOT_DIR)

  ##___________________________________________________________________________
  ## Check for the header files

  find_path(PPF_INCLUDE_DIR ppf_lib.h
    HINTS ${PPF_ROOT_DIR}
    PATH_SUFFIXES include
  )

  if (PPF_INCLUDE_DIR)
    set(PPF_FOUND TRUE)

    set (PPF_LIBRARIES "")
    find_library(PPF_POINTING_LIBRARY ppf_pointing
      HINTS ${PPF_ROOT_DIR}
      PATH_SUFFIXES lib
      )
    if (PPF_POINTING_LIBRARY)
      list (APPEND PPF_LIBRARIES ${PPF_POINTING_LIBRARY})
    endif ()

    find_library(PPF_ORBIT_LIBRARY ppf_orbit
      HINTS ${PPF_ROOT_DIR}
      PATH_SUFFIXES lib
      )
    if (PPF_ORBIT_LIBRARY)
      list (APPEND PPF_LIBRARIES ${PPF_ORBIT_LIBRARY})
    endif ()

    find_library(PPF_LIB_LIBRARY ppf_lib
      HINTS ${PPF_ROOT_DIR}
      PATH_SUFFIXES lib
      )
    if (PPF_LIB_LIBRARY)
      list (APPEND PPF_LIBRARIES ${PPF_LIB_LIBRARY})
    endif ()
  endif ()

  if (PPF_FOUND)
    if (NOT PPF_FIND_QUIETLY)
      message(STATUS "PPF include: ${PPF_INCLUDE_DIR}")
    endif ()
    mark_as_advanced (
      PPF_FOUND
      PPF_LIBRARIES
      PPF_INCLUDE_DIR
    )
  else ()
    if (PPF_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find PPF")
    endif ()
  endif ()
endif (NOT PPF_FOUND)
