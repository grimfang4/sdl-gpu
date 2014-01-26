#ifndef _SDL_GPU_OPENGL_2_H__
#define _SDL_GPU_OPENGL_2_H__

#include "SDL_gpu.h"

#if !defined(SDL_GPU_DISABLE_OPENGL) && !defined(SDL_GPU_DISABLE_OPENGL_2)

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



#define GPU_DEFAULT_TEXTURED_VERTEX_SHADER_SOURCE \
"#version 120\n\
\
varying vec4 color;\n\
varying vec2 texCoord;\n\
\
void main(void)\n\
{\n\
	color = gl_Color;\n\
	texCoord = vec2(gl_MultiTexCoord0);\n\
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n\
}"

#define GPU_DEFAULT_UNTEXTURED_VERTEX_SHADER_SOURCE \
"#version 120\n\
\
varying vec4 color;\n\
\
void main(void)\n\
{\n\
	color = gl_Color;\n\
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n\
}"


#define GPU_DEFAULT_TEXTURED_FRAGMENT_SHADER_SOURCE \
"#version 120\n\
\
varying vec4 color;\n\
varying vec2 texCoord;\n\
\
uniform sampler2D tex;\n\
\
void main(void)\n\
{\n\
    gl_FragColor = texture2D(tex, texCoord) * color;\n\
}"

#define GPU_DEFAULT_UNTEXTURED_FRAGMENT_SHADER_SOURCE \
"#version 120\n\
\
varying vec4 color;\n\
\
void main(void)\n\
{\n\
    gl_FragColor = color;\n\
}"



typedef struct ContextData_OpenGL_2
{
	SDL_Color last_color;
	Uint8 last_use_blending;
	GPU_BlendEnum last_blend_mode;
	GPU_Rect last_viewport;
	GPU_Camera last_camera;
	
	GPU_Image* last_image;
	GPU_Target* last_target;
	float* blit_buffer;  // Holds sets of 4 vertices and 4 tex coords interleaved (e.g. [x0, y0, z0, s0, t0, ...]).
	int blit_buffer_num_vertices;
	int blit_buffer_max_num_vertices;
	unsigned short* index_buffer;  // Indexes into the blit buffer so we can use 4 vertices for every 2 triangles (1 quad)
	int index_buffer_num_vertices;
	int index_buffer_max_num_vertices;
} ContextData_OpenGL_2;

typedef struct RendererData_OpenGL_2
{
	Uint32 handle;
} RendererData_OpenGL_2;

typedef struct ImageData_OpenGL_2
{
	Uint32 handle;
	Uint32 format;
} ImageData_OpenGL_2;

typedef struct TargetData_OpenGL_2
{
	Uint32 handle;
	Uint32 format;
} TargetData_OpenGL_2;



#endif
