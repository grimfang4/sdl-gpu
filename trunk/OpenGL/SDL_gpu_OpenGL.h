#ifndef _SDL_GPU_OPENGL_H__
#define _SDL_GPU_OPENGL_H__

#include "SDL_gpu.h"
#include "SDL_opengl.h"

// FIXME: This should move into a file which users can use...
typedef struct RendererData_OpenGL
{
	GLuint handle;
	// What else?
} RendererData_OpenGL;

typedef struct ImageData_OpenGL
{
	GLuint handle;
	GLenum format;
} ImageData_OpenGL;

typedef struct TargetData_OpenGL
{
	GLuint handle;
} TargetData_OpenGL;


GPU_Renderer* GPU_CreateRenderer_OpenGL(void);
void GPU_FreeRenderer_OpenGL(GPU_Renderer* renderer);




#endif
