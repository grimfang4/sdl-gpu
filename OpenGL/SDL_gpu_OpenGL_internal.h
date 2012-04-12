#ifndef _SDL_GPU_OPENGL_INTERNAL_H__
#define _SDL_GPU_OPENGL_INTERNAL_H__

#include "glew.h"
#include "SDL_gpu_OpenGL.h"


GPU_Renderer* GPU_CreateRenderer_OpenGL(void);
void GPU_FreeRenderer_OpenGL(GPU_Renderer* renderer);


#endif
