cmake_minimum_required(VERSION 2.6)

project(SDL_gpu)



set(SDL_gpu_INSTALL_BY_DEFAULT ON)
set(SDL_gpu_DEFAULT_BUILD_SHARED ON)
set(SDL_gpu_DEFAULT_BUILD_STATIC ON)
set(SDL_gpu_DEFAULT_BUILD_DEMOS ON)
set(SDL_gpu_DEFAULT_DISABLE_OPENGL OFF)
set(SDL_gpu_DEFAULT_DISABLE_GLES ON)

if(IOS)
  message(" Using iOS defaults.")
  set(SDL_gpu_INSTALL_BY_DEFAULT OFF)
  set(SDL_gpu_DEFAULT_BUILD_DEMOS OFF)
  set(SDL_gpu_DEFAULT_DISABLE_OPENGL ON)
  set(SDL_gpu_DEFAULT_DISABLE_GLES OFF)
  # iOS 8 may need the framework option
  set(SDL_gpu_DEFAULT_BUILD_SHARED OFF)
  set(SDL_gpu_DEFAULT_BUILD_STATIC ON)
elseif(APPLE)
  set(SDL_gpu_DEFAULT_BUILD_SHARED ON)
  option(SDL_gpu_BUILD_FRAMEWORK "Build SDL_gpu as Apple framework" ON)
  set(SDL_gpu_DEFAULT_BUILD_STATIC ON)
elseif(ANDROID)
  set(SDL_gpu_INSTALL_BY_DEFAULT OFF)
  set(SDL_gpu_DEFAULT_BUILD_DEMOS OFF)
  set(SDL_gpu_DEFAULT_DISABLE_OPENGL ON)
  set(SDL_gpu_DEFAULT_DISABLE_GLES OFF)
  set(SDL_gpu_DEFAULT_BUILD_SHARED ON)
  set(SDL_gpu_DEFAULT_BUILD_STATIC OFF)
elseif(WIN32)
  set(SDL_gpu_INSTALL_BY_DEFAULT OFF)
