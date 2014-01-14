/* This is an implementation file to be included after certain #defines have been set.
These defines determine the code paths:
SDL_GPU_USE_OPENGL   // "Desktop" OpenGL
SDL_GPU_USE_GLES // "Embedded" OpenGL
SDL_GPU_USE_GL_TIER1 // Fixed-function, glBegin, etc.
SDL_GPU_USE_GL_TIER2 // Fixed-function, glDrawArrays, etc.
SDL_GPU_USE_GL_TIER3 // Shader pipeline, manual transforms
CONTEXT_DATA  // Appropriate type for the context data (via pointer)
IMAGE_DATA  // Appropriate type for the image data (via pointer)
TARGET_DATA  // Appropriate type for the target data (via pointer)
*/


#include "SDL_gpu_GL_matrix.h"
#include "SDL_platform.h"

#include "stb_image.h"
#include "stb_image_write.h"


// Forces a flush when vertex limit is reached (roughly 1000 sprites)
#define GPU_BLIT_BUFFER_VERTICES_PER_SPRITE 4
#define GPU_BLIT_BUFFER_INIT_MAX_NUM_VERTICES (GPU_BLIT_BUFFER_VERTICES_PER_SPRITE*1000)

#ifndef SDL_GPU_USE_GL_TIER3
// x, y, z, s, t
#define GPU_BLIT_BUFFER_FLOATS_PER_VERTEX 5
#else
// x, y, z, s, t, r, g, b, a
#define GPU_BLIT_BUFFER_FLOATS_PER_VERTEX 9
#endif

// bytes per vertex
#define GPU_BLIT_BUFFER_STRIDE (sizeof(float)*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX)
#define GPU_BLIT_BUFFER_VERTEX_OFFSET 0
#define GPU_BLIT_BUFFER_TEX_COORD_OFFSET 3
#define GPU_BLIT_BUFFER_COLOR_OFFSET 5


#include <math.h>
#include <string.h>
#include <strings.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


#ifdef SDL_GPU_USE_SDL2
    #define GET_ALPHA(sdl_color) (sdl_color.a)
#else
    #define GET_ALPHA(sdl_color) (sdl_color.unused)
#endif


#ifndef GL_VERTEX_SHADER
    #ifndef SDL_GPU_DISABLE_SHADERS
        #define SDL_GPU_DISABLE_SHADERS
    #endif
#endif


// Default shaders
#ifndef SDL_GPU_USE_GL_TIER3

#define TEXTURED_VERTEX_SHADER_SOURCE \
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

#define UNTEXTURED_VERTEX_SHADER_SOURCE \
"#version 120\n\
\
varying vec4 color;\n\
\
void main(void)\n\
{\n\
	color = gl_Color;\n\
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n\
}"


#define TEXTURED_FRAGMENT_SHADER_SOURCE \
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

#define UNTEXTURED_FRAGMENT_SHADER_SOURCE \
"#version 120\n\
\
varying vec4 color;\n\
\
void main(void)\n\
{\n\
    gl_FragColor = color;\n\
}"

#else
// Tier 3 uses shader attributes to send position, texcoord, and color data for each vertex.
#ifndef SDL_GPU_USE_GLES
#define TEXTURED_VERTEX_SHADER_SOURCE \
"#version 130\n\
\
attribute vec3 gpu_Vertex;\n\
attribute vec2 gpu_TexCoord;\n\
attribute vec4 gpu_Color;\n\
uniform mat4 modelViewProjection;\n\
\
varying vec4 color;\n\
varying vec2 texCoord;\n\
\
void main(void)\n\
{\n\
	color = gpu_Color;\n\
	texCoord = vec2(gpu_TexCoord);\n\
	gl_Position = modelViewProjection * vec4(gpu_Vertex, 1.0);\n\
}"

// Tier 3 uses shader attributes to send position, texcoord, and color data for each vertex.
#define UNTEXTURED_VERTEX_SHADER_SOURCE \
"#version 130\n\
\
attribute vec3 gpu_Vertex;\n\
attribute vec4 gpu_Color;\n\
uniform mat4 modelViewProjection;\n\
\
varying vec4 color;\n\
\
void main(void)\n\
{\n\
	color = gpu_Color;\n\
	gl_Position = modelViewProjection * vec4(gpu_Vertex, 1.0);\n\
}"


#define TEXTURED_FRAGMENT_SHADER_SOURCE \
"#version 130\n\
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

#define UNTEXTURED_FRAGMENT_SHADER_SOURCE \
"#version 130\n\
\
varying vec4 color;\n\
\
void main(void)\n\
{\n\
    gl_FragColor = color;\n\
}"
#else
// GLES
#define TEXTURED_VERTEX_SHADER_SOURCE \
"#version 100\n\
precision mediump float;\n\
precision mediump int;\n\
\
attribute vec3 gpu_Vertex;\n\
attribute vec2 gpu_TexCoord;\n\
attribute vec4 gpu_Color;\n\
uniform mat4 modelViewProjection;\n\
\
varying vec4 color;\n\
varying vec2 texCoord;\n\
\
void main(void)\n\
{\n\
	color = gpu_Color;\n\
	texCoord = vec2(gpu_TexCoord);\n\
	gl_Position = modelViewProjection * vec4(gpu_Vertex, 1.0);\n\
}"

// Tier 3 uses shader attributes to send position, texcoord, and color data for each vertex.
#define UNTEXTURED_VERTEX_SHADER_SOURCE \
"#version 100\n\
precision mediump float;\n\
precision mediump int;\n\
\
attribute vec3 gpu_Vertex;\n\
attribute vec4 gpu_Color;\n\
uniform mat4 modelViewProjection;\n\
\
varying vec4 color;\n\
\
void main(void)\n\
{\n\
	color = gpu_Color;\n\
	gl_Position = modelViewProjection * vec4(gpu_Vertex, 1.0);\n\
}"


#define TEXTURED_FRAGMENT_SHADER_SOURCE \
"#version 100\n\
precision mediump float;\n\
precision mediump int;\n\
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

#define UNTEXTURED_FRAGMENT_SHADER_SOURCE \
"#version 100\n\
precision mediump float;\n\
precision mediump int;\n\
\
varying vec4 color;\n\
\
void main(void)\n\
{\n\
    gl_FragColor = color;\n\
}"

#endif

#endif




static SDL_PixelFormat* AllocFormat(GLenum glFormat);
static void FreeFormat(SDL_PixelFormat* format);

// To make these public, I need to move them into the renderer.  But should they be?
static float* GPU_GetModelView(void)
{
    #ifdef SDL_GPU_USE_INTERNAL_MATRICES
    return _GPU_GetModelView();
    #else
    static float A[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, A);
    return A;
    #endif
}

static float* GPU_GetProjection(void)
{
    #ifdef SDL_GPU_USE_INTERNAL_MATRICES
    return _GPU_GetProjection();
    #else
    static float A[16];
    glGetFloatv(GL_PROJECTION_MATRIX, A);
    return A;
    #endif
}


static Uint8 isExtensionSupported(const char* extension_str)
{
#ifdef SDL_GPU_USE_OPENGL
    return glewIsExtensionSupported(extension_str);
#else
    // As suggested by Mesa3D.org
    char* p = (char*)glGetString(GL_EXTENSIONS);
    char* end;
    int extNameLen;

    extNameLen = strlen(extension_str);
    end = p + strlen(p);

    while(p < end)
    {
        int n = strcspn(p, " ");
        if((extNameLen == n) && (strncmp(extension_str, p, n) == 0))
            return 1;
        
        p += (n + 1);
    }
    return 0;
#endif
}

static Uint8 checkExtension(const char* str)
{
    if(!isExtensionSupported(str))
    {
        GPU_LogError("GL error: %s is not supported.\n", str);
        return 0;
    }
    return 1;
}

static void init_features(GPU_Renderer* renderer)
{
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
#elif defined(SDL_GPU_USE_GLES)
    if(isExtensionSupported("GL_OES_blend_subtract"))
        renderer->enabled_features |= GPU_FEATURE_BLEND_EQUATIONS;
    else
        renderer->enabled_features &= ~GPU_FEATURE_BLEND_EQUATIONS;
    if(isExtensionSupported("GL_OES_blend_func_separate"))
        renderer->enabled_features |= GPU_FEATURE_BLEND_FUNC_SEPARATE;
    else
        renderer->enabled_features &= ~GPU_FEATURE_BLEND_FUNC_SEPARATE;
#endif

    // GL texture formats
    if(isExtensionSupported("GL_EXT_bgr"))
        renderer->enabled_features |= GPU_FEATURE_GL_BGR;
    if(isExtensionSupported("GL_EXT_bgra"))
        renderer->enabled_features |= GPU_FEATURE_GL_BGRA;
    if(isExtensionSupported("GL_EXT_abgr"))
        renderer->enabled_features |= GPU_FEATURE_GL_ABGR;

    if(isExtensionSupported("GL_ARB_fragment_shader"))
        renderer->enabled_features |= GPU_FEATURE_FRAGMENT_SHADER;
    if(isExtensionSupported("GL_ARB_vertex_shader"))
        renderer->enabled_features |= GPU_FEATURE_VERTEX_SHADER;
    if(isExtensionSupported("GL_ARB_geometry_shader4"))
        renderer->enabled_features |= GPU_FEATURE_GEOMETRY_SHADER;
}

static void extBindFramebuffer(GPU_Renderer* renderer, GLuint handle)
{
    if(renderer->enabled_features & GPU_FEATURE_RENDER_TARGETS)
        glBindFramebuffer(GL_FRAMEBUFFER, handle);
}


static inline Uint8 isPowerOfTwo(unsigned int x)
{
    return ((x != 0) && !(x & (x - 1)));
}

