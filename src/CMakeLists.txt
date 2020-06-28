set(SDL_gpu_SRCS
	${SDL_gpu_SRCS}
	SDL_gpu.c
	SDL_gpu_matrix.c
	SDL_gpu_renderer.c
	SDL_gpu_shapes.c
	renderer_OpenGL_1_BASE.c
	renderer_OpenGL_1.c
	renderer_OpenGL_2.c
	renderer_OpenGL_3.c
	renderer_OpenGL_4.c
	renderer_GLES_1.c
	renderer_GLES_2.c
	renderer_GLES_3.c
)

set(SDL_gpu_HDRS
	${SDL_gpu_HDRS}
	../include/SDL_gpu.h
	../include/SDL_gpu_version.h
	../include/SDL_gpu_RendererImpl.h
	../include/SDL_gpu_OpenGL_1_BASE.h
	../include/SDL_gpu_OpenGL_1.h
	../include/SDL_gpu_OpenGL_2.h
	../include/SDL_gpu_OpenGL_3.h
	../include/SDL_gpu_OpenGL_4.h
	../include/SDL_gpu_GLES_1.h
	../include/SDL_gpu_GLES_2.h
	../include/SDL_gpu_GLES_3.h
	renderer_GL_common.inl
	renderer_shapes_GL_common.inl
)

if(STBI_FOUND)
	include_directories(${STBI_INCLUDE_DIR})
	link_libraries (${STBI_LIBRARY})
else(STBI_FOUND)
	SET(SDL_gpu_SRCS ${SDL_gpu_SRCS} externals/stb_image/stb_image.c)
	SET(SDL_gpu_HDRS ${SDL_gpu_HDRS} externals/stb_image/stb_image.h)
endif(STBI_FOUND)

if(STBI_WRITE_FOUND)
	include_directories(${STBI_WRITE_INCLUDE_DIR})
	link_libraries (${STBI_WRITE_LIBRARY})
else(STBI_WRITE_FOUND)
	SET(SDL_gpu_SRCS ${SDL_gpu_SRCS} externals/stb_image_write/stb_image_write.c)
	SET(SDL_gpu_HDRS ${SDL_gpu_HDRS} externals/stb_image_write/stb_image_write.h)
endif(STBI_WRITE_FOUND)


set(SDL_gpu_public_HDRS
	../include/SDL_gpu.h
	../include/SDL_gpu_version.h
	../include/SDL_gpu_RendererImpl.h
	../include/SDL_gpu_OpenGL_1_BASE.h
	../include/SDL_gpu_OpenGL_1.h
	../include/SDL_gpu_OpenGL_2.h
	../include/SDL_gpu_OpenGL_3.h
	../include/SDL_gpu_OpenGL_4.h
	../include/SDL_gpu_GLES_1.h
	../include/SDL_gpu_GLES_2.h
	../include/SDL_gpu_GLES_3.h
)

# Set the appropriate library name for the version of SDL used
if ( USE_SDL1)
	set (LIBRARY_OUTPUT_NAME "SDL_gpu")
else ( USE_SDL1)
	set (LIBRARY_OUTPUT_NAME "SDL2_gpu")
endif ( USE_SDL1)

set (SHARED_OUTPUT ${LIBRARY_OUTPUT_NAME})

if(WIN32)
    set (STATIC_OUTPUT ${LIBRARY_OUTPUT_NAME}_s)
else(WIN32)
    set (STATIC_OUTPUT ${LIBRARY_OUTPUT_NAME})
endif(WIN32)

if(MSVC)
    set(OUTPUT_DIR SDL_gpu-VS)
elseif(MINGW)
    set(OUTPUT_DIR SDL_gpu-MINGW)
else()
    set(OUTPUT_DIR SDL_gpu)
endif()

if(APPLE)
	set(CMAKE_MACOSX_RPATH 1)
	# use, i.e. don't skip the full RPATH for the build tree
	# SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

	# when building, don't use the install RPATH already
	# (but later on when installing)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

	# add the automatically determined parts of the RPATH
	# which point to directories outside the build tree to the install RPATH
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
endif()


