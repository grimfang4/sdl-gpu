#ifndef _SDL_GPU_H__
#define _SDL_GPU_H__

#include "SDL.h"
#include <stdio.h>
#include <stdarg.h>

/* Auto-detect if we're using the SDL2 API by the headers available. */
#if SDL_VERSION_ATLEAST(2,0,0)
    #define SDL_GPU_USE_SDL2
#else
    #define SDL_Window SDL_Surface
#endif


#ifdef __cplusplus
extern "C" {
#endif


typedef struct GPU_Renderer GPU_Renderer;
typedef struct GPU_Target GPU_Target;

/*! A struct representing a rectangular area with floating point precision.
 * \see GPU_MakeRect() 
 */
typedef struct GPU_Rect
{
    float x, y;
    float w, h;
} GPU_Rect;

typedef Uint32 GPU_RendererEnum;
static const GPU_RendererEnum GPU_RENDERER_UNKNOWN = 0x0;  // invalid value
static const GPU_RendererEnum GPU_RENDERER_DEFAULT = 0x1;
static const GPU_RendererEnum GPU_RENDERER_OPENGL_1 = 0x2;
static const GPU_RendererEnum GPU_RENDERER_OPENGL_2 = 0x4;
static const GPU_RendererEnum GPU_RENDERER_OPENGL_3 = 0x8;
static const GPU_RendererEnum GPU_RENDERER_OPENGL_4 = 0x10;
static const GPU_RendererEnum GPU_RENDERER_GLES_1 = 0x20;
static const GPU_RendererEnum GPU_RENDERER_GLES_2 = 0x40;
static const GPU_RendererEnum GPU_RENDERER_GLES_3 = 0x80;
static const GPU_RendererEnum GPU_RENDERER_D3D9 = 0x100;

/*! Renderer ID object for identifying a specific renderer.
 * \see GPU_MakeRendererID()
 * \see GPU_MakeRendererIDRequest()
 * \see GPU_Init()
 * \see GPU_InitRenderer()
 */
typedef struct GPU_RendererID
{
    GPU_RendererEnum id;
    int major_version;
    int minor_version;
    Uint32 flags;
    
    int index;
} GPU_RendererID;


/*! Texture filtering options.  These affect the quality/interpolation of colors when images are scaled. 
 * \see GPU_SetImageFilter()
 */
typedef unsigned int GPU_FilterEnum;
static const GPU_FilterEnum GPU_NEAREST = 0;
static const GPU_FilterEnum GPU_LINEAR = 1;
static const GPU_FilterEnum GPU_LINEAR_MIPMAP = 2;

/*! Blending options 
 * \see GPU_SetBlendMode()
 */
typedef unsigned int GPU_BlendEnum;
static const GPU_BlendEnum GPU_BLEND_NORMAL = 0;
static const GPU_BlendEnum GPU_BLEND_MULTIPLY = 1;
static const GPU_BlendEnum GPU_BLEND_ADD = 2;
static const GPU_BlendEnum GPU_BLEND_SUBTRACT = 3;
static const GPU_BlendEnum GPU_BLEND_ADD_COLOR = 4;
static const GPU_BlendEnum GPU_BLEND_SUBTRACT_COLOR = 5;
static const GPU_BlendEnum GPU_BLEND_DARKEN = 6;
static const GPU_BlendEnum GPU_BLEND_LIGHTEN = 7;
static const GPU_BlendEnum GPU_BLEND_DIFFERENCE = 8;
static const GPU_BlendEnum GPU_BLEND_PUNCHOUT = 9;
static const GPU_BlendEnum GPU_BLEND_CUTOUT = 10;
static const GPU_BlendEnum GPU_BLEND_OVERRIDE = 11;  // Lets you specify direct GL calls before blitting


/*! Image object for containing pixel/texture data.
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
	int channels;
	SDL_Color color;
	Uint8 use_blending;
	GPU_BlendEnum blend_mode;
	GPU_FilterEnum filter_mode;
	
	void* data;
	int refcount;
} GPU_Image;


/*! Camera object that determines viewing transform.
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


/*! Container for the built-in shader attribute and uniform locations (indices).
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


/*! Rendering context data.  Only GPU_Targets which represent windows will store this. */
typedef struct GPU_Context
{
    /*! SDL_GLContext */
    void* context;
    
    /*! SDL window ID */
	Uint32 windowID;
	
	/*! Actual window dimensions */
	int window_w;
	int window_h;
	
	/*! Internal state */
	Uint32 current_shader_program;
	
	Uint8 shapes_use_blending;
	GPU_BlendEnum shapes_blend_mode;
	
	void* data;
} GPU_Context;


/*! Render target object for use as a blitting destination.
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
	Uint8 useClip;
	GPU_Rect clipRect;
	
	/*! Perspective and object viewing transforms. */
	GPU_Camera camera;
	
	/*! Renderer context data.  NULL if the target does not represent a window or rendering context. */
	GPU_Context* context;
};

