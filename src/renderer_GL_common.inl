/* This is an implementation file to be included after certain #defines have been set.
See a particular renderer's *.c file for specifics. */


#include <stdlib.h>
#include "SDL_platform.h"
#include <math.h>
#include <string.h>

// Check for C99 support
// We'll use it for intptr_t which is used to suppress warnings about converting an int to a ptr for GL calls.
#if __STDC_VERSION__ >= 199901L
    #include <stdint.h>
#else
    #define intptr_t long
#endif

#include "stb_image.h"
#include "stb_image_write.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Visual C does not support static inline
#ifndef static_inline
    #ifdef _MSC_VER
		#define static_inline static
    #else
        #define static_inline static inline
    #endif
#endif

#if defined ( WIN32 )
#define __func__ __FUNCTION__
#endif

// Old Visual C did not support C99 (which includes a safe snprintf)
#if defined(_MSC_VER) && (_MSC_VER < 1900)
	#define snprintf c99_snprintf
	// From Valentin Milea: http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
	static_inline int c99_vsnprintf(char* str, size_t size, const char* format, va_list ap)
	{
		int count = -1;

		if (size != 0)
			count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
		if (count == -1)
			count = _vscprintf(format, ap);

		return count;
	}

	static_inline int c99_snprintf(char* str, size_t size, const char* format, ...)
	{
		int count;
		va_list ap;

		va_start(ap, format);
		count = c99_vsnprintf(str, size, format, ap);
		va_end(ap);

		return count;
	}
#endif

int gpu_strcasecmp(const char* s1, const char* s2);


// Forces a flush when vertex limit is reached (roughly 1000 sprites)
#define GPU_BLIT_BUFFER_VERTICES_PER_SPRITE 4
#define GPU_BLIT_BUFFER_INIT_MAX_NUM_VERTICES (GPU_BLIT_BUFFER_VERTICES_PER_SPRITE*1000)


// Near the unsigned short limit (65535)
#define GPU_BLIT_BUFFER_ABSOLUTE_MAX_VERTICES 60000
// Near the unsigned int limit (4294967295)
#define GPU_INDEX_BUFFER_ABSOLUTE_MAX_VERTICES 4000000000u


// x, y, s, t, r, g, b, a
#define GPU_BLIT_BUFFER_FLOATS_PER_VERTEX 8

// bytes per vertex
#define GPU_BLIT_BUFFER_STRIDE (sizeof(float)*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX)
#define GPU_BLIT_BUFFER_VERTEX_OFFSET 0
#define GPU_BLIT_BUFFER_TEX_COORD_OFFSET 2
#define GPU_BLIT_BUFFER_COLOR_OFFSET 4



// SDL 1.2 / SDL 2.0 translation layer


#ifdef SDL_GPU_USE_SDL2

#define GET_ALPHA(sdl_color) ((sdl_color).a)

static_inline SDL_Window* get_window(Uint32 windowID)
{
    return SDL_GetWindowFromID(windowID);
}

static_inline Uint32 get_window_id(SDL_Window* window)
{
    return SDL_GetWindowID(window);
}

static_inline void get_window_dimensions(SDL_Window* window, int* w, int* h)
{
    SDL_GetWindowSize(window, w, h);
}

static_inline void get_drawable_dimensions(SDL_Window* window, int* w, int* h)
{
    SDL_GL_GetDrawableSize(window, w, h);
}

static_inline void resize_window(GPU_Target* target, int w, int h)
{
    SDL_SetWindowSize(SDL_GetWindowFromID(target->context->windowID), w, h);
}

static_inline Uint8 get_fullscreen_state(SDL_Window* window)
{
    return (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN);
}

static_inline Uint8 has_colorkey(SDL_Surface* surface)
{
    return (SDL_GetColorKey(surface, NULL) == 0);
}

#else

#define SDL_Window SDL_Surface
#define GET_ALPHA(sdl_color) ((sdl_color).unused)

static_inline SDL_Window* get_window(Uint32 windowID)
{
    return (windowID == 1? SDL_GetVideoSurface() : NULL);
}

static_inline Uint32 get_window_id(SDL_Surface* window)
{
    return (SDL_GetVideoSurface() == window? 1 : 0);
}

static_inline void get_window_dimensions(SDL_Window* window, int* w, int* h)
{
    if(window == NULL)
        return;
    *w = window->w;
    *h = window->h;
}

static_inline void get_drawable_dimensions(SDL_Window* window, int* w, int* h)
{
    get_window_dimensions(window, w, h);
}

static_inline void resize_window(GPU_Target* target, int w, int h)
{
    SDL_Surface* screen = SDL_GetVideoSurface();
    Uint32 flags = screen->flags;
    
    screen = SDL_SetVideoMode(w, h, 0, flags);
    // NOTE: There's a bug in SDL 1.2.  This is a workaround.  Let's resize again:
    screen = SDL_SetVideoMode(w, h, 0, flags);
}

static_inline Uint8 get_fullscreen_state(SDL_Window* window)
{
    return (window->flags & SDL_FULLSCREEN);
}

static_inline Uint8 has_colorkey(SDL_Surface* surface)
{
    return (surface->flags & SDL_SRCCOLORKEY);
}

#endif



static_inline void get_target_window_dimensions(GPU_Target* target, int* w, int* h)
{
    SDL_Window* window;
    if(target == NULL || target->context == NULL)
        return;
    window = get_window(target->context->windowID);
    get_window_dimensions(window, w, h);
}

static_inline void get_target_drawable_dimensions(GPU_Target* target, int* w, int* h)
{
    SDL_Window* window;
    if(target == NULL || target->context == NULL)
        return;
    window = get_window(target->context->windowID);
    get_drawable_dimensions(window, w, h);
}




#ifndef GL_VERTEX_SHADER
    #ifndef SDL_GPU_DISABLE_SHADERS
        #define SDL_GPU_DISABLE_SHADERS
    #endif
#endif


// Workaround for Intel HD glVertexAttrib() bug.
#ifdef SDL_GPU_USE_OPENGL
// FIXME: This should probably exist in context storage, as I expect it to be a problem across contexts.
static Uint8 apply_Intel_attrib_workaround = 0;
static Uint8 vendor_is_Intel = 0;
#endif



static SDL_PixelFormat* AllocFormat(GLenum glFormat);
static void FreeFormat(SDL_PixelFormat* format);


static char shader_message[256];



static Uint8 isExtensionSupported(const char* extension_str)
{
#ifdef SDL_GPU_USE_OPENGL
    return glewIsExtensionSupported(extension_str);
#else
    // As suggested by Mesa3D.org
    char* p = (char*)glGetString(GL_EXTENSIONS);
    char* end;
    unsigned long extNameLen;

    extNameLen = strlen(extension_str);
    end = p + strlen(p);

    while(p < end)
    {
        unsigned long n = strcspn(p, " ");
        if((extNameLen == n) && (strncmp(extension_str, p, n) == 0))
            return 1;

        p += (n + 1);
    }
    return 0;
#endif
}

static void init_features(GPU_Renderer* renderer)
{
    // Reset supported features
    renderer->enabled_features = 0;

    // NPOT textures
#ifdef SDL_GPU_USE_OPENGL
    if(isExtensionSupported("GL_ARB_texture_non_power_of_two"))
        renderer->enabled_features |= GPU_FEATURE_NON_POWER_OF_TWO;
    else
        renderer->enabled_features &= ~GPU_FEATURE_NON_POWER_OF_TWO;
#elif defined(SDL_GPU_USE_GLES)
    if(isExtensionSupported("GL_OES_texture_npot") || isExtensionSupported("GL_IMG_texture_npot")
       || isExtensionSupported("GL_APPLE_texture_2D_limited_npot") || isExtensionSupported("GL_ARB_texture_non_power_of_two"))
        renderer->enabled_features |= GPU_FEATURE_NON_POWER_OF_TWO;
    else
        renderer->enabled_features &= ~GPU_FEATURE_NON_POWER_OF_TWO;
#endif

    // FBO
#ifdef SDL_GPU_USE_OPENGL
    if(isExtensionSupported("GL_EXT_framebuffer_object"))
        renderer->enabled_features |= GPU_FEATURE_RENDER_TARGETS;
    else
        renderer->enabled_features &= ~GPU_FEATURE_RENDER_TARGETS;
#elif defined(SDL_GPU_USE_GLES)
    #if SDL_GPU_GL_TIER < 3
        if(isExtensionSupported("GL_OES_framebuffer_object"))
            renderer->enabled_features |= GPU_FEATURE_RENDER_TARGETS;
        else
            renderer->enabled_features &= ~GPU_FEATURE_RENDER_TARGETS;
    #else
            renderer->enabled_features |= GPU_FEATURE_RENDER_TARGETS;
    #endif
#endif

    // Blending
#ifdef SDL_GPU_USE_OPENGL
    renderer->enabled_features |= GPU_FEATURE_BLEND_EQUATIONS;
    renderer->enabled_features |= GPU_FEATURE_BLEND_FUNC_SEPARATE;

    #if SDL_GPU_GL_MAJOR_VERSION > 1
        // Core in GL 2+
        renderer->enabled_features |= GPU_FEATURE_BLEND_EQUATIONS_SEPARATE;
    #else
        if(isExtensionSupported("GL_EXT_blend_equation_separate"))
            renderer->enabled_features |= GPU_FEATURE_BLEND_EQUATIONS_SEPARATE;
        else
            renderer->enabled_features &= ~GPU_FEATURE_BLEND_EQUATIONS_SEPARATE;
    #endif

#elif defined(SDL_GPU_USE_GLES)

    #if SDL_GPU_GLES_MAJOR_VERSION > 1
        // Core in GLES 2+
        renderer->enabled_features |= GPU_FEATURE_BLEND_EQUATIONS;
        renderer->enabled_features |= GPU_FEATURE_BLEND_FUNC_SEPARATE;
        renderer->enabled_features |= GPU_FEATURE_BLEND_EQUATIONS_SEPARATE;
    #else
        if(isExtensionSupported("GL_OES_blend_subtract"))
            renderer->enabled_features |= GPU_FEATURE_BLEND_EQUATIONS;
        else
            renderer->enabled_features &= ~GPU_FEATURE_BLEND_EQUATIONS;

        if(isExtensionSupported("GL_OES_blend_func_separate"))
            renderer->enabled_features |= GPU_FEATURE_BLEND_FUNC_SEPARATE;
        else
            renderer->enabled_features &= ~GPU_FEATURE_BLEND_FUNC_SEPARATE;

        if(isExtensionSupported("GL_OES_blend_equation_separate"))
            renderer->enabled_features |= GPU_FEATURE_BLEND_EQUATIONS_SEPARATE;
        else
            renderer->enabled_features &= ~GPU_FEATURE_BLEND_EQUATIONS_SEPARATE;
    #endif
#endif

    // Wrap modes
#ifdef SDL_GPU_USE_OPENGL
    #if SDL_GPU_GL_MAJOR_VERSION > 1
        renderer->enabled_features |= GPU_FEATURE_WRAP_REPEAT_MIRRORED;
    #else
        if(isExtensionSupported("GL_ARB_texture_mirrored_repeat"))
            renderer->enabled_features |= GPU_FEATURE_WRAP_REPEAT_MIRRORED;
        else
            renderer->enabled_features &= ~GPU_FEATURE_WRAP_REPEAT_MIRRORED;
    #endif
#elif defined(SDL_GPU_USE_GLES)
    #if SDL_GPU_GLES_MAJOR_VERSION > 1
        renderer->enabled_features |= GPU_FEATURE_WRAP_REPEAT_MIRRORED;
    #else
        if(isExtensionSupported("GL_OES_texture_mirrored_repeat"))
            renderer->enabled_features |= GPU_FEATURE_WRAP_REPEAT_MIRRORED;
        else
            renderer->enabled_features &= ~GPU_FEATURE_WRAP_REPEAT_MIRRORED;
    #endif
#endif

    // GL texture formats
    if(isExtensionSupported("GL_EXT_bgr"))
        renderer->enabled_features |= GPU_FEATURE_GL_BGR;
    if(isExtensionSupported("GL_EXT_bgra"))
        renderer->enabled_features |= GPU_FEATURE_GL_BGRA;
    if(isExtensionSupported("GL_EXT_abgr"))
        renderer->enabled_features |= GPU_FEATURE_GL_ABGR;

	// Disable other texture formats for GLES.
	// TODO: Add better (static) checking for format support.  Some GL versions do not report previously non-core features as extensions.
	#ifdef SDL_GPU_USE_GLES
		renderer->enabled_features &= ~GPU_FEATURE_GL_BGR;
		renderer->enabled_features &= ~GPU_FEATURE_GL_BGRA;
		renderer->enabled_features &= ~GPU_FEATURE_GL_ABGR;
	#endif

    // Shader support
    #ifndef SDL_GPU_DISABLE_SHADERS
    if(isExtensionSupported("GL_ARB_fragment_shader"))
        renderer->enabled_features |= GPU_FEATURE_FRAGMENT_SHADER;
    if(isExtensionSupported("GL_ARB_vertex_shader"))
        renderer->enabled_features |= GPU_FEATURE_VERTEX_SHADER;
    if(isExtensionSupported("GL_ARB_geometry_shader4"))
        renderer->enabled_features |= GPU_FEATURE_GEOMETRY_SHADER;
    #endif
    #ifdef SDL_GPU_ASSUME_SHADERS
    renderer->enabled_features |= GPU_FEATURE_BASIC_SHADERS;
    #endif
}

static void extBindFramebuffer(GPU_Renderer* renderer, GLuint handle)
{
    if(renderer->enabled_features & GPU_FEATURE_RENDER_TARGETS)
        glBindFramebuffer(GL_FRAMEBUFFER, handle);
}


static_inline Uint8 isPowerOfTwo(unsigned int x)
{
    return ((x != 0) && !(x & (x - 1)));
}

static_inline unsigned int getNearestPowerOf2(unsigned int n)
{
    unsigned int x = 1;
    while(x < n)
    {
        x <<= 1;
    }
    return x;
}

static void bindTexture(GPU_Renderer* renderer, GPU_Image* image)
{
    // Bind the texture to which subsequent calls refer
    if(image != ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image)
    {
        GLuint handle = ((GPU_IMAGE_DATA*)image->data)->handle;
        renderer->impl->FlushBlitBuffer(renderer);

        glBindTexture( GL_TEXTURE_2D, handle );
        ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image = image;
    }
}

static_inline void flushAndBindTexture(GPU_Renderer* renderer, GLuint handle)
{
    // Bind the texture to which subsequent calls refer
    renderer->impl->FlushBlitBuffer(renderer);

    glBindTexture( GL_TEXTURE_2D, handle );
    ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image = NULL;
}

// Returns false if it can't be bound
static Uint8 bindFramebuffer(GPU_Renderer* renderer, GPU_Target* target)
{
    if(renderer->enabled_features & GPU_FEATURE_RENDER_TARGETS)
    {
        // Bind the FBO
        if(target != ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target)
        {
            GLuint handle = 0;
            if(target != NULL)
                handle = ((GPU_TARGET_DATA*)target->data)->handle;
            renderer->impl->FlushBlitBuffer(renderer);

            extBindFramebuffer(renderer, handle);
            ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target = target;
        }
        return 1;
    }
    else
    {
        // There's only one possible render target, the default framebuffer.
        // Note: Could check against the default framebuffer value (((GPU_TARGET_DATA*)target->data)->handle versus result of GL_FRAMEBUFFER_BINDING)...
        if(target != NULL)
        {
            ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target = target;
            return 1;
        }
        return 0;
    }
}

static_inline void flushAndBindFramebuffer(GPU_Renderer* renderer, GLuint handle)
{
    // Bind the FBO
    renderer->impl->FlushBlitBuffer(renderer);

    extBindFramebuffer(renderer, handle);
    ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target = NULL;
}

static_inline void flushBlitBufferIfCurrentTexture(GPU_Renderer* renderer, GPU_Image* image)
{
    if(image == ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image)
    {
        renderer->impl->FlushBlitBuffer(renderer);
    }
}

static_inline void flushAndClearBlitBufferIfCurrentTexture(GPU_Renderer* renderer, GPU_Image* image)
{
    if(image == ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image)
    {
        renderer->impl->FlushBlitBuffer(renderer);
        ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image = NULL;
    }
}

static_inline Uint8 isCurrentTarget(GPU_Renderer* renderer, GPU_Target* target)
{
    return (target == ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target
            || ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target == NULL);
}

static_inline void flushAndClearBlitBufferIfCurrentFramebuffer(GPU_Renderer* renderer, GPU_Target* target)
{
    if(target == ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target
            || ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target == NULL)
    {
        renderer->impl->FlushBlitBuffer(renderer);
        ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target = NULL;
    }
}

static Uint8 growBlitBuffer(GPU_CONTEXT_DATA* cdata, unsigned int minimum_vertices_needed)
{
	unsigned int new_max_num_vertices;
	float* new_buffer;

    if(minimum_vertices_needed <= cdata->blit_buffer_max_num_vertices)
        return 1;
    if(cdata->blit_buffer_max_num_vertices == GPU_BLIT_BUFFER_ABSOLUTE_MAX_VERTICES)
        return 0;

    // Calculate new size (in vertices)
    new_max_num_vertices = ((unsigned int)cdata->blit_buffer_max_num_vertices) * 2;
    while(new_max_num_vertices <= minimum_vertices_needed)
        new_max_num_vertices *= 2;

    if(new_max_num_vertices > GPU_BLIT_BUFFER_ABSOLUTE_MAX_VERTICES)
        new_max_num_vertices = GPU_BLIT_BUFFER_ABSOLUTE_MAX_VERTICES;

    //GPU_LogError("Growing to %d vertices\n", new_max_num_vertices);
    // Resize the blit buffer
    new_buffer = (float*)SDL_malloc(new_max_num_vertices * GPU_BLIT_BUFFER_STRIDE);
    memcpy(new_buffer, cdata->blit_buffer, cdata->blit_buffer_num_vertices * GPU_BLIT_BUFFER_STRIDE);
    SDL_free(cdata->blit_buffer);
    cdata->blit_buffer = new_buffer;
    cdata->blit_buffer_max_num_vertices = new_max_num_vertices;

    #ifdef SDL_GPU_USE_BUFFER_PIPELINE
        // Resize the VBOs
        #if !defined(SDL_GPU_NO_VAO)
        glBindVertexArray(cdata->blit_VAO);
        #endif

        glBindBuffer(GL_ARRAY_BUFFER, cdata->blit_VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, GPU_BLIT_BUFFER_STRIDE * cdata->blit_buffer_max_num_vertices, NULL, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, cdata->blit_VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, GPU_BLIT_BUFFER_STRIDE * cdata->blit_buffer_max_num_vertices, NULL, GL_STREAM_DRAW);

        #if !defined(SDL_GPU_NO_VAO)
        glBindVertexArray(0);
        #endif
    #endif

    return 1;
}

static Uint8 growIndexBuffer(GPU_CONTEXT_DATA* cdata, unsigned int minimum_vertices_needed)
{
	unsigned int new_max_num_vertices;
	unsigned short* new_indices;

    if(minimum_vertices_needed <= cdata->index_buffer_max_num_vertices)
        return 1;
    if(cdata->index_buffer_max_num_vertices == GPU_INDEX_BUFFER_ABSOLUTE_MAX_VERTICES)
        return 0;

    // Calculate new size (in vertices)
    new_max_num_vertices = cdata->index_buffer_max_num_vertices * 2;
    while(new_max_num_vertices <= minimum_vertices_needed)
        new_max_num_vertices *= 2;

    if(new_max_num_vertices > GPU_INDEX_BUFFER_ABSOLUTE_MAX_VERTICES)
        new_max_num_vertices = GPU_INDEX_BUFFER_ABSOLUTE_MAX_VERTICES;

    //GPU_LogError("Growing to %d indices\n", new_max_num_vertices);
    // Resize the index buffer
    new_indices = (unsigned short*)SDL_malloc(new_max_num_vertices * sizeof(unsigned short));
    memcpy(new_indices, cdata->index_buffer, cdata->index_buffer_num_vertices * sizeof(unsigned short));
    SDL_free(cdata->index_buffer);
    cdata->index_buffer = new_indices;
    cdata->index_buffer_max_num_vertices = new_max_num_vertices;

    #ifdef SDL_GPU_USE_BUFFER_PIPELINE
        // Resize the IBO
        #if !defined(SDL_GPU_NO_VAO)
        glBindVertexArray(cdata->blit_VAO);
        #endif

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cdata->blit_IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * cdata->index_buffer_max_num_vertices, NULL, GL_DYNAMIC_DRAW);

        #if !defined(SDL_GPU_NO_VAO)
        glBindVertexArray(0);
        #endif
    #endif

    return 1;
}


// Only for window targets, which have their own contexts.
static void makeContextCurrent(GPU_Renderer* renderer, GPU_Target* target)
{
    if(target == NULL || target->context == NULL || renderer->current_context_target == target)
        return;

    renderer->impl->FlushBlitBuffer(renderer);

    #ifdef SDL_GPU_USE_SDL2
    SDL_GL_MakeCurrent(SDL_GetWindowFromID(target->context->windowID), target->context->context);
    #endif
    renderer->current_context_target = target;
}

static void setClipRect(GPU_Renderer* renderer, GPU_Target* target)
{
    if(target->use_clip_rect)
    {
        GPU_Target* context_target = renderer->current_context_target;
        glEnable(GL_SCISSOR_TEST);
        if(target->context != NULL)
        {
            int y;
            if(renderer->coordinate_mode == 0)
                y = context_target->h - (target->clip_rect.y + target->clip_rect.h);
            else
                y = target->clip_rect.y;
            float xFactor = ((float)context_target->context->drawable_w)/context_target->w;
            float yFactor = ((float)context_target->context->drawable_h)/context_target->h;
            glScissor(target->clip_rect.x * xFactor, y * yFactor, target->clip_rect.w * xFactor, target->clip_rect.h * yFactor);
        }
        else
            glScissor(target->clip_rect.x, target->clip_rect.y, target->clip_rect.w, target->clip_rect.h);
    }
}

static void unsetClipRect(GPU_Renderer* renderer, GPU_Target* target)
{
	(void)renderer;
    if(target->use_clip_rect)
        glDisable(GL_SCISSOR_TEST);
}

static void prepareToRenderToTarget(GPU_Renderer* renderer, GPU_Target* target)
{
    // Set up the camera
    renderer->impl->SetCamera(renderer, target, &target->camera);
}



static void changeColor(GPU_Renderer* renderer, SDL_Color color)
{
    #ifdef SDL_GPU_USE_BUFFER_PIPELINE
	(void)renderer;
	(void)color;
    return;
    #else
    GPU_CONTEXT_DATA* cdata = (GPU_CONTEXT_DATA*)renderer->current_context_target->context->data;
    if(cdata->last_color.r != color.r
        || cdata->last_color.g != color.g
        || cdata->last_color.b != color.b
        || GET_ALPHA(cdata->last_color) != GET_ALPHA(color))
    {
        renderer->impl->FlushBlitBuffer(renderer);
        cdata->last_color = color;
        glColor4f(color.r/255.01f, color.g/255.01f, color.b/255.01f, GET_ALPHA(color)/255.01f);
    }
    #endif
}

static void changeBlending(GPU_Renderer* renderer, Uint8 enable)
{
    GPU_CONTEXT_DATA* cdata = (GPU_CONTEXT_DATA*)renderer->current_context_target->context->data;
    if(cdata->last_use_blending == enable)
        return;

    renderer->impl->FlushBlitBuffer(renderer);

    if(enable)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);

    cdata->last_use_blending = enable;
}

static void forceChangeBlendMode(GPU_Renderer* renderer, GPU_BlendMode mode)
{
    GPU_CONTEXT_DATA* cdata = (GPU_CONTEXT_DATA*)renderer->current_context_target->context->data;

    renderer->impl->FlushBlitBuffer(renderer);

    cdata->last_blend_mode = mode;

    if(mode.source_color == mode.source_alpha && mode.dest_color == mode.dest_alpha)
    {
        glBlendFunc(mode.source_color, mode.dest_color);
    }
    else if(renderer->enabled_features & GPU_FEATURE_BLEND_FUNC_SEPARATE)
    {
        glBlendFuncSeparate(mode.source_color, mode.dest_color, mode.source_alpha, mode.dest_alpha);
    }
    else
    {
        GPU_PushErrorCode("(SDL_gpu internal)", GPU_ERROR_BACKEND_ERROR, "Could not set blend function because GPU_FEATURE_BLEND_FUNC_SEPARATE is not supported.");
    }

    if(renderer->enabled_features & GPU_FEATURE_BLEND_EQUATIONS)
    {
        if(mode.color_equation == mode.alpha_equation)
            glBlendEquation(mode.color_equation);
        else if(renderer->enabled_features & GPU_FEATURE_BLEND_EQUATIONS_SEPARATE)
            glBlendEquationSeparate(mode.color_equation, mode.alpha_equation);
        else
        {
            GPU_PushErrorCode("(SDL_gpu internal)", GPU_ERROR_BACKEND_ERROR, "Could not set blend equation because GPU_FEATURE_BLEND_EQUATIONS_SEPARATE is not supported.");
        }
    }
    else
    {
        GPU_PushErrorCode("(SDL_gpu internal)", GPU_ERROR_BACKEND_ERROR, "Could not set blend equation because GPU_FEATURE_BLEND_EQUATIONS is not supported.");
    }
}

static void changeBlendMode(GPU_Renderer* renderer, GPU_BlendMode mode)
{
    GPU_CONTEXT_DATA* cdata = (GPU_CONTEXT_DATA*)renderer->current_context_target->context->data;
    if(cdata->last_blend_mode.source_color == mode.source_color
       && cdata->last_blend_mode.dest_color == mode.dest_color
       && cdata->last_blend_mode.source_alpha == mode.source_alpha
       && cdata->last_blend_mode.dest_alpha == mode.dest_alpha
       && cdata->last_blend_mode.color_equation == mode.color_equation
       && cdata->last_blend_mode.alpha_equation == mode.alpha_equation)
        return;

    forceChangeBlendMode(renderer, mode);
}


// If 0 is returned, there is no valid shader.
static Uint32 get_proper_program_id(GPU_Renderer* renderer, Uint32 program_object)
{
    GPU_Context* context = renderer->current_context_target->context;
    if(context->default_textured_shader_program == 0)  // No shaders loaded!
        return 0;

    if(program_object == 0)
        return context->default_textured_shader_program;

    return program_object;
}



