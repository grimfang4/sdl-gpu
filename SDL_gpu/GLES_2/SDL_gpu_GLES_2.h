#ifndef _SDL_GPU_GLES_2_H__
#define _SDL_GPU_GLES_2_H__

#include "SDL_gpu.h"

#if !defined(SDL_GPU_DISABLE_GLES) && !defined(SDL_GPU_DISABLE_GLES_2)
    #include "GLES2/gl2.h"
    #include "GLES2/gl2ext.h"

    #define glFrustum glFrustumf
    #define glOrtho glOrthof
    #define glGenerateMipmap glGenerateMipmapOES
    #define glDeleteFramebuffers glDeleteFramebuffersOES
    #define glGenFramebuffers glGenFramebuffersOES
    #define glFramebufferTexture2D glFramebufferTexture2DOES
    #define glCheckFramebufferStatus glCheckFramebufferStatusOES
    #define glBindFramebuffer glBindFramebufferOES
    #define glBindFramebufferEXT glBindFramebufferOES

    #define glBlendEquation glBlendEquationOES
    #define glBlendFuncSeparate glBlendFuncSeparateOES

#endif


typedef struct RendererData_GLES_2
{
	Uint32 handle;
	float z;
	
	GPU_Image* last_image;
	GPU_Target* last_target;
	float* blit_buffer;  // Holds sets of 4 vertices and 4 tex coords interleaved (e.g. [x0, y0, z0, s0, t0, ...]).
	int blit_buffer_size;
	int blit_buffer_max_size;
} RendererData_GLES_2;

typedef struct ImageData_GLES_2
{
	Uint32 handle;
	Uint32 format;
	Uint8 hasMipmaps;
	Uint32 tex_w, tex_h;  // For power-of-two support
} ImageData_GLES_2;

typedef struct TargetData_GLES_2
{
	Uint32 handle;
	Uint32 format;
	
	Uint8 blending;
	float line_thickness;
	
    #ifdef SDL_GPU_USE_SDL2
    SDL_GLContext context;
    #endif
} TargetData_GLES_2;



#endif