/*! Important GPU features which may not be supported depending on a device's extension support.  Can be OR'd together.
 * \see GPU_IsFeatureEnabled()
 */
typedef Uint32 GPU_FeatureEnum;
static const GPU_FeatureEnum GPU_FEATURE_NON_POWER_OF_TWO = 0x1;
static const GPU_FeatureEnum GPU_FEATURE_RENDER_TARGETS = 0x2;
static const GPU_FeatureEnum GPU_FEATURE_BLEND_EQUATIONS = 0x4;
static const GPU_FeatureEnum GPU_FEATURE_BLEND_FUNC_SEPARATE = 0x8;
static const GPU_FeatureEnum GPU_FEATURE_GL_BGR = 0x10;
static const GPU_FeatureEnum GPU_FEATURE_GL_BGRA = 0x20;
static const GPU_FeatureEnum GPU_FEATURE_GL_ABGR = 0x40;
static const GPU_FeatureEnum GPU_FEATURE_VERTEX_SHADER = 0x100;
static const GPU_FeatureEnum GPU_FEATURE_FRAGMENT_SHADER = 0x200;
static const GPU_FeatureEnum GPU_FEATURE_PIXEL_SHADER = 0x200;
static const GPU_FeatureEnum GPU_FEATURE_GEOMETRY_SHADER = 0x400;

/*! Combined feature flags */
#define GPU_FEATURE_ALL_BASE GPU_FEATURE_RENDER_TARGETS
#define GPU_FEATURE_ALL_BLEND_MODES (GPU_FEATURE_BLEND_EQUATIONS | GPU_FEATURE_BLEND_FUNC_SEPARATE)
#define GPU_FEATURE_ALL_GL_FORMATS (GPU_FEATURE_GL_BGR | GPU_FEATURE_GL_BGRA | GPU_FEATURE_GL_ABGR)
#define GPU_FEATURE_ALL_SHADERS (GPU_FEATURE_FRAGMENT_SHADER | GPU_FEATURE_PIXEL_SHADER | GPU_FEATURE_GEOMETRY_SHADER)


/*! Renderer object which specializes the API to a particular backend. */
struct GPU_Renderer
{
	/*! Struct identifier of the renderer. */
	GPU_RendererID id;
	
	int tier;
    GPU_FeatureEnum enabled_features;
	
	/*! Current display target */
	GPU_Target* current_context_target;
	
	
	/*! \see GPU_Init() */
	GPU_Target* (*Init)(GPU_Renderer* renderer, GPU_RendererID renderer_request, Uint16 w, Uint16 h, Uint32 flags);
	
	/*! \see GPU_IsFeatureEnabled() */
	Uint8 (*IsFeatureEnabled)(GPU_Renderer* renderer, GPU_FeatureEnum feature);
	
    /*! \see GPU_CreateTargetFromWindow
     * The extra parameter is used internally to reuse/reinit a target. */
    GPU_Target* (*CreateTargetFromWindow)(GPU_Renderer* renderer, Uint32 windowID, GPU_Target* target);

    /*! \see GPU_MakeCurrent */
    void (*MakeCurrent)(GPU_Renderer* renderer, GPU_Target* target, Uint32 windowID);
	
	/*! Sets up this renderer to act as the current renderer.  Called automatically by GPU_SetCurrentRenderer(). */
	void (*SetAsCurrent)(GPU_Renderer* renderer);
	
	/*! \see GPU_SetWindowResolution() */
	int (*SetWindowResolution)(GPU_Renderer* renderer, Uint16 w, Uint16 h);
	
	/*! \see GPU_SetVirtualResolution() */
	void (*SetVirtualResolution)(GPU_Renderer* renderer, GPU_Target* target, Uint16 w, Uint16 h);
	
	/*! Clean up the renderer state. */
	void (*Quit)(GPU_Renderer* renderer);
	
	/*! \see GPU_ToggleFullscreen() */
	int (*ToggleFullscreen)(GPU_Renderer* renderer);

	/*! \see GPU_SetCamera() */
	GPU_Camera (*SetCamera)(GPU_Renderer* renderer, GPU_Target* target, GPU_Camera* cam);
	
    /*! \see GPU_CreateImage() */
	GPU_Image* (*CreateImage)(GPU_Renderer* renderer, Uint16 w, Uint16 h, Uint8 channels);
	
