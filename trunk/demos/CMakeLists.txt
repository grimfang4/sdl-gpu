include_directories (${SDL_gpu_SOURCE_DIR})
add_executable(create-demo create/main.c)
target_link_libraries (create-demo SDL_gpu)

add_executable(get-pixel-demo get-pixel/main.c)
target_link_libraries (get-pixel-demo SDL_gpu)

add_executable(scale-demo scale/main.c)
target_link_libraries (scale-demo SDL_gpu)

add_executable(transform-demo transform/main.c)
target_link_libraries (transform-demo SDL_gpu)

add_executable(sprite-demo sprite/main.c)
target_link_libraries (sprite-demo SDL_gpu)

add_executable(rotate-demo rotate/main.c)
target_link_libraries (rotate-demo SDL_gpu)

add_executable(render-target-demo render-target/main.c)
target_link_libraries (render-target-demo SDL_gpu)

add_executable(blend-demo blend/main.c)
target_link_libraries (blend-demo SDL_gpu)

add_executable(blend-mode-demo blend-mode/main.c)
target_link_libraries (blend-mode-demo SDL_gpu)

add_executable(shapes-demo shapes/main.c)
target_link_libraries (shapes-demo SDL_gpu)

add_executable(clip-demo clip/main.c)
target_link_libraries (clip-demo SDL_gpu)

add_executable(color-demo color/main.c)
target_link_libraries (color-demo SDL_gpu)

add_executable(mipmap-demo mipmap/main.c)
target_link_libraries (mipmap-demo SDL_gpu)

add_executable(polygon-blit-demo polygon-blit/main.c)
target_link_libraries (polygon-blit-demo SDL_gpu)

add_executable(camera-demo camera/main.c)
target_link_libraries (camera-demo SDL_gpu)

add_executable(virtual-resolution-demo virtual-resolution/main.c)
target_link_libraries (virtual-resolution-demo SDL_gpu)

add_executable(transformx-demo transformx/main.c)
target_link_libraries (transformx-demo SDL_gpu)

add_executable(transform-matrix-demo transform-matrix/main.c)
target_link_libraries (transform-matrix-demo SDL_gpu)

add_executable(copy-demo copy/main.c)
target_link_libraries (copy-demo SDL_gpu)

add_executable(npot-demo npot/main.c)
target_link_libraries (npot-demo SDL_gpu)