static void applyTexturing(GPU_Renderer* renderer)
{
    GPU_Context* context = renderer->current_context_target->context;
    if(context->use_texturing != ((GPU_CONTEXT_DATA*)context->data)->last_use_texturing)
    {
        ((GPU_CONTEXT_DATA*)context->data)->last_use_texturing = context->use_texturing;
        #ifndef SDL_GPU_SKIP_ENABLE_TEXTURE_2D
        if(context->use_texturing)
            glEnable(GL_TEXTURE_2D);
        else
            glDisable(GL_TEXTURE_2D);
        #endif
    }
}

static void changeTexturing(GPU_Renderer* renderer, Uint8 enable)
{
    GPU_Context* context = renderer->current_context_target->context;
    if(enable != ((GPU_CONTEXT_DATA*)context->data)->last_use_texturing)
    {
        renderer->impl->FlushBlitBuffer(renderer);

        ((GPU_CONTEXT_DATA*)context->data)->last_use_texturing = enable;
        #ifndef SDL_GPU_SKIP_ENABLE_TEXTURE_2D
        if(enable)
            glEnable(GL_TEXTURE_2D);
        else
            glDisable(GL_TEXTURE_2D);
        #endif
    }
}

static void enableTexturing(GPU_Renderer* renderer)
{
    if(!renderer->current_context_target->context->use_texturing)
    {
        renderer->impl->FlushBlitBuffer(renderer);
        renderer->current_context_target->context->use_texturing = 1;
    }
}

static void disableTexturing(GPU_Renderer* renderer)
{
    if(renderer->current_context_target->context->use_texturing)
    {
        renderer->impl->FlushBlitBuffer(renderer);
        renderer->current_context_target->context->use_texturing = 0;
    }
}

#define MIX_COLOR_COMPONENT_NORMALIZED_RESULT(a, b) ((a)/255.0f * (b)/255.0f)
#define MIX_COLOR_COMPONENT(a, b) (((a)/255.0f * (b)/255.0f)*255)

static void prepareToRenderImage(GPU_Renderer* renderer, GPU_Target* target, GPU_Image* image)
{
    GPU_Context* context = renderer->current_context_target->context;

    enableTexturing(renderer);
    if(GL_TRIANGLES != ((GPU_CONTEXT_DATA*)context->data)->last_shape)
    {
        renderer->impl->FlushBlitBuffer(renderer);
        ((GPU_CONTEXT_DATA*)context->data)->last_shape = GL_TRIANGLES;
    }

    // Blitting
    if(target->use_color)
    {
		SDL_Color color;
		color.r = MIX_COLOR_COMPONENT(target->color.r, image->color.r);
		color.g = MIX_COLOR_COMPONENT(target->color.g, image->color.g);
		color.b = MIX_COLOR_COMPONENT(target->color.b, image->color.b);
		GET_ALPHA(color) = MIX_COLOR_COMPONENT(GET_ALPHA(target->color), GET_ALPHA(image->color));

        changeColor(renderer, color);
    }
    else
        changeColor(renderer, image->color);
    changeBlending(renderer, image->use_blending);
    changeBlendMode(renderer, image->blend_mode);

    // If we're using the untextured shader, switch it.
    if(context->current_shader_program == context->default_untextured_shader_program)
        renderer->impl->ActivateShaderProgram(renderer, context->default_textured_shader_program, NULL);
}

static void prepareToRenderShapes(GPU_Renderer* renderer, unsigned int shape)
{
    GPU_Context* context = renderer->current_context_target->context;

    disableTexturing(renderer);
    if(shape != ((GPU_CONTEXT_DATA*)context->data)->last_shape)
    {
        renderer->impl->FlushBlitBuffer(renderer);
        ((GPU_CONTEXT_DATA*)context->data)->last_shape = shape;
    }

    // Shape rendering
    // Color is set elsewhere for shapes
    changeBlending(renderer, context->shapes_use_blending);
    changeBlendMode(renderer, context->shapes_blend_mode);

    // If we're using the textured shader, switch it.
    if(context->current_shader_program == context->default_textured_shader_program)
        renderer->impl->ActivateShaderProgram(renderer, context->default_untextured_shader_program, NULL);
}



static void forceChangeViewport(GPU_Target* target, GPU_Rect viewport)
{
	float y;
    GPU_CONTEXT_DATA* cdata = (GPU_CONTEXT_DATA*)(GPU_GetContextTarget()->context->data);

    cdata->last_viewport = viewport;

    y = viewport.y;
    if(GPU_GetCoordinateMode() == 0)
    {
        // Need the real height to flip the y-coord (from OpenGL coord system)
        if(target->image != NULL)
            y = target->image->h - viewport.h - viewport.y;
        else if(target->context != NULL)
            y = target->context->drawable_h - viewport.h - viewport.y;
    }

    glViewport(viewport.x, y, viewport.w, viewport.h);
}

static void changeViewport(GPU_Target* target)
{
    GPU_CONTEXT_DATA* cdata = (GPU_CONTEXT_DATA*)(GPU_GetContextTarget()->context->data);

    if(cdata->last_viewport.x == target->viewport.x && cdata->last_viewport.y == target->viewport.y && cdata->last_viewport.w == target->viewport.w && cdata->last_viewport.h == target->viewport.h)
        return;

    forceChangeViewport(target, target->viewport);
}

static void applyTargetCamera(GPU_Target* target)
{
    GPU_CONTEXT_DATA* cdata = (GPU_CONTEXT_DATA*)GPU_GetContextTarget()->context->data;
    Uint8 invert = (target->image != NULL);
	float offsetX, offsetY;

    cdata->last_camera = target->camera;

    cdata->last_camera_inverted = invert;

    GPU_MatrixMode( GPU_PROJECTION );
    GPU_LoadIdentity();

    if((!invert ^ GPU_GetCoordinateMode()))
        GPU_Ortho(target->camera.x, target->w + target->camera.x, target->h + target->camera.y, target->camera.y, -1.0f, 1.0f);
    else
        GPU_Ortho(target->camera.x, target->w + target->camera.x, target->camera.y, target->h + target->camera.y, -1.0f, 1.0f);  // Special inverted orthographic projection because tex coords are inverted already for render-to-texture

    GPU_MatrixMode( GPU_MODELVIEW );
    GPU_LoadIdentity();


    offsetX = target->w/2.0f;
    offsetY = target->h/2.0f;
    GPU_Translate(offsetX, offsetY, 0);
    GPU_Rotate(target->camera.angle, 0, 0, 1);
    GPU_Translate(-offsetX, -offsetY, 0);

    GPU_Translate(target->camera.x + offsetX, target->camera.y + offsetY, 0);
    GPU_Scale(target->camera.zoom, target->camera.zoom, 1.0f);
    GPU_Translate(-target->camera.x - offsetX, -target->camera.y - offsetY, 0);
}

static Uint8 equal_cameras(GPU_Camera a, GPU_Camera b)
{
    return (a.x == b.x && a.y == b.y && a.z == b.z && a.angle == b.angle && a.zoom == b.zoom);
}

static void changeCamera(GPU_Target* target)
{
    //GPU_CONTEXT_DATA* cdata = (GPU_CONTEXT_DATA*)GPU_GetContextTarget()->context->data;

    //if(cdata->last_camera_target != target || !equal_cameras(cdata->last_camera, target->camera))
    {
        applyTargetCamera(target);
    }
}

#ifdef SDL_GPU_APPLY_TRANSFORMS_TO_GL_STACK
static void applyTransforms(void)
{
    float* p = GPU_GetProjection();
    float* m = GPU_GetModelView();
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(p);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(m);
}
#endif


static GPU_Target* Init(GPU_Renderer* renderer, GPU_RendererID renderer_request, Uint16 w, Uint16 h, GPU_WindowFlagEnum SDL_flags)
{
	GPU_InitFlagEnum GPU_flags;
	SDL_Window* window;

#ifdef SDL_GPU_USE_OPENGL
	const char* vendor_string;
#endif

    if(renderer_request.major_version < 1)
    {
        renderer_request.major_version = 1;
        renderer_request.minor_version = 1;
    }

    // Tell SDL what we require for the GL context.
    GPU_flags = GPU_GetPreInitFlags();

    renderer->GPU_init_flags = GPU_flags;
    if(GPU_flags & GPU_INIT_DISABLE_DOUBLE_BUFFER)
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
    else
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#ifdef SDL_GPU_USE_SDL2

    // GL profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);  // Disable in case this is a fallback renderer
    #ifdef SDL_GPU_USE_GLES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    #endif
    // GL 3.2 and 3.3 have two profile modes
    // ARB_compatibility brings support for this to GL 3.1, but glGetStringi() via GLEW has chicken and egg problems.
    #if SDL_GPU_GL_MAJOR_VERSION == 3
    if(renderer_request.minor_version >= 2)
    {
        if(GPU_flags & GPU_INIT_REQUEST_COMPATIBILITY_PROFILE)
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
        else
        {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            // Force newer default shader version for core contexts because they don't support lower versions
            renderer->min_shader_version = SDL_GPU_GLSL_VERSION_CORE;
            if(renderer->min_shader_version > renderer->max_shader_version)
                renderer->max_shader_version = SDL_GPU_GLSL_VERSION_CORE;
        }

    }
    #endif

    // GL version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, renderer_request.major_version);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, renderer_request.minor_version);
#else
    // vsync for SDL 1.2
    if(!(GPU_flags & GPU_INIT_DISABLE_VSYNC))
        SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
#endif

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	renderer->requested_id = renderer_request;

#ifdef SDL_GPU_USE_SDL2

	window = NULL;
    // Is there a window already set up that we are supposed to use?
    if(renderer->current_context_target != NULL)
        window = SDL_GetWindowFromID(renderer->current_context_target->context->windowID);
    else
        window = SDL_GetWindowFromID(GPU_GetInitWindow());

    if(window == NULL)
    {
        int win_w, win_h;
        #ifdef __ANDROID__
        win_w = win_h = 0;  // Force Android to create full screen window
        #else
        win_w = w;
        win_h = h;
        #endif

        // Set up window flags
        SDL_flags |= SDL_WINDOW_OPENGL;
        if(!(SDL_flags & SDL_WINDOW_HIDDEN))
            SDL_flags |= SDL_WINDOW_SHOWN;

        renderer->SDL_init_flags = SDL_flags;
        window = SDL_CreateWindow("",
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  win_w, win_h,
                                  SDL_flags);

        if(window == NULL)
        {
            GPU_PushErrorCode("GPU_Init", GPU_ERROR_BACKEND_ERROR, "Window creation failed.");
            return NULL;
        }

        GPU_SetInitWindow(get_window_id(window));
    }
    else
        renderer->SDL_init_flags = SDL_flags;

#else
    SDL_flags |= SDL_OPENGL;
    renderer->SDL_init_flags = SDL_flags;
    window = SDL_SetVideoMode(w, h, 0, SDL_flags);

    if(window == NULL)
    {
        GPU_PushErrorCode("GPU_Init", GPU_ERROR_BACKEND_ERROR, "Screen surface creation failed.");
        return NULL;
    }
#endif

    renderer->enabled_features = 0xFFFFFFFF;  // Pretend to support them all if using incompatible headers


    // Create or re-init the current target.  This also creates the GL context and initializes enabled_features.
    if(renderer->impl->CreateTargetFromWindow(renderer, get_window_id(window), renderer->current_context_target) == NULL)
        return NULL;

    // If the dimensions of the window don't match what we asked for, then set up a virtual resolution to pretend like they are.
    if(!(GPU_flags & GPU_INIT_DISABLE_AUTO_VIRTUAL_RESOLUTION) && w != 0 && h != 0 && (w != renderer->current_context_target->w || h != renderer->current_context_target->h))
        renderer->impl->SetVirtualResolution(renderer, renderer->current_context_target, w, h);

    // Init glVertexAttrib workaround
    #ifdef SDL_GPU_USE_OPENGL
    vendor_string = (const char*)glGetString(GL_VENDOR);
    if(strstr(vendor_string, "Intel") != NULL)
    {
        vendor_is_Intel = 1;
        apply_Intel_attrib_workaround = 1;
    }
    #endif

    return renderer->current_context_target;
}


static Uint8 IsFeatureEnabled(GPU_Renderer* renderer, GPU_FeatureEnum feature)
{
    return ((renderer->enabled_features & feature) == feature);
}

static Uint8 get_GL_version(int* major, int* minor)
{
    const char* version_string;
    #ifdef SDL_GPU_USE_OPENGL
        // OpenGL < 3.0 doesn't have GL_MAJOR_VERSION.  Check via version string instead.
        version_string = (const char*)glGetString(GL_VERSION);
        if(sscanf(version_string, "%d.%d", major, minor) <= 0)
        {
            // Failure
            *major = SDL_GPU_GL_MAJOR_VERSION;
            #if SDL_GPU_GL_MAJOR_VERSION != 3
                *minor = 1;
            #else
                *minor = 0;
            #endif

            GPU_PushErrorCode(__func__, GPU_ERROR_BACKEND_ERROR, "Failed to parse OpenGL version string: \"%s\"", version_string);
            return 0;
        }
        return 1;
    #else
        // GLES doesn't have GL_MAJOR_VERSION.  Check via version string instead.
        version_string = (const char*)glGetString(GL_VERSION);
        // OpenGL ES 2.0?
        if(sscanf(version_string, "OpenGL ES %d.%d", major, minor) <= 0)
        {
            // OpenGL ES-CM 1.1?  OpenGL ES-CL 1.1?
            if(sscanf(version_string, "OpenGL ES-C%*c %d.%d", major, minor) <= 0)
            {
                // Failure
                *major = SDL_GPU_GLES_MAJOR_VERSION;
                #if SDL_GPU_GLES_MAJOR_VERSION == 1
                    *minor = 1;
                #else
                    *minor = 0;
                #endif

                GPU_PushErrorCode(__func__, GPU_ERROR_BACKEND_ERROR, "Failed to parse OpenGL ES version string: \"%s\"", version_string);
                return 0;
            }
        }
        return 1;
    #endif
}

static Uint8 get_GLSL_version(int* version)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
        const char* version_string;
        int major, minor;
        #ifdef SDL_GPU_USE_OPENGL
            {
                version_string = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
                if(version_string == NULL || sscanf(version_string, "%d.%d", &major, &minor) <= 0)
                {
                    GPU_PushErrorCode(__func__, GPU_ERROR_BACKEND_ERROR, "Failed to parse GLSL version string: \"%s\"", version_string);
                    *version = SDL_GPU_GLSL_VERSION;
                    return 0;
                }
                else
                    *version = major*100 + minor;
            }
        #else
            {
                version_string = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
                if(version_string == NULL || sscanf(version_string, "OpenGL ES GLSL ES %d.%d", &major, &minor) <= 0)
                {
                    GPU_PushErrorCode(__func__, GPU_ERROR_BACKEND_ERROR, "Failed to parse GLSL ES version string: \"%s\"", version_string);
                    *version = SDL_GPU_GLSL_VERSION;
                    return 0;
                }
                else
                    *version = major*100 + minor;
            }
        #endif
    #endif
    return 1;
}

static Uint8 get_API_versions(GPU_Renderer* renderer)
{
    return (get_GL_version(&renderer->id.major_version, &renderer->id.minor_version)
           && get_GLSL_version(&renderer->max_shader_version));
}


static void update_stored_dimensions(GPU_Target* target)
{
    Uint8 is_fullscreen;
    SDL_Window* window;
    
    if(target->context == NULL)
        return;
    
    window = get_window(target->context->windowID);
    get_window_dimensions(window, &target->context->window_w, &target->context->window_h);
    is_fullscreen = get_fullscreen_state(window);
    
    if(!is_fullscreen)
    {
        target->context->stored_window_w = target->context->window_w;
        target->context->stored_window_h = target->context->window_h;
    }
}

static GPU_Target* CreateTargetFromWindow(GPU_Renderer* renderer, Uint32 windowID, GPU_Target* target)
{
    Uint8 created = 0;  // Make a new one or repurpose an existing target?
	GPU_CONTEXT_DATA* cdata;
	SDL_Window* window;
	
	int framebuffer_handle;
	SDL_Color white = { 255, 255, 255, 255 };
#ifdef SDL_GPU_USE_OPENGL
	GLenum err;
#endif
	GPU_FeatureEnum required_features = GPU_GetRequiredFeatures();

    if(target == NULL)
	{
		int blit_buffer_storage_size;
		int index_buffer_storage_size;

        created = 1;
        target = (GPU_Target*)SDL_malloc(sizeof(GPU_Target));
        memset(target, 0, sizeof(GPU_Target));
        target->refcount = 1;
        target->is_alias = 0;
        target->data = (GPU_TARGET_DATA*)SDL_malloc(sizeof(GPU_TARGET_DATA));
        memset(target->data, 0, sizeof(GPU_TARGET_DATA));
        ((GPU_TARGET_DATA*)target->data)->refcount = 1;
        target->image = NULL;
        target->context = (GPU_Context*)SDL_malloc(sizeof(GPU_Context));
        memset(target->context, 0, sizeof(GPU_Context));
        cdata = (GPU_CONTEXT_DATA*)SDL_malloc(sizeof(GPU_CONTEXT_DATA));
        memset(cdata, 0, sizeof(GPU_CONTEXT_DATA));
        target->context->data = cdata;
        target->context->context = NULL;

        cdata->last_image = NULL;
        cdata->last_target = NULL;
        // Initialize the blit buffer
        cdata->blit_buffer_max_num_vertices = GPU_BLIT_BUFFER_INIT_MAX_NUM_VERTICES;
        cdata->blit_buffer_num_vertices = 0;
        blit_buffer_storage_size = GPU_BLIT_BUFFER_INIT_MAX_NUM_VERTICES*GPU_BLIT_BUFFER_STRIDE;
        cdata->blit_buffer = (float*)SDL_malloc(blit_buffer_storage_size);
        cdata->index_buffer_max_num_vertices = GPU_BLIT_BUFFER_INIT_MAX_NUM_VERTICES;
        cdata->index_buffer_num_vertices = 0;
        index_buffer_storage_size = GPU_BLIT_BUFFER_INIT_MAX_NUM_VERTICES*sizeof(unsigned short);
        cdata->index_buffer = (unsigned short*)SDL_malloc(index_buffer_storage_size);
    }
    else
    {
        GPU_RemoveWindowMapping(target->context->windowID);
        cdata = (GPU_CONTEXT_DATA*)target->context->data;
    }


    window = get_window(windowID);
    if(window == NULL)
    {
        GPU_PushErrorCode("GPU_CreateTargetFromWindow", GPU_ERROR_BACKEND_ERROR, "Failed to acquire the window from the given ID.");
        if(created)
        {
            SDL_free(cdata->blit_buffer);
            SDL_free(cdata->index_buffer);
            SDL_free(target->context->data);
            SDL_free(target->context);
            SDL_free(target->data);
            SDL_free(target);
        }
        return NULL;
    }

    // Store the window info
    target->context->windowID = get_window_id(window);

    #ifdef SDL_GPU_USE_SDL2
    // Make a new context if needed and make it current
    if(created || target->context->context == NULL)
    {
        target->context->context = SDL_GL_CreateContext(window);
        GPU_AddWindowMapping(target);
    }
    
    // We need a GL context before we can get the drawable size.
    SDL_GL_GetDrawableSize(window, &target->context->drawable_w, &target->context->drawable_h);

    #else

    target->context->drawable_w = window->w;
    target->context->drawable_h = window->h;

    #endif
    
    update_stored_dimensions(target);


    ((GPU_TARGET_DATA*)target->data)->handle = 0;
    ((GPU_TARGET_DATA*)target->data)->format = GL_RGBA;

    target->renderer = renderer;
    target->w = target->context->drawable_w;
    target->h = target->context->drawable_h;
    target->base_w = target->context->drawable_w;
    target->base_h = target->context->drawable_h;

    target->use_clip_rect = 0;
    target->clip_rect.x = 0;
    target->clip_rect.y = 0;
    target->clip_rect.w = target->w;
    target->clip_rect.h = target->h;
    target->use_color = 0;

    target->viewport = GPU_MakeRect(0, 0, target->context->drawable_w, target->context->drawable_h);
    target->camera = GPU_GetDefaultCamera();

    target->context->line_thickness = 1.0f;
    target->context->use_texturing = 1;
    target->context->shapes_use_blending = 1;
    target->context->shapes_blend_mode = GPU_GetBlendModeFromPreset(GPU_BLEND_NORMAL);

    cdata->last_color = white;

    cdata->last_use_texturing = 1;
    cdata->last_shape = GL_TRIANGLES;

    cdata->last_use_blending = 0;
    cdata->last_blend_mode = GPU_GetBlendModeFromPreset(GPU_BLEND_NORMAL);

    cdata->last_viewport = target->viewport;
    cdata->last_camera = target->camera;  // Redundant due to applyTargetCamera(), below
    cdata->last_camera_inverted = 0;

    #ifdef SDL_GPU_USE_OPENGL
    glewExperimental = GL_TRUE;  // Force GLEW to get exported functions instead of checking via extension string
    err = glewInit();
    if (GLEW_OK != err)
    {
        // Probably don't have the right GL version for this renderer
        GPU_PushErrorCode("GPU_CreateTargetFromWindow", GPU_ERROR_BACKEND_ERROR, "Failed to initialize extensions for renderer %s.", renderer->id.name);
        target->context->failed = 1;
        return NULL;
    }
    #endif

    renderer->impl->MakeCurrent(renderer, target, target->context->windowID);

    framebuffer_handle = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer_handle);
    ((GPU_TARGET_DATA*)target->data)->handle = framebuffer_handle;


    // Update our renderer info from the current GL context.
    if(!get_API_versions(renderer))
        GPU_PushErrorCode("GPU_CreateTargetFromWindow", GPU_ERROR_BACKEND_ERROR, "Failed to get backend API versions.");

    // Did the wrong runtime library try to use a later versioned renderer?
    if(renderer->id.major_version < renderer->requested_id.major_version)
    {
        GPU_PushErrorCode("GPU_CreateTargetFromWindow", GPU_ERROR_BACKEND_ERROR, "Renderer major version (%d) is incompatible with the available OpenGL runtime library version (%d).", renderer->requested_id.major_version, renderer->id.major_version);
        target->context->failed = 1;
        return NULL;
    }
    

    init_features(renderer);

    if(!IsFeatureEnabled(renderer, required_features))
    {
        GPU_PushErrorCode("GPU_CreateTargetFromWindow", GPU_ERROR_BACKEND_ERROR, "Renderer does not support required features.");
        target->context->failed = 1;
        return NULL;
    }

    #ifdef SDL_GPU_USE_SDL2
    // No preference for vsync?
    if(!(renderer->GPU_init_flags & (GPU_INIT_DISABLE_VSYNC | GPU_INIT_ENABLE_VSYNC)))
    {
        // Default to late swap vsync if available
        if(SDL_GL_SetSwapInterval(-1) < 0)
            SDL_GL_SetSwapInterval(1);  // Or go for vsync
    }
    else if(renderer->GPU_init_flags & GPU_INIT_ENABLE_VSYNC)
        SDL_GL_SetSwapInterval(1);
    else if(renderer->GPU_init_flags & GPU_INIT_DISABLE_VSYNC)
        SDL_GL_SetSwapInterval(0);
    #endif

    // Set up GL state

    target->context->projection_matrix.size = 1;
    GPU_MatrixIdentity(target->context->projection_matrix.matrix[0]);

    target->context->modelview_matrix.size = 1;
    GPU_MatrixIdentity(target->context->modelview_matrix.matrix[0]);

    target->context->matrix_mode = GPU_MODELVIEW;

    // Modes
    #ifndef SDL_GPU_SKIP_ENABLE_TEXTURE_2D
    glEnable(GL_TEXTURE_2D);
    #endif
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_BLEND);
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    // Viewport and Framebuffer
    glViewport(0.0f, 0.0f, target->viewport.w, target->viewport.h);

    glClear( GL_COLOR_BUFFER_BIT );
    #if SDL_GPU_GL_TIER < 3
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    #endif

    // Set up camera
    applyTargetCamera(target);

    renderer->impl->SetLineThickness(renderer, 1.0f);


    #ifdef SDL_GPU_USE_BUFFER_PIPELINE
        // Create vertex array container and buffer
        #if !defined(SDL_GPU_NO_VAO)
        glGenVertexArrays(1, &cdata->blit_VAO);
        glBindVertexArray(cdata->blit_VAO);
        #endif
    #endif

    target->context->default_textured_shader_program = 0;
    target->context->default_untextured_shader_program = 0;
    target->context->current_shader_program = 0;

    #ifndef SDL_GPU_DISABLE_SHADERS
    // Load default shaders

    if(IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
    {
        Uint32 v, f, p;
        const char* textured_vertex_shader_source = GPU_DEFAULT_TEXTURED_VERTEX_SHADER_SOURCE;
        const char* textured_fragment_shader_source = GPU_DEFAULT_TEXTURED_FRAGMENT_SHADER_SOURCE;
        const char* untextured_vertex_shader_source = GPU_DEFAULT_UNTEXTURED_VERTEX_SHADER_SOURCE;
        const char* untextured_fragment_shader_source = GPU_DEFAULT_UNTEXTURED_FRAGMENT_SHADER_SOURCE;

        #ifdef SDL_GPU_ENABLE_CORE_SHADERS
        // Use core shaders only when supported by the actual context we got
        if(renderer->id.major_version > 3 || (renderer->id.major_version == 3 && renderer->id.minor_version >= 2))
        {
            textured_vertex_shader_source = GPU_DEFAULT_TEXTURED_VERTEX_SHADER_SOURCE_CORE;
            textured_fragment_shader_source = GPU_DEFAULT_TEXTURED_FRAGMENT_SHADER_SOURCE_CORE;
            untextured_vertex_shader_source = GPU_DEFAULT_UNTEXTURED_VERTEX_SHADER_SOURCE_CORE;
            untextured_fragment_shader_source = GPU_DEFAULT_UNTEXTURED_FRAGMENT_SHADER_SOURCE_CORE;
        }
        #endif

        // Textured shader
        v = renderer->impl->CompileShader(renderer, GPU_VERTEX_SHADER, textured_vertex_shader_source);

        if(!v)
        {
            GPU_PushErrorCode("GPU_CreateTargetFromWindow", GPU_ERROR_BACKEND_ERROR, "Failed to load default textured vertex shader: %s.", GPU_GetShaderMessage());
            target->context->failed = 1;
            return NULL;
        }

        f = renderer->impl->CompileShader(renderer, GPU_FRAGMENT_SHADER, textured_fragment_shader_source);

        if(!f)
        {
            GPU_PushErrorCode("GPU_CreateTargetFromWindow", GPU_ERROR_BACKEND_ERROR, "Failed to load default textured fragment shader: %s.", GPU_GetShaderMessage());
            target->context->failed = 1;
            return NULL;
        }

        p = renderer->impl->CreateShaderProgram(renderer);
        renderer->impl->AttachShader(renderer, p, v);
        renderer->impl->AttachShader(renderer, p, f);
        renderer->impl->LinkShaderProgram(renderer, p);

        if(!p)
        {
            GPU_PushErrorCode("GPU_CreateTargetFromWindow", GPU_ERROR_BACKEND_ERROR, "Failed to link default textured shader program: %s.", GPU_GetShaderMessage());
            target->context->failed = 1;
            return NULL;
        }

        target->context->default_textured_shader_program = p;

        // Get locations of the attributes in the shader
        cdata->shader_block[0] = GPU_LoadShaderBlock(p, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "gpu_ModelViewProjectionMatrix");


        // Untextured shader
        v = renderer->impl->CompileShader(renderer, GPU_VERTEX_SHADER, untextured_vertex_shader_source);

        if(!v)
        {
            GPU_PushErrorCode("GPU_CreateTargetFromWindow", GPU_ERROR_BACKEND_ERROR, "Failed to load default untextured vertex shader: %s.", GPU_GetShaderMessage());
            target->context->failed = 1;
            return NULL;
        }

        f = renderer->impl->CompileShader(renderer, GPU_FRAGMENT_SHADER, untextured_fragment_shader_source);

        if(!f)
        {
            GPU_PushErrorCode("GPU_CreateTargetFromWindow", GPU_ERROR_BACKEND_ERROR, "Failed to load default untextured fragment shader: %s.", GPU_GetShaderMessage());
            target->context->failed = 1;
            return NULL;
        }

        p = renderer->impl->CreateShaderProgram(renderer);
        renderer->impl->AttachShader(renderer, p, v);
        renderer->impl->AttachShader(renderer, p, f);
        renderer->impl->LinkShaderProgram(renderer, p);

        if(!p)
        {
            GPU_PushErrorCode("GPU_CreateTargetFromWindow", GPU_ERROR_BACKEND_ERROR, "Failed to link default untextured shader program: %s.", GPU_GetShaderMessage());
            target->context->failed = 1;
            return NULL;
        }

        glUseProgram(p);

        target->context->default_untextured_shader_program = target->context->current_shader_program = p;

        // Get locations of the attributes in the shader
        cdata->shader_block[1] = GPU_LoadShaderBlock(p, "gpu_Vertex", NULL, "gpu_Color", "gpu_ModelViewProjectionMatrix");
        GPU_SetShaderBlock(cdata->shader_block[1]);

    }
    else
    {
        snprintf(shader_message, 256, "Shaders not supported by this hardware.  Default shaders are disabled.\n");
        target->context->default_untextured_shader_program = target->context->current_shader_program = 0;
    }

    #ifdef SDL_GPU_USE_BUFFER_PIPELINE
        // Create vertex array container and buffer

        glGenBuffers(2, cdata->blit_VBO);
        // Create space on the GPU
        glBindBuffer(GL_ARRAY_BUFFER, cdata->blit_VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, GPU_BLIT_BUFFER_STRIDE * cdata->blit_buffer_max_num_vertices, NULL, GL_STREAM_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, cdata->blit_VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, GPU_BLIT_BUFFER_STRIDE * cdata->blit_buffer_max_num_vertices, NULL, GL_STREAM_DRAW);
        cdata->blit_VBO_flop = 0;

        glGenBuffers(1, &cdata->blit_IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cdata->blit_IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * cdata->blit_buffer_max_num_vertices, NULL, GL_DYNAMIC_DRAW);

        glGenBuffers(16, cdata->attribute_VBO);

        // Init 16 attributes to 0 / NULL.
        memset(cdata->shader_attributes, 0, 16*sizeof(GPU_AttributeSource));
    #endif
    #endif

    return target;
}


