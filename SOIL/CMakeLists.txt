set(SOIL_SRCS
	SOIL.c
	image_DXT.c
	image_helper.c
	stb_image_aug.c
)
set(SOIL_HDRS
	SOIL.h
	image_DXT.h
	image_helper.h
	stb_image_aug.h
	stbi_DDS_aug.h
	stbi_DDS_aug_c.h
)

# Build static lib
add_library(SOIL STATIC
	${SOIL_HDRS}
	${SOIL_SRCS}
)
set_target_properties(SOIL PROPERTIES
	CLEAN_DIRECT_OUTPUT 1
	VERSION ${SOIL_VERSION}
)
set_target_properties(SOIL PROPERTIES LINKER_LANGUAGE C)

# These are used to create visual studio folders.
source_group(Common FILES ${SOIL_SRCS} ${SOIL_HDRS})

if(SDL_gpu_BUILD_SHARED OR SDL_gpu_BUILD_STATIC)
	#install(TARGETS SOIL EXPORT SDL_gpu-targets DESTINATION lib)
endif()

if(SDL_gpuShapes_BUILD_SHARED OR SDL_gpuShapes_BUILD_STATIC)
	#install(TARGETS SOIL EXPORT SDL_gpuShapes-targets DESTINATION lib)
endif()

#install(EXPORT SDL_gpu-targets DESTINATION ../SDL_gpu)