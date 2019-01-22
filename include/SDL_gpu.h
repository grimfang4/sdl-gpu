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
#define SDL_GPU_VERSION_MINOR 11
#define SDL_GPU_VERSION_PATCH 0

/* Auto-detect if we're using the SDL2 API by the headers available. */
#if SDL_VERSION_ATLEAST(2,0,0)
    #define SDL_GPU_USE_SDL2
#else
    #define SDL_GPU_USE_SDL1
#endif


// Check for bool support
#ifdef __STDC_VERSION__
    #define GPU_HAVE_STDC 1
#else
    #define GPU_HAVE_STDC 0
#endif

#define GPU_HAVE_C99 (GPU_HAVE_STDC && (__STDC_VERSION__ >= 199901L))

#ifdef __GNUC__ // catches both gcc and clang I believe
    #define GPU_HAVE_GNUC 1
#else
    #define GPU_HAVE_GNUC 0
#endif

#ifdef _MSC_VER
    #define GPU_HAVE_MSVC 1
#else
    #define GPU_HAVE_MSVC 0
#endif

#define GPU_HAVE_MSVC18 (GPU_HAVE_MSVC && (_MSC_VER >= 1800)) // VS2013+

#if defined(GPU_USE_REAL_BOOL) && GPU_USE_REAL_BOOL  // allow user to specify
    #define GPU_bool bool
#elif defined(GPU_USE_INT_BOOL) && GPU_USE_INT_BOOL
    #define GPU_bool int
#elif GPU_HAVE_C99 || GPU_HAVE_GNUC || GPU_HAVE_MSVC18 || (defined(GPU_HAVE_STDBOOL) && GPU_HAVE_STDBOOL)
    #include <stdbool.h>
    #define GPU_bool bool
#else
    #define GPU_bool int
#endif

#define GPU_FALSE 0
#define GPU_TRUE 1


typedef struct GPU_Renderer GPU_Renderer;
typedef struct GPU_Target GPU_Target;

/*!
 * \defgroup Initialization Initialization
 * \defgroup Logging Debugging, Logging, and Error Handling
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
} GPU_RendererID;


/*! \ingroup TargetControls
 * Comparison operations (for depth testing)
 * \see GPU_SetDepthFunction()
 * Values chosen for direct OpenGL compatibility.
 */
typedef enum {
    GPU_NEVER = 0x0200,
    GPU_LESS = 0x0201,
    GPU_EQUAL = 0x0202,
    GPU_LEQUAL = 0x0203,
    GPU_GREATER = 0x0204,
    GPU_NOTEQUAL = 0x0205,
    GPU_GEQUAL = 0x0206,
    GPU_ALWAYS = 0x0207
} GPU_ComparisonEnum;


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
    GPU_BLEND_NORMAL_ADD_ALPHA = 9,
    GPU_BLEND_NORMAL_FACTOR_ALPHA = 10
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
    GPU_FORMAT_YCbCr420P = 8,
    GPU_FORMAT_BGR = 9,
    GPU_FORMAT_BGRA = 10,
    GPU_FORMAT_ABGR = 11
} GPU_FormatEnum;

/*! \ingroup ImageControls
 * File format enum
 * \see GPU_SaveSurface()
 * \see GPU_SaveImage()
 * \see GPU_SaveSurface_RW()
 * \see GPU_SaveImage_RW()
 */
typedef enum {
    GPU_FILE_AUTO = 0,
    GPU_FILE_PNG,
    GPU_FILE_BMP,
    GPU_FILE_TGA
} GPU_FileFormatEnum;



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
	GPU_Target* context_target;
	GPU_Target* target;
	Uint16 w, h;
	GPU_bool using_virtual_resolution;
	GPU_FormatEnum format;
	int num_layers;
	int bytes_per_pixel;
	Uint16 base_w, base_h;  // Original image dimensions
	Uint16 texture_w, texture_h;  // Underlying texture dimensions
	GPU_bool has_mipmaps;
	
	float anchor_x; // Normalized coords for the point at which the image is blitted.  Default is (0.5, 0.5), that is, the image is drawn centered.
	float anchor_y; // These are interpreted according to GPU_SetCoordinateMode() and range from (0.0 - 1.0) normally.
	
	SDL_Color color;
	GPU_bool use_blending;
	GPU_BlendMode blend_mode;
	GPU_FilterEnum filter_mode;
	GPU_SnapEnum snap_mode;
	GPU_WrapEnum wrap_mode_x;
	GPU_WrapEnum wrap_mode_y;
	
	void* data;
	int refcount;
	GPU_bool is_alias;
} GPU_Image;

/*! \ingroup ImageControls
 * A backend-neutral type that is intended to hold a backend-specific handle/pointer to a texture.
 * \see GPU_CreateImageUsingTexture()
 * \see GPU_GetTextureHandle()
 */
typedef uintptr_t GPU_TextureHandle;


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
	float zoom_x, zoom_y;
	float z_near, z_far;  // z clipping planes
	bool use_centered_origin;  // move rotation/scaling origin to the center of the camera's view
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





#define GPU_MODEL 0
#define GPU_VIEW 1
#define GPU_PROJECTION 2

/*! \ingroup Matrix
 * Matrix stack data structure for global vertex transforms.  */
typedef struct GPU_MatrixStack
{
    unsigned int storage_size;
    unsigned int size;
    float** matrix;
} GPU_MatrixStack;


/*! \ingroup ContextControls
 * Rendering context data.  Only GPU_Targets which represent windows will store this. */
