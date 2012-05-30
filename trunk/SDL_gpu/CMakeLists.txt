set(SDL_gpu_SRCS
	SDL_gpu.c
	SDL_gpuShapes.c
	SDL_gpu_Renderer.c
	OpenGL/SDL_gpu_OpenGL.c
	OpenGL/SDL_gpuShapes_OpenGL.c
	OpenGL/glew.c
	OpenGL/SOIL/SOIL.c
	OpenGL/SOIL/image_DXT.c
	OpenGL/SOIL/image_helper.c
	OpenGL/SOIL/stb_image_aug.c
	Direct3D/SDL_gpu_Direct3D.c
	Direct3D/SDL_gpuShapes_Direct3D.c
)
set(SDL_gpu_HDRS
	SDL_gpu.h
	SDL_gpuShapes.h
	SDL_gpu_Renderer.h
	OpenGL/SDL_gpu_OpenGL.h
	OpenGL/SDL_gpu_OpenGL_internal.h
	OpenGL/SDL_gpuShapes_OpenGL.h
	OpenGL/SDL_gpuShapes_OpenGL_internal.h
	OpenGL/glew.h
	OpenGL/glxew.h
	OpenGL/wglew.h
	OpenGL/SOIL/SOIL.h
	OpenGL/SOIL/image_DXT.h
	OpenGL/SOIL/image_helper.h
	OpenGL/SOIL/stb_image_aug.h
	OpenGL/SOIL/stbi_DDS_aug.h
	OpenGL/SOIL/stbi_DDS_aug_c.h
	Direct3D/SDL_gpu_Direct3D.h
	Direct3D/SDL_gpu_Direct3D_internal.h
	Direct3D/SDL_gpuShapes_Direct3D.h
	Direct3D/SDL_gpuShapes_Direct3D_internal.h
)
set(SDL_gpu_install_HDRS
	SDL_gpu.h
	OpenGL/SDL_gpu_OpenGL.h
	OpenGL/SDL_gpuShapes_OpenGL.h
	Direct3D/SDL_gpu_Direct3D.h
	Direct3D/SDL_gpuShapes_Direct3D.h
)

if(SDL_gpu_BUILD_SHARED)
	add_library(SDL_gpu_shared SHARED
		${SDL_gpu_HDRS}
		${SDL_gpu_SRCS}
	)
	set_target_properties(SDL_gpu_shared PROPERTIES
		OUTPUT_NAME "SDL_gpu"
		CLEAN_DIRECT_OUTPUT 1
		VERSION ${SDL_gpu_VERSION}
	)
	set_target_properties(SDL_gpu_shared PROPERTIES LINKER_LANGUAGE C)
endif()

if(SDL_gpu_BUILD_STATIC)
	add_library(SDL_gpu STATIC
		${SDL_gpu_HDRS}
		${SDL_gpu_SRCS}
	)
	set_target_properties(SDL_gpu PROPERTIES
		CLEAN_DIRECT_OUTPUT 1
		VERSION ${SDL_gpu_VERSION}
	)
	set_target_properties(SDL_gpu PROPERTIES LINKER_LANGUAGE C)
endif()

# These are used to create visual studio folders.
source_group(Common FILES ${SDL_gpu_SRCS} ${SDL_gpu_HDRS})

if(SDL_gpu_INSTALL)
	# install headers
	install(FILES ${SDL_gpu_install_HDRS} DESTINATION include/SDL)

	# install libraries
	if(SDL_gpu_BUILD_SHARED)
		install(TARGETS SDL_gpu_shared EXPORT SDL_gpu-targets DESTINATION lib)
	endif()
	if(SDL_gpu_BUILD_STATIC)
		install(TARGETS SDL_gpu EXPORT SDL_gpu-targets DESTINATION lib)
	endif()

	# install build system hooks for third-party apps
	#install(EXPORT SDL_gpu-targets DESTINATION ./SDL_gpu)
	#install(FILES SDL_gpuConfig.cmake DESTINATION ./SDL_gpu)
endif(SDL_gpu_INSTALL)