	/*! \see GPU_LoadImage() */
	GPU_Image* (*LoadImage)(GPU_Renderer* renderer, const char* filename);
	
	/*! \see GPU_SaveImage() */
	Uint8 (*SaveImage)(GPU_Renderer* renderer, GPU_Image* image, const char* filename);
	
	/*! \see GPU_CopyImage() */
	GPU_Image* (*CopyImage)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_UpdateImage */
	void (*UpdateImage)(GPU_Renderer* renderer, GPU_Image* image, const GPU_Rect* rect, SDL_Surface* surface);
	
	/*! \see GPU_CopyImageFromSurface() */
	GPU_Image* (*CopyImageFromSurface)(GPU_Renderer* renderer, SDL_Surface* surface);
	
	/*! \see GPU_CopyImageFromTarget() */
	GPU_Image* (*CopyImageFromTarget)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! \see GPU_CopySurfaceFromTarget() */
	SDL_Surface* (*CopySurfaceFromTarget)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! \see GPU_CopySurfaceFromImage() */
	SDL_Surface* (*CopySurfaceFromImage)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_FreeImage() */
	void (*FreeImage)(GPU_Renderer* renderer, GPU_Image* image);
	
    /*! \see GPU_SubSurfaceCopy() */
    void (*SubSurfaceCopy)(GPU_Renderer* renderer, SDL_Surface* src, GPU_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y);
	
	/*! \see GPU_LoadTarget() */
	GPU_Target* (*LoadTarget)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_FreeTarget() */
	void (*FreeTarget)(GPU_Renderer* renderer, GPU_Target* target);

	/*! \see GPU_Blit() */
	int (*Blit)(GPU_Renderer* renderer, GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y);
	
	/*! \see GPU_BlitRotate() */
	int (*BlitRotate)(GPU_Renderer* renderer, GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float angle);
	
	/*! \see GPU_BlitScale() */
	int (*BlitScale)(GPU_Renderer* renderer, GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float scaleX, float scaleY);
	
	/*! \see GPU_BlitTransform */
	int (*BlitTransform)(GPU_Renderer* renderer, GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float angle, float scaleX, float scaleY);
	
	/*! \see GPU_BlitTransformX() */
	int (*BlitTransformX)(GPU_Renderer* renderer, GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float pivot_x, float pivot_y, float angle, float scaleX, float scaleY);
	
	/*! \see GPU_BlitTransformMatrix() */
	int (*BlitTransformMatrix)(GPU_Renderer* renderer, GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float* matrix3x3);
	
	/*! \see GPU_SetX() */
	float (*SetZ)(GPU_Renderer* renderer, float z);
	
	/*! \see GPU_GetZ() */
	float (*GetZ)(GPU_Renderer* renderer);
	
	/*! \see GPU_GenerateMipmaps() */
	void (*GenerateMipmaps)(GPU_Renderer* renderer, GPU_Image* image);

	/*! \see GPU_SetClip() */
	GPU_Rect (*SetClip)(GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h);

	/*! \see GPU_ClearClip() */
	void (*ClearClip)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! \see GPU_GetPixel() */
	SDL_Color (*GetPixel)(GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y);
	
	/*! \see GPU_SetImageFilter() */
	void (*SetImageFilter)(GPU_Renderer* renderer, GPU_Image* image, GPU_FilterEnum filter);

