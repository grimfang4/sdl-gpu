#ifndef _SDL_GPU_OPENGL_H__
#define _SDL_GPU_OPENGL_H__

#include "SDL_gpu.h"
#include "SDL_opengl.h"


typedef struct RendererData_OpenGL
{
	GLuint handle;
	float z;
} RendererData_OpenGL;

typedef struct ImageData_OpenGL
{
	GLuint handle;
	GLenum format;
	GLboolean hasMipmaps;
} ImageData_OpenGL;

typedef struct TargetData_OpenGL
{
	GLuint handle;
	GLenum format;
} TargetData_OpenGL;


#endif
