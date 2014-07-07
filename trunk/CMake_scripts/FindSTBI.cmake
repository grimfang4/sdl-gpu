# By Jonathan Dearborn
# Based on FindFFMPEG.cmake
# Copyright (c) 2008 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#
# Defines:
# STBI_FOUND
# STBI_INCLUDE_DIR
# STBI_LIBRARY
#

if (STBI_LIBRARY AND STBI_INCLUDE_DIR)
    set(STBI_FOUND TRUE)
else (STBI_LIBRARY AND STBI_INCLUDE_DIR)
    # Use pkg-config if possible
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(_STBI_LIB libstbi)
    endif (PKG_CONFIG_FOUND)

  find_path(STBI_INCLUDE_DIR
    NAMES stb_image.h
    PATHS ${_STBI_INCLUDE_DIRS} /usr/include /usr/local/include /opt/local/include /sw/include
    PATH_SUFFIXES stbi
  )

  find_library(STBI_LIBRARY
    NAMES stbi
    PATHS ${_STBI_LIBRARY_DIRS} /usr/lib /usr/local/lib /opt/local/lib /sw/lib
  )

  if (STBI_LIBRARY)
    set(STBI_FOUND TRUE)
  endif(STBI_LIBRARY)

  if (STBI_FOUND)
    if (NOT STBI_FIND_QUIETLY)
      message(STATUS "Found stb-image: ${STBI_LIBRARY}, ${STBI_INCLUDE_DIR}")
    endif (NOT STBI_FIND_QUIETLY)
  else (STBI_FOUND)
    if (STBI_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find stb-image")
    endif (STBI_FIND_REQUIRED)
  endif (STBI_FOUND)

endif (STBI_LIBRARY AND STBI_INCLUDE_DIR)