	/*! \see GPU_Clear() */
	void (*Clear)(GPU_Renderer* renderer, GPU_Target* target);
	/*! \see GPU_ClearRGBA() */
	void (*ClearRGBA)(GPU_Renderer* renderer, GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	/*! \see GPU_FlushBlitBuffer() */
	void (*FlushBlitBuffer)(GPU_Renderer* renderer);
	/*! \see GPU_Flip() */
	void (*Flip)(GPU_Renderer* renderer, GPU_Target* target);
	
	
    /*! \see GPU_CompileShader_RW() */
	Uint32 (*CompileShader_RW)(GPU_Renderer* renderer, int shader_type, SDL_RWops* shader_source);
	
    /*! \see GPU_CompileShader() */
    Uint32 (*CompileShader)(GPU_Renderer* renderer, int shader_type, const char* shader_source);

    /*! \see GPU_LinkShaderProgram() */
    Uint32 (*LinkShaderProgram)(GPU_Renderer* renderer, Uint32 program_object);

    /*! \see GPU_LinkShaders() */
    Uint32 (*LinkShaders)(GPU_Renderer* renderer, Uint32 shader_object1, Uint32 shader_object2);

    /*! \see GPU_FreeShader() */
    void (*FreeShader)(GPU_Renderer* renderer, Uint32 shader_object);

    /*! \see GPU_FreeShaderProgram() */
    void (*FreeShaderProgram)(GPU_Renderer* renderer, Uint32 program_object);

    /*! \see GPU_AttachShader() */
    void (*AttachShader)(GPU_Renderer* renderer, Uint32 program_object, Uint32 shader_object);

    /*! \see GPU_DetachShader() */
    void (*DetachShader)(GPU_Renderer* renderer, Uint32 program_object, Uint32 shader_object);
    
    /*! \see GPU_IsDefaultShaderProgram() */
    Uint8 (*IsDefaultShaderProgram)(GPU_Renderer* renderer, Uint32 program_object);

    /*! \see GPU_ActivateShaderProgram() */
    void (*ActivateShaderProgram)(GPU_Renderer* renderer, Uint32 program_object, GPU_ShaderBlock* block);

    /*! \see GPU_DeactivateShaderProgram() */
    void (*DeactivateShaderProgram)(GPU_Renderer* renderer);

    /*! \see GPU_GetShaderMessage() */
    const char* (*GetShaderMessage)(GPU_Renderer* renderer);

    /*! \see GPU_GetAttribLocation() */
    int (*GetAttribLocation)(GPU_Renderer* renderer, Uint32 program_object, const char* attrib_name);

    /*! \see GPU_GetUniformLocation() */
    int (*GetUniformLocation)(GPU_Renderer* renderer, Uint32 program_object, const char* uniform_name);
    
    /*! \see GPU_LoadShaderBlock() */
    GPU_ShaderBlock (*LoadShaderBlock)(GPU_Renderer* renderer, Uint32 program_object, const char* position_name, const char* texcoord_name, const char* color_name, const char* modelViewMatrix_name);
    
    /*! \see GPU_SetShaderBlock() */
    void (*SetShaderBlock)(GPU_Renderer* renderer, GPU_ShaderBlock block);
    
    /*! \see GPU_GetUniformiv() */
    void (*GetUniformiv)(GPU_Renderer* renderer, Uint32 program_object, int location, int* values);

    /*! \see GPU_SetUniformi() */
    void (*SetUniformi)(GPU_Renderer* renderer, int location, int value);

    /*! \see GPU_SetUniformiv() */
    void (*SetUniformiv)(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, int* values);

    /*! \see GPU_GetUniformuiv() */
    void (*GetUniformuiv)(GPU_Renderer* renderer, Uint32 program_object, int location, unsigned int* values);

    /*! \see GPU_SetUniformui() */
    void (*SetUniformui)(GPU_Renderer* renderer, int location, unsigned int value);

    /*! \see GPU_SetUniformuiv() */
    void (*SetUniformuiv)(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, unsigned int* values);

    /*! \see GPU_GetUniformfv() */
    void (*GetUniformfv)(GPU_Renderer* renderer, Uint32 program_object, int location, float* values);

    /*! \see GPU_SetUniformf() */
    void (*SetUniformf)(GPU_Renderer* renderer, int location, float value);

    /*! \see GPU_SetUniformfv() */
    void (*SetUniformfv)(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, float* values);

    /*! \see GPU_SetUniformMatrixfv() */
    void (*SetUniformMatrixfv)(GPU_Renderer* renderer, int location, int num_matrices, int num_rows, int num_columns, Uint8 transpose, float* values);
    
    
    // Shapes
    
	float (*SetThickness)(GPU_Renderer* renderer, float thickness);
	float (*GetThickness)(GPU_Renderer* renderer);
	
	void (*Pixel)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, SDL_Color color);

	void (*Line)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

	void (*Arc)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, float startAngle, float endAngle, SDL_Color color);
	
	void (*ArcFilled)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, float startAngle, float endAngle, SDL_Color color);

	void (*Circle)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color);

	void (*CircleFilled)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color);

	void (*Tri)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

	void (*TriFilled)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

	void (*Rectangle)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

	void (*RectangleFilled)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

	void (*RectangleRound)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

	void (*RectangleRoundFilled)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

	void (*Polygon)(GPU_Renderer* renderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color);

	void (*PolygonFilled)(GPU_Renderer* renderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color);

	void (*PolygonBlit)(GPU_Renderer* renderer, GPU_Image* src, GPU_Rect* srcrect, GPU_Target* target, Uint16 n, float* vertices, float textureX, float textureY, float angle, float scaleX, float scaleY);
	
};


// System calls
/*! Prints an informational log message. */
void GPU_LogInfo(const char* format, ...);

/*! Prints a warning log message. */
void GPU_LogWarning(const char* format, ...);

/*! Prints an error log message. */
void GPU_LogError(const char* format, ...);

#define GPU_Log GPU_LogInfo


