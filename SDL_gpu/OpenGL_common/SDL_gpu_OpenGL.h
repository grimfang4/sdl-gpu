#ifndef _SDL_GPU_OPENGL_H__
#define _SDL_GPU_OPENGL_H__

#include "SDL_gpu.h"

#ifdef SDL_GPU_USE_OPENGL
    #include "glew.h"
#elif defined(SDL_GPU_USE_OPENGLES)
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

    #define glBlendEquation glBlendEquationOES
    #define glBlendFuncSeparate glBlendFuncSeparateOES

    #define GL_FUNC_ADD GL_FUNC_ADD_OES
    #define GL_FUNC_SUBTRACT GL_FUNC_SUBTRACT_OES
    #define GL_FUNC_REVERSE_SUBTRACT GL_FUNC_REVERSE_SUBTRACT_OES
#endif

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
} TargetData_OpenGL;



typedef struct ShapeRendererData_OpenGL
{
	GLuint handle;
	float line_thickness;
} ShapeRendererData_OpenGL;

#endif
