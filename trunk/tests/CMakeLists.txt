include_directories (${SDL_gpu_SOURCE_DIR})
include_directories (${SDL_gpu_SOURCE_DIR}/demos/common)

add_executable(create-test create/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (create-test SDL_gpu)

add_executable(get-pixel-test get-pixel/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (get-pixel-test SDL_gpu)

add_executable(scale-test scale/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (scale-test SDL_gpu)

add_executable(transform-test transform/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (transform-test SDL_gpu)

add_executable(shapes-test shapes/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (shapes-test SDL_gpu)

add_executable(sprite-test sprite/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (sprite-test SDL_gpu)

add_executable(sprite-stress-test sprite-stress/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (sprite-stress-test SDL_gpu)

add_executable(rotate-test rotate/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (rotate-test SDL_gpu)

add_executable(render-target-test render-target/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (render-target-test SDL_gpu)

add_executable(blend-test blend/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (blend-test SDL_gpu)

add_executable(blend-mode-test blend-mode/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (blend-mode-test SDL_gpu)

add_executable(clip-test clip/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (clip-test SDL_gpu)

add_executable(color-test color/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (color-test SDL_gpu)

add_executable(mipmap-test mipmap/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (mipmap-test SDL_gpu)

add_executable(polygon-blit-test polygon-blit/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (polygon-blit-test SDL_gpu)

add_executable(camera-test camera/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (camera-test SDL_gpu)

add_executable(virtual-resolution-test virtual-resolution/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (virtual-resolution-test SDL_gpu)

add_executable(transformx-test transformx/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (transformx-test SDL_gpu)

add_executable(transform-matrix-test transform-matrix/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (transform-matrix-test SDL_gpu)

add_executable(copy-test copy/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (copy-test SDL_gpu)

add_executable(npot-test npot/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (npot-test SDL_gpu)

add_executable(features-test features/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (features-test SDL_gpu)

add_executable(save-test save/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (save-test SDL_gpu)

add_executable(shader-test shader/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (shader-test SDL_gpu)

add_executable(windows-test windows/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (windows-test SDL_gpu)

add_executable(windows2-test windows2/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (windows2-test SDL_gpu)

add_executable(shader-attributes-test shader-attributes/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (shader-attributes-test SDL_gpu)

add_executable(blit-batch-test blit-batch/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (blit-batch-test SDL_gpu)

add_executable(viewport-test viewport/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (viewport-test SDL_gpu)

add_executable(multitexture-test multitexture/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (multitexture-test SDL_gpu)

add_executable(intermediate-test intermediate/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (intermediate-test SDL_gpu)

add_executable(pixel-perfect-test pixel-perfect/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (pixel-perfect-test SDL_gpu)

add_executable(alias-test alias/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (alias-test SDL_gpu)

add_executable(triangle-batch-test triangle-batch/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (triangle-batch-test SDL_gpu)

add_executable(wrap-test wrap/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (wrap-test SDL_gpu)

add_executable(upload-image-test upload-image/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (upload-image-test SDL_gpu)

add_executable(image-formats-test image-formats/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (image-formats-test SDL_gpu)

add_executable(sandbox-test sandbox/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (sandbox-test SDL_gpu)

add_executable(renderer-test renderer/main.c ../demos/common/common.c)
target_link_libraries (renderer-test SDL_gpu)

add_executable(video-test video/main.c ../demos/common/common.c ../demos/common/demo-font.c)
target_link_libraries (video-test SDL_gpu)