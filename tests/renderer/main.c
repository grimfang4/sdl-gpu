#include "SDL.h"
#include "SDL_gpu.h"
#include "SDL_gpu_RendererImpl.h"
#include "common.h"
#include <stdlib.h>


#ifdef _MSC_VER
	#define __func__ __FUNCTION__
#endif


#ifdef SDL_GPU_USE_SDL1
// This demo doesn't work for SDL 1.2 because of the assumed windowing features here.
int main(int argc, char* argv[])
{
    GPU_LogError("Sorry, this demo requires SDL 2.\n");
    return 0;
}

#else

// Custom renderer implementation
/*
Notes:
    - Every function must be implemented (even just a dummy implementation is fine).
        SDL_gpu does not do NULL checks on the renderer implementation function pointers because it is better to have to
        copy and paste a bunch of dummy functions than to force every good renderer to do extra NULL checks.
    - When SDL_gpu calls one of these functions internally, 'renderer' and 'renderer->current_context_target' will never be NULL.
    - Since each of these calls needs a renderer, SDL_gpu will always be using the current active renderer.
    - These dummy implementations do not always flush the blit buffer as needed by real renderers.  This is because the dummy has no storage for renderer-specific state changes.
*/

static GPU_Target* Init(GPU_Renderer* renderer, GPU_RendererID renderer_request, Uint16 w, Uint16 h, GPU_WindowFlagEnum SDL_flags)
{
	SDL_Window* window;
	
    GPU_Log(" %s (dummy)\n", __func__);
    
	renderer->requested_id = renderer_request;
    renderer->GPU_init_flags = GPU_GetPreInitFlags();
	renderer->SDL_init_flags = SDL_flags;
	
	window = NULL;
    // Is there a window already set up that we are supposed to use?
    if(renderer->current_context_target != NULL)
        window = SDL_GetWindowFromID(renderer->current_context_target->context->windowID);
    else
        window = SDL_GetWindowFromID(GPU_GetInitWindow());
    
    if(window == NULL)
    {
        // Set up window flags
        if(!(renderer->SDL_init_flags & SDL_WINDOW_HIDDEN))
            renderer->SDL_init_flags |= SDL_WINDOW_SHOWN;
        
        window = SDL_CreateWindow("",
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  w, h,
                                  renderer->SDL_init_flags);

        if(window == NULL)
        {
            GPU_PushErrorCode("GPU_Init", GPU_ERROR_BACKEND_ERROR, "Window creation failed.");
            return NULL;
        }
        
        GPU_SetInitWindow(SDL_GetWindowID(window));
    }
    else
        renderer->SDL_init_flags = SDL_flags;
	
	renderer->enabled_features = 0xFFFFFFFF;  // Pretend to support them all
	
	renderer->current_context_target = renderer->impl->CreateTargetFromWindow(renderer, SDL_GetWindowID(window), renderer->current_context_target);
    if(renderer->current_context_target == NULL)
        return NULL;
    
    
    // If the dimensions of the window don't match what we asked for, then set up a virtual resolution to pretend like they are.
    if(!(renderer->GPU_init_flags & GPU_INIT_DISABLE_AUTO_VIRTUAL_RESOLUTION) && w != 0 && h != 0 && (w != renderer->current_context_target->w || h != renderer->current_context_target->h))
        renderer->impl->SetVirtualResolution(renderer, renderer->current_context_target, w, h);
    
    return renderer->current_context_target;
}