// Setup calls
/*! Initializes SDL and SDL_gpu.  Creates a window and the default renderer context. */
GPU_Target* GPU_Init(Uint16 w, Uint16 h, Uint32 flags);

/*! Initializes SDL and SDL_gpu.  Creates a window and renderer context. */
GPU_Target* GPU_InitRenderer(GPU_RendererID renderer_request, Uint16 w, Uint16 h, Uint32 flags);

/*! Returns the default renderer ID for the current platform. */
GPU_RendererID GPU_GetDefaultRendererID(void);

/*! Checks for important GPU features which may not be supported depending on a device's extension support.  Feature flags (GPU_FEATURE_*) can be bitwise OR'd together. 
 * \return 1 if all of the passed features are enabled/supported
 * \return 0 if any of the passed features are disabled/unsupported
 */
Uint8 GPU_IsFeatureEnabled(GPU_FeatureEnum feature);

/*! Creates a separate context for the given window using the current renderer and returns a GPU_Target that represents it. */
GPU_Target* GPU_CreateTargetFromWindow(Uint32 windowID);

/*! Makes the given window the current rendering destination for the given target.
 * This also makes the target the current context for image loading and window operations.
 * If the target does not represent a window, this does nothing.
 */
void GPU_MakeCurrent(GPU_Target* target, Uint32 windowID);

/*! Change the actual size of the current window. */
int GPU_SetWindowResolution(Uint16 w, Uint16 h);

/*! Change the logical size of the given target.  Rendering to this target will be scaled as if the dimensions were actually the ones given. */
void GPU_SetVirtualResolution(GPU_Target* target, Uint16 w, Uint16 h);

/*! Clean up the renderer state. */
void GPU_CloseCurrentRenderer(void);

/*! Clean up the renderer state and shut down SDL_gpu. */
void GPU_Quit(void);

/*! Sets the current error string. */
void GPU_SetError(const char* fmt, ...);

/*! Gets the current error string. */
const char* GPU_GetErrorString(void);

/*! Converts screen space coordinates (such as from mouse input) to logical drawing coordinates. */
void GPU_GetVirtualCoords(GPU_Target* target, float* x, float* y, float displayX, float displayY);

/*! Enable/disable fullscreen mode for the current window.
 * On some platforms, this may destroy the renderer context and require that textures be reloaded. */
int GPU_ToggleFullscreen(void);


// Renderer controls

/*! Returns the default GPU_RendererID for the current platform. */
GPU_RendererID GPU_GetDefaultRendererID(void);

/*! Translates a GPU_RendererEnum into a string. */
const char* GPU_GetRendererEnumString(GPU_RendererEnum id);

/*! Returns an initialized GPU_RendererRequestID. */
GPU_RendererID GPU_MakeRendererIDRequest(GPU_RendererEnum id, int major_version, int minor_version, Uint32 flags);

/*! Returns an initialized GPU_RendererID. */
GPU_RendererID GPU_MakeRendererID(GPU_RendererEnum id, int major_version, int minor_version, int index);

/*! Gets the renderer identifier for the given registration index. */
GPU_RendererID GPU_GetRendererID(unsigned int index);

/*! Gets the number of active (created) renderers. */
int GPU_GetNumActiveRenderers(void);

/*! Gets an array of identifiers for the active renderers. */
void GPU_GetActiveRendererList(GPU_RendererID* renderers_array);

/*! Gets the number of registered (available) renderers. */
int GPU_GetNumRegisteredRenderers(void);

/*! Gets an array of identifiers for the registered (available) renderers. */
void GPU_GetRegisteredRendererList(GPU_RendererID* renderers_array);

/*! Creates a new renderer matching the given identifier. */
GPU_Renderer* GPU_AddRenderer(GPU_RendererID id);

/*! Deletes the renderer matching the given identifier. */
void GPU_RemoveRenderer(GPU_RendererID id);

/*! Gets the renderer for the given renderer index. */
GPU_Renderer* GPU_GetRenderer(unsigned int index);

/*! \return The renderer matching the given identifier. */
GPU_Renderer* GPU_GetRendererByID(GPU_RendererID id);

/*! Switches the current renderer to the renderer matching the given identifier. */
void GPU_SetCurrentRenderer(GPU_RendererID id);

/*! \return The current renderer */
GPU_Renderer* GPU_GetCurrentRenderer(void);

/*! \return A GPU_Rect with the given values. */
GPU_Rect GPU_MakeRect(float x, float y, float w, float h);

/*! \return A GPU_Camera with position (0, 0, -10), angle of 0, and zoom of 1. */
GPU_Camera GPU_GetDefaultCamera(void);

