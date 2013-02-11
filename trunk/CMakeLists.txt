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
option(SDL_gpu_BUILD_DEMOS "Build SDL_gpu demo programs" ON)
option(SDL_gpu_BUILD_TOOLS "Build SDL_gpu tool programs" ON)
option(SDL_gpu_USE_SDL2 "Use SDL2 headers and library instead of SDL1.2" OFF)

set(SDL_gpu_VERSION 0.0.1)
set(SOIL_VERSION 7.26.07)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeTests)

find_package(OpenGL REQUIRED)
if ( NOT SDL_gpu_USE_SDL2 )
	find_package(SDL REQUIRED)
	
	if ( NOT SDL_FOUND )
	   message ( FATAL_ERROR "SDL not found!" )
	endif ( NOT SDL_FOUND )
	
	include_directories(${SDL_INCLUDE_DIR})
	link_libraries(${SDL_LIBRARY})
else ( NOT SDL_gpu_USE_SDL2 )
	#set(SDL2_BUILDING_LIBRARY TRUE)
	find_package(SDL2 REQUIRED)
	
	if ( NOT SDL2_FOUND )
	   message ( FATAL_ERROR "SDL not found!" )
	endif ( NOT SDL2_FOUND )
	
	include_directories(${SDL2_INCLUDE_DIR})
	link_libraries(${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY})
endif( NOT SDL_gpu_USE_SDL2 )



include_directories(${OPENGL_INCLUDE_DIR})
include_directories(SDL_gpu)
include_directories(SDL_gpu/OpenGL/SOIL)

link_libraries (${OPENGL_LIBRARIES})

add_definitions("-g -Wall -std=c99 -pedantic -DSTBI_FAILURE_USERMSG")

# The SDL_gpu library.
add_subdirectory(SDL_gpu)


if(SDL_gpu_BUILD_EXAMPLES)
  add_subdirectory(demos)
endif(SDL_gpu_BUILD_EXAMPLES)

if(SDL_gpu_BUILD_TOOLS)
  add_subdirectory(tools)
endif(SDL_gpu_BUILD_TOOLS)

