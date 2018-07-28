#include "SDL_gpu_GLES_1.h"
#include "SDL_gpu_RendererImpl.h"

#if defined(SDL_GPU_DISABLE_GLES) || defined(SDL_GPU_DISABLE_GLES_1)

// Dummy implementations
GPU_Renderer* GPU_CreateRenderer_GLES_1(GPU_RendererID request) {return NULL;}
void GPU_FreeRenderer_GLES_1(GPU_Renderer* renderer) {}

#else


// Most of the code pulled in from here...
#define SDL_GPU_USE_GLES
#define SDL_GPU_GLES_MAJOR_VERSION 1

#define SDL_GPU_USE_ARRAY_PIPELINE
#define SDL_GPU_DISABLE_TEXTURE_GETS
#define SDL_GPU_DISABLE_SHADERS
#define SDL_GPU_DISABLE_RENDER_TO_TEXTURE
#define SDL_GPU_APPLY_TRANSFORMS_TO_GL_STACK
#define SDL_GPU_NO_VAO

#include "renderer_GL_common.inl"
#include "renderer_shapes_GL_common.inl"


GPU_Renderer* GPU_CreateRenderer_GLES_1(GPU_RendererID request)
{
    GPU_Renderer* renderer = (GPU_Renderer*)SDL_malloc(sizeof(GPU_Renderer));
    if(renderer == NULL)
        return NULL;

    memset(renderer, 0, sizeof(GPU_Renderer));

    renderer->id = request;
    renderer->id.renderer = GPU_RENDERER_GLES_1;
    renderer->shader_language = GPU_LANGUAGE_NONE;
    renderer->min_shader_version = 0;
    renderer->max_shader_version = 0;
    
    renderer->default_image_anchor_x = 0.5f;
    renderer->default_image_anchor_y = 0.5f;
    
    renderer->current_context_target = NULL;
    
    renderer->impl = (GPU_RendererImpl*)SDL_malloc(sizeof(GPU_RendererImpl));
    memset(renderer->impl, 0, sizeof(GPU_RendererImpl));
    SET_COMMON_FUNCTIONS(renderer->impl);

    return renderer;
}

void GPU_FreeRenderer_GLES_1(GPU_Renderer* renderer)
{
    if(renderer == NULL)
        return;

    SDL_free(renderer->impl);
    SDL_free(renderer);
}

#endif
