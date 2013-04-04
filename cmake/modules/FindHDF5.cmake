# - Check for the presence of HDF5
#
# The following variables are set when HDF5 is found:
# HDF5_FOUND = Set to true, if all components of HDF5 have been found.
# HDF5_INCLUDE_DIRS = Include path for the header files of HDF5
# HDF5_C_LIBRARIES = Required libraries for the HDF5 C bindings.
# HDF5_LIBRARIES = Required libraries for the HDF5 C++ bindings
# HDF5_VERSION = version of the HDF5 library
# HDF5_VERSION_MAJOR = Major version of the HDF5 library
# HDF5_VERSION_MINOR = Minor version of the HDF5 library
# HDF5_VERSION_RELEASE = Release version of the HDF5 library

if (NOT FIND_HDF5_CMAKE)

  if (NOT HDF5_ROOT_DIR)
    if (NOT "$ENV{HDF5_ROOT_DIR}" STREQUAL "")
      set (HDF5_ROOT_DIR $ENV{HDF5_ROOT_DIR})
    else (NOT "$ENV{HDF5_ROOT_DIR}" STREQUAL "")
      set (HDF5_ROOT_DIR ${CMAKE_INSTALL_PREFIX})
    endif (NOT "$ENV{HDF5_ROOT_DIR}" STREQUAL "")
  endif (NOT HDF5_ROOT_DIR)

  set (FIND_HDF5_CMAKE TRUE )
  set (HDF5_VERSION_STRING 0 )

  ##___________________________________________________________________________
  ## Check for the header files

  ## Include directory

  find_path (HDF5_INCLUDE_DIRS hdf5.h hdf5_hl.h
    HINTS ${HDF5_ROOT_DIR}
    PATH_SUFFIXES include hdf5 include/hdf5
    )

  ## Individual header files

  find_path (HDF5_FOUND_HDF5_H hdf5.h
    HINTS ${HDF5_ROOT_DIR}
    PATH_SUFFIXES include hdf5 include/hdf5
    NO_DEFAULT_PATH
    )

  find_path (HDF5_FOUND_H5LT_H H5LT.h
    HINTS ${HDF5_ROOT_DIR}
    PATH_SUFFIXES include hdf5 include/hdf5
    NO_DEFAULT_PATH
    )

  find_path (HDF5_FOUND_HDF5_HL_H hdf5_hl.h
    HINTS ${HDF5_ROOT_DIR}
    PATH_SUFFIXES include hdf5 include/hdf5
    NO_DEFAULT_PATH
    )

  ##___________________________________________________________________________
  ## Check for the library components

  set (HDF5_LIBRARIES "")
  set (HDF5_C_LIBRARIES "")

  find_library (HDF5_HDF5_LIBRARY hdf5
    HINTS ${HDF5_ROOT_DIR}
    PATH_SUFFIXES lib hdf5/lib
    )

  find_library (HDF5_HDF5_HL_LIBRARY hdf5_hl
    HINTS ${HDF5_ROOT_DIR}
    PATH_SUFFIXES lib hdf5/lib
    )

  find_library (HDF5_HDF5_CPP_LIBRARY hdf5_cpp
    HINTS ${HDF5_ROOT_DIR}
    PATH_SUFFIXES lib hdf5/lib
    )

  find_library (HDF5_Z_LIBRARY z
    HINTS ${HDF5_ROOT_DIR}
    PATH_SUFFIXES lib hdf5/lib
    )

  if (HDF5_HDF5_LIBRARY)
    set (HDF5_LIBRARIES ${HDF5_Z_LIBRARY} ${HDF5_HDF5_LIBRARY})
    set (HDF5_C_LIBRARIES ${HDF5_Z_LIBRARY} ${HDF5_HDF5_LIBRARY})
    set (HDF5_CXXLIBRARIES ${HDF5_Z_LIBRARY} ${HDF5_HDF5_LIBRARY})
  endif (HDF5_HDF5_LIBRARY)

  if (HDF5_HDF5_HL_LIBRARY)
    list (APPEND HDF5_LIBRARIES ${HDF5_HDF5_HL_LIBRARY})
    list (APPEND HDF5_C_LIBRARIES ${HDF5_HDF5_HL_LIBRARY})
  endif (HDF5_HDF5_HL_LIBRARY)

  if (HDF5_HDF5_CPP_LIBRARY)
    list (APPEND HDF5_LIBRARIES ${HDF5_HDF5_CPP_LIBRARY})
  endif (HDF5_HDF5_CPP_LIBRARY)

  list(REVERSE HDF5_LIBRARIES)
  list(REVERSE HDF5_C_LIBRARIES)

  ## Actions taken when all components have been found

  if (HDF5_INCLUDE_DIRS AND HDF5_LIBRARIES)
    set (HDF5_FOUND TRUE)
    set (HDF5_FOUND TRUE)
  else (HDF5_INCLUDE_DIRS AND HDF5_LIBRARIES)
    set (HDF5_FOUND FALSE)
    set (HDF5_FOUND FALSE)
    if (NOT HDF5_FIND_QUIETLY)
      if (NOT HDF5_INCLUDE_DIRS)
	message (STATUS "Unable to find HDF5 header files!")
      endif (NOT HDF5_INCLUDE_DIRS)
      if (NOT HDF5_LIBRARIES)
	message (STATUS "Unable to find HDF5 library files!")
      endif (NOT HDF5_LIBRARIES)
    endif (NOT HDF5_FIND_QUIETLY)
  endif (HDF5_INCLUDE_DIRS AND HDF5_LIBRARIES)

  ##___________________________________________________________________________
  ## Test HDF5 library for:
  ## - library version <major.minor.release>
  ## - parallel I/O support
  ## - default API version

  foreach(HDF5_INCLUDE ${HDF5_INCLUDE_DIRS})
    file(STRINGS "${HDF5_INCLUDE}/H5pubconf.h" HDF5_VERSION REGEX "H5_VERSION")
    if (HDF5_VERSION)
      break()
    endif(HDF5_VERSION)
  endforeach()
  string(REGEX REPLACE ".*([0-9]+).[0-9]+.[0-9]+.*" "\\1"
    HDF5_VERSION_MAJOR ${HDF5_VERSION})
  string(REGEX REPLACE ".*[0-9]+.([0-9]+).[0-9]+.*" "\\1"
    HDF5_VERSION_MINOR ${HDF5_VERSION})
  string(REGEX REPLACE ".*[0-9]+.[0-9]+.([0-9]+).*" "\\1"
    HDF5_VERSION_RELEASE ${HDF5_VERSION})

  set (HDF5_VERSION "${HDF5_VERSION_MAJOR}.${HDF5_VERSION_MINOR}.${HDF5_VERSION_RELEASE}")
  if (NOT HDF5_VERSION_MAJOR EQUAL 1 OR NOT HDF5_VERSION_MINOR GREATER 7)
    message (FATAL_ERROR
      "Could not find correct HDF5 version! Looking for >= 1.8")
  endif()
  ##___________________________________________________________________________
  ## Feedback

  if (HDF5_FOUND)
    if (NOT HDF5_FIND_QUIETLY)
      message (STATUS "Found components for HDF5")
      message (STATUS "HDF5_INCLUDE_DIRS = ${HDF5_INCLUDE_DIRS}")
      message (STATUS "HDF5_LIBRARIES = ${HDF5_LIBRARIES}")
      message (STATUS "HDF5_VERSION_MAJOR = ${HDF5_VERSION_MAJOR}")
      message (STATUS "HDF5_VERSION_MINOR = ${HDF5_VERSION_MINOR}")
      message (STATUS "HDF5_VERSION_RELEASE = ${HDF5_VERSION_RELEASE}")
    endif (NOT HDF5_FIND_QUIETLY)
  else (HDF5_FOUND)
    if (HDF5_FIND_REQUIRED)
      message (FATAL_ERROR "Could not find HDF5!")
    endif (HDF5_FIND_REQUIRED)
  endif (HDF5_FOUND)

  ## Set standard variable
  set (HDF5_FOUND ${HDF5_FOUND})

  ##_____________________________________________________________________________
  ## Mark as advanced ...

  mark_as_advanced (
    HDF5_INCLUDE_DIRS
    HDF5_LIBRARIES
    HDF5_C_LIBRARIES
    HAVE_H5PUBLIC_H
    HDF5_VERSION
    HDF5_VERSION_MAJOR
    HDF5_VERSION_MINOR
    HDF5_VERSION_RELEASE
    )
# HAVE_TESTHDF5VERSION
# HDF5_IS_PARALLEL

endif (NOT FIND_HDF5_CMAKE)