typedef struct GPU_Context
{
    /*! SDL_GLContext */
    void* context;
    GPU_bool failed;
    
    /*! SDL window ID */
	Uint32 windowID;
	
	/*! Actual window dimensions */
	int window_w;
	int window_h;
	
	/*! Drawable region dimensions */
	int drawable_w;
	int drawable_h;
	
	/*! Window dimensions for restoring windowed mode after GPU_SetFullscreen(1,1). */
	int stored_window_w;
	int stored_window_h;
	
	
	/*! Last target used */
	GPU_Target* active_target;
	
	/*! Internal state */
	Uint32 current_shader_program;
	Uint32 default_textured_shader_program;
	Uint32 default_untextured_shader_program;
	
    GPU_ShaderBlock current_shader_block;
    GPU_ShaderBlock default_textured_shader_block;
    GPU_ShaderBlock default_untextured_shader_block;
	
	GPU_bool shapes_use_blending;
	GPU_BlendMode shapes_blend_mode;
	float line_thickness;
	GPU_bool use_texturing;
    
	int refcount;
	
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
	GPU_Target* context_target;
	GPU_Image* image;
	void* data;
	Uint16 w, h;
	GPU_bool using_virtual_resolution;
	Uint16 base_w, base_h;  // The true dimensions of the underlying image or window
	GPU_bool use_clip_rect;
	GPU_Rect clip_rect;
	GPU_bool use_color;
	SDL_Color color;
	
	GPU_Rect viewport;
	
	/*! Perspective and object viewing transforms. */
	int matrix_mode;
	GPU_MatrixStack projection_matrix;
	GPU_MatrixStack view_matrix;
	GPU_MatrixStack model_matrix;

	GPU_Camera camera;
	GPU_bool use_camera;

	
	GPU_bool use_depth_test;
	GPU_bool use_depth_write;
	GPU_ComparisonEnum depth_function;
	
	/*! Renderer context data.  NULL if the target does not represent a window or rendering context. */
	GPU_Context* context;
	int refcount;
	GPU_bool is_alias;
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
static const GPU_FeatureEnum GPU_FEATURE_CORE_FRAMEBUFFER_OBJECTS = 0x1000;

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
static const GPU_InitFlagEnum GPU_INIT_REQUEST_COMPATIBILITY_PROFILE = 0x10;
static const GPU_InitFlagEnum GPU_INIT_USE_ROW_BY_ROW_TEXTURE_UPLOAD_FALLBACK = 0x20;
static const GPU_InitFlagEnum GPU_INIT_USE_COPY_TEXTURE_UPLOAD_FALLBACK = 0x40;

#define GPU_DEFAULT_INIT_FLAGS 0


static const Uint32 GPU_NONE = 0x0;

/*! \ingroup Rendering
 * Primitive types for rendering arbitrary geometry.  The values are intentionally identical to the GL_* primitives.
 * \see GPU_PrimitiveBatch()
 * \see GPU_PrimitiveBatchV()
 */
typedef Uint32 GPU_PrimitiveEnum;
static const GPU_PrimitiveEnum GPU_POINTS = 0x0;
static const GPU_PrimitiveEnum GPU_LINES = 0x1;
static const GPU_PrimitiveEnum GPU_LINE_LOOP = 0x2;
static const GPU_PrimitiveEnum GPU_LINE_STRIP = 0x3;
static const GPU_PrimitiveEnum GPU_TRIANGLES = 0x4;
static const GPU_PrimitiveEnum GPU_TRIANGLE_STRIP = 0x5;
static const GPU_PrimitiveEnum GPU_TRIANGLE_FAN = 0x6;

 
/*! Bit flags for geometry batching.
 * \see GPU_TriangleBatch()
 * \see GPU_TriangleBatchX()
 * \see GPU_PrimitiveBatch()
 * \see GPU_PrimitiveBatchV()
 */
typedef Uint32 GPU_BatchFlagEnum;
static const GPU_BatchFlagEnum GPU_BATCH_XY = 0x1;
static const GPU_BatchFlagEnum GPU_BATCH_XYZ = 0x2;
static const GPU_BatchFlagEnum GPU_BATCH_ST = 0x4;
static const GPU_BatchFlagEnum GPU_BATCH_RGB = 0x8;
static const GPU_BatchFlagEnum GPU_BATCH_RGBA = 0x10;
static const GPU_BatchFlagEnum GPU_BATCH_RGB8 = 0x20;
static const GPU_BatchFlagEnum GPU_BATCH_RGBA8 = 0x40;

#define GPU_BATCH_XY_ST (GPU_BATCH_XY | GPU_BATCH_ST)
#define GPU_BATCH_XYZ_ST (GPU_BATCH_XYZ | GPU_BATCH_ST)
#define GPU_BATCH_XY_RGB (GPU_BATCH_XY | GPU_BATCH_RGB)
#define GPU_BATCH_XYZ_RGB (GPU_BATCH_XYZ | GPU_BATCH_RGB)
#define GPU_BATCH_XY_RGBA (GPU_BATCH_XY | GPU_BATCH_RGBA)
#define GPU_BATCH_XYZ_RGBA (GPU_BATCH_XYZ | GPU_BATCH_RGBA)
#define GPU_BATCH_XY_ST_RGBA (GPU_BATCH_XY | GPU_BATCH_ST | GPU_BATCH_RGBA)
#define GPU_BATCH_XYZ_ST_RGBA (GPU_BATCH_XYZ | GPU_BATCH_ST | GPU_BATCH_RGBA)
#define GPU_BATCH_XY_RGB8 (GPU_BATCH_XY | GPU_BATCH_RGB8)
#define GPU_BATCH_XYZ_RGB8 (GPU_BATCH_XYZ | GPU_BATCH_RGB8)
#define GPU_BATCH_XY_RGBA8 (GPU_BATCH_XY | GPU_BATCH_RGBA8)
#define GPU_BATCH_XYZ_RGBA8 (GPU_BATCH_XYZ | GPU_BATCH_RGBA8)
#define GPU_BATCH_XY_ST_RGBA8 (GPU_BATCH_XY | GPU_BATCH_ST | GPU_BATCH_RGBA8)
#define GPU_BATCH_XYZ_ST_RGBA8 (GPU_BATCH_XYZ | GPU_BATCH_ST | GPU_BATCH_RGBA8)


/*! Bit flags for blitting into a rectangular region.
 * \see GPU_BlitRect
 * \see GPU_BlitRectX
 */
typedef Uint32 GPU_FlipEnum;
static const GPU_FlipEnum GPU_FLIP_NONE = 0x0;
static const GPU_FlipEnum GPU_FLIP_HORIZONTAL = 0x1;
static const GPU_FlipEnum GPU_FLIP_VERTICAL = 0x2;


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
    GPU_bool is_per_sprite;  // Per-sprite values are expanded to 4 vertices
    int num_elems_per_value;
    GPU_TypeEnum type;  // GPU_TYPE_FLOAT, GPU_TYPE_INT, GPU_TYPE_UNSIGNED_INT, etc.
    GPU_bool normalize;
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
    GPU_bool enabled;
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


/*! \ingroup Logging
 * Type enumeration for logging levels.
 * \see GPU_SetLogCallback()
 */
typedef enum {
    GPU_LOG_INFO = 0,
    GPU_LOG_WARNING,
    GPU_LOG_ERROR
} GPU_LogLevelEnum;


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
	int min_shader_version;
	int max_shader_version;
    GPU_FeatureEnum enabled_features;
	
	/*! Current display target */
	GPU_Target* current_context_target;
	
	/*! 0 for inverted, 1 for mathematical */
	GPU_bool coordinate_mode;
	
	/*! Default is (0.5, 0.5) - images draw centered. */
	float default_image_anchor_x;
	float default_image_anchor_y;
	
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

/*! Initializes SDL's video subsystem (if necessary) and all of SDL_gpu's internal structures.
 * Chooses a renderer and creates a window with the given dimensions and window creation flags.
 * A pointer to the resulting window's render target is returned.
 * 
 * \param w Desired window width in pixels
 * \param h Desired window height in pixels
 * \param SDL_flags The bit flags to pass to SDL when creating the window.  Use GPU_DEFAULT_INIT_FLAGS if you don't care.
 * \return On success, returns the new context target (i.e. render target backed by a window).  On failure, returns NULL.
 * 
 * Initializes these systems:
 *  The 'error queue': Stores error codes and description strings.
 *  The 'renderer registry': An array of information about the supported renderers on the current platform,
 *    such as the renderer name and id and its life cycle functions.
 *  The SDL library and its video subsystem: Calls SDL_Init() if SDL has not already been initialized.
 *    Use SDL_InitSubsystem() to initialize more parts of SDL.
 *  The current renderer:  Walks through each renderer in the renderer registry and tries to initialize them until one succeeds.
 *
 * \see GPU_RendererID
 * \see GPU_InitRenderer()
 * \see GPU_InitRendererByID()
 * \see GPU_SetRendererOrder()
 * \see GPU_PushErrorCode()
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
DECLSPEC GPU_bool SDLCALL GPU_IsFeatureEnabled(GPU_FeatureEnum feature);

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

/*! Sets a custom callback for handling logging.  Use stdio's vsnprintf() to process the va_list into a string.  Passing NULL as the callback will reset to the default internal logging. */
DECLSPEC void SDLCALL GPU_SetLogCallback(int (*callback)(GPU_LogLevelEnum log_level, const char* format, va_list args));

/*! Pushes a new error code into the error queue.  If the queue is full, the queue is not modified.
 * \param function The name of the function that pushed the error
 * \param error The error code to push on the error queue
 * \param details Additional information string, can be NULL.
 */
DECLSPEC void SDLCALL GPU_PushErrorCode(const char* function, GPU_ErrorEnum error, const char* details, ...);

/*! Pops an error object from the error queue and returns it.  If the error queue is empty, it returns an error object with NULL function, GPU_ERROR_NONE error, and NULL details. */
DECLSPEC GPU_ErrorObject SDLCALL GPU_PopErrorCode(void);

/*! Gets the string representation of an error code. */
DECLSPEC const char* SDLCALL GPU_GetErrorString(GPU_ErrorEnum error);

/*! Changes the maximum number of error objects that SDL_gpu will store.  This deletes all currently stored errors. */
DECLSPEC void SDLCALL GPU_SetErrorQueueMax(unsigned int max);

// End of Logging
/*! @} */







/*! \ingroup RendererSetup
 *  @{ */

/*! Returns an initialized GPU_RendererID. */
DECLSPEC GPU_RendererID SDLCALL GPU_MakeRendererID(const char* name, GPU_RendererEnum renderer, int major_version, int minor_version);

/*! Gets the first registered renderer identifier for the given enum value. */
DECLSPEC GPU_RendererID SDLCALL GPU_GetRendererID(GPU_RendererEnum renderer);

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

/*! \return The current renderer */
DECLSPEC GPU_Renderer* SDLCALL GPU_GetCurrentRenderer(void);

/*! Switches the current renderer to the renderer matching the given identifier. */
DECLSPEC void SDLCALL GPU_SetCurrentRenderer(GPU_RendererID id);

/*! \return The renderer matching the given identifier. */
DECLSPEC GPU_Renderer* SDLCALL GPU_GetRenderer(GPU_RendererID id);

DECLSPEC void SDLCALL GPU_FreeRenderer(GPU_Renderer* renderer);

/*! Reapplies the renderer state to the backend API (e.g. OpenGL, Direct3D).  Use this if you want SDL_gpu to be able to render after you've used direct backend calls. */
DECLSPEC void SDLCALL GPU_ResetRendererState(void);

/*! Sets the coordinate mode for this renderer.  Target and image coordinates will be either "inverted" (0,0 is the upper left corner, y increases downward) or "mathematical" (0,0 is the bottom-left corner, y increases upward).
 * The default is inverted (0), as this is traditional for 2D graphics.
 * \param inverted 0 is for inverted coordinates, 1 is for mathematical coordinates */
DECLSPEC void SDLCALL GPU_SetCoordinateMode(GPU_bool use_math_coords);

DECLSPEC GPU_bool SDLCALL GPU_GetCoordinateMode(void);

/*! Sets the default image blitting anchor for newly created images.
 * \see GPU_SetAnchor
 */
DECLSPEC void SDLCALL GPU_SetDefaultAnchor(float anchor_x, float anchor_y);

/*! Returns the default image blitting anchor through the given variables.
 * \see GPU_GetAnchor
 */
DECLSPEC void SDLCALL GPU_GetDefaultAnchor(float* anchor_x, float* anchor_y);

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
DECLSPEC GPU_bool SDLCALL GPU_SetWindowResolution(Uint16 w, Uint16 h);

/*! Enable/disable fullscreen mode for the current context target's window.
 * On some platforms, this may destroy the renderer context and require that textures be reloaded.  Unfortunately, SDL does not provide a notification mechanism for this.
 * \param enable_fullscreen If true, make the application go fullscreen.  If false, make the application go to windowed mode.
 * \param use_desktop_resolution If true, lets the window change its resolution when it enters fullscreen mode (via SDL_WINDOW_FULLSCREEN_DESKTOP).
 * \return 0 if the new mode is windowed, 1 if the new mode is fullscreen.  */
DECLSPEC GPU_bool SDLCALL GPU_SetFullscreen(GPU_bool enable_fullscreen, GPU_bool use_desktop_resolution);

/*! Returns true if the current context target's window is in fullscreen mode. */
DECLSPEC GPU_bool SDLCALL GPU_GetFullscreen(void);

/*! \return Returns the last active target. */
DECLSPEC GPU_Target* SDLCALL GPU_GetActiveTarget(void);

/*! \return Sets the currently active target for matrix modification functions. */
DECLSPEC GPU_bool SDLCALL GPU_SetActiveTarget(GPU_Target* target);

/*! Enables/disables alpha blending for shape rendering on the current window. */
DECLSPEC void SDLCALL GPU_SetShapeBlending(GPU_bool enable);

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

/*! Creates a new render target from the given image.  It can then be accessed from image->target.  This increments the internal refcount of the target, so it should be matched with a GPU_FreeTarget(). */
DECLSPEC GPU_Target* SDLCALL GPU_LoadTarget(GPU_Image* image);

/*! Creates a new render target from the given image.  It can then be accessed from image->target.  This does not increment the internal refcount of the target, so it will be invalidated when the image is freed. */
DECLSPEC GPU_Target* SDLCALL GPU_GetTarget(GPU_Image* image);

/*! Deletes a render target in the proper way for this renderer. */
DECLSPEC void SDLCALL GPU_FreeTarget(GPU_Target* target);

/*! Change the logical size of the given target.  Rendering to this target will be scaled as if the dimensions were actually the ones given. */
DECLSPEC void SDLCALL GPU_SetVirtualResolution(GPU_Target* target, Uint16 w, Uint16 h);

/*! Query the logical size of the given target. */
DECLSPEC void SDLCALL GPU_GetVirtualResolution(GPU_Target* target, Uint16* w, Uint16* h);

/*! Converts screen space coordinates (such as from mouse input) to logical drawing coordinates.  This interacts with GPU_SetCoordinateMode() when the y-axis is flipped (screen space is assumed to be inverted: (0,0) in the upper-left corner). */
DECLSPEC void SDLCALL GPU_GetVirtualCoords(GPU_Target* target, float* x, float* y, float displayX, float displayY);

/*! Reset the logical size of the given target to its original value. */
DECLSPEC void SDLCALL GPU_UnsetVirtualResolution(GPU_Target* target);

/*! \return A GPU_Rect with the given values. */
DECLSPEC GPU_Rect SDLCALL GPU_MakeRect(float x, float y, float w, float h);

/*! \return An SDL_Color with the given values. */
DECLSPEC SDL_Color SDLCALL GPU_MakeColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/*! Sets the given target's viewport. */
DECLSPEC void SDLCALL GPU_SetViewport(GPU_Target* target, GPU_Rect viewport);

/*! Resets the given target's viewport to the entire target area. */
DECLSPEC void SDLCALL GPU_UnsetViewport(GPU_Target* target);

/*! \return A GPU_Camera with position (0, 0, 0), angle of 0, zoom of 1, centered origin, and near/far clipping planes of -100 and 100. */
DECLSPEC GPU_Camera SDLCALL GPU_GetDefaultCamera(void);

/*! \return The camera of the given render target.  If target is NULL, returns the default camera. */
DECLSPEC GPU_Camera SDLCALL GPU_GetCamera(GPU_Target* target);

/*! Sets the current render target's current camera.
 * \param target A pointer to the target that will copy this camera.
 * \param cam A pointer to the camera data to use or NULL to use the default camera.
 * \return The old camera. */
DECLSPEC GPU_Camera SDLCALL GPU_SetCamera(GPU_Target* target, GPU_Camera* cam);

/*! Enables or disables using the built-in camera matrix transforms. */
DECLSPEC void SDLCALL GPU_EnableCamera(GPU_Target* target, GPU_bool use_camera);

/*! Returns 1 if the camera transforms are enabled, 0 otherwise. */
DECLSPEC GPU_bool SDLCALL GPU_IsCameraEnabled(GPU_Target* target);

/*! Attach a new depth buffer to the given target so that it can use depth testing.  Context targets automatically have a depth buffer already.
 *  If successful, also enables depth testing for this target.
 */
DECLSPEC GPU_bool SDLCALL GPU_AddDepthBuffer(GPU_Target* target);

/*! Enables or disables the depth test, which will skip drawing pixels/fragments behind other fragments.  Disabled by default.
 *  This has implications for alpha blending, where compositing might not work correctly depending on render order.
 */
DECLSPEC void SDLCALL GPU_SetDepthTest(GPU_Target* target, GPU_bool enable);

/*! Enables or disables writing the depth (effective view z-coordinate) of new pixels to the depth buffer.  Enabled by default, but you must call GPU_SetDepthTest() to use it. */
DECLSPEC void SDLCALL GPU_SetDepthWrite(GPU_Target* target, GPU_bool enable);

/*! Sets the operation to perform when depth testing. */
DECLSPEC void SDLCALL GPU_SetDepthFunction(GPU_Target* target, GPU_ComparisonEnum compare_operation);

/*! \return The RGBA color of a pixel. */
DECLSPEC SDL_Color SDLCALL GPU_GetPixel(GPU_Target* target, Sint16 x, Sint16 y);

/*! Sets the clipping rect for the given render target. */
DECLSPEC GPU_Rect SDLCALL GPU_SetClipRect(GPU_Target* target, GPU_Rect rect);

/*! Sets the clipping rect for the given render target. */
DECLSPEC GPU_Rect SDLCALL GPU_SetClip(GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h);

/*! Turns off clipping for the given target. */
DECLSPEC void SDLCALL GPU_UnsetClip(GPU_Target* target);

/*! Returns GPU_TRUE if the given rects A and B overlap, in which case it also fills the given result rect with the intersection.  `result` can be NULL if you don't need the intersection. */
DECLSPEC GPU_bool SDLCALL GPU_IntersectRect(GPU_Rect A, GPU_Rect B, GPU_Rect* result);

/*! Returns GPU_TRUE if the given target's clip rect and the given B rect overlap, in which case it also fills the given result rect with the intersection.  `result` can be NULL if you don't need the intersection.
 * If the target doesn't have a clip rect enabled, this uses the whole target area.
 */
DECLSPEC GPU_bool SDLCALL GPU_IntersectClipRect(GPU_Target* target, GPU_Rect B, GPU_Rect* result);

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

/*! Load surface from an image file in memory.  Don't forget to SDL_FreeSurface() it. */
DECLSPEC SDL_Surface* SDLCALL GPU_LoadSurface_RW(SDL_RWops* rwops, GPU_bool free_rwops);

/*! Save surface to a file.
 * With a format of GPU_FILE_AUTO, the file type is deduced from the extension.  Supported formats are: png, bmp, tga.
 * Returns 0 on failure. */
DECLSPEC GPU_bool SDLCALL GPU_SaveSurface(SDL_Surface* surface, const char* filename, GPU_FileFormatEnum format);

/*! Save surface to a RWops stream.
 * Does not support format of GPU_FILE_AUTO, because the file type cannot be deduced.  Supported formats are: png, bmp, tga.
 * Returns 0 on failure. */
DECLSPEC GPU_bool SDLCALL GPU_SaveSurface_RW(SDL_Surface* surface, SDL_RWops* rwops, GPU_bool free_rwops, GPU_FileFormatEnum format);

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
DECLSPEC GPU_Image* SDLCALL GPU_CreateImageUsingTexture(GPU_TextureHandle handle, GPU_bool take_ownership);

/*! Load image from an image file that is supported by this renderer.  Don't forget to GPU_FreeImage() it. */
DECLSPEC GPU_Image* SDLCALL GPU_LoadImage(const char* filename);

/*! Load image from an image file in memory.  Don't forget to GPU_FreeImage() it. */
DECLSPEC GPU_Image* SDLCALL GPU_LoadImage_RW(SDL_RWops* rwops, GPU_bool free_rwops);

/*! Creates an image that aliases the given image.  Aliases can be used to store image settings (e.g. modulation color) for easy switching.
 * GPU_FreeImage() frees the alias's memory, but does not affect the original. */
DECLSPEC GPU_Image* SDLCALL GPU_CreateAliasImage(GPU_Image* image);

/*! Copy an image to a new image.  Don't forget to GPU_FreeImage() both. */
DECLSPEC GPU_Image* SDLCALL GPU_CopyImage(GPU_Image* image);

/*! Deletes an image in the proper way for this renderer.  Also deletes the corresponding GPU_Target if applicable.  Be careful not to use that target afterward! */
DECLSPEC void SDLCALL GPU_FreeImage(GPU_Image* image);

/*! Change the logical size of the given image.  Rendering this image will scaled it as if the dimensions were actually the ones given. */
DECLSPEC void SDLCALL GPU_SetImageVirtualResolution(GPU_Image* image, Uint16 w, Uint16 h);

/*! Reset the logical size of the given image to its original value. */
DECLSPEC void SDLCALL GPU_UnsetImageVirtualResolution(GPU_Image* image);

/*! Update an image from surface data.  Ignores virtual resolution on the image so the number of pixels needed from the surface is known. */
DECLSPEC void SDLCALL GPU_UpdateImage(GPU_Image* image, const GPU_Rect* image_rect, SDL_Surface* surface, const GPU_Rect* surface_rect);

/*! Update an image from an array of pixel data.  Ignores virtual resolution on the image so the number of pixels needed from the surface is known. */
DECLSPEC void SDLCALL GPU_UpdateImageBytes(GPU_Image* image, const GPU_Rect* image_rect, const unsigned char* bytes, int bytes_per_row);

/*! Update an image from surface data, replacing its underlying texture to allow for size changes.  Ignores virtual resolution on the image so the number of pixels needed from the surface is known. */
DECLSPEC GPU_bool SDLCALL GPU_ReplaceImage(GPU_Image* image, SDL_Surface* surface, const GPU_Rect* surface_rect);

/*! Save image to a file.
 * With a format of GPU_FILE_AUTO, the file type is deduced from the extension.  Supported formats are: png, bmp, tga.
 * Returns 0 on failure. */
DECLSPEC GPU_bool SDLCALL GPU_SaveImage(GPU_Image* image, const char* filename, GPU_FileFormatEnum format);

/*! Save image to a RWops stream.
 * Does not support format of GPU_FILE_AUTO, because the file type cannot be deduced.  Supported formats are: png, bmp, tga.
 * Returns 0 on failure. */
DECLSPEC GPU_bool SDLCALL GPU_SaveImage_RW(GPU_Image* image, SDL_RWops* rwops, GPU_bool free_rwops, GPU_FileFormatEnum format);

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
DECLSPEC GPU_bool SDLCALL GPU_GetBlending(GPU_Image* image);

/*! Enables/disables alpha blending for the given image. */
DECLSPEC void SDLCALL GPU_SetBlending(GPU_Image* image, GPU_bool enable);

/*! Sets the blending component functions. */
DECLSPEC void SDLCALL GPU_SetBlendFunction(GPU_Image* image, GPU_BlendFuncEnum source_color, GPU_BlendFuncEnum dest_color, GPU_BlendFuncEnum source_alpha, GPU_BlendFuncEnum dest_alpha);

/*! Sets the blending component equations. */
DECLSPEC void SDLCALL GPU_SetBlendEquation(GPU_Image* image, GPU_BlendEqEnum color_equation, GPU_BlendEqEnum alpha_equation);

/*! Sets the blending mode, if supported by the renderer. */
DECLSPEC void SDLCALL GPU_SetBlendMode(GPU_Image* image, GPU_BlendPresetEnum mode);

/*! Sets the image filtering mode, if supported by the renderer. */
DECLSPEC void SDLCALL GPU_SetImageFilter(GPU_Image* image, GPU_FilterEnum filter);

/*! Sets the image anchor, which is the point about which the image is blitted.  The default is to blit the image on-center (0.5, 0.5).  The anchor is in normalized coordinates (0.0-1.0). */
DECLSPEC void SDLCALL GPU_SetAnchor(GPU_Image* image, float anchor_x, float anchor_y);

/*! Returns the image anchor via the passed parameters.  The anchor is in normalized coordinates (0.0-1.0). */
DECLSPEC void SDLCALL GPU_GetAnchor(GPU_Image* image, float* anchor_x, float* anchor_y);

/*! Gets the current pixel snap setting.  The default value is GPU_SNAP_POSITION_AND_DIMENSIONS.  */
DECLSPEC GPU_SnapEnum SDLCALL GPU_GetSnapMode(GPU_Image* image);

/*! Sets the pixel grid snapping mode for the given image. */
DECLSPEC void SDLCALL GPU_SetSnapMode(GPU_Image* image, GPU_SnapEnum mode);

/*! Sets the image wrapping mode, if supported by the renderer. */
DECLSPEC void SDLCALL GPU_SetWrapMode(GPU_Image* image, GPU_WrapEnum wrap_mode_x, GPU_WrapEnum wrap_mode_y);

/*! Returns the backend-specific texture handle associated with the given image.  Note that SDL_gpu will be unaware of changes made to the texture.  */
DECLSPEC GPU_TextureHandle SDLCALL GPU_GetTextureHandle(GPU_Image* image);

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

// Basic vector operations (3D)

/*! Returns the magnitude (length) of the given vector. */
DECLSPEC float SDLCALL GPU_VectorLength(const float* vec3);

/*! Modifies the given vector so that it has a new length of 1. */
DECLSPEC void SDLCALL GPU_VectorNormalize(float* vec3);

/*! Returns the dot product of two vectors. */
DECLSPEC float SDLCALL GPU_VectorDot(const float* A, const float* B);

/*! Performs the cross product of vectors A and B (result = A x B).  Do not use A or B as 'result'. */
DECLSPEC void SDLCALL GPU_VectorCross(float* result, const float* A, const float* B);

/*! Overwrite 'result' vector with the values from vector A. */
DECLSPEC void SDLCALL GPU_VectorCopy(float* result, const float* A);

/*! Multiplies the given matrix into the given vector (vec3 = matrix*vec3). */
DECLSPEC void SDLCALL GPU_VectorApplyMatrix(float* vec3, const float* matrix_4x4);

/*! Multiplies the given matrix into the given vector (vec4 = matrix*vec4). */
DECLSPEC void SDLCALL GPU_Vector4ApplyMatrix(float* vec4, const float* matrix_4x4);



// Basic matrix operations (4x4)

/*! Overwrite 'result' matrix with the values from matrix A. */
DECLSPEC void SDLCALL GPU_MatrixCopy(float* result, const float* A);

/*! Fills 'result' matrix with the identity matrix. */
DECLSPEC void SDLCALL GPU_MatrixIdentity(float* result);

/*! Multiplies an orthographic projection matrix into the given matrix. */
DECLSPEC void SDLCALL GPU_MatrixOrtho(float* result, float left, float right, float bottom, float top, float z_near, float z_far);

/*! Multiplies a perspective projection matrix into the given matrix. */
DECLSPEC void SDLCALL GPU_MatrixFrustum(float* result, float left, float right, float bottom, float top, float z_near, float z_far);

/*! Multiplies a perspective projection matrix into the given matrix. */
DECLSPEC void SDLCALL GPU_MatrixPerspective(float* result, float fovy, float aspect, float z_near, float z_far);

/*! Multiplies a view matrix into the given matrix. */
DECLSPEC void SDLCALL GPU_MatrixLookAt(float* matrix, float eye_x, float eye_y, float eye_z, float target_x, float target_y, float target_z, float up_x, float up_y, float up_z);

/*! Adds a translation into the given matrix. */
DECLSPEC void SDLCALL GPU_MatrixTranslate(float* result, float x, float y, float z);

/*! Multiplies a scaling matrix into the given matrix. */
DECLSPEC void SDLCALL GPU_MatrixScale(float* result, float sx, float sy, float sz);

/*! Multiplies a rotation matrix into the given matrix. */
DECLSPEC void SDLCALL GPU_MatrixRotate(float* result, float degrees, float x, float y, float z);

/*! Multiplies matrices A and B and stores the result in the given 'result' matrix (result = A*B).  Do not use A or B as 'result'.
 * \see GPU_MultiplyAndAssign
*/
DECLSPEC void SDLCALL GPU_MatrixMultiply(float* result, const float* A, const float* B);

/*! Multiplies matrices 'result' and B and stores the result in the given 'result' matrix (result = result * B). */
DECLSPEC void SDLCALL GPU_MultiplyAndAssign(float* result, const float* B);


// Matrix stack accessors

/*! Returns an internal string that represents the contents of matrix A. */
DECLSPEC const char* SDLCALL GPU_GetMatrixString(const float* A);

/*! Returns the current matrix from the active target.  Returns NULL if stack is empty. */
DECLSPEC float* SDLCALL GPU_GetCurrentMatrix(void);

/*! Returns the current matrix from the top of the matrix stack.  Returns NULL if stack is empty. */
DECLSPEC float* SDLCALL GPU_GetTopMatrix(GPU_MatrixStack* stack);

/*! Returns the current model matrix from the active target.  Returns NULL if stack is empty. */
DECLSPEC float* SDLCALL GPU_GetModel(void);

/*! Returns the current view matrix from the active target.  Returns NULL if stack is empty. */
DECLSPEC float* SDLCALL GPU_GetView(void);

/*! Returns the current projection matrix from the active target.  Returns NULL if stack is empty. */
DECLSPEC float* SDLCALL GPU_GetProjection(void);

/*! Copies the current modelview-projection matrix from the active target into the given 'result' matrix (result = P*V*M). */
DECLSPEC void SDLCALL GPU_GetModelViewProjection(float* result);


// Matrix stack manipulators

/*! Returns a newly allocated matrix stack that has already been initialized. */
DECLSPEC GPU_MatrixStack* SDLCALL GPU_CreateMatrixStack(void);

/*! Frees the memory for the matrix stack and any matrices it contains. */
DECLSPEC void SDLCALL GPU_FreeMatrixStack(GPU_MatrixStack* stack);

/*! Allocate new matrices for the given stack. */
DECLSPEC void SDLCALL GPU_InitMatrixStack(GPU_MatrixStack* stack);

/*! Copies matrices from one stack to another. */
DECLSPEC void SDLCALL GPU_CopyMatrixStack(const GPU_MatrixStack* source, GPU_MatrixStack* dest);

/*! Deletes matrices in the given stack. */
DECLSPEC void SDLCALL GPU_ClearMatrixStack(GPU_MatrixStack* stack);

/*! Reapplies the default orthographic projection matrix, based on camera and coordinate settings. */
DECLSPEC void SDLCALL GPU_ResetProjection(GPU_Target* target);

/*! Sets the active target and changes matrix mode to GPU_PROJECTION, GPU_VIEW, or GPU_MODEL.  Further matrix stack operations manipulate that particular stack. */
DECLSPEC void SDLCALL GPU_MatrixMode(GPU_Target* target, int matrix_mode);

/*! Copies the given matrix to the active target's projection matrix. */
DECLSPEC void SDLCALL GPU_SetProjection(const float* A);

/*! Copies the given matrix to the active target's view matrix. */
DECLSPEC void SDLCALL GPU_SetView(const float* A);

/*! Copies the given matrix to the active target's model matrix. */
DECLSPEC void SDLCALL GPU_SetModel(const float* A);

/*! Copies the given matrix to the active target's projection matrix. */
DECLSPEC void SDLCALL GPU_SetProjectionFromStack(GPU_MatrixStack* stack);

/*! Copies the given matrix to the active target's view matrix. */
DECLSPEC void SDLCALL GPU_SetViewFromStack(GPU_MatrixStack* stack);

/*! Copies the given matrix to the active target's model matrix. */
DECLSPEC void SDLCALL GPU_SetModelFromStack(GPU_MatrixStack* stack);

/*! Pushes the current matrix as a new matrix stack item to be restored later. */
DECLSPEC void SDLCALL GPU_PushMatrix(void);

/*! Removes the current matrix from the stack, restoring the previously pushed matrix. */
DECLSPEC void SDLCALL GPU_PopMatrix(void);

/*! Fills current matrix with the identity matrix. */
DECLSPEC void SDLCALL GPU_LoadIdentity(void);

/*! Copies a given matrix to be the current matrix. */
DECLSPEC void SDLCALL GPU_LoadMatrix(const float* matrix4x4);

/*! Multiplies an orthographic projection matrix into the current matrix. */
DECLSPEC void SDLCALL GPU_Ortho(float left, float right, float bottom, float top, float z_near, float z_far);

/*! Multiplies a perspective projection matrix into the current matrix. */
DECLSPEC void SDLCALL GPU_Frustum(float left, float right, float bottom, float top, float z_near, float z_far);

/*! Multiplies a perspective projection matrix into the current matrix. */
DECLSPEC void SDLCALL GPU_Perspective(float fovy, float aspect, float z_near, float z_far);

/*! Multiplies a view matrix into the current matrix. */
DECLSPEC void SDLCALL GPU_LookAt(float eye_x, float eye_y, float eye_z, float target_x, float target_y, float target_z, float up_x, float up_y, float up_z);

/*! Adds a translation into the current matrix. */
DECLSPEC void SDLCALL GPU_Translate(float x, float y, float z);

/*! Multiplies a scaling matrix into the current matrix. */
DECLSPEC void SDLCALL GPU_Scale(float sx, float sy, float sz);

/*! Multiplies a rotation matrix into the current matrix. */
DECLSPEC void SDLCALL GPU_Rotate(float degrees, float x, float y, float z);

/*! Multiplies a given matrix into the current matrix. */
DECLSPEC void SDLCALL GPU_MultMatrix(const float* matrix4x4);

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
    * \param src_rect The region of the source image to use.  Pass NULL for the entire image.
    * \param x Destination x-position
    * \param y Destination y-position */
DECLSPEC void SDLCALL GPU_Blit(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y);

/*! Rotates and draws the given image to the given render target.
    * \param src_rect The region of the source image to use.  Pass NULL for the entire image.
    * \param x Destination x-position
    * \param y Destination y-position
    * \param degrees Rotation angle (in degrees) */
DECLSPEC void SDLCALL GPU_BlitRotate(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees);

/*! Scales and draws the given image to the given render target.
    * \param src_rect The region of the source image to use.  Pass NULL for the entire image.
    * \param x Destination x-position
    * \param y Destination y-position
    * \param scaleX Horizontal stretch factor
    * \param scaleY Vertical stretch factor */
DECLSPEC void SDLCALL GPU_BlitScale(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float scaleX, float scaleY);

/*! Scales, rotates, and draws the given image to the given render target.
    * \param src_rect The region of the source image to use.  Pass NULL for the entire image.
    * \param x Destination x-position
    * \param y Destination y-position
    * \param degrees Rotation angle (in degrees)
    * \param scaleX Horizontal stretch factor
    * \param scaleY Vertical stretch factor */
DECLSPEC void SDLCALL GPU_BlitTransform(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees, float scaleX, float scaleY);

/*! Scales, rotates around a pivot point, and draws the given image to the given render target.  The drawing point (x, y) coincides with the pivot point on the src image (pivot_x, pivot_y).
	* \param src_rect The region of the source image to use.  Pass NULL for the entire image.
	* \param x Destination x-position
	* \param y Destination y-position
	* \param pivot_x Pivot x-position (in image coordinates)
	* \param pivot_y Pivot y-position (in image coordinates)
	* \param degrees Rotation angle (in degrees)
	* \param scaleX Horizontal stretch factor
	* \param scaleY Vertical stretch factor */
DECLSPEC void SDLCALL GPU_BlitTransformX(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float pivot_x, float pivot_y, float degrees, float scaleX, float scaleY);

/*! Draws the given image to the given render target, scaling it to fit the destination region.
    * \param src_rect The region of the source image to use.  Pass NULL for the entire image.
    * \param dest_rect The region of the destination target image to draw upon.  Pass NULL for the entire target.
    */
DECLSPEC void SDLCALL GPU_BlitRect(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, GPU_Rect* dest_rect);

/*! Draws the given image to the given render target, scaling it to fit the destination region.
    * \param src_rect The region of the source image to use.  Pass NULL for the entire image.
    * \param dest_rect The region of the destination target image to draw upon.  Pass NULL for the entire target.
	* \param degrees Rotation angle (in degrees)
	* \param pivot_x Pivot x-position (in image coordinates)
	* \param pivot_y Pivot y-position (in image coordinates)
	* \param flip_direction A GPU_FlipEnum value (or bitwise OR'd combination) that specifies which direction the image should be flipped.
    */
DECLSPEC void SDLCALL GPU_BlitRectX(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, GPU_Rect* dest_rect, float degrees, float pivot_x, float pivot_y, GPU_FlipEnum flip_direction);


/*! Renders triangles from the given set of vertices.  This lets you render arbitrary geometry.  It is a direct path to the GPU, so the format is different than typical SDL_gpu calls.
 * \param values A tightly-packed array of vertex position (e.g. x,y), texture coordinates (e.g. s,t), and color (e.g. r,g,b,a) values.  Texture coordinates and color values are expected to be already normalized to 0.0 - 1.0.  Pass NULL to render with only custom shader attributes.
 * \param indices If not NULL, this is used to specify which vertices to use and in what order (i.e. it indexes the vertices in the 'values' array).
 * \param flags Bit flags to control the interpretation of the 'values' array parameters.
 */
DECLSPEC void SDLCALL GPU_TriangleBatch(GPU_Image* image, GPU_Target* target, unsigned short num_vertices, float* values, unsigned int num_indices, unsigned short* indices, GPU_BatchFlagEnum flags);

/*! Renders triangles from the given set of vertices.  This lets you render arbitrary geometry.  It is a direct path to the GPU, so the format is different than typical SDL_gpu calls.
 * \param values A tightly-packed array of vertex position (e.g. x,y), texture coordinates (e.g. s,t), and color (e.g. r,g,b,a) values.  Texture coordinates and color values are expected to be already normalized to 0.0 - 1.0 (or 0 - 255 for 8-bit color components).  Pass NULL to render with only custom shader attributes.
 * \param indices If not NULL, this is used to specify which vertices to use and in what order (i.e. it indexes the vertices in the 'values' array).
 * \param flags Bit flags to control the interpretation of the 'values' array parameters.
 */
DECLSPEC void SDLCALL GPU_TriangleBatchX(GPU_Image* image, GPU_Target* target, unsigned short num_vertices, void* values, unsigned int num_indices, unsigned short* indices, GPU_BatchFlagEnum flags);

/*! Renders primitives from the given set of vertices.  This lets you render arbitrary geometry.  It is a direct path to the GPU, so the format is different than typical SDL_gpu calls.
 * \param primitive_type The kind of primitive to render.
 * \param values A tightly-packed array of vertex position (e.g. x,y), texture coordinates (e.g. s,t), and color (e.g. r,g,b,a) values.  Texture coordinates and color values are expected to be already normalized to 0.0 - 1.0 (or 0 - 255 for 8-bit color components).  Pass NULL to render with only custom shader attributes.
 * \param indices If not NULL, this is used to specify which vertices to use and in what order (i.e. it indexes the vertices in the 'values' array).
 * \param flags Bit flags to control the interpretation of the 'values' array parameters.
 */
DECLSPEC void SDLCALL GPU_PrimitiveBatch(GPU_Image* image, GPU_Target* target, GPU_PrimitiveEnum primitive_type, unsigned short num_vertices, float* values, unsigned int num_indices, unsigned short* indices, GPU_BatchFlagEnum flags);

/*! Renders primitives from the given set of vertices.  This lets you render arbitrary geometry.  It is a direct path to the GPU, so the format is different than typical SDL_gpu calls.
 * \param primitive_type The kind of primitive to render.
 * \param values A tightly-packed array of vertex position (e.g. x,y), texture coordinates (e.g. s,t), and color (e.g. r,g,b,a) values.  Texture coordinates and color values are expected to be already normalized to 0.0 - 1.0 (or 0 - 255 for 8-bit color components).  Pass NULL to render with only custom shader attributes.
 * \param indices If not NULL, this is used to specify which vertices to use and in what order (i.e. it indexes the vertices in the 'values' array).
 * \param flags Bit flags to control the interpretation of the 'values' array parameters.
 */
DECLSPEC void SDLCALL GPU_PrimitiveBatchV(GPU_Image* image, GPU_Target* target, GPU_PrimitiveEnum primitive_type, unsigned short num_vertices, void* values, unsigned int num_indices, unsigned short* indices, GPU_BatchFlagEnum flags);

/*! Send all buffered blitting data to the current context target. */
DECLSPEC void SDLCALL GPU_FlushBlitBuffer(void);

/*! Updates the given target's associated window.  For non-context targets (e.g. image targets), this will flush the blit buffer. */
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

/*! Renders a colored rectangle outline.
 * \param target The destination render target
 * \param rect The rectangular area to draw
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_Rectangle2(GPU_Target* target, GPU_Rect rect, SDL_Color color);

/*! Renders a colored filled rectangle.
 * \param target The destination render target
 * \param x1 x-coord of top-left corner
 * \param y1 y-coord of top-left corner
 * \param x2 x-coord of bottom-right corner
 * \param y2 y-coord of bottom-right corner
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_RectangleFilled(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

/*! Renders a colored filled rectangle.
 * \param target The destination render target
 * \param rect The rectangular area to draw
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_RectangleFilled2(GPU_Target* target, GPU_Rect rect, SDL_Color color);

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

/*! Renders a colored rounded (filleted) rectangle outline.
 * \param target The destination render target
 * \param rect The rectangular area to draw
 * \param radius The radius of the corners
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_RectangleRound2(GPU_Target* target, GPU_Rect rect, float radius, SDL_Color color);

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

/*! Renders a colored filled rounded (filleted) rectangle.
 * \param target The destination render target
 * \param rect The rectangular area to draw
 * \param radius The radius of the corners
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_RectangleRoundFilled2(GPU_Target* target, GPU_Rect rect, float radius, SDL_Color color);

/*! Renders a colored polygon outline.  The vertices are expected to define a convex polygon.
 * \param target The destination render target
 * \param num_vertices Number of vertices (x and y pairs)
 * \param vertices An array of vertex positions stored as interlaced x and y coords, e.g. {x1, y1, x2, y2, ...}
 * \param color The color of the shape to render
 */
DECLSPEC void SDLCALL GPU_Polygon(GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color);

/*! Renders a colored sequence of line segments.
 * \param target The destination render target
 * \param num_vertices Number of vertices (x and y pairs)
 * \param vertices An array of vertex positions stored as interlaced x and y coords, e.g. {x1, y1, x2, y2, ...}
 * \param color The color of the shape to render
 * \param close_loop Make a closed polygon by drawing a line at the end back to the start point
 */
DECLSPEC void SDLCALL GPU_Polyline(GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color, GPU_bool close_loop);
	
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
DECLSPEC Uint32 SDLCALL GPU_CompileShader_RW(GPU_ShaderEnum shader_type, SDL_RWops* shader_source, GPU_bool free_rwops);

/*! Compiles shader source and returns the new shader object. */
DECLSPEC Uint32 SDLCALL GPU_CompileShader(GPU_ShaderEnum shader_type, const char* shader_source);

/*! Loads shader source from a file, compiles it, and returns the new shader object. */
DECLSPEC Uint32 SDLCALL GPU_LoadShader(GPU_ShaderEnum shader_type, const char* filename);

/*! Creates and links a shader program with the given shader objects. */
DECLSPEC Uint32 SDLCALL GPU_LinkShaders(Uint32 shader_object1, Uint32 shader_object2);

/*! Creates and links a shader program with the given shader objects. */
DECLSPEC Uint32 SDLCALL GPU_LinkManyShaders(Uint32 *shader_objects, int count);

/*! Deletes a shader object. */
DECLSPEC void SDLCALL GPU_FreeShader(Uint32 shader_object);

/*! Attaches a shader object to a shader program for future linking. */
DECLSPEC void SDLCALL GPU_AttachShader(Uint32 program_object, Uint32 shader_object);

/*! Detaches a shader object from a shader program. */
DECLSPEC void SDLCALL GPU_DetachShader(Uint32 program_object, Uint32 shader_object);

/*! Links a shader program with any attached shader objects. */
DECLSPEC GPU_bool SDLCALL GPU_LinkShaderProgram(Uint32 program_object);

/*! \return The current shader program */
DECLSPEC Uint32 SDLCALL GPU_GetCurrentShaderProgram(void);

/*! Returns 1 if the given shader program is a default shader for the current context, 0 otherwise. */
DECLSPEC GPU_bool SDLCALL GPU_IsDefaultShaderProgram(Uint32 program_object);

/*! Activates the given shader program.  Passing NULL for 'block' will disable the built-in shader variables for custom shaders until a GPU_ShaderBlock is set again. */
DECLSPEC void SDLCALL GPU_ActivateShaderProgram(Uint32 program_object, GPU_ShaderBlock* block);

/*! Deactivates the current shader program (activates program 0). */
DECLSPEC void SDLCALL GPU_DeactivateShaderProgram(void);

/*! Returns the last shader log message. */
DECLSPEC const char* SDLCALL GPU_GetShaderMessage(void);

/*! Returns an integer representing the location of the specified attribute shader variable. */
DECLSPEC int SDLCALL GPU_GetAttributeLocation(Uint32 program_object, const char* attrib_name);

/*! Returns a filled GPU_AttributeFormat object. */
DECLSPEC GPU_AttributeFormat SDLCALL GPU_MakeAttributeFormat(int num_elems_per_vertex, GPU_TypeEnum type, GPU_bool normalize, int stride_bytes, int offset_bytes);

/*! Returns a filled GPU_Attribute object. */
DECLSPEC GPU_Attribute SDLCALL GPU_MakeAttribute(int location, void* values, GPU_AttributeFormat format);

/*! Returns an integer representing the location of the specified uniform shader variable. */
DECLSPEC int SDLCALL GPU_GetUniformLocation(Uint32 program_object, const char* uniform_name);

/*! Loads the given shader program's built-in attribute and uniform locations. */
DECLSPEC GPU_ShaderBlock SDLCALL GPU_LoadShaderBlock(Uint32 program_object, const char* position_name, const char* texcoord_name, const char* color_name, const char* modelViewMatrix_name);

/*! Sets the current shader block to use the given attribute and uniform locations. */
DECLSPEC void SDLCALL GPU_SetShaderBlock(GPU_ShaderBlock block);

/*! Gets the shader block for the current shader. */
DECLSPEC GPU_ShaderBlock SDLCALL GPU_GetShaderBlock(void);

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
DECLSPEC void SDLCALL GPU_SetUniformMatrixfv(int location, int num_matrices, int num_rows, int num_columns, GPU_bool transpose, float* values);

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

