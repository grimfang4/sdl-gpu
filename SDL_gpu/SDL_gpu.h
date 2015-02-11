#ifndef _SDL_GPU_H__
#define _SDL_GPU_H__

#include "SDL.h"
#include <stdio.h>
#include <stdarg.h>

// Use SDL's DLL defines
#include "begin_code.h"

#ifdef __cplusplus
extern "C" {
#endif

// Compile-time versions
#define SDL_GPU_VERSION_MAJOR 0
#define SDL_GPU_VERSION_MINOR 10
#define SDL_GPU_VERSION_PATCH 0

/* Auto-detect if we're using the SDL2 API by the headers available. */
#if SDL_VERSION_ATLEAST(2,0,0)
    #define SDL_GPU_USE_SDL2
#else
    #define SDL_GPU_USE_SDL1
#endif

typedef struct GPU_Renderer GPU_Renderer;
typedef struct GPU_Target GPU_Target;

/*!
 * \defgroup Initialization Initialization
 * SDL_gpu has a fairly simple initialization process.  If you need nothing more than the default initialization, call:
 * <pre>GPU_Target* screen = GPU_Init(width, height, GPU_DEFAULT_INIT_FLAGS);</pre>
 * Then when you're done, clean up with:
 * <pre>GPU_Quit();</pre>
 * 
 * Other functions in the Initialization module control how initialization is performed.
 * 
 * \defgroup Logging Debugging, Logging, and Error Handling
 * Use GPU_Log() for normal logging output (e.g. to replace printf).  Other logging priorities are handled by GPU_LogWarning() and GPU_LogError().
 * 
 * SDL_gpu stores an error stack that you can read and manipulate using GPU_PopErrorCode() and GPU_PushErrorCode().  If you set the debug level using GPU_SetDebugLevel(), you can have any errors automatically logged as they are generated.
 * 
 * \defgroup RendererSetup Renderer Setup
 * \defgroup RendererControls Renderer Controls
 * \defgroup ContextControls Context Controls
 * \defgroup TargetControls Target Controls
 * \defgroup SurfaceControls Surface Controls
 * \defgroup ImageControls Image Controls
 * \defgroup Conversions Surface, Image, and Target Conversions
 * \defgroup Matrix Matrix Controls
 * \defgroup Rendering Rendering
 * \defgroup Shapes Shapes
 * \defgroup ShaderInterface Shader Interface
 */

/*! \ingroup Rendering
 * A struct representing a rectangular area with floating point precision.
 * \see GPU_MakeRect() 
 */
typedef struct GPU_Rect
{
    float x, y;
    float w, h;
} GPU_Rect;

#define GPU_RENDERER_ORDER_MAX 10

typedef Uint32 GPU_RendererEnum;
static const GPU_RendererEnum GPU_RENDERER_UNKNOWN = 0;  // invalid value
static const GPU_RendererEnum GPU_RENDERER_OPENGL_1_BASE = 1;
static const GPU_RendererEnum GPU_RENDERER_OPENGL_1 = 2;
static const GPU_RendererEnum GPU_RENDERER_OPENGL_2 = 3;
static const GPU_RendererEnum GPU_RENDERER_OPENGL_3 = 4;
static const GPU_RendererEnum GPU_RENDERER_OPENGL_4 = 5;
static const GPU_RendererEnum GPU_RENDERER_GLES_1 = 11;
static const GPU_RendererEnum GPU_RENDERER_GLES_2 = 12;
static const GPU_RendererEnum GPU_RENDERER_GLES_3 = 13;
static const GPU_RendererEnum GPU_RENDERER_D3D9 = 21;
static const GPU_RendererEnum GPU_RENDERER_D3D10 = 22;
static const GPU_RendererEnum GPU_RENDERER_D3D11 = 23;
#define GPU_RENDERER_CUSTOM_0 1000

/*! \ingroup Initialization
 * \ingroup RendererSetup
 * \ingroup RendererControls
 * Renderer ID object for identifying a specific renderer.
 * \see GPU_MakeRendererID()
 * \see GPU_InitRendererByID()
 */
typedef struct GPU_RendererID
{
    const char* name;
    GPU_RendererEnum renderer;
    int major_version;
    int minor_version;
    
    int index;
} GPU_RendererID;


/*! \ingroup ImageControls
 * Blend component functions
 * \see GPU_SetBlendFunction()
 * Values chosen for direct OpenGL compatibility.
 */
typedef enum {
    GPU_FUNC_ZERO = 0,
    GPU_FUNC_ONE = 1,
    GPU_FUNC_SRC_COLOR = 0x0300,
    GPU_FUNC_DST_COLOR = 0x0306,
    GPU_FUNC_ONE_MINUS_SRC = 0x0301,
    GPU_FUNC_ONE_MINUS_DST = 0x0307,
    GPU_FUNC_SRC_ALPHA = 0x0302,
    GPU_FUNC_DST_ALPHA = 0x0304,
    GPU_FUNC_ONE_MINUS_SRC_ALPHA = 0x0303,
    GPU_FUNC_ONE_MINUS_DST_ALPHA = 0x0305
} GPU_BlendFuncEnum;

/*! \ingroup ImageControls
 * Blend component equations
 * \see GPU_SetBlendEquation()
 * Values chosen for direct OpenGL compatibility.
 */
typedef enum {
    GPU_EQ_ADD = 0x8006,
    GPU_EQ_SUBTRACT = 0x800A,
    GPU_EQ_REVERSE_SUBTRACT = 0x800B
} GPU_BlendEqEnum;

/*! \ingroup ImageControls
 * Blend mode storage struct */
typedef struct GPU_BlendMode
{
    GPU_BlendFuncEnum source_color;
    GPU_BlendFuncEnum dest_color;
    GPU_BlendFuncEnum source_alpha;
    GPU_BlendFuncEnum dest_alpha;
    
    GPU_BlendEqEnum color_equation;
    GPU_BlendEqEnum alpha_equation;
} GPU_BlendMode;

/*! \ingroup ImageControls
 * Blend mode presets 
 * \see GPU_SetBlendMode()
 * \see GPU_GetBlendModeFromPreset()
 */
typedef enum {
    GPU_BLEND_NORMAL = 0,
    GPU_BLEND_PREMULTIPLIED_ALPHA = 1,
    GPU_BLEND_MULTIPLY = 2,
    GPU_BLEND_ADD = 3,
    GPU_BLEND_SUBTRACT = 4,
    GPU_BLEND_MOD_ALPHA = 5,
    GPU_BLEND_SET_ALPHA = 6,
    GPU_BLEND_SET = 7,
    GPU_BLEND_NORMAL_KEEP_ALPHA = 8,
    GPU_BLEND_NORMAL_ADD_ALPHA = 9
} GPU_BlendPresetEnum;

/*! \ingroup ImageControls
 * Image filtering options.  These affect the quality/interpolation of colors when images are scaled. 
 * \see GPU_SetImageFilter()
 */
typedef enum {
    GPU_FILTER_NEAREST = 0,
    GPU_FILTER_LINEAR = 1,
    GPU_FILTER_LINEAR_MIPMAP = 2
} GPU_FilterEnum;

/*! \ingroup ImageControls
 * Snap modes.  Blitting with these modes will align the sprite with the target's pixel grid.
 * \see GPU_SetSnapMode()
 * \see GPU_GetSnapMode()
 */
typedef enum {
    GPU_SNAP_NONE = 0,
    GPU_SNAP_POSITION = 1,
    GPU_SNAP_DIMENSIONS = 2,
    GPU_SNAP_POSITION_AND_DIMENSIONS = 3
} GPU_SnapEnum;


/*! \ingroup ImageControls
 * Image wrapping options.  These affect how images handle src_rect coordinates beyond their dimensions when blitted.
 * \see GPU_SetWrapMode()
 */
typedef enum {
    GPU_WRAP_NONE = 0,
    GPU_WRAP_REPEAT = 1,
    GPU_WRAP_MIRRORED = 2
} GPU_WrapEnum;

/*! \ingroup ImageControls
 * Image format enum
 * \see GPU_CreateImage()
 */
typedef enum {
    GPU_FORMAT_LUMINANCE = 1,
    GPU_FORMAT_LUMINANCE_ALPHA = 2,
    GPU_FORMAT_RGB = 3,
    GPU_FORMAT_RGBA = 4,
    GPU_FORMAT_ALPHA = 5,
    GPU_FORMAT_RG = 6,
    GPU_FORMAT_YCbCr422 = 7,
    GPU_FORMAT_YCbCr420P = 8
} GPU_FormatEnum;



/*! \ingroup ImageControls
 * Image object for containing pixel/texture data.
 * A GPU_Image can be created with GPU_CreateImage(), GPU_LoadImage(), GPU_CopyImage(), or GPU_CopyImageFromSurface().
 * Free the memory with GPU_FreeImage() when you're done.
 * \see GPU_CreateImage()
 * \see GPU_LoadImage()
 * \see GPU_CopyImage()
 * \see GPU_CopyImageFromSurface()
 * \see GPU_Target
 */
typedef struct GPU_Image
{
	struct GPU_Renderer* renderer;
	GPU_Target* target;
	Uint16 w, h;
	GPU_FormatEnum format;
	int num_layers;
	int bytes_per_pixel;
	Uint16 base_w, base_h;  // Underlying texture dimensions
	Uint8 has_mipmaps;
	
	SDL_Color color;
	Uint8 use_blending;
	GPU_BlendMode blend_mode;
	GPU_FilterEnum filter_mode;
	GPU_SnapEnum snap_mode;
	GPU_WrapEnum wrap_mode_x;
	GPU_WrapEnum wrap_mode_y;
	
	void* data;
	int refcount;
	Uint8 is_alias;
} GPU_Image;


/*! \ingroup TargetControls
 * Camera object that determines viewing transform.
 * \see GPU_SetCamera() 
 * \see GPU_GetDefaultCamera() 
 * \see GPU_GetCamera()
 */
typedef struct GPU_Camera
{
	float x, y, z;
	float angle;
	float zoom;
} GPU_Camera;


/*! \ingroup ShaderInterface
 * Container for the built-in shader attribute and uniform locations (indices).
 * \see GPU_LoadShaderBlock()
 * \see GPU_SetShaderBlock()
 */
typedef struct GPU_ShaderBlock
{
    // Attributes
    int position_loc;
    int texcoord_loc;
    int color_loc;
    // Uniforms
    int modelViewProjection_loc;
} GPU_ShaderBlock;





#define GPU_MODELVIEW 0
#define GPU_PROJECTION 1

#ifndef GPU_MATRIX_STACK_MAX
#define GPU_MATRIX_STACK_MAX 5
#endif

/*! \ingroup Matrix
 * Matrix stack data structure for global vertex transforms.  */
typedef struct GPU_MatrixStack
{
    unsigned int size;
    float matrix[GPU_MATRIX_STACK_MAX][16];
} GPU_MatrixStack;


/*! \ingroup ContextControls
 * Rendering context data.  Only GPU_Targets which represent windows will store this. */
typedef struct GPU_Context
{
    /*! SDL_GLContext */
    void* context;
    Uint8 failed;
    
    /*! SDL window ID */
	Uint32 windowID;
	
	/*! Actual window dimensions */
	int window_w;
	int window_h;
	
	/*! Window dimensions for restoring windowed mode after GPU_SetFullscreen(1,1). */
	int stored_window_w;
	int stored_window_h;
	
	/*! Internal state */
	Uint32 current_shader_program;
	Uint32 default_textured_shader_program;
	Uint32 default_untextured_shader_program;
	
	Uint8 shapes_use_blending;
	GPU_BlendMode shapes_blend_mode;
	float line_thickness;
	Uint8 use_texturing;
	
    int matrix_mode;
    GPU_MatrixStack projection_matrix;
    GPU_MatrixStack modelview_matrix;
	
	void* data;
} GPU_Context;


/*! \ingroup TargetControls
 * Render target object for use as a blitting destination.
 * A GPU_Target can be created from a GPU_Image with GPU_LoadTarget().
 * A GPU_Target can also represent a separate window with GPU_CreateTargetFromWindow().  In that case, 'context' is allocated and filled in.
 * Note: You must have passed the SDL_WINDOW_OPENGL flag to SDL_CreateWindow() for OpenGL renderers to work with new windows.
 * Free the memory with GPU_FreeTarget() when you're done.
 * \see GPU_LoadTarget()
 * \see GPU_CreateTargetFromWindow()
 * \see GPU_FreeTarget()
 */
struct GPU_Target
{
	struct GPU_Renderer* renderer;
	GPU_Image* image;
	void* data;
	Uint16 w, h;
	Uint8 using_virtual_resolution;
	Uint16 base_w, base_h;  // The true dimensions of the underlying image or window
	Uint8 use_clip_rect;
	GPU_Rect clip_rect;
	Uint8 use_color;
	SDL_Color color;
	
	GPU_Rect viewport;
	
	/*! Perspective and object viewing transforms. */
	GPU_Camera camera;
	
	/*! Renderer context data.  NULL if the target does not represent a window or rendering context. */
	GPU_Context* context;
	int refcount;
	Uint8 is_alias;
};

/*! \ingroup Initialization
 * Important GPU features which may not be supported depending on a device's extension support.  Can be bitwise OR'd together.
 * \see GPU_IsFeatureEnabled()
 * \see GPU_SetRequiredFeatures()
 */
typedef Uint32 GPU_FeatureEnum;
static const GPU_FeatureEnum GPU_FEATURE_NON_POWER_OF_TWO = 0x1;
static const GPU_FeatureEnum GPU_FEATURE_RENDER_TARGETS = 0x2;
static const GPU_FeatureEnum GPU_FEATURE_BLEND_EQUATIONS = 0x4;
static const GPU_FeatureEnum GPU_FEATURE_BLEND_FUNC_SEPARATE = 0x8;
static const GPU_FeatureEnum GPU_FEATURE_BLEND_EQUATIONS_SEPARATE = 0x10;
static const GPU_FeatureEnum GPU_FEATURE_GL_BGR = 0x20;
static const GPU_FeatureEnum GPU_FEATURE_GL_BGRA = 0x40;
static const GPU_FeatureEnum GPU_FEATURE_GL_ABGR = 0x80;
static const GPU_FeatureEnum GPU_FEATURE_VERTEX_SHADER = 0x100;
static const GPU_FeatureEnum GPU_FEATURE_FRAGMENT_SHADER = 0x200;
static const GPU_FeatureEnum GPU_FEATURE_PIXEL_SHADER = 0x200;
static const GPU_FeatureEnum GPU_FEATURE_GEOMETRY_SHADER = 0x400;
static const GPU_FeatureEnum GPU_FEATURE_WRAP_REPEAT_MIRRORED = 0x800;

/*! Combined feature flags */
#define GPU_FEATURE_ALL_BASE GPU_FEATURE_RENDER_TARGETS
#define GPU_FEATURE_ALL_BLEND_PRESETS (GPU_FEATURE_BLEND_EQUATIONS | GPU_FEATURE_BLEND_FUNC_SEPARATE)
#define GPU_FEATURE_ALL_GL_FORMATS (GPU_FEATURE_GL_BGR | GPU_FEATURE_GL_BGRA | GPU_FEATURE_GL_ABGR)
#define GPU_FEATURE_BASIC_SHADERS (GPU_FEATURE_FRAGMENT_SHADER | GPU_FEATURE_VERTEX_SHADER)
#define GPU_FEATURE_ALL_SHADERS (GPU_FEATURE_FRAGMENT_SHADER | GPU_FEATURE_VERTEX_SHADER | GPU_FEATURE_GEOMETRY_SHADER)


typedef Uint32 GPU_WindowFlagEnum;

/*! \ingroup Initialization
 * Initialization flags for changing default init parameters.  Can be bitwise OR'ed together.
 * Default (0) is to use late swap vsync and double buffering.
 * \see GPU_SetPreInitFlags()
 * \see GPU_GetPreInitFlags()
 */
typedef Uint32 GPU_InitFlagEnum;
static const GPU_InitFlagEnum GPU_INIT_ENABLE_VSYNC = 0x1;
static const GPU_InitFlagEnum GPU_INIT_DISABLE_VSYNC = 0x2;
static const GPU_InitFlagEnum GPU_INIT_DISABLE_DOUBLE_BUFFER = 0x4;
static const GPU_InitFlagEnum GPU_INIT_DISABLE_AUTO_VIRTUAL_RESOLUTION = 0x8;

#define GPU_DEFAULT_INIT_FLAGS 0


static const Uint32 GPU_NONE = 0x0;

/*! \ingroup Rendering
 * Bit flags for the blit batch functions.
 * \see GPU_BlitBatch()
 * \see GPU_BlitBatchSeparate()
 */
typedef Uint32 GPU_BlitFlagEnum;
static const GPU_BlitFlagEnum GPU_PASSTHROUGH_VERTICES = 0x1;
static const GPU_BlitFlagEnum GPU_PASSTHROUGH_TEXCOORDS = 0x2;
static const GPU_BlitFlagEnum GPU_PASSTHROUGH_COLORS = 0x4;
static const GPU_BlitFlagEnum GPU_USE_DEFAULT_POSITIONS = 0x8;
static const GPU_BlitFlagEnum GPU_USE_DEFAULT_SRC_RECTS = 0x10;
static const GPU_BlitFlagEnum GPU_USE_DEFAULT_COLORS = 0x20;

#define GPU_PASSTHROUGH_ALL (GPU_PASSTHROUGH_VERTICES | GPU_PASSTHROUGH_TEXCOORDS | GPU_PASSTHROUGH_COLORS)

/*! \ingroup ShaderInterface
 * Type enumeration for GPU_AttributeFormat specifications.
 */
typedef Uint32 GPU_TypeEnum;
// Use OpenGL's values for simpler translation
static const GPU_TypeEnum GPU_TYPE_BYTE = 0x1400;
static const GPU_TypeEnum GPU_TYPE_UNSIGNED_BYTE = 0x1401;
static const GPU_TypeEnum GPU_TYPE_SHORT = 0x1402;
static const GPU_TypeEnum GPU_TYPE_UNSIGNED_SHORT = 0x1403;
static const GPU_TypeEnum GPU_TYPE_INT = 0x1404;
static const GPU_TypeEnum GPU_TYPE_UNSIGNED_INT = 0x1405;
static const GPU_TypeEnum GPU_TYPE_FLOAT = 0x1406;
static const GPU_TypeEnum GPU_TYPE_DOUBLE = 0x140A;






/*! \ingroup ShaderInterface
 * Shader type enum.
 * \see GPU_LoadShader()
 * \see GPU_CompileShader()
 * \see GPU_CompileShader_RW()
 */
typedef enum {
    GPU_VERTEX_SHADER = 0,
    GPU_FRAGMENT_SHADER = 1,
    GPU_PIXEL_SHADER = 1,
    GPU_GEOMETRY_SHADER = 2
} GPU_ShaderEnum;



/*! \ingroup ShaderInterface
 * Type enumeration for the shader language used by the renderer.
 */
typedef enum {
    GPU_LANGUAGE_NONE = 0,
    GPU_LANGUAGE_ARB_ASSEMBLY = 1,
    GPU_LANGUAGE_GLSL = 2,
    GPU_LANGUAGE_GLSLES = 3,
    GPU_LANGUAGE_HLSL = 4,
    GPU_LANGUAGE_CG = 5
} GPU_ShaderLanguageEnum;

/*! \ingroup ShaderInterface */
typedef struct GPU_AttributeFormat
{
    Uint8 is_per_sprite;  // Per-sprite values are expanded to 4 vertices
    int num_elems_per_value;
    GPU_TypeEnum type;  // GPU_TYPE_FLOAT, GPU_TYPE_INT, GPU_TYPE_UNSIGNED_INT, etc.
    Uint8 normalize;
    int stride_bytes;  // Number of bytes between two vertex specifications
    int offset_bytes;  // Number of bytes to skip at the beginning of 'values'
} GPU_AttributeFormat;

/*! \ingroup ShaderInterface */
typedef struct GPU_Attribute
{
    int location;
    void* values;  // Expect 4 values for each sprite
    GPU_AttributeFormat format;
} GPU_Attribute;

/*! \ingroup ShaderInterface */
typedef struct GPU_AttributeSource
{
    Uint8 enabled;
    int num_values;
    void* next_value;
    // Automatic storage format
    int per_vertex_storage_stride_bytes;
    int per_vertex_storage_offset_bytes;
    int per_vertex_storage_size;  // Over 0 means that the per-vertex storage has been automatically allocated
    void* per_vertex_storage;  // Could point to the attribute's values or to allocated storage
    GPU_Attribute attribute;
} GPU_AttributeSource;


/*! \ingroup Logging
 * Type enumeration for error codes.
 * \see GPU_PushErrorCode()
 * \see GPU_PopErrorCode()
 */
typedef enum {
    GPU_ERROR_NONE = 0,
    GPU_ERROR_BACKEND_ERROR = 1,
    GPU_ERROR_DATA_ERROR = 2,
    GPU_ERROR_USER_ERROR = 3,
    GPU_ERROR_UNSUPPORTED_FUNCTION = 4,
    GPU_ERROR_NULL_ARGUMENT = 5,
    GPU_ERROR_FILE_NOT_FOUND = 6
} GPU_ErrorEnum;

/*! \ingroup Logging */
typedef struct GPU_ErrorObject
{
    char* function;
    GPU_ErrorEnum error;
    char* details;
} GPU_ErrorObject;


/*! \ingroup Logging
 * Type enumeration for debug levels.
 * \see GPU_SetDebugLevel()
 * \see GPU_GetDebugLevel()
 */
typedef enum {
    GPU_DEBUG_LEVEL_0 = 0,
    GPU_DEBUG_LEVEL_1 = 1,
    GPU_DEBUG_LEVEL_2 = 2,
    GPU_DEBUG_LEVEL_3 = 3,
    GPU_DEBUG_LEVEL_MAX = 3
} GPU_DebugLevelEnum;


/* Private implementation of renderer members */
struct GPU_RendererImpl;

/*! Renderer object which specializes the API to a particular backend. */
struct GPU_Renderer
{
	/*! Struct identifier of the renderer. */
	GPU_RendererID id;
	GPU_RendererID requested_id;
	GPU_WindowFlagEnum SDL_init_flags;
	GPU_InitFlagEnum GPU_init_flags;
	
	GPU_ShaderLanguageEnum shader_language;
	int shader_version;
    GPU_FeatureEnum enabled_features;
	
	/*! Current display target */
	GPU_Target* current_context_target;
	
	struct GPU_RendererImpl* impl;
};






/*! \ingroup Initialization
 *  @{ */

// Visual C does not support static inline
#ifdef _MSC_VER
static SDL_version SDLCALL GPU_GetCompiledVersion(void)
#else
static inline SDL_version SDLCALL GPU_GetCompiledVersion(void)
#endif
{
    SDL_version v = {SDL_GPU_VERSION_MAJOR, SDL_GPU_VERSION_MINOR, SDL_GPU_VERSION_PATCH};
    return v;
}

DECLSPEC SDL_version SDLCALL GPU_GetLinkedVersion(void);

/*! The window corresponding to 'windowID' will be used to create the rendering context instead of creating a new window. */
DECLSPEC void SDLCALL GPU_SetInitWindow(Uint32 windowID);

/*! Returns the window ID that has been set via GPU_SetInitWindow(). */
DECLSPEC Uint32 SDLCALL GPU_GetInitWindow(void);

/*! Set special flags to use for initialization. Set these before calling GPU_Init().
 * \param GPU_flags An OR'ed combination of GPU_InitFlagEnum flags.  Default flags (0) enable late swap vsync and double buffering. */
DECLSPEC void SDLCALL GPU_SetPreInitFlags(GPU_InitFlagEnum GPU_flags);

/*! Returns the current special flags to use for initialization. */
DECLSPEC GPU_InitFlagEnum SDLCALL GPU_GetPreInitFlags(void);

/*! Set required features to use for initialization. Set these before calling GPU_Init().
 * \param features An OR'ed combination of GPU_FeatureEnum flags.  Required features will force GPU_Init() to create a renderer that supports all of the given flags or else fail. */
DECLSPEC void SDLCALL GPU_SetRequiredFeatures(GPU_FeatureEnum features);

/*! Returns the current required features to use for initialization. */
DECLSPEC GPU_FeatureEnum SDLCALL GPU_GetRequiredFeatures(void);

/*! Gets the default initialization renderer IDs for the current platform copied into the 'order' array and the number of renderer IDs into 'order_size'.  Pass NULL for 'order' to just get the size of the renderer order array.  Will return at most GPU_RENDERER_ORDER_MAX renderers. */
DECLSPEC void SDLCALL GPU_GetDefaultRendererOrder(int* order_size, GPU_RendererID* order);

/*! Gets the current renderer ID order for initialization copied into the 'order' array and the number of renderer IDs into 'order_size'.  Pass NULL for 'order' to just get the size of the renderer order array. */
DECLSPEC void SDLCALL GPU_GetRendererOrder(int* order_size, GPU_RendererID* order);

/*! Sets the renderer ID order to use for initialization.  If 'order' is NULL, it will restore the default order. */
DECLSPEC void SDLCALL GPU_SetRendererOrder(int order_size, GPU_RendererID* order);

/*! Initializes SDL and SDL_gpu.  Creates a window and goes through the renderer order to create a renderer context.
 * \see GPU_SetRendererOrder()
 */
DECLSPEC GPU_Target* SDLCALL GPU_Init(Uint16 w, Uint16 h, GPU_WindowFlagEnum SDL_flags);

/*! Initializes SDL and SDL_gpu.  Creates a window and the requested renderer context. */
DECLSPEC GPU_Target* SDLCALL GPU_InitRenderer(GPU_RendererEnum renderer_enum, Uint16 w, Uint16 h, GPU_WindowFlagEnum SDL_flags);

/*! Initializes SDL and SDL_gpu.  Creates a window and the requested renderer context.
 * By requesting a renderer via ID, you can specify the major and minor versions of an individual renderer backend.
 * \see GPU_MakeRendererID
 */
DECLSPEC GPU_Target* SDLCALL GPU_InitRendererByID(GPU_RendererID renderer_request, Uint16 w, Uint16 h, GPU_WindowFlagEnum SDL_flags);

/*! Checks for important GPU features which may not be supported depending on a device's extension support.  Feature flags (GPU_FEATURE_*) can be bitwise OR'd together. 
 * \return 1 if all of the passed features are enabled/supported
 * \return 0 if any of the passed features are disabled/unsupported
 */
DECLSPEC Uint8 SDLCALL GPU_IsFeatureEnabled(GPU_FeatureEnum feature);

/*! Clean up the renderer state. */
DECLSPEC void SDLCALL GPU_CloseCurrentRenderer(void);

/*! Clean up the renderer state and shut down SDL_gpu. */
DECLSPEC void SDLCALL GPU_Quit(void);

// End of Initialization
/*! @} */



// Debugging, logging, and error handling

#define GPU_Log GPU_LogInfo
/*! \ingroup Logging
 *  @{ */

/*! Sets the global debug level.
 * GPU_DEBUG_LEVEL_0: Normal
 * GPU_DEBUG_LEVEL_1: Prints messages when errors are pushed via GPU_PushErrorCode()
 * GPU_DEBUG_LEVEL_2: Elevates warning logs to error priority
 * GPU_DEBUG_LEVEL_3: Elevates info logs to error priority
 */
DECLSPEC void SDLCALL GPU_SetDebugLevel(GPU_DebugLevelEnum level);

/*! Returns the current global debug level. */
DECLSPEC GPU_DebugLevelEnum SDLCALL GPU_GetDebugLevel(void);

/*! Prints an informational log message. */
DECLSPEC void SDLCALL GPU_LogInfo(const char* format, ...);

/*! Prints a warning log message. */
DECLSPEC void SDLCALL GPU_LogWarning(const char* format, ...);

/*! Prints an error log message. */
DECLSPEC void SDLCALL GPU_LogError(const char* format, ...);

/*! Pushes a new error code onto the error stack.  If the stack is full, this function does nothing.
 * \param function The name of the function that pushed the error
 * \param error The error code to push on the error stack
 * \param details Additional information string, can be NULL.
 */
DECLSPEC void SDLCALL GPU_PushErrorCode(const char* function, GPU_ErrorEnum error, const char* details, ...);

/*! Pops an error object from the error stack and returns it.  If the error stack is empty, it returns an error object with NULL function, GPU_ERROR_NONE error, and NULL details. */
DECLSPEC GPU_ErrorObject SDLCALL GPU_PopErrorCode(void);

/*! Gets the string representation of an error code. */
DECLSPEC const char* SDLCALL GPU_GetErrorString(GPU_ErrorEnum error);

// End of Logging
/*! @} */







/*! \ingroup RendererSetup
 *  @{ */

/*! Returns an initialized GPU_RendererID. */
DECLSPEC GPU_RendererID SDLCALL GPU_MakeRendererID(const char* name, GPU_RendererEnum renderer, int major_version, int minor_version);

/*! Gets the first registered renderer identifier for the given enum value. */
DECLSPEC GPU_RendererID SDLCALL GPU_GetRendererID(GPU_RendererEnum renderer);

/*! Gets the renderer identifier for the given registration index. */
DECLSPEC GPU_RendererID SDLCALL GPU_GetRendererIDByIndex(unsigned int index);

/*! Gets the number of registered (available) renderers. */
DECLSPEC int SDLCALL GPU_GetNumRegisteredRenderers(void);

/*! Gets an array of identifiers for the registered (available) renderers. */
DECLSPEC void SDLCALL GPU_GetRegisteredRendererList(GPU_RendererID* renderers_array);

/*! Prepares a renderer for use by SDL_gpu. */
DECLSPEC void SDLCALL GPU_RegisterRenderer(GPU_RendererID id, GPU_Renderer* (SDLCALL *create_renderer)(GPU_RendererID request), void (SDLCALL *free_renderer)(GPU_Renderer* renderer));

// End of RendererSetup
/*! @} */



/*! \ingroup RendererControls
 *  @{ */

/*! Gets the next enum ID that can be used for a custom renderer. */
DECLSPEC GPU_RendererEnum SDLCALL GPU_ReserveNextRendererEnum(void);

/*! Gets the number of active (created) renderers. */
DECLSPEC int SDLCALL GPU_GetNumActiveRenderers(void);

/*! Gets an array of identifiers for the active renderers. */
DECLSPEC void SDLCALL GPU_GetActiveRendererList(GPU_RendererID* renderers_array);

/*! Gets the renderer for the given renderer index. */
DECLSPEC GPU_Renderer* SDLCALL GPU_GetRenderer(unsigned int index);

/*! \return The renderer matching the given identifier. */
DECLSPEC GPU_Renderer* SDLCALL GPU_GetRendererByID(GPU_RendererID id);

/*! \return The current renderer */
DECLSPEC GPU_Renderer* SDLCALL GPU_GetCurrentRenderer(void);

/*! Switches the current renderer to the renderer matching the given identifier. */
DECLSPEC void SDLCALL GPU_SetCurrentRenderer(GPU_RendererID id);

/*! Reapplies the renderer state to the backend API (e.g. OpenGL, Direct3D).  Use this if you want SDL_gpu to be able to render after you've used direct backend calls. */
DECLSPEC void SDLCALL GPU_ResetRendererState(void);

// End of RendererControls
/*! @} */




// Context / window controls

/*! \ingroup ContextControls
 *  @{ */

/*! \return The renderer's current context target. */
DECLSPEC GPU_Target* SDLCALL GPU_GetContextTarget(void);

/*! \return The target that is associated with the given windowID. */
DECLSPEC GPU_Target* SDLCALL GPU_GetWindowTarget(Uint32 windowID);

/*! Creates a separate context for the given window using the current renderer and returns a GPU_Target that represents it. */
DECLSPEC GPU_Target* SDLCALL GPU_CreateTargetFromWindow(Uint32 windowID);

/*! Makes the given window the current rendering destination for the given context target.
 * This also makes the target the current context for image loading and window operations.
 * If the target does not represent a window, this does nothing.
 */
DECLSPEC void SDLCALL GPU_MakeCurrent(GPU_Target* target, Uint32 windowID);

/*! Change the actual size of the current context target's window.  This resets the virtual resolution and viewport of the context target.
 * Aside from direct resolution changes, this should also be called in response to SDL_WINDOWEVENT_RESIZED window events for resizable windows. */
DECLSPEC Uint8 SDLCALL GPU_SetWindowResolution(Uint16 w, Uint16 h);

/*! Enable/disable fullscreen mode for the current context target's window.
 * On some platforms, this may destroy the renderer context and require that textures be reloaded.  Unfortunately, SDL does not provide a notification mechanism for this.
 * \param enable_fullscreen If true, make the application go fullscreen.  If false, make the application go to windowed mode.
 * \param use_desktop_resolution If true, lets the window change its resolution when it enters fullscreen mode (via SDL_WINDOW_FULLSCREEN_DESKTOP).
 * \return 0 if the new mode is windowed, 1 if the new mode is fullscreen.  */
DECLSPEC Uint8 SDLCALL GPU_SetFullscreen(Uint8 enable_fullscreen, Uint8 use_desktop_resolution);

/*! Returns true if the current context target's window is in fullscreen mode. */
DECLSPEC Uint8 SDLCALL GPU_GetFullscreen(void);

/*! Enables/disables alpha blending for shape rendering on the current window. */
DECLSPEC void SDLCALL GPU_SetShapeBlending(Uint8 enable);

/*! Translates a blend preset into a blend mode. */
DECLSPEC GPU_BlendMode SDLCALL GPU_GetBlendModeFromPreset(GPU_BlendPresetEnum preset);

/*! Sets the blending component functions for shape rendering. */
DECLSPEC void SDLCALL GPU_SetShapeBlendFunction(GPU_BlendFuncEnum source_color, GPU_BlendFuncEnum dest_color, GPU_BlendFuncEnum source_alpha, GPU_BlendFuncEnum dest_alpha);

/*! Sets the blending component equations for shape rendering. */
DECLSPEC void SDLCALL GPU_SetShapeBlendEquation(GPU_BlendEqEnum color_equation, GPU_BlendEqEnum alpha_equation);
	
/*! Sets the blending mode for shape rendering on the current window, if supported by the renderer. */
DECLSPEC void SDLCALL GPU_SetShapeBlendMode(GPU_BlendPresetEnum mode);

/*! Sets the thickness of lines for the current context. 
 * \param thickness New line thickness in pixels measured across the line.  Default is 1.0f.
 * \return The old thickness value
 */
DECLSPEC float SDLCALL GPU_SetLineThickness(float thickness);

/*! Returns the current line thickness value. */
DECLSPEC float SDLCALL GPU_GetLineThickness(void);

// End of ContextControls
/*! @} */




/*! \ingroup TargetControls
 *  @{ */

/*! Creates a target that aliases the given target.  Aliases can be used to store target settings (e.g. viewports) for easy switching.
 * GPU_FreeTarget() frees the alias's memory, but does not affect the original. */
DECLSPEC GPU_Target* SDLCALL GPU_CreateAliasTarget(GPU_Target* target);

/*! Creates a new render target from the given image.  It can then be accessed from image->target. */
DECLSPEC GPU_Target* SDLCALL GPU_LoadTarget(GPU_Image* image);

/*! Deletes a render target in the proper way for this renderer. */
DECLSPEC void SDLCALL GPU_FreeTarget(GPU_Target* target);

/*! Change the logical size of the given target.  Rendering to this target will be scaled as if the dimensions were actually the ones given. */
DECLSPEC void SDLCALL GPU_SetVirtualResolution(GPU_Target* target, Uint16 w, Uint16 h);

/*! Converts screen space coordinates (such as from mouse input) to logical drawing coordinates. */
DECLSPEC void SDLCALL GPU_GetVirtualCoords(GPU_Target* target, float* x, float* y, float displayX, float displayY);

/*! Reset the logical size of the given target to its original value. */
DECLSPEC void SDLCALL GPU_UnsetVirtualResolution(GPU_Target* target);

/*! \return A GPU_Rect with the given values. */
DECLSPEC GPU_Rect SDLCALL GPU_MakeRect(float x, float y, float w, float h);

/*! \return An SDL_Color with the given values. */
DECLSPEC SDL_Color SDLCALL GPU_MakeColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/*! Sets the given target's viewport. */
DECLSPEC void SDLCALL GPU_SetViewport(GPU_Target* target, GPU_Rect viewport);

/*! \return A GPU_Camera with position (0, 0, -10), angle of 0, and zoom of 1. */
DECLSPEC GPU_Camera SDLCALL GPU_GetDefaultCamera(void);

/*! \return The camera of the given render target.  If target is NULL, returns the default camera. */
DECLSPEC GPU_Camera SDLCALL GPU_GetCamera(GPU_Target* target);

/*! Sets the current render target's current camera.
 * \param target A pointer to the target that will copy this camera.
 * \param cam A pointer to the camera data to use or NULL to use the default camera.
 * \return The old camera. */
DECLSPEC GPU_Camera SDLCALL GPU_SetCamera(GPU_Target* target, GPU_Camera* cam);

/*! \return The RGBA color of a pixel. */
DECLSPEC SDL_Color SDLCALL GPU_GetPixel(GPU_Target* target, Sint16 x, Sint16 y);

/*! Sets the clipping rect for the given render target. */
DECLSPEC GPU_Rect SDLCALL GPU_SetClipRect(GPU_Target* target, GPU_Rect rect);

/*! Sets the clipping rect for the given render target. */
DECLSPEC GPU_Rect SDLCALL GPU_SetClip(GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h);

/*! Turns off clipping for the given target. */
DECLSPEC void SDLCALL GPU_UnsetClip(GPU_Target* target);

/*! Sets the modulation color for subsequent drawing of images and shapes on the given target. 
 *  This has a cumulative effect with the image coloring functions.
 *  e.g. GPU_SetRGB(image, 255, 128, 0); GPU_SetTargetRGB(target, 128, 128, 128);
 *  Would make the image draw with color of roughly (128, 64, 0).
 */
DECLSPEC void SDLCALL GPU_SetTargetColor(GPU_Target* target, SDL_Color color);

/*! Sets the modulation color for subsequent drawing of images and shapes on the given target. 
 *  This has a cumulative effect with the image coloring functions.
 *  e.g. GPU_SetRGB(image, 255, 128, 0); GPU_SetTargetRGB(target, 128, 128, 128);
 *  Would make the image draw with color of roughly (128, 64, 0).
 */
DECLSPEC void SDLCALL GPU_SetTargetRGB(GPU_Target* target, Uint8 r, Uint8 g, Uint8 b);

/*! Sets the modulation color for subsequent drawing of images and shapes on the given target. 
 *  This has a cumulative effect with the image coloring functions.
 *  e.g. GPU_SetRGB(image, 255, 128, 0); GPU_SetTargetRGB(target, 128, 128, 128);
 *  Would make the image draw with color of roughly (128, 64, 0).
 */
DECLSPEC void SDLCALL GPU_SetTargetRGBA(GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/*! Unsets the modulation color for subsequent drawing of images and shapes on the given target. 
 *  This has the same effect as coloring with pure opaque white (255, 255, 255, 255).
 */
DECLSPEC void SDLCALL GPU_UnsetTargetColor(GPU_Target* target);

// End of TargetControls
/*! @} */



/*! \ingroup SurfaceControls
 *  @{ */

/*! Load surface from an image file that is supported by this renderer.  Don't forget to SDL_FreeSurface() it. */
DECLSPEC SDL_Surface* SDLCALL GPU_LoadSurface(const char* filename);

/*! Save surface to a file.  The file type is deduced from the extension.  Supported formats are: png, bmp, tga.  Returns 0 on failure. */
DECLSPEC Uint8 SDLCALL GPU_SaveSurface(SDL_Surface* surface, const char* filename);

// End of SurfaceControls
/*! @} */




/*! \ingroup ImageControls
 *  @{ */

/*! Create a new, blank image with the given format.  Don't forget to GPU_FreeImage() it.
	 * \param w Image width in pixels
	 * \param h Image height in pixels
	 * \param format Format of color channels.
	 */
DECLSPEC GPU_Image* SDLCALL GPU_CreateImage(Uint16 w, Uint16 h, GPU_FormatEnum format);

/*! Create a new image that uses the given native texture handle as the image texture. */
DECLSPEC GPU_Image* SDLCALL GPU_CreateImageUsingTexture(Uint32 handle, Uint8 take_ownership);

/*! Load image from an image file that is supported by this renderer.  Don't forget to GPU_FreeImage() it. */
DECLSPEC GPU_Image* SDLCALL GPU_LoadImage(const char* filename);

/*! Creates an image that aliases the given image.  Aliases can be used to store image settings (e.g. modulation color) for easy switching.
 * GPU_FreeImage() frees the alias's memory, but does not affect the original. */
DECLSPEC GPU_Image* SDLCALL GPU_CreateAliasImage(GPU_Image* image);

/*! Copy an image to a new image.  Don't forget to GPU_FreeImage() both. */
DECLSPEC GPU_Image* SDLCALL GPU_CopyImage(GPU_Image* image);

/*! Deletes an image in the proper way for this renderer.  Also deletes the corresponding GPU_Target if applicable.  Be careful not to use that target afterward! */
DECLSPEC void SDLCALL GPU_FreeImage(GPU_Image* image);

/*! Update an image from surface data. */
DECLSPEC void SDLCALL GPU_UpdateImage(GPU_Image* image, SDL_Surface* surface, const GPU_Rect* surface_rect);

/*! Update an image from surface data. */
DECLSPEC void SDLCALL GPU_UpdateSubImage(GPU_Image* image, const GPU_Rect* image_rect, SDL_Surface* surface, const GPU_Rect* surface_rect);

/*! Update an image from an array of pixel data. */
DECLSPEC void SDLCALL GPU_UpdateImageBytes(GPU_Image* image, const GPU_Rect* image_rect, const unsigned char* bytes, int bytes_per_row);

/*! Save image to a file.  The file type is deduced from the extension.  Supported formats are: png, bmp, tga.  Returns 0 on failure. */
DECLSPEC Uint8 SDLCALL GPU_SaveImage(GPU_Image* image, const char* filename);

/*! Loads mipmaps for the given image, if supported by the renderer. */
DECLSPEC void SDLCALL GPU_GenerateMipmaps(GPU_Image* image);

/*! Sets the modulation color for subsequent drawing of the given image. */
DECLSPEC void SDLCALL GPU_SetColor(GPU_Image* image, SDL_Color color);

/*! Sets the modulation color for subsequent drawing of the given image. */
DECLSPEC void SDLCALL GPU_SetRGB(GPU_Image* image, Uint8 r, Uint8 g, Uint8 b);

/*! Sets the modulation color for subsequent drawing of the given image. */
DECLSPEC void SDLCALL GPU_SetRGBA(GPU_Image* image, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/*! Unsets the modulation color for subsequent drawing of the given image.
 *  This is equivalent to coloring with pure opaque white (255, 255, 255, 255). */
DECLSPEC void SDLCALL GPU_UnsetColor(GPU_Image* image);

/*! Gets the current alpha blending setting. */
DECLSPEC Uint8 SDLCALL GPU_GetBlending(GPU_Image* image);

/*! Enables/disables alpha blending for the given image. */
DECLSPEC void SDLCALL GPU_SetBlending(GPU_Image* image, Uint8 enable);

/*! Sets the blending component functions. */
DECLSPEC void SDLCALL GPU_SetBlendFunction(GPU_Image* image, GPU_BlendFuncEnum source_color, GPU_BlendFuncEnum dest_color, GPU_BlendFuncEnum source_alpha, GPU_BlendFuncEnum dest_alpha);

/*! Sets the blending component equations. */
DECLSPEC void SDLCALL GPU_SetBlendEquation(GPU_Image* image, GPU_BlendEqEnum color_equation, GPU_BlendEqEnum alpha_equation);

/*! Sets the blending mode, if supported by the renderer. */
DECLSPEC void SDLCALL GPU_SetBlendMode(GPU_Image* image, GPU_BlendPresetEnum mode);

/*! Sets the image filtering mode, if supported by the renderer. */
DECLSPEC void SDLCALL GPU_SetImageFilter(GPU_Image* image, GPU_FilterEnum filter);

/*! Gets the current pixel snap setting.  The default value is GPU_SNAP_POSITION_AND_DIMENSIONS.  */
DECLSPEC GPU_SnapEnum SDLCALL GPU_GetSnapMode(GPU_Image* image);

/*! Sets the pixel grid snapping mode for the given image. */
DECLSPEC void SDLCALL GPU_SetSnapMode(GPU_Image* image, GPU_SnapEnum mode);

/*! Sets the image wrapping mode, if supported by the renderer. */
DECLSPEC void SDLCALL GPU_SetWrapMode(GPU_Image* image, GPU_WrapEnum wrap_mode_x, GPU_WrapEnum wrap_mode_y);

// End of ImageControls
/*! @} */


// Surface / Image / Target conversions
/*! \ingroup Conversions
 *  @{ */

/*! Copy SDL_Surface data into a new GPU_Image.  Don't forget to SDL_FreeSurface() the surface and GPU_FreeImage() the image.*/
DECLSPEC GPU_Image* SDLCALL GPU_CopyImageFromSurface(SDL_Surface* surface);

/*! Copy GPU_Target data into a new GPU_Image.  Don't forget to GPU_FreeImage() the image.*/
DECLSPEC GPU_Image* SDLCALL GPU_CopyImageFromTarget(GPU_Target* target);

/*! Copy GPU_Target data into a new SDL_Surface.  Don't forget to SDL_FreeSurface() the surface.*/
DECLSPEC SDL_Surface* SDLCALL GPU_CopySurfaceFromTarget(GPU_Target* target);

/*! Copy GPU_Image data into a new SDL_Surface.  Don't forget to SDL_FreeSurface() the surface and GPU_FreeImage() the image.*/
DECLSPEC SDL_Surface* SDLCALL GPU_CopySurfaceFromImage(GPU_Image* image);

// End of Conversions
/*! @} */





/*! \ingroup Matrix
 *  @{ */


// Basic matrix operations (4x4)

/*! Copy matrix A to the given 'result' matrix. */
DECLSPEC void SDLCALL GPU_MatrixCopy(float* result, const float* A);

/*! Fills 'result' matrix with the identity matrix. */
DECLSPEC void SDLCALL GPU_MatrixIdentity(float* result);

/*! Multiplies matrices A and B and stores the result in the given 'result' matrix (result = A*B).  Do not use A or B as 'result'.
 * \see GPU_MultiplyAndAssign
*/
DECLSPEC void SDLCALL GPU_Multiply4x4(float* result, float* A, float* B);

/*! Multiplies matrices 'result' and A and stores the result in the given 'result' matrix (result = result * A). */
DECLSPEC void SDLCALL GPU_MultiplyAndAssign(float* result, float* A);


// Matrix stack accessors

/*! Returns an internal string that represents the contents of matrix A. */
DECLSPEC const char* SDLCALL GPU_GetMatrixString(float* A);

/*! Returns the current matrix from the top of the matrix stack.  Returns NULL if stack is empty. */
DECLSPEC float* SDLCALL GPU_GetCurrentMatrix(void);

/*! Returns the current modelview matrix from the top of the matrix stack.  Returns NULL if stack is empty. */
DECLSPEC float* SDLCALL GPU_GetModelView(void);

/*! Returns the current projection matrix from the top of the matrix stack.  Returns NULL if stack is empty. */
DECLSPEC float* SDLCALL GPU_GetProjection(void);

/*! Copies the current modelview-projection matrix into the given 'result' matrix (result = P*M). */
DECLSPEC void SDLCALL GPU_GetModelViewProjection(float* result);


// Matrix stack manipulators

/*! Changes matrix mode to either GPU_PROJECTION or GPU_MODELVIEW.  Further matrix stack operations manipulate that particular stack. */
DECLSPEC void SDLCALL GPU_MatrixMode(int matrix_mode);

/*! Pushes the current matrix as a new matrix stack item. */
DECLSPEC void SDLCALL GPU_PushMatrix(void);

/*! Removes the current matrix from the stack. */
DECLSPEC void SDLCALL GPU_PopMatrix(void);

/*! Fills current matrix with the identity matrix. */
DECLSPEC void SDLCALL GPU_LoadIdentity(void);

/*! Multiplies an orthographic projection matrix into the current matrix. */
DECLSPEC void SDLCALL GPU_Ortho(float left, float right, float bottom, float top, float near, float far);

/*! Multiplies a perspective projection matrix into the current matrix. */
DECLSPEC void SDLCALL GPU_Frustum(float right, float left, float bottom, float top, float near, float far);

/*! Adds a translation into the current matrix. */
DECLSPEC void SDLCALL GPU_Translate(float x, float y, float z);

/*! Multiplies a scaling matrix into the current matrix. */
DECLSPEC void SDLCALL GPU_Scale(float sx, float sy, float sz);

/*! Multiplies a rotation matrix into the current matrix. */
DECLSPEC void SDLCALL GPU_Rotate(float degrees, float x, float y, float z);

/*! Multiplies a given matrix into the current matrix. */
DECLSPEC void SDLCALL GPU_MultMatrix(float* matrix4x4);

// End of Matrix
/*! @} */






/*! \ingroup Rendering
 *  @{ */

/*! Clears the contents of the given render target.  Fills the target with color {0, 0, 0, 0}. */
DECLSPEC void SDLCALL GPU_Clear(GPU_Target* target);

/*! Fills the given render target with a color. */
DECLSPEC void SDLCALL GPU_ClearColor(GPU_Target* target, SDL_Color color);

/*! Fills the given render target with a color (alpha is 255, fully opaque). */
DECLSPEC void SDLCALL GPU_ClearRGB(GPU_Target* target, Uint8 r, Uint8 g, Uint8 b);

/*! Fills the given render target with a color. */
DECLSPEC void SDLCALL GPU_ClearRGBA(GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/*! Draws the given image to the given render target.
    * \param src_rect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position */
DECLSPEC void SDLCALL GPU_Blit(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y);

/*! Rotates and draws the given image to the given render target.
    * \param src_rect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position
    * \param degrees Rotation angle (in degrees) */
DECLSPEC void SDLCALL GPU_BlitRotate(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees);

/*! Scales and draws the given image to the given render target.
    * \param src_rect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position
    * \param scaleX Horizontal stretch factor
    * \param scaleY Vertical stretch factor */
DECLSPEC void SDLCALL GPU_BlitScale(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float scaleX, float scaleY);

/*! Scales, rotates, and draws the given image to the given render target.
    * \param src_rect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position
    * \param degrees Rotation angle (in degrees)
    * \param scaleX Horizontal stretch factor
    * \param scaleY Vertical stretch factor */
DECLSPEC void SDLCALL GPU_BlitTransform(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees, float scaleX, float scaleY);

/*! Scales, rotates around a pivot point, and draws the given image to the given render target.  The drawing point (x, y) coincides with the pivot point on the src image (pivot_x, pivot_y).
	* \param src_rect The region of the source image to use.
	* \param x Destination x-position
	* \param y Destination y-position
	* \param pivot_x Pivot x-position (in image coordinates)
	* \param pivot_y Pivot y-position (in image coordinates)
	* \param degrees Rotation angle (in degrees)
	* \param scaleX Horizontal stretch factor
	* \param scaleY Vertical stretch factor */
DECLSPEC void SDLCALL GPU_BlitTransformX(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float pivot_x, float pivot_y, float degrees, float scaleX, float scaleY);

/*! Transforms and draws the given image to the given render target.
	* \param src_rect The region of the source image to use.
	* \param x Destination x-position
	* \param y Destination y-position
	* \param matrix3x3 3x3 matrix in column-major order (index = row + column*numColumns) */
DECLSPEC void SDLCALL GPU_BlitTransformMatrix(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float* matrix3x3);

/*! Performs 'num_sprites' blits of the given image to the given target.
 * Note: GPU_BlitBatch() cannot interpret a mix of normal values and "passthrough" values due to format ambiguity.
 * \param values A tightly-packed array of position (x,y), src_rect (x,y,w,h) values in image coordinates, and color (r,g,b,a) values with a range from 0-255.  Pass NULL to render with only custom shader attributes.
 * \param flags Bit flags to control the interpretation of the array parameters.  The only passthrough option accepted is GPU_PASSTHROUGH_ALL.
 */
DECLSPEC void SDLCALL GPU_BlitBatch(GPU_Image* image, GPU_Target* target, unsigned int num_sprites, float* values, GPU_BlitFlagEnum flags);

/*! Performs 'num_sprites' blits of the given image to the given target.
 * \param positions A tightly-packed array of (x,y) values
 * \param src_rects A tightly-packed array of (x,y,w,h) values in image coordinates
 * \param colors A tightly-packed array of (r,g,b,a) values with a range from 0-255
 * \param flags Bit flags to control the interpretation of the array parameters
 */
DECLSPEC void SDLCALL GPU_BlitBatchSeparate(GPU_Image* image, GPU_Target* target, unsigned int num_sprites, float* positions, float* src_rects, float* colors, GPU_BlitFlagEnum flags);

/*! Renders triangles from the given set of vertices.  This lets you render arbitrary 2D geometry.
 * \param values A tightly-packed array of vertex position (x,y), image coordinates (s,t), and color (r,g,b,a) values with a range from 0-255.  Pass NULL to render with only custom shader attributes.
 * \param indices If not NULL, this is used to specify which vertices to use and in what order (i.e. it indexes the vertices in the 'values' array).
 * \param flags Bit flags to control the interpretation of the array parameters.  Since 'values' contains per-vertex data, GPU_PASSTHROUGH_VERTICES is ignored.  Texture coordinates are scaled down using the image dimensions and color components are normalized to [0.0, 1.0].
 */
DECLSPEC void SDLCALL GPU_TriangleBatch(GPU_Image* image, GPU_Target* target, unsigned short num_vertices, float* values, unsigned int num_indices, unsigned short* indices, GPU_BlitFlagEnum flags);

/*! Send all buffered blitting data to the current context target. */
DECLSPEC void SDLCALL GPU_FlushBlitBuffer(void);

/*! Updates the given target's associated window. */
DECLSPEC void SDLCALL GPU_Flip(GPU_Target* target);

// End of Rendering
/*! @} */





/*! \ingroup Shapes
 *  @{ */

/*! Renders a colored point.
 * \param target The destination render target
 * \param x x-coord of the point
 * \param y y-coord of the point
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_Pixel(GPU_Target* target, float x, float y, SDL_Color color);

/*! Renders a colored line.
 * \param target The destination render target
 * \param x1 x-coord of starting point
 * \param y1 y-coord of starting point
 * \param x2 x-coord of ending point
 * \param y2 y-coord of ending point
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_Line(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

/*! Renders a colored arc curve (circle segment).
 * \param target The destination render target
 * \param x x-coord of center point
 * \param y y-coord of center point
 * \param radius The radius of the circle / distance from the center point that rendering will occur
 * \param start_angle The angle to start from, in degrees.  Measured clockwise from the positive x-axis.
 * \param end_angle The angle to end at, in degrees.  Measured clockwise from the positive x-axis.
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_Arc(GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color);

/*! Renders a colored filled arc (circle segment / pie piece).
 * \param target The destination render target
 * \param x x-coord of center point
 * \param y y-coord of center point
 * \param radius The radius of the circle / distance from the center point that rendering will occur
 * \param start_angle The angle to start from, in degrees.  Measured clockwise from the positive x-axis.
 * \param end_angle The angle to end at, in degrees.  Measured clockwise from the positive x-axis.
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_ArcFilled(GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color);

/*! Renders a colored circle outline.
 * \param target The destination render target
 * \param x x-coord of center point
 * \param y y-coord of center point
 * \param radius The radius of the circle / distance from the center point that rendering will occur
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_Circle(GPU_Target* target, float x, float y, float radius, SDL_Color color);

/*! Renders a colored filled circle.
 * \param target The destination render target
 * \param x x-coord of center point
 * \param y y-coord of center point
 * \param radius The radius of the circle / distance from the center point that rendering will occur
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_CircleFilled(GPU_Target* target, float x, float y, float radius, SDL_Color color);

/*! Renders a colored ellipse outline.
 * \param target The destination render target
 * \param x x-coord of center point
 * \param y y-coord of center point
 * \param rx x-radius of ellipse
 * \param ry y-radius of ellipse
 * \param degrees The angle to rotate the ellipse
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_Ellipse(GPU_Target* target, float x, float y, float rx, float ry, float degrees, SDL_Color color);

/*! Renders a colored filled ellipse.
 * \param target The destination render target
 * \param x x-coord of center point
 * \param y y-coord of center point
 * \param rx x-radius of ellipse
 * \param ry y-radius of ellipse
 * \param degrees The angle to rotate the ellipse
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_EllipseFilled(GPU_Target* target, float x, float y, float rx, float ry, float degrees, SDL_Color color);

/*! Renders a colored annular sector outline (ring segment).
 * \param target The destination render target
 * \param x x-coord of center point
 * \param y y-coord of center point
 * \param inner_radius The inner radius of the ring
 * \param outer_radius The outer radius of the ring
 * \param start_angle The angle to start from, in degrees.  Measured clockwise from the positive x-axis.
 * \param end_angle The angle to end at, in degrees.  Measured clockwise from the positive x-axis.
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_Sector(GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color);

/*! Renders a colored filled annular sector (ring segment).
 * \param target The destination render target
 * \param x x-coord of center point
 * \param y y-coord of center point
 * \param inner_radius The inner radius of the ring
 * \param outer_radius The outer radius of the ring
 * \param start_angle The angle to start from, in degrees.  Measured clockwise from the positive x-axis.
 * \param end_angle The angle to end at, in degrees.  Measured clockwise from the positive x-axis.
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_SectorFilled(GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color);

/*! Renders a colored triangle outline.
 * \param target The destination render target
 * \param x1 x-coord of first point
 * \param y1 y-coord of first point
 * \param x2 x-coord of second point
 * \param y2 y-coord of second point
 * \param x3 x-coord of third point
 * \param y3 y-coord of third point
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_Tri(GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

/*! Renders a colored filled triangle.
 * \param target The destination render target
 * \param x1 x-coord of first point
 * \param y1 y-coord of first point
 * \param x2 x-coord of second point
 * \param y2 y-coord of second point
 * \param x3 x-coord of third point
 * \param y3 y-coord of third point
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_TriFilled(GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

/*! Renders a colored rectangle outline.
 * \param target The destination render target
 * \param x1 x-coord of top-left corner
 * \param y1 y-coord of top-left corner
 * \param x2 x-coord of bottom-right corner
 * \param y2 y-coord of bottom-right corner
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_Rectangle(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

/*! Renders a colored filled rectangle.
 * \param target The destination render target
 * \param x1 x-coord of top-left corner
 * \param y1 y-coord of top-left corner
 * \param x2 x-coord of bottom-right corner
 * \param y2 y-coord of bottom-right corner
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_RectangleFilled(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

/*! Renders a colored rounded (filleted) rectangle outline.
 * \param target The destination render target
 * \param x1 x-coord of top-left corner
 * \param y1 y-coord of top-left corner
 * \param x2 x-coord of bottom-right corner
 * \param y2 y-coord of bottom-right corner
 * \param radius The radius of the corners
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_RectangleRound(GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

/*! Renders a colored filled rounded (filleted) rectangle.
 * \param target The destination render target
 * \param x1 x-coord of top-left corner
 * \param y1 y-coord of top-left corner
 * \param x2 x-coord of bottom-right corner
 * \param y2 y-coord of bottom-right corner
 * \param radius The radius of the corners
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_RectangleRoundFilled(GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

/*! Renders a colored polygon outline.  The vertices are expected to define a convex polygon.
 * \param target The destination render target
 * \param num_vertices Number of vertices (x and y pairs)
 * \param vertices An array of vertex positions stored as interlaced x and y coords, e.g. {x1, y1, x2, y2, ...}
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_Polygon(GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color);

/*! Renders a colored filled polygon.  The vertices are expected to define a convex polygon.
 * \param target The destination render target
 * \param num_vertices Number of vertices (x and y pairs)
 * \param vertices An array of vertex positions stored as interlaced x and y coords, e.g. {x1, y1, x2, y2, ...}
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_PolygonFilled(GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color);

// End of Shapes
/*! @} */





/*! \ingroup ShaderInterface
 *  @{ */

/*! Creates a new, empty shader program.  You will need to compile shaders, attach them to the program, then link the program.
 * \see GPU_AttachShader
 * \see GPU_LinkShaderProgram
 */
DECLSPEC Uint32 SDLCALL GPU_CreateShaderProgram(void);

/*! Deletes a shader program. */
DECLSPEC void SDLCALL GPU_FreeShaderProgram(Uint32 program_object);

/*! Loads shader source from an SDL_RWops, compiles it, and returns the new shader object. */
DECLSPEC Uint32 SDLCALL GPU_CompileShader_RW(GPU_ShaderEnum shader_type, SDL_RWops* shader_source);

/*! Compiles shader source and returns the new shader object. */
DECLSPEC Uint32 SDLCALL GPU_CompileShader(GPU_ShaderEnum shader_type, const char* shader_source);

/*! Loads shader source from a file, compiles it, and returns the new shader object. */
DECLSPEC Uint32 SDLCALL GPU_LoadShader(GPU_ShaderEnum shader_type, const char* filename);

/*! Creates and links a shader program with the given shader objects. */
DECLSPEC Uint32 SDLCALL GPU_LinkShaders(Uint32 shader_object1, Uint32 shader_object2);

/*! Deletes a shader object. */
DECLSPEC void SDLCALL GPU_FreeShader(Uint32 shader_object);

/*! Attaches a shader object to a shader program for future linking. */
DECLSPEC void SDLCALL GPU_AttachShader(Uint32 program_object, Uint32 shader_object);

/*! Detaches a shader object from a shader program. */
DECLSPEC void SDLCALL GPU_DetachShader(Uint32 program_object, Uint32 shader_object);

/*! Links a shader program with any attached shader objects. */
DECLSPEC Uint8 SDLCALL GPU_LinkShaderProgram(Uint32 program_object);

/*! \return The current shader program */
DECLSPEC Uint32 SDLCALL GPU_GetCurrentShaderProgram(void);

/*! Returns 1 if the given shader program is a default shader for the current context, 0 otherwise. */
DECLSPEC Uint8 SDLCALL GPU_IsDefaultShaderProgram(Uint32 program_object);

/*! Activates the given shader program.  Passing NULL for 'block' will disable the built-in shader variables for custom shaders until a GPU_ShaderBlock is set again. */
DECLSPEC void SDLCALL GPU_ActivateShaderProgram(Uint32 program_object, GPU_ShaderBlock* block);

/*! Deactivates the current shader program (activates program 0). */
DECLSPEC void SDLCALL GPU_DeactivateShaderProgram(void);

/*! Returns the last shader log message. */
DECLSPEC const char* SDLCALL GPU_GetShaderMessage(void);

/*! Returns an integer representing the location of the specified attribute shader variable. */
DECLSPEC int SDLCALL GPU_GetAttributeLocation(Uint32 program_object, const char* attrib_name);

/*! Returns a filled GPU_AttributeFormat object. */
DECLSPEC GPU_AttributeFormat SDLCALL GPU_MakeAttributeFormat(int num_elems_per_vertex, GPU_TypeEnum type, Uint8 normalize, int stride_bytes, int offset_bytes);

/*! Returns a filled GPU_Attribute object. */
DECLSPEC GPU_Attribute SDLCALL GPU_MakeAttribute(int location, void* values, GPU_AttributeFormat format);

/*! Returns an integer representing the location of the specified uniform shader variable. */
DECLSPEC int SDLCALL GPU_GetUniformLocation(Uint32 program_object, const char* uniform_name);

/*! Loads the given shader program's built-in attribute and uniform locations. */
DECLSPEC GPU_ShaderBlock SDLCALL GPU_LoadShaderBlock(Uint32 program_object, const char* position_name, const char* texcoord_name, const char* color_name, const char* modelViewMatrix_name);

/*! Sets the current shader block to use the given attribute and uniform locations. */
DECLSPEC void SDLCALL GPU_SetShaderBlock(GPU_ShaderBlock block);

/*! Sets the given image unit to the given image so that a custom shader can sample multiple textures.
    \param image The source image/texture.  Pass NULL to disable the image unit.
    \param location The uniform location of a texture sampler
    \param image_unit The index of the texture unit to set.  0 is the first unit, which is used by SDL_gpu's blitting functions.  1 would be the second unit. */
DECLSPEC void SDLCALL GPU_SetShaderImage(GPU_Image* image, int location, int image_unit);

/*! Fills "values" with the value of the uniform shader variable at the given location. */
DECLSPEC void SDLCALL GPU_GetUniformiv(Uint32 program_object, int location, int* values);

/*! Sets the value of the integer uniform shader variable at the given location.
    This is equivalent to calling GPU_SetUniformiv(location, 1, 1, &value). */
DECLSPEC void SDLCALL GPU_SetUniformi(int location, int value);

/*! Sets the value of the integer uniform shader variable at the given location. */
DECLSPEC void SDLCALL GPU_SetUniformiv(int location, int num_elements_per_value, int num_values, int* values);

/*! Fills "values" with the value of the uniform shader variable at the given location. */
DECLSPEC void SDLCALL GPU_GetUniformuiv(Uint32 program_object, int location, unsigned int* values);

/*! Sets the value of the unsigned integer uniform shader variable at the given location.
    This is equivalent to calling GPU_SetUniformuiv(location, 1, 1, &value). */
DECLSPEC void SDLCALL GPU_SetUniformui(int location, unsigned int value);

/*! Sets the value of the unsigned integer uniform shader variable at the given location. */
DECLSPEC void SDLCALL GPU_SetUniformuiv(int location, int num_elements_per_value, int num_values, unsigned int* values);

/*! Fills "values" with the value of the uniform shader variable at the given location. */
DECLSPEC void SDLCALL GPU_GetUniformfv(Uint32 program_object, int location, float* values);

/*! Sets the value of the floating point uniform shader variable at the given location.
    This is equivalent to calling GPU_SetUniformfv(location, 1, 1, &value). */
DECLSPEC void SDLCALL GPU_SetUniformf(int location, float value);

/*! Sets the value of the floating point uniform shader variable at the given location. */
DECLSPEC void SDLCALL GPU_SetUniformfv(int location, int num_elements_per_value, int num_values, float* values);

/*! Fills "values" with the value of the uniform shader variable at the given location.  The results are identical to calling GPU_GetUniformfv().  Matrices are gotten in column-major order. */
DECLSPEC void SDLCALL GPU_GetUniformMatrixfv(Uint32 program_object, int location, float* values);

/*! Sets the value of the matrix uniform shader variable at the given location.  The size of the matrices sent is specified by num_rows and num_columns.  Rows and columns must be between 2 and 4. */
DECLSPEC void SDLCALL GPU_SetUniformMatrixfv(int location, int num_matrices, int num_rows, int num_columns, Uint8 transpose, float* values);

/*! Sets a constant-value shader attribute that will be used for each rendered vertex. */
DECLSPEC void SDLCALL GPU_SetAttributef(int location, float value);

/*! Sets a constant-value shader attribute that will be used for each rendered vertex. */
DECLSPEC void SDLCALL GPU_SetAttributei(int location, int value);

/*! Sets a constant-value shader attribute that will be used for each rendered vertex. */
DECLSPEC void SDLCALL GPU_SetAttributeui(int location, unsigned int value);

/*! Sets a constant-value shader attribute that will be used for each rendered vertex. */
DECLSPEC void SDLCALL GPU_SetAttributefv(int location, int num_elements, float* value);

/*! Sets a constant-value shader attribute that will be used for each rendered vertex. */
DECLSPEC void SDLCALL GPU_SetAttributeiv(int location, int num_elements, int* value);

/*! Sets a constant-value shader attribute that will be used for each rendered vertex. */
DECLSPEC void SDLCALL GPU_SetAttributeuiv(int location, int num_elements, unsigned int* value);

/*! Enables a shader attribute and sets its source data. */
DECLSPEC void SDLCALL GPU_SetAttributeSource(int num_values, GPU_Attribute source);

// End of ShaderInterface
/*! @} */


#ifdef __cplusplus
}
#endif

#include "close_code.h"


#endif

