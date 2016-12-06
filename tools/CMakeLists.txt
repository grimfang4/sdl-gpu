include_directories (${SDL_gpu_SOURCE_DIR})
include_directories (${SDL_gpu_SOURCE_DIR}/common)
link_directories (${SDL_gpu_SOURCE_DIR}/src)

set(TOOL_LIBS test-common ${SDL_gpu_LIBRARY})

add_executable(compare-images compare-images-src/main.c)
target_link_libraries (compare-images ${TOOL_LIBS})

add_executable(thumb-viewer thumb-viewer-src/main.c)
target_link_libraries (thumb-viewer ${TOOL_LIBS})