/*! \return The current camera of the current render target. */
GPU_Camera GPU_GetCamera(void);


/*! Sets the current render target's current camera.
 * \param target A pointer to the target that will copy this camera.
 * \param cam A pointer to the camera data to use or NULL to use the default camera.
 * \return The old camera. */
GPU_Camera GPU_SetCamera(GPU_Target* target, GPU_Camera* cam);

/*! Create a new, blank image with a format determined by the number of channels requested.  Don't forget to GPU_FreeImage() it. 
	 * \param w Image width in pixels
	 * \param h Image height in pixels
	 * \param channels Number of color channels.  Usually in the range of [1,4] with 3 being RGB and 4 being RGBA.
	 */
GPU_Image* GPU_CreateImage(Uint16 w, Uint16 h, Uint8 channels);

/*! Load image from an image file that is supported by this renderer.  Don't forget to GPU_FreeImage() it. */
GPU_Image* GPU_LoadImage(const char* filename);

/*! Save image to a file.  The file type is deduced from the extension.  Returns 0 on failure. */
Uint8 GPU_SaveImage(GPU_Image* image, const char* filename);

/*! Save surface to a file.  The file type is deduced from the extension.  Returns 0 on failure. */
Uint8 GPU_SaveSurface(SDL_Surface* surface, const char* filename);

/*! Copy an image to a new image.  Don't forget to GPU_FreeImage() both. */
GPU_Image* GPU_CopyImage(GPU_Image* image);

/*! Update an image from surface data. */
void GPU_UpdateImage(GPU_Image* image, const GPU_Rect* rect, SDL_Surface* surface);

/*! Load surface from an image file that is supported by this renderer.  Don't forget to SDL_FreeSurface() it. */
SDL_Surface* GPU_LoadSurface(const char* filename);

/*! Copy SDL_Surface data into a new GPU_Image.  Don't forget to SDL_FreeSurface() the surface and GPU_FreeImage() the image.*/
GPU_Image* GPU_CopyImageFromSurface(SDL_Surface* surface);

/*! Copy GPU_Target data into a new GPU_Image.  Don't forget to GPU_FreeImage() the image.*/
GPU_Image* GPU_CopyImageFromTarget(GPU_Target* target);

/*! Copy GPU_Target data into a new SDL_Surface.  Don't forget to SDL_FreeSurface() the surface.*/
SDL_Surface* GPU_CopySurfaceFromTarget(GPU_Target* target);

/*! Copy GPU_Image data into a new SDL_Surface.  Don't forget to SDL_FreeSurface() the surface and GPU_FreeImage() the image.*/
SDL_Surface* GPU_CopySurfaceFromImage(GPU_Image* image);

/*! Deletes an image in the proper way for this renderer.  Also deletes the corresponding GPU_Target if applicable.  Be careful not to use that target afterward! */
void GPU_FreeImage(GPU_Image* image);

/*! Copies software surface data to a hardware texture.  Draws data with the upper left corner being (x,y).  */
void GPU_SubSurfaceCopy(SDL_Surface* src, GPU_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y);

/*! \return The renderer's current output surface/framebuffer. */
GPU_Target* GPU_GetContextTarget(void);

/*! Creates a new render target from the given image.  It can then be accessed from image->target. */
GPU_Target* GPU_LoadTarget(GPU_Image* image);

/*! Deletes a render target in the proper way for this renderer. */
void GPU_FreeTarget(GPU_Target* target);

/*! Draws the 'src' image to the 'dest' render target.
    * \param srcrect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position */
int GPU_Blit(GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y);

/*! Rotates and draws the 'src' image to the 'dest' render target.
    * \param srcrect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position
    * \param angle Rotation angle (in degrees) */
int GPU_BlitRotate(GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float angle);

/*! Scales and draws the 'src' image to the 'dest' render target.
    * \param srcrect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position
    * \param scaleX Horizontal stretch factor
    * \param scaleY Vertical stretch factor */
int GPU_BlitScale(GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float scaleX, float scaleY);

/*! Scales, rotates, and draws the 'src' image to the 'dest' render target.
    * \param srcrect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position
    * \param angle Rotation angle (in degrees)
    * \param scaleX Horizontal stretch factor
    * \param scaleY Vertical stretch factor */
int GPU_BlitTransform(GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float angle, float scaleX, float scaleY);

	
/*! Scales, rotates around a pivot point, and draws the 'src' image to the 'dest' render target.  The drawing point (x, y) coincides with the pivot point on the src image (pivot_x, pivot_y).
	* \param srcrect The region of the source image to use.
	* \param x Destination x-position
	* \param y Destination y-position
	* \param pivot_x Pivot x-position (in src image coordinates)
	* \param pivot_y Pivot y-position (in src image coordinates)
	* \param angle Rotation angle (in degrees)
	* \param scaleX Horizontal stretch factor
	* \param scaleY Vertical stretch factor */
