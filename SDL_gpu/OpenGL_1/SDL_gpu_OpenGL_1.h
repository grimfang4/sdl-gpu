#ifndef _SDL_GPU_OPENGL_1_H__
#define _SDL_GPU_OPENGL_1_H__

#include "SDL_gpu.h"

#if !defined(SDL_GPU_DISABLE_OPENGL) && !defined(SDL_GPU_DISABLE_OPENGL_1)

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


typedef struct RendererData_OpenGL_1
{
	Uint32 handle;
	float z;
	
	GPU_Image* last_image;
	GPU_Target* last_target;
	float* blit_buffer;  // Holds sets of 4 vertices and 4 tex coords interleaved (e.g. [x0, y0, z0, s0, t0, ...]).
	int blit_buffer_num_vertices;
	int blit_buffer_max_num_vertices;
} RendererData_OpenGL_1;

typedef struct ImageData_OpenGL_1
{
	Uint32 handle;
	Uint32 format;
	Uint8 hasMipmaps;
	Uint32 tex_w, tex_h;  // For power-of-two support
} ImageData_OpenGL_1;

typedef struct TargetData_OpenGL_1
{
	Uint32 handle;
	Uint32 format;
	
	float line_thickness;
	
    #ifdef SDL_GPU_USE_SDL2
    SDL_GLContext context;
    #endif
} TargetData_OpenGL_1;



#endif