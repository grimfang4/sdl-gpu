set(SDL_gpuShapes_SRCS
	SDL_gpuShapes.c
)
set(SDL_gpuShapes_HDRS
	SDL_gpuShapes.h
)

if(SDL_gpuShapes_BUILD_SHARED)
	add_library(SDL_gpuShapes_shared SHARED
		${SDL_gpuShapes_HDRS}
		${SDL_gpuShapes_SRCS}
	)
	set_target_properties(SDL_gpuShapes_shared PROPERTIES
		OUTPUT_NAME "SDL_gpuShapes"
		CLEAN_DIRECT_OUTPUT 1
		VERSION ${SDL_gpuShapes_VERSION}
	)
	set_target_properties(SDL_gpuShapes_shared PROPERTIES LINKER_LANGUAGE C)
endif()

if(SDL_gpuShapes_BUILD_STATIC)
	add_library(SDL_gpuShapes STATIC
		${SDL_gpuShapes_HDRS}
		${SDL_gpuShapes_SRCS}
	)
	set_target_properties(SDL_gpuShapes PROPERTIES
		CLEAN_DIRECT_OUTPUT 1
		VERSION ${SDL_gpuShapes_VERSION}
	)
	set_target_properties(SDL_gpuShapes PROPERTIES LINKER_LANGUAGE C)
endif()

# These are used to create visual studio folders.
source_group(Common FILES ${SDL_gpuShapes_SRCS} ${SDL_gpuShapes_HDRS})

if(SDL_gpuShapes_INSTALL)
	# install headers
	install(FILES ${SDL_gpuShapes_HDRS} DESTINATION include/SDL)

	# install libraries
	if(SDL_gpuShapes_BUILD_SHARED)
		install(TARGETS SDL_gpuShapes_shared EXPORT SDL_gpuShapes-targets DESTINATION .)
	endif()
	if(SDL_gpuShapes_BUILD_STATIC)
		install(TARGETS SDL_gpuShapes EXPORT SDL_gpuShapes-targets DESTINATION .)
	endif()

	# install build system hooks for third-party apps
	install(EXPORT SDL_gpuShapes-targets DESTINATION ./SDL_gpuShapes)
	install(FILES SDL_gpuShapesConfig.cmake DESTINATION ./SDL_gpuShapes)
endif(SDL_gpuShapes_INSTALL)