int GPU_BlitTransformX(GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float pivot_x, float pivot_y, float angle, float scaleX, float scaleY);


/*! Transforms and draws the 'src' image to the 'dest' render target.
	* \param srcrect The region of the source image to use.
	* \param x Destination x-position
	* \param y Destination y-position
	* \param matrix3x3 3x3 matrix in column-major order (index = row + column*numColumns) */
int GPU_BlitTransformMatrix(GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float* matrix3x3);


/*! Sets the renderer's z-depth.
    * \return The previous z-depth */
float GPU_SetZ(float z);

/*! Gets the renderer's z-depth.
    * \return The current z-depth */
float GPU_GetZ(void);

/*! Loads mipmaps for the given image, if supported by the renderer. */
void GPU_GenerateMipmaps(GPU_Image* image);

/*! Sets the clipping rect for the given render target. */
GPU_Rect GPU_SetClipRect(GPU_Target* target, GPU_Rect rect);

/*! Sets the clipping rect for the given render target. */
GPU_Rect GPU_SetClip(GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h);

/*! Clears (resets) the clipping rect for the given render target. */
void GPU_ClearClip(GPU_Target* target);

/*! Sets the modulation color for subsequent drawing. */
void GPU_SetColor(GPU_Image* image, SDL_Color* color);

/*! Sets the modulation color for subsequent drawing. */
void GPU_SetRGB(GPU_Image* image, Uint8 r, Uint8 g, Uint8 b);

