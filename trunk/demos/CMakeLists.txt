include_directories (${SDL_gpu_SOURCE_DIR})
include_directories(${SDL_gpu_SOURCE_DIR}/SDL_gpu/OpenGL_common)
include_directories(${SDL_gpu_SOURCE_DIR}/demos/common)

add_executable(init-demo init/main.c common/common.c common/demo-font.c)
target_link_libraries (init-demo SDL_gpu)

add_executable(create-demo create/main.c common/common.c common/demo-font.c)
target_link_libraries (create-demo SDL_gpu)

add_executable(get-pixel-demo get-pixel/main.c common/common.c common/demo-font.c)
target_link_libraries (get-pixel-demo SDL_gpu)

add_executable(scale-demo scale/main.c common/common.c common/demo-font.c)
target_link_libraries (scale-demo SDL_gpu)

add_executable(transform-demo transform/main.c common/common.c common/demo-font.c)
target_link_libraries (transform-demo SDL_gpu)

add_executable(sprite-demo sprite/main.c common/common.c common/demo-font.c)
target_link_libraries (sprite-demo SDL_gpu)

add_executable(sprite-stress-demo sprite-stress/main.c common/common.c common/demo-font.c)
target_link_libraries (sprite-stress-demo SDL_gpu)

add_executable(rotate-demo rotate/main.c common/common.c common/demo-font.c)
target_link_libraries (rotate-demo SDL_gpu)

add_executable(render-target-demo render-target/main.c common/common.c common/demo-font.c)
target_link_libraries (render-target-demo SDL_gpu)

add_executable(blend-demo blend/main.c common/common.c common/demo-font.c)
target_link_libraries (blend-demo SDL_gpu)

add_executable(blend-mode-demo blend-mode/main.c common/common.c common/demo-font.c)
target_link_libraries (blend-mode-demo SDL_gpu)

add_executable(shapes-demo shapes/main.c common/common.c common/demo-font.c)
target_link_libraries (shapes-demo SDL_gpu)

add_executable(clip-demo clip/main.c common/common.c common/demo-font.c)
target_link_libraries (clip-demo SDL_gpu)

add_executable(color-demo color/main.c common/common.c common/demo-font.c)
target_link_libraries (color-demo SDL_gpu)

add_executable(mipmap-demo mipmap/main.c common/common.c common/demo-font.c)
target_link_libraries (mipmap-demo SDL_gpu)

add_executable(polygon-blit-demo polygon-blit/main.c common/common.c common/demo-font.c)
target_link_libraries (polygon-blit-demo SDL_gpu)

add_executable(camera-demo camera/main.c common/common.c common/demo-font.c)
target_link_libraries (camera-demo SDL_gpu)

add_executable(virtual-resolution-demo virtual-resolution/main.c common/common.c common/demo-font.c)
target_link_libraries (virtual-resolution-demo SDL_gpu)

add_executable(transformx-demo transformx/main.c common/common.c common/demo-font.c)
target_link_libraries (transformx-demo SDL_gpu)

add_executable(transform-matrix-demo transform-matrix/main.c common/common.c common/demo-font.c)
target_link_libraries (transform-matrix-demo SDL_gpu)

add_executable(copy-demo copy/main.c common/common.c common/demo-font.c)
target_link_libraries (copy-demo SDL_gpu)

add_executable(npot-demo npot/main.c common/common.c common/demo-font.c)
target_link_libraries (npot-demo SDL_gpu)

add_executable(features-demo features/main.c common/common.c common/demo-font.c)
target_link_libraries (features-demo SDL_gpu)

add_executable(save-demo save/main.c common/common.c common/demo-font.c)
target_link_libraries (save-demo SDL_gpu)

add_executable(shader-demo shader/main.c common/common.c common/demo-font.c)
target_link_libraries (shader-demo SDL_gpu)

add_executable(3d-demo 3d/main.c common/common.c common/demo-font.c)
target_link_libraries (3d-demo SDL_gpu)

add_executable(windows-demo windows/main.c common/common.c common/demo-font.c)
target_link_libraries (windows-demo SDL_gpu)

add_executable(windows2-demo windows2/main.c common/common.c common/demo-font.c)
target_link_libraries (windows2-demo SDL_gpu)

add_executable(shader-attributes-demo shader-attributes/main.c common/common.c common/demo-font.c)
target_link_libraries (shader-attributes-demo SDL_gpu)

add_executable(blit-batch-demo blit-batch/main.c common/common.c common/demo-font.c)
target_link_libraries (blit-batch-demo SDL_gpu)

add_executable(viewport-demo viewport/main.c common/common.c common/demo-font.c)
target_link_libraries (viewport-demo SDL_gpu)

add_executable(multitexture-demo multitexture/main.c common/common.c common/demo-font.c)
target_link_libraries (multitexture-demo SDL_gpu)

add_executable(intermediate-demo intermediate/main.c common/common.c common/demo-font.c)
target_link_libraries (intermediate-demo SDL_gpu)

add_executable(pixel-perfect-demo pixel-perfect/main.c common/common.c common/demo-font.c)
target_link_libraries (pixel-perfect-demo SDL_gpu)

add_executable(alias-demo alias/main.c common/common.c common/demo-font.c)
target_link_libraries (alias-demo SDL_gpu)

add_executable(triangle-batch-demo triangle-batch/main.c common/common.c common/demo-font.c)
target_link_libraries (triangle-batch-demo SDL_gpu)

add_executable(wrap-demo wrap/main.c common/common.c common/demo-font.c)
target_link_libraries (wrap-demo SDL_gpu)

add_executable(upload-image-demo upload-image/main.c common/common.c common/demo-font.c)
target_link_libraries (upload-image-demo SDL_gpu)

add_executable(tutorial-space-demo tutorial-space/main.c common/common.c)
target_link_libraries (tutorial-space-demo SDL_gpu)

add_executable(image-formats-demo image-formats/main.c common/common.c common/demo-font.c)
target_link_libraries (image-formats-demo SDL_gpu)