static GPU_Target* CreateAliasTarget(GPU_Renderer* renderer, GPU_Target* target)
{
	GPU_Target* result;
	(void)renderer;

    if(target == NULL)
        return NULL;

    result = (GPU_Target*)SDL_malloc(sizeof(GPU_Target));

    // Copy the members
    *result = *target;

    // Alias info
    if(target->image != NULL)
        target->image->refcount++;
    ((GPU_TARGET_DATA*)target->data)->refcount++;
    result->refcount = 1;
    result->is_alias = 1;

    return result;
}

static void MakeCurrent(GPU_Renderer* renderer, GPU_Target* target, Uint32 windowID)
{
	SDL_Window* window;

    if(target == NULL)
        return;
    
    if(target->image != NULL)
        return;
    

    if(target->context->context != NULL)
    {
        renderer->current_context_target = target;
        #ifdef SDL_GPU_USE_SDL2
        SDL_GL_MakeCurrent(SDL_GetWindowFromID(windowID), target->context->context);
        #endif
        
        // Reset window mapping, base size, and camera if the target's window was changed
        if(target->context->windowID != windowID)
        {
            renderer->impl->FlushBlitBuffer(renderer);

            // Update the window mappings
            GPU_RemoveWindowMapping(windowID);
            // Don't remove the target's current mapping.  That lets other windows refer to it.
            target->context->windowID = windowID;
            GPU_AddWindowMapping(target);

            // Update target's window size
            window = get_window(windowID);
            if(window != NULL)
            {
                get_window_dimensions(window, &target->context->window_w, &target->context->window_h);
                get_drawable_dimensions(window, &target->context->drawable_w, &target->context->drawable_h);
                target->base_w = target->context->drawable_w;
                target->base_h = target->context->drawable_h;
            }

            // Reset the camera for this window
            applyTargetCamera(((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target);
        }
    }
}


static void SetAsCurrent(GPU_Renderer* renderer)
{
    if(renderer->current_context_target == NULL)
        return;

    renderer->impl->MakeCurrent(renderer, renderer->current_context_target, renderer->current_context_target->context->windowID);
}

static void ResetRendererState(GPU_Renderer* renderer)
{
    GPU_Target* target;
    GPU_CONTEXT_DATA* cdata;

    if(renderer->current_context_target == NULL)
        return;

    target = renderer->current_context_target;
    cdata = (GPU_CONTEXT_DATA*)target->context->data;


    #ifndef SDL_GPU_DISABLE_SHADERS
    if(IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        glUseProgram(target->context->current_shader_program);
    #endif

    #ifdef SDL_GPU_USE_SDL2
    SDL_GL_MakeCurrent(SDL_GetWindowFromID(target->context->windowID), target->context->context);
    #endif


    #ifndef SDL_GPU_USE_BUFFER_PIPELINE
    glColor4f(cdata->last_color.r/255.01f, cdata->last_color.g/255.01f, cdata->last_color.b/255.01f, GET_ALPHA(cdata->last_color)/255.01f);
    #endif
    #ifndef SDL_GPU_SKIP_ENABLE_TEXTURE_2D
    if(cdata->last_use_texturing)
        glEnable(GL_TEXTURE_2D);
    else
        glDisable(GL_TEXTURE_2D);
    #endif

    if(cdata->last_use_blending)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);

    forceChangeBlendMode(renderer, cdata->last_blend_mode);

    forceChangeViewport(target, target->viewport);

    if(cdata->last_image != NULL)
        glBindTexture(GL_TEXTURE_2D, ((GPU_IMAGE_DATA*)(cdata->last_image)->data)->handle);

    if(cdata->last_target != NULL)
        extBindFramebuffer(renderer, ((GPU_TARGET_DATA*)cdata->last_target->data)->handle);
    else
        extBindFramebuffer(renderer, ((GPU_TARGET_DATA*)target->data)->handle);
}

static Uint8 SetWindowResolution(GPU_Renderer* renderer, Uint16 w, Uint16 h)
{
    GPU_Target* target = renderer->current_context_target;

    Uint8 isCurrent = isCurrentTarget(renderer, target);
    if(isCurrent)
        renderer->impl->FlushBlitBuffer(renderer);

    // Don't need to resize (only update internals) when resolution isn't changing.
    get_target_window_dimensions(target, &target->context->window_w, &target->context->window_h);
    get_target_drawable_dimensions(target, &target->context->drawable_w, &target->context->drawable_h);
    if(target->context->window_w != w || target->context->window_h != h)
    {
        resize_window(target, w, h);
        get_target_window_dimensions(target, &target->context->window_w, &target->context->window_h);
        get_target_drawable_dimensions(target, &target->context->drawable_w, &target->context->drawable_h);
    }

#ifdef SDL_GPU_USE_SDL1

    // FIXME: Does the entire GL state need to be reset because the screen was recreated?
    {
        GPU_Context* context;

        // Reset texturing state
        context = renderer->current_context_target->context;
        context->use_texturing = 1;
        ((GPU_CONTEXT_DATA*)context->data)->last_use_texturing = 0;
    }

    // Clear target (no state change)
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT );
#endif

    // Store the resolution for fullscreen_desktop changes
    update_stored_dimensions(target);

    // Update base dimensions
    target->base_w = target->context->drawable_w;
    target->base_h = target->context->drawable_h;

    // Resets virtual resolution
    target->w = target->base_w;
    target->h = target->base_h;
    target->using_virtual_resolution = 0;

    // Resets viewport
    target->viewport = GPU_MakeRect(0, 0, target->w, target->h);
    changeViewport(target);

    GPU_UnsetClip(target);

    if(isCurrent)
        applyTargetCamera(target);

    return 1;
}

static void SetVirtualResolution(GPU_Renderer* renderer, GPU_Target* target, Uint16 w, Uint16 h)
{
	Uint8 isCurrent;

    if(target == NULL)
        return;

    isCurrent = isCurrentTarget(renderer, target);
    if(isCurrent)
        renderer->impl->FlushBlitBuffer(renderer);

    target->w = w;
    target->h = h;
    target->using_virtual_resolution = 1;

    if(isCurrent)
        applyTargetCamera(target);
}

static void UnsetVirtualResolution(GPU_Renderer* renderer, GPU_Target* target)
{
	Uint8 isCurrent;

    if(target == NULL)
        return;

    isCurrent = isCurrentTarget(renderer, target);
    if(isCurrent)
        renderer->impl->FlushBlitBuffer(renderer);

    target->w = target->base_w;
    target->h = target->base_h;

    target->using_virtual_resolution = 0;

    if(isCurrent)
        applyTargetCamera(target);
}

static void Quit(GPU_Renderer* renderer)
{
    renderer->impl->FreeTarget(renderer, renderer->current_context_target);
    renderer->current_context_target = NULL;
}



static Uint8 SetFullscreen(GPU_Renderer* renderer, Uint8 enable_fullscreen, Uint8 use_desktop_resolution)
{
    GPU_Target* target = renderer->current_context_target;

#ifdef SDL_GPU_USE_SDL2
    SDL_Window* window = SDL_GetWindowFromID(target->context->windowID);
    Uint32 old_flags = SDL_GetWindowFlags(window);
    Uint8 was_fullscreen = (old_flags & SDL_WINDOW_FULLSCREEN);
    Uint8 is_fullscreen = was_fullscreen;

    Uint32 flags = 0;

    if(enable_fullscreen)
    {
        if(use_desktop_resolution)
            flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
        else
            flags = SDL_WINDOW_FULLSCREEN;
    }

    if(SDL_SetWindowFullscreen(window, flags) >= 0)
    {
        flags = SDL_GetWindowFlags(window);
        is_fullscreen = (flags & SDL_WINDOW_FULLSCREEN);

        // If we just went fullscreen, save the original resolution
        // We do this because you can't depend on the resolution to be preserved by SDL
        // SDL_WINDOW_FULLSCREEN_DESKTOP changes the resolution and SDL_WINDOW_FULLSCREEN can change it when a given mode is not available
        if(!was_fullscreen && is_fullscreen)
        {
            target->context->stored_window_w = target->context->window_w;
            target->context->stored_window_h = target->context->window_h;
        }

        // If we're in windowed mode now and a resolution was stored, restore the original window resolution
        if(was_fullscreen && !is_fullscreen && (target->context->stored_window_w != 0 && target->context->stored_window_h != 0))
            SDL_SetWindowSize(window, target->context->stored_window_w, target->context->stored_window_h);
    }

#else
    SDL_Surface* surf = SDL_GetVideoSurface();
	Uint8 was_fullscreen = (surf->flags & SDL_FULLSCREEN);
	Uint8 is_fullscreen = was_fullscreen;

    if(was_fullscreen ^ enable_fullscreen)
    {
        SDL_WM_ToggleFullScreen(surf);
        is_fullscreen = (surf->flags & SDL_FULLSCREEN);
    }

#endif

    if(is_fullscreen != was_fullscreen)
    {
        // Update window dims
        get_target_window_dimensions(target, &target->context->window_w, &target->context->window_h);
        get_target_drawable_dimensions(target, &target->context->drawable_w, &target->context->drawable_h);
        
        // If virtual res is not set, we need to update the target dims and reset stuff that no longer is right
        if(!target->using_virtual_resolution)
        {
            // Update dims
            target->w = target->context->drawable_w;
            target->h = target->context->drawable_h;
        }

        // Reset viewport
        target->viewport = GPU_MakeRect(0, 0, target->context->drawable_w, target->context->drawable_h);
        changeViewport(target);

        // Reset clip
        GPU_UnsetClip(target);

        // Update camera
        if(isCurrentTarget(renderer, target))
            applyTargetCamera(target);
    }

    target->base_w = target->context->drawable_w;
    target->base_h = target->context->drawable_h;

    return is_fullscreen;
}


static GPU_Camera SetCamera(GPU_Renderer* renderer, GPU_Target* target, GPU_Camera* cam)
{
    GPU_Camera new_camera;
	GPU_Camera old_camera;

    if(target == NULL)
    {
        GPU_PushErrorCode("GPU_SetCamera", GPU_ERROR_NULL_ARGUMENT, "target");
        return GPU_GetDefaultCamera();
    }

    if(cam == NULL)
        new_camera = GPU_GetDefaultCamera();
    else
        new_camera = *cam;

    old_camera = target->camera;

    if(!equal_cameras(new_camera, old_camera))
    {
        if(isCurrentTarget(renderer, target))
            renderer->impl->FlushBlitBuffer(renderer);

        target->camera = new_camera;
    }

    return old_camera;
}

static GLuint CreateUninitializedTexture(GPU_Renderer* renderer)
{
    GLuint handle;

    glGenTextures(1, &handle);
    if(handle == 0)
        return 0;

    flushAndBindTexture(renderer, handle);

    // Set the texture's stretching properties
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    #if defined(SDL_GPU_USE_GLES) && (SDL_GPU_GLES_TIER == 1)
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    #endif

    return handle;
}

static GPU_Image* CreateUninitializedImage(GPU_Renderer* renderer, Uint16 w, Uint16 h, GPU_FormatEnum format)
{
    GLuint handle, num_layers, bytes_per_pixel;
    GLenum gl_format;
	GPU_Image* result;
	GPU_IMAGE_DATA* data;
	SDL_Color white = { 255, 255, 255, 255 };

    switch(format)
    {
        case GPU_FORMAT_LUMINANCE:
            gl_format = GL_LUMINANCE;
            num_layers = 1;
            bytes_per_pixel = 1;
            break;
        case GPU_FORMAT_LUMINANCE_ALPHA:
            gl_format = GL_LUMINANCE_ALPHA;
            num_layers = 1;
            bytes_per_pixel = 2;
            break;
        case GPU_FORMAT_RGB:
            gl_format = GL_RGB;
            num_layers = 1;
            bytes_per_pixel = 3;
            break;
        case GPU_FORMAT_RGBA:
            gl_format = GL_RGBA;
            num_layers = 1;
            bytes_per_pixel = 4;
            break;
        case GPU_FORMAT_ALPHA:
            gl_format = GL_ALPHA;
            num_layers = 1;
            bytes_per_pixel = 1;
            break;
        #ifndef SDL_GPU_USE_GLES
        case GPU_FORMAT_RG:
            gl_format = GL_RG;
            num_layers = 1;
            bytes_per_pixel = 2;
            break;
        #endif
        case GPU_FORMAT_YCbCr420P:
            gl_format = GL_LUMINANCE;
            num_layers = 3;
            bytes_per_pixel = 1;
            break;
        case GPU_FORMAT_YCbCr422:
            gl_format = GL_LUMINANCE;
            num_layers = 3;
            bytes_per_pixel = 1;
            break;
        default:
            GPU_PushErrorCode("GPU_CreateUninitializedImage", GPU_ERROR_DATA_ERROR, "Unsupported image format (0x%x)", format);
            return NULL;
    }

    if(bytes_per_pixel < 1 || bytes_per_pixel > 4)
    {
        GPU_PushErrorCode("GPU_CreateUninitializedImage", GPU_ERROR_DATA_ERROR, "Unsupported number of bytes per pixel (%d)", bytes_per_pixel);
        return NULL;
    }

    // Create the underlying texture
    handle = CreateUninitializedTexture(renderer);
    if(handle == 0)
    {
        GPU_PushErrorCode("GPU_CreateUninitializedImage", GPU_ERROR_BACKEND_ERROR, "Failed to generate a texture handle.");
        return NULL;
    }

    // Create the GPU_Image
    result = (GPU_Image*)SDL_malloc(sizeof(GPU_Image));
    result->refcount = 1;
    data = (GPU_IMAGE_DATA*)SDL_malloc(sizeof(GPU_IMAGE_DATA));
    data->refcount = 1;
    result->target = NULL;
    result->renderer = renderer;
    result->format = format;
    result->num_layers = num_layers;
    result->bytes_per_pixel = bytes_per_pixel;
    result->has_mipmaps = 0;

    result->color = white;
    result->use_blending = 1;
    result->blend_mode = GPU_GetBlendModeFromPreset(GPU_BLEND_NORMAL);
    result->filter_mode = GPU_FILTER_LINEAR;
    result->snap_mode = GPU_SNAP_POSITION_AND_DIMENSIONS;
    result->wrap_mode_x = GPU_WRAP_NONE;
    result->wrap_mode_y = GPU_WRAP_NONE;

    result->data = data;
    result->is_alias = 0;
    data->handle = handle;
    data->owns_handle = 1;
    data->format = gl_format;

    result->using_virtual_resolution = 0;
    result->w = w;
    result->h = h;
    result->base_w = w;
    result->base_h = h;
    // POT textures will change this later
    result->texture_w = w;
    result->texture_h = h;

    return result;
}


static GPU_Image* CreateImage(GPU_Renderer* renderer, Uint16 w, Uint16 h, GPU_FormatEnum format)
{
	GPU_Image* result;
	GLenum internal_format;
	static unsigned char* zero_buffer = NULL;
	static unsigned int zero_buffer_size = 0;

    if(format < 1)
    {
        GPU_PushErrorCode("GPU_CreateImage", GPU_ERROR_DATA_ERROR, "Unsupported image format (0x%x)", format);
        return NULL;
    }

    result = CreateUninitializedImage(renderer, w, h, format);

    if(result == NULL)
    {
        GPU_PushErrorCode("GPU_CreateImage", GPU_ERROR_BACKEND_ERROR, "Could not create image as requested.");
        return NULL;
    }

    changeTexturing(renderer, 1);
    bindTexture(renderer, result);

    internal_format = ((GPU_IMAGE_DATA*)(result->data))->format;
    w = result->w;
    h = result->h;
    if(!(renderer->enabled_features & GPU_FEATURE_NON_POWER_OF_TWO))
    {
        if(!isPowerOfTwo(w))
            w = getNearestPowerOf2(w);
        if(!isPowerOfTwo(h))
            h = getNearestPowerOf2(h);
    }

    // Initialize texture using a blank buffer
    if(zero_buffer_size < (unsigned int)(w*h*result->bytes_per_pixel))
    {
        SDL_free(zero_buffer);
        zero_buffer_size = w*h*result->bytes_per_pixel;
        zero_buffer = (unsigned char*)SDL_malloc(zero_buffer_size);
        memset(zero_buffer, 0, zero_buffer_size);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    #ifdef SDL_GPU_USE_OPENGL
    glPixelStorei(GL_UNPACK_ROW_LENGTH, w);
    #endif

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0,
                 internal_format, GL_UNSIGNED_BYTE, zero_buffer);
    // Tell SDL_gpu what we got (power-of-two requirements have made this change)
    result->texture_w = w;
    result->texture_h = h;

    // Restore GL defaults
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    #ifdef SDL_GPU_USE_OPENGL
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    #endif

    return result;
}


static GPU_Image* CreateImageUsingTexture(GPU_Renderer* renderer, Uint32 handle, Uint8 take_ownership)
{
    #ifdef SDL_GPU_DISABLE_TEXTURE_GETS
    GPU_PushErrorCode("GPU_CreateImageUsingTexture", GPU_ERROR_UNSUPPORTED_FUNCTION, "Renderer %s does not support this function", renderer->id.name);
    return NULL;
    #else

    GLint w, h;
    GLuint num_layers, bytes_per_pixel;
    GLint gl_format;
    GLint wrap_s, wrap_t;
    GLint min_filter;

    GPU_FormatEnum format;
    GPU_WrapEnum wrap_x, wrap_y;
    GPU_FilterEnum filter_mode;
	SDL_Color white = { 255, 255, 255, 255 };

	GPU_Image* result;
	GPU_IMAGE_DATA* data;

    flushAndBindTexture(renderer, handle);

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &gl_format);

    switch(gl_format)
    {
        case GL_LUMINANCE:
            format = GPU_FORMAT_LUMINANCE;
            num_layers = 1;
            bytes_per_pixel = 1;
            break;
        case GL_LUMINANCE_ALPHA:
            format = GPU_FORMAT_LUMINANCE_ALPHA;
            num_layers = 1;
            bytes_per_pixel = 2;
            break;
        case GL_RGB:
            format = GPU_FORMAT_RGB;
            num_layers = 1;
            bytes_per_pixel = 3;
            break;
        case GL_RGBA:
            format = GPU_FORMAT_RGBA;
            num_layers = 1;
            bytes_per_pixel = 4;
            break;
        case GL_ALPHA:
            format = GPU_FORMAT_ALPHA;
            num_layers = 1;
            bytes_per_pixel = 1;
            break;
        #ifndef SDL_GPU_USE_GLES
        case GL_RG:
            format = GPU_FORMAT_RG;
            num_layers = 1;
            bytes_per_pixel = 2;
            break;
        #endif
        default:
            GPU_PushErrorCode("GPU_CreateImageUsingTexture", GPU_ERROR_DATA_ERROR, "Unsupported GL image format (0x%x)", gl_format);
            return NULL;
    }

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);


	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &min_filter);
	// Ignore mag filter...  Maybe the wrong thing to do?

	// Let the user use one that we don't support and pretend that we're okay with that.
    switch(min_filter)
    {
        case GL_NEAREST:
            filter_mode = GPU_FILTER_NEAREST;
            break;
        case GL_LINEAR:
        case GL_LINEAR_MIPMAP_NEAREST:
            filter_mode = GPU_FILTER_LINEAR;
            break;
        case GL_LINEAR_MIPMAP_LINEAR:
            filter_mode = GPU_FILTER_LINEAR_MIPMAP;
            break;
        default:
            GPU_PushErrorCode("GPU_CreateImageUsingTexture", GPU_ERROR_USER_ERROR, "Unsupported value for GL_TEXTURE_MIN_FILTER (0x%x)", min_filter);
            filter_mode = GPU_FILTER_LINEAR;
            break;
    }


	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrap_s);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &wrap_t);

	// Let the user use one that we don't support and pretend that we're okay with that.
	switch(wrap_s)
	{
    case GL_CLAMP_TO_EDGE:
        wrap_x = GPU_WRAP_NONE;
        break;
    case GL_REPEAT:
        wrap_x = GPU_WRAP_REPEAT;
        break;
    case GL_MIRRORED_REPEAT:
        wrap_x = GPU_WRAP_MIRRORED;
        break;
    default:
        GPU_PushErrorCode("GPU_CreateImageUsingTexture", GPU_ERROR_USER_ERROR, "Unsupported value for GL_TEXTURE_WRAP_S (0x%x)", wrap_s);
        wrap_x = GPU_WRAP_NONE;
        break;
	}

	switch(wrap_t)
	{
    case GL_CLAMP_TO_EDGE:
        wrap_y = GPU_WRAP_NONE;
        break;
    case GL_REPEAT:
        wrap_y = GPU_WRAP_REPEAT;
        break;
    case GL_MIRRORED_REPEAT:
        wrap_y = GPU_WRAP_MIRRORED;
        break;
    default:
        GPU_PushErrorCode("GPU_CreateImageUsingTexture", GPU_ERROR_USER_ERROR, "Unsupported value for GL_TEXTURE_WRAP_T (0x%x)", wrap_t);
        wrap_y = GPU_WRAP_NONE;
        break;
	}

	// Finally create the image

    data = (GPU_IMAGE_DATA*)SDL_malloc(sizeof(GPU_IMAGE_DATA));
    data->refcount = 1;
    data->handle = handle;
    data->owns_handle = take_ownership;
    data->format = gl_format;


    result = (GPU_Image*)SDL_malloc(sizeof(GPU_Image));
    result->refcount = 1;
    result->target = NULL;
    result->renderer = renderer;
    result->format = format;
    result->num_layers = num_layers;
    result->bytes_per_pixel = bytes_per_pixel;
    result->has_mipmaps = 0;

    result->color = white;
    result->use_blending = 1;
    result->blend_mode = GPU_GetBlendModeFromPreset(GPU_BLEND_NORMAL);
    result->snap_mode = GPU_SNAP_POSITION_AND_DIMENSIONS;
    result->filter_mode = filter_mode;
    result->wrap_mode_x = wrap_x;
    result->wrap_mode_y = wrap_y;

    result->data = data;
    result->is_alias = 0;

    result->using_virtual_resolution = 0;
    result->w = w;
    result->h = h;

    result->base_w = w;
    result->base_h = h;
    result->texture_w = w;
    result->texture_h = h;

    return result;
    #endif
}

static GPU_Image* LoadImage(GPU_Renderer* renderer, const char* filename)
{
	GPU_Image* result;
    SDL_Surface* surface = GPU_LoadSurface(filename);
    if(surface == NULL)
    {
        GPU_PushErrorCode("GPU_LoadImage", GPU_ERROR_DATA_ERROR, "Failed to load image data.");
        return NULL;
    }

    result = renderer->impl->CopyImageFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    return result;
}


static GPU_Image* CreateAliasImage(GPU_Renderer* renderer, GPU_Image* image)
{
	GPU_Image* result;
	(void)renderer;

    if(image == NULL)
        return NULL;

    result = (GPU_Image*)SDL_malloc(sizeof(GPU_Image));
    // Copy the members
    *result = *image;

    // Alias info
    ((GPU_IMAGE_DATA*)image->data)->refcount++;
    result->refcount = 1;
    result->is_alias = 1;

    return result;
}


