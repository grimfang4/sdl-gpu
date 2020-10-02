cmake_minimum_required(VERSION 3.0)

project(SDL_gpu)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version.txt VERSION_FILE_CONTENTS)

if((NOT DEFINED VERSION_MAJOR OR NOT DEFINED VERSION_MINOR OR NOT DEFINED VERSION_BUGFIX) AND VERSION_FILE_CONTENTS STREQUAL "")
	message(ERROR "Missing version.txt, VERSION_MAJOR, VERSION_MINOR, and VERSION_BUGFIX.")
	message(FATAL_ERROR "Failed to read version.txt and cannot proceed without a version number for SDL_gpu.")
endif()

if(NOT DEFINED VERSION_MAJOR)
	string(REGEX MATCH "^[0-9]+" VERSION_MAJOR ${VERSION_FILE_CONTENTS})
endif()
if(NOT DEFINED VERSION_MINOR)
	string(REGEX MATCH "[.][0-9]+[.]" VERSION_MINOR ${VERSION_FILE_CONTENTS})
	# Chop off the dots
	string(LENGTH ${VERSION_MINOR} VERSION_MINOR_LENGTH)
	math(EXPR VERSION_MINOR_LENGTH_MINUS_2 "${VERSION_MINOR_LENGTH}-2")
	string(SUBSTRING ${VERSION_MINOR} 1 ${VERSION_MINOR_LENGTH_MINUS_2} VERSION_MINOR)
endif()
if(NOT DEFINED VERSION_BUGFIX)
	string(REGEX MATCH "[0-9]+$" VERSION_BUGFIX ${VERSION_FILE_CONTENTS})
endif()

set(SDL_GPU_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUGFIX}")

if(${CMAKE_CURRENT_SOURCE_DIR}/version.txt IS_NEWER_THAN ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_gpu_version.h)
    # This is to prevent CMake from rebuilding SDL_gpu, which is important when it is an external project in another project.
    # This works since whenever version.txt is changed, it will be newer than SDL_gpu_version.h. The only time SDL_gpu_version.h
    # will be newer than version.txt is when it is created or modified after the latter file.
    message("Writing version header for v${SDL_GPU_VERSION}")
    string(CONCAT VERSION_HEADER_CONTENTS "#ifndef _SDL_GPU_VERSION_H__
#define _SDL_GPU_VERSION_H__

#define SDL_GPU_VERSION_MAJOR ${VERSION_MAJOR}
#define SDL_GPU_VERSION_MINOR ${VERSION_MINOR}
#define SDL_GPU_VERSION_PATCH ${VERSION_BUGFIX}

#endif")

    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_gpu_version.h ${VERSION_HEADER_CONTENTS})
endif()

set(DEFAULT_INSTALL_LIBRARY ON)
set(DEFAULT_BUILD_SHARED ON)
set(DEFAULT_BUILD_STATIC ON)
set(DEFAULT_BUILD_DEMOS ON)
set(DEFAULT_DISABLE_OPENGL OFF)
set(DEFAULT_DISABLE_GLES ON)

if(IOS)
  message(" Using iOS defaults.")
  set(DEFAULT_INSTALL_LIBRARY OFF)
  set(DEFAULT_BUILD_DEMOS OFF)
  set(DEFAULT_DISABLE_OPENGL ON)
  set(DEFAULT_DISABLE_GLES OFF)
  # iOS 8 may need the framework option
  set(DEFAULT_BUILD_SHARED OFF)
  set(DEFAULT_BUILD_STATIC ON)
elseif(APPLE)
  message(" Using Apple defaults.")
  set(DEFAULT_BUILD_SHARED ON)
  option(BUILD_FRAMEWORK "Build SDL_gpu as Apple framework" ON)
  set(DEFAULT_BUILD_STATIC ON)
elseif(ANDROID)
  message(" Using Android defaults.")
  set(DEFAULT_INSTALL_LIBRARY OFF)
  set(DEFAULT_BUILD_DEMOS OFF)
  set(DEFAULT_DISABLE_OPENGL ON)
  set(DEFAULT_DISABLE_GLES OFF)
  set(DEFAULT_BUILD_SHARED ON)
  set(DEFAULT_BUILD_STATIC OFF)
elseif(WIN32)
  message(" Using Windows defaults.")
  set(DEFAULT_INSTALL_LIBRARY OFF)
