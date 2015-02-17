include_directories (${SDL_gpu_SOURCE_DIR})

add_executable(init-demo init/main.c)
target_link_libraries (init-demo SDL_gpu)

add_executable(3d-demo 3d/main.c)
target_link_libraries (3d-demo SDL_gpu)

add_executable(tutorial-space-demo tutorial-space/main.c)
target_link_libraries (tutorial-space-demo SDL_gpu)