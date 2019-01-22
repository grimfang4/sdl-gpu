#ifndef _SDL_GPU_OPENGL_1_BASE_H__
#define _SDL_GPU_OPENGL_1_BASE_H__

#include "SDL_gpu.h"

#if !defined(SDL_GPU_DISABLE_OPENGL) && !defined(SDL_GPU_DISABLE_OPENGL_1_BASE)

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
	
	#undef GL_MIRRORED_REPEAT
    #define GL_MIRRORED_REPEAT GL_MIRRORED_REPEAT_ARB
#endif



#define GPU_CONTEXT_DATA ContextData_OpenGL_1_BASE
#define GPU_IMAGE_DATA ImageData_OpenGL_1_BASE
#define GPU_TARGET_DATA TargetData_OpenGL_1_BASE




typedef struct ContextData_OpenGL_1_BASE
{
	SDL_Color last_color;
	GPU_bool last_use_texturing;
	unsigned int last_shape;
	GPU_bool last_use_blending;
	GPU_BlendMode last_blend_mode;
	GPU_Rect last_viewport;
	GPU_Camera last_camera;
	GPU_bool last_camera_inverted;
	
	GPU_bool last_depth_test;
	GPU_bool last_depth_write;
	GPU_ComparisonEnum last_depth_function;
	
	GPU_Image* last_image;
	float* blit_buffer;  // Holds sets of 4 vertices and 4 tex coords interleaved (e.g. [x0, y0, z0, s0, t0, ...]).
	unsigned short blit_buffer_num_vertices;
	unsigned short blit_buffer_max_num_vertices;
	unsigned short* index_buffer;  // Indexes into the blit buffer so we can use 4 vertices for every 2 triangles (1 quad)
	unsigned int index_buffer_num_vertices;
	unsigned int index_buffer_max_num_vertices;
} ContextData_OpenGL_1_BASE;

typedef struct ImageData_OpenGL_1_BASE
{
    int refcount;
    GPU_bool owns_handle;
	Uint32 handle;
	Uint32 format;
} ImageData_OpenGL_1_BASE;

typedef struct TargetData_OpenGL_1_BASE
{
    int refcount;
	Uint32 handle;
	Uint32 format;
} TargetData_OpenGL_1_BASE;



#endif
