include_directories (${SDL_gpu_SOURCE_DIR})
add_executable(scaling-demo scaling/main.c)
target_link_libraries (scaling-demo SDL_gpu) 