/*! Sets the modulation color for subsequent drawing. */
void GPU_SetRGBA(GPU_Image* image, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/*! Gets the current alpha blending setting. */
Uint8 GPU_GetBlending(GPU_Image* image);

/*! Enables/disables alpha blending for the given image. */
void GPU_SetBlending(GPU_Image* image, Uint8 enable);

/*! Enables/disables alpha blending for shape rendering on the current window. */
void GPU_SetShapeBlending(Uint8 enable);
	
/*! Sets the blending mode, if supported by the renderer. */
void GPU_SetBlendMode(GPU_Image* image, GPU_BlendEnum mode);
	
/*! Sets the blending mode for shape rendering on the current window, if supported by the renderer. */
void GPU_SetShapeBlendMode(GPU_BlendEnum mode);

/*! Sets the image filtering mode, if supported by the renderer. */
void GPU_SetImageFilter(GPU_Image* image, GPU_FilterEnum filter);

/*! \return The RGBA color of a pixel. */
SDL_Color GPU_GetPixel(GPU_Target* target, Sint16 x, Sint16 y);

/*! Clears the contents of the given render target. */
void GPU_Clear(GPU_Target* target);

/*! Fills the given render target with a color. */
void GPU_ClearRGBA(GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/*! Send all buffered blitting data to the current context target. */
void GPU_FlushBlitBuffer(void);

/*! Updates the given target's associated window. */
void GPU_Flip(GPU_Target* target);





// Shaders

#define GPU_VERTEX_SHADER 0
#define GPU_FRAGMENT_SHADER 1
#define GPU_PIXEL_SHADER 1
#define GPU_GEOMETRY_SHADER 2

/*! Loads shader source from an SDL_RWops, compiles it, and returns the new shader object. */
Uint32 GPU_CompileShader_RW(int shader_type, SDL_RWops* shader_source);

/*! Loads shader source from a file, compiles it, and returns the new shader object. */
Uint32 GPU_LoadShader(int shader_type, const char* filename);

/*! Compiles shader source and returns the new shader object. */
Uint32 GPU_CompileShader(int shader_type, const char* shader_source);

/*! Links a shader program with any attached shader objects. */
Uint32 GPU_LinkShaderProgram(Uint32 program_object);

/*! Creates and links a shader program with the given shader objects. */
Uint32 GPU_LinkShaders(Uint32 shader_object1, Uint32 shader_object2);

/*! Deletes a shader object. */
void GPU_FreeShader(Uint32 shader_object);

/*! Deletes a shader program. */
void GPU_FreeShaderProgram(Uint32 program_object);

/*! Attaches a shader object to a shader program for future linking. */
void GPU_AttachShader(Uint32 program_object, Uint32 shader_object);

/*! Detaches a shader object from a shader program. */
void GPU_DetachShader(Uint32 program_object, Uint32 shader_object);

/*! Returns 1 if the given shader program is a default shader for the current context, 0 otherwise. */
Uint8 GPU_IsDefaultShaderProgram(Uint32 program_object);

/*! Activates the given shader program.  Passing NULL for 'block' will disable the built-in shader variables for custom shaders until a GPU_ShaderBlock is set again. */
void GPU_ActivateShaderProgram(Uint32 program_object, GPU_ShaderBlock* block);

/*! Deactivates the current shader program (activates program 0). */
void GPU_DeactivateShaderProgram(void);

/*! Returns the last shader log message. */
const char* GPU_GetShaderMessage(void);

/*! Returns an integer representing the location of the specified attribute shader variable. */
int GPU_GetAttributeLocation(Uint32 program_object, const char* attrib_name);

/*! Returns an integer representing the location of the specified uniform shader variable. */
int GPU_GetUniformLocation(Uint32 program_object, const char* uniform_name);

/*! Loads the given shader program's built-in attribute and uniform locations. */
GPU_ShaderBlock GPU_LoadShaderBlock(Uint32 program_object, const char* position_name, const char* texcoord_name, const char* color_name, const char* modelViewMatrix_name);

/*! Sets the current shader block to use the given attribute and uniform locations. */
void GPU_SetShaderBlock(GPU_ShaderBlock block);

/*! Fills "values" with the value of the uniform shader variable at the given location. */
void GPU_GetUniformiv(Uint32 program_object, int location, int* values);

/*! Sets the value of the integer uniform shader variable at the given location.
    This is equivalent to calling GPU_SetUniformiv(location, 1, 1, &value). */
void GPU_SetUniformi(int location, int value);

/*! Sets the value of the integer uniform shader variable at the given location. */
void GPU_SetUniformiv(int location, int num_elements_per_value, int num_values, int* values);

/*! Fills "values" with the value of the uniform shader variable at the given location. */
void GPU_GetUniformuiv(Uint32 program_object, int location, unsigned int* values);

/*! Sets the value of the unsigned integer uniform shader variable at the given location.
    This is equivalent to calling GPU_SetUniformuiv(location, 1, 1, &value). */
void GPU_SetUniformui(int location, unsigned int value);

/*! Sets the value of the unsigned integer uniform shader variable at the given location. */
void GPU_SetUniformuiv(int location, int num_elements_per_value, int num_values, unsigned int* values);

/*! Fills "values" with the value of the uniform shader variable at the given location. */
void GPU_GetUniformfv(Uint32 program_object, int location, float* values);

/*! Sets the value of the floating point uniform shader variable at the given location.
    This is equivalent to calling GPU_SetUniformfv(location, 1, 1, &value). */
void GPU_SetUniformf(int location, float value);

/*! Sets the value of the floating point uniform shader variable at the given location. */
void GPU_SetUniformfv(int location, int num_elements_per_value, int num_values, float* values);

/*! Fills "values" with the value of the uniform shader variable at the given location.  The results are identical to calling GPU_GetUniformfv().  Matrices are gotten in column-major order. */
void GPU_GetUniformMatrixfv(Uint32 program_object, int location, float* values);

/*! Sets the value of the matrix uniform shader variable at the given location.  The size of the matrices sent is specified by num_rows and num_columns.  Rows and columns must be between 2 and 4. */
void GPU_SetUniformMatrixfv(int location, int num_matrices, int num_rows, int num_columns, Uint8 transpose, float* values);







// Shapes

float GPU_SetThickness(float thickness);

float GPU_GetThickness(void);

void GPU_Pixel(GPU_Target* target, float x, float y, SDL_Color color);

void GPU_Line(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

void GPU_Arc(GPU_Target* target, float x, float y, float radius, float startAngle, float endAngle, SDL_Color color);

void GPU_ArcFilled(GPU_Target* target, float x, float y, float radius, float startAngle, float endAngle, SDL_Color color);

void GPU_Circle(GPU_Target* target, float x, float y, float radius, SDL_Color color);

void GPU_CircleFilled(GPU_Target* target, float x, float y, float radius, SDL_Color color);

void GPU_Tri(GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

void GPU_TriFilled(GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

void GPU_Rectangle(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

void GPU_RectangleFilled(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

void GPU_RectangleRound(GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

void GPU_RectangleRoundFilled(GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

void GPU_Polygon(GPU_Target* target, Uint16 n, float* vertices, SDL_Color color);

void GPU_PolygonFilled(GPU_Target* target, Uint16 n, float* vertices, SDL_Color color);

void GPU_PolygonBlit(GPU_Image* src, GPU_Rect* srcrect, GPU_Target* target, Uint16 n, float* vertices, float textureX, float textureY, float angle, float scaleX, float scaleY);



#ifdef __cplusplus
}
#endif



#endif

