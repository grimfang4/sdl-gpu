#ifndef _SDL_GPU_H__
#define _SDL_GPU_H__

#include "SDL.h"
#include <stdio.h>
#include <stdarg.h>

#if SDL_VERSION_ATLEAST(2,0,0)
    #define SDL_GPU_USE_SDL2
#endif


#ifdef __cplusplus
extern "C" {
#endif




typedef struct GPU_Renderer GPU_Renderer;

/*! Image object for containing pixel/texture data. */
typedef struct GPU_Image
{
	struct GPU_Renderer* renderer;
	void* data;
	Uint16 w, h;
	int channels;
} GPU_Image;


/*! Render target object for use as a blitting destination.
 * A GPU_Target can be created from a GPU_Image with GPU_LoadTarget(). */
typedef struct GPU_Target
{
	struct GPU_Renderer* renderer;
	void* data;
	Uint16 w, h;
	Uint8 useClip;
	SDL_Rect clipRect;
} GPU_Target;

/*! Texture filtering options */
typedef unsigned int GPU_FilterEnum;
static const GPU_FilterEnum GPU_NEAREST = 0;
static const GPU_FilterEnum GPU_LINEAR = 1;
static const GPU_FilterEnum GPU_LINEAR_MIPMAP = 2;

/*! Blending options */
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

/*! Camera object that determines viewing transform. */
typedef struct GPU_Camera
{
	float x, y, z;
	float angle;
	float zoom;
} GPU_Camera;

/*! Renderer object which specializes the API to a particular backend. */
struct GPU_Renderer
{
	/*! String identifier of the renderer. */
	char* id;
	
	/*! Main display surface/framebuffer.  Virtual dimensions can be gotten from this. */
	GPU_Target* display;
	
	/*! Actual window width */
	int window_w;
	
	/*! Actual window height */
	int window_h;
	
	/*! Transforms for the global view. */
	GPU_Camera camera;
	
	/*! Initializes SDL and SDL_gpu.  Creates a window and renderer context. */
	GPU_Target* (*Init)(GPU_Renderer* renderer, Uint16 w, Uint16 h, Uint32 flags);
	
	/*! Sets up this renderer to act as the current renderer.  Called automatically by GPU_SetCurrentRenderer(). */
	void (*SetAsCurrent)(GPU_Renderer* renderer);
	
	/*! Change the actual size of the window. */
	int (*SetDisplayResolution)(GPU_Renderer* renderer, Uint16 w, Uint16 h);
	
	/*! Change the logical size of the window which the drawing commands use. */
	void (*SetVirtualResolution)(GPU_Renderer* renderer, Uint16 w, Uint16 h);
	
	/*! Clean up the renderer state. */
	void (*Quit)(GPU_Renderer* renderer);
	
	/*! Enable/disable fullscreen mode.
	 * On some platforms, this will destroy the renderer context and require that textures be reloaded. */
	int (*ToggleFullscreen)(GPU_Renderer* renderer);

	/*! Sets the renderer's current camera.  If cam is NULL, the default camera is used.
	* \return The old camera. */
	GPU_Camera (*SetCamera)(GPU_Renderer* renderer, GPU_Target* screen, GPU_Camera* cam);
	
	/*! Create a new, blank image.  Don't forget to GPU_FreeImage() it. */
	GPU_Image* (*CreateImage)(GPU_Renderer* renderer, Uint16 w, Uint16 h, Uint8 channels);
	
	/*! Load image from an image file that is supported by this renderer.  Don't forget to GPU_FreeImage() it. */
	GPU_Image* (*LoadImage)(GPU_Renderer* renderer, const char* filename);
	
	/*! Copy an image to a new image.  Don't forget to GPU_FreeImage() both. */
	GPU_Image* (*CopyImage)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! Copy SDL_Surface data into a new GPU_Image.  Don't forget to SDL_FreeSurface() the surface and GPU_FreeImage() the image.*/
	GPU_Image* (*CopyImageFromSurface)(GPU_Renderer* renderer, SDL_Surface* surface);
	
	/*! Deletes an image in the proper way for this renderer. */
	void (*FreeImage)(GPU_Renderer* renderer, GPU_Image* image);
	
    /*! Copies software surface data to a hardware texture.  Draws data with the upper left corner being (x,y).  */
    void (*SubSurfaceCopy)(GPU_Renderer* renderer, SDL_Surface* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y);

	/*! \return The renderer's main display surface/framebuffer. */
	GPU_Target* (*GetDisplayTarget)(GPU_Renderer* renderer);
	
	/*! Creates a new render target from the given image. */
	GPU_Target* (*LoadTarget)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! Deletes a render target in the proper way for this renderer. */
	void (*FreeTarget)(GPU_Renderer* renderer, GPU_Target* target);

	/*! Draws the 'src' image to the 'dest' render target.  Draws the image centered at (x, y).  Note that this is different from many other graphics libraries, but has none of the consequences of an arbitrary offset.
	 * \param srcrect The region of the source image to use.
	 * \param x Destination x-position (centered)
	 * \param y Destination y-position (centered) */
	int (*Blit)(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y);
	
	/*! Rotates and draws the 'src' image to the 'dest' render target.  Draws the image centered at (x, y).
	 * \param srcrect The region of the source image to use.
	 * \param x Destination x-position (centered)
	 * \param y Destination y-position (centered)
	 * \param angle Rotation angle (in degrees) */
	int (*BlitRotate)(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float angle);
	
	/*! Scales and draws the 'src' image to the 'dest' render target.  Draws the image centered at (x, y).
	 * \param srcrect The region of the source image to use.
	 * \param x Destination x-position (centered)
	 * \param y Destination y-position (centered)
	 * \param scaleX Horizontal stretch factor
	 * \param scaleY Vertical stretch factor */
	int (*BlitScale)(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float scaleX, float scaleY);
	
	/*! Scales, rotates, and draws the 'src' image to the 'dest' render target.  Draws the image centered at (x, y).
	 * \param srcrect The region of the source image to use.
	 * \param x Destination x-position (centered)
	 * \param y Destination y-position (centered)
	 * \param angle Rotation angle (in degrees)
	 * \param scaleX Horizontal stretch factor
	 * \param scaleY Vertical stretch factor */
	int (*BlitTransform)(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float angle, float scaleX, float scaleY);
	
	/*! Scales, rotates around a pivot point, and draws the 'src' image to the 'dest' render target.  Draws the image centered at (x, y).
	 * \param srcrect The region of the source image to use.
	 * \param x Destination x-position (centered)
	 * \param y Destination y-position (centered)
	 * \param pivot_x Pivot x-position (on src image)
	 * \param pivot_y Pivot y-position (on src image)
	 * \param angle Rotation angle (in degrees)
	 * \param scaleX Horizontal stretch factor
	 * \param scaleY Vertical stretch factor */
	int (*BlitTransformX)(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float pivot_x, float pivot_y, float angle, float scaleX, float scaleY);
	
	/*! Transforms and draws the 'src' image to the 'dest' render target.  Draws the image centered at (x, y).
	 * \param srcrect The region of the source image to use.
	 * \param x Destination x-position (centered)
	 * \param y Destination y-position (centered)
	 * \param matrix3x3 3x3 matrix in column-major order (index = row + column*numColumns) */
	int (*BlitTransformMatrix)(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float* matrix3x3);
	
	/*! Sets the renderer's z-depth.
	 * \return The previous z-depth */
	float (*SetZ)(GPU_Renderer* renderer, float z);
	
	/*! Gets the renderer's z-depth.
	 * \return The current z-depth */
	float (*GetZ)(GPU_Renderer* renderer);
	
	/*! Loads mipmaps for the given image, if supported by the renderer. */
	void (*GenerateMipmaps)(GPU_Renderer* renderer, GPU_Image* image);

	/*! Gets the current alpha blending setting. */
	Uint8 (*GetBlending)(GPU_Renderer* renderer);

	/*! Enables/disables alpha blending. */
	void (*SetBlending)(GPU_Renderer* renderer, Uint8 enable);
	
	/*! Sets the modulation color for subsequent drawing, if supported by the renderer. */
	void (*SetRGBA)(GPU_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	/*! Changes all pixels of a given color into another color. */
	void (*ReplaceRGB)(GPU_Renderer* renderer, GPU_Image* image, Uint8 from_r, Uint8 from_g, Uint8 from_b, Uint8 to_r, Uint8 to_g, Uint8 to_b);
	
	/*! Changes the alpha value of all pixels of the given color to fully transparent. */
	void (*MakeRGBTransparent)(GPU_Renderer* renderer, GPU_Image* image, Uint8 r, Uint8 g, Uint8 b);
	
	/*! Changes the color of each pixel by shifting the colors in HSV space. */
	void (*ShiftHSV)(GPU_Renderer* renderer, GPU_Image* image, int hue, int saturation, int value);
	
	/*! Changes the color of each pixel by shifting the colors in HSV space, skipping pixels in the given HSV color range. */
	void (*ShiftHSVExcept)(GPU_Renderer* renderer, GPU_Image* image, int hue, int saturation, int value, int notHue, int notSat, int notVal, int range);
	
	/*! \return The RGBA color of a pixel. */
	SDL_Color (*GetPixel)(GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y);
	
	/*! Sets the image filtering mode, if supported by the renderer. */
	void (*SetImageFilter)(GPU_Renderer* renderer, GPU_Image* image, GPU_FilterEnum filter);
	
	/*! Sets the blending mode, if supported by the renderer. */
	void (*SetBlendMode)(GPU_Renderer* renderer, GPU_BlendEnum mode);

	/*! Clears the contents of the given render target. */
	void (*Clear)(GPU_Renderer* renderer, GPU_Target* target);
	/*! Fills the given render target with a color. */
	void (*ClearRGBA)(GPU_Renderer* renderer, GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	/*! Updates the physical display (monitor) with the contents of the display surface/framebuffer. */
	void (*Flip)(GPU_Renderer* renderer);
	
	/*! Renderer-specific data. */
	void* data;
	
};


// System calls
/*! Prints an informational log message. */
void GPU_LogInfo(const char* format, ...);

/*! Prints a warning log message. */
void GPU_LogWarning(const char* format, ...);

/*! Prints an error log message. */
void GPU_LogError(const char* format, ...);


// Setup calls
/*! Initializes SDL and SDL_gpu.  Creates a window and renderer context. */
GPU_Target* GPU_Init(const char* renderer_id, Uint16 w, Uint16 h, Uint32 flags);

/*! Get the actual resolution of the window. */
void GPU_GetDisplayResolution(int* w, int* h);

/*! Change the actual size of the window. */
int GPU_SetDisplayResolution(Uint16 w, Uint16 h);

/*! Change the logical size of the window which the drawing commands use. */
void GPU_SetVirtualResolution(Uint16 w, Uint16 h);

/*! Clean up the renderer state. */
void GPU_CloseCurrentRenderer(void);

/*! Clean up the renderer state and shut down SDL_gpu. */
void GPU_Quit(void);

/*! Sets the current error string. */
void GPU_SetError(const char* fmt, ...);

/*! Gets the current error string. */
const char* GPU_GetErrorString(void);

/*! Converts screen space coordinates to logical drawing coordinates. */
void GPU_GetVirtualCoords(float* x, float* y, float displayX, float displayY);

/*! Enable/disable fullscreen mode.
 * On some platforms, this will destroy the renderer context and require that textures be reloaded. */
int GPU_ToggleFullscreen(void);


// Renderer controls
/*! Gets the current renderer identifier string. */
const char* GPU_GetCurrentRendererID(void);

/*! Gets the renderer identifier string for the given renderer index. */
const char* GPU_GetRendererID(unsigned int index);

/*! Gets the number of active (created) renderers. */
int GPU_GetNumActiveRenderers(void);

/*! Gets an array of identifiers for the active renderers. */
void GPU_GetActiveRendererList(const char** renderers_array);

/*! Gets the number of registered (available) renderers. */
int GPU_GetNumRegisteredRenderers(void);

/*! Gets an array of identifiers for the registered (available) renderers. */
void GPU_GetRegisteredRendererList(const char** renderers_array);

/*! Creates a new renderer matching the given identifier. */
GPU_Renderer* GPU_AddRenderer(const char* id);

/*! Deletes the renderer matching the given identifier. */
void GPU_RemoveRenderer(const char* id);

/*! \return The renderer matching the given identifier. */
GPU_Renderer* GPU_GetRendererByID(const char* id);

/*! Switches the current renderer to the renderer matching the given identifier. */
void GPU_SetCurrentRenderer(const char* id);

/*! \return The current renderer */
GPU_Renderer* GPU_GetCurrentRenderer(void);

/*! \return A GPU_Camera with position (0, 0, -10), angle of 0, and zoom of 1. */
GPU_Camera GPU_GetDefaultCamera(void);

/*! \return The current camera of the current renderer. */
GPU_Camera GPU_GetCamera(void);


// Defined by renderer
/*! Sets the current renderer's current camera.  If cam is NULL, the default camera is used.
 * \return The old camera. */
GPU_Camera GPU_SetCamera(GPU_Target* screen, GPU_Camera* cam);

/*! Create a new, blank image.  Don't forget to GPU_FreeImage() it. */
GPU_Image* GPU_CreateImage(Uint16 w, Uint16 h, Uint8 channels);

/*! Load image from an image file that is supported by this renderer.  Don't forget to GPU_FreeImage() it. */
GPU_Image* GPU_LoadImage(const char* filename);

/*! Copy an image to a new image.  Don't forget to GPU_FreeImage() both. */
GPU_Image* GPU_CopyImage(GPU_Image* image);

/*! Copy SDL_Surface data into a new GPU_Image.  Don't forget to SDL_FreeSurface() the surface and GPU_FreeImage() the image.*/
GPU_Image* GPU_CopyImageFromSurface(SDL_Surface* surface);

/*! Deletes an image in the proper way for this renderer. */
void GPU_FreeImage(GPU_Image* image);

/*! Copies software surface data to a hardware texture.  Draws data with the upper left corner being (x,y).  */
void GPU_SubSurfaceCopy(SDL_Surface* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y);

/*! \return The renderer's main display surface/framebuffer. */
GPU_Target* GPU_GetDisplayTarget(void);

/*! Creates a new render target from the given image. */
GPU_Target* GPU_LoadTarget(GPU_Image* image);

/*! Deletes a render target in the proper way for this renderer. */
void GPU_FreeTarget(GPU_Target* target);

/*! Draws the 'src' image to the 'dest' render target.
    * \param srcrect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position */
int GPU_Blit(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y);

/*! Rotates and draws the 'src' image to the 'dest' render target.
    * \param srcrect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position
    * \param angle Rotation angle (in degrees) */
int GPU_BlitRotate(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float angle);

/*! Scales and draws the 'src' image to the 'dest' render target.
    * \param srcrect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position
    * \param scaleX Horizontal stretch factor
    * \param scaleY Vertical stretch factor */
int GPU_BlitScale(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float scaleX, float scaleY);

/*! Scales, rotates, and draws the 'src' image to the 'dest' render target.
    * \param srcrect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position
    * \param angle Rotation angle (in degrees)
    * \param scaleX Horizontal stretch factor
    * \param scaleY Vertical stretch factor */
int GPU_BlitTransform(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float angle, float scaleX, float scaleY);

	
/*! Scales, rotates around a pivot point, and draws the 'src' image to the 'dest' render target.  The drawing point (x, y) coincides with the pivot point on the src image (pivot_x, pivot_y).
	* \param srcrect The region of the source image to use.
	* \param x Destination x-position
	* \param y Destination y-position
	* \param pivot_x Pivot x-position (in src image coordinates)
	* \param pivot_y Pivot y-position (in src image coordinates)
	* \param angle Rotation angle (in degrees)
	* \param scaleX Horizontal stretch factor
	* \param scaleY Vertical stretch factor */
int GPU_BlitTransformX(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float pivot_x, float pivot_y, float angle, float scaleX, float scaleY);


/*! Transforms and draws the 'src' image to the 'dest' render target.
	* \param srcrect The region of the source image to use.
	* \param x Destination x-position
	* \param y Destination y-position
	* \param matrix3x3 3x3 matrix in column-major order (index = row + column*numColumns) */
int GPU_BlitTransformMatrix(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float* matrix3x3);


/*! Sets the renderer's z-depth.
    * \return The previous z-depth */
float GPU_SetZ(float z);

/*! Gets the renderer's z-depth.
    * \return The current z-depth */
float GPU_GetZ(void);

/*! Loads mipmaps for the given image, if supported by the renderer. */
void GPU_GenerateMipmaps(GPU_Image* image);

/*! Sets the clipping rect for the given render target. */
SDL_Rect GPU_SetClipRect(GPU_Target* target, SDL_Rect rect);

/*! Sets the clipping rect for the given render target. */
SDL_Rect GPU_SetClip(GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h);

/*! Clears (resets) the clipping rect for the given render target. */
void GPU_ClearClip(GPU_Target* target);

/*! Gets the current alpha blending setting. */
Uint8 GPU_GetBlending(void);

/*! Enables/disables alpha blending. */
void GPU_SetBlending(Uint8 enable);

/*! Sets the modulation color for subsequent drawing, if supported by the renderer. */
void GPU_SetColor(SDL_Color* color);

/*! Sets the modulation color for subsequent drawing, if supported by the renderer. */
void GPU_SetRGB(Uint8 r, Uint8 g, Uint8 b);

/*! Sets the modulation color for subsequent drawing, if supported by the renderer. */
void GPU_SetRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/*! Changes all pixels of a given color into another color. */
void GPU_ReplaceColor(GPU_Image* image, SDL_Color from, SDL_Color to);

/*! Changes the alpha value of all pixels of the given color to fully transparent. */
void GPU_MakeColorTransparent(GPU_Image* image, SDL_Color color);

/*! Changes the color of each pixel by shifting the colors in HSV space. */
void GPU_ShiftHSV(GPU_Image* image, int hue, int saturation, int value);

/*! Changes the color of each pixel by shifting the colors in HSV space, skipping pixels in the given HSV color range. */
void GPU_ShiftHSVExcept(GPU_Image* image, int hue, int saturation, int value, int notHue, int notSat, int notVal, int range);

/*! \return The RGBA color of a pixel. */
SDL_Color GPU_GetPixel(GPU_Target* target, Sint16 x, Sint16 y);

/*! Sets the image filtering mode, if supported by the renderer. */
void GPU_SetImageFilter(GPU_Image* image, GPU_FilterEnum filter);
	
/*! Sets the blending mode, if supported by the renderer. */
void GPU_SetBlendMode(GPU_BlendEnum mode);

/*! Clears the contents of the given render target. */
void GPU_Clear(GPU_Target* target);

/*! Fills the given render target with a color. */
void GPU_ClearRGBA(GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/*! Updates the physical display (monitor) with the contents of the display surface/framebuffer. */
void GPU_Flip(void);








// Shapes

typedef struct GPU_ShapeRenderer GPU_ShapeRenderer;

struct GPU_ShapeRenderer
{
	GPU_Renderer* renderer;
	
	float (*SetThickness)(GPU_ShapeRenderer* shapeRenderer, float thickness);
	float (*GetThickness)(GPU_ShapeRenderer* shapeRenderer);
	
	void (*Pixel)(GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, float x, float y, SDL_Color color);

	void (*Line)(GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

	void (*Arc)(GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, float x, float y, float radius, float startAngle, float endAngle, SDL_Color color);
	
	void (*ArcFilled)(GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, float x, float y, float radius, float startAngle, float endAngle, SDL_Color color);

	void (*Circle)(GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, float x, float y, float radius, SDL_Color color);

	void (*CircleFilled)(GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, float x, float y, float radius, SDL_Color color);

	void (*Tri)(GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

	void (*TriFilled)(GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

	void (*Rect)(GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

	void (*RectFilled)(GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

	void (*RectRound)(GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

	void (*RectRoundFilled)(GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

	void (*Polygon)(GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color);

	void (*PolygonFilled)(GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color);

	void (*PolygonBlit)(GPU_ShapeRenderer* shapeRenderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* target, Uint16 n, float* vertices, float textureX, float textureY, float angle, float scaleX, float scaleY);
	
	void* data;
	
};

// Call this after setting a GPU_Renderer (e.g after GPU_Init())
void GPU_LoadShapeRenderer(void);

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

void GPU_Rect(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

void GPU_RectFilled(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

void GPU_RectRound(GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

void GPU_RectRoundFilled(GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

void GPU_Polygon(GPU_Target* target, Uint16 n, float* vertices, SDL_Color color);

void GPU_PolygonFilled(GPU_Target* target, Uint16 n, float* vertices, SDL_Color color);

void GPU_PolygonBlit(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* target, Uint16 n, float* vertices, float textureX, float textureY, float angle, float scaleX, float scaleY);




#ifdef __cplusplus
}
#endif



#endif

