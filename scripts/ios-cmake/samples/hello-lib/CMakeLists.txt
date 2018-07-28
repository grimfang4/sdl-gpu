# Standard setup
cmake_minimum_required (VERSION 2.8)
project (hello-lib CXX C)

# Includes
include_directories (${hello-lib_SOURCE_DIR})

# Source files
set (SRC
	HelloWorld.cpp
	HelloIOS.mm
)

# Headers
set (HDR
	HelloWorld.h
	HelloIOS.h
)

# Library
add_library (hello-lib STATIC ${SRC} ${HDR})

# Installation
set (CMAKE_INSTALL_PREFIX "${hello-lib_SOURCE_DIR}/../hello-app")
install (TARGETS hello-lib DESTINATION .)
install (FILES ${HDR} DESTINATION .)
