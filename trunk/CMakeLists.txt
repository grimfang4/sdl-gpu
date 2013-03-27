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
option(SDL_gpu_USE_OPENGL "Use OpenGL backend" ON)
option(SDL_gpu_USE_OPENGLES "Use OpenGLES backend" OFF)
option(SDL_gpu_USE_OPENGLv1 "Restrict function calls to those compatible with OpenGL version 1.1 with extensions" OFF)

set(SDL_gpu_VERSION 0.0.1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeTests)

# Find the package for SDL or SDL2
if ( NOT SDL_gpu_USE_SDL2 )
	find_package(SDL REQUIRED)
	
	if ( NOT SDL_FOUND )
	   message ( FATAL_ERROR "SDL not found!" )
	endif ( NOT SDL_FOUND )
	
	include_directories(${SDL_INCLUDE_DIR})
	link_libraries(${SDL_LIBRARY})
else ( NOT SDL_gpu_USE_SDL2 )
	find_package(SDL2 REQUIRED)
	
	if ( NOT SDL2_FOUND )
	   message ( FATAL_ERROR "SDL2 not found!" )
	endif ( NOT SDL2_FOUND )
	
	include_directories(${SDL2_INCLUDE_DIR})
	link_libraries(${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY})
endif( NOT SDL_gpu_USE_SDL2 )

# Find the package for OpenGL or OpenGLES
if (SDL_gpu_USE_OPENGL)

	find_package(OpenGL REQUIRED)
	include_directories(${OPENGL_INCLUDE_DIR})
	include_directories(SDL_gpu/externals/glew)
	link_libraries (${OPENGL_LIBRARIES})
	add_definitions("-DSDL_GPU_USE_OPENGL")
	
	if (SDL_gpu_USE_OPENGLv1)
		add_definitions("-DSDL_GPU_USE_OPENGLv1")
	endif (SDL_gpu_USE_OPENGLv1)
	
else (SDL_gpu_USE_OPENGL)

	if (SDL_gpu_USE_OPENGLES)

		find_package(OpenGLES REQUIRED)
		include_directories(${OPENGLES_INCLUDE_DIR})
		link_libraries (${OPENGLES_LIBRARIES})
		add_definitions("-DSDL_GPU_USE_OPENGLES")
		
	endif (SDL_gpu_USE_OPENGLES)

endif (SDL_gpu_USE_OPENGL)

# Make local includes look in the right places
include_directories(SDL_gpu)
include_directories(SDL_gpu/externals/glew)
include_directories(SDL_gpu/externals/stb_image)


add_definitions("-g -Wall -std=c99 -pedantic -DSTBI_FAILURE_USERMSG")

# Build the SDL_gpu library.
add_subdirectory(SDL_gpu)

# Build the demos
if(SDL_gpu_BUILD_DEMOS)
  add_subdirectory(demos)
endif(SDL_gpu_BUILD_DEMOS)

# Build sample tools
if(SDL_gpu_BUILD_TOOLS)
  add_subdirectory(tools)
endif(SDL_gpu_BUILD_TOOLS)