static GPU_Target* CreateTargetFromWindow(GPU_Renderer* renderer, Uint32 windowID, GPU_Target* target)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    if(target == NULL)
    {
        target = (GPU_Target*)malloc(sizeof(GPU_Target));
        memset(target, 0, sizeof(GPU_Target));
        
        target->refcount = 1;
        target->is_alias = 0;
        target->data = NULL;  // Allocate a data structure as needed for other render target data
        target->image = NULL;
        
        target->context = (GPU_Context*)malloc(sizeof(GPU_Context));
        memset(target->context, 0, sizeof(GPU_Context));
        
        target->context->refcount = 1;
        target->context->windowID = windowID;
        target->context->data = NULL;  // Allocate a data structure as needed for other context data
        target->context->context = NULL;
    }
    else
    {
        GPU_RemoveWindowMapping(target->context->windowID);
    }
    
    // This is used for restoring window after fullscreen switches
    SDL_GetWindowSize(SDL_GetWindowFromID(target->context->windowID), &target->context->window_w, &target->context->window_h);
    target->context->stored_window_w = target->context->window_w;
    target->context->stored_window_h = target->context->window_h;
    
    GPU_AddWindowMapping(target);
    
    
    target->renderer = renderer;
    target->w = target->context->window_w;
    target->h = target->context->window_h;
    target->base_w = target->context->window_w;
    target->base_h = target->context->window_h;

    target->use_clip_rect = 0;
    target->clip_rect.x = 0;
    target->clip_rect.y = 0;
    target->clip_rect.w = target->w;
    target->clip_rect.h = target->h;
    target->use_color = 0;
    
    target->viewport = GPU_MakeRect(0, 0, target->context->window_w, target->context->window_h);
    target->camera = GPU_GetDefaultCamera();
    
    target->context->line_thickness = 1.0f;
    target->context->use_texturing = 1;
    target->context->shapes_use_blending = 1;
    target->context->shapes_blend_mode = GPU_GetBlendModeFromPreset(GPU_BLEND_NORMAL);
    

    GPU_InitMatrixStack(&target->projection_matrix);
    GPU_InitMatrixStack(&target->view_matrix);
    GPU_InitMatrixStack(&target->model_matrix);

    target->matrix_mode = GPU_MODEL;
    
    
    renderer->impl->SetLineThickness(renderer, 1.0f);
    
    
    target->context->default_textured_shader_program = 0;
    target->context->default_untextured_shader_program = 0;
    target->context->current_shader_program = 0;
    
    return target;
}

static GPU_Target* CreateAliasTarget(GPU_Renderer* renderer, GPU_Target* target)
{
	GPU_Target* result;
    
    GPU_Log(" %s (dummy)\n", __func__);

    if(target == NULL)
        return NULL;
    
    result = (GPU_Target*)malloc(sizeof(GPU_Target));
    
    // Copy the members
    *result = *target;

	// Deep copies
	GPU_CopyMatrixStack(&target->projection_matrix, &result->projection_matrix);
	GPU_CopyMatrixStack(&target->view_matrix, &result->view_matrix);
	GPU_CopyMatrixStack(&target->model_matrix, &result->model_matrix);
    
    // Alias info
    if(target->image != NULL)
        target->image->refcount++;
    if(target->context != NULL)
        target->context->refcount++;
    result->refcount = 1;
    result->is_alias = 1;

    return result;
}

static void MakeCurrent(GPU_Renderer* renderer, GPU_Target* target, Uint32 windowID)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    if(target == NULL || target->context == NULL)
        return;
    
    renderer->current_context_target = target;
    
    // Reset if the target's window was changed
    if(target->context->windowID != windowID)
    {
        // Update the window mappings
        GPU_RemoveWindowMapping(windowID);
        // Don't remove the target's current mapping.  That lets other windows refer to it.
        target->context->windowID = windowID;
        GPU_AddWindowMapping(target);
        
        // Update target's window size
        SDL_GetWindowSize(SDL_GetWindowFromID(windowID), &target->context->window_w, &target->context->window_h);
        target->base_w = target->context->window_w;
        target->base_h = target->context->window_h;
        
        // Reset the camera here for this window
    }
}
	
static void SetAsCurrent(GPU_Renderer* renderer)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    if(renderer->current_context_target == NULL)
        return;
    
    renderer->impl->MakeCurrent(renderer, renderer->current_context_target, renderer->current_context_target->context->windowID);
}

static GPU_bool SetActiveTarget(GPU_Renderer* renderer, GPU_Target* target)
{
    GPU_Log(" %s (dummy)\n", __func__);
    if(renderer->current_context_target == NULL)
        return GPU_FALSE;
    
    if(renderer->current_context_target->context->active_target != target)
    {
        renderer->impl->FlushBlitBuffer(renderer);
        renderer->current_context_target->context->active_target = target;
    }
    return GPU_TRUE;
}
	
static void ResetRendererState(GPU_Renderer* renderer)
{
    GPU_Log(" %s (dummy)\n", __func__);
}

static GPU_bool AddDepthBuffer(GPU_Renderer* renderer, GPU_Target* target)
{
    GPU_Log(" %s (dummy)\n", __func__);
    return GPU_TRUE;
}
	