static Uint8 readTargetPixels(GPU_Renderer* renderer, GPU_Target* source, GLint format, GLubyte* pixels)
{
    if(source == NULL)
        return 0;

    if(isCurrentTarget(renderer, source))
        renderer->impl->FlushBlitBuffer(renderer);

    if(bindFramebuffer(renderer, source))
    {
        glReadPixels(0, 0, source->base_w, source->base_h, format, GL_UNSIGNED_BYTE, pixels);
        return 1;
    }
    return 0;
}

static Uint8 readImagePixels(GPU_Renderer* renderer, GPU_Image* source, GLint format, GLubyte* pixels)
{
#ifdef SDL_GPU_USE_GLES
	Uint8 created_target;
	Uint8 result;
#endif

    if(source == NULL)
        return 0;

    // No glGetTexImage() in OpenGLES
    #ifdef SDL_GPU_USE_GLES
    // Load up the target
    created_target = 0;
    if(source->target == NULL)
    {
        renderer->impl->LoadTarget(renderer, source);
        created_target = 1;
    }
    // Get the data
    // FIXME: This may use different dimensions than the OpenGL code... (base_w vs texture_w)
    // FIXME: I should force it to use the texture dims.
    result = readTargetPixels(renderer, source->target, format, pixels);
    // Free the target
    if(created_target)
        renderer->impl->FreeTarget(renderer, source->target);
    return result;
    #else
    // Bind the texture temporarily
    glBindTexture(GL_TEXTURE_2D, ((GPU_IMAGE_DATA*)source->data)->handle);
    // Get the data
    glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, pixels);
    // Rebind the last texture
    if(((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image != NULL)
        glBindTexture(GL_TEXTURE_2D, ((GPU_IMAGE_DATA*)(((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image)->data)->handle);
    return 1;
    #endif
}

static unsigned char* getRawTargetData(GPU_Renderer* renderer, GPU_Target* target)
{
	int bytes_per_pixel;
	unsigned char* data;
	int pitch;
	unsigned char* copy;
	int y;

    if(isCurrentTarget(renderer, target))
        renderer->impl->FlushBlitBuffer(renderer);

    bytes_per_pixel = 4;
    if(target->image != NULL)
        bytes_per_pixel = target->image->bytes_per_pixel;
    data = (unsigned char*)SDL_malloc(target->base_w * target->base_h * bytes_per_pixel);

    // This can take regions of pixels, so using base_w and base_h with an image target should be fine.
    if(!readTargetPixels(renderer, target, ((GPU_TARGET_DATA*)target->data)->format, data))
    {
        SDL_free(data);
        return NULL;
    }

    // Flip the data vertically (OpenGL framebuffer is read upside down)
    pitch = target->base_w * bytes_per_pixel;
    copy = (unsigned char*)SDL_malloc(pitch);

    for(y = 0; y < target->base_h/2; y++)
    {
        unsigned char* top = &data[target->base_w * y * bytes_per_pixel];
        unsigned char* bottom = &data[target->base_w * (target->base_h - y - 1) * bytes_per_pixel];
        memcpy(copy, top, pitch);
        memcpy(top, bottom, pitch);
        memcpy(bottom, copy, pitch);
    }
    SDL_free(copy);

    return data;
}

static unsigned char* getRawImageData(GPU_Renderer* renderer, GPU_Image* image)
{
	unsigned char* data;

    if(image->target != NULL && isCurrentTarget(renderer, image->target))
        renderer->impl->FlushBlitBuffer(renderer);

    data = (unsigned char*)SDL_malloc(image->texture_w * image->texture_h * image->bytes_per_pixel);

    // FIXME: Sometimes the texture is stored and read in RGBA even when I specify RGB.  getRawImageData() might need to return the stored format or Bpp.
    if(!readImagePixels(renderer, image, ((GPU_IMAGE_DATA*)image->data)->format, data))
    {
        SDL_free(data);
        return NULL;
    }

    return data;
}

static Uint8 SaveImage(GPU_Renderer* renderer, GPU_Image* image, const char* filename, GPU_FileFormatEnum format)
{
    Uint8 result;
    SDL_Surface* surface;

    if(image == NULL || filename == NULL ||
            image->texture_w < 1 || image->texture_h < 1 || image->bytes_per_pixel < 1 || image->bytes_per_pixel > 4)
    {
        return 0;
    }

    surface = renderer->impl->CopySurfaceFromImage(renderer, image);

    if(surface == NULL)
        return 0;
    
    result = GPU_SaveSurface(surface, filename, format);

    SDL_FreeSurface(surface);
    return result;
}

static SDL_Surface* CopySurfaceFromTarget(GPU_Renderer* renderer, GPU_Target* target)
{
    unsigned char* data;
    SDL_Surface* result;
	SDL_PixelFormat* format;

    if(target == NULL)
    {
        GPU_PushErrorCode("GPU_CopySurfaceFromTarget", GPU_ERROR_NULL_ARGUMENT, "target");
        return NULL;
    }
    if(target->base_w < 1 || target->base_h < 1)
    {
        GPU_PushErrorCode("GPU_CopySurfaceFromTarget", GPU_ERROR_DATA_ERROR, "Invalid target dimensions (%dx%d)", target->base_w, target->base_h);
        return NULL;
    }

    data = getRawTargetData(renderer, target);

    if(data == NULL)
    {
        GPU_PushErrorCode("GPU_CopySurfaceFromTarget", GPU_ERROR_BACKEND_ERROR, "Could not retrieve target data.");
        return NULL;
    }

    format = AllocFormat(((GPU_TARGET_DATA*)target->data)->format);

    result = SDL_CreateRGBSurface(SDL_SWSURFACE, target->base_w, target->base_h, format->BitsPerPixel, format->Rmask, format->Gmask, format->Bmask, format->Amask);

	if(result == NULL)
	{
	    GPU_PushErrorCode("GPU_CopySurfaceFromTarget", GPU_ERROR_DATA_ERROR, "Failed to create new %dx%d surface", target->base_w, target->base_h);
	    SDL_free(data);
		return NULL;
	}

    // Copy row-by-row in case the pitch doesn't match
    {
        int i;
        int source_pitch = target->base_w*format->BytesPerPixel;
        for(i = 0; i < target->base_h; ++i)
        {
            memcpy((Uint8*)result->pixels + i*result->pitch, data + source_pitch*i, source_pitch);
        }
    }

    SDL_free(data);

    FreeFormat(format);
    return result;
}

static SDL_Surface* CopySurfaceFromImage(GPU_Renderer* renderer, GPU_Image* image)
{
    unsigned char* data;
    SDL_Surface* result;
	SDL_PixelFormat* format;
	int w, h;

    if(image == NULL)
    {
        GPU_PushErrorCode("GPU_CopySurfaceFromImage", GPU_ERROR_NULL_ARGUMENT, "image");
        return NULL;
    }
    if(image->w < 1 || image->h < 1)
    {
        GPU_PushErrorCode("GPU_CopySurfaceFromImage", GPU_ERROR_DATA_ERROR, "Invalid image dimensions (%dx%d)", image->base_w, image->base_h);
        return NULL;
    }

    // FIXME: Virtual resolutions overwrite the NPOT dimensions when NPOT textures are not supported!
    if(image->using_virtual_resolution)
    {
        w = image->texture_w;
        h = image->texture_h;
    }
    else
    {
        w = image->w;
        h = image->h;
    }
    data = getRawImageData(renderer, image);

    if(data == NULL)
    {
        GPU_PushErrorCode("GPU_CopySurfaceFromImage", GPU_ERROR_BACKEND_ERROR, "Could not retrieve target data.");
        return NULL;
    }

    format = AllocFormat(((GPU_IMAGE_DATA*)image->data)->format);

    result = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, format->BitsPerPixel, format->Rmask, format->Gmask, format->Bmask, format->Amask);

	if(result == NULL)
	{
	    GPU_PushErrorCode("GPU_CopySurfaceFromImage", GPU_ERROR_DATA_ERROR, "Failed to create new %dx%d surface", w, h);
	    SDL_free(data);
		return NULL;
	}

    // Copy row-by-row in case the pitch doesn't match
    {
        int i;
        int source_pitch = image->texture_w*format->BytesPerPixel;  // Use the actual texture width to pull from the data
        for(i = 0; i < h; ++i)
        {
            memcpy((Uint8*)result->pixels + i*result->pitch, data + source_pitch*i, source_pitch);
        }
    }

    SDL_free(data);

    FreeFormat(format);
    return result;
}















// Returns 0 if a direct conversion (asking OpenGL to do it) is safe.  Returns 1 if a copy is needed.  Returns -1 on error.
// The surfaceFormatResult is used to specify what direct conversion format the surface pixels are in (source format).
#ifdef SDL_GPU_USE_GLES
// OpenGLES does not do direct conversion.  Internal format (glFormat) and original format (surfaceFormatResult) must be the same.
static int compareFormats(GPU_Renderer* renderer, GLenum glFormat, SDL_Surface* surface, GLenum* surfaceFormatResult)
{
    SDL_PixelFormat* format = surface->format;
    switch(glFormat)
    {
        // 3-channel formats
    case GL_RGB:
        if(format->BytesPerPixel != 3)
            return 1;

        if(format->Rmask == 0x0000FF && format->Gmask == 0x00FF00 && format->Bmask ==  0xFF0000)
        {
            if(surfaceFormatResult != NULL)
                *surfaceFormatResult = GL_RGB;
            return 0;
        }
#ifdef GL_BGR
        if(format->Rmask == 0xFF0000 && format->Gmask == 0x00FF00 && format->Bmask == 0x0000FF)
        {
            if(renderer->enabled_features & GPU_FEATURE_GL_BGR)
            {
                if(surfaceFormatResult != NULL)
                    *surfaceFormatResult = GL_BGR;
				return 0;
            }
        }
#endif
        return 1;
        // 4-channel formats
    case GL_RGBA:
        if(format->BytesPerPixel != 4)
            return 1;

        if (format->Rmask == 0x000000FF && format->Gmask == 0x0000FF00 && format->Bmask ==  0x00FF0000)
        {
            if(surfaceFormatResult != NULL)
                *surfaceFormatResult = GL_RGBA;
            return 0;
        }
#ifdef GL_BGRA
        if (format->Rmask == 0x00FF0000 && format->Gmask == 0x0000FF00 && format->Bmask == 0x000000FF)
        {
            if(renderer->enabled_features & GPU_FEATURE_GL_BGRA)
            {
				if(surfaceFormatResult != NULL)
					*surfaceFormatResult = GL_BGRA;
				return 0;
			}
        }
#endif
#ifdef GL_ABGR
        if (format->Rmask == 0xFF000000 && format->Gmask == 0x00FF0000 && format->Bmask == 0x0000FF00)
        {
            if(renderer->enabled_features & GPU_FEATURE_GL_ABGR)
            {
				if(surfaceFormatResult != NULL)
					*surfaceFormatResult = GL_ABGR;
				return 0;
			}
        }
#endif
        return 1;
    default:
        GPU_PushErrorCode("GPU_CompareFormats", GPU_ERROR_DATA_ERROR, "Invalid texture format (0x%x)", glFormat);
        return -1;
    }
}
#else
//GL_RGB/GL_RGBA and Surface format
static int compareFormats(GPU_Renderer* renderer, GLenum glFormat, SDL_Surface* surface, GLenum* surfaceFormatResult)
{
    SDL_PixelFormat* format = surface->format;
    switch(glFormat)
    {
        // 3-channel formats
    case GL_RGB:
        if(format->BytesPerPixel != 3)
            return 1;

        // Looks like RGB?  Easy!
        if(format->Rmask == 0x0000FF && format->Gmask == 0x00FF00 && format->Bmask == 0xFF0000)
        {
            if(surfaceFormatResult != NULL)
                *surfaceFormatResult = GL_RGB;
            return 0;
        }
        // Looks like BGR?
        if(format->Rmask == 0xFF0000 && format->Gmask == 0x00FF00 && format->Bmask == 0x0000FF)
        {
#ifdef GL_BGR
            if(renderer->enabled_features & GPU_FEATURE_GL_BGR)
            {
                if(surfaceFormatResult != NULL)
                    *surfaceFormatResult = GL_BGR;
                return 0;
            }
#endif
        }
        return 1;

        // 4-channel formats
    case GL_RGBA:

        if(format->BytesPerPixel != 4)
            return 1;

        // Looks like RGBA?  Easy!
        if(format->Rmask == 0x000000FF && format->Gmask == 0x0000FF00 && format->Bmask == 0x00FF0000)
        {
            if(surfaceFormatResult != NULL)
                *surfaceFormatResult = GL_RGBA;
            return 0;
        }
        // Looks like ABGR?
        if(format->Rmask == 0xFF000000 && format->Gmask == 0x00FF0000 && format->Bmask == 0x0000FF00)
        {
#ifdef GL_ABGR
            if(renderer->enabled_features & GPU_FEATURE_GL_ABGR)
            {
                if(surfaceFormatResult != NULL)
                    *surfaceFormatResult = GL_ABGR;
                return 0;
            }
#endif
        }
        // Looks like BGRA?
        else if(format->Rmask == 0x00FF0000 && format->Gmask == 0x0000FF00 && format->Bmask == 0x000000FF)
        {
#ifdef GL_BGRA
            if(renderer->enabled_features & GPU_FEATURE_GL_BGRA)
            {
                //ARGB, for OpenGL BGRA
                if(surfaceFormatResult != NULL)
                    *surfaceFormatResult = GL_BGRA;
                return 0;
            }
#endif
        }
        return 1;
    default:
        GPU_PushErrorCode("GPU_CompareFormats", GPU_ERROR_DATA_ERROR, "Invalid texture format (0x%x)", glFormat);
        return -1;
    }
}
#endif


// Adapted from SDL_AllocFormat()
static SDL_PixelFormat* AllocFormat(GLenum glFormat)
{
    // Yes, I need to do the whole thing myself... :(
    int channels;
    Uint32 Rmask, Gmask, Bmask, Amask = 0, mask;
	SDL_PixelFormat* result;

    switch(glFormat)
    {
    case GL_RGB:
        channels = 3;
        Rmask = 0x0000FF;
        Gmask = 0x00FF00;
        Bmask = 0xFF0000;
        break;
#ifdef GL_BGR
    case GL_BGR:
        channels = 3;
        Rmask = 0xFF0000;
        Gmask = 0x00FF00;
        Bmask = 0x0000FF;
        break;
#endif
    case GL_RGBA:
        channels = 4;
        Rmask = 0x000000FF;
        Gmask = 0x0000FF00;
        Bmask = 0x00FF0000;
        Amask = 0xFF000000;
        break;
#ifdef GL_BGRA
    case GL_BGRA:
        channels = 4;
        Rmask = 0x00FF0000;
        Gmask = 0x0000FF00;
        Bmask = 0x000000FF;
        Amask = 0xFF000000;
        break;
#endif
#ifdef GL_ABGR
    case GL_ABGR:
        channels = 4;
        Rmask = 0xFF000000;
        Gmask = 0x00FF0000;
        Bmask = 0x0000FF00;
        Amask = 0x000000FF;
        break;
#endif
    default:
        return NULL;
    }

    //GPU_LogError("AllocFormat(): %d, Masks: %X %X %X %X\n", glFormat, Rmask, Gmask, Bmask, Amask);

    result = (SDL_PixelFormat*)SDL_malloc(sizeof(SDL_PixelFormat));
    memset(result, 0, sizeof(SDL_PixelFormat));

    result->BitsPerPixel = 8*channels;
    result->BytesPerPixel = channels;

    result->Rmask = Rmask;
    result->Rshift = 0;
    result->Rloss = 8;
    if (Rmask) {
        for (mask = Rmask; !(mask & 0x01); mask >>= 1)
            ++result->Rshift;
        for (; (mask & 0x01); mask >>= 1)
            --result->Rloss;
    }

    result->Gmask = Gmask;
    result->Gshift = 0;
    result->Gloss = 8;
    if (Gmask) {
        for (mask = Gmask; !(mask & 0x01); mask >>= 1)
            ++result->Gshift;
        for (; (mask & 0x01); mask >>= 1)
            --result->Gloss;
    }

    result->Bmask = Bmask;
    result->Bshift = 0;
    result->Bloss = 8;
    if (Bmask) {
        for (mask = Bmask; !(mask & 0x01); mask >>= 1)
            ++result->Bshift;
        for (; (mask & 0x01); mask >>= 1)
            --result->Bloss;
    }

    result->Amask = Amask;
    result->Ashift = 0;
    result->Aloss = 8;
    if (Amask) {
        for (mask = Amask; !(mask & 0x01); mask >>= 1)
            ++result->Ashift;
        for (; (mask & 0x01); mask >>= 1)
            --result->Aloss;
    }

    return result;
}

static void FreeFormat(SDL_PixelFormat* format)
{
    SDL_free(format);
}

#ifdef SDL_GPU_USE_GLES
// Based on SDL_UpdateTexture()
static SDL_Surface* pack_surface_if_needed(SDL_Surface* surface)
{
	SDL_Surface* result;
	int width, height;
	int packed_pitch;
	int pitch;

    // NULL or already packed?
	if(surface == NULL || surface->pitch == surface->w * surface->format->BytesPerPixel)
        return surface;

	width = surface->w;
	height = surface->h;
    packed_pitch = width * surface->format->BytesPerPixel;
    pitch = surface->pitch;

    // Bail out if we're supposed to update an empty rectangle
    if(width <= 0 || height <= 0)
        return NULL;

    // Reformat the data into a tightly packed array
    result = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, surface->format->BytesPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);

    if(result == NULL)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_DATA_ERROR, "Failed to create new %dx%d surface", width, height);
        return NULL;
    }

    // Force a new pitch in spite of SDL's alignment calculations
    result->pitch = packed_pitch;

    // Copy to the new surface
    {
        int i;
        for(i = 0; i < width; ++i)
        {
            memcpy((Uint8*)result->pixels + i*packed_pitch, surface->pixels + surface->pitch*i, packed_pitch);
        }
    }

    return result;
}
#endif

// Returns NULL on failure.  Returns the original surface if no copy is needed.  Returns a new surface converted to the right format otherwise.
static SDL_Surface* copySurfaceIfNeeded(GPU_Renderer* renderer, GLenum glFormat, SDL_Surface* surface, GLenum* surfaceFormatResult)
{
    #ifdef SDL_GPU_USE_GLES
    SDL_Surface* original = surface;
    #endif

    // If format doesn't match, we need to do a copy
    int format_compare = compareFormats(renderer, glFormat, surface, surfaceFormatResult);

    // There's a problem, logged in compareFormats()
    if(format_compare < 0)
        return NULL;


    // Copy it to a different format
    if(format_compare > 0)
    {
        // Convert to the right format
        SDL_PixelFormat* dst_fmt = AllocFormat(glFormat);
        surface = SDL_ConvertSurface(surface, dst_fmt, 0);
        FreeFormat(dst_fmt);
        if(surfaceFormatResult != NULL && surface != NULL)
            *surfaceFormatResult = glFormat;
    }

    #ifdef SDL_GPU_USE_GLES
    // GLES needs a tightly-packed pixel array
    {
        SDL_Surface* intermediate_surface = surface;
        surface = pack_surface_if_needed(intermediate_surface);

        // Delete the intermediate surface since it won't be used.
        if(intermediate_surface != surface && intermediate_surface != original)
            SDL_FreeSurface(intermediate_surface);
    }
    #endif

    // No copy needed
    return surface;
}

static GPU_Image* CopyImage(GPU_Renderer* renderer, GPU_Image* image)
{
    GPU_Image* result = NULL;

    if(image == NULL)
        return NULL;

    switch(image->format)
    {
        case GPU_FORMAT_RGB:
        case GPU_FORMAT_RGBA:
        // Copy via framebuffer blitting (fast)
		{
			GPU_Target* target;

            result = renderer->impl->CreateImage(renderer, image->texture_w, image->texture_h, image->format);
            if(result == NULL)
            {
                GPU_PushErrorCode("GPU_CopyImage", GPU_ERROR_BACKEND_ERROR, "Failed to create new image.");
                return NULL;
            }

            target = GPU_LoadTarget(result);
            if(target == NULL)
            {
                GPU_FreeImage(result);
                GPU_PushErrorCode("GPU_CopyImage", GPU_ERROR_BACKEND_ERROR, "Failed to load target.");
                return NULL;
            }

            // For some reason, I wasn't able to get glCopyTexImage2D() or glCopyTexSubImage2D() working without getting GL_INVALID_ENUM (0x500).
            // It seemed to only work for the default framebuffer...

			{
				// Clear the color, blending, and filter mode
				SDL_Color color = image->color;
				Uint8 use_blending = image->use_blending;
				GPU_FilterEnum filter_mode = image->filter_mode;
				Uint8 use_virtual = image->using_virtual_resolution;
				Uint16 w = 0, h = 0;
				GPU_UnsetColor(image);
				GPU_SetBlending(image, 0);
				GPU_SetImageFilter(image, GPU_FILTER_NEAREST);
				if(use_virtual)
                {
                    w = image->w;
                    h = image->h;
                    GPU_UnsetImageVirtualResolution(image);
                }

				renderer->impl->Blit(renderer, image, NULL, target, image->w / 2, image->h / 2);

				// Restore the saved settings
				GPU_SetColor(image, color);
				GPU_SetBlending(image, use_blending);
				GPU_SetImageFilter(image, filter_mode);
				if(use_virtual)
                {
                    GPU_SetImageVirtualResolution(image, w, h);
                }
			}

            // Don't free the target yet (a waste of perf), but let it be freed next time...
            target->refcount--;
        }
        break;
        case GPU_FORMAT_LUMINANCE:
        case GPU_FORMAT_LUMINANCE_ALPHA:
        case GPU_FORMAT_ALPHA:
        case GPU_FORMAT_RG:
        // Copy via texture download and upload (slow)
		{
			GLenum internal_format;
			int w;
			int h;
            unsigned char* texture_data = getRawImageData(renderer, image);
            if(texture_data == NULL)
            {
                GPU_PushErrorCode("GPU_CopyImage", GPU_ERROR_BACKEND_ERROR, "Failed to get raw texture data.");
                return NULL;
            }

            result = CreateUninitializedImage(renderer, image->texture_w, image->texture_h, image->format);
            if(result == NULL)
            {
                SDL_free(texture_data);
                GPU_PushErrorCode("GPU_CopyImage", GPU_ERROR_BACKEND_ERROR, "Failed to create new image.");
                return NULL;
            }

            changeTexturing(renderer, 1);
            bindTexture(renderer, result);

            internal_format = ((GPU_IMAGE_DATA*)(result->data))->format;
            w = result->w;
            h = result->h;
            if(!(renderer->enabled_features & GPU_FEATURE_NON_POWER_OF_TWO))
            {
                if(!isPowerOfTwo(w))
                    w = getNearestPowerOf2(w);
                if(!isPowerOfTwo(h))
                    h = getNearestPowerOf2(h);
            }

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            #ifdef SDL_GPU_USE_OPENGL
            glPixelStorei(GL_UNPACK_ROW_LENGTH, w);
            #endif

            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0,
                         internal_format, GL_UNSIGNED_BYTE, texture_data);
            // Tell SDL_gpu what we got.
            result->texture_w = w;
            result->texture_h = h;

            // Restore GL defaults
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            #ifdef SDL_GPU_USE_OPENGL
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            #endif

            SDL_free(texture_data);
        }
        break;
        default:
            GPU_PushErrorCode("GPU_CopyImage", GPU_ERROR_BACKEND_ERROR, "Could not copy the given image format.");
        break;
    }

    if(result != NULL)
    {
        // Copy the image settings
        GPU_SetColor(result, image->color);
        GPU_SetBlending(result, image->use_blending);
        result->blend_mode = image->blend_mode;
        GPU_SetImageFilter(result, image->filter_mode);
        GPU_SetSnapMode(result, image->snap_mode);
        GPU_SetWrapMode(result, image->wrap_mode_x, image->wrap_mode_y);
        if(image->has_mipmaps)
            GPU_GenerateMipmaps(result);
        if(image->using_virtual_resolution)
            GPU_SetImageVirtualResolution(result, image->w, image->h);
    }

    return result;
}



static void UpdateImage(GPU_Renderer* renderer, GPU_Image* image, const GPU_Rect* image_rect, SDL_Surface* surface, const GPU_Rect* surface_rect)
{
	GPU_IMAGE_DATA* data;
	GLenum original_format;

	SDL_Surface* newSurface;
	GPU_Rect updateRect;
	GPU_Rect sourceRect;
	int alignment;
	Uint8* pixels;

    if(image == NULL || surface == NULL)
        return;

    data = (GPU_IMAGE_DATA*)image->data;
    original_format = data->format;

    newSurface = copySurfaceIfNeeded(renderer, data->format, surface, &original_format);
    if(newSurface == NULL)
    {
        GPU_PushErrorCode("GPU_UpdateImage", GPU_ERROR_BACKEND_ERROR, "Failed to convert surface to proper pixel format.");
        return;
    }

    if(image_rect != NULL)
    {
        updateRect = *image_rect;
        if(updateRect.x < 0)
        {
            updateRect.w += updateRect.x;
            updateRect.x = 0;
        }
        if(updateRect.y < 0)
        {
            updateRect.h += updateRect.y;
            updateRect.y = 0;
        }
        if(updateRect.x + updateRect.w > image->base_w)
            updateRect.w += image->base_w - (updateRect.x + updateRect.w);
        if(updateRect.y + updateRect.h > image->base_h)
            updateRect.h += image->base_h - (updateRect.y + updateRect.h);

        if(updateRect.w <= 0)
            updateRect.w = 0;
        if(updateRect.h <= 0)
            updateRect.h = 0;
    }
    else
    {
        updateRect.x = 0;
        updateRect.y = 0;
        updateRect.w = image->base_w;
        updateRect.h = image->base_h;
        if(updateRect.w < 0.0f || updateRect.h < 0.0f)
        {
            GPU_PushErrorCode("GPU_UpdateImage", GPU_ERROR_USER_ERROR, "Given negative image rectangle.");
            return;
        }
    }

    if(surface_rect != NULL)
    {
        sourceRect = *surface_rect;
        if(sourceRect.x < 0)
        {
            sourceRect.w += sourceRect.x;
            sourceRect.x = 0;
        }
        if(sourceRect.y < 0)
        {
            sourceRect.h += sourceRect.y;
            sourceRect.y = 0;
        }
        if(sourceRect.x + sourceRect.w > newSurface->w)
            sourceRect.w += newSurface->w - (sourceRect.x + sourceRect.w);
        if(sourceRect.y + sourceRect.h > newSurface->h)
            sourceRect.h += newSurface->h - (sourceRect.y + sourceRect.h);

        if(sourceRect.w <= 0)
            sourceRect.w = 0;
        if(sourceRect.h <= 0)
            sourceRect.h = 0;
    }
    else
    {
        sourceRect.x = 0;
        sourceRect.y = 0;
        sourceRect.w = newSurface->w;
        sourceRect.h = newSurface->h;
    }


    changeTexturing(renderer, 1);
    if(image->target != NULL && isCurrentTarget(renderer, image->target))
        renderer->impl->FlushBlitBuffer(renderer);
    bindTexture(renderer, image);
    alignment = 8;
    while(newSurface->pitch % alignment)
        alignment >>= 1;
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    #ifdef SDL_GPU_USE_OPENGL
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (newSurface->pitch / newSurface->format->BytesPerPixel));
    #endif

    // Use the smaller of the image and surface rect dimensions
    if(sourceRect.w < updateRect.w)
        updateRect.w = sourceRect.w;
    if(sourceRect.h < updateRect.h)
        updateRect.h = sourceRect.h;

    pixels = (Uint8*)newSurface->pixels;
    // Shift the pixels pointer to the proper source position
    pixels += (int)(newSurface->pitch * sourceRect.y + (newSurface->format->BytesPerPixel)*sourceRect.x);

    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    updateRect.x, updateRect.y, updateRect.w, updateRect.h,
                    original_format, GL_UNSIGNED_BYTE, pixels);

    // Delete temporary surface
    if(surface != newSurface)
        SDL_FreeSurface(newSurface);

    // Restore GL defaults
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    #ifdef SDL_GPU_USE_OPENGL
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    #endif
}


