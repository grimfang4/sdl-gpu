#ifndef _SDL_GPU_OPENGL_3_H__
#define _SDL_GPU_OPENGL_3_H__

#include "SDL_gpu.h"

#if !defined(SDL_GPU_DISABLE_OPENGL) && !defined(SDL_GPU_DISABLE_OPENGL_3)

    // Hacks to fix compile errors due to polluted namespace
    #ifdef _WIN32
    #define _WINUSER_H
    #define _WINGDI_H
    #endif
    
    #include "glew.h"
	
	#if defined(GL_EXT_bgr) && !defined(GL_BGR)
		#define GL_BGR GL_BGR_EXT
	#endif
	#if defined(GL_EXT_bgra) && !defined(GL_BGRA)
		#define GL_BGRA GL_BGRA_EXT
	#endif
	#if defined(GL_EXT_abgr) && !defined(GL_ABGR)
		#define GL_ABGR GL_ABGR_EXT
	#endif
#endif


typedef struct ContextData_OpenGL_3
{
	Uint32 default_textured_shader_program;
	Uint32 default_untextured_shader_program;
	
	SDL_Color last_color;
	Uint8 last_use_blending;
	GPU_BlendEnum last_blend_mode;
	GPU_Camera last_camera;
} ContextData_OpenGL_3;

typedef struct RendererData_OpenGL_3
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
} RendererData_OpenGL_3;

typedef struct ImageData_OpenGL_3
{
	Uint32 handle;
	Uint32 format;
	Uint8 hasMipmaps;
	Uint32 tex_w, tex_h;  // For power-of-two support
} ImageData_OpenGL_3;

typedef struct TargetData_OpenGL_3
{
	Uint32 handle;
	Uint32 format;
	
	float line_thickness;
	
    #ifdef SDL_GPU_USE_SDL2
    SDL_GLContext context;
    #endif
    
    // Tier 3 rendering
    unsigned int blit_VAO;
    unsigned int blit_VBO[2];  // For double-buffering
    Uint8 blit_VBO_flop;
    GPU_ShaderBlock shader_block[2];
    GPU_ShaderBlock current_shader_block;
    
} TargetData_OpenGL_3;



#endif