static GPU_bool SetWindowResolution(GPU_Renderer* renderer, Uint16 w, Uint16 h)
{
    GPU_Target* target = renderer->current_context_target;
    
    GPU_Log(" %s (dummy)\n", __func__);
    
    if(target == NULL)
        return 0;
    
    // Don't need to resize (only update internals) when resolution isn't changing.
    SDL_GetWindowSize(SDL_GetWindowFromID(target->context->windowID), &target->context->window_w, &target->context->window_h);
    if(target->context->window_w != w || target->context->window_h != h)
    {
        SDL_SetWindowSize(SDL_GetWindowFromID(target->context->windowID), w, h);
        SDL_GetWindowSize(SDL_GetWindowFromID(target->context->windowID), &target->context->window_w, &target->context->window_h);
    }
    
    // Store the resolution for fullscreen_desktop changes
    target->context->stored_window_w = target->context->window_w;
    target->context->stored_window_h = target->context->window_h;
    
    // Update base dimensions
    target->base_w = target->context->window_w;
    target->base_h = target->context->window_h;
    
    // Resets virtual resolution
    target->w = target->base_w;
    target->h = target->base_h;
    target->using_virtual_resolution = 0;
    
    return 1;
}
	
static void SetVirtualResolution(GPU_Renderer* renderer, GPU_Target* target, Uint16 w, Uint16 h)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    if(target == NULL)
        return;
    
    target->w = w;
    target->h = h;
    target->using_virtual_resolution = 1;
}


static void UnsetVirtualResolution(GPU_Renderer* renderer, GPU_Target* target)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    if(target == NULL)
        return;
    
    target->w = target->base_w;
    target->h = target->base_h;
    target->using_virtual_resolution = 0;
}


static void Quit(GPU_Renderer* renderer)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    renderer->impl->FreeTarget(renderer, renderer->current_context_target);
    renderer->current_context_target = NULL;
}


static GPU_bool SetFullscreen(GPU_Renderer* renderer, GPU_bool enable_fullscreen, GPU_bool use_desktop_resolution)
{
    GPU_Target* target = renderer->current_context_target;
    
    // These values should actually come from the window
    Uint8 was_fullscreen = !enable_fullscreen;
    Uint8 is_fullscreen = enable_fullscreen;
    
    GPU_Log(" %s (dummy)\n", __func__);
    
    //if(SetWindowFullscreen(target->context->windowID, enable_fullscreen) >= 0)
    {
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
            SDL_SetWindowSize(SDL_GetWindowFromID(target->context->windowID), target->context->stored_window_w, target->context->stored_window_h);
        
        // Update window dims
        SDL_GetWindowSize(SDL_GetWindowFromID(target->context->windowID), &target->context->window_w, &target->context->window_h);
    }

    if(is_fullscreen != was_fullscreen)
    {
        // If virtual res is not set, we need to update the target dims and reset stuff that no longer is right
        if(!target->using_virtual_resolution)
        {
            // Update dims
            target->w = target->context->window_w;
            target->h = target->context->window_h;
        }

        // Reset viewport
        target->viewport = GPU_MakeRect(0, 0, target->context->window_w, target->context->window_h);
        // Update viewport here
        
        // Reset clip
        GPU_UnsetClip(target);
        
        // Update camera here
    }
    
    target->base_w = target->context->window_w;
    target->base_h = target->context->window_h;
    
    return is_fullscreen;
}


static GPU_Camera SetCamera(GPU_Renderer* renderer, GPU_Target* target, GPU_Camera* cam)
{
    GPU_Camera new_camera;
	GPU_Camera old_camera;
	
    GPU_Log(" %s (dummy)\n", __func__);

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
    target->camera = new_camera;

    return old_camera;
}


