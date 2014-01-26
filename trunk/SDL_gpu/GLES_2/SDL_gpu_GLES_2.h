#ifndef _SDL_GPU_GLES_2_H__
#define _SDL_GPU_GLES_2_H__

#include "SDL_gpu.h"
#include "SDL_platform.h"

#if !defined(SDL_GPU_DISABLE_GLES) && !defined(SDL_GPU_DISABLE_GLES_2)

#ifdef __IPHONEOS__
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
#else
    #include "GLES2/gl2.h"
    #include "GLES2/gl2ext.h"
#endif

#endif


typedef struct ContextData_GLES_2
{
	SDL_Color last_color;
	Uint8 last_use_blending;
	GPU_BlendEnum last_blend_mode;
	GPU_Rect last_viewport;
	GPU_Camera last_camera;
	
	GPU_Image* last_image;
	GPU_Target* last_target;
	float* blit_buffer;  // Holds sets of 4 vertices, each with interleaved position, tex coords, and colors (e.g. [x0, y0, z0, s0, t0, r0, g0, b0, a0, ...]).
	int blit_buffer_num_vertices;
	int blit_buffer_max_num_vertices;
	unsigned short* index_buffer;  // Indexes into the blit buffer so we can use 4 vertices for every 2 triangles (1 quad)
	int index_buffer_num_vertices;
	int index_buffer_max_num_vertices;
    
    // Tier 3 rendering
    unsigned int blit_VBO[2];  // For double-buffering
    Uint8 blit_VBO_flop;
    GPU_ShaderBlock shader_block[2];
    GPU_ShaderBlock current_shader_block;
    
	GPU_AttributeSource shader_attributes[16];
	unsigned int attribute_VBO[16];
} ContextData_GLES_2;

typedef struct RendererData_GLES_2
{
	Uint32 handle;
} RendererData_GLES_2;

typedef struct ImageData_GLES_2
{
	Uint32 handle;
	Uint32 format;
} ImageData_GLES_2;

typedef struct TargetData_GLES_2
{
	Uint32 handle;
	Uint32 format;
} TargetData_GLES_2;



#endif