static void UpdateImageBytes(GPU_Renderer* renderer, GPU_Image* image, const GPU_Rect* image_rect, const unsigned char* bytes, int bytes_per_row)
{
	GPU_IMAGE_DATA* data;
	GLenum original_format;

	GPU_Rect updateRect;
	int alignment;

    if(image == NULL || bytes == NULL)
        return;

    data = (GPU_IMAGE_DATA*)image->data;
    original_format = data->format;

    if(image_rect != NULL)
    {
        updateRect = *image_rect;
        if(updateRect.x < 0)
        {
            updateRect.w += updateRect.x;
            updateRect.x = 0;
        }
        if(updateRect.y < 0)
        {
            updateRect.h += updateRect.y;
            updateRect.y = 0;
        }
        if(updateRect.x + updateRect.w > image->base_w)
            updateRect.w += image->base_w - (updateRect.x + updateRect.w);
        if(updateRect.y + updateRect.h > image->base_h)
            updateRect.h += image->base_h - (updateRect.y + updateRect.h);

        if(updateRect.w <= 0)
            updateRect.w = 0;
        if(updateRect.h <= 0)
            updateRect.h = 0;
    }
    else
    {
        updateRect.x = 0;
        updateRect.y = 0;
        updateRect.w = image->base_w;
        updateRect.h = image->base_h;
        if(updateRect.w < 0.0f || updateRect.h < 0.0f)
        {
            GPU_PushErrorCode("GPU_UpdateImage", GPU_ERROR_USER_ERROR, "Given negative image rectangle.");
            return;
        }
    }


    changeTexturing(renderer, 1);
    if(image->target != NULL && isCurrentTarget(renderer, image->target))
        renderer->impl->FlushBlitBuffer(renderer);
    bindTexture(renderer, image);
    alignment = 8;
    while(bytes_per_row % alignment)
        alignment >>= 1;
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    #ifdef SDL_GPU_USE_OPENGL
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (bytes_per_row / image->bytes_per_pixel));
    #endif

    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    updateRect.x, updateRect.y, updateRect.w, updateRect.h,
                    original_format, GL_UNSIGNED_BYTE, bytes);

    // Restore GL defaults
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    #ifdef SDL_GPU_USE_OPENGL
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    #endif
}



static Uint8 ReplaceImage(GPU_Renderer* renderer, GPU_Image* image, SDL_Surface* surface, const GPU_Rect* surface_rect)
{
	GPU_IMAGE_DATA* data;
	GPU_Rect sourceRect;
	SDL_Surface* newSurface;
	GLenum internal_format;
	Uint8* pixels;
	int w, h;
	int alignment;

    if(image == NULL)
    {
        GPU_PushErrorCode("GPU_ReplaceImage", GPU_ERROR_NULL_ARGUMENT, "image");
        return 0;
    }

    if(surface == NULL)
    {
        GPU_PushErrorCode("GPU_ReplaceImage", GPU_ERROR_NULL_ARGUMENT, "surface");
        return 0;
    }

    data = (GPU_IMAGE_DATA*)image->data;
    internal_format = data->format;

    newSurface = copySurfaceIfNeeded(renderer, internal_format, surface, &internal_format);
    if(newSurface == NULL)
    {
        GPU_PushErrorCode("GPU_ReplaceImage", GPU_ERROR_BACKEND_ERROR, "Failed to convert surface to proper pixel format.");
        return 0;
    }

    // Free the attached framebuffer
    if((renderer->enabled_features & GPU_FEATURE_RENDER_TARGETS) && image->target != NULL)
    {
        GPU_TARGET_DATA* tdata = (GPU_TARGET_DATA*)image->target->data;
        if(renderer->current_context_target != NULL)
            flushAndClearBlitBufferIfCurrentFramebuffer(renderer, image->target);
        if(tdata->handle != 0)
            glDeleteFramebuffers(1, &tdata->handle);
        tdata->handle = 0;
    }

    // Free the old texture
    if(data->owns_handle)
        glDeleteTextures( 1, &data->handle);
    data->handle = 0;

    // Get the area of the surface we'll use
    if(surface_rect == NULL)
    {
        sourceRect.x = 0;
        sourceRect.y = 0;
        sourceRect.w = surface->w;
        sourceRect.h = surface->h;
    }
    else
        sourceRect = *surface_rect;

    // Clip the source rect to the surface
    if(sourceRect.x < 0)
    {
        sourceRect.w += sourceRect.x;
        sourceRect.x = 0;
    }
    if(sourceRect.y < 0)
    {
        sourceRect.h += sourceRect.y;
        sourceRect.y = 0;
    }
    if(sourceRect.x >= surface->w)
        sourceRect.x = surface->w - 1;
    if(sourceRect.y >= surface->h)
        sourceRect.y = surface->h - 1;

    if(sourceRect.x + sourceRect.w > surface->w)
        sourceRect.w = surface->w - sourceRect.x;
    if(sourceRect.y + sourceRect.h > surface->h)
        sourceRect.h = surface->h - sourceRect.y;

    if(sourceRect.w <= 0 || sourceRect.h <= 0)
    {
        GPU_PushErrorCode("GPU_ReplaceImage", GPU_ERROR_DATA_ERROR, "Clipped source rect has zero size.");
        return 0;
    }

    // Allocate new texture
    data->handle = CreateUninitializedTexture(renderer);
    data->owns_handle = 1;
    if(data->handle == 0)
    {
        GPU_PushErrorCode("GPU_ReplaceImage", GPU_ERROR_BACKEND_ERROR, "Failed to create a new texture handle.");
        return 0;
    }

    // Update image members
    w = sourceRect.w;
    h = sourceRect.h;

    if(!image->using_virtual_resolution)
    {
        image->w = w;
        image->h = h;
    }
    image->base_w = w;
    image->base_h = h;

    if(!(renderer->enabled_features & GPU_FEATURE_NON_POWER_OF_TWO))
    {
        if(!isPowerOfTwo(w))
            w = getNearestPowerOf2(w);
        if(!isPowerOfTwo(h))
            h = getNearestPowerOf2(h);
    }
    image->texture_w = w;
    image->texture_h = h;

    image->has_mipmaps = 0;


    // Upload surface pixel data
    alignment = 8;
    while(newSurface->pitch % alignment)
        alignment >>= 1;
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    #ifdef SDL_GPU_USE_OPENGL
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (newSurface->pitch / newSurface->format->BytesPerPixel));
    #endif

    pixels = (Uint8*)newSurface->pixels;
    // Shift the pixels pointer to the proper source position
    pixels += (int)(newSurface->pitch * sourceRect.y + (newSurface->format->BytesPerPixel)*sourceRect.x);

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0,
                 internal_format, GL_UNSIGNED_BYTE, pixels);

    // Delete temporary surface
    if(surface != newSurface)
        SDL_FreeSurface(newSurface);

    // Restore GL defaults
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    #ifdef SDL_GPU_USE_OPENGL
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    #endif



    // Update target members
    if((renderer->enabled_features & GPU_FEATURE_RENDER_TARGETS) && image->target != NULL)
    {
        GLenum status;
        GPU_Target* target = image->target;
        GPU_TARGET_DATA* tdata = (GPU_TARGET_DATA*)target->data;

        // Create framebuffer object
        glGenFramebuffers(1, &tdata->handle);
        if(tdata->handle == 0)
        {
            GPU_PushErrorCode("GPU_ReplaceImage", GPU_ERROR_BACKEND_ERROR, "Failed to create new framebuffer target.");
            return 0;
        }

        flushAndBindFramebuffer(renderer, tdata->handle);

        // Attach the texture to it
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data->handle, 0);

        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE)
        {
            GPU_PushErrorCode("GPU_ReplaceImage", GPU_ERROR_BACKEND_ERROR, "Failed to recreate framebuffer target.");
            return 0;
        }

        if(!target->using_virtual_resolution)
        {
            target->w = image->base_w;
            target->h = image->base_h;
        }
        target->base_w = image->texture_w;
        target->base_h = image->texture_h;

        // Reset viewport?
        target->viewport = GPU_MakeRect(0, 0, target->w, target->h);
    }

    return 1;
}


static_inline Uint32 getPixel(SDL_Surface *Surface, int x, int y)
{
    Uint8* bits;
    Uint32 bpp;

    if(x < 0 || x >= Surface->w)
        return 0;  // Best I could do for errors

    bpp = Surface->format->BytesPerPixel;
    bits = ((Uint8*)Surface->pixels) + y*Surface->pitch + x*bpp;

    switch (bpp)
    {
    case 1:
        return *((Uint8*)Surface->pixels + y * Surface->pitch + x);
        break;
    case 2:
        return *((Uint16*)Surface->pixels + y * Surface->pitch/2 + x);
        break;
    case 3:
        // Endian-correct, but slower
    {
        Uint8 r, g, b;
        r = *((bits)+Surface->format->Rshift/8);
        g = *((bits)+Surface->format->Gshift/8);
        b = *((bits)+Surface->format->Bshift/8);
        return SDL_MapRGB(Surface->format, r, g, b);
    }
    break;
    case 4:
        return *((Uint32*)Surface->pixels + y * Surface->pitch/4 + x);
        break;
    }

    return 0;  // FIXME: Handle errors better
}

static GPU_Image* CopyImageFromSurface(GPU_Renderer* renderer, SDL_Surface* surface)
{
    GPU_FormatEnum format;
	GPU_Image* image;

    if(surface == NULL)
    {
        GPU_PushErrorCode("GPU_CopyImageFromSurface", GPU_ERROR_NULL_ARGUMENT, "surface");
        return NULL;
    }

    if(surface->w == 0 || surface->h == 0)
    {
        GPU_PushErrorCode("GPU_CopyImageFromSurface", GPU_ERROR_DATA_ERROR, "Surface has a zero dimension.");
        return NULL;
    }

    // See what the best image format is.
    if(surface->format->Amask == 0)
    {
        if(has_colorkey(surface))
            format = GPU_FORMAT_RGBA;
        else
            format = GPU_FORMAT_RGB;
    }
    else
        format = GPU_FORMAT_RGBA;

    image = renderer->impl->CreateImage(renderer, surface->w, surface->h, format);
    if(image == NULL)
        return NULL;

    renderer->impl->UpdateImage(renderer, image, NULL, surface, NULL);

    return image;
}


static GPU_Image* CopyImageFromTarget(GPU_Renderer* renderer, GPU_Target* target)
{
	SDL_Surface* surface;
	GPU_Image* image;

    if(target == NULL)
        return NULL;

    surface = renderer->impl->CopySurfaceFromTarget(renderer, target);
    image = renderer->impl->CopyImageFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    return image;
}


static void FreeImage(GPU_Renderer* renderer, GPU_Image* image)
{
	GPU_IMAGE_DATA* data;

    if(image == NULL)
        return;

    if(image->refcount > 1)
    {
        image->refcount--;
        return;
    }

    // Delete the attached target first
    if(image->target != NULL)
    {
        GPU_Target* target = image->target;
        image->target = NULL;
        renderer->impl->FreeTarget(renderer, target);
    }

    flushAndClearBlitBufferIfCurrentTexture(renderer, image);

    // Does the renderer data need to be freed too?
    data = (GPU_IMAGE_DATA*)image->data;
    if(data->refcount > 1)
    {
        data->refcount--;
    }
    else
    {
        if(data->owns_handle)
            glDeleteTextures( 1, &data->handle);
        SDL_free(data);
    }

    SDL_free(image);
}



static GPU_Target* LoadTarget(GPU_Renderer* renderer, GPU_Image* image)
{
    GLuint handle;
	GLenum status;
	GPU_Target* result;
	GPU_TARGET_DATA* data;

    if(image == NULL)
        return NULL;

    if(image->target != NULL)
    {
        image->target->refcount++;
        ((GPU_TARGET_DATA*)image->target->data)->refcount++;
        return image->target;
    }

    if(!(renderer->enabled_features & GPU_FEATURE_RENDER_TARGETS))
        return NULL;

    // Create framebuffer object
    glGenFramebuffers(1, &handle);
    flushAndBindFramebuffer(renderer, handle);

    // Attach the texture to it
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ((GPU_IMAGE_DATA*)image->data)->handle, 0);

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        return NULL;

    result = (GPU_Target*)SDL_malloc(sizeof(GPU_Target));
    memset(result, 0, sizeof(GPU_Target));
    result->refcount = 1;
    data = (GPU_TARGET_DATA*)SDL_malloc(sizeof(GPU_TARGET_DATA));
    data->refcount = 1;
    result->data = data;
    data->handle = handle;
    data->format = ((GPU_IMAGE_DATA*)image->data)->format;

    result->renderer = renderer;
    result->context = NULL;
    result->image = image;
    result->w = image->w;
    result->h = image->h;
    result->base_w = image->texture_w;
    result->base_h = image->texture_h;
    result->using_virtual_resolution = image->using_virtual_resolution;

    result->viewport = GPU_MakeRect(0, 0, result->w, result->h);

    result->camera = GPU_GetDefaultCamera();

    result->use_clip_rect = 0;
    result->clip_rect.x = 0;
    result->clip_rect.y = 0;
    result->clip_rect.w = result->w;
    result->clip_rect.h = result->h;
    result->use_color = 0;

    image->target = result;
    return result;
}



static void FreeTarget(GPU_Renderer* renderer, GPU_Target* target)
{
	GPU_TARGET_DATA* data;

    if(target == NULL)
        return;

    if(target->refcount > 1)
    {
        target->refcount--;
        return;
    }

    if(target->context != NULL && target->context->failed)
    {
        GPU_CONTEXT_DATA* cdata = (GPU_CONTEXT_DATA*)target->context->data;

        if(target == renderer->current_context_target)
            renderer->current_context_target = NULL;

        SDL_free(cdata->blit_buffer);
        SDL_free(cdata->index_buffer);

        #ifdef SDL_GPU_USE_SDL2
        if(target->context->context != 0)
            SDL_GL_DeleteContext(target->context->context);
        #endif

        // Remove all of the window mappings that refer to this target
        GPU_RemoveWindowMappingByTarget(target);

        SDL_free(target->context->data);
        SDL_free(target->context);

        // Does the renderer data need to be freed too?
        data = ((GPU_TARGET_DATA*)target->data);

        SDL_free(data);
        SDL_free(target);

        return;
    }

    if(target == renderer->current_context_target)
    {
        renderer->impl->FlushBlitBuffer(renderer);
        renderer->current_context_target = NULL;
    }

    if(!target->is_alias && target->image != NULL)
        target->image->target = NULL;  // Remove reference to this object


    // Does the renderer data need to be freed too?
    data = ((GPU_TARGET_DATA*)target->data);
    if(data->refcount > 1)
    {
        data->refcount--;
        SDL_free(target);
        return;
    }

    if(renderer->enabled_features & GPU_FEATURE_RENDER_TARGETS)
    {
        if(renderer->current_context_target != NULL)
            flushAndClearBlitBufferIfCurrentFramebuffer(renderer, target);
        if(data->handle != 0)
            glDeleteFramebuffers(1, &data->handle);
    }

    if(target->context != NULL)
    {
        GPU_CONTEXT_DATA* cdata = (GPU_CONTEXT_DATA*)target->context->data;

        SDL_free(cdata->blit_buffer);
        SDL_free(cdata->index_buffer);

        #ifdef SDL_GPU_USE_BUFFER_PIPELINE
        glDeleteBuffers(2, cdata->blit_VBO);
        glDeleteBuffers(1, &cdata->blit_IBO);
        glDeleteBuffers(16, cdata->attribute_VBO);
            #if !defined(SDL_GPU_NO_VAO)
            glDeleteVertexArrays(1, &cdata->blit_VAO);
            #endif
        #endif

        #ifdef SDL_GPU_USE_SDL2
        if(target->context->context != 0)
            SDL_GL_DeleteContext(target->context->context);
        #endif

        // Remove all of the window mappings that refer to this target
        GPU_RemoveWindowMappingByTarget(target);

        SDL_free(target->context->data);
        SDL_free(target->context);
        target->context = NULL;
    }

    SDL_free(data);
    SDL_free(target);
}





#define SET_TEXTURED_VERTEX(x, y, s, t, r, g, b, a) \
    blit_buffer[vert_index] = x; \
    blit_buffer[vert_index+1] = y; \
    blit_buffer[tex_index] = s; \
    blit_buffer[tex_index+1] = t; \
    blit_buffer[color_index] = r; \
    blit_buffer[color_index+1] = g; \
    blit_buffer[color_index+2] = b; \
    blit_buffer[color_index+3] = a; \
    index_buffer[cdata->index_buffer_num_vertices++] = cdata->blit_buffer_num_vertices++; \
    vert_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX; \
    tex_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX; \
    color_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;

#define SET_TEXTURED_VERTEX_UNINDEXED(x, y, s, t, r, g, b, a) \
    blit_buffer[vert_index] = x; \
    blit_buffer[vert_index+1] = y; \
    blit_buffer[tex_index] = s; \
    blit_buffer[tex_index+1] = t; \
    blit_buffer[color_index] = r; \
    blit_buffer[color_index+1] = g; \
    blit_buffer[color_index+2] = b; \
    blit_buffer[color_index+3] = a; \
    vert_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX; \
    tex_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX; \
    color_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;

#define SET_UNTEXTURED_VERTEX(x, y, r, g, b, a) \
    blit_buffer[vert_index] = x; \
    blit_buffer[vert_index+1] = y; \
    blit_buffer[color_index] = r; \
    blit_buffer[color_index+1] = g; \
    blit_buffer[color_index+2] = b; \
    blit_buffer[color_index+3] = a; \
    index_buffer[cdata->index_buffer_num_vertices++] = cdata->blit_buffer_num_vertices++; \
    vert_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX; \
    color_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;

#define SET_UNTEXTURED_VERTEX_UNINDEXED(x, y, r, g, b, a) \
    blit_buffer[vert_index] = x; \
    blit_buffer[vert_index+1] = y; \
    blit_buffer[color_index] = r; \
    blit_buffer[color_index+1] = g; \
    blit_buffer[color_index+2] = b; \
    blit_buffer[color_index+3] = a; \
    vert_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX; \
    color_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;

#define SET_INDEXED_VERTEX(offset) \
    index_buffer[cdata->index_buffer_num_vertices++] = blit_buffer_starting_index + (offset);

#define SET_RELATIVE_INDEXED_VERTEX(offset) \
    index_buffer[cdata->index_buffer_num_vertices++] = cdata->blit_buffer_num_vertices + (offset);



#define BEGIN_UNTEXTURED_SEGMENTS(x1, y1, x2, y2, r, g, b, a) \
    SET_UNTEXTURED_VERTEX(x1, y1, r, g, b, a); \
    SET_UNTEXTURED_VERTEX(x2, y2, r, g, b, a);

// Finish previous triangles and start the next one
#define SET_UNTEXTURED_SEGMENTS(x1, y1, x2, y2, r, g, b, a) \
    SET_UNTEXTURED_VERTEX(x1, y1, r, g, b, a); \
    SET_RELATIVE_INDEXED_VERTEX(-2); \
    SET_UNTEXTURED_VERTEX(x2, y2, r, g, b, a); \
    SET_RELATIVE_INDEXED_VERTEX(-2); \
    SET_RELATIVE_INDEXED_VERTEX(-2); \
    SET_RELATIVE_INDEXED_VERTEX(-1);

// Finish previous triangles
#define LOOP_UNTEXTURED_SEGMENTS() \
    SET_INDEXED_VERTEX(0); \
    SET_RELATIVE_INDEXED_VERTEX(-1); \
    SET_INDEXED_VERTEX(1); \
    SET_INDEXED_VERTEX(0);

#define END_UNTEXTURED_SEGMENTS(x1, y1, x2, y2, r, g, b, a) \
    SET_UNTEXTURED_VERTEX(x1, y1, r, g, b, a); \
    SET_RELATIVE_INDEXED_VERTEX(-2); \
    SET_UNTEXTURED_VERTEX(x2, y2, r, g, b, a); \
    SET_RELATIVE_INDEXED_VERTEX(-2);



static void Blit(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y)
{
	Uint32 tex_w, tex_h;
	float w;
	float h;
	float x1, y1, x2, y2;
	float dx1, dy1, dx2, dy2;
	GPU_CONTEXT_DATA* cdata;
	float* blit_buffer;
	unsigned short* index_buffer;
	unsigned short blit_buffer_starting_index;
	int vert_index;
	int tex_index;
	int color_index;
	float r, g, b, a;

    if(image == NULL)
    {
        GPU_PushErrorCode("GPU_Blit", GPU_ERROR_NULL_ARGUMENT, "image");
        return;
    }
    if(target == NULL)
    {
        GPU_PushErrorCode("GPU_Blit", GPU_ERROR_NULL_ARGUMENT, "target");
        return;
    }
    if(renderer != image->renderer || renderer != target->renderer)
    {
        GPU_PushErrorCode("GPU_Blit", GPU_ERROR_USER_ERROR, "Mismatched renderer");
        return;
    }

    makeContextCurrent(renderer, target);
    if(renderer->current_context_target == NULL)
    {
        GPU_PushErrorCode("GPU_Blit", GPU_ERROR_USER_ERROR, "NULL context");
        return;
    }

    prepareToRenderToTarget(renderer, target);
    prepareToRenderImage(renderer, target, image);

    // Bind the texture to which subsequent calls refer
    bindTexture(renderer, image);

    // Bind the FBO
    if(!bindFramebuffer(renderer, target))
    {
        GPU_PushErrorCode("GPU_Blit", GPU_ERROR_BACKEND_ERROR, "Failed to bind framebuffer.");
        return;
    }

    tex_w = image->texture_w;
    tex_h = image->texture_h;

    if(image->snap_mode == GPU_SNAP_POSITION || image->snap_mode == GPU_SNAP_POSITION_AND_DIMENSIONS)
    {
        // Avoid rounding errors in texture sampling by insisting on integral pixel positions
        x = floorf(x);
        y = floorf(y);
    }

    if(src_rect == NULL)
    {
        // Scale tex coords according to actual texture dims
        x1 = 0.0f;
        y1 = 0.0f;
        x2 = ((float)image->w)/tex_w;
        y2 = ((float)image->h)/tex_h;
        w = image->w;
        h = image->h;
    }
    else
    {
        // Scale src_rect tex coords according to actual texture dims
        x1 = src_rect->x/(float)tex_w;
        y1 = src_rect->y/(float)tex_h;
        x2 = (src_rect->x + src_rect->w)/(float)tex_w;
        y2 = (src_rect->y + src_rect->h)/(float)tex_h;
        w = src_rect->w;
        h = src_rect->h;
    }

    if(image->using_virtual_resolution)
    {
        // Scale texture coords to fit the original dims
        x1 *= image->base_w/(float)image->w;
        y1 *= image->base_h/(float)image->h;
        x2 *= image->base_w/(float)image->w;
        y2 *= image->base_h/(float)image->h;
    }

    // Center the image on the given coords
    dx1 = x - w/2.0f;
    dy1 = y - h/2.0f;
    dx2 = x + w/2.0f;
    dy2 = y + h/2.0f;

    if(image->snap_mode == GPU_SNAP_DIMENSIONS || image->snap_mode == GPU_SNAP_POSITION_AND_DIMENSIONS)
    {
        float fractional;
        fractional = w/2.0f - floorf(w/2.0f);
        dx1 += fractional;
        dx2 += fractional;
        fractional = h/2.0f - floorf(h/2.0f);
        dy1 += fractional;
        dy2 += fractional;
    }

    if(renderer->coordinate_mode == 1)
    {
        float temp = dy1;
        dy1 = dy2;
        dy2 = temp;
    }

    cdata = (GPU_CONTEXT_DATA*)renderer->current_context_target->context->data;

    if(cdata->blit_buffer_num_vertices + 4 >= cdata->blit_buffer_max_num_vertices)
    {
        if(!growBlitBuffer(cdata, cdata->blit_buffer_num_vertices + 4))
            renderer->impl->FlushBlitBuffer(renderer);
    }
    if(cdata->index_buffer_num_vertices + 6 >= cdata->index_buffer_max_num_vertices)
    {
        if(!growIndexBuffer(cdata, cdata->index_buffer_num_vertices + 6))
            renderer->impl->FlushBlitBuffer(renderer);
    }

    blit_buffer = cdata->blit_buffer;
    index_buffer = cdata->index_buffer;

    blit_buffer_starting_index = cdata->blit_buffer_num_vertices;

    vert_index = GPU_BLIT_BUFFER_VERTEX_OFFSET + cdata->blit_buffer_num_vertices*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
    tex_index = GPU_BLIT_BUFFER_TEX_COORD_OFFSET + cdata->blit_buffer_num_vertices*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
    color_index = GPU_BLIT_BUFFER_COLOR_OFFSET + cdata->blit_buffer_num_vertices*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
    if(target->use_color)
    {
        r = MIX_COLOR_COMPONENT_NORMALIZED_RESULT(target->color.r, image->color.r);
        g = MIX_COLOR_COMPONENT_NORMALIZED_RESULT(target->color.g, image->color.g);
        b = MIX_COLOR_COMPONENT_NORMALIZED_RESULT(target->color.b, image->color.b);
        a = MIX_COLOR_COMPONENT_NORMALIZED_RESULT(GET_ALPHA(target->color), GET_ALPHA(image->color));
    }
    else
    {
        r = image->color.r/255.0f;
        g = image->color.g/255.0f;
        b = image->color.b/255.0f;
        a = GET_ALPHA(image->color)/255.0f;
    }

    // 4 Quad vertices
    SET_TEXTURED_VERTEX_UNINDEXED(dx1, dy1, x1, y1, r, g, b, a);
    SET_TEXTURED_VERTEX_UNINDEXED(dx2, dy1, x2, y1, r, g, b, a);
    SET_TEXTURED_VERTEX_UNINDEXED(dx2, dy2, x2, y2, r, g, b, a);
    SET_TEXTURED_VERTEX_UNINDEXED(dx1, dy2, x1, y2, r, g, b, a);

    // 6 Triangle indices
    SET_INDEXED_VERTEX(0);
    SET_INDEXED_VERTEX(1);
    SET_INDEXED_VERTEX(2);

    SET_INDEXED_VERTEX(0);
    SET_INDEXED_VERTEX(2);
    SET_INDEXED_VERTEX(3);

    cdata->blit_buffer_num_vertices += GPU_BLIT_BUFFER_VERTICES_PER_SPRITE;
}