elseif(("${CMAKE_SYSTEM}" MATCHES "Linux") AND ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm"))
  message(" Using Linux ARM defaults.")

  # Check specifically for Raspberry Pi (somewhat hacky).
  if(NOT SKIP_BCM_HOST_H_CHECK AND EXISTS /opt/vc/include/bcm_host.h)
    message("Defining ADD_MISSING_OES_FUNCTIONS")
    # There is a bug with the GLES v1 library shipping with the Raspberry Pi 2 where 3 of the symbols are misnamed.
    # If you have an arm/ES device without this problem, don't enable this macro.
    add_definitions(-DADD_MISSING_OES_FUNCTIONS)
  endif()

  set(DEFAULT_BUILD_DEMOS OFF)
  set(DEFAULT_DISABLE_OPENGL ON)
  set(DEFAULT_DISABLE_GLES OFF)
elseif(EMSCRIPTEN)
  message(" Using Emscripten defaults.")
  set(DEFAULT_INSTALL_LIBRARY OFF)
  set(DEFAULT_BUILD_DEMOS OFF)
  set(DEFAULT_DISABLE_OPENGL ON)
  set(DEFAULT_DISABLE_GLES OFF)
  set(DEFAULT_DISABLE_GLES_1 ON)
  set(DEFAULT_BUILD_SHARED OFF)
else()
  message(" Using generic defaults.")
endif()


option(INSTALL_LIBRARY "Install SDL_gpu libs, includes, and CMake scripts" ${DEFAULT_INSTALL_LIBRARY})
#option(BUILD_DEBUG "Build with debugging symbols" ON)
option(BUILD_DEMOS "Build SDL_gpu demo programs" ${DEFAULT_BUILD_DEMOS})
option(BUILD_TESTS "Build SDL_gpu test programs" OFF)
option(BUILD_VIDEO_TEST "Build SDL_gpu video test program (requires FFMPEG)" OFF)
option(BUILD_TOOLS "Build SDL_gpu tool programs" OFF)
option(BUILD_DOCS "Build SDL_gpu documentation" OFF)
option(USE_SDL1 "Use SDL 1.2 headers and library instead of SDL 2" OFF)
option(DISABLE_OPENGL "Disable OpenGL renderers.  Overrides specific OpenGL renderer flags." ${DEFAULT_DISABLE_OPENGL})
option(DISABLE_GLES "Disable OpenGLES renderers.  Overrides specific GLES renderer flags." ${DEFAULT_DISABLE_GLES})
option(DISABLE_OPENGL_1_BASE "Disable OpenGL 1 BASE renderer" OFF)
option(DISABLE_OPENGL_1 "Disable OpenGL 1.X renderer" OFF)
option(DISABLE_OPENGL_2 "Disable OpenGL 2.X renderer" OFF)
option(DISABLE_OPENGL_3 "Disable OpenGL 3.X renderer" OFF)
option(DISABLE_OPENGL_4 "Disable OpenGL 4.X renderer" OFF)
option(DISABLE_GLES_1 "Disable OpenGLES 1.X renderer" OFF)
option(DISABLE_GLES_2 "Disable OpenGLES 2.X renderer" OFF)
option(DISABLE_GLES_3 "Disable OpenGLES 3.X renderer" OFF)

option(USE_SYSTEM_GLEW "Attempt to use the system GLEW library (may not support GL 3+)" OFF)
option(DYNAMIC_GLES_3 "Attempt to run-time link to GLES 3" OFF)

option(USE_BUFFER_RESET "Upload VBOs by requesting a new one each time (default).  This is often the best for driver optimization)" ON)
option(USE_BUFFER_UPDATE "Upload VBOs by updating only the needed portion" OFF)
option(USE_BUFFER_MAPPING "Upload VBOs by mapping to client memory" OFF)



option(BUILD_SHARED "Build SDL_gpu shared libraries" ${DEFAULT_BUILD_SHARED})
option(BUILD_STATIC "Build SDL_gpu static libraries" ${DEFAULT_BUILD_STATIC})


if (USE_SDL1)
    set(SDL_GPU_LIBRARY SDL_gpu)
else (USE_SDL1)
    set(SDL_GPU_LIBRARY SDL2_gpu)
endif (USE_SDL1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/scripts)

if(APPLE)
	include(ios-cmake/toolchain/XcodeDefaults)
endif()

#if ( BUILD_DEBUG )
#	SET(CMAKE_BUILD_TYPE Debug)
#else ( BUILD_DEBUG )
#	SET(CMAKE_BUILD_TYPE Release)
#endif ( BUILD_DEBUG )

if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_C_FLAGS "-std=c99 ${CMAKE_C_FLAGS}")
    endif ()
else ()
    set (CMAKE_C_STANDARD 99)
endif ()

# Find the package for SDL or SDL2
if ( EMSCRIPTEN )
	add_definitions ("-s USE_SDL=2")
elseif ( USE_SDL1 )
	find_package(SDL REQUIRED)

	if ( NOT SDL_FOUND )
	   message ( FATAL_ERROR "SDL not found!" )
	endif ( NOT SDL_FOUND )

	include_directories(${SDL_INCLUDE_DIR})
	link_libraries(${SDL_LIBRARY})
else ( )
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
endif ()

# Find the package for OpenGL
if (DISABLE_OPENGL)
	add_definitions("-DSDL_GPU_DISABLE_OPENGL")
