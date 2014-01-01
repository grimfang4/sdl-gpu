#ifndef _SDL_GPU_GLES_1_H__
#define _SDL_GPU_GLES_1_H__

#include "SDL_gpu.h"
#include "SDL_platform.h"

#if !defined(SDL_GPU_DISABLE_GLES) && !defined(SDL_GPU_DISABLE_GLES_1)

#ifdef __IPHONEOS__
    #include <OpenGLES/ES1/gl.h>
    #include <OpenGLES/ES1/glext.h>
#else
    #include "GLES/gl.h"
    #include "GLES/glext.h"
#endif

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


typedef struct RendererData_GLES_1
{
	Uint32 handle;
	float z;
	
	GPU_Image* last_image;
	GPU_Target* last_target;
	float* blit_buffer;  // Holds sets of 4 vertices and 4 tex coords interleaved (e.g. [x0, y0, z0, s0, t0, ...]).
	int blit_buffer_num_vertices;
	int blit_buffer_max_num_vertices;
} RendererData_GLES_1;

typedef struct ImageData_GLES_1
{
	Uint32 handle;
	Uint32 format;
	Uint8 hasMipmaps;
	Uint32 tex_w, tex_h;  // For power-of-two support
} ImageData_GLES_1;

typedef struct TargetData_GLES_1
{
	Uint32 handle;
	Uint32 format;
	
	Uint8 blending;
	float line_thickness;
	
    #ifdef SDL_GPU_USE_SDL2
    SDL_GLContext context;
    #endif
} TargetData_GLES_1;



#endif
