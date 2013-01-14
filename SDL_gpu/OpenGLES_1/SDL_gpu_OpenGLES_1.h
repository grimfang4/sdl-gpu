#ifndef _SDL_GPU_OPENGLES_1_H__
#define _SDL_GPU_OPENGLES_1_H__

#include "SDL_gpu.h"


typedef struct RendererData_OpenGLES_1
{
    #ifdef SDL_GPU_USE_SDL2
    SDL_Window* window;
    SDL_GLContext context;
    #endif
	GLuint handle;
	float z;
} RendererData_OpenGLES_1;

typedef struct ImageData_OpenGLES_1
{
	GLuint handle;
	GLenum format;
	GLboolean hasMipmaps;
	GLuint tex_w, tex_h;  // For power-of-two support
} ImageData_OpenGLES_1;

typedef struct TargetData_OpenGLES_1
{
	GLuint handle;
	GLenum format;
} TargetData_OpenGLES_1;



typedef struct ShapeRendererData_OpenGLES_1
{
	GLuint handle;
} ShapeRendererData_OpenGLES_1;

#endif