elseif(("${CMAKE_SYSTEM}" MATCHES "Linux") AND ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm"))
  
  # Assuming Raspberry Pi 2 for now.
  # There is a bug with the shipping GLES v1 library where 3 of the symbols are misnamed.
  # If you have an arm/ES device without this problem, don't enable this macro.
  # TODO: Need way to identify Raspberry Pi/Broadcom
  add_definitions(-DSDL_GPU_USE_BROADCOM_RASPBERRYPI_WORKAROUND)

  set(SDL_gpu_DEFAULT_BUILD_DEMOS OFF)
  set(SDL_gpu_DEFAULT_DISABLE_OPENGL ON)
  set(SDL_gpu_DEFAULT_DISABLE_GLES OFF)
elseif(EMSCRIPTEN)
  set(SDL_gpu_INSTALL_BY_DEFAULT OFF)
  set(SDL_gpu_DEFAULT_BUILD_DEMOS OFF)
  set(SDL_gpu_DEFAULT_DISABLE_OPENGL ON)
  set(SDL_gpu_DEFAULT_DISABLE_GLES OFF)
  set(SDL_gpu_DEFAULT_BUILD_SHARED OFF)
endif()


option(SDL_gpu_INSTALL "Install SDL_gpu libs, includes, and CMake scripts" ${SDL_gpu_INSTALL_BY_DEFAULT})
#option(SDL_gpu_BUILD_DEBUG "Build with debugging symbols" ON)
option(SDL_gpu_BUILD_DEMOS "Build SDL_gpu demo programs" ${SDL_gpu_DEFAULT_BUILD_DEMOS})
option(SDL_gpu_BUILD_TESTS "Build SDL_gpu test programs" OFF)
option(SDL_gpu_BUILD_VIDEO_TEST "Build SDL_gpu video test program (requires FFMPEG)" OFF)
option(SDL_gpu_BUILD_TOOLS "Build SDL_gpu tool programs" OFF)
option(SDL_gpu_USE_SDL1 "Use SDL 1.2 headers and library instead of SDL 2" OFF)
option(SDL_gpu_DISABLE_OPENGL "Disable OpenGL renderers.  Overrides specific OpenGL renderer flags." ${SDL_gpu_DEFAULT_DISABLE_OPENGL})
option(SDL_gpu_DISABLE_GLES "Disable OpenGLES renderers.  Overrides specific GLES renderer flags." ${SDL_gpu_DEFAULT_DISABLE_GLES})
option(SDL_gpu_DISABLE_OPENGL_1_BASE "Disable OpenGL 1 BASE renderer" OFF)
option(SDL_gpu_DISABLE_OPENGL_1 "Disable OpenGL 1.X renderer" OFF)
option(SDL_gpu_DISABLE_OPENGL_2 "Disable OpenGL 2.X renderer" OFF)
option(SDL_gpu_DISABLE_OPENGL_3 "Disable OpenGL 3.X renderer" OFF)
option(SDL_gpu_DISABLE_OPENGL_4 "Disable OpenGL 4.X renderer" OFF)
option(SDL_gpu_DISABLE_GLES_1 "Disable OpenGLES 1.X renderer" OFF)
option(SDL_gpu_DISABLE_GLES_2 "Disable OpenGLES 2.X renderer" OFF)
option(SDL_gpu_DISABLE_GLES_3 "Disable OpenGLES 3.X renderer" OFF)

option(SDL_gpu_USE_SYSTEM_GLEW "Attempt to use the system GLEW library (may not support GL 3+)" OFF)
option(SDL_gpu_DYNAMIC_GLES_3 "Attempt to run-time link to GLES 3" OFF)

option(SDL_gpu_USE_BUFFER_RESET "Upload VBOs by requesting a new one each time (default).  This is often the best for driver optimization)" ON)
option(SDL_gpu_USE_BUFFER_UPDATE "Upload VBOs by updating only the needed portion" OFF)
option(SDL_gpu_USE_BUFFER_MAPPING "Upload VBOs by mapping to client memory" OFF)



option(SDL_gpu_BUILD_SHARED "Build SDL_gpu shared libraries" ${SDL_gpu_DEFAULT_BUILD_SHARED})
option(SDL_gpu_BUILD_STATIC "Build SDL_gpu static libraries" ${SDL_gpu_DEFAULT_BUILD_STATIC})

set(SDL_gpu_VERSION 0.11.0)

if (SDL_gpu_USE_SDL1)
    set(SDL_gpu_LIBRARY SDL_gpu)
else (SDL_gpu_USE_SDL1)
    set(SDL_gpu_LIBRARY SDL2_gpu)
endif (SDL_gpu_USE_SDL1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/scripts)

if(APPLE)
	include(ios-cmake/toolchain/XcodeDefaults)
endif()

#if ( SDL_gpu_BUILD_DEBUG )
#	SET(CMAKE_BUILD_TYPE Debug)
#else ( SDL_gpu_BUILD_DEBUG )
#	SET(CMAKE_BUILD_TYPE Release)
#endif ( SDL_gpu_BUILD_DEBUG )

# Find the package for SDL or SDL2
if ( SDL_gpu_USE_SDL1 )
	find_package(SDL REQUIRED)

	if ( NOT SDL_FOUND )
	   message ( FATAL_ERROR "SDL not found!" )
	endif ( NOT SDL_FOUND )

	include_directories(${SDL_INCLUDE_DIR})
	link_libraries(${SDL_LIBRARY})
else ( SDL_gpu_USE_SDL1 )
	find_package(SDL2 REQUIRED)

	if ( NOT SDL2_FOUND )
	   message ( FATAL_ERROR "SDL2 not found!" )
	endif ( NOT SDL2_FOUND )

	if ( NOT SDL2MAIN_LIBRARY )
		message ( WARNING "SDL2MAIN_LIBRARY is NOTFOUND" )
		SET( SDL2MAIN_LIBRARY "" )
	endif ( NOT SDL2MAIN_LIBRARY )

	include_directories(${SDL2_INCLUDE_DIR})
	link_libraries(${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY})
endif( SDL_gpu_USE_SDL1 )

# Find the package for OpenGL
if (SDL_gpu_DISABLE_OPENGL)
	add_definitions("-DSDL_GPU_DISABLE_OPENGL")
else (SDL_gpu_DISABLE_OPENGL)

	find_package(OpenGL REQUIRED)
	include_directories(${OPENGL_INCLUDE_DIR})
	if(APPLE AND NOT IOS)
		# CMake incorrectly includes AGL.framework in OPENGL_LIBRARIES which is obsolete.
		# link_libraries(${OPENGL_gl_LIBRARY})
		set(SDL_gpu_GL_LIBRARIES ${OPENGL_gl_LIBRARY})
	else()
		# link_libraries(${OPENGL_LIBRARIES})
		set(SDL_gpu_GL_LIBRARIES ${OPENGL_LIBRARIES})
	endif()


	if (SDL_gpu_DISABLE_OPENGL_1_BASE)
		add_definitions("-DSDL_GPU_DISABLE_OPENGL_1_BASE")
	endif (SDL_gpu_DISABLE_OPENGL_1_BASE)
	if (SDL_gpu_DISABLE_OPENGL_1)
		add_definitions("-DSDL_GPU_DISABLE_OPENGL_1")
	endif (SDL_gpu_DISABLE_OPENGL_1)
	if (SDL_gpu_DISABLE_OPENGL_2)
		add_definitions("-DSDL_GPU_DISABLE_OPENGL_2")
	endif (SDL_gpu_DISABLE_OPENGL_2)
	if (SDL_gpu_DISABLE_OPENGL_3)
		add_definitions("-DSDL_GPU_DISABLE_OPENGL_3")
	endif (SDL_gpu_DISABLE_OPENGL_3)
	if (SDL_gpu_DISABLE_OPENGL_4)
		add_definitions("-DSDL_GPU_DISABLE_OPENGL_4")
	endif (SDL_gpu_DISABLE_OPENGL_4)

	if(SDL_gpu_USE_SYSTEM_GLEW)
	    # If glew is not found here, we’ll use the bundled version
		find_package(GLEW)
	endif()

    if(NOT GLEW_FOUND)
        message(" Using bundled version of GLEW")
    endif(NOT GLEW_FOUND)


	if (NOT SDL_gpu_DISABLE_OPENGL)
		if(GLEW_FOUND)
			# Look in the GL subdirectory, too.
			foreach(GL_DIR ${GLEW_INCLUDE_DIRS})
				set(GLEW_GL_DIRS ${GLEW_GL_DIRS} "${GL_DIR}/GL")
			endforeach(GL_DIR ${GLEW_INCLUDE_DIRS})

			include_directories(${GLEW_INCLUDE_DIRS} ${GLEW_GL_DIRS})
			link_libraries (${GLEW_LIBRARIES})
		else(GLEW_FOUND)
			SET(SDL_gpu_SRCS ${SDL_gpu_SRCS} externals/glew/glew.c)
			SET(SDL_gpu_HDRS ${SDL_gpu_HDRS} externals/glew/GL/glew.h externals/glew/GL/glxew.h externals/glew/GL/wglew.h)
			add_definitions("-DGLEW_STATIC")
		endif(GLEW_FOUND)
	endif(NOT SDL_gpu_DISABLE_OPENGL)

endif (SDL_gpu_DISABLE_OPENGL)

# Find the package for OpenGLES
if (SDL_gpu_DISABLE_GLES)
	add_definitions("-DSDL_GPU_DISABLE_GLES")
else (SDL_gpu_DISABLE_GLES)

	if(ANDROID)
		find_library( ANDROID_LOG_LIBRARY log )
		find_library( ANDROID_GLES2_LIBRARY GLESv2 )
		find_library( ANDROID_GLES1_LIBRARY GLESv1_CM )
		link_libraries(
			${ANDROID_LOG_LIBRARY}
			${ANDROID_GLES2_LIBRARY}
			${ANDROID_GLES1_LIBRARY}
		)
	else()
		find_package(OpenGLES REQUIRED)
		include_directories(${OPENGLES_INCLUDE_DIR})
#		link_libraries (${OPENGLES_LIBRARIES})
		set(SDL_gpu_GL_LIBRARIES ${OPENGLES_LIBRARIES})
	endif()

	if (SDL_gpu_DISABLE_GLES_1)
		add_definitions("-DSDL_GPU_DISABLE_GLES_1")
	endif (SDL_gpu_DISABLE_GLES_1)
	if (SDL_gpu_DISABLE_GLES_2)
		add_definitions("-DSDL_GPU_DISABLE_GLES_2")
	endif (SDL_gpu_DISABLE_GLES_2)
	if (SDL_gpu_DISABLE_GLES_3)
		add_definitions("-DSDL_GPU_DISABLE_GLES_3")
	endif (SDL_gpu_DISABLE_GLES_3)
    
    include_directories(src/externals/gl3stub)
    if (SDL_gpu_DYNAMIC_GLES_3)
        SET(SDL_gpu_HDRS ${SDL_gpu_HDRS} externals/gl3stub/gl3stub.c externals/gl3stub/gl3stub.h)
        add_definitions("-DSDL_GPU_DYNAMIC_GLES_3")
    endif(SDL_gpu_DYNAMIC_GLES_3)
endif (SDL_gpu_DISABLE_GLES)

# If stb-image is not found here, we’ll use the bundled version
find_package(STBI)
if(NOT STBI_FOUND)
  message(" Using bundled version of stb-image")
endif(NOT STBI_FOUND)

find_package(STBI_write)
if(NOT STBI_WRITE_FOUND)
  message(" Using bundled version of stb-image-write")
endif(NOT STBI_WRITE_FOUND)

if(NOT WIN32 AND NOT EMSCRIPTEN)
  find_library(M_LIB m)
  link_libraries(${M_LIB})
endif (NOT WIN32 AND NOT EMSCRIPTEN)

# Make local includes look in the right places
include_directories(include)
include_directories(src)

if(NOT GLEW_FOUND)
  include_directories(src/externals/glew)
  include_directories(src/externals/glew/GL)
endif(NOT GLEW_FOUND)

# Set stuff up for using bundled stbi
if(NOT STBI_FOUND OR NOT STBI_WRITE_FOUND)
  add_definitions("-DSTBI_FAILURE_USERMSG")
  
	if(NOT STBI_FOUND)
	  include_directories(src/externals/stb_image)
	endif(NOT STBI_FOUND)
	
	if(NOT STBI_WRITE_FOUND)
	  include_directories(src/externals/stb_image_write)
	endif(NOT STBI_WRITE_FOUND)
endif(NOT STBI_FOUND OR NOT STBI_WRITE_FOUND)

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_INPUT ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)
    configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)