static GPU_Image* CreateImage(GPU_Renderer* renderer, Uint16 w, Uint16 h, GPU_FormatEnum format)
{
    int num_layers, bytes_per_pixel;
	GPU_Image* result;
	SDL_Color white = { 255, 255, 255, 255 };
	
    GPU_Log(" %s (dummy)\n", __func__);

    if(format < 1)
    {
        GPU_PushErrorCode("GPU_CreateImage", GPU_ERROR_DATA_ERROR, "Unsupported image format (0x%x)", format);
        return NULL;
    }
    

    switch(format)
    {
        case GPU_FORMAT_LUMINANCE:
            num_layers = 1;
            bytes_per_pixel = 1;
            break;
        case GPU_FORMAT_LUMINANCE_ALPHA:
            num_layers = 1;
            bytes_per_pixel = 2;
            break;
        case GPU_FORMAT_RGB:
            num_layers = 1;
            bytes_per_pixel = 3;
            break;
        case GPU_FORMAT_RGBA:
            num_layers = 1;
            bytes_per_pixel = 4;
            break;
        case GPU_FORMAT_ALPHA:
            num_layers = 1;
            bytes_per_pixel = 1;
            break;
        case GPU_FORMAT_RG:
            num_layers = 1;
            bytes_per_pixel = 2;
            break;
        case GPU_FORMAT_YCbCr420P:
            num_layers = 3;
            bytes_per_pixel = 1;
            break;
        case GPU_FORMAT_YCbCr422:
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

    result = (GPU_Image*)malloc(sizeof(GPU_Image));
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
    result->filter_mode = GPU_FILTER_LINEAR;
    result->snap_mode = GPU_SNAP_POSITION_AND_DIMENSIONS;
    result->wrap_mode_x = GPU_WRAP_NONE;
    result->wrap_mode_y = GPU_WRAP_NONE;
    
    result->data = NULL;  // Allocate a data structure as needed for other image data
    result->is_alias = 0;

    result->using_virtual_resolution = 0;
    result->w = w;
    result->h = h;
    result->base_w = w;
    result->base_h = h;
    result->texture_w = w;
    result->texture_h = h;

    return result;
}


static GPU_Image* CreateImageUsingTexture(GPU_Renderer* renderer, GPU_TextureHandle handle, GPU_bool take_ownership)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    return renderer->impl->CreateImage(renderer, 100, 100, GPU_FORMAT_RGBA);
}


static GPU_Image* CreateAliasImage(GPU_Renderer* renderer, GPU_Image* image)
{
	GPU_Image* result;
	
    GPU_Log(" %s (dummy)\n", __func__);
    
    if(image == NULL)
        return NULL;

    result = (GPU_Image*)malloc(sizeof(GPU_Image));
    // Copy the members
    *result = *image;
    
    // Alias info
    result->refcount = 1;
    result->is_alias = 1;

    return result;
}


static GPU_bool SaveImage(GPU_Renderer* renderer, GPU_Image* image, const char* filename, GPU_FileFormatEnum format)
{
    SDL_Surface* surface;
    
    GPU_Log(" %s (dummy)\n", __func__);
    
    surface = GPU_CopySurfaceFromImage(image);
    GPU_SaveSurface(surface, filename, format);
    
    SDL_FreeSurface(surface);
    return 1;
}


static GPU_Image* CopyImage(GPU_Renderer* renderer, GPU_Image* image)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    if(image == NULL)
        return NULL;
    
    return renderer->impl->CreateImage(renderer, image->w, image->h, image->format);
}


static void UpdateImage(GPU_Renderer* renderer, GPU_Image* image, const GPU_Rect* image_rect, SDL_Surface* surface, const GPU_Rect* surface_rect)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void UpdateImageBytes(GPU_Renderer* renderer, GPU_Image* image, const GPU_Rect* image_rect, const unsigned char* bytes, int bytes_per_row)
{
    GPU_Log(" %s (dummy)\n", __func__);
}

static GPU_bool ReplaceImage(GPU_Renderer* renderer, GPU_Image* image, SDL_Surface* surface, const GPU_Rect* surface_rect)
{
    GPU_Log(" %s (dummy)\n", __func__);
    return 0;
}

static GPU_Image* CopyImageFromSurface(GPU_Renderer* renderer, SDL_Surface* surface)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    if(surface == NULL)
        return NULL;
    
    return renderer->impl->CreateImage(renderer, surface->w, surface->h, GPU_FORMAT_RGBA);
}


static GPU_Image* CopyImageFromTarget(GPU_Renderer* renderer, GPU_Target* target)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    if(target == NULL)
        return NULL;
    
    return renderer->impl->CreateImage(renderer, target->w, target->h, GPU_FORMAT_RGBA);
}


static SDL_Surface* CopySurfaceFromTarget(GPU_Renderer* renderer, GPU_Target* target)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    if(target == NULL)
        return NULL;
    
    return SDL_CreateRGBSurface(SDL_SWSURFACE, target->base_w, target->base_h, 32, 0, 0, 0, 0);
}


static SDL_Surface* CopySurfaceFromImage(GPU_Renderer* renderer, GPU_Image* image)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    if(image == NULL)
        return NULL;
    
    return SDL_CreateRGBSurface(SDL_SWSURFACE, image->texture_w, image->texture_h, 32, 0, 0, 0, 0);
}


