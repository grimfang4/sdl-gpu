# By Jonathan Dearborn
# Based on FindFFMPEG.cmake
# Copyright (c) 2008 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#
# Defines:
# STBI_WRITE_FOUND
# STBI_WRITE_INCLUDE_DIR
# STBI_WRITE_LIBRARY
#

if (STBI_WRITE_LIBRARY AND STBI_WRITE_INCLUDE_DIR)
    set(STBI_WRITE_FOUND TRUE)
else (STBI_WRITE_LIBRARY AND STBI_WRITE_INCLUDE_DIR)
    # Use pkg-config if possible
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(_STBI_WRITE_LIB libstbi_write)
    endif (PKG_CONFIG_FOUND)

  find_path(STBI_WRITE_INCLUDE_DIR
    NAMES stb_image_write.h
    PATHS ${_STBI_WRITE_INCLUDE_DIRS} /usr/include /usr/local/include /opt/local/include /sw/include
    PATH_SUFFIXES stbi
  )

  find_library(STBI_WRITE_LIBRARY
    NAMES stbi_write
    PATHS ${_STBI_WRITE_LIBRARY_DIRS} /usr/lib /usr/local/lib /opt/local/lib /sw/lib
  )

  if (STBI_WRITE_LIBRARY)
    set(STBI_WRITE_FOUND TRUE)
  endif(STBI_WRITE_LIBRARY)

  if (STBI_WRITE_FOUND)
    if (NOT STBI_WRITE_FIND_QUIETLY)
      message(STATUS "Found stb-image-write: ${STBI_WRITE_LIBRARY}, ${STBI_WRITE_INCLUDE_DIR}")
    endif (NOT STBI_WRITE_FIND_QUIETLY)
  else (STBI_WRITE_FOUND)
    if (STBI_WRITE_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find stb-image-write")
    endif (STBI_WRITE_FIND_REQUIRED)
  endif (STBI_WRITE_FOUND)

endif (STBI_WRITE_LIBRARY AND STBI_WRITE_INCLUDE_DIR)