static void BlitRotate(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees)
{
	float w, h;
    if(image == NULL)
    {
        GPU_PushErrorCode("GPU_BlitRotate", GPU_ERROR_NULL_ARGUMENT, "image");
        return;
    }
    if(target == NULL)
    {
        GPU_PushErrorCode("GPU_BlitRotate", GPU_ERROR_NULL_ARGUMENT, "target");
        return;
    }

    w = (src_rect == NULL? image->w : src_rect->w);
    h = (src_rect == NULL? image->h : src_rect->h);
    renderer->impl->BlitTransformX(renderer, image, src_rect, target, x, y, w/2.0f, h/2.0f, degrees, 1.0f, 1.0f);
}

static void BlitScale(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float scaleX, float scaleY)
{
	float w, h;
    if(image == NULL)
    {
        GPU_PushErrorCode("GPU_BlitScale", GPU_ERROR_NULL_ARGUMENT, "image");
        return;
    }
    if(target == NULL)
    {
        GPU_PushErrorCode("GPU_BlitScale", GPU_ERROR_NULL_ARGUMENT, "target");
        return;
    }

    w = (src_rect == NULL? image->w : src_rect->w);
    h = (src_rect == NULL? image->h : src_rect->h);
    renderer->impl->BlitTransformX(renderer, image, src_rect, target, x, y, w/2.0f, h/2.0f, 0.0f, scaleX, scaleY);
}

static void BlitTransform(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees, float scaleX, float scaleY)
{
	float w, h;
    if(image == NULL)
    {
        GPU_PushErrorCode("GPU_BlitTransform", GPU_ERROR_NULL_ARGUMENT, "image");
        return;
    }
    if(target == NULL)
    {
        GPU_PushErrorCode("GPU_BlitTransform", GPU_ERROR_NULL_ARGUMENT, "target");
        return;
    }

    w = (src_rect == NULL? image->w : src_rect->w);
    h = (src_rect == NULL? image->h : src_rect->h);
    renderer->impl->BlitTransformX(renderer, image, src_rect, target, x, y, w/2.0f, h/2.0f, degrees, scaleX, scaleY);
}

static void BlitTransformX(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float pivot_x, float pivot_y, float degrees, float scaleX, float scaleY)
{
	Uint32 tex_w, tex_h;
	float x1, y1, x2, y2;
	float dx1, dy1, dx2, dy2, dx3, dy3, dx4, dy4;
	float w, h;
	GPU_CONTEXT_DATA* cdata;
	float* blit_buffer;
	unsigned short* index_buffer;
	unsigned short blit_buffer_starting_index;
	int vert_index;
	int tex_index;
	int color_index;
	float r, g, b, a;

    if(image == NULL)
    {
        GPU_PushErrorCode("GPU_BlitTransformX", GPU_ERROR_NULL_ARGUMENT, "image");
        return;
    }
    if(target == NULL)
    {
        GPU_PushErrorCode("GPU_BlitTransformX", GPU_ERROR_NULL_ARGUMENT, "target");
        return;
    }
    if(renderer != image->renderer || renderer != target->renderer)
    {
        GPU_PushErrorCode("GPU_BlitTransformX", GPU_ERROR_USER_ERROR, "Mismatched renderer");
        return;
    }


    makeContextCurrent(renderer, target);

    prepareToRenderToTarget(renderer, target);
    prepareToRenderImage(renderer, target, image);

    // Bind the texture to which subsequent calls refer
    bindTexture(renderer, image);

    // Bind the FBO
    if(!bindFramebuffer(renderer, target))
    {
        GPU_PushErrorCode("GPU_BlitTransformX", GPU_ERROR_BACKEND_ERROR, "Failed to bind framebuffer.");
        return;
    }

    tex_w = image->texture_w;
    tex_h = image->texture_h;

    if(image->snap_mode == GPU_SNAP_POSITION || image->snap_mode == GPU_SNAP_POSITION_AND_DIMENSIONS)
    {
        // Avoid rounding errors in texture sampling by insisting on integral pixel positions
        x = floorf(x);
        y = floorf(y);
    }

    /*
        1,1 --- 3,3
         |       |
         |       |
        4,4 --- 2,2
    */
    if(src_rect == NULL)
    {
        // Scale tex coords according to actual texture dims
        x1 = 0.0f;
        y1 = 0.0f;
        x2 = ((float)image->w)/tex_w;
        y2 = ((float)image->h)/tex_h;
        w = image->w;
        h = image->h;
    }
    else
    {
        // Scale src_rect tex coords according to actual texture dims
        x1 = src_rect->x/(float)tex_w;
        y1 = src_rect->y/(float)tex_h;
        x2 = (src_rect->x + src_rect->w)/(float)tex_w;
        y2 = (src_rect->y + src_rect->h)/(float)tex_h;
        w = src_rect->w;
        h = src_rect->h;
    }

    if(image->using_virtual_resolution)
    {
        // Scale texture coords to fit the original dims
        x1 *= image->base_w/(float)image->w;
        y1 *= image->base_h/(float)image->h;
        x2 *= image->base_w/(float)image->w;
        y2 *= image->base_h/(float)image->h;
    }

    // Center the image on the given coords (offset later)
    dx1 = -w/2.0f;
    dy1 = -h/2.0f;
    dx2 = w/2.0f;
    dy2 = h/2.0f;

    if(image->snap_mode == GPU_SNAP_DIMENSIONS || image->snap_mode == GPU_SNAP_POSITION_AND_DIMENSIONS)
    {
        // This is a little weird for rotating sprites, but oh well.
        float fractional;
        fractional = w/2.0f - floorf(w/2.0f);
        dx1 += fractional;
        dx2 += fractional;
        fractional = h/2.0f - floorf(h/2.0f);
        dy1 += fractional;
        dy2 += fractional;
    }

    if(renderer->coordinate_mode == 1)
    {
        float temp = dy1;
        dy1 = dy2;
        dy2 = temp;
    }

    // Apply transforms

    // Scale
    if(scaleX != 1.0f || scaleY != 1.0f)
    {
        float w = (dx2 - dx1)*scaleX;
        float h = (dy2 - dy1)*scaleY;
        dx1 = (dx2 + dx1)/2 - w/2;
        dx2 = dx1 + w;
        dy1 = (dy2 + dy1)/2 - h/2;
        dy2 = dy1 + h;
    }

    // Shift away from the center (these are relative to the image corner)
    pivot_x -= w/2.0f;
    pivot_y -= h/2.0f;

    // Translate origin to pivot
    dx1 -= pivot_x*scaleX;
    dy1 -= pivot_y*scaleY;
    dx2 -= pivot_x*scaleX;
    dy2 -= pivot_y*scaleY;

    // Get extra vertices for rotation
    dx3 = dx2;
    dy3 = dy1;
    dx4 = dx1;
    dy4 = dy2;

    // Rotate about origin (the pivot)
    if(degrees != 0.0f)
    {
        float cosA = cos(degrees*M_PI/180);
        float sinA = sin(degrees*M_PI/180);
        float tempX = dx1;
        dx1 = dx1*cosA - dy1*sinA;
        dy1 = tempX*sinA + dy1*cosA;
        tempX = dx2;
        dx2 = dx2*cosA - dy2*sinA;
        dy2 = tempX*sinA + dy2*cosA;
        tempX = dx3;
        dx3 = dx3*cosA - dy3*sinA;
        dy3 = tempX*sinA + dy3*cosA;
        tempX = dx4;
        dx4 = dx4*cosA - dy4*sinA;
        dy4 = tempX*sinA + dy4*cosA;
    }

    // Translate to pos
    dx1 += x;
    dx2 += x;
    dx3 += x;
    dx4 += x;
    dy1 += y;
    dy2 += y;
    dy3 += y;
    dy4 += y;

    cdata = (GPU_CONTEXT_DATA*)renderer->current_context_target->context->data;

    if(cdata->blit_buffer_num_vertices + 4 >= cdata->blit_buffer_max_num_vertices)
    {
        if(!growBlitBuffer(cdata, cdata->blit_buffer_num_vertices + 4))
            renderer->impl->FlushBlitBuffer(renderer);
    }
    if(cdata->index_buffer_num_vertices + 6 >= cdata->index_buffer_max_num_vertices)
    {
        if(!growIndexBuffer(cdata, cdata->index_buffer_num_vertices + 6))
            renderer->impl->FlushBlitBuffer(renderer);
    }

    blit_buffer = cdata->blit_buffer;
    index_buffer = cdata->index_buffer;

    blit_buffer_starting_index = cdata->blit_buffer_num_vertices;

    vert_index = GPU_BLIT_BUFFER_VERTEX_OFFSET + cdata->blit_buffer_num_vertices*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
    tex_index = GPU_BLIT_BUFFER_TEX_COORD_OFFSET + cdata->blit_buffer_num_vertices*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
    color_index = GPU_BLIT_BUFFER_COLOR_OFFSET + cdata->blit_buffer_num_vertices*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;

    if(target->use_color)
    {
        r = MIX_COLOR_COMPONENT_NORMALIZED_RESULT(target->color.r, image->color.r);
        g = MIX_COLOR_COMPONENT_NORMALIZED_RESULT(target->color.g, image->color.g);
        b = MIX_COLOR_COMPONENT_NORMALIZED_RESULT(target->color.b, image->color.b);
        a = MIX_COLOR_COMPONENT_NORMALIZED_RESULT(GET_ALPHA(target->color), GET_ALPHA(image->color));
    }
    else
    {
        r = image->color.r/255.0f;
        g = image->color.g/255.0f;
        b = image->color.b/255.0f;
        a = GET_ALPHA(image->color)/255.0f;
    }

    // 4 Quad vertices
    SET_TEXTURED_VERTEX_UNINDEXED(dx1, dy1, x1, y1, r, g, b, a);
    SET_TEXTURED_VERTEX_UNINDEXED(dx3, dy3, x2, y1, r, g, b, a);
    SET_TEXTURED_VERTEX_UNINDEXED(dx2, dy2, x2, y2, r, g, b, a);
    SET_TEXTURED_VERTEX_UNINDEXED(dx4, dy4, x1, y2, r, g, b, a);

    // 6 Triangle indices
    SET_INDEXED_VERTEX(0);
    SET_INDEXED_VERTEX(1);
    SET_INDEXED_VERTEX(2);

    SET_INDEXED_VERTEX(0);
    SET_INDEXED_VERTEX(2);
    SET_INDEXED_VERTEX(3);

    cdata->blit_buffer_num_vertices += GPU_BLIT_BUFFER_VERTICES_PER_SPRITE;
}



#ifdef SDL_GPU_USE_BUFFER_PIPELINE


static_inline int sizeof_GPU_type(GPU_TypeEnum type)
{
    if(type == GPU_TYPE_DOUBLE) return sizeof(double);
    if(type == GPU_TYPE_FLOAT) return sizeof(float);
    if(type == GPU_TYPE_INT) return sizeof(int);
    if(type == GPU_TYPE_UNSIGNED_INT) return sizeof(unsigned int);
    if(type == GPU_TYPE_SHORT) return sizeof(short);
    if(type == GPU_TYPE_UNSIGNED_SHORT) return sizeof(unsigned short);
    if(type == GPU_TYPE_BYTE) return sizeof(char);
    if(type == GPU_TYPE_UNSIGNED_BYTE) return sizeof(unsigned char);
    return 0;
}

static void refresh_attribute_data(GPU_CONTEXT_DATA* cdata)
{
    int i;
    for(i = 0; i < 16; i++)
    {
        GPU_AttributeSource* a = &cdata->shader_attributes[i];
        if(a->attribute.values != NULL && a->attribute.location >= 0 && a->num_values > 0 && a->attribute.format.is_per_sprite)
        {
            // Expand the values to 4 vertices
            int n;
            void* storage_ptr = a->per_vertex_storage;
            void* values_ptr = (void*)((char*)a->attribute.values + a->attribute.format.offset_bytes);
            int value_size_bytes = a->attribute.format.num_elems_per_value * sizeof_GPU_type(a->attribute.format.type);
            for(n = 0; n < a->num_values; n+=4)
            {
                memcpy(storage_ptr, values_ptr, value_size_bytes);
                storage_ptr = (void*)((char*)storage_ptr + a->per_vertex_storage_stride_bytes);
                memcpy(storage_ptr, values_ptr, value_size_bytes);
                storage_ptr = (void*)((char*)storage_ptr + a->per_vertex_storage_stride_bytes);
                memcpy(storage_ptr, values_ptr, value_size_bytes);
                storage_ptr = (void*)((char*)storage_ptr + a->per_vertex_storage_stride_bytes);
                memcpy(storage_ptr, values_ptr, value_size_bytes);
                storage_ptr = (void*)((char*)storage_ptr + a->per_vertex_storage_stride_bytes);

                values_ptr = (void*)((char*)values_ptr + a->attribute.format.stride_bytes);
            }
        }
    }
}

static void upload_attribute_data(GPU_CONTEXT_DATA* cdata, int num_vertices)
{
    int i;
    for(i = 0; i < 16; i++)
    {
        GPU_AttributeSource* a = &cdata->shader_attributes[i];
        if(a->attribute.values != NULL && a->attribute.location >= 0 && a->num_values > 0)
        {
            int num_values_used = num_vertices;
			int bytes_used;

            if(a->num_values < num_values_used)
                num_values_used = a->num_values;

            glBindBuffer(GL_ARRAY_BUFFER, cdata->attribute_VBO[i]);

            bytes_used = a->per_vertex_storage_stride_bytes * num_values_used;
            glBufferData(GL_ARRAY_BUFFER, bytes_used, a->next_value, GL_STREAM_DRAW);

            glEnableVertexAttribArray(a->attribute.location);
            glVertexAttribPointer(a->attribute.location, a->attribute.format.num_elems_per_value, a->attribute.format.type, a->attribute.format.normalize, a->per_vertex_storage_stride_bytes, (void*)(intptr_t)a->per_vertex_storage_offset_bytes);

            a->enabled = 1;
            // Move the data along so we use the next values for the next flush
            a->num_values -= num_values_used;
            if(a->num_values <= 0)
                a->next_value = a->per_vertex_storage;
            else
                a->next_value = (void*)(((char*)a->next_value) + bytes_used);
        }
    }
}

static void disable_attribute_data(GPU_CONTEXT_DATA* cdata)
{
    int i;
    for(i = 0; i < 16; i++)
    {
        GPU_AttributeSource* a = &cdata->shader_attributes[i];
        if(a->enabled)
        {
            glDisableVertexAttribArray(a->attribute.location);
            a->enabled = 0;
        }
    }
}

#endif

static int get_lowest_attribute_num_values(GPU_CONTEXT_DATA* cdata, int cap)
{
    int lowest = cap;

#ifdef SDL_GPU_USE_BUFFER_PIPELINE
    int i;
    for(i = 0; i < 16; i++)
    {
        GPU_AttributeSource* a = &cdata->shader_attributes[i];
        if(a->attribute.values != NULL && a->attribute.location >= 0)
        {
            if(a->num_values < lowest)
                lowest = a->num_values;
        }
    }
#endif

    return lowest;
}

static_inline void submit_buffer_data(int bytes, float* values, int bytes_indices, unsigned short* indices)
{
    #ifdef SDL_GPU_USE_BUFFER_PIPELINE
        #ifdef SDL_GPU_USE_BUFFER_MAPPING
        float* data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        unsigned short* data_i = (unsigned short*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        if(data != NULL)
        {
            memcpy(data, values, bytes);
            glUnmapBuffer(GL_ARRAY_BUFFER);
        }
        if(data_i != NULL)
        {
            memcpy(data_i, indices, bytes_indices);
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        }
        #elif defined(SDL_GPU_USE_BUFFER_RESET)
        glBufferData(GL_ARRAY_BUFFER, bytes, values, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytes_indices, indices, GL_DYNAMIC_DRAW);
        #else
        glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, values);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, bytes_indices, indices);
        #endif
    #endif
}

// Assumes the right format
static void TriangleBatch(GPU_Renderer* renderer, GPU_Image* image, GPU_Target* target, unsigned short num_vertices, float* values, unsigned int num_indices, unsigned short* indices, GPU_BatchFlagEnum flags)
{
	GPU_CONTEXT_DATA* cdata;
	int stride, offset_texcoords, offset_colors;
	int size_vertices, size_texcoords, size_colors;

	Uint8 using_texture = (image != NULL);
	Uint8 use_vertices = (flags & (GPU_BATCH_XY | GPU_BATCH_XYZ));
	Uint8 use_texcoords = (flags & GPU_BATCH_ST);
	Uint8 use_colors = (flags & (GPU_BATCH_RGB | GPU_BATCH_RGBA));
	Uint8 use_z = (flags & GPU_BATCH_XYZ);
	Uint8 use_a = (flags & GPU_BATCH_RGBA);

    if(num_vertices == 0)
        return;

    if(target == NULL)
    {
        GPU_PushErrorCode("GPU_TriangleBatch", GPU_ERROR_NULL_ARGUMENT, "target");
        return;
    }
    if((image != NULL && renderer != image->renderer) || renderer != target->renderer)
    {
        GPU_PushErrorCode("GPU_TriangleBatch", GPU_ERROR_USER_ERROR, "Mismatched renderer");
        return;
    }

    makeContextCurrent(renderer, target);

    // Bind the texture to which subsequent calls refer
    if(using_texture)
        bindTexture(renderer, image);

    // Bind the FBO
    if(!bindFramebuffer(renderer, target))
    {
        GPU_PushErrorCode("GPU_TriangleBatch", GPU_ERROR_BACKEND_ERROR, "Failed to bind framebuffer.");
        return;
    }

    prepareToRenderToTarget(renderer, target);
    if(using_texture)
        prepareToRenderImage(renderer, target, image);
    else
        prepareToRenderShapes(renderer, GL_TRIANGLES);
    changeViewport(target);
    changeCamera(target);

    if(using_texture)
        changeTexturing(renderer, 1);

    setClipRect(renderer, target);

    #ifdef SDL_GPU_APPLY_TRANSFORMS_TO_GL_STACK
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_VERTEX_SHADER))
        applyTransforms();
    #endif


    cdata = (GPU_CONTEXT_DATA*)renderer->current_context_target->context->data;

    renderer->impl->FlushBlitBuffer(renderer);

    if(cdata->index_buffer_num_vertices + num_indices >= cdata->index_buffer_max_num_vertices)
    {
        growBlitBuffer(cdata, cdata->index_buffer_num_vertices + num_indices);
    }
    if(cdata->blit_buffer_num_vertices + num_vertices >= cdata->blit_buffer_max_num_vertices)
    {
        growBlitBuffer(cdata, cdata->blit_buffer_num_vertices + num_vertices);
    }

    // Only need to check the blit buffer because of the VBO storage
    if(cdata->blit_buffer_num_vertices + num_vertices >= cdata->blit_buffer_max_num_vertices)
    {
        if(!growBlitBuffer(cdata, cdata->blit_buffer_num_vertices + num_vertices))
        {
            // Can't do all of these sprites!  Only do some of them...
            num_vertices = (cdata->blit_buffer_max_num_vertices - cdata->blit_buffer_num_vertices);
        }
    }
    if(cdata->index_buffer_num_vertices + num_indices >= cdata->index_buffer_max_num_vertices)
    {
        if(!growIndexBuffer(cdata, cdata->index_buffer_num_vertices + num_indices))
        {
            // Can't do all of these sprites!  Only do some of them...
            num_indices = (cdata->index_buffer_max_num_vertices - cdata->index_buffer_num_vertices);
        }
    }

    #ifdef SDL_GPU_USE_BUFFER_PIPELINE
    refresh_attribute_data(cdata);
    #endif

    if(indices == NULL)
        num_indices = num_vertices;

    (void)stride;
    (void)offset_texcoords;
    (void)offset_colors;
    (void)size_vertices;
    (void)size_texcoords;
    (void)size_colors;

    stride = 0;
    offset_texcoords = offset_colors = 0;
    size_vertices = size_texcoords = size_colors = 0;

    // Determine stride, size, and offsets
    if(use_vertices)
    {
        if(use_z)
            size_vertices = 3;
        else
            size_vertices = 2;

        stride += size_vertices;

        offset_texcoords = stride;
        offset_colors = stride;
    }

    if(use_texcoords)
    {
        size_texcoords = 2;

        stride += size_texcoords;

        offset_colors = stride;
    }

    if(use_colors)
    {
        if(use_a)
            size_colors = 4;
        else
            size_colors = 3;

        stride += size_colors;
    }

    // Convert to a number of bytes
    stride *= sizeof(float);


#ifdef SDL_GPU_USE_ARRAY_PIPELINE

    {
        // Enable
        if(use_vertices)
            glEnableClientState(GL_VERTEX_ARRAY);
        if(use_texcoords)
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        if(use_colors)
            glEnableClientState(GL_COLOR_ARRAY);

        // Set pointers
        if(use_vertices)
            glVertexPointer(size_vertices, GL_FLOAT, stride, values);
        if(use_texcoords)
            glTexCoordPointer(size_texcoords, GL_FLOAT, stride, values + offset_texcoords);
        if(use_colors)
            glColorPointer(size_colors, GL_FLOAT, stride, values + offset_colors);

        // Upload
        if(indices == NULL)
            glDrawArrays(GL_TRIANGLES, 0, num_indices);
        else
            glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, indices);

        // Disable
        if(use_colors)
            glDisableClientState(GL_COLOR_ARRAY);
        if(use_texcoords)
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if(use_vertices)
            glDisableClientState(GL_VERTEX_ARRAY);
    }
#endif



#ifdef SDL_GPU_USE_BUFFER_PIPELINE_FALLBACK
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_VERTEX_SHADER))
#endif
#ifdef SDL_GPU_USE_FIXED_FUNCTION_PIPELINE
    {
        if(values != NULL)
        {
            unsigned int i;
            unsigned int index;
            float* vertex_pointer = values;
            float* texcoord_pointer = values + offset_texcoords;
            float* color_pointer = values + offset_colors;

            glBegin(GL_TRIANGLES);
            for(i = 0; i < num_indices; i++)
            {
                if(indices == NULL)
                    index = i*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
                else
                    index = indices[i]*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
                if(use_colors)
                    glColor4f(color_pointer[index], color_pointer[index+1], color_pointer[index+2], (use_a? color_pointer[index+3] : 1.0f));
                if(use_texcoords)
                    glTexCoord2f( texcoord_pointer[index], texcoord_pointer[index+1] );
                if(use_vertices)
                    glVertex3f( vertex_pointer[index], vertex_pointer[index+1], (use_z? vertex_pointer[index+2] : 0.0f) );
            }
            glEnd();
        }
    }
#endif
#ifdef SDL_GPU_USE_BUFFER_PIPELINE_FALLBACK
    else
#endif



#ifdef SDL_GPU_USE_BUFFER_PIPELINE
    {
        // Skip uploads if we have no attribute location
        if(cdata->current_shader_block.position_loc < 0)
            use_vertices = 0;
        if(cdata->current_shader_block.texcoord_loc < 0)
            use_texcoords = 0;
        if(cdata->current_shader_block.color_loc < 0)
            use_colors = 0;

        // Update the vertex array object's buffers
        #if !defined(SDL_GPU_NO_VAO)
        glBindVertexArray(cdata->blit_VAO);
        #endif

        // Upload our modelviewprojection matrix
        if(cdata->current_shader_block.modelViewProjection_loc >= 0)
        {
            float mvp[16];
            GPU_GetModelViewProjection(mvp);
            glUniformMatrix4fv(cdata->current_shader_block.modelViewProjection_loc, 1, 0, mvp);
        }

        if(values != NULL)
        {
            // Upload blit buffer to a single buffer object
            glBindBuffer(GL_ARRAY_BUFFER, cdata->blit_VBO[cdata->blit_VBO_flop]);
            cdata->blit_VBO_flop = !cdata->blit_VBO_flop;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cdata->blit_IBO);

            // Copy the whole blit buffer to the GPU
            submit_buffer_data(stride * num_vertices, values, sizeof(unsigned short)*num_indices, indices);  // Fills GPU buffer with data.

            // Specify the formatting of the blit buffer
            if(use_vertices)
            {
                glEnableVertexAttribArray(cdata->current_shader_block.position_loc);  // Tell GL to use client-side attribute data
                glVertexAttribPointer(cdata->current_shader_block.position_loc, size_vertices, GL_FLOAT, GL_FALSE, stride, 0);  // Tell how the data is formatted
            }
            if(use_texcoords)
            {
                glEnableVertexAttribArray(cdata->current_shader_block.texcoord_loc);
                glVertexAttribPointer(cdata->current_shader_block.texcoord_loc, size_texcoords, GL_FLOAT, GL_FALSE, stride, (void*)(offset_texcoords * sizeof(float)));
            }
            if(use_colors)
            {
                glEnableVertexAttribArray(cdata->current_shader_block.color_loc);
                glVertexAttribPointer(cdata->current_shader_block.color_loc, size_colors, GL_FLOAT, GL_FALSE, stride, (void*)(offset_colors * sizeof(float)));
            }
        }

        upload_attribute_data(cdata, num_indices);

        if(indices == NULL)
            glDrawArrays(GL_TRIANGLES, 0, num_indices);
        else
            glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, (void*)0);

        // Disable the vertex arrays again
        if(use_vertices)
            glDisableVertexAttribArray(cdata->current_shader_block.position_loc);
        if(use_texcoords)
            glDisableVertexAttribArray(cdata->current_shader_block.texcoord_loc);
        if(use_colors)
            glDisableVertexAttribArray(cdata->current_shader_block.color_loc);

        disable_attribute_data(cdata);

        #if !defined(SDL_GPU_NO_VAO)
        glBindVertexArray(0);
        #endif
    }
#endif


    cdata->blit_buffer_num_vertices = 0;
    cdata->index_buffer_num_vertices = 0;

    unsetClipRect(renderer, target);
}