static void FreeImage(GPU_Renderer* renderer, GPU_Image* image)
{
    GPU_Log(" %s (dummy)\n", __func__);

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
    
    free(image);
}


static GPU_Target* GetTarget(GPU_Renderer* renderer, GPU_Image* image)
{
	GPU_Target* result;
	
    GPU_Log(" %s (dummy)\n", __func__);
    
    if(image == NULL)
        return NULL;

    if(image->target != NULL)
        return image->target;

    if(!(renderer->enabled_features & GPU_FEATURE_RENDER_TARGETS))
        return NULL;

    result = (GPU_Target*)malloc(sizeof(GPU_Target));
    memset(result, 0, sizeof(GPU_Target));
    result->refcount = 0;
    result->data = NULL;  // Allocate a data structure as needed for other render target data
    
    result->renderer = renderer;
    result->context = NULL;
    result->image = image;
    result->w = image->w;
    result->h = image->h;
    result->base_w = image->texture_w;
    result->base_h = image->texture_h;
    
    result->viewport = GPU_MakeRect(0, 0, result->w, result->h);
    
    GPU_InitMatrixStack(&result->projection_matrix);
    GPU_InitMatrixStack(&result->view_matrix);
    GPU_InitMatrixStack(&result->model_matrix);

    result->matrix_mode = GPU_MODEL;
    
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
    GPU_Log(" %s (dummy)\n", __func__);

    if(target == NULL)
        return;
    
    if(target->refcount > 1)
    {
        target->refcount--;
        return;
    }
    
    if(target->context != NULL && target->context->failed)
    {
        if(target == renderer->current_context_target)
            renderer->current_context_target = NULL;
        
        // Remove all of the window mappings that refer to this target
        GPU_RemoveWindowMappingByTarget(target);
        
        free(target->context);
        free(target);
        return;
    }
    
    if(target == renderer->current_context_target)
    {
        renderer->impl->FlushBlitBuffer(renderer);
        renderer->current_context_target = NULL;
    }
    
    if(!target->is_alias && target->image != NULL)
        target->image->target = NULL;  // Remove reference to this object
    
    if(target->context != NULL)
    {
        // Remove all of the window mappings that refer to this target
        GPU_RemoveWindowMappingByTarget(target);
        
        free(target->context);
        target->context = NULL;
    }
    
    free(target);
}


static void Blit(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void BlitRotate(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void BlitScale(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float scaleX, float scaleY)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void BlitTransform(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees, float scaleX, float scaleY)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void BlitTransformX(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float pivot_x, float pivot_y, float degrees, float scaleX, float scaleY)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void PrimitiveBatchV(GPU_Renderer* renderer, GPU_Image* image, GPU_Target* target, GPU_PrimitiveEnum primitive_type, unsigned short num_vertices, void* values, unsigned int num_indices, unsigned short* indices, GPU_BatchFlagEnum flags)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void GenerateMipmaps(GPU_Renderer* renderer, GPU_Image* image)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static GPU_Rect SetClip(GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
	GPU_Rect r;
	
    GPU_Log(" %s (dummy)\n", __func__);
    
    if(target == NULL)
    {
        GPU_Rect r = {0,0,0,0};
        return r;
    }

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
    GPU_Log(" %s (dummy)\n", __func__);
    
    target->use_clip_rect = 0;
}


static SDL_Color GetPixel(GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    return GPU_MakeColor(0, 0, 0, 0);
}


static void SetImageFilter(GPU_Renderer* renderer, GPU_Image* image, GPU_FilterEnum filter)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
	image->filter_mode = filter;
}


static void SetWrapMode(GPU_Renderer* renderer, GPU_Image* image, GPU_WrapEnum wrap_mode_x, GPU_WrapEnum wrap_mode_y)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
	image->wrap_mode_x = wrap_mode_x;
	image->wrap_mode_y = wrap_mode_y;
}

static GPU_TextureHandle GetTextureHandle(GPU_Renderer* renderer, GPU_Image* image)
{
    GPU_Log(" %s (dummy)\n", __func__);
    return 0;
}

static void ClearRGBA(GPU_Renderer* renderer, GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    GPU_Log(" %s (dummy)\n", __func__);
}

static void FlushBlitBuffer(GPU_Renderer* renderer)
{
    GPU_Log(" %s (dummy)\n", __func__);
}

