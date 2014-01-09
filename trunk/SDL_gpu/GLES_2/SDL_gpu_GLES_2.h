#ifndef _SDL_GPU_GLES_2_H__
#define _SDL_GPU_GLES_2_H__

#include "SDL_gpu.h"
#include "SDL_platform.h"

#if !defined(SDL_GPU_DISABLE_GLES) && !defined(SDL_GPU_DISABLE_GLES_2)

#ifdef __IPHONEOS__
    #include <OpenGLES/ES2/gl2.h>
    #include <OpenGLES/ES2/gl2ext.h>
#else
    #include "GLES2/gl2.h"
    #include "GLES2/gl2ext.h"
#endif

    #define glFrustum glFrustumf
    #define glOrtho glOrthof
    #define glGenerateMipmap glGenerateMipmapOES

    #define glBlendEquation glBlendEquationOES
    #define glBlendFuncSeparate glBlendFuncSeparateOES
    
    #define glGetUniformuiv glGetUniformiv
    #define glUniform1ui glUniform1i
    #define glUniform1uiv glUniform1iv
    #define glUniform2uiv glUniform2iv
    #define glUniform3uiv glUniform3iv
    #define glUniform4uiv glUniform4iv

#endif


typedef struct RendererData_GLES_2
{
	Uint32 handle;
	float z;
	
	GPU_Image* last_image;
	GPU_Target* last_target;
	float* blit_buffer;  // Holds sets of 4 vertices, each with interleaved position, tex coords, and colors (e.g. [x0, y0, z0, s0, t0, r0, g0, b0, a0, ...]).
	int blit_buffer_num_vertices;
	int blit_buffer_max_num_vertices;
	unsigned short* index_buffer;  // Indexes into the blit buffer so we can use 4 vertices for every 2 triangles (1 quad)
	int index_buffer_num_vertices;
	int index_buffer_max_num_vertices;
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
	
	float line_thickness;
	
    #ifdef SDL_GPU_USE_SDL2
    SDL_GLContext context;
    #endif
    
    // Tier 3 rendering
    unsigned int blit_VBO[2];  // For double-buffering
    Uint8 blit_VBO_flop;
    GPU_ShaderBlock shader_block[2];
    GPU_ShaderBlock current_shader_block;
    
} TargetData_GLES_2;



#endif