static void GenerateMipmaps(GPU_Renderer* renderer, GPU_Image* image)
{
    #ifndef __IPHONEOS__
    GLint filter;
    if(image == NULL)
        return;

    if(image->target != NULL && isCurrentTarget(renderer, image->target))
        renderer->impl->FlushBlitBuffer(renderer);
    bindTexture(renderer, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    image->has_mipmaps = 1;

    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &filter);
    if(filter == GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    #endif
}




static GPU_Rect SetClip(GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
	GPU_Rect r;
    if(target == NULL)
    {
        GPU_Rect r = {0,0,0,0};
        return r;
    }

    if(isCurrentTarget(renderer, target))
        renderer->impl->FlushBlitBuffer(renderer);
    target->use_clip_rect = 1;

    r = target->clip_rect;

    target->clip_rect.x = x;
    target->clip_rect.y = y;
    target->clip_rect.w = w;
    target->clip_rect.h = h;

    return r;
}

static void UnsetClip(GPU_Renderer* renderer, GPU_Target* target)
{
    if(target == NULL)
        return;

    makeContextCurrent(renderer, target);

    if(isCurrentTarget(renderer, target))
        renderer->impl->FlushBlitBuffer(renderer);
    // Leave the clip rect values intact so they can still be useful as storage
    target->use_clip_rect = 0;
}






static SDL_Color GetPixel(GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y)
{
    SDL_Color result = {0,0,0,0};
    if(target == NULL)
        return result;
    if(renderer != target->renderer)
        return result;
    if(x < 0 || y < 0 || x >= target->w || y >= target->h)
        return result;

    if(isCurrentTarget(renderer, target))
        renderer->impl->FlushBlitBuffer(renderer);
    if(bindFramebuffer(renderer, target))
    {
        unsigned char pixels[4];
        glReadPixels(x, y, 1, 1, ((GPU_TARGET_DATA*)target->data)->format, GL_UNSIGNED_BYTE, pixels);

        result.r = pixels[0];
        result.g = pixels[1];
        result.b = pixels[2];
        GET_ALPHA(result) = pixels[3];
    }

    return result;
}

static void SetImageFilter(GPU_Renderer* renderer, GPU_Image* image, GPU_FilterEnum filter)
{
	GLenum minFilter, magFilter;

    if(image == NULL)
    {
        GPU_PushErrorCode("GPU_SetImageFilter", GPU_ERROR_NULL_ARGUMENT, "image");
        return;
    }
    if(renderer != image->renderer)
    {
        GPU_PushErrorCode("GPU_SetImageFilter", GPU_ERROR_USER_ERROR, "Mismatched renderer");
        return;
    }

    switch(filter)
    {
        case GPU_FILTER_NEAREST:
            minFilter = GL_NEAREST;
            magFilter = GL_NEAREST;
            break;
        case GPU_FILTER_LINEAR:
            if(image->has_mipmaps)
                minFilter = GL_LINEAR_MIPMAP_NEAREST;
            else
                minFilter = GL_LINEAR;

            magFilter = GL_LINEAR;
            break;
        case GPU_FILTER_LINEAR_MIPMAP:
            if(image->has_mipmaps)
                minFilter = GL_LINEAR_MIPMAP_LINEAR;
            else
                minFilter = GL_LINEAR;

            magFilter = GL_LINEAR;
            break;
        default:
            GPU_PushErrorCode("GPU_SetImageFilter", GPU_ERROR_USER_ERROR, "Unsupported value for filter (0x%x)", filter);
            return;
    }

    flushBlitBufferIfCurrentTexture(renderer, image);
    bindTexture(renderer, image);

	image->filter_mode = filter;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}

static void SetWrapMode(GPU_Renderer* renderer, GPU_Image* image, GPU_WrapEnum wrap_mode_x, GPU_WrapEnum wrap_mode_y)
{
	GLenum wrap_x, wrap_y;

    if(image == NULL)
    {
        GPU_PushErrorCode("GPU_SetWrapMode", GPU_ERROR_NULL_ARGUMENT, "image");
        return;
    }
    if(renderer != image->renderer)
    {
        GPU_PushErrorCode("GPU_SetWrapMode", GPU_ERROR_USER_ERROR, "Mismatched renderer");
        return;
    }

	switch(wrap_mode_x)
	{
    case GPU_WRAP_NONE:
        wrap_x = GL_CLAMP_TO_EDGE;
        break;
    case GPU_WRAP_REPEAT:
        wrap_x = GL_REPEAT;
        break;
    case GPU_WRAP_MIRRORED:
        if(renderer->enabled_features & GPU_FEATURE_WRAP_REPEAT_MIRRORED)
            wrap_x = GL_MIRRORED_REPEAT;
        else
        {
            GPU_PushErrorCode("GPU_SetWrapMode", GPU_ERROR_BACKEND_ERROR, "This renderer does not support GPU_WRAP_MIRRORED.");
            return;
        }
        break;
    default:
        GPU_PushErrorCode("GPU_SetWrapMode", GPU_ERROR_USER_ERROR, "Unsupported value for wrap_mode_x (0x%x)", wrap_mode_x);
        return;
	}

	switch(wrap_mode_y)
	{
    case GPU_WRAP_NONE:
        wrap_y = GL_CLAMP_TO_EDGE;
        break;
    case GPU_WRAP_REPEAT:
        wrap_y = GL_REPEAT;
        break;
    case GPU_WRAP_MIRRORED:
        if(renderer->enabled_features & GPU_FEATURE_WRAP_REPEAT_MIRRORED)
            wrap_y = GL_MIRRORED_REPEAT;
        else
        {
            GPU_PushErrorCode("GPU_SetWrapMode", GPU_ERROR_BACKEND_ERROR, "This renderer does not support GPU_WRAP_MIRRORED.");
            return;
        }
        break;
    default:
        GPU_PushErrorCode("GPU_SetWrapMode", GPU_ERROR_USER_ERROR, "Unsupported value for wrap_mode_y (0x%x)", wrap_mode_y);
        return;
	}

    flushBlitBufferIfCurrentTexture(renderer, image);
    bindTexture(renderer, image);

	image->wrap_mode_x = wrap_mode_x;
	image->wrap_mode_y = wrap_mode_y;

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_x );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_y );
}





static void ClearRGBA(GPU_Renderer* renderer, GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    if(target == NULL)
        return;
    if(renderer != target->renderer)
        return;

    makeContextCurrent(renderer, target);

    if(isCurrentTarget(renderer, target))
        renderer->impl->FlushBlitBuffer(renderer);
    if(bindFramebuffer(renderer, target))
    {
        setClipRect(renderer, target);

        glClearColor(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        unsetClipRect(renderer, target);
    }
}

static void DoPartialFlush(GPU_Renderer* renderer, GPU_CONTEXT_DATA* cdata, unsigned short num_vertices, float* blit_buffer, unsigned int num_indices, unsigned short* index_buffer)
{
	(void)renderer;
#ifdef SDL_GPU_USE_ARRAY_PIPELINE
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(2, GL_FLOAT, GPU_BLIT_BUFFER_STRIDE, blit_buffer + GPU_BLIT_BUFFER_VERTEX_OFFSET);
    glTexCoordPointer(2, GL_FLOAT, GPU_BLIT_BUFFER_STRIDE, blit_buffer + GPU_BLIT_BUFFER_TEX_COORD_OFFSET);
    glColorPointer(4, GL_FLOAT, GPU_BLIT_BUFFER_STRIDE, blit_buffer + GPU_BLIT_BUFFER_COLOR_OFFSET);

    glDrawElements(cdata->last_shape, num_indices, GL_UNSIGNED_SHORT, index_buffer);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
#endif




#ifdef SDL_GPU_USE_BUFFER_PIPELINE_FALLBACK
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_VERTEX_SHADER))
#endif
#ifdef SDL_GPU_USE_FIXED_FUNCTION_PIPELINE
    {
        unsigned short i;
        unsigned int index;
        float* vertex_pointer = blit_buffer + GPU_BLIT_BUFFER_VERTEX_OFFSET;
        float* texcoord_pointer = blit_buffer + GPU_BLIT_BUFFER_TEX_COORD_OFFSET;
        float* color_pointer = blit_buffer + GPU_BLIT_BUFFER_COLOR_OFFSET;

        glBegin(cdata->last_shape);
        for(i = 0; i < num_indices; i++)
        {
            index = index_buffer[i]*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
            glColor4f( color_pointer[index], color_pointer[index+1], color_pointer[index+2], color_pointer[index+3] );
            glTexCoord2f( texcoord_pointer[index], texcoord_pointer[index+1] );
            glVertex3f( vertex_pointer[index], vertex_pointer[index+1], 0.0f );
        }
        glEnd();

        return;
    }
#endif




#ifdef SDL_GPU_USE_BUFFER_PIPELINE
        {
            // Update the vertex array object's buffers
            #if !defined(SDL_GPU_NO_VAO)
            glBindVertexArray(cdata->blit_VAO);
            #endif

            // Upload our modelviewprojection matrix
            if(cdata->current_shader_block.modelViewProjection_loc >= 0)
            {
                float mvp[16];
                GPU_GetModelViewProjection(mvp);
                glUniformMatrix4fv(cdata->current_shader_block.modelViewProjection_loc, 1, 0, mvp);
            }

            // Upload blit buffer to a single buffer object
            glBindBuffer(GL_ARRAY_BUFFER, cdata->blit_VBO[cdata->blit_VBO_flop]);
            cdata->blit_VBO_flop = !cdata->blit_VBO_flop;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cdata->blit_IBO);

            // Copy the whole blit buffer to the GPU
            submit_buffer_data(GPU_BLIT_BUFFER_STRIDE * num_vertices, blit_buffer, sizeof(unsigned short)*num_indices, index_buffer);  // Fills GPU buffer with data.

            // Specify the formatting of the blit buffer
            if(cdata->current_shader_block.position_loc >= 0)
            {
                glEnableVertexAttribArray(cdata->current_shader_block.position_loc);  // Tell GL to use client-side attribute data
                glVertexAttribPointer(cdata->current_shader_block.position_loc, 2, GL_FLOAT, GL_FALSE, GPU_BLIT_BUFFER_STRIDE, 0);  // Tell how the data is formatted
            }
            if(cdata->current_shader_block.texcoord_loc >= 0)
            {
                glEnableVertexAttribArray(cdata->current_shader_block.texcoord_loc);
                glVertexAttribPointer(cdata->current_shader_block.texcoord_loc, 2, GL_FLOAT, GL_FALSE, GPU_BLIT_BUFFER_STRIDE, (void*)(GPU_BLIT_BUFFER_TEX_COORD_OFFSET * sizeof(float)));
            }
            if(cdata->current_shader_block.color_loc >= 0)
            {
                glEnableVertexAttribArray(cdata->current_shader_block.color_loc);
                glVertexAttribPointer(cdata->current_shader_block.color_loc, 4, GL_FLOAT, GL_FALSE, GPU_BLIT_BUFFER_STRIDE, (void*)(GPU_BLIT_BUFFER_COLOR_OFFSET * sizeof(float)));
            }

            upload_attribute_data(cdata, num_vertices);

            glDrawElements(cdata->last_shape, num_indices, GL_UNSIGNED_SHORT, (void*)0);

            // Disable the vertex arrays again
            if(cdata->current_shader_block.position_loc >= 0)
                glDisableVertexAttribArray(cdata->current_shader_block.position_loc);
            if(cdata->current_shader_block.texcoord_loc >= 0)
                glDisableVertexAttribArray(cdata->current_shader_block.texcoord_loc);
            if(cdata->current_shader_block.color_loc >= 0)
                glDisableVertexAttribArray(cdata->current_shader_block.color_loc);

            disable_attribute_data(cdata);

            #if !defined(SDL_GPU_NO_VAO)
            glBindVertexArray(0);
            #endif
        }
#endif
}

static void DoUntexturedFlush(GPU_Renderer* renderer, GPU_CONTEXT_DATA* cdata, unsigned short num_vertices, float* blit_buffer, unsigned int num_indices, unsigned short* index_buffer)
{
	(void)renderer;

#ifdef SDL_GPU_USE_ARRAY_PIPELINE
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(2, GL_FLOAT, GPU_BLIT_BUFFER_STRIDE, blit_buffer + GPU_BLIT_BUFFER_VERTEX_OFFSET);
    glColorPointer(4, GL_FLOAT, GPU_BLIT_BUFFER_STRIDE, blit_buffer + GPU_BLIT_BUFFER_COLOR_OFFSET);

    glDrawElements(cdata->last_shape, num_indices, GL_UNSIGNED_SHORT, index_buffer);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
#endif


#ifdef SDL_GPU_USE_BUFFER_PIPELINE_FALLBACK
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_VERTEX_SHADER))
#endif
#ifdef SDL_GPU_USE_FIXED_FUNCTION_PIPELINE
    {
        unsigned short i;
        unsigned int index;
        float* vertex_pointer = blit_buffer + GPU_BLIT_BUFFER_VERTEX_OFFSET;
        float* color_pointer = blit_buffer + GPU_BLIT_BUFFER_COLOR_OFFSET;

        glBegin(cdata->last_shape);
        for(i = 0; i < num_indices; i++)
        {
            index = index_buffer[i]*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
            glColor4f( color_pointer[index], color_pointer[index+1], color_pointer[index+2], color_pointer[index+3] );
            glVertex3f( vertex_pointer[index], vertex_pointer[index+1], 0.0f );
        }
        glEnd();

        return;
    }
#endif

#ifdef SDL_GPU_USE_BUFFER_PIPELINE
    {
        // Update the vertex array object's buffers
        #if !defined(SDL_GPU_NO_VAO)
        glBindVertexArray(cdata->blit_VAO);
        #endif

        // Upload our modelviewprojection matrix
        if(cdata->current_shader_block.modelViewProjection_loc >= 0)
        {
            float mvp[16];
            GPU_GetModelViewProjection(mvp);
            glUniformMatrix4fv(cdata->current_shader_block.modelViewProjection_loc, 1, 0, mvp);
        }

        // Upload blit buffer to a single buffer object
        glBindBuffer(GL_ARRAY_BUFFER, cdata->blit_VBO[cdata->blit_VBO_flop]);
        cdata->blit_VBO_flop = !cdata->blit_VBO_flop;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cdata->blit_IBO);

        // Copy the whole blit buffer to the GPU
        submit_buffer_data(GPU_BLIT_BUFFER_STRIDE * num_vertices, blit_buffer, sizeof(unsigned short)*num_indices, index_buffer);  // Fills GPU buffer with data.

        // Specify the formatting of the blit buffer
        if(cdata->current_shader_block.position_loc >= 0)
        {
            glEnableVertexAttribArray(cdata->current_shader_block.position_loc);  // Tell GL to use client-side attribute data
            glVertexAttribPointer(cdata->current_shader_block.position_loc, 2, GL_FLOAT, GL_FALSE, GPU_BLIT_BUFFER_STRIDE, 0);  // Tell how the data is formatted
        }
        if(cdata->current_shader_block.color_loc >= 0)
        {
            glEnableVertexAttribArray(cdata->current_shader_block.color_loc);
            glVertexAttribPointer(cdata->current_shader_block.color_loc, 4, GL_FLOAT, GL_FALSE, GPU_BLIT_BUFFER_STRIDE, (void*)(GPU_BLIT_BUFFER_COLOR_OFFSET * sizeof(float)));
        }

        upload_attribute_data(cdata, num_vertices);

        glDrawElements(cdata->last_shape, num_indices, GL_UNSIGNED_SHORT, (void*)0);

        // Disable the vertex arrays again
        if(cdata->current_shader_block.position_loc >= 0)
            glDisableVertexAttribArray(cdata->current_shader_block.position_loc);
        if(cdata->current_shader_block.color_loc >= 0)
            glDisableVertexAttribArray(cdata->current_shader_block.color_loc);

        disable_attribute_data(cdata);

        #if !defined(SDL_GPU_NO_VAO)
        glBindVertexArray(0);
        #endif
    }
#endif
}

#define MAX(a, b) ((a) > (b)? (a) : (b))

static void FlushBlitBuffer(GPU_Renderer* renderer)
{
    GPU_CONTEXT_DATA* cdata;
    if(renderer->current_context_target == NULL)
        return;

    cdata = (GPU_CONTEXT_DATA*)renderer->current_context_target->context->data;
    if(cdata->blit_buffer_num_vertices > 0 && cdata->last_target != NULL)
    {
		GPU_Target* dest = cdata->last_target;
		int num_vertices;
		int num_indices;
		float* blit_buffer;
		unsigned short* index_buffer;

        changeViewport(dest);
        changeCamera(dest);

        applyTexturing(renderer);

        #ifdef SDL_GPU_APPLY_TRANSFORMS_TO_GL_STACK
        if(!IsFeatureEnabled(renderer, GPU_FEATURE_VERTEX_SHADER))
            applyTransforms();
        #endif

        setClipRect(renderer, dest);

        #ifdef SDL_GPU_USE_BUFFER_PIPELINE
        refresh_attribute_data(cdata);
        #endif

        blit_buffer = cdata->blit_buffer;
        index_buffer = cdata->index_buffer;

        if(cdata->last_use_texturing)
        {
            while(cdata->blit_buffer_num_vertices > 0)
            {
                num_vertices = MAX(cdata->blit_buffer_num_vertices, get_lowest_attribute_num_values(cdata, cdata->blit_buffer_num_vertices));
                num_indices = num_vertices * 3 / 2;  // 6 indices per sprite / 4 vertices per sprite = 3/2

                DoPartialFlush(renderer, cdata, num_vertices, blit_buffer, num_indices, index_buffer);

                cdata->blit_buffer_num_vertices -= num_vertices;
                // Move our pointers ahead
                blit_buffer += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX*num_vertices;
                index_buffer += num_indices;
            }
        }
        else
        {
            DoUntexturedFlush(renderer, cdata, cdata->blit_buffer_num_vertices, blit_buffer, cdata->index_buffer_num_vertices, index_buffer);
        }

        cdata->blit_buffer_num_vertices = 0;
        cdata->index_buffer_num_vertices = 0;

        unsetClipRect(renderer, dest);
    }
}

static void Flip(GPU_Renderer* renderer, GPU_Target* target)
{
    renderer->impl->FlushBlitBuffer(renderer);

    makeContextCurrent(renderer, target);

#ifdef SDL_GPU_USE_SDL2
    SDL_GL_SwapWindow(SDL_GetWindowFromID(renderer->current_context_target->context->windowID));
#else
    SDL_GL_SwapBuffers();
#endif

    #ifdef SDL_GPU_USE_OPENGL
    if(vendor_is_Intel)
        apply_Intel_attrib_workaround = 1;
    #endif
}




// Shader API


#include <string.h>

// On some platforms (e.g. Android), it might not be possible to just create a rwops and get the expected #included files.
// To do it, I might want to add an optional argument that specifies a base directory to prepend to #include file names.

static Uint32 GetShaderSourceSize(const char* filename);
static Uint32 GetShaderSource(const char* filename, char* result);

static void read_until_end_of_comment(SDL_RWops* rwops, char multiline)
{
    char buffer;
    while(SDL_RWread(rwops, &buffer, 1, 1) > 0)
    {
        if(!multiline)
        {
            if(buffer == '\n')
                break;
        }
        else
        {
            if(buffer == '*')
            {
                // If the stream ends at the next character or it is a '/', then we're done.
                if(SDL_RWread(rwops, &buffer, 1, 1) <= 0 || buffer == '/')
                    break;
            }
        }
    }
}

static Uint32 GetShaderSourceSize_RW(SDL_RWops* shader_source)
{
	Uint32 size;
	char last_char;
	char buffer[512];
	long len;

    if(shader_source == NULL)
        return 0;

    size = 0;

    // Read 1 byte at a time until we reach the end
    last_char = ' ';
    len = 0;
    while((len = SDL_RWread(shader_source, &buffer, 1, 1)) > 0)
    {
        // Follow through an #include directive?
        if(buffer[0] == '#')
        {
            // Get the rest of the line
            int line_size = 1;
            unsigned long line_len;
			char* token;
            while((line_len = SDL_RWread(shader_source, buffer+line_size, 1, 1)) > 0)
            {
                line_size += line_len;
                if(buffer[line_size - line_len] == '\n')
                    break;
            }
            buffer[line_size] = '\0';

            // Is there "include" after '#'?
            token = strtok(buffer, "# \t");

            if(token != NULL && strcmp(token, "include") == 0)
            {
                // Get filename token
                token = strtok(NULL, "\"");  // Skip the empty token before the quote
                if(token != NULL)
                {
                    // Add the size of the included file and a newline character
                    size += GetShaderSourceSize(token) + 1;
                }
            }
            else
                size += line_size;
            last_char = ' ';
            continue;
        }

        size += len;

        if(last_char == '/')
        {
            if(buffer[0] == '/')
            {
                read_until_end_of_comment(shader_source, 0);
                size++;  // For the end of the comment
            }
            else if(buffer[0] == '*')
            {
                read_until_end_of_comment(shader_source, 1);
                size += 2;  // For the end of the comments
            }
            last_char = ' ';
        }
        else
            last_char = buffer[0];
    }

    // Go back to the beginning of the stream
    SDL_RWseek(shader_source, 0, SEEK_SET);
    return size;
}


static Uint32 GetShaderSource_RW(SDL_RWops* shader_source, char* result)
{
	Uint32 size;
	char last_char;
	char buffer[512];
	long len;

    if(shader_source == NULL)
    {
        result[0] = '\0';
        return 0;
    }

    size = 0;

    // Read 1 byte at a time until we reach the end
    last_char = ' ';
    len = 0;
    while((len = SDL_RWread(shader_source, &buffer, 1, 1)) > 0)
    {
        // Follow through an #include directive?
        if(buffer[0] == '#')
        {
            // Get the rest of the line
            int line_size = 1;
			unsigned long line_len;
			char token_buffer[512];  // strtok() is destructive
			char* token;
            while((line_len = SDL_RWread(shader_source, buffer+line_size, 1, 1)) > 0)
            {
                line_size += line_len;
                if(buffer[line_size - line_len] == '\n')
                    break;
            }

            // Is there "include" after '#'?
            memcpy(token_buffer, buffer, line_size+1);
            token_buffer[line_size] = '\0';
            token = strtok(token_buffer, "# \t");

            if(token != NULL && strcmp(token, "include") == 0)
            {
                // Get filename token
                token = strtok(NULL, "\"");  // Skip the empty token before the quote
                if(token != NULL)
                {
                    // Add the size of the included file and a newline character
                    size += GetShaderSource(token, result + size);
                    result[size] = '\n';
                    size++;
                }
            }
            else
            {
                memcpy(result + size, buffer, line_size);
                size += line_size;
            }
            last_char = ' ';
            continue;
        }

        memcpy(result + size, buffer, len);
        size += len;

        if(last_char == '/')
        {
            if(buffer[0] == '/')
            {
                read_until_end_of_comment(shader_source, 0);
                memcpy(result + size, "\n", 1);
                size++;
            }
            else if(buffer[0] == '*')
            {
                read_until_end_of_comment(shader_source, 1);
                memcpy(result + size, "*/", 2);
                size += 2;
            }
            last_char = ' ';
        }
        else
            last_char = buffer[0];
    }
    result[size] = '\0';

    // Go back to the beginning of the stream
    SDL_RWseek(shader_source, 0, SEEK_SET);
    return size;
}

static Uint32 GetShaderSource(const char* filename, char* result)
{
	SDL_RWops* rwops;
	Uint32 size;

    if(filename == NULL)
        return 0;
    rwops = SDL_RWFromFile(filename, "r");

    size = GetShaderSource_RW(rwops, result);

    SDL_RWclose(rwops);
    return size;
}

static Uint32 GetShaderSourceSize(const char* filename)
{
	SDL_RWops* rwops;
	Uint32 result;

    if(filename == NULL)
        return 0;
    rwops = SDL_RWFromFile(filename, "r");

    result = GetShaderSourceSize_RW(rwops);

    SDL_RWclose(rwops);
    return result;
}


static Uint32 compile_shader_source(GPU_ShaderEnum shader_type, const char* shader_source)
{
    // Create the proper new shader object
    GLuint shader_object = 0;
	(void)shader_type;
	(void)shader_source;

    #ifndef SDL_GPU_DISABLE_SHADERS
    GLint compiled;

    switch(shader_type)
    {
    case GPU_VERTEX_SHADER:
        shader_object = glCreateShader(GL_VERTEX_SHADER);
        break;
    case GPU_FRAGMENT_SHADER:
        shader_object = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    case GPU_GEOMETRY_SHADER:
    #ifdef GL_GEOMETRY_SHADER
        shader_object = glCreateShader(GL_GEOMETRY_SHADER);
    #else
        GPU_PushErrorCode("GPU_CompileShader", GPU_ERROR_BACKEND_ERROR, "Hardware does not support GPU_GEOMETRY_SHADER.");
        snprintf(shader_message, 256, "Failed to create geometry shader object.\n");
        return 0;
    #endif
        break;
    }

    if(shader_object == 0)
    {
        GPU_PushErrorCode("GPU_CompileShader", GPU_ERROR_BACKEND_ERROR, "Failed to create new shader object");
        snprintf(shader_message, 256, "Failed to create new shader object.\n");
        return 0;
    }

	glShaderSource(shader_object, 1, &shader_source, NULL);

    // Compile the shader source

	glCompileShader(shader_object);

    glGetShaderiv(shader_object, GL_COMPILE_STATUS, &compiled);
    if(!compiled)
    {
        GPU_PushErrorCode("GPU_CompileShader", GPU_ERROR_DATA_ERROR, "Failed to compile shader source");
        glGetShaderInfoLog(shader_object, 256, NULL, shader_message);
        glDeleteShader(shader_object);
        return 0;
    }

    #endif

    return shader_object;
}


static Uint32 CompileShader_RW(GPU_Renderer* renderer, GPU_ShaderEnum shader_type, SDL_RWops* shader_source, Uint8 free_rwops)
{
    // Read in the shader source code
    Uint32 size = GetShaderSourceSize_RW(shader_source);
    char* source_string = (char*)SDL_malloc(size+1);
    int result = GetShaderSource_RW(shader_source, source_string);
	Uint32 result2;
	(void)renderer;

    if(free_rwops)
        SDL_RWclose(shader_source);
    
    if(!result)
    {
        GPU_PushErrorCode("GPU_CompileShader", GPU_ERROR_DATA_ERROR, "Failed to read shader source");
        snprintf(shader_message, 256, "Failed to read shader source.\n");
        SDL_free(source_string);
        return 0;
    }

    result2 = compile_shader_source(shader_type, source_string);
    SDL_free(source_string);

    return result2;
}

