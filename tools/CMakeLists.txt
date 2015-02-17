include_directories (${SDL_gpu_SOURCE_DIR})
include_directories (${SDL_gpu_SOURCE_DIR}/common)

set(COMMON_SOURCE ${SDL_gpu_SOURCE_DIR}/common/common.c ${SDL_gpu_SOURCE_DIR}/common/demo-font.c)

add_executable(compare-images compare-images-src/main.c ${COMMON_SOURCE})
target_link_libraries (compare-images SDL_gpu)

add_executable(thumb-viewer thumb-viewer-src/main.c ${COMMON_SOURCE})
target_link_libraries (thumb-viewer SDL_gpu)