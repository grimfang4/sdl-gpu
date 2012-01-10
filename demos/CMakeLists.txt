include_directories (${SDL_gpu_SOURCE_DIR})
add_executable(scale-demo scale/main.c)
target_link_libraries (scale-demo SDL_gpu)

add_executable(sprite-demo sprite/main.c)
target_link_libraries (sprite-demo SDL_gpu)

add_executable(rotate-demo rotate/main.c)
target_link_libraries (rotate-demo SDL_gpu)

add_executable(render-target-demo render-target/main.c)
target_link_libraries (render-target-demo SDL_gpu SDL_gpuShapes)

add_executable(blend-demo blend/main.c)
target_link_libraries (blend-demo SDL_gpu)

add_executable(shapes-demo shapes/main.c)
target_link_libraries (shapes-demo SDL_gpu SDL_gpuShapes)

add_executable(clip-demo clip/main.c)
target_link_libraries (clip-demo SDL_gpu SDL_gpuShapes)
