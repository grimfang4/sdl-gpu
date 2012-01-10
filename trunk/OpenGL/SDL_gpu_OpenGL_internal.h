#ifndef _SDL_GPU_OPENGL_INTERNAL_H__
#define _SDL_GPU_OPENGL_INTERNAL_H__

#include "SDL_gpu_OpenGL.h"
#include "GL/glext.h"


GPU_Renderer* GPU_CreateRenderer_OpenGL(void);
void GPU_FreeRenderer_OpenGL(GPU_Renderer* renderer);

// GL extensions
extern PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;


#endif