# Copy includes to output ${OUTPUT_DIR}/include
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTPUT_DIR})
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/bin)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/include)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/lib)
foreach(INC_FILE ${SDL_gpu_public_HDRS})
    get_filename_component(BASE ${INC_FILE} NAME)
    configure_file(${INC_FILE} ${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/include/${BASE} COPYONLY)
endforeach(INC_FILE)

configure_file(../LICENSE.txt ${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/LICENSE.txt COPYONLY)
configure_file(../README.md ${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/README.md COPYONLY)

# Build the shared library (.so or .dll)
if(BUILD_SHARED)
	add_library(SDL_gpu_shared SHARED
		${SDL_gpu_HDRS}
		${SDL_gpu_SRCS}
	)
	
	target_link_libraries(SDL_gpu_shared ${GL_LIBRARIES})

	if(BUILD_FRAMEWORK)
        if(NOT CMAKE_VERSION VERSION_LESS "3.1")
            set_property(TARGET SDL_gpu_shared PROPERTY C_STANDARD 99)
        endif()
		set_target_properties(SDL_gpu_shared PROPERTIES
			FRAMEWORK TRUE
			FRAMEWORK_VERSION "A"
			PUBLIC_HEADER "${SDL_gpu_public_HDRS}"
			#RESOURCE "${RESOURCE_FILES}"
			VERSION ${SDL_GPU_VERSION}
			SOVERSION ${SDL_GPU_VERSION}
			OUTPUT_NAME ${SHARED_OUTPUT}
			ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/lib"
			LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/lib"
			RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/bin"
		)
		# Short Version is the "marketing version". It is the version
		# the user sees in an information panel.
		SET(MACOSX_FRAMEWORK_SHORT_VERSION_STRING "${SDL_GPU_VERSION}")
		# Bundle version is the version the OS looks at.
		SET(MACOSX_FRAMEWORK_BUNDLE_VERSION "${SDL_GPU_VERSION}")
		SET(MACOSX_FRAMEWORK_IDENTIFIER "com.dinomage.sdl-gpu")



	else()
		if(ANDROID)
			set_target_properties(SDL_gpu_shared PROPERTIES
				OUTPUT_NAME ${SHARED_OUTPUT}
				CLEAN_DIRECT_OUTPUT 1
				ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/lib"
				LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/lib"
				RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/bin"
			)
		else()
			set_target_properties(SDL_gpu_shared PROPERTIES
				OUTPUT_NAME ${SHARED_OUTPUT}
				CLEAN_DIRECT_OUTPUT 1
#				VERSION ${SDL_GPU_VERSION}
				ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/lib"
				LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/lib"
				RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/bin"
			)
		endif()

	endif()
	set_target_properties(SDL_gpu_shared PROPERTIES LINKER_LANGUAGE C)
	if(APPLE)
		CONFIGURE_XCODE_DEFAULTS(SDL_gpu_shared)
	endif()
endif()

# Build the static library (.a or .lib)
if(BUILD_STATIC)
	add_library(SDL_gpu STATIC
		${SDL_gpu_HDRS}
		${SDL_gpu_SRCS}
	)
    if(NOT CMAKE_VERSION VERSION_LESS "3.1")
        set_property(TARGET SDL_gpu PROPERTY C_STANDARD 99)
    endif()
	set_target_properties(SDL_gpu PROPERTIES
		OUTPUT_NAME ${STATIC_OUTPUT}
		CLEAN_DIRECT_OUTPUT 1
		VERSION ${SDL_GPU_VERSION}
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/lib"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/lib"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DIR}/bin"
	)
	set_target_properties(SDL_gpu PROPERTIES LINKER_LANGUAGE C)
	if(APPLE)
		CONFIGURE_XCODE_DEFAULTS(SDL_gpu)	
	endif()
endif()

# These are used to create visual studio folders.
source_group(Common FILES ${SDL_gpu_SRCS} ${SDL_gpu_HDRS})

# Install the headers and library
if(INSTALL_LIBRARY)
	if(USE_SDL1)
		install(FILES ${SDL_gpu_public_HDRS} DESTINATION include/SDL)
	else(USE_SDL1)
		install(FILES ${SDL_gpu_public_HDRS} DESTINATION include/SDL2)
	endif(USE_SDL1)

	if(BUILD_SHARED)
		install(TARGETS SDL_gpu_shared EXPORT SDL_gpu-targets DESTINATION lib)
	endif()
	if(BUILD_STATIC)
		install(TARGETS SDL_gpu EXPORT SDL_gpu-targets DESTINATION lib)
	endif()

endif(INSTALL_LIBRARY)
