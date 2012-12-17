include_directories (${SDL_gpu_SOURCE_DIR})

add_executable(compare-images compare-images-src/main.c)
target_link_libraries (compare-images SDL_gpu)

add_executable(thumb-viewer thumb-viewer-src/main.c)
target_link_libraries (thumb-viewer SDL_gpu)