static void Flip(GPU_Renderer* renderer, GPU_Target* target)
{
    GPU_Log(" %s (dummy)\n", __func__);
}

static Uint32 CreateShaderProgram(GPU_Renderer* renderer)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    return 0;
}

static void FreeShaderProgram(GPU_Renderer* renderer, Uint32 program_object)
{
    GPU_Log(" %s (dummy)\n", __func__);
}

static Uint32 CompileShader_RW(GPU_Renderer* renderer, GPU_ShaderEnum shader_type, SDL_RWops* shader_source, GPU_bool free_rwops)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    return 0;
}

static Uint32 CompileShader(GPU_Renderer* renderer, GPU_ShaderEnum shader_type, const char* shader_source)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    return 0;
}

static void FreeShader(GPU_Renderer* renderer, Uint32 shader_object)
{
    GPU_Log(" %s (dummy)\n", __func__);
}

static void AttachShader(GPU_Renderer* renderer, Uint32 program_object, Uint32 shader_object)
{
    GPU_Log(" %s (dummy)\n", __func__);
}

static void DetachShader(GPU_Renderer* renderer, Uint32 program_object, Uint32 shader_object)
{
    GPU_Log(" %s (dummy)\n", __func__);
}

static GPU_bool LinkShaderProgram(GPU_Renderer* renderer, Uint32 program_object)
{
    GPU_Log(" %s (dummy)\n", __func__);
    
    return 1;
}

static void ActivateShaderProgram(GPU_Renderer* renderer, Uint32 program_object, GPU_ShaderBlock* block)
{
    GPU_Target* target = renderer->current_context_target;
    
    GPU_Log(" %s (dummy)\n", __func__);
    
    if(target == NULL)
        return;
    
    if((renderer->enabled_features & GPU_FEATURE_BASIC_SHADERS) == GPU_FEATURE_BASIC_SHADERS)
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
    }
    
    target->context->current_shader_program = program_object;
}


static void DeactivateShaderProgram(GPU_Renderer* renderer)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static const char* GetShaderMessage(GPU_Renderer* renderer)
{
    GPU_Log(" %s (dummy)\n", __func__);
    return "";
}


static int GetAttributeLocation(GPU_Renderer* renderer, Uint32 program_object, const char* attrib_name)
{
    GPU_Log(" %s (dummy)\n", __func__);
    return -1;
}


static int GetUniformLocation(GPU_Renderer* renderer, Uint32 program_object, const char* uniform_name)
{
    GPU_Log(" %s (dummy)\n", __func__);
    return -1;
}


static GPU_ShaderBlock LoadShaderBlock(GPU_Renderer* renderer, Uint32 program_object, const char* position_name, const char* texcoord_name, const char* color_name, const char* modelViewMatrix_name)
{
    GPU_ShaderBlock b;
    
    GPU_Log(" %s (dummy)\n", __func__);
    
    b.position_loc = -1;
    b.texcoord_loc = -1;
    b.color_loc = -1;
    b.modelViewProjection_loc = -1;
    return b;
}


static void SetShaderImage(GPU_Renderer* renderer, GPU_Image* image, int location, int image_unit)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void GetUniformiv(GPU_Renderer* renderer, Uint32 program_object, int location, int* values)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void SetUniformi(GPU_Renderer* renderer, int location, int value)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void SetUniformiv(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, int* values)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void GetUniformuiv(GPU_Renderer* renderer, Uint32 program_object, int location, unsigned int* values)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void SetUniformui(GPU_Renderer* renderer, int location, unsigned int value)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void SetUniformuiv(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, unsigned int* values)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void GetUniformfv(GPU_Renderer* renderer, Uint32 program_object, int location, float* values)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void SetUniformf(GPU_Renderer* renderer, int location, float value)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void SetUniformfv(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, float* values)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void SetUniformMatrixfv(GPU_Renderer* renderer, int location, int num_matrices, int num_rows, int num_columns, GPU_bool transpose, float* values)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void SetAttributef(GPU_Renderer* renderer, int location, float value)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void SetAttributei(GPU_Renderer* renderer, int location, int value)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void SetAttributeui(GPU_Renderer* renderer, int location, unsigned int value)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void SetAttributefv(GPU_Renderer* renderer, int location, int num_elements, float* value)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void SetAttributeiv(GPU_Renderer* renderer, int location, int num_elements, int* value)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void SetAttributeuiv(GPU_Renderer* renderer, int location, int num_elements, unsigned int* value)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void SetAttributeSource(GPU_Renderer* renderer, int num_values, GPU_Attribute source)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


