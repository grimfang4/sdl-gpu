include_directories (${SDL_gpu_SOURCE_DIR})
include_directories (${SDL_gpu_SOURCE_DIR}/common)

set(TEST_LIBS test-common SDL_gpu)

add_executable(compare-images compare-images-src/main.c)
target_link_libraries (compare-images ${TEST_LIBS})

add_executable(thumb-viewer thumb-viewer-src/main.c)
target_link_libraries (thumb-viewer ${TEST_LIBS})