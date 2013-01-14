#ifndef _SDL_GPU_OPENGLES_1_INTERNAL_H__
#define _SDL_GPU_OPENGLES_1_INTERNAL_H__

#ifndef SDL_GPU_FAKE_GLES

#include "GLES/gl.h"
#include "GLES/glext.h"

#define glFrustum glFrustumf
#define glOrtho glOrthof
#define glGenerateMipmap glGenerateMipmapOES
#define glDeleteFramebuffers glDeleteFramebuffersOES
#define glGenFramebuffers glGenFramebuffersOES
#define glFramebufferTexture2D glFramebufferTexture2DOES
#define glCheckFramebufferStatus glCheckFramebufferStatusOES
#define glBindFramebuffer glBindFramebufferOES
#define glBindFramebufferEXT glBindFramebufferOES
#define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_OES
#define GL_FRAMEBUFFER_COMPLETE GL_FRAMEBUFFER_COMPLETE_OES
#else

#include "GL/gl.h"
#include "GL/glext.h"

#endif

#include "SDL_gpu_OpenGLES_1.h"


GPU_Renderer* GPU_CreateRenderer_OpenGLES_1(void);
void GPU_FreeRenderer_OpenGLES_1(GPU_Renderer* renderer);

GPU_ShapeRenderer* GPU_CreateShapeRenderer_OpenGLES_1(void);
void GPU_FreeShapeRenderer_OpenGLES_1(GPU_ShapeRenderer* renderer);


#endif
