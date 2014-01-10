#include "SDL_gpu_OpenGL_1.h"


#if defined(SDL_GPU_DISABLE_OPENGL) || defined(SDL_GPU_DISABLE_OPENGL_1)

// Dummy implementations
GPU_Renderer* GPU_CreateRenderer_OpenGL_1(GPU_RendererID request) {return NULL;}
void GPU_FreeRenderer_OpenGL_1(GPU_Renderer* renderer) {}

#else

// Most of the code pulled in from here...
#define SDL_GPU_USE_OPENGL
//#define SDL_GPU_DISABLE_SHADERS
#define SDL_GPU_USE_GL_TIER1
#define SDL_GPU_GL_TIER 1
#define SDL_GPU_GL_MAJOR_VERSION 1
#define CONTEXT_DATA ContextData_OpenGL_1
#define RENDERER_DATA RendererData_OpenGL_1
#define IMAGE_DATA ImageData_OpenGL_1
#define TARGET_DATA TargetData_OpenGL_1
#include "../GL_common/SDL_gpu_GL_common.inl"
#include "../GL_common/SDL_gpuShapes_GL_common.inl"


GPU_Renderer* GPU_CreateRenderer_OpenGL_1(GPU_RendererID request)
{
    GPU_Renderer* renderer = (GPU_Renderer*)malloc(sizeof(GPU_Renderer));
    if(renderer == NULL)
        return NULL;

    memset(renderer, 0, sizeof(GPU_Renderer));

    renderer->id = request;
	renderer->id.id = GPU_RENDERER_OPENGL_1;
	renderer->tier = SDL_GPU_GL_TIER;
    
    renderer->current_context_target = NULL;

    renderer->data = (RENDERER_DATA*)malloc(sizeof(RENDERER_DATA));
    memset(renderer->data, 0, sizeof(RENDERER_DATA));

    SET_COMMON_FUNCTIONS(renderer);

    return renderer;
}

void GPU_FreeRenderer_OpenGL_1(GPU_Renderer* renderer)
{
    if(renderer == NULL)
        return;

    free(renderer->data);
    free(renderer);
}

#endif
