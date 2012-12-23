#ifndef _SDL_GPU_OPENGL_H__
#define _SDL_GPU_OPENGL_H__

#include "SDL_gpu.h"


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



typedef struct ShapeRendererData_OpenGL
{
	GLuint handle;
} ShapeRendererData_OpenGL;

#endif
