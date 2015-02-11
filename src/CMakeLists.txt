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
	renderer_GLES_1.c
	renderer_GLES_2.c
)

set(SDL_gpu_HDRS
	${SDL_gpu_HDRS}
	../include/SDL_gpu.h
	../include/SDL_gpu_RendererImpl.h
	../include/SDL_gpu_OpenGL_1_BASE.h
	../include/SDL_gpu_OpenGL_1.h
	../include/SDL_gpu_OpenGL_2.h
	../include/SDL_gpu_OpenGL_3.h
	../include/SDL_gpu_GLES_1.h
	../include/SDL_gpu_GLES_2.h
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
	SET(SDL_gpu_SRCS ${SDL_gpu_SRCS} externals/stb_image/stb_image_write.c)
	SET(SDL_gpu_HDRS ${SDL_gpu_HDRS} externals/stb_image/stb_image_write.h)
endif(STBI_WRITE_FOUND)


set(SDL_gpu_install_HDRS
	../include/SDL_gpu.h
	../include/SDL_gpu_RendererImpl.h
	../include/SDL_gpu_OpenGL_1_BASE.h
	../include/SDL_gpu_OpenGL_1.h
	../include/SDL_gpu_OpenGL_2.h
	../include/SDL_gpu_OpenGL_3.h
	../include/SDL_gpu_GLES_1.h
	../include/SDL_gpu_GLES_2.h
)

# Set the appropriate library name for the version of SDL used
if ( SDL_gpu_USE_SDL1)
	set (SDL_gpu_OUTPUT_NAME "SDL_gpu")
else ( SDL_gpu_USE_SDL1)
	set (SDL_gpu_OUTPUT_NAME "SDL2_gpu")
endif ( SDL_gpu_USE_SDL1)

# Build the shared library (.so or .dll)
if(SDL_gpu_BUILD_SHARED)
	add_library(SDL_gpu_shared SHARED
		${SDL_gpu_HDRS}
		${SDL_gpu_SRCS}
	)
	set_target_properties(SDL_gpu_shared PROPERTIES
		OUTPUT_NAME ${SDL_gpu_OUTPUT_NAME}
		CLEAN_DIRECT_OUTPUT 1
		VERSION ${SDL_gpu_VERSION}
	)
	set_target_properties(SDL_gpu_shared PROPERTIES LINKER_LANGUAGE C)
endif()

# Build the static library (.a or .lib)
if(SDL_gpu_BUILD_STATIC)
	add_library(SDL_gpu STATIC
		${SDL_gpu_HDRS}
		${SDL_gpu_SRCS}
	)
	set_target_properties(SDL_gpu PROPERTIES
		OUTPUT_NAME ${SDL_gpu_OUTPUT_NAME}
		CLEAN_DIRECT_OUTPUT 1
		VERSION ${SDL_gpu_VERSION}
	)
	set_target_properties(SDL_gpu PROPERTIES LINKER_LANGUAGE C)
endif()

# These are used to create visual studio folders.
source_group(Common FILES ${SDL_gpu_SRCS} ${SDL_gpu_HDRS})

# Install the headers and library
if(SDL_gpu_INSTALL)
	install(FILES ${SDL_gpu_install_HDRS} DESTINATION include/SDL)

	if(SDL_gpu_BUILD_SHARED)
		install(TARGETS SDL_gpu_shared EXPORT SDL_gpu-targets DESTINATION lib)
	endif()
	if(SDL_gpu_BUILD_STATIC)
		install(TARGETS SDL_gpu EXPORT SDL_gpu-targets DESTINATION lib)
	endif()

endif(SDL_gpu_INSTALL)