static inline unsigned int getNearestPowerOf2(unsigned int n)
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
    if(image != ((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image)
    {
        GLuint handle = ((IMAGE_DATA*)image->data)->handle;
        renderer->FlushBlitBuffer(renderer);

        glBindTexture( GL_TEXTURE_2D, handle );
        ((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image = image;
    }
}

static inline void flushAndBindTexture(GPU_Renderer* renderer, GLuint handle)
{
    // Bind the texture to which subsequent calls refer
    renderer->FlushBlitBuffer(renderer);

    glBindTexture( GL_TEXTURE_2D, handle );
    ((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image = NULL;
}

// Returns false if it can't be bound
static Uint8 bindFramebuffer(GPU_Renderer* renderer, GPU_Target* target)
{
    if(renderer->enabled_features & GPU_FEATURE_RENDER_TARGETS)
    {
        // Bind the FBO
        if(target != ((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target)
        {
            GLuint handle = 0;
            if(target != NULL)
                handle = ((TARGET_DATA*)target->data)->handle;
            renderer->FlushBlitBuffer(renderer);

            extBindFramebuffer(renderer, handle);
            ((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target = target;
        }
        return 1;
    }
    else
    {
        return (target != NULL && ((TARGET_DATA*)target->data)->handle == 0);
    }
}

static inline void flushAndBindFramebuffer(GPU_Renderer* renderer, GLuint handle)
{
    // Bind the FBO
    renderer->FlushBlitBuffer(renderer);

    extBindFramebuffer(renderer, handle);
    ((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target = NULL;
}

static inline void flushBlitBufferIfCurrentTexture(GPU_Renderer* renderer, GPU_Image* image)
{
    if(image == ((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image)
    {
        renderer->FlushBlitBuffer(renderer);
    }
}

static inline void flushAndClearBlitBufferIfCurrentTexture(GPU_Renderer* renderer, GPU_Image* image)
{
    if(image == ((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image)
    {
        renderer->FlushBlitBuffer(renderer);
        ((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image = NULL;
    }
}

static inline Uint8 isCurrentTarget(GPU_Renderer* renderer, GPU_Target* target)
{
    return (target == ((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target
            || ((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target == NULL);
}

static inline void flushAndClearBlitBufferIfCurrentFramebuffer(GPU_Renderer* renderer, GPU_Target* target)
{
    if(target == ((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target
            || ((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target == NULL)
    {
        renderer->FlushBlitBuffer(renderer);
        ((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target = NULL;
    }
}


// Only for window targets, which have their own contexts.
static void makeContextCurrent(GPU_Renderer* renderer, GPU_Target* target)
{
    if(target == NULL || target->context == NULL || renderer->current_context_target == target)
        return;
    
    renderer->FlushBlitBuffer(renderer);
    
    #ifdef SDL_GPU_USE_SDL2
    SDL_GL_MakeCurrent(SDL_GetWindowFromID(target->context->windowID), target->context->context);
    renderer->current_context_target = target;
    #endif
}

static void setClipRect(GPU_Renderer* renderer, GPU_Target* target)
{
    if(target->use_clip_rect)
    {
        glEnable(GL_SCISSOR_TEST);
        GPU_Target* context_target = renderer->current_context_target;
        int y = (target->context != NULL? context_target->h - (target->clip_rect.y + target->clip_rect.h) : target->clip_rect.y);
        float xFactor = ((float)context_target->context->window_w)/context_target->w;
        float yFactor = ((float)context_target->context->window_h)/context_target->h;
        glScissor(target->clip_rect.x * xFactor, y * yFactor, target->clip_rect.w * xFactor, target->clip_rect.h * yFactor);
    }
}

static void unsetClipRect(GPU_Renderer* renderer, GPU_Target* target)
{
    if(target->use_clip_rect)
        glDisable(GL_SCISSOR_TEST);
}

static void prepareToRenderToTarget(GPU_Renderer* renderer, GPU_Target* target)
{
    // Set up the camera
    renderer->SetCamera(renderer, target, &target->camera);
    
    setClipRect(renderer, target);
}



static void changeColor(GPU_Renderer* renderer, SDL_Color color)
{
    #ifdef SDL_GPU_USE_GL_TIER3
    return;
    #else
    CONTEXT_DATA* cdata = (CONTEXT_DATA*)renderer->current_context_target->context->data;
    if(cdata->last_color.r != color.r
        || cdata->last_color.g != color.g
        || cdata->last_color.b != color.b
        || GET_ALPHA(cdata->last_color) != GET_ALPHA(color))
    {
        renderer->FlushBlitBuffer(renderer);
        cdata->last_color = color;
        glColor4f(color.r/255.01f, color.g/255.01f, color.b/255.01f, color.a/255.01f);
    }
    #endif
}

static void changeBlending(GPU_Renderer* renderer, Uint8 enable)
{
    CONTEXT_DATA* cdata = (CONTEXT_DATA*)renderer->current_context_target->context->data;
    if(cdata->last_use_blending == enable)
        return;
    
    renderer->FlushBlitBuffer(renderer);

    if(enable)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);

    cdata->last_use_blending = enable;
}

static void changeBlendMode(GPU_Renderer* renderer, GPU_BlendEnum mode)
{
    CONTEXT_DATA* cdata = (CONTEXT_DATA*)renderer->current_context_target->context->data;
    if(cdata->last_blend_mode != GPU_BLEND_OVERRIDE && cdata->last_blend_mode == mode)
        return;
    
    renderer->FlushBlitBuffer(renderer);

    cdata->last_blend_mode = mode;
    
    if(mode == GPU_BLEND_NORMAL)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if(!(renderer->enabled_features & GPU_FEATURE_BLEND_EQUATIONS))
            return;  // TODO: Return false so we can avoid depending on it if it fails
        glBlendEquation(GL_FUNC_ADD);
    }
    else if(mode == GPU_BLEND_MULTIPLY)
    {
        if(!(renderer->enabled_features & GPU_FEATURE_BLEND_FUNC_SEPARATE))
            return;
        glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if(!(renderer->enabled_features & GPU_FEATURE_BLEND_EQUATIONS))
            return;
        glBlendEquation(GL_FUNC_ADD);
    }
    else if(mode == GPU_BLEND_ADD)
    {
        glBlendFunc(GL_ONE, GL_ONE);
        if(!(renderer->enabled_features & GPU_FEATURE_BLEND_EQUATIONS))
            return;
        glBlendEquation(GL_FUNC_ADD);
    }
    else if(mode == GPU_BLEND_SUBTRACT)
    {
        if(!(renderer->enabled_features & GPU_FEATURE_BLEND_EQUATIONS))
            return;
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_SUBTRACT);
    }
    else if(mode == GPU_BLEND_ADD_COLOR)
    {
        if(!(renderer->enabled_features & GPU_FEATURE_BLEND_FUNC_SEPARATE))
            return;
        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if(!(renderer->enabled_features & GPU_FEATURE_BLEND_EQUATIONS))
            return;
        glBlendEquation(GL_FUNC_ADD);
    }
    else if(mode == GPU_BLEND_SUBTRACT_COLOR)
    {
        if(!(renderer->enabled_features & GPU_FEATURE_BLEND_FUNC_SEPARATE))
            return;
        if(!(renderer->enabled_features & GPU_FEATURE_BLEND_EQUATIONS))
            return;
        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
        glBlendEquation(GL_FUNC_SUBTRACT);
    }
    else if(mode == GPU_BLEND_DIFFERENCE)
    {
        if(!(renderer->enabled_features & GPU_FEATURE_BLEND_FUNC_SEPARATE))
            return;
        if(!(renderer->enabled_features & GPU_FEATURE_BLEND_EQUATIONS))
            return;
        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
        glBlendEquation(GL_FUNC_SUBTRACT);
    }
    else if(mode == GPU_BLEND_PUNCHOUT)
    {
        if(!(renderer->enabled_features & GPU_FEATURE_BLEND_EQUATIONS))
            return;
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
    }
    else if(mode == GPU_BLEND_CUTOUT)
    {
        if(!(renderer->enabled_features & GPU_FEATURE_BLEND_EQUATIONS))
            return;
        glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
        glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
    }
}

#define MIX_COLOR_COMPONENT(a, b) (((a)/255.0f * (b)/255.0f)*255)
#define MIX_COLORS(color1, color2) {MIX_COLOR_COMPONENT(color1.r, color2.r), MIX_COLOR_COMPONENT(color1.g, color2.g), MIX_COLOR_COMPONENT(color1.b, color2.b), MIX_COLOR_COMPONENT(GET_ALPHA(color1), GET_ALPHA(color2))}

static void prepareToRenderImage(GPU_Renderer* renderer, GPU_Target* target, GPU_Image* image)
{
    GPU_Context* context = renderer->current_context_target->context;
    CONTEXT_DATA* cdata = (CONTEXT_DATA*)context->data;
    
    // TODO: Store this state and only call it from FlushBlitBuffer()
    glEnable(GL_TEXTURE_2D);
    
    // Blitting
    if(target->use_color)
    {
        SDL_Color color = MIX_COLORS(target->color, image->color);
        changeColor(renderer, color);
    }
    else
        changeColor(renderer, image->color);
    changeBlending(renderer, image->use_blending);
    changeBlendMode(renderer, image->blend_mode);
    
    // If we're using the untextured shader, switch it.
    if(context->current_shader_program == cdata->default_untextured_shader_program)
        renderer->ActivateShaderProgram(renderer, cdata->default_textured_shader_program, NULL);
}

static void prepareToRenderShapes(GPU_Renderer* renderer)
{
    GPU_Context* context = renderer->current_context_target->context;
    CONTEXT_DATA* cdata = (CONTEXT_DATA*)context->data;
    
    // TODO: Store this state and only call it from FlushBlitBuffer()
    glDisable(GL_TEXTURE_2D);
    
    // Shape rendering
    // Color is set elsewhere for shapes
    changeBlending(renderer, context->shapes_use_blending);
    changeBlendMode(renderer, context->shapes_blend_mode);
    
    // If we're using the textured shader, switch it.
    if(context->current_shader_program == cdata->default_textured_shader_program)
        renderer->ActivateShaderProgram(renderer, cdata->default_untextured_shader_program, NULL);
}



static void applyTargetCamera(GPU_Target* target)
{
    ((CONTEXT_DATA*)(GPU_GetContextTarget()->context->data))->last_camera = target->camera;
    
    GPU_MatrixMode( GPU_PROJECTION );
    GPU_LoadIdentity();

    // The default z for objects is 0
    
    GPU_Ortho(target->camera.x, target->w + target->camera.x, target->h + target->camera.y, target->camera.y, -1.0f, 1.0f);

    GPU_MatrixMode( GPU_MODELVIEW );
    GPU_LoadIdentity();
    //GPU_Translate(0.375f, 0.375f, 0.0f);


    float offsetX = target->w/2.0f;
    float offsetY = target->h/2.0f;
    GPU_Translate(offsetX, offsetY, -0.01);
    GPU_Rotate(target->camera.angle, 0, 0, 1);
    GPU_Translate(-offsetX, -offsetY, 0);

    GPU_Translate(target->camera.x + offsetX, target->camera.y + offsetY, 0);
    GPU_Scale(target->camera.zoom, target->camera.zoom, 1.0f);
    GPU_Translate(-target->camera.x - offsetX, -target->camera.y - offsetY, 0);
}

static GPU_Target* Init(GPU_Renderer* renderer, GPU_RendererID renderer_request, Uint16 w, Uint16 h, Uint32 flags)
{
    // Tell SDL what we want.
#ifdef SDL_GPU_USE_SDL2
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if(renderer_request.major_version < 1)
    {
        renderer_request.major_version = 1;
        renderer_request.minor_version = 1;
    }
    #ifdef SDL_GPU_USE_GLES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    #endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, renderer_request.major_version);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, renderer_request.minor_version);
#else
    Uint8 useDoubleBuffering = flags & SDL_DOUBLEBUF;
    if(useDoubleBuffering)
    {
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        flags &= ~SDL_DOUBLEBUF;
    }
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
    flags |= SDL_OPENGL;
#endif

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

#ifdef SDL_GPU_USE_SDL2

    SDL_Window* window = NULL;
    
    if(renderer->current_context_target != NULL)
    {
        window = SDL_GetWindowFromID(renderer->current_context_target->context->windowID);
    }
    
    if(window == NULL)
    {
        window = SDL_CreateWindow("",
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  w, h,
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

        if(window == NULL)
        {
            GPU_LogError("Window creation failed.\n");
            return NULL;
        }
        
    }

#else
    SDL_Surface* screen = SDL_SetVideoMode(w, h, 0, flags);

    if(screen == NULL)
        return NULL;
#endif
    
    renderer->enabled_features = 0xFFFFFFFF;  // Pretend to support them all if using incompatible headers
    
    
    // Create or re-init the current target.  This also creates the GL context and initializes enabled_features.
    #ifdef SDL_GPU_USE_SDL2
    renderer->CreateTargetFromWindow(renderer, SDL_GetWindowID(window), renderer->current_context_target);
    #else
    renderer->CreateTargetFromWindow(renderer, 0, renderer->current_context_target);
    #endif
    
    // Update our renderer info from the current GL context.
    #ifdef GL_MAJOR_VERSION
    glGetIntegerv(GL_MAJOR_VERSION, &renderer->id.major_version);
    glGetIntegerv(GL_MINOR_VERSION, &renderer->id.minor_version);
    #else
    // GLES doesn't have GL_MAJOR_VERSION.  Check via version string instead.
    const char* version_string = (const char*)glGetString(GL_VERSION);
    // OpenGL ES 2.0?
    if(sscanf(version_string, "OpenGL ES %d.%d", &renderer->id.major_version, &renderer->id.minor_version) <= 0)
    {
        // OpenGL ES-CM 1.1?  OpenGL ES-CL 1.1?
        if(sscanf(version_string, "OpenGL ES-C%*c %d.%d", &renderer->id.major_version, &renderer->id.minor_version) <= 0)
        {
            renderer->id.major_version = SDL_GPU_GLES_MAJOR_VERSION;
            #if SDL_GPU_GLES_MAJOR_VERSION == 1
                renderer->id.minor_version = 1;
            #else
                renderer->id.minor_version = 0;
            #endif
            
            GPU_LogError("Failed to parse OpenGLES version string: %s\n  Defaulting to version %d.%d.\n", version_string, renderer->id.major_version, renderer->id.minor_version);
        }
    }
    #endif
    
    // Did the wrong runtime library try to use a later versioned renderer?
    if(renderer->id.major_version < renderer_request.major_version)
    {
        GPU_LogError("GPU_Init failed: Renderer %s can not be run by the version %d.%d library that is linked.\n", GPU_GetRendererEnumString(renderer_request.id), renderer->id.major_version, renderer->id.minor_version);
        return NULL;
    }
    
    return renderer->current_context_target;
}


static Uint8 IsFeatureEnabled(GPU_Renderer* renderer, GPU_FeatureEnum feature)
{
    return ((renderer->enabled_features & feature) == feature);
}


static GPU_Target* CreateTargetFromWindow(GPU_Renderer* renderer, Uint32 windowID, GPU_Target* target)
{
    Uint8 created = 0;  // Make a new one or repurpose an existing target?
    CONTEXT_DATA* cdata;
    if(target == NULL)
    {
        created = 1;
        target = (GPU_Target*)malloc(sizeof(GPU_Target));
        target->data = (TARGET_DATA*)malloc(sizeof(TARGET_DATA));
        target->image = NULL;
        cdata = (CONTEXT_DATA*)malloc(sizeof(CONTEXT_DATA));
        target->context = (GPU_Context*)malloc(sizeof(GPU_Context));
        target->context->data = cdata;
        target->context->context = NULL;
        
        cdata->last_image = NULL;
        cdata->last_target = NULL;
        // Initialize the blit buffer
        cdata->blit_buffer_max_num_vertices = GPU_BLIT_BUFFER_INIT_MAX_NUM_VERTICES;
        cdata->blit_buffer_num_vertices = 0;
        int blit_buffer_storage_size = GPU_BLIT_BUFFER_INIT_MAX_NUM_VERTICES*GPU_BLIT_BUFFER_STRIDE;
        cdata->blit_buffer = (float*)malloc(blit_buffer_storage_size);
        cdata->index_buffer_max_num_vertices = GPU_BLIT_BUFFER_INIT_MAX_NUM_VERTICES;
        cdata->index_buffer_num_vertices = 0;
        int index_buffer_storage_size = GPU_BLIT_BUFFER_INIT_MAX_NUM_VERTICES*GPU_BLIT_BUFFER_STRIDE;
        cdata->index_buffer = (unsigned short*)malloc(index_buffer_storage_size);
    }
    else
        cdata = (CONTEXT_DATA*)target->context->data;
    
    #ifdef SDL_GPU_USE_SDL2
    
    SDL_Window* window = SDL_GetWindowFromID(windowID);
    if(window == NULL)
    {
        if(created)
        {
            free(cdata->blit_buffer);
            free(cdata->index_buffer);
            free(target->context->data);
            free(target->context);
            free(target->data);
            free(target);
        }
        return NULL;
    }
    
    // Store the window info
    SDL_GetWindowSize(window, &target->context->window_w, &target->context->window_h);
    target->context->windowID = SDL_GetWindowID(window);
    
    // Make a new context if needed and make it current
    if(created || target->context->context == NULL)
    {
        target->context->context = SDL_GL_CreateContext(window);
        renderer->current_context_target = target;
    }
    else
        renderer->MakeCurrent(renderer, target, target->context->windowID);
    
    #else
    
    SDL_Surface* screen = SDL_GetVideoSurface();
    if(screen == NULL)
    {
        if(created)
        {
            free(cdata->blit_buffer);
            free(cdata->index_buffer);
            free(target->context->data);
            free(target->context);
            free(target->data);
            free(target);
        }
        return NULL;
    }
    
    target->context->windowID = 0;
    target->context->window_w = screen->w;
    target->context->window_h = screen->h;
    
    renderer->MakeCurrent(renderer, target, target->context->windowID);
    
    #endif
    

    #ifdef SDL_GPU_USE_OPENGL
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        GPU_LogError("Failed to initialize: %s\n", glewGetErrorString(err));
    }
    #endif

    init_features(renderer);
    
    if(!renderer->IsFeatureEnabled(renderer, GPU_FEATURE_RENDER_TARGETS))
        GPU_LogError("RENDER TARGETS not supported.\n");
    if(!renderer->IsFeatureEnabled(renderer, GPU_FEATURE_NON_POWER_OF_TWO))
        GPU_LogError("NPOT TEXTURES not supported.\n");
    if(!renderer->IsFeatureEnabled(renderer, GPU_FEATURE_BLEND_FUNC_SEPARATE))
        GPU_LogError("BLEND FUNC SEPARATE not supported.\n");
    
    ((TARGET_DATA*)target->data)->handle = 0;
    ((TARGET_DATA*)target->data)->format = GL_RGBA;

    target->renderer = renderer;
    target->w = target->context->window_w;
    target->h = target->context->window_h;

    target->use_clip_rect = 0;
    target->clip_rect.x = 0;
    target->clip_rect.y = 0;
    target->clip_rect.w = target->w;
    target->clip_rect.h = target->h;
    target->use_color = 0;
    
    target->camera = GPU_GetDefaultCamera();
    
    target->context->line_thickness = 1.0f;
    target->context->shapes_use_blending = 1;
    target->context->shapes_blend_mode = GPU_BLEND_NORMAL;
    
    SDL_Color white = {255, 255, 255, 255};
    cdata->z = 0;
    cdata->last_color = white;
    cdata->last_use_blending = 0;
    cdata->last_blend_mode = GPU_BLEND_NORMAL;
    cdata->last_camera = target->camera;  // Redundant due to applyTargetCamera()
    
    // Set up GL state
    
    GPU_InitMatrix();
    
    // Modes
    glEnable( GL_TEXTURE_2D );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_BLEND);
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    // Viewport and Framebuffer
    glViewport( 0, 0, target->context->window_w, target->context->window_h);

    glClear( GL_COLOR_BUFFER_BIT );
    #if SDL_GPU_GL_TIER < 3
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    #endif
    
    // Set up camera
    applyTargetCamera(target);
    
    renderer->SetLineThickness(renderer, 1.0f);
    
    
    cdata->default_textured_shader_program = 0;
    cdata->default_untextured_shader_program = 0;
    target->context->current_shader_program = 0;
    
    #ifndef SDL_GPU_DISABLE_SHADERS
    // Do we need a default shader?
    if(renderer->id.major_version >= 2)
    {
        // Textured shader
        Uint32 v = renderer->CompileShader(renderer, GPU_VERTEX_SHADER, TEXTURED_VERTEX_SHADER_SOURCE);
        
        if(!v)
            GPU_LogError("Failed to load default textured vertex shader: %s\n", renderer->GetShaderMessage(renderer));
        
        Uint32 f = renderer->CompileShader(renderer, GPU_FRAGMENT_SHADER, TEXTURED_FRAGMENT_SHADER_SOURCE);
        
        if(!f)
            GPU_LogError("Failed to load default textured fragment shader: %s\n", renderer->GetShaderMessage(renderer));
        
        Uint32 p = renderer->LinkShaders(renderer, v, f);
        
        if(!p)
            GPU_LogError("Failed to link default textured shader program: %s\n", renderer->GetShaderMessage(renderer));
        
        cdata->default_textured_shader_program = p;
        
        #ifdef SDL_GPU_USE_GL_TIER3
        TARGET_DATA* data = ((TARGET_DATA*)target->data);
        // Get locations of the attributes in the shader
        data->shader_block[0] = GPU_LoadShaderBlock(p, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "modelViewProjection");
        #endif
        
        // Untextured shader
        v = renderer->CompileShader(renderer, GPU_VERTEX_SHADER, UNTEXTURED_VERTEX_SHADER_SOURCE);
        
        if(!v)
            GPU_LogError("Failed to load default untextured vertex shader: %s\n", renderer->GetShaderMessage(renderer));
        
        f = renderer->CompileShader(renderer, GPU_FRAGMENT_SHADER, UNTEXTURED_FRAGMENT_SHADER_SOURCE);
        
        if(!f)
            GPU_LogError("Failed to load default untextured fragment shader: %s\n", renderer->GetShaderMessage(renderer));
        
        p = renderer->LinkShaders(renderer, v, f);
        
        if(!p)
            GPU_LogError("Failed to link default untextured shader program: %s\n", renderer->GetShaderMessage(renderer));
        
        glUseProgram(p);
        
        cdata->default_untextured_shader_program = target->context->current_shader_program = p;
        
        #ifdef SDL_GPU_USE_GL_TIER3
            // Get locations of the attributes in the shader
            data->shader_block[1] = GPU_LoadShaderBlock(p, "gpu_Vertex", NULL, "gpu_Color", "modelViewProjection");
            GPU_SetShaderBlock(data->shader_block[1]);
            
            // Create vertex array container and buffer
            #if !defined(SDL_GPU_USE_GLES) || SDL_GPU_GLES_MAJOR_VERSION != 2
            glGenVertexArrays(1, &data->blit_VAO);
            glBindVertexArray(data->blit_VAO);
            #endif
            
            glGenBuffers(2, data->blit_VBO);
            // Create space on the GPU
            glBindBuffer(GL_ARRAY_BUFFER, data->blit_VBO[0]);
            glBufferData(GL_ARRAY_BUFFER, GPU_BLIT_BUFFER_STRIDE * cdata->blit_buffer_max_num_vertices, NULL, GL_STREAM_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, data->blit_VBO[1]);
            glBufferData(GL_ARRAY_BUFFER, GPU_BLIT_BUFFER_STRIDE * cdata->blit_buffer_max_num_vertices, NULL, GL_STREAM_DRAW);
            data->blit_VBO_flop = 0;
        #endif
    }
    #endif
    
    
    return target;
}

static void MakeCurrent(GPU_Renderer* renderer, GPU_Target* target, Uint32 windowID)
{
    if(target == NULL)
        return;
    #ifdef SDL_GPU_USE_SDL2
    if(target->image != NULL)
        return;
    
    SDL_GLContext c = target->context->context;
    if(c != NULL)
    {
        renderer->current_context_target = target;
        SDL_GL_MakeCurrent(SDL_GetWindowFromID(windowID), c);
        // Reset camera if the target's window was changed
        if(target->context->windowID != windowID)
        {
            renderer->FlushBlitBuffer(renderer);
            target->context->windowID = windowID;
            applyTargetCamera(((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_target);
        }
    }
    #else
    renderer->current_context_target = target;
    #endif
}


static void SetAsCurrent(GPU_Renderer* renderer)
{
    if(renderer->current_context_target == NULL)
        return;
    
    renderer->MakeCurrent(renderer, renderer->current_context_target, renderer->current_context_target->context->windowID);
}

static int SetWindowResolution(GPU_Renderer* renderer, Uint16 w, Uint16 h)
{
    if(renderer->current_context_target == NULL)
        return 0;

#ifdef SDL_GPU_USE_SDL2
    SDL_SetWindowSize(SDL_GetWindowFromID(renderer->current_context_target->context->windowID), w, h);
    SDL_GetWindowSize(SDL_GetWindowFromID(renderer->current_context_target->context->windowID), &renderer->current_context_target->context->window_w, &renderer->current_context_target->context->window_h);
#else
    SDL_Surface* surf = SDL_GetVideoSurface();
    Uint32 flags = surf->flags;


    SDL_Surface* screen = SDL_SetVideoMode(w, h, 0, flags);
    // There's a bug in SDL.  This is a workaround.  Let's resize again:
    screen = SDL_SetVideoMode(w, h, 0, flags);

    if(screen == NULL)
        return 0;

    renderer->current_context_target->window_w = screen->w;
    renderer->current_context_target->window_h = screen->h;
#endif

    Uint16 virtualW = renderer->current_context_target->w;
    Uint16 virtualH = renderer->current_context_target->h;
    
    // FIXME: This might interfere with cameras or be ruined by them.
    glEnable( GL_TEXTURE_2D );
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    glViewport( 0, 0, w, h);

    glClear( GL_COLOR_BUFFER_BIT );

    GPU_MatrixMode( GPU_PROJECTION );
    GPU_LoadIdentity();

    GPU_Ortho(0.0f, virtualW, virtualH, 0.0f, -1.0f, 1.0f);

    GPU_MatrixMode( GPU_MODELVIEW );
    GPU_LoadIdentity();
    
    // Center the pixels
    //GPU_Translate(0.375f, 0.375f, 0.0f);

    // Update display
    GPU_ClearClip(renderer->current_context_target);

    return 1;
}

static void SetVirtualResolution(GPU_Renderer* renderer, GPU_Target* target, Uint16 w, Uint16 h)
{
    if(target == NULL)
        return;

    target->w = w;
    target->h = h;
    
    if(isCurrentTarget(renderer, target))
    {
        renderer->FlushBlitBuffer(renderer);
        applyTargetCamera(target);
    }
}

static void Quit(GPU_Renderer* renderer)
{
    renderer->FreeTarget(renderer, renderer->current_context_target);
    renderer->current_context_target = NULL;
}



static int ToggleFullscreen(GPU_Renderer* renderer)
{
#ifdef SDL_GPU_USE_SDL2
    if(renderer->current_context_target == NULL)
        return 0;
    
    Uint8 enable = !(SDL_GetWindowFlags(SDL_GetWindowFromID(renderer->current_context_target->context->windowID)) & SDL_WINDOW_FULLSCREEN);

    if(SDL_SetWindowFullscreen(SDL_GetWindowFromID(renderer->current_context_target->context->windowID), enable) < 0)
        return 0;

    return 1;
#else
    SDL_Surface* surf = SDL_GetVideoSurface();

    if(SDL_WM_ToggleFullScreen(surf))
        return 1;

    Uint16 w = surf->w;
    Uint16 h = surf->h;
    surf->flags ^= SDL_FULLSCREEN;
    return SetWindowResolution(renderer, w, h);
#endif
}


static GPU_Camera SetCamera(GPU_Renderer* renderer, GPU_Target* target, GPU_Camera* cam)
{
    if(target == NULL || renderer->current_context_target == NULL)
        return GPU_GetDefaultCamera();
    
    
    GPU_Camera result = target->camera;

    if(cam == NULL)
        target->camera = GPU_GetDefaultCamera();
    else
        target->camera = *cam;
    
    if(isCurrentTarget(renderer, target))
    {
        // Change the active camera
        
        // Skip change if the camera is already the same.
        CONTEXT_DATA* cdata = (CONTEXT_DATA*)renderer->current_context_target->context->data;
        if(result.x == cdata->last_camera.x && result.y == cdata->last_camera.y && result.z == cdata->last_camera.z
           && result.angle == cdata->last_camera.angle && result.zoom == cdata->last_camera.zoom)
            return result;

        renderer->FlushBlitBuffer(renderer);
        applyTargetCamera(target);
    }

    return result;
}


static GPU_Image* CreateUninitializedImage(GPU_Renderer* renderer, Uint16 w, Uint16 h, Uint8 channels)
{
    if(channels < 3 || channels > 4)
    {
        GPU_LogError("GPU_CreateUninitializedImage() could not create an image with %d color channels.  Try 3 or 4 instead.\n", channels);
        return NULL;
    }

    GLuint handle;
    GLenum format;
    if(channels == 3)
        format = GL_RGB;
    else
        format = GL_RGBA;

    glGenTextures( 1, &handle );
    if(handle == 0)
    {
        GPU_LogError("GPU_CreateUninitializedImage() failed to generate a texture handle.\n");
        return NULL;
    }

    flushAndBindTexture( renderer, handle );

    // Set the texture's stretching properties
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    #if defined(SDL_GPU_USE_GLES) && (SDL_GPU_GLES_TIER == 1)
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    #endif

    GPU_Image* result = (GPU_Image*)malloc(sizeof(GPU_Image));
    IMAGE_DATA* data = (IMAGE_DATA*)malloc(sizeof(IMAGE_DATA));
    result->target = NULL;
    result->renderer = renderer;
    result->channels = channels;
    SDL_Color white = {255, 255, 255, 255};
    result->color = white;
    result->use_blending = (channels > 3? 1 : 0);
    result->blend_mode = GPU_BLEND_NORMAL;
    result->filter_mode = GPU_LINEAR;
    
    result->data = data;
    result->refcount = 1;
    data->handle = handle;
    data->format = format;
    data->has_mipmaps = 0;

    result->w = w;
    result->h = h;
    // POT textures will change this later
    data->tex_w = w;
    data->tex_h = h;

    return result;
}


static GPU_Image* CreateImage(GPU_Renderer* renderer, Uint16 w, Uint16 h, Uint8 channels)
{
    if(channels < 3 || channels > 4)
    {
        GPU_LogError("GPU_CreateImage() could not create an image with %d color channels.  Try 3 or 4 instead.\n", channels);
        return NULL;
    }

    GPU_Image* result = CreateUninitializedImage(renderer, w, h, channels);

    if(result == NULL)
    {
        GPU_LogError("GPU_CreateImage() could not create %ux%ux%u image.\n", w, h, channels);
        return NULL;
    }

    glEnable(GL_TEXTURE_2D);
    bindTexture(renderer, result);

    GLenum internal_format = ((IMAGE_DATA*)(result->data))->format;
    w = result->w;
    h = result->h;
    if(!(renderer->enabled_features & GPU_FEATURE_NON_POWER_OF_TWO))
    {
        if(!isPowerOfTwo(w))
            w = getNearestPowerOf2(w);
        if(!isPowerOfTwo(h))
            h = getNearestPowerOf2(h);
    }

    // Initialize texture
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0,
                 internal_format, GL_UNSIGNED_BYTE, NULL);
    // Tell SDL_gpu what we got.
    ((IMAGE_DATA*)(result->data))->tex_w = w;
    ((IMAGE_DATA*)(result->data))->tex_h = h;

    return result;
}

static GPU_Image* LoadImage(GPU_Renderer* renderer, const char* filename)
{
    SDL_Surface* surface = GPU_LoadSurface(filename);
    if(surface == NULL)
    {
        GPU_LogError("Failed to load image \"%s\"\n", filename);
        return NULL;
    }

    GPU_Image* result = renderer->CopyImageFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    return result;
}


static Uint8 readTargetPixels(GPU_Renderer* renderer, GPU_Target* source, GLint format, GLubyte* pixels)
{
    if(source == NULL)
        return 0;
    
    if(isCurrentTarget(renderer, source))
        renderer->FlushBlitBuffer(renderer);
    
    if(bindFramebuffer(renderer, source))
    {
        glReadPixels(0, 0, source->w, source->h, format, GL_UNSIGNED_BYTE, pixels);
        return 1;
    }
    return 0;
}

static Uint8 readImagePixels(GPU_Renderer* renderer, GPU_Image* source, GLint format, GLubyte* pixels)
{
    if(source == NULL)
        return 0;
    
    // No glGetTexImage() in OpenGLES
    #ifdef SDL_GPU_USE_GLES
    // Load up the target
    Uint8 created_target = 0;
    if(source->target == NULL)
    {
        renderer->LoadTarget(renderer, source);
        created_target = 1;
    }
    // Get the data
    Uint8 result = readTargetPixels(renderer, source->target, format, pixels);
    // Free the target
    if(created_target)
        renderer->FreeTarget(renderer, source->target);
    return result;
    #else
    // Bind the texture temporarily
    glBindTexture(GL_TEXTURE_2D, ((IMAGE_DATA*)source->data)->handle);
    // Get the data
    glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, pixels);
    // Rebind the last texture
    if(((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image != NULL)
        glBindTexture(GL_TEXTURE_2D, ((IMAGE_DATA*)(((CONTEXT_DATA*)renderer->current_context_target->context->data)->last_image)->data)->handle);
    return 1;
    #endif
}

static unsigned char* getRawTargetData(GPU_Renderer* renderer, GPU_Target* target)
{
    int channels = 4;
    if(target->image != NULL)
        channels = target->image->channels;
    unsigned char* data = (unsigned char*)malloc(target->w * target->h * channels);
    
    if(!readTargetPixels(renderer, target, ((TARGET_DATA*)target->data)->format, data))
    {
        free(data);
        return NULL;
    }
    
    // Flip the data vertically (OpenGL framebuffer is read upside down)
    int pitch = target->w * channels;
    unsigned char* copy = (unsigned char*)malloc(pitch);
    int y;
    for(y = 0; y < target->h/2; y++)
    {
        unsigned char* top = &data[target->w * y * channels];
        unsigned char* bottom = &data[target->w * (target->h - y - 1) * channels];
        memcpy(copy, top, pitch);
        memcpy(top, bottom, pitch);
        memcpy(bottom, copy, pitch);
    }

    return data;
}

static unsigned char* getRawImageData(GPU_Renderer* renderer, GPU_Image* image)
{
    unsigned char* data = (unsigned char*)malloc(image->w * image->h * image->channels);

    if(!readImagePixels(renderer, image, ((IMAGE_DATA*)image->data)->format, data))
    {
        free(data);
        return NULL;
    }

    return data;
}

// From http://stackoverflow.com/questions/5309471/getting-file-extension-in-c
static const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename)
        return "";
    return dot + 1;
}

static Uint8 SaveImage(GPU_Renderer* renderer, GPU_Image* image, const char* filename)
{
    const char* extension;
    Uint8 result;
    unsigned char* data;

    if(image == NULL || filename == NULL ||
            image->w < 1 || image->h < 1 || image->channels < 1 || image->channels > 4)
    {
        return 0;
    }

    extension = get_filename_ext(filename);

    data = getRawImageData(renderer, image);

    if(data == NULL)
    {
        GPU_LogError("GPU_SaveImage() failed: Could not retrieve image data.\n");
        return 0;
    }

    if(SDL_strcasecmp(extension, "png") == 0)
        result = stbi_write_png(filename, image->w, image->h, image->channels, (const unsigned char *const)data, 0);
    else if(SDL_strcasecmp(extension, "bmp") == 0)
        result = stbi_write_bmp(filename, image->w, image->h, image->channels, (void*)data);
    else if(SDL_strcasecmp(extension, "tga") == 0)
        result = stbi_write_tga(filename, image->w, image->h, image->channels, (void*)data);
    //else if(SDL_strcasecmp(extension, "dds") == 0)
    //    result = stbi_write_dds(filename, image->w, image->h, image->channels, (const unsigned char *const)data);
    else
    {
        GPU_LogError("GPU_SaveImage() failed: Unsupported format (%s).\n", extension);
        result = 0;
    }

    free(data);
    return result;
}

static SDL_Surface* CopySurfaceFromTarget(GPU_Renderer* renderer, GPU_Target* target)
{
    unsigned char* data;
    SDL_Surface* result;

    if(target == NULL || target->w < 1 || target->h < 1)
        return NULL;

    data = getRawTargetData(renderer, target);

    if(data == NULL)
    {
        GPU_LogError("GPU_CopySurfaceFromTarget() failed: Could not retrieve target data.\n");
        return NULL;
    }
    
    SDL_PixelFormat* format = AllocFormat(((TARGET_DATA*)target->data)->format);
    
    result = SDL_CreateRGBSurfaceFrom(data, target->w, target->h, format->BitsPerPixel, target->w*format->BytesPerPixel, format->Rmask, format->Gmask, format->Bmask, format->Amask);
    
    FreeFormat(format);
    return result;
}

static SDL_Surface* CopySurfaceFromImage(GPU_Renderer* renderer, GPU_Image* image)
{
    unsigned char* data;
    SDL_Surface* result;

    if(image == NULL || image->w < 1 || image->h < 1)
        return NULL;

    data = getRawImageData(renderer, image);

    if(data == NULL)
    {
        GPU_LogError("GPU_CopySurfaceFromImage() failed: Could not retrieve image data.\n");
        return NULL;
    }
    
    SDL_PixelFormat* format = AllocFormat(((IMAGE_DATA*)image->data)->format);
    
    result = SDL_CreateRGBSurfaceFrom(data, image->w, image->h, format->BitsPerPixel, image->w*format->BytesPerPixel, format->Rmask, format->Gmask, format->Bmask, format->Amask);
    
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
            }
            else
                return 1;
            return 0;
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
            if(surfaceFormatResult != NULL)
                *surfaceFormatResult = GL_BGRA;
            return 0;
        }
#endif
#ifdef GL_ABGR
        if (format->Rmask == 0xFF000000 && format->Gmask == 0x00FF0000 && format->Bmask == 0x0000FF00)
        {
            if(surfaceFormatResult != NULL)
                *surfaceFormatResult = GL_ABGR;
            return 0;
        }
#endif
        return 1;
    default:
        GPU_LogError("GPU_UpdateImage() was passed an image with an invalid format.\n");
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
        GPU_LogError("GPU_UpdateImage() was passed an image with an invalid format.\n");
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

    SDL_PixelFormat* result = (SDL_PixelFormat*)malloc(sizeof(SDL_PixelFormat));
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

static Uint8 hasColorkey(SDL_Surface* surface)
{
#ifdef SDL_GPU_USE_SDL2
    return (SDL_GetColorKey(surface, NULL) == 0);
#else
    return (surface->flags & SDL_SRCCOLORKEY);
#endif
}

static void FreeFormat(SDL_PixelFormat* format)
{
    free(format);
}

// Returns NULL on failure.  Returns the original surface if no copy is needed.  Returns a new surface converted to the right format otherwise.
static SDL_Surface* copySurfaceIfNeeded(GPU_Renderer* renderer, GLenum glFormat, SDL_Surface* surface, GLenum* surfaceFormatResult)
{
    // If format doesn't match, we need to do a copy
    int format_compare = compareFormats(renderer, glFormat, surface, surfaceFormatResult);

    // There's a problem
    if(format_compare < 0)
        return NULL;
    
    #ifdef SDL_GPU_USE_GLES
    // GLES needs a tightly-packed pixel array
    // Based on SDL_UpdateTexture()
    SDL_Surface* newSurface = NULL;
    Uint8 *blob = NULL;
    SDL_Rect rect = {0, 0, surface->w, surface->h};
    int srcPitch = rect.w * surface->format->BytesPerPixel;
    int pitch = surface->pitch;
    if(srcPitch != pitch)
    {
        Uint8 *src;
        Uint8 *pixels = (Uint8*)surface->pixels;
        int y;
        
        /* Bail out if we're supposed to update an empty rectangle */
        if(rect.w <= 0 || rect.h <= 0)
            return NULL;
        
        /* Reformat the texture data into a tightly packed array */
        src = pixels;
        if(pitch != srcPitch)
        {
            blob = (Uint8*)malloc(srcPitch * rect.h);
            if(blob == NULL)
            {
                // Out of memory
                return NULL;
            }
            src = blob;
            for(y = 0; y < rect.h; ++y)
            {
                memcpy(src, pixels, srcPitch);
                src += srcPitch;
                pixels += pitch;
            }
            src = blob;
        }
        
        newSurface = SDL_CreateRGBSurfaceFrom(src, rect.w, rect.h, surface->format->BytesPerPixel, srcPitch, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
    }
    
    // Copy it to a different format
    if(format_compare > 0)
    {
        // Convert to the right format
        SDL_PixelFormat* dst_fmt = AllocFormat(glFormat);
        if(newSurface != NULL)
        {
            surface = SDL_ConvertSurface(newSurface, dst_fmt, 0);
            SDL_FreeSurface(newSurface);
            free(blob);
        }
        else
            surface = SDL_ConvertSurface(surface, dst_fmt, 0);
        FreeFormat(dst_fmt);
        if(surfaceFormatResult != NULL && surface != NULL)
            *surfaceFormatResult = glFormat;
    }
    
    #else
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
    #endif

    // No copy needed
    return surface;
}

// From SDL_UpdateTexture()
static int InitImageWithSurface(GPU_Renderer* renderer, GPU_Image* image, SDL_Surface* surface)
{
    if(renderer == NULL || image == NULL || surface == NULL)
        return 0;

    IMAGE_DATA* data = (IMAGE_DATA*)image->data;

    GLenum internal_format = data->format;
    GLenum original_format = internal_format;

    SDL_Surface* newSurface = copySurfaceIfNeeded(renderer, internal_format, surface, &original_format);
    if(newSurface == NULL)
    {
        GPU_LogError("GPU_InitImageWithSurface() failed to convert surface to proper pixel format.\n");
        return 0;
    }

    Uint8 need_power_of_two_upload = 0;
    unsigned int w = newSurface->w;
    unsigned int h = newSurface->h;
    if(!(renderer->enabled_features & GPU_FEATURE_NON_POWER_OF_TWO))
    {
        if(!isPowerOfTwo(w))
        {
            w = getNearestPowerOf2(w);
            need_power_of_two_upload = 1;
        }
        if(!isPowerOfTwo(h))
        {
            h = getNearestPowerOf2(h);
            need_power_of_two_upload = 1;
        }
    }

    glEnable(GL_TEXTURE_2D);
    bindTexture(renderer, image);
    int alignment = 1;
    if(newSurface->format->BytesPerPixel == 4)
        alignment = 4;

    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    #ifdef SDL_GPU_USE_OPENGL
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (newSurface->pitch / newSurface->format->BytesPerPixel));
    #endif
    if(!need_power_of_two_upload)
    {
        //GPU_LogError("InitImageWithSurface(), Copy? %d, internal: %d, original: %d, GL_RGB: %d, GL_RGBA: %d\n", (newSurface != surface), internal_format, original_format, GL_RGB, GL_RGBA);
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, newSurface->w, newSurface->h, 0,
                     original_format, GL_UNSIGNED_BYTE, newSurface->pixels);
    }
    else
    {
        //GPU_LogError("InitImageWithSurface(), POT upload. Copy? %d, internal: %d, original: %d, GL_RGB: %d, GL_RGBA: %d\n", (newSurface != surface), internal_format, original_format, GL_RGB, GL_RGBA);
        
        // Create POT texture
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0,
                     original_format, GL_UNSIGNED_BYTE, NULL);

        // Upload NPOT data
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, newSurface->w, newSurface->h,
                        original_format, GL_UNSIGNED_BYTE, newSurface->pixels);

        // Tell everyone what we did.
        ((IMAGE_DATA*)(image->data))->tex_w = w;
        ((IMAGE_DATA*)(image->data))->tex_h = h;
    }

    // Delete temporary surface
    if(surface != newSurface)
        SDL_FreeSurface(newSurface);
    return 1;
}

static GPU_Image* CopyImage(GPU_Renderer* renderer, GPU_Image* image)
{
    if(image == NULL)
        return NULL;
    
    GPU_Image* result = CreateUninitializedImage(renderer, image->w, image->h, image->channels);
    if(result == NULL)
        return NULL;
    
    SDL_Surface* surface = renderer->CopySurfaceFromImage(renderer, image);
    if(surface == NULL)
        return NULL;
    
    InitImageWithSurface(renderer, result, surface);
    
    SDL_FreeSurface(surface);
    
    return result;
}



// From SDL_UpdateTexture()
static void UpdateImage(GPU_Renderer* renderer, GPU_Image* image, const GPU_Rect* rect, SDL_Surface* surface)
{
    if(renderer == NULL || image == NULL || surface == NULL)
        return;

    IMAGE_DATA* data = (IMAGE_DATA*)image->data;
    GLenum original_format = data->format;

    SDL_Surface* newSurface = copySurfaceIfNeeded(renderer, data->format, surface, &original_format);
    if(newSurface == NULL)
    {
        GPU_LogError("GPU_UpdateImage() failed to convert surface to proper pixel format.\n");
        return;
    }


    GPU_Rect updateRect;
    if(rect != NULL)
        updateRect = *rect;
    else
    {
        updateRect.x = 0;
        updateRect.y = 0;
        updateRect.w = newSurface->w;
        updateRect.h = newSurface->h;
        if(updateRect.w < 0.0f || updateRect.h < 0.0f)
        {
            GPU_LogError("GPU_UpdateImage(): Given negative rect: %dx%d\n", (int)updateRect.w, (int)updateRect.h);
            return;
        }
    }


    glEnable(GL_TEXTURE_2D);
    if(image->target != NULL && isCurrentTarget(renderer, image->target))
        renderer->FlushBlitBuffer(renderer);
    bindTexture(renderer, image);
    int alignment = 1;
    if(newSurface->format->BytesPerPixel == 4)
        alignment = 4;
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    #ifdef SDL_GPU_USE_OPENGL
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (newSurface->pitch / newSurface->format->BytesPerPixel));
    #endif
    glTexSubImage2D(GL_TEXTURE_2D, 0, updateRect.x, updateRect.y, updateRect.w,
                    updateRect.h, original_format, GL_UNSIGNED_BYTE,
                    newSurface->pixels);

    // Delete temporary surface
    if(surface != newSurface)
        SDL_FreeSurface(newSurface);
}


static inline Uint32 getPixel(SDL_Surface *Surface, int x, int y)
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

// From SDL_CreateTextureFromSurface
static GPU_Image* CopyImageFromSurface(GPU_Renderer* renderer, SDL_Surface* surface)
{
    const SDL_PixelFormat *fmt;
    Uint8 needAlpha;
    GPU_Image* image;
    int channels;

    if(renderer == NULL)
        return NULL;

    if (!surface) {
        GPU_LogError("GPU_CopyImageFromSurface() passed NULL surface.\n");
        return NULL;
    }

    /* See what the best texture format is */
    fmt = surface->format;
    if (fmt->Amask || hasColorkey(surface)) {
        needAlpha = 1;
    } else {
        needAlpha = 0;
    }

    // Get appropriate storage format
    // TODO: More options would be nice...
    if(needAlpha)
    {
        channels = 4;
    }
    else
    {
        channels = 3;
    }
    
    //GPU_LogError("Format...  Channels: %d, BPP: %d, Masks: %X %X %X %X\n", channels, fmt->BytesPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    
    //Uint32 pix = getPixel(surface, surface->w/2, surface->h/2);
    //GPU_LogError("Middle pixel: %X\n", pix);
    image = CreateUninitializedImage(renderer, surface->w, surface->h, channels);
    if(image == NULL)
        return NULL;

    if(SDL_MUSTLOCK(surface))
    {
        SDL_LockSurface(surface);
        InitImageWithSurface(renderer, image, surface);
        SDL_UnlockSurface(surface);
    }
    else
        InitImageWithSurface(renderer, image, surface);

    return image;
}


static GPU_Image* CopyImageFromTarget(GPU_Renderer* renderer, GPU_Target* target)
{
    if(target == NULL)
        return NULL;
    
    SDL_Surface* surface = renderer->CopySurfaceFromTarget(renderer, target);
    GPU_Image* image = renderer->CopyImageFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    return image;
}


static void FreeImage(GPU_Renderer* renderer, GPU_Image* image)
{
    if(image == NULL)
        return;
    
    if(image->refcount > 1)
    {
        image->refcount--;
        return;
    }

    // Delete the attached target first
    if(image->target != NULL)
        renderer->FreeTarget(renderer, image->target);

    flushAndClearBlitBufferIfCurrentTexture(renderer, image);
    glDeleteTextures( 1, &((IMAGE_DATA*)image->data)->handle);
    free(image->data);
    free(image);
}



static void SubSurfaceCopy(GPU_Renderer* renderer, SDL_Surface* src, GPU_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y)
{
    if(renderer == NULL || src == NULL || dest == NULL || dest->image == NULL)
        return;

    if(renderer != dest->renderer)
        return;

    GPU_Rect r;
    if(srcrect != NULL)
        r = *srcrect;
    else
    {
        r.x = 0;
        r.y = 0;
        r.w = src->w;
        r.h = src->h;
        if(r.w < 0.0f || r.h < 0.0f)
        {
            GPU_LogError("GPU_SubSurfaceCopy(): Given negative rectangle: %.2fx%.2f\n", r.w, r.h);
            return;
        }
    }

    bindTexture(renderer, dest->image);

    //GLenum texture_format = GL_RGBA;//((IMAGE_DATA*)image->data)->format;

    SDL_Surface* temp = SDL_CreateRGBSurface(SDL_SWSURFACE, r.w, r.h, src->format->BitsPerPixel, src->format->Rmask, src->format->Gmask, src->format->Bmask, src->format->Amask);

    if(temp == NULL)
    {
        GPU_LogError("GPU_SubSurfaceCopy(): Failed to create new %dx%d RGB surface.\n", (int)r.w, (int)r.h);
        return;
    }

    // Copy data to new surface
#ifdef SDL_GPU_USE_SDL2
    SDL_BlendMode blendmode;
    SDL_GetSurfaceBlendMode(src, &blendmode);
    SDL_SetSurfaceBlendMode(src, SDL_BLENDMODE_NONE);
#else
    Uint32 srcAlpha = src->flags & SDL_SRCALPHA;
    SDL_SetAlpha(src, 0, src->format->alpha);
#endif

    SDL_Rect destrect = {r.x, r.y, r.w, r.h};
    SDL_BlitSurface(src, &destrect, temp, NULL);
    // FIXME: What if destrect does not equal r anymore?

#ifdef SDL_GPU_USE_SDL2
    SDL_SetSurfaceBlendMode(src, blendmode);
#else
    SDL_SetAlpha(src, srcAlpha, src->format->alpha);
#endif

    // Make surface into an image
    GPU_Image* image = GPU_CopyImageFromSurface(temp);
    if(image == NULL)
    {
        GPU_LogError("GPU_SubSurfaceCopy(): Failed to create new image texture.\n");
        return;
    }

    // Copy image to dest
    GPU_FlushBlitBuffer();
    GPU_SetBlending(image, 1);
    GPU_Blit(image, NULL, dest, x + r.w/2, y + r.h/2);
    GPU_FlushBlitBuffer();

    // Using glTexSubImage might be more efficient
    //glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, r.w, r.h, texture_format, GL_UNSIGNED_BYTE, buffer);

    GPU_FreeImage(image);

    SDL_FreeSurface(temp);
}


static GPU_Target* LoadTarget(GPU_Renderer* renderer, GPU_Image* image)
{
    if(renderer == NULL || image == NULL)
        return NULL;

    if(image->target != NULL)
        return image->target;

    if(!(renderer->enabled_features & GPU_FEATURE_RENDER_TARGETS))
        return NULL;

    GLuint handle;
    // Create framebuffer object
    glGenFramebuffers(1, &handle);
    flushAndBindFramebuffer(renderer, handle);

    // Attach the texture to it
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ((IMAGE_DATA*)image->data)->handle, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        return NULL;

    GPU_Target* result = (GPU_Target*)malloc(sizeof(GPU_Target));
    TARGET_DATA* data = (TARGET_DATA*)malloc(sizeof(TARGET_DATA));
    result->data = data;
    data->handle = handle;
    data->format = ((IMAGE_DATA*)image->data)->format;
    
    result->renderer = renderer;
    result->context = NULL;
    result->image = image;
    result->w = image->w;
    result->h = image->h;
    
    result->camera = GPU_GetDefaultCamera();
    
    result->use_clip_rect = 0;
    result->clip_rect.x = 0;
    result->clip_rect.y = 0;
    result->clip_rect.w = image->w;
    result->clip_rect.h = image->h;
    result->use_color = 0;

    image->target = result;
    return result;
}



static void FreeTarget(GPU_Renderer* renderer, GPU_Target* target)
{
    if(target == NULL)
        return;
    if(target == renderer->current_context_target)
    {
        renderer->FlushBlitBuffer(renderer);
        renderer->current_context_target = NULL;
    }

    TARGET_DATA* data = ((TARGET_DATA*)target->data);
    
    if(renderer->enabled_features & GPU_FEATURE_RENDER_TARGETS)
    {
        flushAndClearBlitBufferIfCurrentFramebuffer(renderer, target);
        glDeleteFramebuffers(1, &data->handle);
    }

    if(target->image != NULL)
        target->image->target = NULL;  // Remove reference to this object
    
    if(target->context != NULL)
    {
        CONTEXT_DATA* cdata = (CONTEXT_DATA*)target->context->data;
        
        free(cdata->blit_buffer);
        free(cdata->index_buffer);
    
        #ifdef SDL_GPU_USE_SDL2
        if(target->context->context != 0)
            SDL_GL_DeleteContext(target->context->context);
        #endif
    
        #ifdef SDL_GPU_USE_GL_TIER3
        glDeleteBuffers(2, data->blit_VBO);
            #if !defined(SDL_GPU_USE_GLES) || SDL_GPU_GLES_MAJOR_VERSION != 2
            glDeleteVertexArrays(1, &data->blit_VAO);
            #endif
        #endif
        
        free(target->context->data);
        free(target->context);
        target->context = NULL;
    }
    
    // Free specialized data
    
    free(target->data);
    target->data = NULL;
    free(target);
}



static int Blit(GPU_Renderer* renderer, GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y)
{
    if(src == NULL || dest == NULL)
        return -1;
    if(renderer != src->renderer || renderer != dest->renderer)
        return -2;
    
    makeContextCurrent(renderer, dest);
    if(renderer->current_context_target == NULL)
        return -3;

    // Bind the texture to which subsequent calls refer
    bindTexture(renderer, src);

    // Bind the FBO
    if(bindFramebuffer(renderer, dest))
    {
        prepareToRenderToTarget(renderer, dest);
        prepareToRenderImage(renderer, dest, src);
        
        Uint16 tex_w = ((IMAGE_DATA*)src->data)->tex_w;
        Uint16 tex_h = ((IMAGE_DATA*)src->data)->tex_h;

        float x1, y1, x2, y2;
        float dx1, dy1, dx2, dy2;
        if(srcrect == NULL)
        {
            // Scale tex coords according to actual texture dims
            x1 = 0.1f/tex_w;
            y1 = 0.1f/tex_h;
            x2 = ((float)src->w - 0.1f)/tex_w;
            y2 = ((float)src->h - 0.1f)/tex_h;
            // Center the image on the given coords
            dx1 = x - src->w/2.0f;
            dy1 = y - src->h/2.0f;
            dx2 = x + src->w/2.0f;
            dy2 = y + src->h/2.0f;
        }
        else
        {
            // Scale srcrect tex coords according to actual texture dims
            x1 = (srcrect->x + 0.1f)/(float)tex_w;
            y1 = (srcrect->y + 0.1f)/(float)tex_h;
            x2 = (srcrect->x + srcrect->w - 0.1f)/(float)tex_w;
            y2 = (srcrect->y + srcrect->h - 0.1f)/(float)tex_h;
            // Center the image on the given coords
            dx1 = x - srcrect->w/2.0f;
            dy1 = y - srcrect->h/2.0f;
            dx2 = x + srcrect->w/2.0f;
            dy2 = y + srcrect->h/2.0f;
        }

        CONTEXT_DATA* cdata = (CONTEXT_DATA*)renderer->current_context_target->context->data;
        float* blit_buffer = cdata->blit_buffer;
        unsigned short* index_buffer = cdata->index_buffer;

        if(cdata->blit_buffer_num_vertices + GPU_BLIT_BUFFER_VERTICES_PER_SPRITE >= cdata->blit_buffer_max_num_vertices)
            renderer->FlushBlitBuffer(renderer);

        #ifdef SDL_GPU_USE_GL_TIER3
        int color_index = GPU_BLIT_BUFFER_COLOR_OFFSET + cdata->blit_buffer_num_vertices*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        float r =  src->color.r/255.0f;
        float g =  src->color.g/255.0f;
        float b =  src->color.b/255.0f;
        float a =  GET_ALPHA(src->color)/255.0f;
        #endif
        
        // Sprite quad vertices
        
        // Vertex 0
        int vert_index = GPU_BLIT_BUFFER_VERTEX_OFFSET + cdata->blit_buffer_num_vertices*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        int tex_index = GPU_BLIT_BUFFER_TEX_COORD_OFFSET + cdata->blit_buffer_num_vertices*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        blit_buffer[vert_index] = dx1;
        blit_buffer[vert_index+1] = dy1;
        blit_buffer[vert_index+2] = 0.0f;
        blit_buffer[tex_index] = x1;
        blit_buffer[tex_index+1] = y1;
        #ifdef SDL_GPU_USE_GL_TIER3
        blit_buffer[color_index] = r;
        blit_buffer[color_index+1] = g;
        blit_buffer[color_index+2] = b;
        blit_buffer[color_index+3] = a;
        #endif
        
        // Vertex 1
        vert_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        tex_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        blit_buffer[vert_index] = dx2;
        blit_buffer[vert_index+1] = dy1;
        blit_buffer[vert_index+2] = 0.0f;
        blit_buffer[tex_index] = x2;
        blit_buffer[tex_index+1] = y1;
        #ifdef SDL_GPU_USE_GL_TIER3
        color_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        blit_buffer[color_index] = r;
        blit_buffer[color_index+1] = g;
        blit_buffer[color_index+2] = b;
        blit_buffer[color_index+3] = a;
        #endif
        
        // Vertex 2
        vert_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        tex_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        blit_buffer[vert_index] = dx2;
        blit_buffer[vert_index+1] = dy2;
        blit_buffer[vert_index+2] = 0.0f;
        blit_buffer[tex_index] = x2;
        blit_buffer[tex_index+1] = y2;
        #ifdef SDL_GPU_USE_GL_TIER3
        color_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        blit_buffer[color_index] = r;
        blit_buffer[color_index+1] = g;
        blit_buffer[color_index+2] = b;
        blit_buffer[color_index+3] = a;
        #endif
        
        // Vertex 3
        vert_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        tex_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        blit_buffer[vert_index] = dx1;
        blit_buffer[vert_index+1] = dy2;
        blit_buffer[vert_index+2] = 0.0f;
        blit_buffer[tex_index] = x1;
        blit_buffer[tex_index+1] = y2;
        #ifdef SDL_GPU_USE_GL_TIER3
        color_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        blit_buffer[color_index] = r;
        blit_buffer[color_index+1] = g;
        blit_buffer[color_index+2] = b;
        blit_buffer[color_index+3] = a;
        #endif

        // Triangle indices
        
        // First tri
        index_buffer[cdata->index_buffer_num_vertices++] = cdata->blit_buffer_num_vertices;  // 0
        index_buffer[cdata->index_buffer_num_vertices++] = cdata->blit_buffer_num_vertices+1;  // 1
        index_buffer[cdata->index_buffer_num_vertices++] = cdata->blit_buffer_num_vertices+2;  // 2

        // Second tri
        index_buffer[cdata->index_buffer_num_vertices++] = cdata->blit_buffer_num_vertices; // 0
        index_buffer[cdata->index_buffer_num_vertices++] = cdata->blit_buffer_num_vertices+2;  // 2
        index_buffer[cdata->index_buffer_num_vertices++] = cdata->blit_buffer_num_vertices+3;  // 3

        cdata->blit_buffer_num_vertices += GPU_BLIT_BUFFER_VERTICES_PER_SPRITE;
    }

    return 0;
}


static int BlitRotate(GPU_Renderer* renderer, GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float angle)
{
    if(src == NULL || dest == NULL)
        return -1;

    return renderer->BlitTransformX(renderer, src, srcrect, dest, x, y, src->w/2.0f, src->h/2.0f, angle, 1.0f, 1.0f);
}

static int BlitScale(GPU_Renderer* renderer, GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float scaleX, float scaleY)
{
    if(src == NULL || dest == NULL)
        return -1;

    return renderer->BlitTransformX(renderer, src, srcrect, dest, x, y, src->w/2.0f, src->h/2.0f, 0.0f, scaleX, scaleY);
}

static int BlitTransform(GPU_Renderer* renderer, GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float angle, float scaleX, float scaleY)
{
    if(src == NULL || dest == NULL)
        return -1;

    return renderer->BlitTransformX(renderer, src, srcrect, dest, x, y, src->w/2.0f, src->h/2.0f, angle, scaleX, scaleY);
}

static int BlitTransformX(GPU_Renderer* renderer, GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float pivot_x, float pivot_y, float angle, float scaleX, float scaleY)
{
    if(src == NULL || dest == NULL)
        return -1;
    if(renderer != src->renderer || renderer != dest->renderer)
        return -2;


    makeContextCurrent(renderer, dest);
    if(renderer->current_context_target == NULL)
        return -3;
    
    // Bind the texture to which subsequent calls refer
    bindTexture(renderer, src);

    // Bind the FBO
    if(bindFramebuffer(renderer, dest))
    {
        prepareToRenderToTarget(renderer, dest);
        prepareToRenderImage(renderer, dest, src);
        
        Uint16 tex_w = ((IMAGE_DATA*)src->data)->tex_w;
        Uint16 tex_h = ((IMAGE_DATA*)src->data)->tex_h;

        float x1, y1, x2, y2;
        /*
            1,1 --- 3,3
             |       |
             |       |
            4,4 --- 2,2
        */
        float dx1, dy1, dx2, dy2, dx3, dy3, dx4, dy4;
        if(srcrect == NULL)
        {
            // Scale tex coords according to actual texture dims
            x1 = 0.1f/tex_w;
            y1 = 0.1f/tex_h;
            x2 = ((float)src->w - 0.1f)/tex_w;
            y2 = ((float)src->h - 0.1f)/tex_h;
            // Center the image on the given coords
            dx1 = -src->w/2.0f;
            dy1 = -src->h/2.0f;
            dx2 = src->w/2.0f;
            dy2 = src->h/2.0f;
        }
        else
        {
            // Scale srcrect tex coords according to actual texture dims
            x1 = (srcrect->x + 0.1f)/(float)tex_w;
            y1 = (srcrect->y + 0.1f)/(float)tex_h;
            x2 = (srcrect->x + srcrect->w - 0.1f)/(float)tex_w;
            y2 = (srcrect->y + srcrect->h - 0.1f)/(float)tex_h;
            // Center the image on the given coords
            dx1 = -srcrect->w/2.0f;
            dy1 = -srcrect->h/2.0f;
            dx2 = srcrect->w/2.0f;
            dy2 = srcrect->h/2.0f;
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
        pivot_x -= src->w/2.0f;
        pivot_y -= src->h/2.0f;

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
        if(angle != 0.0f)
        {
            float cosA = cos(angle*M_PI/180);
            float sinA = sin(angle*M_PI/180);
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

        CONTEXT_DATA* cdata = (CONTEXT_DATA*)renderer->current_context_target->context->data;
        float* blit_buffer = cdata->blit_buffer;
        unsigned short* index_buffer = cdata->index_buffer;

        if(cdata->blit_buffer_num_vertices + GPU_BLIT_BUFFER_VERTICES_PER_SPRITE >= cdata->blit_buffer_max_num_vertices)
            renderer->FlushBlitBuffer(renderer);

        #ifdef SDL_GPU_USE_GL_TIER3
        int color_index = GPU_BLIT_BUFFER_COLOR_OFFSET + cdata->blit_buffer_num_vertices*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        float r =  src->color.r/255.0f;
        float g =  src->color.g/255.0f;
        float b =  src->color.b/255.0f;
        float a =  GET_ALPHA(src->color)/255.0f;
        #endif

        // Sprite quad vertices
        
        // Vertex 0
        int vert_index = GPU_BLIT_BUFFER_VERTEX_OFFSET + cdata->blit_buffer_num_vertices*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        int tex_index = GPU_BLIT_BUFFER_TEX_COORD_OFFSET + cdata->blit_buffer_num_vertices*GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        blit_buffer[vert_index] = dx1;
        blit_buffer[vert_index+1] = dy1;
        blit_buffer[vert_index+2] = 0.0f;
        blit_buffer[tex_index] = x1;
        blit_buffer[tex_index+1] = y1;
        #ifdef SDL_GPU_USE_GL_TIER3
        blit_buffer[color_index] = r;
        blit_buffer[color_index+1] = g;
        blit_buffer[color_index+2] = b;
        blit_buffer[color_index+3] = a;
        #endif

        // Vertex 1
        vert_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        tex_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        blit_buffer[vert_index] = dx3;
        blit_buffer[vert_index+1] = dy3;
        blit_buffer[vert_index+2] = 0.0f;
        blit_buffer[tex_index] = x2;
        blit_buffer[tex_index+1] = y1;
        #ifdef SDL_GPU_USE_GL_TIER3
        color_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        blit_buffer[color_index] = r;
        blit_buffer[color_index+1] = g;
        blit_buffer[color_index+2] = b;
        blit_buffer[color_index+3] = a;
        #endif

        // Vertex 2
        vert_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        tex_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        blit_buffer[vert_index] = dx2;
        blit_buffer[vert_index+1] = dy2;
        blit_buffer[vert_index+2] = 0.0f;
        blit_buffer[tex_index] = x2;
        blit_buffer[tex_index+1] = y2;
        #ifdef SDL_GPU_USE_GL_TIER3
        color_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        blit_buffer[color_index] = r;
        blit_buffer[color_index+1] = g;
        blit_buffer[color_index+2] = b;
        blit_buffer[color_index+3] = a;
        #endif

        // Vertex 3
        vert_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        tex_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        blit_buffer[vert_index] = dx4;
        blit_buffer[vert_index+1] = dy4;
        blit_buffer[vert_index+2] = 0.0f;
        blit_buffer[tex_index] = x1;
        blit_buffer[tex_index+1] = y2;
        #ifdef SDL_GPU_USE_GL_TIER3
        color_index += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        blit_buffer[color_index] = r;
        blit_buffer[color_index+1] = g;
        blit_buffer[color_index+2] = b;
        blit_buffer[color_index+3] = a;
        #endif
        

        // Triangle indices
        
        // First tri
        index_buffer[cdata->index_buffer_num_vertices++] = cdata->blit_buffer_num_vertices;  // 0
        index_buffer[cdata->index_buffer_num_vertices++] = cdata->blit_buffer_num_vertices+1;  // 1
        index_buffer[cdata->index_buffer_num_vertices++] = cdata->blit_buffer_num_vertices+2;  // 2

        // Second tri
        index_buffer[cdata->index_buffer_num_vertices++] = cdata->blit_buffer_num_vertices; // 0
        index_buffer[cdata->index_buffer_num_vertices++] = cdata->blit_buffer_num_vertices+2;  // 2
        index_buffer[cdata->index_buffer_num_vertices++] = cdata->blit_buffer_num_vertices+3;  // 3

        cdata->blit_buffer_num_vertices += GPU_BLIT_BUFFER_VERTICES_PER_SPRITE;
    }

    return 0;
}

static int BlitTransformMatrix(GPU_Renderer* renderer, GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float* matrix3x3)
{
    if(src == NULL || dest == NULL)
        return -1;
    if(renderer != src->renderer || renderer != dest->renderer)
        return -2;
    
    // TODO: See below.
    renderer->FlushBlitBuffer(renderer);
    
    GPU_PushMatrix();

    // column-major 3x3 to column-major 4x4 (and scooting the 2D translations to the homogeneous column)
    // TODO: Should index 8 replace the homogeneous 1?  This looks like it adjusts the z-value...
    float matrix[16] = {matrix3x3[0], matrix3x3[1], matrix3x3[2], 0,
                        matrix3x3[3], matrix3x3[4], matrix3x3[5], 0,
                        0,            0,            matrix3x3[8], 0,
                        matrix3x3[6], matrix3x3[7], 0,            1
                       };
    GPU_Translate(x, y, 0);
    GPU_MultMatrix(matrix);

    int result = renderer->Blit(renderer, src, srcrect, dest, 0, 0);
    
    // Popping the matrix will revert the transform before it can be used, so we have to flush for now.
    // TODO: Do the matrix math myself on the vertex coords.
    renderer->FlushBlitBuffer(renderer);

    GPU_PopMatrix();

    return result;
}




static int BlitBatch(GPU_Renderer* renderer, GPU_Image* src, GPU_Target* dest, unsigned int numSprites, float* values, GPU_BlitFlagEnum flags)
{
    if(src == NULL || dest == NULL)
        return -1;
    if(renderer != src->renderer || renderer != dest->renderer)
        return -2;
    
    makeContextCurrent(renderer, dest);
    if(renderer->current_context_target == NULL)
        return -3;

    // Bind the texture to which subsequent calls refer
    bindTexture(renderer, src);

    // Bind the FBO
    if(bindFramebuffer(renderer, dest))
    {
        prepareToRenderToTarget(renderer, dest);
        prepareToRenderImage(renderer, dest, src);
        
        glEnable(GL_TEXTURE_2D);
        Uint8 isRTT = (dest->image != NULL);
        
        // Modify the viewport and projection matrix if rendering to a texture
        GLint vp[4];
        if(isRTT)
        {
            glGetIntegerv(GL_VIEWPORT, vp);

            unsigned int w = dest->w;
            unsigned int h = dest->h;

            glViewport( 0, 0, w, h);

            GPU_MatrixMode( GPU_PROJECTION );
            GPU_PushMatrix();
            GPU_LoadIdentity();

            GPU_Ortho(0.0f, w, 0.0f, h, -1.0f, 1.0f); // Special inverted orthographic projection because tex coords are inverted already.

            GPU_MatrixMode( GPU_MODELVIEW );
        }

        setClipRect(renderer, dest);
        

        CONTEXT_DATA* cdata = (CONTEXT_DATA*)renderer->current_context_target->context->data;
        unsigned short* index_buffer = cdata->index_buffer;

        renderer->FlushBlitBuffer(renderer);
        
        // Only do so many at a time
        int partial_num_sprites = cdata->blit_buffer_max_num_vertices/4;
        while(1)
        {
            if(numSprites < partial_num_sprites)
                partial_num_sprites = numSprites;
            if(partial_num_sprites <= 0)
                break;

            // Triangle indices
            int i;
            for(i = 0; i < partial_num_sprites; i++)
            {
                int buffer_num_vertices = i*4;
                // First tri
                index_buffer[cdata->index_buffer_num_vertices++] = buffer_num_vertices;  // 0
                index_buffer[cdata->index_buffer_num_vertices++] = buffer_num_vertices+1;  // 1
                index_buffer[cdata->index_buffer_num_vertices++] = buffer_num_vertices+2;  // 2

                // Second tri
                index_buffer[cdata->index_buffer_num_vertices++] = buffer_num_vertices; // 0
                index_buffer[cdata->index_buffer_num_vertices++] = buffer_num_vertices+2;  // 2
                index_buffer[cdata->index_buffer_num_vertices++] = buffer_num_vertices+3;  // 3
            }
            
    #ifdef SDL_GPU_USE_GL_TIER1

            float* vertex_pointer = values;
            float* texcoord_pointer = values + 3;
            float* color_pointer = values + 5;
            
            glBegin(GL_QUADS);
            for(i = 0; i < numSprites; i++)
            {
                glColor4f( *color_pointer, *(color_pointer+1), *(color_pointer+2), *(color_pointer+3) );
                glTexCoord2f( *texcoord_pointer, *(texcoord_pointer+1) );
                glVertex3f( *vertex_pointer, *(vertex_pointer+1), *(vertex_pointer+2) );
                color_pointer += 9;
                texcoord_pointer += 9;
                vertex_pointer += 9;

                glColor4f( *color_pointer, *(color_pointer+1), *(color_pointer+2), *(color_pointer+3) );
                glTexCoord2f( *texcoord_pointer, *(texcoord_pointer+1) );
                glVertex3f( *vertex_pointer, *(vertex_pointer+1), *(vertex_pointer+2) );
                color_pointer += 9;
                texcoord_pointer += 9;
                vertex_pointer += 9;

                glColor4f( *color_pointer, *(color_pointer+1), *(color_pointer+2), *(color_pointer+3) );
                glTexCoord2f( *texcoord_pointer, *(texcoord_pointer+1) );
                glVertex3f( *vertex_pointer, *(vertex_pointer+1), *(vertex_pointer+2) );
                color_pointer += 9;
                texcoord_pointer += 9;
                vertex_pointer += 9;

                glColor4f( *color_pointer, *(color_pointer+1), *(color_pointer+2), *(color_pointer+3) );
                glTexCoord2f( *texcoord_pointer, *(texcoord_pointer+1) );
                glVertex3f( *vertex_pointer, *(vertex_pointer+1), *(vertex_pointer+2) );
                color_pointer += 9;
                texcoord_pointer += 9;
                vertex_pointer += 9;
            }
            glEnd();
    #elif defined(SDL_GPU_USE_GL_TIER2)

            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnableClientState(GL_COLOR_ARRAY);
            
            glVertexPointer(3, GL_FLOAT, GPU_BLIT_BUFFER_STRIDE, values + GPU_BLIT_BUFFER_VERTEX_OFFSET);
            glTexCoordPointer(2, GL_FLOAT, GPU_BLIT_BUFFER_STRIDE, values + GPU_BLIT_BUFFER_TEX_COORD_OFFSET);
            glColorPointer(4, GL_FLOAT, GPU_BLIT_BUFFER_STRIDE, values + GPU_BLIT_BUFFER_COLOR_OFFSET);

            glDrawElements(GL_TRIANGLES, cdata->index_buffer_num_vertices, GL_UNSIGNED_SHORT, cdata->index_buffer);

            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);

    #elif defined(SDL_GPU_USE_GL_TIER3)

            TARGET_DATA* data = ((TARGET_DATA*)renderer->current_context_target->data);
            
            // Upload our modelviewprojection matrix
            if(data->current_shader_block.modelViewProjection_loc >= 0)
            {
                float mvp[16];
                GPU_GetModelViewProjection(mvp);
                glUniformMatrix4fv(data->current_shader_block.modelViewProjection_loc, 1, 0, mvp);
            }
        
            // Update the vertex array object's buffers
            #if !defined(SDL_GPU_USE_GLES) || SDL_GPU_GLES_MAJOR_VERSION != 2
            glBindVertexArray(data->blit_VAO);
            #endif
            
            // Upload blit buffer to a single buffer object
            glBindBuffer(GL_ARRAY_BUFFER, data->blit_VBO[data->blit_VBO_flop]);
            data->blit_VBO_flop = !data->blit_VBO_flop;
            
            // Copy the whole blit buffer to the GPU
            glBufferSubData(GL_ARRAY_BUFFER, 0, GPU_BLIT_BUFFER_STRIDE * (partial_num_sprites*4), values);  // Fills GPU buffer with data.
            
            // Specify the formatting of the blit buffer
            if(data->current_shader_block.position_loc >= 0)
            {
                glEnableVertexAttribArray(data->current_shader_block.position_loc);  // Tell GL to use client-side attribute data
                glVertexAttribPointer(data->current_shader_block.position_loc, 3, GL_FLOAT, GL_FALSE, GPU_BLIT_BUFFER_STRIDE, 0);  // Tell how the data is formatted
            }
            if(data->current_shader_block.texcoord_loc >= 0)
            {
                glEnableVertexAttribArray(data->current_shader_block.texcoord_loc);
                glVertexAttribPointer(data->current_shader_block.texcoord_loc, 2, GL_FLOAT, GL_FALSE, GPU_BLIT_BUFFER_STRIDE, (void*)(GPU_BLIT_BUFFER_TEX_COORD_OFFSET * sizeof(float)));
            }
            if(data->current_shader_block.color_loc >= 0)
            {
                glEnableVertexAttribArray(data->current_shader_block.color_loc);
                glVertexAttribPointer(data->current_shader_block.color_loc, 4, GL_FLOAT, GL_FALSE, GPU_BLIT_BUFFER_STRIDE, (void*)(GPU_BLIT_BUFFER_COLOR_OFFSET * sizeof(float)));
            }
            
            glDrawElements(GL_TRIANGLES, cdata->index_buffer_num_vertices, GL_UNSIGNED_SHORT, cdata->index_buffer);
            
            #if !defined(SDL_GPU_USE_GLES) || SDL_GPU_GLES_MAJOR_VERSION != 2
            glBindVertexArray(0);
            #endif

    #endif

            values += partial_num_sprites*4*9;  // 9 floats per vertex
            
            numSprites -= partial_num_sprites;
            
            cdata->blit_buffer_num_vertices = 0;
            cdata->index_buffer_num_vertices = 0;
        }

        unsetClipRect(renderer, dest);

        // restore viewport and projection
        if(isRTT)
        {
            glViewport(vp[0], vp[1], vp[2], vp[3]);

            GPU_MatrixMode( GPU_PROJECTION );
            GPU_PopMatrix();
            GPU_MatrixMode( GPU_MODELVIEW );
        }
    }

    return 0;
}

static float SetZ(GPU_Renderer* renderer, float z)
{
    if(renderer == NULL)
        return 0.0f;

    float oldZ = ((CONTEXT_DATA*)renderer->current_context_target->context->data)->z;
    ((CONTEXT_DATA*)renderer->current_context_target->context->data)->z = z;

    return oldZ;
}

static float GetZ(GPU_Renderer* renderer)
{
    if(renderer == NULL)
        return 0.0f;
    return ((CONTEXT_DATA*)renderer->current_context_target->context->data)->z;
}

static void GenerateMipmaps(GPU_Renderer* renderer, GPU_Image* image)
{
    #ifndef __IPHONEOS__
    if(image == NULL)
        return;
    
    if(image->target != NULL && isCurrentTarget(renderer, image->target))
        renderer->FlushBlitBuffer(renderer);
    bindTexture(renderer, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    ((IMAGE_DATA*)image->data)->has_mipmaps = 1;

    GLint filter;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &filter);
    if(filter == GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    #endif
}




static GPU_Rect SetClip(GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
    if(target == NULL)
    {
        GPU_Rect r = {0,0,0,0};
        return r;
    }

    if(isCurrentTarget(renderer, target))
        renderer->FlushBlitBuffer(renderer);
    target->use_clip_rect = 1;

    GPU_Rect r = target->clip_rect;

    target->clip_rect.x = x;
    target->clip_rect.y = y;
    target->clip_rect.w = w;
    target->clip_rect.h = h;

    return r;
}

static void ClearClip(GPU_Renderer* renderer, GPU_Target* target)
{
    if(target == NULL)
        return;

    makeContextCurrent(renderer, target);
    
    if(isCurrentTarget(renderer, target))
        renderer->FlushBlitBuffer(renderer);
    target->use_clip_rect = 0;
    target->clip_rect.x = 0;
    target->clip_rect.y = 0;
    target->clip_rect.w = target->w;
    target->clip_rect.h = target->h;
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
        renderer->FlushBlitBuffer(renderer);
    if(bindFramebuffer(renderer, target))
    {
        unsigned char pixels[4];
        glReadPixels(x, y, 1, 1, ((TARGET_DATA*)target->data)->format, GL_UNSIGNED_BYTE, pixels);

        result.r = pixels[0];
        result.g = pixels[1];
        result.b = pixels[2];
#ifdef SDL_GPU_USE_SDL2
        result.a = pixels[3];
#else
        result.unused = pixels[3];
#endif
    }

    return result;
}

static void SetImageFilter(GPU_Renderer* renderer, GPU_Image* image, GPU_FilterEnum filter)
{
    if(image == NULL)
        return;
    if(renderer != image->renderer)
        return;

    bindTexture(renderer, image);

    GLenum minFilter = GL_NEAREST;
    GLenum magFilter = GL_NEAREST;

    if(filter == GPU_LINEAR)
    {
        if(((IMAGE_DATA*)image->data)->has_mipmaps)
            minFilter = GL_LINEAR_MIPMAP_NEAREST;
        else
            minFilter = GL_LINEAR;

        magFilter = GL_LINEAR;
    }
    else if(filter == GPU_LINEAR_MIPMAP)
    {
        if(((IMAGE_DATA*)image->data)->has_mipmaps)
            minFilter = GL_LINEAR_MIPMAP_LINEAR;
        else
            minFilter = GL_LINEAR;

        magFilter = GL_LINEAR;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}





static GPU_Rect getViewport()
{
    float v[4];
    glGetFloatv(GL_VIEWPORT, v);
    GPU_Rect r = {v[0], v[1], v[2], v[3]};
    return r;
}

static void setViewport(GPU_Rect rect)
{
    if(rect.w < 0.0f || rect.h < 0.0f)
    {
        GPU_LogError("SDL_gpu: Couldn't set viewport to negative rect: %dx%d\n", (int)rect.w, (int)rect.h);
        return;
    }
    
    glViewport(rect.x, rect.y, rect.w, rect.h);
}


static void Clear(GPU_Renderer* renderer, GPU_Target* target)
{
    if(target == NULL)
        return;
    if(renderer != target->renderer)
        return;

    makeContextCurrent(renderer, target);
    
    if(isCurrentTarget(renderer, target))
        renderer->FlushBlitBuffer(renderer);
    if(bindFramebuffer(renderer, target))
    {
        GPU_Rect viewport = getViewport();
        glViewport(0,0,target->w, target->h);

        setClipRect(renderer, target);

        //glPushAttrib(GL_COLOR_BUFFER_BIT);

        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
        //glPopAttrib();

        unsetClipRect(renderer, target);

        setViewport(viewport);
    }
}


static void ClearRGBA(GPU_Renderer* renderer, GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    if(target == NULL)
        return;
    if(renderer != target->renderer)
        return;

    makeContextCurrent(renderer, target);
    
    if(isCurrentTarget(renderer, target))
        renderer->FlushBlitBuffer(renderer);
    if(bindFramebuffer(renderer, target))
    {
        GPU_Rect viewport = getViewport();
        glViewport(0,0,target->w, target->h);
        setClipRect(renderer, target);

        glClearColor(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        unsetClipRect(renderer, target);

        setViewport(viewport);
    }
}

static void FlushBlitBuffer(GPU_Renderer* renderer)
{
    CONTEXT_DATA* cdata = (CONTEXT_DATA*)renderer->current_context_target->context->data;
    if(cdata->blit_buffer_num_vertices > 0 && cdata->last_target != NULL && cdata->last_image != NULL)
    {
        glEnable(GL_TEXTURE_2D);
        GPU_Target* dest = cdata->last_target;
        Uint8 isRTT = (dest->image != NULL);

        // Modify the viewport and projection matrix if rendering to a texture
        GLint vp[4];
        if(isRTT)
        {
            glGetIntegerv(GL_VIEWPORT, vp);

            unsigned int w = dest->w;
            unsigned int h = dest->h;

            glViewport( 0, 0, w, h);

            GPU_MatrixMode( GPU_PROJECTION );
            GPU_PushMatrix();
            GPU_LoadIdentity();

            GPU_Ortho(0.0f, w, 0.0f, h, -1.0f, 1.0f); // Special inverted orthographic projection because tex coords are inverted already.

            GPU_MatrixMode( GPU_MODELVIEW );
        }

        setClipRect(renderer, dest);



#ifdef SDL_GPU_USE_GL_TIER1

        float* vertex_pointer = cdata->blit_buffer + GPU_BLIT_BUFFER_VERTEX_OFFSET;
        float* texcoord_pointer = cdata->blit_buffer + GPU_BLIT_BUFFER_TEX_COORD_OFFSET;
        
        glBegin(GL_QUADS);
        int i;
        for(i = 0; i < cdata->blit_buffer_num_vertices; i += GPU_BLIT_BUFFER_VERTICES_PER_SPRITE)
        {
            glTexCoord2f( *texcoord_pointer, *(texcoord_pointer+1) );
            glVertex3f( *vertex_pointer, *(vertex_pointer+1), *(vertex_pointer+2) );
            texcoord_pointer += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
            vertex_pointer += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;

            glTexCoord2f( *texcoord_pointer, *(texcoord_pointer+1) );
            glVertex3f( *vertex_pointer, *(vertex_pointer+1), *(vertex_pointer+2) );
            texcoord_pointer += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
            vertex_pointer += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;

            glTexCoord2f( *texcoord_pointer, *(texcoord_pointer+1) );
            glVertex3f( *vertex_pointer, *(vertex_pointer+1), *(vertex_pointer+2) );
            texcoord_pointer += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
            vertex_pointer += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;

            glTexCoord2f( *texcoord_pointer, *(texcoord_pointer+1) );
            glVertex3f( *vertex_pointer, *(vertex_pointer+1), *(vertex_pointer+2) );
            texcoord_pointer += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
            vertex_pointer += GPU_BLIT_BUFFER_FLOATS_PER_VERTEX;
        }
        glEnd();
#elif defined(SDL_GPU_USE_GL_TIER2)

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glVertexPointer(3, GL_FLOAT, GPU_BLIT_BUFFER_STRIDE, cdata->blit_buffer + GPU_BLIT_BUFFER_VERTEX_OFFSET);
        glTexCoordPointer(2, GL_FLOAT, GPU_BLIT_BUFFER_STRIDE, cdata->blit_buffer + GPU_BLIT_BUFFER_TEX_COORD_OFFSET);

        glDrawElements(GL_TRIANGLES, cdata->index_buffer_num_vertices, GL_UNSIGNED_SHORT, cdata->index_buffer);

        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);

#elif defined(SDL_GPU_USE_GL_TIER3)

        TARGET_DATA* data = ((TARGET_DATA*)renderer->current_context_target->data);
        
        // Upload our modelviewprojection matrix
        if(data->current_shader_block.modelViewProjection_loc >= 0)
        {
            float mvp[16];
            GPU_GetModelViewProjection(mvp);
            glUniformMatrix4fv(data->current_shader_block.modelViewProjection_loc, 1, 0, mvp);
        }
    
        // Update the vertex array object's buffers
        #if !defined(SDL_GPU_USE_GLES) || SDL_GPU_GLES_MAJOR_VERSION != 2
        glBindVertexArray(data->blit_VAO);
        #endif
        
        // Upload blit buffer to a single buffer object
        glBindBuffer(GL_ARRAY_BUFFER, data->blit_VBO[data->blit_VBO_flop]);
        data->blit_VBO_flop = !data->blit_VBO_flop;
        
        // Copy the whole blit buffer to the GPU
        glBufferSubData(GL_ARRAY_BUFFER, 0, GPU_BLIT_BUFFER_STRIDE * cdata->blit_buffer_num_vertices, cdata->blit_buffer);  // Fills GPU buffer with data.
        
        // Specify the formatting of the blit buffer
        if(data->current_shader_block.position_loc >= 0)
        {
            glEnableVertexAttribArray(data->current_shader_block.position_loc);  // Tell GL to use client-side attribute data
            glVertexAttribPointer(data->current_shader_block.position_loc, 3, GL_FLOAT, GL_FALSE, GPU_BLIT_BUFFER_STRIDE, 0);  // Tell how the data is formatted
        }
        if(data->current_shader_block.texcoord_loc >= 0)
        {
            glEnableVertexAttribArray(data->current_shader_block.texcoord_loc);
            glVertexAttribPointer(data->current_shader_block.texcoord_loc, 2, GL_FLOAT, GL_FALSE, GPU_BLIT_BUFFER_STRIDE, (void*)(GPU_BLIT_BUFFER_TEX_COORD_OFFSET * sizeof(float)));
        }
        if(data->current_shader_block.color_loc >= 0)
        {
            glEnableVertexAttribArray(data->current_shader_block.color_loc);
            glVertexAttribPointer(data->current_shader_block.color_loc, 4, GL_FLOAT, GL_FALSE, GPU_BLIT_BUFFER_STRIDE, (void*)(GPU_BLIT_BUFFER_COLOR_OFFSET * sizeof(float)));
        }
        
        glDrawElements(GL_TRIANGLES, cdata->index_buffer_num_vertices, GL_UNSIGNED_SHORT, cdata->index_buffer);
        
        #if !defined(SDL_GPU_USE_GLES) || SDL_GPU_GLES_MAJOR_VERSION != 2
        glBindVertexArray(0);
        #endif

#endif

        cdata->blit_buffer_num_vertices = 0;
        cdata->index_buffer_num_vertices = 0;

        unsetClipRect(renderer, dest);

        // restore viewport and projection
        if(isRTT)
        {
            glViewport(vp[0], vp[1], vp[2], vp[3]);

            GPU_MatrixMode( GPU_PROJECTION );
            GPU_PopMatrix();
            GPU_MatrixMode( GPU_MODELVIEW );
        }

    }
}

static void Flip(GPU_Renderer* renderer, GPU_Target* target)
{
    renderer->FlushBlitBuffer(renderer);
    
    makeContextCurrent(renderer, target);

#ifdef SDL_GPU_USE_SDL2
    if(renderer->current_context_target == NULL)
        return;
    SDL_GL_SwapWindow(SDL_GetWindowFromID(renderer->current_context_target->context->windowID));
#else
    SDL_GL_SwapBuffers();
#endif
}




// Shader API



static int read_string_rw(SDL_RWops* rwops, char* result)
{
   if(rwops == NULL)
        return 0;
    
    size_t size = 100;
    long total = 0;
    long len = 0;
    while((len = SDL_RWread(rwops, &result[total], 1, size)) > 0)
    {
        total += len;
    }
    
    SDL_RWclose(rwops);
    
    result[total] = '\0';
    
    return total;
}

static char shader_message[256];

static Uint32 CompileShader_RW(GPU_Renderer* renderer, int shader_type, SDL_RWops* shader_source)
{
    // Read in the shader source code
    // TODO: It'd be nice to check the file size first...
    char* source_string = (char*)malloc(2000);
    int result = read_string_rw(shader_source, source_string);
    if(!result)
    {
        GPU_LogError("Failed to read shader source.\n");
        snprintf(shader_message, 256, "Failed to read shader source.\n");
        free(source_string);
        return 0;
    }
    
    Uint32 result2 = renderer->CompileShader(renderer, shader_type, source_string);
    free(source_string);
    
    return result2;
}

static Uint32 CompileShader(GPU_Renderer* renderer, int shader_type, const char* shader_source)
{
    // Create the proper new shader object
    GLuint shader_object = 0;
    
    #ifndef SDL_GPU_DISABLE_SHADERS
    
    switch(shader_type)
    {
    case GPU_VERTEX_SHADER:
        shader_object = glCreateShader(GL_VERTEX_SHADER);
        break;
    case GPU_FRAGMENT_SHADER:
        shader_object = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    #ifdef GL_GEOMETRY_SHADER
    case GPU_GEOMETRY_SHADER:
        shader_object = glCreateShader(GL_GEOMETRY_SHADER);
        break;
    #endif
    }
    
    if(shader_object == 0)
    {
        GPU_LogError("Failed to create new shader object.\n");
        snprintf(shader_message, 256, "Failed to create new shader object.\n");
        return 0;
    }
   
	glShaderSource(shader_object, 1, &shader_source, NULL);
    
    // Compile the shader source
    GLint compiled;
	
	glCompileShader(shader_object);
	
    glGetShaderiv(shader_object, GL_COMPILE_STATUS, &compiled);
    if(!compiled)
    {
        GPU_LogError("Failed to compile shader source.\n");
        glGetShaderInfoLog(shader_object, 256, NULL, shader_message);
        glDeleteShader(shader_object);
        return 0;
    }
    
    #endif
    
    return shader_object;
}

static Uint32 LinkShaderProgram(GPU_Renderer* renderer, Uint32 program_object)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
	glLinkProgram(program_object);
	
	int linked;
	glGetProgramiv(program_object, GL_LINK_STATUS, &linked);
	
	if(!linked)
    {
        GPU_LogError("Failed to link shader program.\n");
        glGetProgramInfoLog(program_object, 256, NULL, shader_message);
        glDeleteProgram(program_object);
        return 0;
    }
	#endif
    
	return program_object;
}

static Uint32 LinkShaders(GPU_Renderer* renderer, Uint32 shader_object1, Uint32 shader_object2)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    GLuint p = glCreateProgram();

	glAttachShader(p, shader_object1);
	glAttachShader(p, shader_object2);
	
	return renderer->LinkShaderProgram(renderer, p);
	#else
	return 0;
	#endif
}

static void FreeShader(GPU_Renderer* renderer, Uint32 shader_object)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    glDeleteShader(shader_object);
    #endif
}

static void FreeShaderProgram(GPU_Renderer* renderer, Uint32 program_object)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    glDeleteProgram(program_object);
    #endif
}

static void AttachShader(GPU_Renderer* renderer, Uint32 program_object, Uint32 shader_object)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    glAttachShader(program_object, shader_object);
    #endif
}

static void DetachShader(GPU_Renderer* renderer, Uint32 program_object, Uint32 shader_object)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    glDetachShader(program_object, shader_object);
    #endif
}

static Uint8 IsDefaultShaderProgram(GPU_Renderer* renderer, Uint32 program_object)
{
    CONTEXT_DATA* cdata = (CONTEXT_DATA*)renderer->current_context_target->context->data;
    return (program_object == cdata->default_textured_shader_program || program_object == cdata->default_untextured_shader_program);
}

static void ActivateShaderProgram(GPU_Renderer* renderer, Uint32 program_object, GPU_ShaderBlock* block)
{
    GPU_Target* target = renderer->current_context_target;
    #ifndef SDL_GPU_DISABLE_SHADERS
    if(target == NULL)
        return;
    
    CONTEXT_DATA* cdata = (CONTEXT_DATA*)target->context->data;
    if(program_object == 0) // Implies default shader
    {
        // Already using a default shader?
        if(target->context->current_shader_program == cdata->default_textured_shader_program
            || target->context->current_shader_program == cdata->default_untextured_shader_program)
            return;
        
        program_object = cdata->default_untextured_shader_program;
    }
    
    if(target == NULL || target->context->current_shader_program == program_object)
        return;
    
    renderer->FlushBlitBuffer(renderer);
    glUseProgram(program_object);
    
        #ifdef SDL_GPU_USE_GL_TIER3
        // Set up our shader attribute and uniform locations
        TARGET_DATA* data = ((TARGET_DATA*)target->data);
        if(program_object == cdata->default_textured_shader_program)
            data->current_shader_block = data->shader_block[0];
        else if(program_object == cdata->default_untextured_shader_program)
            data->current_shader_block = data->shader_block[1];
        else
        {
            if(block == NULL)
            {
                GPU_ShaderBlock b;
                b.position_loc = -1;
                b.texcoord_loc = -1;
                b.color_loc = -1;
                b.modelViewProjection_loc = -1;
                data->current_shader_block = b;
            }
            else
                data->current_shader_block = *block;
        }
        #endif
    #endif
    
    target->context->current_shader_program = program_object;
}

static void DeactivateShaderProgram(GPU_Renderer* renderer)
{
    renderer->ActivateShaderProgram(renderer, 0, NULL);
}

static const char* GetShaderMessage(GPU_Renderer* renderer)
{
    return shader_message;
}

static int GetAttribLocation(GPU_Renderer* renderer, Uint32 program_object, const char* attrib_name)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    return glGetAttribLocation(program_object, attrib_name);
    #else
    return -1;
    #endif
}

static int GetUniformLocation(GPU_Renderer* renderer, Uint32 program_object, const char* uniform_name)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    return glGetUniformLocation(program_object, uniform_name);
    #else
    return -1;
    #endif
}

static GPU_ShaderBlock LoadShaderBlock(GPU_Renderer* renderer, Uint32 program_object, const char* position_name, const char* texcoord_name, const char* color_name, const char* modelViewMatrix_name)
{
    GPU_ShaderBlock b;
    
    if(position_name == NULL)
        b.position_loc = -1;
    else
        b.position_loc = renderer->GetAttribLocation(renderer, program_object, position_name);
        
    if(texcoord_name == NULL)
        b.texcoord_loc = -1;
    else
        b.texcoord_loc = renderer->GetAttribLocation(renderer, program_object, texcoord_name);
        
    if(color_name == NULL)
        b.color_loc = -1;
    else
        b.color_loc = renderer->GetAttribLocation(renderer, program_object, color_name);
        
    if(modelViewMatrix_name == NULL)
        b.modelViewProjection_loc = -1;
    else
        b.modelViewProjection_loc = renderer->GetUniformLocation(renderer, program_object, modelViewMatrix_name);
    
    return b;
}

static void SetShaderBlock(GPU_Renderer* renderer, GPU_ShaderBlock block)
{
    #ifdef SDL_GPU_USE_GL_TIER3
    GPU_Target* target = renderer->current_context_target;
    if(target == NULL)
        return;
    TARGET_DATA* data = ((TARGET_DATA*)target->data);
    data->current_shader_block = block;
    #endif
}


static void GetUniformiv(GPU_Renderer* renderer, Uint32 program_object, int location, int* values)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    glGetUniformiv(program_object, location, values);
    #endif
}

static void SetUniformi(GPU_Renderer* renderer, int location, int value)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    renderer->FlushBlitBuffer(renderer);
    glUniform1i(location, value);
    #endif
}

static void SetUniformiv(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, int* values)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    renderer->FlushBlitBuffer(renderer);
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
    #ifndef SDL_GPU_DISABLE_SHADERS
    #if defined(SDL_GPU_USE_GLES) && SDL_GPU_GLES_MAJOR_VERSION < 3
    glGetUniformiv(program_object, location, (int*)values);
    #else
    glGetUniformuiv(program_object, location, values);
    #endif
    #endif
}

static void SetUniformui(GPU_Renderer* renderer, int location, unsigned int value)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    renderer->FlushBlitBuffer(renderer);
    #if defined(SDL_GPU_USE_GLES) && SDL_GPU_GLES_MAJOR_VERSION < 3
    glUniform1i(location, (int)value);
    #else
    glUniform1ui(location, value);
    #endif
    #endif
}

static void SetUniformuiv(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, unsigned int* values)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    renderer->FlushBlitBuffer(renderer);
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
    #ifndef SDL_GPU_DISABLE_SHADERS
    glGetUniformfv(program_object, location, values);
    #endif
}

static void SetUniformf(GPU_Renderer* renderer, int location, float value)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    renderer->FlushBlitBuffer(renderer);
    glUniform1f(location, value);
    #endif
}

static void SetUniformfv(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, float* values)
{
    #ifndef SDL_GPU_DISABLE_SHADERS
    renderer->FlushBlitBuffer(renderer);
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
    #ifndef SDL_GPU_DISABLE_SHADERS
    renderer->FlushBlitBuffer(renderer);
    if(num_rows < 2 || num_rows > 4 || num_columns < 2 || num_columns > 4)
    {
        GPU_LogError("GPU_SetUniformMatrixfv(): Given invalid dimensions (%dx%d).\n", num_rows, num_columns);
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
        GPU_LogError("GPU_SetUniformMatrixfv(): GLES renderers do not accept non-square matrices (%dx%d).\n", num_rows, num_columns);
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


#define SET_COMMON_FUNCTIONS(renderer) \
    renderer->Init = &Init; \
    renderer->IsFeatureEnabled = &IsFeatureEnabled; \
    renderer->CreateTargetFromWindow = &CreateTargetFromWindow; \
    renderer->MakeCurrent = &MakeCurrent; \
    renderer->SetAsCurrent = &SetAsCurrent; \
    renderer->SetWindowResolution = &SetWindowResolution; \
    renderer->SetVirtualResolution = &SetVirtualResolution; \
    renderer->Quit = &Quit; \
 \
    renderer->ToggleFullscreen = &ToggleFullscreen; \
    renderer->SetCamera = &SetCamera; \
 \
    renderer->CreateImage = &CreateImage; \
    renderer->LoadImage = &LoadImage; \
    renderer->SaveImage = &SaveImage; \
    renderer->CopyImage = &CopyImage; \
    renderer->UpdateImage = &UpdateImage; \
    renderer->CopyImageFromSurface = &CopyImageFromSurface; \
    renderer->CopyImageFromTarget = &CopyImageFromTarget; \
    renderer->CopySurfaceFromTarget = &CopySurfaceFromTarget; \
    renderer->CopySurfaceFromImage = &CopySurfaceFromImage; \
    renderer->SubSurfaceCopy = &SubSurfaceCopy; \
    renderer->FreeImage = &FreeImage; \
 \
    renderer->LoadTarget = &LoadTarget; \
    renderer->FreeTarget = &FreeTarget; \
 \
    renderer->Blit = &Blit; \
    renderer->BlitRotate = &BlitRotate; \
    renderer->BlitScale = &BlitScale; \
    renderer->BlitTransform = &BlitTransform; \
    renderer->BlitTransformX = &BlitTransformX; \
    renderer->BlitTransformMatrix = &BlitTransformMatrix; \
    renderer->BlitBatch = &BlitBatch; \
 \
    renderer->SetZ = &SetZ; \
    renderer->GetZ = &GetZ; \
    renderer->GenerateMipmaps = &GenerateMipmaps; \
 \
    renderer->SetClip = &SetClip; \
    renderer->ClearClip = &ClearClip; \
     \
    renderer->GetPixel = &GetPixel; \
    renderer->SetImageFilter = &SetImageFilter; \
 \
    renderer->Clear = &Clear; \
    renderer->ClearRGBA = &ClearRGBA; \
    renderer->FlushBlitBuffer = &FlushBlitBuffer; \
    renderer->Flip = &Flip; \
     \
    renderer->CompileShader_RW = &CompileShader_RW; \
    renderer->CompileShader = &CompileShader; \
    renderer->LinkShaderProgram = &LinkShaderProgram; \
    renderer->LinkShaders = &LinkShaders; \
    renderer->FreeShader = &FreeShader; \
    renderer->FreeShaderProgram = &FreeShaderProgram; \
    renderer->AttachShader = &AttachShader; \
    renderer->DetachShader = &DetachShader; \
    renderer->IsDefaultShaderProgram = &IsDefaultShaderProgram; \
    renderer->ActivateShaderProgram = &ActivateShaderProgram; \
    renderer->DeactivateShaderProgram = &DeactivateShaderProgram; \
    renderer->GetShaderMessage = &GetShaderMessage; \
    renderer->GetAttribLocation = &GetAttribLocation; \
    renderer->GetUniformLocation = &GetUniformLocation; \
    renderer->LoadShaderBlock = &LoadShaderBlock; \
    renderer->SetShaderBlock = &SetShaderBlock; \
    renderer->GetUniformiv = &GetUniformiv; \
    renderer->SetUniformi = &SetUniformi; \
    renderer->SetUniformiv = &SetUniformiv; \
    renderer->GetUniformuiv = &GetUniformuiv; \
    renderer->SetUniformui = &SetUniformui; \
    renderer->SetUniformuiv = &SetUniformuiv; \
    renderer->GetUniformfv = &GetUniformfv; \
    renderer->SetUniformf = &SetUniformf; \
    renderer->SetUniformfv = &SetUniformfv; \
    renderer->SetUniformMatrixfv = &SetUniformMatrixfv; \
	 \
	/* Shape rendering */ \
	 \
    renderer->SetLineThickness = &SetLineThickness; \
    renderer->SetLineThickness(renderer, 1.0f); \
    renderer->GetLineThickness = &GetLineThickness; \
    renderer->Pixel = &Pixel; \
    renderer->Line = &Line; \
    renderer->Arc = &Arc; \
    renderer->ArcFilled = &ArcFilled; \
    renderer->Circle = &Circle; \
    renderer->CircleFilled = &CircleFilled; \
    renderer->Tri = &Tri; \
    renderer->TriFilled = &TriFilled; \
    renderer->Rectangle = &Rectangle; \
    renderer->RectangleFilled = &RectangleFilled; \
    renderer->RectangleRound = &RectangleRound; \
    renderer->RectangleRoundFilled = &RectangleRoundFilled; \
    renderer->Polygon = &Polygon; \
    renderer->PolygonFilled = &PolygonFilled; \
    renderer->PolygonBlit = &PolygonBlit;

