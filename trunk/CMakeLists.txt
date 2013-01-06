cmake_minimum_required(VERSION 2.6)

project(SDL_gpu)

if(UNIX)
	set(SDL_gpu_INSTALL_BY_DEFAULT ON)
else(UNIX)
	set(SDL_gpu_INSTALL_BY_DEFAULT OFF)
endif(UNIX)

option(SDL_gpu_INSTALL "Install SDL_gpu libs, includes, and CMake scripts" ${SDL_gpu_INSTALL_BY_DEFAULT})
option(SDL_gpu_BUILD_SHARED "Build SDL_gpu shared libraries" OFF)
option(SDL_gpu_BUILD_STATIC "Build SDL_gpu static libraries" ON)
option(SDL_gpu_BUILD_EXAMPLES "Build SDL_gpu examples" ON)
option(SDL_gpu_BUILD_TOOLS "Build SDL_gpu tool programs" ON)

set(SDL_gpu_VERSION 0.0.1)
set(SOIL_VERSION 7.26.07)


find_package(OpenGL REQUIRED)
find_package(SDL REQUIRED)

# Workaround for the non-working REQUIRED flag
if ( NOT SDL_FOUND )
   message ( FATAL_ERROR "SDL not found!" )
endif ( NOT SDL_FOUND )


include_directories(${SDL_INCLUDE_DIR})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(SDL_gpu)
include_directories(SDL_gpu/OpenGL/SOIL)

link_libraries (
   ${SDL_LIBRARY}
   ${OPENGL_LIBRARIES}
)

add_definitions("-g -Wall -std=c99 -pedantic")

# The SDL_gpu library.
add_subdirectory(SDL_gpu)


if(SDL_gpu_BUILD_EXAMPLES)
  add_subdirectory(demos)
endif(SDL_gpu_BUILD_EXAMPLES)

if(SDL_gpu_BUILD_TOOLS)
  add_subdirectory(tools)
endif(SDL_gpu_BUILD_TOOLS)

