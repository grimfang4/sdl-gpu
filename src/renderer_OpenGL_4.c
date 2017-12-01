#include "SDL_gpu_OpenGL_4.h"
#include "SDL_gpu_RendererImpl.h"


#if defined(SDL_GPU_DISABLE_OPENGL) || defined(SDL_GPU_DISABLE_OPENGL_4)

// Dummy implementations
GPU_Renderer* GPU_CreateRenderer_OpenGL_4(GPU_RendererID request) {return NULL;}
void GPU_FreeRenderer_OpenGL_4(GPU_Renderer* renderer) {}

#else


// Most of the code pulled in from here...
#define SDL_GPU_USE_OPENGL
#define SDL_GPU_USE_BUFFER_PIPELINE
#define SDL_GPU_ASSUME_CORE_FBO
#define SDL_GPU_ASSUME_SHADERS
#define SDL_GPU_SKIP_ENABLE_TEXTURE_2D
#define SDL_GPU_SKIP_LINE_WIDTH
#define SDL_GPU_GLSL_VERSION 150
#define SDL_GPU_GL_MAJOR_VERSION 4

#include "renderer_GL_common.inl"
#include "renderer_shapes_GL_common.inl"


GPU_Renderer* GPU_CreateRenderer_OpenGL_4(GPU_RendererID request)
{
    GPU_Renderer* renderer = (GPU_Renderer*)SDL_malloc(sizeof(GPU_Renderer));
    if(renderer == NULL)
        return NULL;

    memset(renderer, 0, sizeof(GPU_Renderer));

    renderer->id = request;
    renderer->id.renderer = GPU_RENDERER_OPENGL_4;
    renderer->shader_language = GPU_LANGUAGE_GLSL;
    renderer->min_shader_version = 110;
    renderer->max_shader_version = SDL_GPU_GLSL_VERSION;
    
    renderer->default_image_anchor_x = 0.5f;
    renderer->default_image_anchor_y = 0.5f;
    
    renderer->current_context_target = NULL;
    
    renderer->impl = (GPU_RendererImpl*)SDL_malloc(sizeof(GPU_RendererImpl));
    memset(renderer->impl, 0, sizeof(GPU_RendererImpl));
    SET_COMMON_FUNCTIONS(renderer->impl);

    return renderer;
}

void GPU_FreeRenderer_OpenGL_4(GPU_Renderer* renderer)
{
    if(renderer == NULL)
        return;

    SDL_free(renderer->impl);
    SDL_free(renderer);
}


#endif