// Shapes


static float SetLineThickness(GPU_Renderer* renderer, float thickness)
{
    GPU_Log(" %s (dummy)\n", __func__);
    return 1.0f;
}


static float GetLineThickness(GPU_Renderer* renderer)
{
    GPU_Log(" %s (dummy)\n", __func__);
    return 1.0f;
}


static void Pixel(GPU_Renderer* renderer, GPU_Target* target, float x, float y, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void Line(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void Arc(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void ArcFilled(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void Circle(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void CircleFilled(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void Ellipse(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float rx, float ry, float degrees, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void EllipseFilled(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float rx, float ry, float degrees, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void Sector(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void SectorFilled(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void Tri(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void TriFilled(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void Rectangle(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void RectangleFilled(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void RectangleRound(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void RectangleRoundFilled(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}


static void Polygon(GPU_Renderer* renderer, GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}

static void PolygonFilled(GPU_Renderer* renderer, GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color)
{
    GPU_Log(" %s (dummy)\n", __func__);
}




void set_renderer_functions(GPU_RendererImpl* impl)
{
    impl->Init = &Init;
    impl->CreateTargetFromWindow = &CreateTargetFromWindow;
    impl->CreateAliasTarget = &CreateAliasTarget;
    impl->MakeCurrent = &MakeCurrent;
    impl->SetAsCurrent = &SetAsCurrent;
    impl->SetActiveTarget = &SetActiveTarget;
    impl->ResetRendererState = &ResetRendererState;
    impl->AddDepthBuffer = &AddDepthBuffer;
    impl->SetWindowResolution = &SetWindowResolution;
    impl->SetVirtualResolution = &SetVirtualResolution;
    impl->UnsetVirtualResolution = &UnsetVirtualResolution;
    impl->Quit = &Quit;

    impl->SetFullscreen = &SetFullscreen;
    impl->SetCamera = &SetCamera;

    impl->CreateImage = &CreateImage;
    impl->CreateImageUsingTexture = &CreateImageUsingTexture;
    impl->CreateAliasImage = &CreateAliasImage;
    impl->SaveImage = &SaveImage;
    impl->CopyImage = &CopyImage;
    impl->UpdateImage = &UpdateImage;
    impl->UpdateImageBytes = &UpdateImageBytes;
    impl->ReplaceImage = &ReplaceImage;
    impl->CopyImageFromSurface = &CopyImageFromSurface;
    impl->CopyImageFromTarget = &CopyImageFromTarget;
    impl->CopySurfaceFromTarget = &CopySurfaceFromTarget;
    impl->CopySurfaceFromImage = &CopySurfaceFromImage;
    impl->FreeImage = &FreeImage;

    impl->GetTarget = &GetTarget;
    impl->FreeTarget = &FreeTarget;

    impl->Blit = &Blit;
    impl->BlitRotate = &BlitRotate;
    impl->BlitScale = &BlitScale;
    impl->BlitTransform = &BlitTransform;
    impl->BlitTransformX = &BlitTransformX;
    impl->PrimitiveBatchV = &PrimitiveBatchV;

    impl->GenerateMipmaps = &GenerateMipmaps;

    impl->SetClip = &SetClip;
    impl->UnsetClip = &UnsetClip;
    
    impl->GetPixel = &GetPixel;
    impl->SetImageFilter = &SetImageFilter;
    impl->SetWrapMode = &SetWrapMode;
    impl->GetTextureHandle = &GetTextureHandle;

    impl->ClearRGBA = &ClearRGBA;
    impl->FlushBlitBuffer = &FlushBlitBuffer;
    impl->Flip = &Flip;
    
    impl->CreateShaderProgram = &CreateShaderProgram;
    impl->FreeShaderProgram = &FreeShaderProgram;
    impl->CompileShader_RW = &CompileShader_RW;
    impl->CompileShader = &CompileShader;
    impl->FreeShader = &FreeShader;
    impl->AttachShader = &AttachShader;
    impl->DetachShader = &DetachShader;
    impl->LinkShaderProgram = &LinkShaderProgram;
    impl->ActivateShaderProgram = &ActivateShaderProgram;
    impl->DeactivateShaderProgram = &DeactivateShaderProgram;
    impl->GetShaderMessage = &GetShaderMessage;
    impl->GetAttributeLocation = &GetAttributeLocation;
    impl->GetUniformLocation = &GetUniformLocation;
    impl->LoadShaderBlock = &LoadShaderBlock;
    impl->SetShaderImage = &SetShaderImage;
    impl->GetUniformiv = &GetUniformiv;
    impl->SetUniformi = &SetUniformi;
    impl->SetUniformiv = &SetUniformiv;
    impl->GetUniformuiv = &GetUniformuiv;
    impl->SetUniformui = &SetUniformui;
    impl->SetUniformuiv = &SetUniformuiv;
    impl->GetUniformfv = &GetUniformfv;
    impl->SetUniformf = &SetUniformf;
    impl->SetUniformfv = &SetUniformfv;
    impl->SetUniformMatrixfv = &SetUniformMatrixfv;
    impl->SetAttributef = &SetAttributef;
    impl->SetAttributei = &SetAttributei;
    impl->SetAttributeui = &SetAttributeui;
    impl->SetAttributefv = &SetAttributefv;
    impl->SetAttributeiv = &SetAttributeiv;
    impl->SetAttributeuiv = &SetAttributeuiv;
    impl->SetAttributeSource = &SetAttributeSource;
	
	/* Shape rendering */
	
    impl->SetLineThickness = &SetLineThickness;
    impl->GetLineThickness = &GetLineThickness;
    impl->Pixel = &Pixel;
    impl->Line = &Line;
    impl->Arc = &Arc;
    impl->ArcFilled = &ArcFilled;
    impl->Circle = &Circle;
    impl->CircleFilled = &CircleFilled;
    impl->Ellipse = &Ellipse;
    impl->EllipseFilled = &EllipseFilled;
    impl->Sector = &Sector;
    impl->SectorFilled = &SectorFilled;
    impl->Tri = &Tri;
    impl->TriFilled = &TriFilled;
    impl->Rectangle = &Rectangle;
    impl->RectangleFilled = &RectangleFilled;
    impl->RectangleRound = &RectangleRound;
    impl->RectangleRoundFilled = &RectangleRoundFilled;
    impl->Polygon = &Polygon;
    impl->PolygonFilled = &PolygonFilled;
}
    

GPU_Renderer* create_dummy_renderer(GPU_RendererID request)
{
    GPU_Renderer* renderer = (GPU_Renderer*)malloc(sizeof(GPU_Renderer));
    if(renderer == NULL)
        return NULL;

    memset(renderer, 0, sizeof(GPU_Renderer));

    renderer->id = request;
    renderer->shader_language = GPU_LANGUAGE_NONE;
    renderer->min_shader_version = 0;
    renderer->max_shader_version = 0;
    
    renderer->current_context_target = NULL;
    
    renderer->impl = (GPU_RendererImpl*)malloc(sizeof(GPU_RendererImpl));
    memset(renderer->impl, 0, sizeof(GPU_RendererImpl));
    set_renderer_functions(renderer->impl);

    return renderer;
}

void free_dummy_renderer(GPU_Renderer* renderer)
{
    if(renderer == NULL)
        return;
    
    free(renderer);
}

int main(int argc, char* argv[])
{
	GPU_Target* screen;
	
	// Prepare renderer for SDL_gpu to use
	GPU_RendererID rendererID = GPU_MakeRendererID("Dummy", GPU_ReserveNextRendererEnum(), 1, 0);
	GPU_RegisterRenderer(rendererID, &create_dummy_renderer, &free_dummy_renderer);

	printRenderers();
	
	// Request this specific renderer
	screen = GPU_InitRenderer(rendererID.renderer, 800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	{
		Uint8 done;
		SDL_Event event;
		
		GPU_Image* image = GPU_LoadImage("data/test.bmp");
		if(image == NULL)
            GPU_Log("Failed to load image.\n");
        
        done = 0;
        while(!done)
        {
            while(SDL_PollEvent(&event))
            {
                if(event.type == SDL_QUIT)
                    done = 1;
                else if(event.type == SDL_KEYDOWN)
                {
                    if(event.key.keysym.sym == SDLK_ESCAPE)
                        done = 1;
                }
            }
            
            GPU_Clear(screen);
        
            GPU_Blit(image, NULL, screen, screen->w/2, screen->h/2);
            
            GPU_Flip(screen);
            
            // Long delay to keep the logging from piling up too much
            SDL_Delay(500);
        }
        
        GPU_FreeImage(image);
	}
	
	GPU_Quit();
	
	return 0;
}

#endif
