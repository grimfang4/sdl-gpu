#ifndef _SDL_GPU_OPENGL_H__
#define _SDL_GPU_OPENGL_H__

#include "SDL_gpu.h"


typedef struct RendererData_OpenGL
{
    #ifdef SDL_GPU_USE_SDL2
    SDL_Window* window;
    SDL_GLContext context;
    #endif
	GLuint handle;
	float z;
	Uint8 blending;
} RendererData_OpenGL;

typedef struct ImageData_OpenGL
{
	GLuint handle;
	GLenum format;
	GLboolean hasMipmaps;
	GLuint tex_w, tex_h;  // For power-of-two support
} ImageData_OpenGL;

typedef struct TargetData_OpenGL
{
	GLuint handle;
	GLenum format;
	GLuint textureHandle;
} TargetData_OpenGL;



typedef struct ShapeRendererData_OpenGL
{
	GLuint handle;
	float line_thickness;
} ShapeRendererData_OpenGL;

#endif