else (DISABLE_OPENGL)

	find_package(OpenGL REQUIRED)
	include_directories(${OPENGL_INCLUDE_DIR})
	if(APPLE AND NOT IOS)
		# CMake incorrectly includes AGL.framework in OPENGL_LIBRARIES which is obsolete.
		# link_libraries(${OPENGL_gl_LIBRARY})
		set(GL_LIBRARIES ${OPENGL_gl_LIBRARY})
	else()
		# link_libraries(${OPENGL_LIBRARIES})
		set(GL_LIBRARIES ${OPENGL_LIBRARIES})
	endif()


	if (DISABLE_OPENGL_1_BASE)
		add_definitions("-DSDL_GPU_DISABLE_OPENGL_1_BASE")
	endif (DISABLE_OPENGL_1_BASE)
	if (DISABLE_OPENGL_1)
		add_definitions("-DSDL_GPU_DISABLE_OPENGL_1")
	endif (DISABLE_OPENGL_1)
	if (DISABLE_OPENGL_2)
		add_definitions("-DSDL_GPU_DISABLE_OPENGL_2")
	endif (DISABLE_OPENGL_2)
	if (DISABLE_OPENGL_3)
		add_definitions("-DSDL_GPU_DISABLE_OPENGL_3")
	endif (DISABLE_OPENGL_3)
	if (DISABLE_OPENGL_4)
		add_definitions("-DSDL_GPU_DISABLE_OPENGL_4")
	endif (DISABLE_OPENGL_4)

	if(USE_SYSTEM_GLEW)
	    # If glew is not found here, we’ll use the bundled version
		find_package(GLEW)
	endif()

    if(NOT GLEW_FOUND)
        message(" Using bundled version of GLEW")
    endif(NOT GLEW_FOUND)


	if (NOT DISABLE_OPENGL)
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
	endif(NOT DISABLE_OPENGL)

endif (DISABLE_OPENGL)

# Find the package for OpenGLES
if (DISABLE_GLES)
	add_definitions("-DSDL_GPU_DISABLE_GLES")
else (DISABLE_GLES)

	if(ANDROID)
	
		if (NOT DISABLE_GLES_1)
			find_library( ANDROID_GLES1_LIBRARY GLESv1_CM )
			link_libraries(
				${ANDROID_GLES1_LIBRARY}
			)
		endif()
		if (NOT DISABLE_GLES_2)
			find_library( ANDROID_GLES2_LIBRARY GLESv2 )
			link_libraries(
				${ANDROID_GLES2_LIBRARY}
			)
		endif()
		link_libraries(
			${ANDROID_LOG_LIBRARY}
		)
	else()
		find_package(OpenGLES REQUIRED)
		include_directories(${OPENGLES_INCLUDE_DIR})
#		link_libraries (${OPENGLES_LIBRARIES})
		set(GL_LIBRARIES ${OPENGLES_LIBRARIES})
	endif()

	if (DISABLE_GLES_1)
		add_definitions("-DSDL_GPU_DISABLE_GLES_1")
	endif (DISABLE_GLES_1)
	if (DISABLE_GLES_2)
		add_definitions("-DSDL_GPU_DISABLE_GLES_2")
	endif (DISABLE_GLES_2)
	if (DISABLE_GLES_3)
		add_definitions("-DSDL_GPU_DISABLE_GLES_3")
	endif (DISABLE_GLES_3)
    
    include_directories(src/externals/gl3stub)
    if (DYNAMIC_GLES_3)
        SET(SDL_gpu_HDRS ${SDL_gpu_HDRS} externals/gl3stub/gl3stub.c externals/gl3stub/gl3stub.h)
        add_definitions("-DSDL_GPU_DYNAMIC_GLES_3")
    endif(DYNAMIC_GLES_3)
endif (DISABLE_GLES)

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
if (BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
        add_custom_target(doc
    `        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
    endif(DOXYGEN_FOUND)
endif (BUILD_DOCS)

add_definitions("-Wall -pedantic")

if (USE_BUFFER_RESET)
    add_definitions("-DSDL_GPU_USE_BUFFER_RESET")
endif (USE_BUFFER_RESET)
if (USE_BUFFER_UPDATE)
    add_definitions("-DSDL_GPU_USE_BUFFER_UPDATE")
endif (USE_BUFFER_UPDATE)
if (USE_BUFFER_MAPPING)
    add_definitions("-DSDL_GPU_USE_BUFFER_MAPPING")
endif (USE_BUFFER_MAPPING)

# Build the SDL_gpu library.
add_subdirectory(src)

if(BUILD_DEMOS OR BUILD_TESTS OR BUILD_TOOLS)
	add_library(test-common STATIC ${SDL_gpu_SOURCE_DIR}/common/common.c ${SDL_gpu_SOURCE_DIR}/common/demo-font.c)
	set(TEST_LIBS test-common SDL_gpu)
endif(BUILD_DEMOS OR BUILD_TESTS OR BUILD_TOOLS)

# Build the demos
if(BUILD_DEMOS)
  add_subdirectory(demos)
endif(BUILD_DEMOS)

# Build the tests
if(BUILD_TESTS)
  if(BUILD_VIDEO_TEST)

	find_package(FFMPEG REQUIRED)
	include_directories(${FFMPEG_INCLUDE_DIR})
	link_libraries (${FFMPEG_LIBRARIES})

    add_definitions("-DSDL_GPU_BUILD_VIDEO_TEST")
  endif(BUILD_VIDEO_TEST)

  add_subdirectory(tests)
endif(BUILD_TESTS)

# Build sample tools
if(BUILD_TOOLS)
  add_subdirectory(tools)
endif(BUILD_TOOLS)

