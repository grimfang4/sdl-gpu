include_directories (${SDL_gpu_SOURCE_DIR})
include_directories (${SDL_gpu_SOURCE_DIR}/common)
link_directories (${SDL_gpu_SOURCE_DIR}/src)

set(TEST_LIBS test-common SDL_gpu ${GL_LIBRARIES})

if(MINGW)
    set(TEST_LIBS ${TEST_LIBS} mingw32)
endif(MINGW)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(EMCC_COMPILER_FLAGS "-O2 -s USE_WEBGL2=1 --preload-file ../../tests/data@/data --emrun")
    set(EMCC_LINKER_FLAGS ${EMCC_COMPILER_FLAGS})
    add_definitions(${EMCC_COMPILER_FLAGS})
    
    add_executable(sprite-emscripten-test sprite-emscripten/main.c)
    set_target_properties(sprite-emscripten-test PROPERTIES LINK_FLAGS "${EMCC_LINKER_FLAGS}")
    target_link_libraries (sprite-emscripten-test ${TEST_LIBS})
    
    return()
endif(EMSCRIPTEN)

SET(CMAKE_BUILD_TYPE Debug)

add_executable(create-test create/main.c)
target_link_libraries (create-test ${TEST_LIBS})

add_executable(get-pixel-test get-pixel/main.c)
target_link_libraries (get-pixel-test ${TEST_LIBS})

add_executable(scale-test scale/main.c)
target_link_libraries (scale-test ${TEST_LIBS})

add_executable(transform-test transform/main.c)
target_link_libraries (transform-test ${TEST_LIBS})

add_executable(shapes-test shapes/main.c)
target_link_libraries (shapes-test ${TEST_LIBS})

add_executable(sprite-test sprite/main.c)
target_link_libraries (sprite-test ${TEST_LIBS})

add_executable(sprite-stress-test sprite-stress/main.c)
target_link_libraries (sprite-stress-test ${TEST_LIBS})

add_executable(rotate-test rotate/main.c)
target_link_libraries (rotate-test ${TEST_LIBS})

add_executable(render-target-test render-target/main.c)
target_link_libraries (render-target-test ${TEST_LIBS})

add_executable(blend-test blend/main.c)
target_link_libraries (blend-test ${TEST_LIBS})

add_executable(blend-mode-test blend-mode/main.c)
target_link_libraries (blend-mode-test ${TEST_LIBS})

add_executable(clip-test clip/main.c)
target_link_libraries (clip-test ${TEST_LIBS})

add_executable(color-test color/main.c)
target_link_libraries (color-test ${TEST_LIBS})

add_executable(mipmap-test mipmap/main.c)
target_link_libraries (mipmap-test ${TEST_LIBS})

add_executable(polygon-blit-test polygon-blit/main.c)
target_link_libraries (polygon-blit-test ${TEST_LIBS})

add_executable(camera-test camera/main.c)
target_link_libraries (camera-test ${TEST_LIBS})

add_executable(virtual-resolution-test virtual-resolution/main.c)
target_link_libraries (virtual-resolution-test ${TEST_LIBS})

add_executable(transformx-test transformx/main.c)
target_link_libraries (transformx-test ${TEST_LIBS})

add_executable(copy-test copy/main.c)
target_link_libraries (copy-test ${TEST_LIBS})

add_executable(npot-test npot/main.c)
target_link_libraries (npot-test ${TEST_LIBS})

add_executable(features-test features/main.c)
target_link_libraries (features-test ${TEST_LIBS})

add_executable(save-test save/main.c)
target_link_libraries (save-test ${TEST_LIBS})

add_executable(shader-test shader/main.c)
target_link_libraries (shader-test ${TEST_LIBS})

add_executable(windows-test windows/main.c)
target_link_libraries (windows-test ${TEST_LIBS})

add_executable(windows2-test windows2/main.c)
target_link_libraries (windows2-test ${TEST_LIBS})

add_executable(shader-attributes-test shader-attributes/main.c)
target_link_libraries (shader-attributes-test ${TEST_LIBS})

add_executable(blit-batch-test blit-batch/main.c)
target_link_libraries (blit-batch-test ${TEST_LIBS})

add_executable(viewport-test viewport/main.c)
target_link_libraries (viewport-test ${TEST_LIBS})

add_executable(multitexture-test multitexture/main.c)
target_link_libraries (multitexture-test ${TEST_LIBS})

add_executable(intermediate-test intermediate/main.c)
target_link_libraries (intermediate-test ${TEST_LIBS})

add_executable(pixel-perfect-test pixel-perfect/main.c)
target_link_libraries (pixel-perfect-test ${TEST_LIBS})

add_executable(alias-test alias/main.c)
target_link_libraries (alias-test ${TEST_LIBS})

add_executable(triangle-batch-test triangle-batch/main.c)
target_link_libraries (triangle-batch-test ${TEST_LIBS})

add_executable(wrap-test wrap/main.c)
target_link_libraries (wrap-test ${TEST_LIBS})

add_executable(upload-image-test upload-image/main.c)
target_link_libraries (upload-image-test ${TEST_LIBS})

add_executable(image-formats-test image-formats/main.c)
target_link_libraries (image-formats-test ${TEST_LIBS})

add_executable(coordinate-mode-test coordinate-mode/main.c)
target_link_libraries (coordinate-mode-test ${TEST_LIBS})

add_executable(replace-image-test replace-image/main.c)
target_link_libraries (replace-image-test ${TEST_LIBS})

add_executable(sandbox-test sandbox/main.c)
target_link_libraries (sandbox-test ${TEST_LIBS})

add_executable(renderer-test renderer/main.c)
target_link_libraries (renderer-test ${TEST_LIBS})

add_executable(transform-matrix-test transform-matrix/main.c)
target_link_libraries (transform-matrix-test ${TEST_LIBS})

add_executable(anchor-test anchor/main.c)
target_link_libraries (anchor-test ${TEST_LIBS})

add_executable(memory-test memory/main.c)
target_link_libraries (memory-test ${TEST_LIBS})

add_executable(reinit-test reinit/main.c)
target_link_libraries (reinit-test ${TEST_LIBS})

add_executable(depth-test depth/main.c)
target_link_libraries (depth-test ${TEST_LIBS})

add_executable(camera-matrix-test camera-matrix/main.c)
target_link_libraries (camera-matrix-test ${TEST_LIBS})

add_executable(video-test video/main.c)
target_link_libraries (video-test ${TEST_LIBS})
