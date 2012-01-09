cmake_minimum_required(VERSION 2.6)

project(SDL_gpu)

if(UNIX)
	set(SDL_gpu_INSTALL_BY_DEFAULT ON)
	set(SDL_gpuShapes_INSTALL_BY_DEFAULT ON)
else(UNIX)
	set(SDL_gpu_INSTALL_BY_DEFAULT OFF)
	set(SDL_gpuShapes_INSTALL_BY_DEFAULT OFF)
endif(UNIX)

option(SDL_gpu_INSTALL "Install SDL_gpu libs, includes, and CMake scripts" ${SDL_gpu_INSTALL_BY_DEFAULT})
option(SDL_gpu_BUILD_SHARED "Build SDL_gpu shared libraries" OFF)
option(SDL_gpu_BUILD_STATIC "Build SDL_gpu static libraries" ON)
option(SDL_gpu_BUILD_EXAMPLES "Build SDL_gpu examples" ON)

option(SDL_gpuShapes_INSTALL "Install SDL_gpuShapes libs, includes, and CMake scripts" ${SDL_gpuShapes_INSTALL_BY_DEFAULT})
option(SDL_gpuShapes_BUILD_SHARED "Build SDL_gpuShapes shared libraries" OFF)
option(SDL_gpuShapes_BUILD_STATIC "Build SDL_gpuShapes static libraries" ON)
option(SDL_gpuShapes_BUILD_EXAMPLES "Build SDL_gpuShapes examples" ON)

set(SDL_gpu_VERSION 0.0.1)
set(SDL_gpuShapes_VERSION 0.0.1)
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
include_directories(SDL_gpuShapes)
include_directories(SOIL)

link_libraries (
   ${SDL_LIBRARY}
   ${OPENGL_LIBRARIES}
   SDLmain # Not included in SDL_LIBRARY variable
	SOIL
)

add_definitions("-g -Wall -std=c99 -pedantic")

# The SDL_gpu library.
add_subdirectory(SDL_gpu)

# The SDL_gpuShapes library.
add_subdirectory(SDL_gpuShapes)

# The SOIL (static) library.
add_subdirectory(SOIL)


if(SDL_gpu_BUILD_EXAMPLES)
  add_subdirectory(demos)
endif(SDL_gpu_BUILD_EXAMPLES)