static Uint32 CompileShader(GPU_Renderer* renderer, GPU_ShaderEnum shader_type, const char* shader_source)
{
    Uint32 size = (Uint32)strlen(shader_source);
	SDL_RWops* rwops;
    if(size == 0)
        return 0;
    rwops = SDL_RWFromConstMem(shader_source, size);
    return renderer->impl->CompileShader_RW(renderer, shader_type, rwops, 1);
}

static Uint32 CreateShaderProgram(GPU_Renderer* renderer)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    GLuint p;

    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return 0;

    p = glCreateProgram();

    return p;
	#else
	(void)renderer;
	return 0;
	#endif
}

static Uint8 LinkShaderProgram(GPU_Renderer* renderer, Uint32 program_object)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
	int linked;

    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return 0;

	glLinkProgram(program_object);

	glGetProgramiv(program_object, GL_LINK_STATUS, &linked);

	if(!linked)
    {
        GPU_PushErrorCode("GPU_LinkShaderProgram", GPU_ERROR_BACKEND_ERROR, "Failed to link shader program");
        glGetProgramInfoLog(program_object, 256, NULL, shader_message);
        glDeleteProgram(program_object);
        return 0;
    }

	return 1;

    #else
	(void)renderer;
	(void)program_object;
    return 0;

	#endif
}

static void FreeShader(GPU_Renderer* renderer, Uint32 shader_object)
{
	(void)renderer;
	(void)shader_object;
    #ifndef SDL_GPU_DISABLE_SHADERS
    if(IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        glDeleteShader(shader_object);
    #endif
}

static void FreeShaderProgram(GPU_Renderer* renderer, Uint32 program_object)
{
	(void)renderer;
	(void)program_object;
    #ifndef SDL_GPU_DISABLE_SHADERS
    if(IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        glDeleteProgram(program_object);
    #endif
}

static void AttachShader(GPU_Renderer* renderer, Uint32 program_object, Uint32 shader_object)
{
	(void)renderer;
	(void)program_object;
	(void)shader_object;
    #ifndef SDL_GPU_DISABLE_SHADERS
    if(IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        glAttachShader(program_object, shader_object);
    #endif
}

static void DetachShader(GPU_Renderer* renderer, Uint32 program_object, Uint32 shader_object)
{
	(void)renderer;
	(void)program_object;
	(void)shader_object;
    #ifndef SDL_GPU_DISABLE_SHADERS
    if(IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        glDetachShader(program_object, shader_object);
    #endif
}

static void ActivateShaderProgram(GPU_Renderer* renderer, Uint32 program_object, GPU_ShaderBlock* block)
{
	GPU_Target* target = renderer->current_context_target;
	(void)block;
    #ifndef SDL_GPU_DISABLE_SHADERS
    if(IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
    {
        if(program_object == 0) // Implies default shader
        {
            // Already using a default shader?
            if(target->context->current_shader_program == target->context->default_textured_shader_program
                || target->context->current_shader_program == target->context->default_untextured_shader_program)
                return;

            program_object = target->context->default_untextured_shader_program;
        }

        renderer->impl->FlushBlitBuffer(renderer);
        glUseProgram(program_object);

		{
			// Set up our shader attribute and uniform locations
			GPU_CONTEXT_DATA* cdata = ((GPU_CONTEXT_DATA*)target->context->data);
			if(block == NULL)
			{
				if(program_object == target->context->default_textured_shader_program)
					cdata->current_shader_block = cdata->shader_block[0];
				else if(program_object == target->context->default_untextured_shader_program)
					cdata->current_shader_block = cdata->shader_block[1];
				else
				{
						GPU_ShaderBlock b;
						b.position_loc = -1;
						b.texcoord_loc = -1;
						b.color_loc = -1;
						b.modelViewProjection_loc = -1;
						cdata->current_shader_block = b;
				}
			}
			else
				cdata->current_shader_block = *block;
		}
    }
    #endif

    target->context->current_shader_program = program_object;
}

static void DeactivateShaderProgram(GPU_Renderer* renderer)
{
    renderer->impl->ActivateShaderProgram(renderer, 0, NULL);
}

static const char* GetShaderMessage(GPU_Renderer* renderer)
{
	(void)renderer;
    return shader_message;
}

static int GetAttributeLocation(GPU_Renderer* renderer, Uint32 program_object, const char* attrib_name)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return -1;
    program_object = get_proper_program_id(renderer, program_object);
    if(program_object == 0)
        return -1;
    return glGetAttribLocation(program_object, attrib_name);
	#else
	(void)renderer;
	(void)program_object;
	(void)attrib_name;
    return -1;
    #endif
}

static int GetUniformLocation(GPU_Renderer* renderer, Uint32 program_object, const char* uniform_name)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return -1;
    program_object = get_proper_program_id(renderer, program_object);
    if(program_object == 0)
        return -1;
    return glGetUniformLocation(program_object, uniform_name);
	#else
	(void)renderer;
	(void)program_object;
	(void)uniform_name;
    return -1;
    #endif
}

static GPU_ShaderBlock LoadShaderBlock(GPU_Renderer* renderer, Uint32 program_object, const char* position_name, const char* texcoord_name, const char* color_name, const char* modelViewMatrix_name)
{
    GPU_ShaderBlock b;
    program_object = get_proper_program_id(renderer, program_object);
    if(program_object == 0 || !IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
    {
        b.position_loc = -1;
        b.texcoord_loc = -1;
        b.color_loc = -1;
        b.modelViewProjection_loc = -1;
        return b;
    }

    if(position_name == NULL)
        b.position_loc = -1;
    else
        b.position_loc = renderer->impl->GetAttributeLocation(renderer, program_object, position_name);

    if(texcoord_name == NULL)
        b.texcoord_loc = -1;
    else
        b.texcoord_loc = renderer->impl->GetAttributeLocation(renderer, program_object, texcoord_name);

    if(color_name == NULL)
        b.color_loc = -1;
    else
        b.color_loc = renderer->impl->GetAttributeLocation(renderer, program_object, color_name);

    if(modelViewMatrix_name == NULL)
        b.modelViewProjection_loc = -1;
    else
        b.modelViewProjection_loc = renderer->impl->GetUniformLocation(renderer, program_object, modelViewMatrix_name);

    return b;
}

static void SetShaderBlock(GPU_Renderer* renderer, GPU_ShaderBlock block)
{
	(void)renderer;
	(void)block;
    #ifndef SDL_GPU_DISABLE_SHADERS
    ((GPU_CONTEXT_DATA*)renderer->current_context_target->context->data)->current_shader_block = block;
    #endif
}

static void SetShaderImage(GPU_Renderer* renderer, GPU_Image* image, int location, int image_unit)
{
    // TODO: OpenGL 1 needs to check for ARB_multitexture to use glActiveTexture().
    #ifndef SDL_GPU_DISABLE_SHADERS
	Uint32 new_texture;

    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;

    renderer->impl->FlushBlitBuffer(renderer);
    if(renderer->current_context_target->context->current_shader_program == 0 || image_unit < 0)
        return;

    new_texture = 0;
    if(image != NULL)
        new_texture = ((GPU_IMAGE_DATA*)image->data)->handle;

    // Set the new image unit
    glUniform1i(location, image_unit);
    glActiveTexture(GL_TEXTURE0 + image_unit);
    glBindTexture(GL_TEXTURE_2D, new_texture);

    if(image_unit != 0)
        glActiveTexture(GL_TEXTURE0);

	#endif

	(void)renderer;
	(void)image;
	(void)location;
	(void)image_unit;
}


static void GetUniformiv(GPU_Renderer* renderer, Uint32 program_object, int location, int* values)
{
	(void)renderer;
	(void)program_object;
	(void)location;
	(void)values;

    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    program_object = get_proper_program_id(renderer, program_object);
    if(program_object != 0)
        glGetUniformiv(program_object, location, values);
    #endif
}

static void SetUniformi(GPU_Renderer* renderer, int location, int value)
{
	(void)renderer;
	(void)location;
	(void)value;

    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    renderer->impl->FlushBlitBuffer(renderer);
    if(renderer->current_context_target->context->current_shader_program == 0)
        return;
    glUniform1i(location, value);
    #endif
}

static void SetUniformiv(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, int* values)
{
	(void)renderer;
	(void)location;
	(void)num_elements_per_value;
	(void)num_values;
	(void)values;

    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    renderer->impl->FlushBlitBuffer(renderer);
    if(renderer->current_context_target->context->current_shader_program == 0)
        return;
    switch(num_elements_per_value)
    {
        case 1:
        glUniform1iv(location, num_values, values);
        break;
        case 2:
        glUniform2iv(location, num_values, values);
        break;
        case 3:
        glUniform3iv(location, num_values, values);
        break;
        case 4:
        glUniform4iv(location, num_values, values);
        break;
    }
    #endif
}


static void GetUniformuiv(GPU_Renderer* renderer, Uint32 program_object, int location, unsigned int* values)
{
	(void)renderer;
	(void)program_object;
	(void)location;
	(void)values;

    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    program_object = get_proper_program_id(renderer, program_object);
    if(program_object != 0)
        #if defined(SDL_GPU_USE_GLES) && SDL_GPU_GLES_MAJOR_VERSION < 3
        glGetUniformiv(program_object, location, (int*)values);
        #else
        glGetUniformuiv(program_object, location, values);
        #endif
    #endif
}

static void SetUniformui(GPU_Renderer* renderer, int location, unsigned int value)
{
	(void)renderer;
	(void)location;
	(void)value;

    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    renderer->impl->FlushBlitBuffer(renderer);
    if(renderer->current_context_target->context->current_shader_program == 0)
        return;
    #if defined(SDL_GPU_USE_GLES) && SDL_GPU_GLES_MAJOR_VERSION < 3
    glUniform1i(location, (int)value);
    #else
    glUniform1ui(location, value);
    #endif
    #endif
}

static void SetUniformuiv(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, unsigned int* values)
{
	(void)renderer;
	(void)location;
	(void)num_elements_per_value;
	(void)num_values;
	(void)values;

    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    renderer->impl->FlushBlitBuffer(renderer);
    if(renderer->current_context_target->context->current_shader_program == 0)
        return;
    #if defined(SDL_GPU_USE_GLES) && SDL_GPU_GLES_MAJOR_VERSION < 3
    switch(num_elements_per_value)
    {
        case 1:
        glUniform1iv(location, num_values, (int*)values);
        break;
        case 2:
        glUniform2iv(location, num_values, (int*)values);
        break;
        case 3:
        glUniform3iv(location, num_values, (int*)values);
        break;
        case 4:
        glUniform4iv(location, num_values, (int*)values);
        break;
    }
    #else
    switch(num_elements_per_value)
    {
        case 1:
        glUniform1uiv(location, num_values, values);
        break;
        case 2:
        glUniform2uiv(location, num_values, values);
        break;
        case 3:
        glUniform3uiv(location, num_values, values);
        break;
        case 4:
        glUniform4uiv(location, num_values, values);
        break;
    }
    #endif
    #endif
}


static void GetUniformfv(GPU_Renderer* renderer, Uint32 program_object, int location, float* values)
{
	(void)renderer;
	(void)program_object;
	(void)location;
	(void)values;

    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    program_object = get_proper_program_id(renderer, program_object);
    if(program_object != 0)
        glGetUniformfv(program_object, location, values);
    #endif
}

static void SetUniformf(GPU_Renderer* renderer, int location, float value)
{
	(void)renderer;
	(void)location;
	(void)value;

    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    renderer->impl->FlushBlitBuffer(renderer);
    if(renderer->current_context_target->context->current_shader_program == 0)
        return;
    glUniform1f(location, value);
    #endif
}

static void SetUniformfv(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, float* values)
{
	(void)renderer;
	(void)location;
	(void)num_elements_per_value;
	(void)num_values;
	(void)values;

    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    renderer->impl->FlushBlitBuffer(renderer);
    if(renderer->current_context_target->context->current_shader_program == 0)
        return;
    switch(num_elements_per_value)
    {
        case 1:
        glUniform1fv(location, num_values, values);
        break;
        case 2:
        glUniform2fv(location, num_values, values);
        break;
        case 3:
        glUniform3fv(location, num_values, values);
        break;
        case 4:
        glUniform4fv(location, num_values, values);
        break;
    }
    #endif
}

static void SetUniformMatrixfv(GPU_Renderer* renderer, int location, int num_matrices, int num_rows, int num_columns, Uint8 transpose, float* values)
{
	(void)renderer;
	(void)location;
	(void)num_matrices;
	(void)num_rows;
	(void)num_columns;
	(void)transpose;
	(void)values;

    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    renderer->impl->FlushBlitBuffer(renderer);
    if(renderer->current_context_target->context->current_shader_program == 0)
        return;
    if(num_rows < 2 || num_rows > 4 || num_columns < 2 || num_columns > 4)
    {
        GPU_PushErrorCode("GPU_SetUniformMatrixfv", GPU_ERROR_DATA_ERROR, "Given invalid dimensions (%dx%d)", num_rows, num_columns);
        return;
    }
    #if defined(SDL_GPU_USE_GLES)
    // Hide these symbols so it compiles, but make sure they never get called because GLES only supports square matrices.
    #define glUniformMatrix2x3fv glUniformMatrix2fv
    #define glUniformMatrix2x4fv glUniformMatrix2fv
    #define glUniformMatrix3x2fv glUniformMatrix2fv
    #define glUniformMatrix3x4fv glUniformMatrix2fv
    #define glUniformMatrix4x2fv glUniformMatrix2fv
    #define glUniformMatrix4x3fv glUniformMatrix2fv
    if(num_rows != num_columns)
    {
        GPU_PushErrorCode("GPU_SetUniformMatrixfv", GPU_ERROR_DATA_ERROR, "GLES renderers do not accept non-square matrices (given %dx%d)", num_rows, num_columns);
        return;
    }
    #endif

    switch(num_rows)
    {
    case 2:
        if(num_columns == 2)
            glUniformMatrix2fv(location, num_matrices, transpose, values);
        else if(num_columns == 3)
            glUniformMatrix2x3fv(location, num_matrices, transpose, values);
        else if(num_columns == 4)
            glUniformMatrix2x4fv(location, num_matrices, transpose, values);
        break;
    case 3:
        if(num_columns == 2)
            glUniformMatrix3x2fv(location, num_matrices, transpose, values);
        else if(num_columns == 3)
            glUniformMatrix3fv(location, num_matrices, transpose, values);
        else if(num_columns == 4)
            glUniformMatrix3x4fv(location, num_matrices, transpose, values);
        break;
    case 4:
        if(num_columns == 2)
            glUniformMatrix4x2fv(location, num_matrices, transpose, values);
        else if(num_columns == 3)
            glUniformMatrix4x3fv(location, num_matrices, transpose, values);
        else if(num_columns == 4)
            glUniformMatrix4fv(location, num_matrices, transpose, values);
        break;
    }
    #endif
}


static void SetAttributef(GPU_Renderer* renderer, int location, float value)
{
	(void)renderer;
	(void)location;
	(void)value;

    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    renderer->impl->FlushBlitBuffer(renderer);
    if(renderer->current_context_target->context->current_shader_program == 0)
        return;

    #ifdef SDL_GPU_USE_OPENGL
    if(apply_Intel_attrib_workaround && location == 0)
    {
        apply_Intel_attrib_workaround = 0;
        glBegin(GL_TRIANGLES);
        glEnd();
    }
    #endif

    glVertexAttrib1f(location, value);

    #endif
}

static void SetAttributei(GPU_Renderer* renderer, int location, int value)
{
	(void)renderer;
	(void)location;
	(void)value;

    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    renderer->impl->FlushBlitBuffer(renderer);
    if(renderer->current_context_target->context->current_shader_program == 0)
        return;

    #ifdef SDL_GPU_USE_OPENGL
    if(apply_Intel_attrib_workaround && location == 0)
    {
        apply_Intel_attrib_workaround = 0;
        glBegin(GL_TRIANGLES);
        glEnd();
    }
    #endif

    glVertexAttribI1i(location, value);

    #endif
}

static void SetAttributeui(GPU_Renderer* renderer, int location, unsigned int value)
{
	(void)renderer;
	(void)location;
	(void)value;

    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    renderer->impl->FlushBlitBuffer(renderer);
    if(renderer->current_context_target->context->current_shader_program == 0)
        return;

    #ifdef SDL_GPU_USE_OPENGL
    if(apply_Intel_attrib_workaround && location == 0)
    {
        apply_Intel_attrib_workaround = 0;
        glBegin(GL_TRIANGLES);
        glEnd();
    }
    #endif

    glVertexAttribI1ui(location, value);

    #endif
}


static void SetAttributefv(GPU_Renderer* renderer, int location, int num_elements, float* value)
{
	(void)renderer;
	(void)location;
	(void)num_elements;
	(void)value;

    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    renderer->impl->FlushBlitBuffer(renderer);
    if(renderer->current_context_target->context->current_shader_program == 0)
        return;

    #ifdef SDL_GPU_USE_OPENGL
    if(apply_Intel_attrib_workaround && location == 0)
    {
        apply_Intel_attrib_workaround = 0;
        glBegin(GL_TRIANGLES);
        glEnd();
    }
    #endif

    switch(num_elements)
    {
        case 1:
            glVertexAttrib1f(location, value[0]);
            break;
        case 2:
            glVertexAttrib2f(location, value[0], value[1]);
            break;
        case 3:
            glVertexAttrib3f(location, value[0], value[1], value[2]);
            break;
        case 4:
            glVertexAttrib4f(location, value[0], value[1], value[2], value[3]);
            break;
    }

    #endif
}

static void SetAttributeiv(GPU_Renderer* renderer, int location, int num_elements, int* value)
{
	(void)renderer;
	(void)location;
	(void)num_elements;
	(void)value;
    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    renderer->impl->FlushBlitBuffer(renderer);
    if(renderer->current_context_target->context->current_shader_program == 0)
        return;

    #ifdef SDL_GPU_USE_OPENGL
    if(apply_Intel_attrib_workaround && location == 0)
    {
        apply_Intel_attrib_workaround = 0;
        glBegin(GL_TRIANGLES);
        glEnd();
    }
    #endif

    switch(num_elements)
    {
        case 1:
            glVertexAttribI1i(location, value[0]);
            break;
        case 2:
            glVertexAttribI2i(location, value[0], value[1]);
            break;
        case 3:
            glVertexAttribI3i(location, value[0], value[1], value[2]);
            break;
        case 4:
            glVertexAttribI4i(location, value[0], value[1], value[2], value[3]);
            break;
    }

    #endif
}

static void SetAttributeuiv(GPU_Renderer* renderer, int location, int num_elements, unsigned int* value)
{
	(void)renderer;
	(void)location;
	(void)num_elements;
	(void)value;

    #ifndef SDL_GPU_DISABLE_SHADERS
    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    renderer->impl->FlushBlitBuffer(renderer);
    if(renderer->current_context_target->context->current_shader_program == 0)
        return;

    #ifdef SDL_GPU_USE_OPENGL
    if(apply_Intel_attrib_workaround && location == 0)
    {
        apply_Intel_attrib_workaround = 0;
        glBegin(GL_TRIANGLES);
        glEnd();
    }
    #endif

    switch(num_elements)
    {
        case 1:
            glVertexAttribI1ui(location, value[0]);
            break;
        case 2:
            glVertexAttribI2ui(location, value[0], value[1]);
            break;
        case 3:
            glVertexAttribI3ui(location, value[0], value[1], value[2]);
            break;
        case 4:
            glVertexAttribI4ui(location, value[0], value[1], value[2], value[3]);
            break;
    }

    #endif
}

static void SetAttributeSource(GPU_Renderer* renderer, int num_values, GPU_Attribute source)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
	GPU_CONTEXT_DATA* cdata;
	GPU_AttributeSource* a;

    if(!IsFeatureEnabled(renderer, GPU_FEATURE_BASIC_SHADERS))
        return;
    if(source.location < 0 || source.location >= 16)
        return;
    cdata = (GPU_CONTEXT_DATA*)renderer->current_context_target->context->data;
    a = &cdata->shader_attributes[source.location];
    if(source.format.is_per_sprite)
    {
		int needed_size;

        a->per_vertex_storage_offset_bytes = 0;
        a->per_vertex_storage_stride_bytes = source.format.num_elems_per_value * sizeof_GPU_type(source.format.type);
        a->num_values = 4 * num_values;  // 4 vertices now
        needed_size = a->num_values * a->per_vertex_storage_stride_bytes;

        // Make sure we have enough room for converted per-vertex data
        if(a->per_vertex_storage_size < needed_size)
        {
            SDL_free(a->per_vertex_storage);
            a->per_vertex_storage = SDL_malloc(needed_size);
            a->per_vertex_storage_size = needed_size;
        }
    }
    else if(a->per_vertex_storage_size > 0)
    {
        SDL_free(a->per_vertex_storage);
        a->per_vertex_storage = NULL;
        a->per_vertex_storage_size = 0;
    }

    a->enabled = 0;
    a->attribute = source;

    if(!source.format.is_per_sprite)
    {
        a->per_vertex_storage = source.values;
        a->num_values = num_values;
        a->per_vertex_storage_stride_bytes = source.format.stride_bytes;
        a->per_vertex_storage_offset_bytes = source.format.offset_bytes;
    }

    a->next_value = a->per_vertex_storage;

	#endif

	(void)renderer;
	(void)num_values;
	(void)source;
}



#define SET_COMMON_FUNCTIONS(impl) \
    impl->Init = &Init; \
    impl->CreateTargetFromWindow = &CreateTargetFromWindow; \
    impl->CreateAliasTarget = &CreateAliasTarget; \
    impl->MakeCurrent = &MakeCurrent; \
    impl->SetAsCurrent = &SetAsCurrent; \
    impl->ResetRendererState = &ResetRendererState; \
    impl->SetWindowResolution = &SetWindowResolution; \
    impl->SetVirtualResolution = &SetVirtualResolution; \
    impl->UnsetVirtualResolution = &UnsetVirtualResolution; \
    impl->Quit = &Quit; \
 \
    impl->SetFullscreen = &SetFullscreen; \
    impl->SetCamera = &SetCamera; \
 \
    impl->CreateImage = &CreateImage; \
    impl->CreateImageUsingTexture = &CreateImageUsingTexture; \
    impl->LoadImage = &LoadImage; \
    impl->CreateAliasImage = &CreateAliasImage; \
    impl->SaveImage = &SaveImage; \
    impl->CopyImage = &CopyImage; \
    impl->UpdateImage = &UpdateImage; \
    impl->UpdateImageBytes = &UpdateImageBytes; \
    impl->ReplaceImage = &ReplaceImage; \
    impl->CopyImageFromSurface = &CopyImageFromSurface; \
    impl->CopyImageFromTarget = &CopyImageFromTarget; \
    impl->CopySurfaceFromTarget = &CopySurfaceFromTarget; \
    impl->CopySurfaceFromImage = &CopySurfaceFromImage; \
    impl->FreeImage = &FreeImage; \
 \
    impl->LoadTarget = &LoadTarget; \
    impl->FreeTarget = &FreeTarget; \
 \
    impl->Blit = &Blit; \
    impl->BlitRotate = &BlitRotate; \
    impl->BlitScale = &BlitScale; \
    impl->BlitTransform = &BlitTransform; \
    impl->BlitTransformX = &BlitTransformX; \
    impl->TriangleBatch = &TriangleBatch; \
 \
    impl->GenerateMipmaps = &GenerateMipmaps; \
 \
    impl->SetClip = &SetClip; \
    impl->UnsetClip = &UnsetClip; \
     \
    impl->GetPixel = &GetPixel; \
    impl->SetImageFilter = &SetImageFilter; \
    impl->SetWrapMode = &SetWrapMode; \
 \
    impl->ClearRGBA = &ClearRGBA; \
    impl->FlushBlitBuffer = &FlushBlitBuffer; \
    impl->Flip = &Flip; \
     \
    impl->CompileShader_RW = &CompileShader_RW; \
    impl->CompileShader = &CompileShader; \
    impl->CreateShaderProgram = &CreateShaderProgram; \
    impl->LinkShaderProgram = &LinkShaderProgram; \
    impl->FreeShader = &FreeShader; \
    impl->FreeShaderProgram = &FreeShaderProgram; \
    impl->AttachShader = &AttachShader; \
    impl->DetachShader = &DetachShader; \
    impl->ActivateShaderProgram = &ActivateShaderProgram; \
    impl->DeactivateShaderProgram = &DeactivateShaderProgram; \
    impl->GetShaderMessage = &GetShaderMessage; \
    impl->GetAttributeLocation = &GetAttributeLocation; \
    impl->GetUniformLocation = &GetUniformLocation; \
    impl->LoadShaderBlock = &LoadShaderBlock; \
    impl->SetShaderBlock = &SetShaderBlock; \
    impl->SetShaderImage = &SetShaderImage; \
    impl->GetUniformiv = &GetUniformiv; \
    impl->SetUniformi = &SetUniformi; \
    impl->SetUniformiv = &SetUniformiv; \
    impl->GetUniformuiv = &GetUniformuiv; \
    impl->SetUniformui = &SetUniformui; \
    impl->SetUniformuiv = &SetUniformuiv; \
    impl->GetUniformfv = &GetUniformfv; \
    impl->SetUniformf = &SetUniformf; \
    impl->SetUniformfv = &SetUniformfv; \
    impl->SetUniformMatrixfv = &SetUniformMatrixfv; \
    impl->SetAttributef = &SetAttributef; \
    impl->SetAttributei = &SetAttributei; \
    impl->SetAttributeui = &SetAttributeui; \
    impl->SetAttributefv = &SetAttributefv; \
    impl->SetAttributeiv = &SetAttributeiv; \
    impl->SetAttributeuiv = &SetAttributeuiv; \
    impl->SetAttributeSource = &SetAttributeSource; \
	 \
	/* Shape rendering */ \
	 \
    impl->SetLineThickness = &SetLineThickness; \
    impl->GetLineThickness = &GetLineThickness; \
    impl->Pixel = &Pixel; \
    impl->Line = &Line; \
    impl->Arc = &Arc; \
    impl->ArcFilled = &ArcFilled; \
    impl->Circle = &Circle; \
    impl->CircleFilled = &CircleFilled; \
    impl->Ellipse = &Ellipse; \
    impl->EllipseFilled = &EllipseFilled; \
    impl->Sector = &Sector; \
    impl->SectorFilled = &SectorFilled; \
    impl->Tri = &Tri; \
    impl->TriFilled = &TriFilled; \
    impl->Rectangle = &Rectangle; \
    impl->RectangleFilled = &RectangleFilled; \
    impl->RectangleRound = &RectangleRound; \
    impl->RectangleRoundFilled = &RectangleRoundFilled; \
    impl->Polygon = &Polygon; \
    impl->PolygonFilled = &PolygonFilled;

