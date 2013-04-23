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

#if defined(GL_EXT_bgr) && !defined(GL_BGR)
    #define GL_BGR GL_BGR_EXT
#endif
#if defined(GL_EXT_bgra) && !defined(GL_BGRA)
    #define GL_BGRA GL_BGRA_EXT
#endif
#if defined(GL_EXT_abgr) && !defined(GL_ABGR)
    #define GL_ABGR GL_ABGR_EXT
#endif

// Forces a flush when limit is reached (roughly 1000 sprites)
#define GPU_BLIT_BUFFER_INIT_MAX_SIZE 6000
// x, y, z, s, t
#define GPU_BLIT_BUFFER_FLOATS_PER_VERTEX 5
// bytes per vertex
#define GPU_BLIT_BUFFER_STRIDE (sizeof(float)*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX)
#define GPU_BLIT_BUFFER_VERTEX_OFFSET 0
#define GPU_BLIT_BUFFER_TEX_COORD_OFFSET 3

typedef struct RendererData_OpenGL
{
    #ifdef SDL_GPU_USE_SDL2
    SDL_Window* window;
    SDL_GLContext context;
    #endif
	GLuint handle;
	float z;
	Uint8 blending;
	
	GPU_Image* last_image;
	GPU_Target* last_target;
	float* blit_buffer;  // Holds sets of 4 vertices and 4 tex coords interleaved (e.g. [x0, y0, z0, s0, t0, ...]).
	int blit_buffer_size;
	int blit_buffer_max_size;
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