add_definitions("-Wall -std=c99 -pedantic")

if (SDL_gpu_USE_BUFFER_RESET)
    add_definitions("-DSDL_GPU_USE_BUFFER_RESET")
endif (SDL_gpu_USE_BUFFER_RESET)
if (SDL_gpu_USE_BUFFER_UPDATE)
    add_definitions("-DSDL_GPU_USE_BUFFER_UPDATE")
endif (SDL_gpu_USE_BUFFER_UPDATE)
if (SDL_gpu_USE_BUFFER_MAPPING)
    add_definitions("-DSDL_GPU_USE_BUFFER_MAPPING")
endif (SDL_gpu_USE_BUFFER_MAPPING)

# Build the SDL_gpu library.
add_subdirectory(src)

if(SDL_gpu_BUILD_DEMOS OR SDL_gpu_BUILD_TESTS OR SDL_gpu_BUILD_TOOLS)
	add_library(test-common STATIC ${SDL_gpu_SOURCE_DIR}/common/common.c ${SDL_gpu_SOURCE_DIR}/common/demo-font.c)
	set(TEST_LIBS test-common SDL_gpu)
endif(SDL_gpu_BUILD_DEMOS OR SDL_gpu_BUILD_TESTS OR SDL_gpu_BUILD_TOOLS)

# Build the demos
if(SDL_gpu_BUILD_DEMOS)
  add_subdirectory(demos)
endif(SDL_gpu_BUILD_DEMOS)

# Build the tests
if(SDL_gpu_BUILD_TESTS)
  if(SDL_gpu_BUILD_VIDEO_TEST)

	find_package(FFMPEG REQUIRED)
	include_directories(${FFMPEG_INCLUDE_DIR})
	link_libraries (${FFMPEG_LIBRARIES})

    add_definitions("-DSDL_GPU_BUILD_VIDEO_TEST")
  endif(SDL_gpu_BUILD_VIDEO_TEST)

  add_subdirectory(tests)
endif(SDL_gpu_BUILD_TESTS)

# Build sample tools
if(SDL_gpu_BUILD_TOOLS)
  add_subdirectory(tools)
endif(SDL_gpu_BUILD_TOOLS)
