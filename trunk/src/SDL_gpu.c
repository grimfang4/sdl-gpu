#include "SDL_gpu.h"
#include "SDL_gpu_RendererImpl.h"
#include "SDL_platform.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef _MSC_VER
	#define __func__ __FUNCTION__
	#pragma warning(push)
	// Visual Studio wants to complain about while(0)
	#pragma warning(disable: 4127)
#endif

#include "stb_image.h"

#ifdef SDL_GPU_USE_SDL2
    #define GET_ALPHA(sdl_color) ((sdl_color).a)
#else
    #define GET_ALPHA(sdl_color) ((sdl_color).unused)
#endif

#define CHECK_RENDERER (_gpu_current_renderer != NULL)
#define MAKE_CURRENT_IF_NONE(target) do{ if(_gpu_current_renderer->current_context_target == NULL && target != NULL && target->context != NULL) GPU_MakeCurrent(target, target->context->windowID); } while(0)
#define CHECK_CONTEXT (_gpu_current_renderer->current_context_target != NULL)
#define RETURN_ERROR(code, details) do{ GPU_PushErrorCode(__func__, code, "%s", details); return; } while(0)

int gpu_strcasecmp(const char* s1, const char* s2);

void gpu_init_renderer_register(void);
GPU_Renderer* gpu_create_and_add_renderer(GPU_RendererID id);

static GPU_Renderer* _gpu_current_renderer = NULL;

static GPU_DebugLevelEnum _gpu_debug_level = GPU_DEBUG_LEVEL_0;

#define GPU_DEFAULT_MAX_NUM_ERRORS 20
#define GPU_ERROR_FUNCTION_STRING_MAX 128
#define GPU_ERROR_DETAILS_STRING_MAX 512
static GPU_ErrorObject* _gpu_error_code_stack = NULL;
static unsigned int _gpu_num_error_codes = 0;
static unsigned int _gpu_error_code_stack_size = GPU_DEFAULT_MAX_NUM_ERRORS;

/*! A mapping of windowID to a GPU_Target to facilitate GPU_GetWindowTarget(). */
typedef struct GPU_WindowMapping
{
    Uint32 windowID;
    GPU_Target* target;
} GPU_WindowMapping;

#define GPU_INITIAL_WINDOW_MAPPINGS_SIZE 10
static GPU_WindowMapping* _gpu_window_mappings = NULL;
static int _gpu_window_mappings_size = 0;
static int _gpu_num_window_mappings = 0;


SDL_version GPU_GetLinkedVersion(void)
{
    return GPU_GetCompiledVersion();
}

void GPU_SetCurrentRenderer(GPU_RendererID id)
{
	_gpu_current_renderer = GPU_GetRenderer(id);
	
	if(_gpu_current_renderer != NULL)
		_gpu_current_renderer->impl->SetAsCurrent(_gpu_current_renderer);
}

void GPU_ResetRendererState(void)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->ResetRendererState(_gpu_current_renderer);
}

GPU_Renderer* GPU_GetCurrentRenderer(void)
{
	return _gpu_current_renderer;
}

Uint32 GPU_GetCurrentShaderProgram(void)
{
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return 0;
    
    return _gpu_current_renderer->current_context_target->context->current_shader_program;
}


int gpu_default_print(GPU_LogLevelEnum log_level, const char* format, va_list args)
{
    switch(log_level)
    {
	#ifdef __ANDROID__
	case GPU_LOG_INFO:
		return __android_log_vprint((GPU_GetDebugLevel() >= GPU_DEBUG_LEVEL_3? ANDROID_LOG_ERROR : ANDROID_LOG_INFO), "APPLICATION", format, args);
	case GPU_LOG_WARNING:
		return __android_log_vprint((GPU_GetDebugLevel() >= GPU_DEBUG_LEVEL_2? ANDROID_LOG_ERROR : ANDROID_LOG_WARN), "APPLICATION", format, args);
	case GPU_LOG_ERROR:
		return __android_log_vprint(ANDROID_LOG_ERROR, "APPLICATION", format, args);
	#else
	case GPU_LOG_INFO:
		return vfprintf((GPU_GetDebugLevel() >= GPU_DEBUG_LEVEL_3? stderr : stdout), format, args);
	case GPU_LOG_WARNING:
		return vfprintf((GPU_GetDebugLevel() >= GPU_DEBUG_LEVEL_2? stderr : stdout), format, args);
	case GPU_LOG_ERROR:
		return vfprintf(stderr, format, args);
	#endif
    default:
        return 0;
    }
}

static int (*_gpu_print)(GPU_LogLevelEnum log_level, const char* format, va_list args) = &gpu_default_print;

void GPU_SetLogCallback(int (*callback)(GPU_LogLevelEnum log_level, const char* format, va_list args))
{
    if(callback == NULL)
        _gpu_print = &gpu_default_print;
    else
        _gpu_print = callback;
}

void GPU_LogInfo(const char* format, ...)
{
	va_list args;
	va_start(args, format);
    _gpu_print(GPU_LOG_INFO, format, args);
	va_end(args);
}

void GPU_LogWarning(const char* format, ...)
{
	va_list args;
	va_start(args, format);
    _gpu_print(GPU_LOG_WARNING, format, args);
	va_end(args);
}

void GPU_LogError(const char* format, ...)
{
	va_list args;
	va_start(args, format);
    _gpu_print(GPU_LOG_ERROR, format, args);
	va_end(args);
}


static Uint8 gpu_init_SDL(void)
{
	if(GPU_GetNumActiveRenderers() == 0)
	{
	    Uint32 subsystems = SDL_WasInit(SDL_INIT_EVERYTHING);
	    if(!subsystems)
        {
            // Nothing has been set up, so init SDL and the video subsystem.
            if(SDL_Init(SDL_INIT_VIDEO) < 0)
            {
                GPU_PushErrorCode("GPU_Init", GPU_ERROR_BACKEND_ERROR, "Failed to initialize SDL video subsystem");
                return 0;
            }
        }
        else if(!(subsystems & SDL_INIT_VIDEO))
        {
            // Something already set up SDL, so just init video.
            if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
            {
                GPU_PushErrorCode("GPU_Init", GPU_ERROR_BACKEND_ERROR, "Failed to initialize SDL video subsystem");
                return 0;
            }
        }
	}
	return 1;
}

static Uint32 _gpu_init_windowID = 0;

void GPU_SetInitWindow(Uint32 windowID)
{
    _gpu_init_windowID = windowID;
}

Uint32 GPU_GetInitWindow(void)
{
    return _gpu_init_windowID;
}

static GPU_InitFlagEnum _gpu_preinit_flags = GPU_DEFAULT_INIT_FLAGS;
static GPU_InitFlagEnum _gpu_required_features = 0;

void GPU_SetPreInitFlags(GPU_InitFlagEnum GPU_flags)
{
    _gpu_preinit_flags = GPU_flags;
}

GPU_InitFlagEnum GPU_GetPreInitFlags(void)
{
    return _gpu_preinit_flags;
}

void GPU_SetRequiredFeatures(GPU_FeatureEnum features)
{
    _gpu_required_features = features;
}

GPU_FeatureEnum GPU_GetRequiredFeatures(void)
{
    return _gpu_required_features;
}

static void gpu_init_error_stack(void)
{
    if(_gpu_error_code_stack == NULL)
    {
        unsigned int i;
        _gpu_error_code_stack = (GPU_ErrorObject*)malloc(sizeof(GPU_ErrorObject)*_gpu_error_code_stack_size);
        
        for(i = 0; i < _gpu_error_code_stack_size; i++)
        {
            _gpu_error_code_stack[i].function = (char*)malloc(GPU_ERROR_FUNCTION_STRING_MAX+1);
            _gpu_error_code_stack[i].details = (char*)malloc(GPU_ERROR_DETAILS_STRING_MAX+1);
        }
        _gpu_num_error_codes = 0;
    }
}

static void gpu_init_window_mappings(void)
{
    if(_gpu_window_mappings == NULL)
    {
        _gpu_window_mappings_size = GPU_INITIAL_WINDOW_MAPPINGS_SIZE;
        _gpu_window_mappings = (GPU_WindowMapping*)malloc(_gpu_window_mappings_size * sizeof(GPU_WindowMapping));
        _gpu_num_window_mappings = 0;
    }
}

void GPU_AddWindowMapping(GPU_Target* target)
{
	Uint32 windowID;
	int i;

	if(_gpu_window_mappings == NULL)
        gpu_init_window_mappings();
    
    if(target == NULL || target->context == NULL)
        return;
    
    windowID = target->context->windowID;
    if(windowID == 0)  // Invalid window ID
        return;
    
    // Check for duplicates
    for(i = 0; i < _gpu_num_window_mappings; i++)
    {
        if(_gpu_window_mappings[i].windowID == windowID)
        {
            if(_gpu_window_mappings[i].target != target)
                GPU_PushErrorCode(__func__, GPU_ERROR_DATA_ERROR, "WindowID %u already has a mapping.", windowID);
            return;
        }
        // Don't check the target because it's okay for a single target to be used with multiple windows
    }
    
    // Check if list is big enough to hold another
    if(_gpu_num_window_mappings >= _gpu_window_mappings_size)
    {
		GPU_WindowMapping* new_array;
        _gpu_window_mappings_size *= 2;
        new_array = (GPU_WindowMapping*)malloc(_gpu_window_mappings_size * sizeof(GPU_WindowMapping));
        memcpy(new_array, _gpu_window_mappings, _gpu_num_window_mappings * sizeof(GPU_WindowMapping));
        free(_gpu_window_mappings);
        _gpu_window_mappings = new_array;
    }
    
    // Add to end of list
	{
		GPU_WindowMapping m;
		m.windowID = windowID;
		m.target = target;
		_gpu_window_mappings[_gpu_num_window_mappings] = m;
	}
    _gpu_num_window_mappings++;
}

void GPU_RemoveWindowMapping(Uint32 windowID)
{
	int i;

    if(_gpu_window_mappings == NULL)
        gpu_init_window_mappings();
    
    if(windowID == 0)  // Invalid window ID
        return;
    
    // Find the occurrence
    for(i = 0; i < _gpu_num_window_mappings; i++)
    {
        if(_gpu_window_mappings[i].windowID == windowID)
        {
			int num_to_move;

            // Unset the target's window
            _gpu_window_mappings[i].target->context->windowID = 0;
            
            // Move the remaining entries to replace the removed one
            _gpu_num_window_mappings--;
            num_to_move = _gpu_num_window_mappings - i;
            if(num_to_move > 0)
                memmove(&_gpu_window_mappings[i], &_gpu_window_mappings[i+1], num_to_move * sizeof(GPU_WindowMapping));
            return;
        }
    }
    
}

void GPU_RemoveWindowMappingByTarget(GPU_Target* target)
{
	Uint32 windowID;
	int i;

    if(_gpu_window_mappings == NULL)
        gpu_init_window_mappings();
    
    if(target == NULL || target->context == NULL)
        return;
    
    windowID = target->context->windowID;
    if(windowID == 0)  // Invalid window ID
        return;
    
    // Unset the target's window
    target->context->windowID = 0;
    
    // Find the occurrences
    for(i = 0; i < _gpu_num_window_mappings; )
    {
        if(_gpu_window_mappings[i].target == target)
        {
            // Move the remaining entries to replace the removed one
			int num_to_move;
            _gpu_num_window_mappings--;
            num_to_move = _gpu_num_window_mappings - i;
            if(num_to_move > 0)
                memmove(&_gpu_window_mappings[i], &_gpu_window_mappings[i+1], num_to_move * sizeof(GPU_WindowMapping));
            return;
        }
        else
            i++;
    }
    
}

GPU_Target* GPU_GetWindowTarget(Uint32 windowID)
{
    int i;

    if(_gpu_window_mappings == NULL)
        gpu_init_window_mappings();
    
    if(windowID == 0)  // Invalid window ID
        return NULL;
    
    // Find the occurrence
    for(i = 0; i < _gpu_num_window_mappings; i++)
    {
        if(_gpu_window_mappings[i].windowID == windowID)
            return _gpu_window_mappings[i].target;
    }
    
    return NULL;
}

GPU_Target* GPU_Init(Uint16 w, Uint16 h, GPU_WindowFlagEnum SDL_flags)
{
	int renderer_order_size;
	int i;
    GPU_RendererID renderer_order[GPU_RENDERER_ORDER_MAX];

    gpu_init_error_stack();
    
	gpu_init_renderer_register();
	
	if(!gpu_init_SDL())
        return NULL;
        
    renderer_order_size = 0;
    GPU_GetRendererOrder(&renderer_order_size, renderer_order);
	
    // Init the renderers in order
    for(i = 0; i < renderer_order_size; i++)
    {
        GPU_Target* screen = GPU_InitRendererByID(renderer_order[i], w, h, SDL_flags);
        if(screen != NULL)
            return screen;
    }
    
    return NULL;
}

GPU_Target* GPU_InitRenderer(GPU_RendererEnum renderer_enum, Uint16 w, Uint16 h, GPU_WindowFlagEnum SDL_flags)
{
    // Search registry for this renderer and use that id
    return GPU_InitRendererByID(GPU_GetRendererID(renderer_enum), w, h, SDL_flags);
}

GPU_Target* GPU_InitRendererByID(GPU_RendererID renderer_request, Uint16 w, Uint16 h, GPU_WindowFlagEnum SDL_flags)
{
	GPU_Renderer* renderer;
	GPU_Target* screen;

    gpu_init_error_stack();
	gpu_init_renderer_register();
	
	if(!gpu_init_SDL())
        return NULL;
	
	renderer = gpu_create_and_add_renderer(renderer_request);
	if(renderer == NULL)
		return NULL;
    
	GPU_SetCurrentRenderer(renderer->id);
	
	screen = renderer->impl->Init(renderer, renderer_request, w, h, SDL_flags);
	if(screen == NULL)
    {
        // Init failed, destroy the renderer...
        // Erase the window mappings
        _gpu_num_window_mappings = 0;
        GPU_CloseCurrentRenderer();
    }
    else
        GPU_SetInitWindow(0);
    return screen;
}

Uint8 GPU_IsFeatureEnabled(GPU_FeatureEnum feature)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return 0;
    
	return ((_gpu_current_renderer->enabled_features & feature) == feature);
}

GPU_Target* GPU_CreateTargetFromWindow(Uint32 windowID)
{
	if(_gpu_current_renderer == NULL)
		return NULL;
	
	return _gpu_current_renderer->impl->CreateTargetFromWindow(_gpu_current_renderer, windowID, NULL);
}

GPU_Target* GPU_CreateAliasTarget(GPU_Target* target)
{
    if(!CHECK_RENDERER)
        return NULL;
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        return NULL;
	
	return _gpu_current_renderer->impl->CreateAliasTarget(_gpu_current_renderer, target);
}

void GPU_MakeCurrent(GPU_Target* target, Uint32 windowID)
{
	if(_gpu_current_renderer == NULL)
		return;
	
	_gpu_current_renderer->impl->MakeCurrent(_gpu_current_renderer, target, windowID);
}

Uint8 GPU_SetFullscreen(Uint8 enable_fullscreen, Uint8 use_desktop_resolution)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return 0;
	
	return _gpu_current_renderer->impl->SetFullscreen(_gpu_current_renderer, enable_fullscreen, use_desktop_resolution);
}

Uint8 GPU_GetFullscreen(void)
{
#ifdef SDL_GPU_USE_SDL2
    GPU_Target* target = GPU_GetContextTarget();
    if(target == NULL)
        return 0;
    return (Uint8)(SDL_GetWindowFlags(SDL_GetWindowFromID(target->context->windowID))
             & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP));
#else
    SDL_Surface* surf = SDL_GetVideoSurface();
    if(surf == NULL)
        return 0;
    return (surf->flags & SDL_FULLSCREEN);
#endif
}

Uint8 GPU_SetWindowResolution(Uint16 w, Uint16 h)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL || w == 0 || h == 0)
		return 0;
	
	return _gpu_current_renderer->impl->SetWindowResolution(_gpu_current_renderer, w, h);
}


void GPU_SetVirtualResolution(GPU_Target* target, Uint16 w, Uint16 h)
{
    if(!CHECK_RENDERER)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL renderer");
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL context");
	if(w == 0 || h == 0)
		return;
	
	_gpu_current_renderer->impl->SetVirtualResolution(_gpu_current_renderer, target, w, h);
}

void GPU_UnsetVirtualResolution(GPU_Target* target)
{
    if(!CHECK_RENDERER)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL renderer");
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL context");
	
	_gpu_current_renderer->impl->UnsetVirtualResolution(_gpu_current_renderer, target);
}

void GPU_SetImageVirtualResolution(GPU_Image* image, Uint16 w, Uint16 h)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL || w == 0 || h == 0)
		return;
	
	if(image == NULL)
        return;
    
    image->w = w;
    image->h = h;
    image->using_virtual_resolution = 1;
}

void GPU_UnsetImageVirtualResolution(GPU_Image* image)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	if(image == NULL)
        return;
	
    image->w = image->base_w;
    image->h = image->base_h;
    image->using_virtual_resolution = 0;
}

// Deletes all existing errors
void GPU_SetErrorStackMax(unsigned int max)
{
    unsigned int i;
    // Free the error stack
    for(i = 0; i < _gpu_error_code_stack_size; i++)
    {
        free(_gpu_error_code_stack[i].function);
        _gpu_error_code_stack[i].function = NULL;
        free(_gpu_error_code_stack[i].details);
        _gpu_error_code_stack[i].details = NULL;
    }
    free(_gpu_error_code_stack);
    _gpu_error_code_stack = NULL;
    _gpu_num_error_codes = 0;
    
    // Reallocate with new size
    _gpu_error_code_stack_size = max;
    gpu_init_error_stack();
}

void GPU_CloseCurrentRenderer(void)
{
	if(_gpu_current_renderer == NULL)
		return;
	
	_gpu_current_renderer->impl->Quit(_gpu_current_renderer);
	GPU_FreeRenderer(_gpu_current_renderer);
}

void GPU_Quit(void)
{
    unsigned int i;
    if(_gpu_num_error_codes > 0 && GPU_GetDebugLevel() >= GPU_DEBUG_LEVEL_1)
        GPU_LogError("GPU_Quit: %d uncleared error%s.\n", _gpu_num_error_codes, (_gpu_num_error_codes > 1? "s" : ""));
        
    // Free the error stack
    for(i = 0; i < _gpu_error_code_stack_size; i++)
    {
        free(_gpu_error_code_stack[i].function);
        _gpu_error_code_stack[i].function = NULL;
        free(_gpu_error_code_stack[i].details);
        _gpu_error_code_stack[i].details = NULL;
    }
    free(_gpu_error_code_stack);
    _gpu_error_code_stack = NULL;
    _gpu_num_error_codes = 0;
    
	if(_gpu_current_renderer == NULL)
		return;
	
	_gpu_current_renderer->impl->Quit(_gpu_current_renderer);
	GPU_FreeRenderer(_gpu_current_renderer);
	// FIXME: Free all renderers
	
	if(GPU_GetNumActiveRenderers() == 0)
		SDL_Quit();
}

void GPU_SetDebugLevel(GPU_DebugLevelEnum level)
{
    if(level > GPU_DEBUG_LEVEL_MAX)
        level = GPU_DEBUG_LEVEL_MAX;
    _gpu_debug_level = level;
}

GPU_DebugLevelEnum GPU_GetDebugLevel(void)
{
    return _gpu_debug_level;
}

void GPU_PushErrorCode(const char* function, GPU_ErrorEnum error, const char* details, ...)
{
    if(GPU_GetDebugLevel() >= GPU_DEBUG_LEVEL_1)
    {
        // Print the message
        if(details != NULL)
        {
            char buf[GPU_ERROR_DETAILS_STRING_MAX];
            va_list lst;
            va_start(lst, details);
            vsnprintf(buf, GPU_ERROR_DETAILS_STRING_MAX, details, lst);
            va_end(lst);
            
            GPU_LogError("%s: %s - %s\n", (function == NULL? "NULL" : function), GPU_GetErrorString(error), buf);
        }
        else
            GPU_LogError("%s: %s\n", (function == NULL? "NULL" : function), GPU_GetErrorString(error));
    }
    
    if(_gpu_num_error_codes < _gpu_error_code_stack_size)
    {
        if(function == NULL)
            _gpu_error_code_stack[_gpu_num_error_codes].function[0] = '\0';
        else
        {
            strncpy(_gpu_error_code_stack[_gpu_num_error_codes].function, function, GPU_ERROR_FUNCTION_STRING_MAX);
            _gpu_error_code_stack[_gpu_num_error_codes].function[GPU_ERROR_FUNCTION_STRING_MAX] = '\0';
        }
        _gpu_error_code_stack[_gpu_num_error_codes].error = error;
        if(details == NULL)
            _gpu_error_code_stack[_gpu_num_error_codes].details[0] = '\0';
        else
        {
            va_list lst;
            va_start(lst, details);
            vsnprintf(_gpu_error_code_stack[_gpu_num_error_codes].details, GPU_ERROR_DETAILS_STRING_MAX, details, lst);
            va_end(lst);
        }
        _gpu_num_error_codes++;
    }
}

GPU_ErrorObject GPU_PopErrorCode(void)
{
    if(_gpu_num_error_codes <= 0)
    {
        GPU_ErrorObject e = {NULL, GPU_ERROR_NONE, NULL};
        return e;
    }
    
    return _gpu_error_code_stack[--_gpu_num_error_codes];
}

const char* GPU_GetErrorString(GPU_ErrorEnum error)
{
    switch(error)
    {
        case GPU_ERROR_NONE:
            return "NO ERROR";
        case GPU_ERROR_BACKEND_ERROR:
            return "BACKEND ERROR";
        case GPU_ERROR_DATA_ERROR:
            return "DATA ERROR";
        case GPU_ERROR_USER_ERROR:
            return "USER ERROR";
        case GPU_ERROR_UNSUPPORTED_FUNCTION:
            return "UNSUPPORTED FUNCTION";
        case GPU_ERROR_NULL_ARGUMENT:
            return "NULL ARGUMENT";
        case GPU_ERROR_FILE_NOT_FOUND:
            return "FILE NOT FOUND";
    }
    return "UNKNOWN ERROR";
}


void GPU_GetVirtualCoords(GPU_Target* target, float* x, float* y, float displayX, float displayY)
{
	if(target == NULL)
		return;
	
	if(target->context != NULL)
    {
        if(x != NULL)
            *x = (displayX*target->w)/target->context->window_w;
        if(y != NULL)
            *y = (displayY*target->h)/target->context->window_h;
    }
	else if(target->image != NULL)
    {
        if(x != NULL)
            *x = (displayX*target->w)/target->image->w;
        if(y != NULL)
            *y = (displayY*target->h)/target->image->h;
    }
    else
    {
        if(x != NULL)
            *x = displayX;
        if(y != NULL)
            *y = displayY;
    }
}

GPU_Rect GPU_MakeRect(float x, float y, float w, float h)
{
	GPU_Rect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;

    return r;
}

SDL_Color GPU_MakeColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	GET_ALPHA(c) = a;

    return c;
}

GPU_RendererID GPU_MakeRendererID(const char* name, GPU_RendererEnum renderer, int major_version, int minor_version)
{
	GPU_RendererID r;
	r.name = name;
	r.renderer = renderer;
	r.major_version = major_version;
	r.minor_version = minor_version;
	
    return r;
}

void GPU_SetViewport(GPU_Target* target, GPU_Rect viewport)
{
    if(target != NULL)
        target->viewport = viewport;
}

GPU_Camera GPU_GetDefaultCamera(void)
{
	GPU_Camera cam = {0.0f, 0.0f, -10.0f, 0.0f, 1.0f};
	return cam;
}

GPU_Camera GPU_GetCamera(GPU_Target* target)
{
	if(target == NULL)
		return GPU_GetDefaultCamera();
	return target->camera;
}

GPU_Camera GPU_SetCamera(GPU_Target* target, GPU_Camera* cam)
{
	if(_gpu_current_renderer == NULL)
		return GPU_GetDefaultCamera();
    MAKE_CURRENT_IF_NONE(target);
	if(_gpu_current_renderer->current_context_target == NULL)
		return GPU_GetDefaultCamera();
	
	return _gpu_current_renderer->impl->SetCamera(_gpu_current_renderer, target, cam);
}

GPU_Image* GPU_CreateImage(Uint16 w, Uint16 h, GPU_FormatEnum format)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return NULL;
	
	return _gpu_current_renderer->impl->CreateImage(_gpu_current_renderer, w, h, format);
}

GPU_Image* GPU_CreateImageUsingTexture(Uint32 handle, Uint8 take_ownership)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return NULL;
	
	return _gpu_current_renderer->impl->CreateImageUsingTexture(_gpu_current_renderer, handle, take_ownership);
}

GPU_Image* GPU_LoadImage(const char* filename)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return NULL;
	
	return _gpu_current_renderer->impl->LoadImage(_gpu_current_renderer, filename);
}

GPU_Image* GPU_CreateAliasImage(GPU_Image* image)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return NULL;
	
	return _gpu_current_renderer->impl->CreateAliasImage(_gpu_current_renderer, image);
}

Uint8 GPU_SaveImage(GPU_Image* image, const char* filename, GPU_FileFormatEnum format)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return 0;
	
	return _gpu_current_renderer->impl->SaveImage(_gpu_current_renderer, image, filename, format);
}

GPU_Image* GPU_CopyImage(GPU_Image* image)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return NULL;
	
	return _gpu_current_renderer->impl->CopyImage(_gpu_current_renderer, image);
}

void GPU_UpdateImage(GPU_Image* image, const GPU_Rect* image_rect, SDL_Surface* surface, const GPU_Rect* surface_rect)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->UpdateImage(_gpu_current_renderer, image, image_rect, surface, surface_rect);
}

void GPU_UpdateImageBytes(GPU_Image* image, const GPU_Rect* image_rect, const unsigned char* bytes, int bytes_per_row)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->UpdateImageBytes(_gpu_current_renderer, image, image_rect, bytes, bytes_per_row);
}

SDL_Surface* GPU_LoadSurface(const char* filename)
{
	int width, height, channels;
	Uint32 Rmask, Gmask, Bmask, Amask = 0;
	unsigned char* data;
	SDL_Surface* result;
	
	if(filename == NULL)
    {
        GPU_PushErrorCode("GPU_LoadSurface", GPU_ERROR_NULL_ARGUMENT, "filename");
        return NULL;
    }
	
	#ifdef __ANDROID__
	if(strlen(filename) > 0 && filename[0] != '/')
	{
        // Must use SDL_RWops to access the assets directory automatically
        SDL_RWops* rwops = SDL_RWFromFile(filename, "r");
        if(rwops == NULL)
            return NULL;
        int data_bytes = SDL_RWseek(rwops, 0, SEEK_END);
        SDL_RWseek(rwops, 0, SEEK_SET);
        unsigned char* c_data = (unsigned char*)malloc(data_bytes);
        SDL_RWread(rwops, c_data, 1, data_bytes);
        data = stbi_load_from_memory(c_data, data_bytes, &width, &height, &channels, 0);
        free(c_data);
        SDL_FreeRW(rwops);
	}
	else
    {
        // Absolute filename
        data = stbi_load(filename, &width, &height, &channels, 0);
    }
	#else
	data = stbi_load(filename, &width, &height, &channels, 0);
	#endif
	
	if(data == NULL)
	{
		GPU_PushErrorCode(__func__, GPU_ERROR_DATA_ERROR, "Failed to load \"%s\": %s", filename, stbi_failure_reason());
		return NULL;
	}
	if(channels < 1 || channels > 4)
	{
		GPU_PushErrorCode(__func__, GPU_ERROR_DATA_ERROR, "Failed to load \"%s\": Unsupported pixel format", filename);
		stbi_image_free(data);
		return NULL;
	}
	
	switch(channels)
	{
        case 1:
            Rmask = Gmask = Bmask = 0;  // Use default RGB masks for 8-bit
            break;
        case 2:
            Rmask = Gmask = Bmask = 0;  // Use default RGB masks for 16-bit
            break;
        case 3:
            // These are reversed from what SDL_image uses...  That is bad. :(  Needs testing.
            #if SDL_BYTEORDER == SDL_BIG_ENDIAN
            Rmask = 0xff0000;
            Gmask = 0x00ff00;
            Bmask = 0x0000ff;
            #else
            Rmask = 0x0000ff;
            Gmask = 0x00ff00;
            Bmask = 0xff0000;
            #endif
            break;
        case 4:
            Rmask = 0x000000ff;
            Gmask = 0x0000ff00;
            Bmask = 0x00ff0000;
            Amask = 0xff000000;
            break;
	}
	
	result = SDL_CreateRGBSurfaceFrom(data, width, height, channels*8, width*channels, Rmask, Gmask, Bmask, Amask);
	if(result != NULL)
        result->flags &= ~SDL_PREALLOC;  // Make SDL take ownership of the data memory
    
	if(result != NULL && result->format->palette != NULL)
    {
        // SDL_CreateRGBSurface has no idea what palette to use, so it uses a blank one.
        // We'll at least create a grayscale one, but it's not ideal...
        // Better would be to get the palette from stbi, but stbi doesn't do that!
        SDL_Color colors[256];
        int i;
        
        for(i = 0; i < 256; i++)
        {
            colors[i].r = colors[i].g = colors[i].b = (Uint8)i;
        }

        /* Set palette */
        #ifdef SDL_GPU_USE_SDL2
        SDL_SetPaletteColors(result->format->palette, colors, 0, 256);
        #else
        SDL_SetPalette(result, SDL_LOGPAL, colors, 0, 256);
        #endif
    }
	
	return result;
}

#include "stb_image.h"
#include "stb_image_write.h"

// From http://stackoverflow.com/questions/5309471/getting-file-extension-in-c
static const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename)
        return "";
    return dot + 1;
}

Uint8 GPU_SaveSurface(SDL_Surface* surface, const char* filename, GPU_FileFormatEnum format)
{
    Uint8 result;
    unsigned char* data;

    if(surface == NULL || filename == NULL ||
            surface->w < 1 || surface->h < 1)
    {
        return 0;
    }


    data = surface->pixels;
    
    if(format == GPU_FILE_AUTO)
    {
        const char* extension = get_filename_ext(filename);
        if(gpu_strcasecmp(extension, "png") == 0)
            format = GPU_FILE_PNG;
        else if(gpu_strcasecmp(extension, "bmp") == 0)
            format = GPU_FILE_BMP;
        else if(gpu_strcasecmp(extension, "tga") == 0)
            format = GPU_FILE_TGA;
        else
        {
            GPU_PushErrorCode(__func__, GPU_ERROR_DATA_ERROR, "Could not detect output file format from file name");
            return 0;
        }
    }
    
    switch(format)
    {
        case GPU_FILE_PNG:
            result = (stbi_write_png(filename, surface->w, surface->h, surface->format->BytesPerPixel, (const unsigned char *const)data, 0) > 0);
            break;
        case GPU_FILE_BMP:
            result = (stbi_write_bmp(filename, surface->w, surface->h, surface->format->BytesPerPixel, (void*)data) > 0);
            break;
        case GPU_FILE_TGA:
            result = (stbi_write_tga(filename, surface->w, surface->h, surface->format->BytesPerPixel, (void*)data) > 0);
            break;
        default:
            GPU_PushErrorCode(__func__, GPU_ERROR_DATA_ERROR, "Unsupported output file format");
            result = 0;
            break;
    }

    return result;
}

GPU_Image* GPU_CopyImageFromSurface(SDL_Surface* surface)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return NULL;
	
	return _gpu_current_renderer->impl->CopyImageFromSurface(_gpu_current_renderer, surface);
}

GPU_Image* GPU_CopyImageFromTarget(GPU_Target* target)
{
	if(_gpu_current_renderer == NULL)
		return NULL;
    MAKE_CURRENT_IF_NONE(target);
	if(_gpu_current_renderer->current_context_target == NULL)
		return NULL;
	
	return _gpu_current_renderer->impl->CopyImageFromTarget(_gpu_current_renderer, target);
}

SDL_Surface* GPU_CopySurfaceFromTarget(GPU_Target* target)
{
	if(_gpu_current_renderer == NULL)
		return NULL;
    MAKE_CURRENT_IF_NONE(target);
	if(_gpu_current_renderer->current_context_target == NULL)
		return NULL;
	
	return _gpu_current_renderer->impl->CopySurfaceFromTarget(_gpu_current_renderer, target);
}

SDL_Surface* GPU_CopySurfaceFromImage(GPU_Image* image)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return NULL;
	
	return _gpu_current_renderer->impl->CopySurfaceFromImage(_gpu_current_renderer, image);
}

void GPU_FreeImage(GPU_Image* image)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->FreeImage(_gpu_current_renderer, image);
}


GPU_Target* GPU_GetContextTarget(void)
{
	if(_gpu_current_renderer == NULL)
		return NULL;
	
	return _gpu_current_renderer->current_context_target;
}


GPU_Target* GPU_LoadTarget(GPU_Image* image)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return NULL;
	
	return _gpu_current_renderer->impl->LoadTarget(_gpu_current_renderer, image);
}



void GPU_FreeTarget(GPU_Target* target)
{
	if(_gpu_current_renderer == NULL)
		return;
	
	_gpu_current_renderer->impl->FreeTarget(_gpu_current_renderer, target);
}



void GPU_Blit(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y)
{
    if(!CHECK_RENDERER)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL renderer");
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL context");
    
	if(image == NULL)
        RETURN_ERROR(GPU_ERROR_NULL_ARGUMENT, "image");
	if(target == NULL)
        RETURN_ERROR(GPU_ERROR_NULL_ARGUMENT, "target");
	
	_gpu_current_renderer->impl->Blit(_gpu_current_renderer, image, src_rect, target, x, y);
}


void GPU_BlitRotate(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float angle)
{
    if(!CHECK_RENDERER)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL renderer");
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL context");
    
	if(image == NULL)
        RETURN_ERROR(GPU_ERROR_NULL_ARGUMENT, "image");
	if(target == NULL)
        RETURN_ERROR(GPU_ERROR_NULL_ARGUMENT, "target");
	
	_gpu_current_renderer->impl->BlitRotate(_gpu_current_renderer, image, src_rect, target, x, y, angle);
}

void GPU_BlitScale(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float scaleX, float scaleY)
{
    if(!CHECK_RENDERER)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL renderer");
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL context");
    
	if(image == NULL)
        RETURN_ERROR(GPU_ERROR_NULL_ARGUMENT, "image");
	if(target == NULL)
        RETURN_ERROR(GPU_ERROR_NULL_ARGUMENT, "target");
	
	_gpu_current_renderer->impl->BlitScale(_gpu_current_renderer, image, src_rect, target, x, y, scaleX, scaleY);
}

void GPU_BlitTransform(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float angle, float scaleX, float scaleY)
{
    if(!CHECK_RENDERER)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL renderer");
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL context");
    
	if(image == NULL)
        RETURN_ERROR(GPU_ERROR_NULL_ARGUMENT, "image");
	if(target == NULL)
        RETURN_ERROR(GPU_ERROR_NULL_ARGUMENT, "target");
	
	_gpu_current_renderer->impl->BlitTransform(_gpu_current_renderer, image, src_rect, target, x, y, angle, scaleX, scaleY);
}

void GPU_BlitTransformX(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float pivot_x, float pivot_y, float angle, float scaleX, float scaleY)
{
    if(!CHECK_RENDERER)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL renderer");
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL context");
    
	if(image == NULL)
        RETURN_ERROR(GPU_ERROR_NULL_ARGUMENT, "image");
	if(target == NULL)
        RETURN_ERROR(GPU_ERROR_NULL_ARGUMENT, "target");
	
	_gpu_current_renderer->impl->BlitTransformX(_gpu_current_renderer, image, src_rect, target, x, y, pivot_x, pivot_y, angle, scaleX, scaleY);
}

void GPU_BlitTransformMatrix(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float* matrix3x3)
{
    if(!CHECK_RENDERER)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL renderer");
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL context");
    
	if(image == NULL)
        RETURN_ERROR(GPU_ERROR_NULL_ARGUMENT, "image");
	if(target == NULL)
        RETURN_ERROR(GPU_ERROR_NULL_ARGUMENT, "target");
    
    if(matrix3x3 == NULL)
		return;
	
	_gpu_current_renderer->impl->BlitTransformMatrix(_gpu_current_renderer, image, src_rect, target, x, y, matrix3x3);
}

void GPU_TriangleBatch(GPU_Image* image, GPU_Target* target, unsigned short num_vertices, float* values, unsigned int num_indices, unsigned short* indices, GPU_BatchFlagEnum flags)
{
    if(!CHECK_RENDERER)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL renderer");
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL context");
    
	if(target == NULL)
        RETURN_ERROR(GPU_ERROR_NULL_ARGUMENT, "target");
    
    if(num_vertices == 0)
        return;
    
    
    _gpu_current_renderer->impl->TriangleBatch(_gpu_current_renderer, image, target, num_vertices, values, num_indices, indices, flags);
}




void GPU_GenerateMipmaps(GPU_Image* image)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->GenerateMipmaps(_gpu_current_renderer, image);
}




GPU_Rect GPU_SetClipRect(GPU_Target* target, GPU_Rect rect)
{
	if(target == NULL || _gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
	{
		GPU_Rect r = {0,0,0,0};
		return r;
	}
	
	return _gpu_current_renderer->impl->SetClip(_gpu_current_renderer, target, (Sint16)rect.x, (Sint16)rect.y, (Uint16)rect.w, (Uint16)rect.h);
}

GPU_Rect GPU_SetClip(GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
	if(target == NULL || _gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
	{
		GPU_Rect r = {0,0,0,0};
		return r;
	}
	
	return _gpu_current_renderer->impl->SetClip(_gpu_current_renderer, target, x, y, w, h);
}

void GPU_UnsetClip(GPU_Target* target)
{
	if(target == NULL || _gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return;
	
	_gpu_current_renderer->impl->UnsetClip(_gpu_current_renderer, target);
}




void GPU_SetColor(GPU_Image* image, SDL_Color color)
{
	if(image == NULL)
		return;
	
	image->color = color;
}

void GPU_SetRGB(GPU_Image* image, Uint8 r, Uint8 g, Uint8 b)
{
	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	GET_ALPHA(c) = 255;

	if(image == NULL)
		return;
	
	image->color = c;
}

void GPU_SetRGBA(GPU_Image* image, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	GET_ALPHA(c) = a;

	if(image == NULL)
		return;
	
	image->color = c;
}

void GPU_UnsetColor(GPU_Image* image)
{
    SDL_Color c = {255, 255, 255, 255};
	if(image == NULL)
		return;
	
    image->color = c;
}

void GPU_SetTargetColor(GPU_Target* target, SDL_Color color)
{
	if(target == NULL)
		return;
	
    target->use_color = 1;
    target->color = color;
}

void GPU_SetTargetRGB(GPU_Target* target, Uint8 r, Uint8 g, Uint8 b)
{
	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	GET_ALPHA(c) = 255;

	if(target == NULL)
		return;
	
    target->use_color = !(r == 255 && g == 255 && b == 255);
    target->color = c;
}

void GPU_SetTargetRGBA(GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	SDL_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	GET_ALPHA(c) = a;

	if(target == NULL)
		return;
	
    target->use_color = !(r == 255 && g == 255 && b == 255 && a == 255);
    target->color = c;
}

void GPU_UnsetTargetColor(GPU_Target* target)
{
    SDL_Color c = {255, 255, 255, 255};
	if(target == NULL)
		return;
    
    target->use_color = 0;
    target->color = c;
}

Uint8 GPU_GetBlending(GPU_Image* image)
{
	if(image == NULL)
		return 0;
	
	return image->use_blending;
}


void GPU_SetBlending(GPU_Image* image, Uint8 enable)
{
	if(image == NULL)
		return;
	
	image->use_blending = enable;
}

void GPU_SetShapeBlending(Uint8 enable)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->current_context_target->context->shapes_use_blending = enable;
}


GPU_BlendMode GPU_GetBlendModeFromPreset(GPU_BlendPresetEnum preset)
{
	switch(preset)
	{
    case GPU_BLEND_NORMAL:
        {
            GPU_BlendMode b = {GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_EQ_ADD, GPU_EQ_ADD};
            return b;
        }
        break;
    case GPU_BLEND_PREMULTIPLIED_ALPHA:
        {
            GPU_BlendMode b = {GPU_FUNC_ONE, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_FUNC_ONE, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_EQ_ADD, GPU_EQ_ADD};
            return b;
        }
        break;
    case GPU_BLEND_MULTIPLY:
        {
            GPU_BlendMode b = {GPU_FUNC_DST_COLOR, GPU_FUNC_ZERO, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_EQ_ADD, GPU_EQ_ADD};
            return b;
        }
        break;
    case GPU_BLEND_ADD:
        {
            GPU_BlendMode b = {GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE, GPU_EQ_ADD, GPU_EQ_ADD};
            return b;
        }
        break;
    case GPU_BLEND_SUBTRACT:
        // FIXME: Use src alpha for source components?
        {
            GPU_BlendMode b = {GPU_FUNC_ONE, GPU_FUNC_ONE, GPU_FUNC_ONE, GPU_FUNC_ONE, GPU_EQ_SUBTRACT, GPU_EQ_SUBTRACT};
            return b;
        }
        break;
    case GPU_BLEND_MOD_ALPHA:
        // Don't disturb the colors, but multiply the dest alpha by the src alpha
        {
            GPU_BlendMode b = {GPU_FUNC_ZERO, GPU_FUNC_ONE, GPU_FUNC_ZERO, GPU_FUNC_SRC_ALPHA, GPU_EQ_ADD, GPU_EQ_ADD};
            return b;
        }
        break;
    case GPU_BLEND_SET_ALPHA:
        // Don't disturb the colors, but set the alpha to the src alpha
        {
            GPU_BlendMode b = {GPU_FUNC_ZERO, GPU_FUNC_ONE, GPU_FUNC_ONE, GPU_FUNC_ZERO, GPU_EQ_ADD, GPU_EQ_ADD};
            return b;
        }
        break;
    case GPU_BLEND_SET:
        {
            GPU_BlendMode b = {GPU_FUNC_ONE, GPU_FUNC_ZERO, GPU_FUNC_ONE, GPU_FUNC_ZERO, GPU_EQ_ADD, GPU_EQ_ADD};
            return b;
        }
        break;
    case GPU_BLEND_NORMAL_KEEP_ALPHA:
        {
            GPU_BlendMode b = {GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_FUNC_ZERO, GPU_FUNC_ONE, GPU_EQ_ADD, GPU_EQ_ADD};
            return b;
        }
        break;
    case GPU_BLEND_NORMAL_ADD_ALPHA:
        {
            GPU_BlendMode b = {GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_FUNC_ONE, GPU_FUNC_ONE, GPU_EQ_ADD, GPU_EQ_ADD};
            return b;
        }
        break;
    default:
        GPU_PushErrorCode(__func__, GPU_ERROR_USER_ERROR, "Blend preset not supported: %d", preset);
        {
            GPU_BlendMode b = {GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_EQ_ADD, GPU_EQ_ADD};
            return b;
        }
        break;
	}
}


void GPU_SetBlendFunction(GPU_Image* image, GPU_BlendFuncEnum source_color, GPU_BlendFuncEnum dest_color, GPU_BlendFuncEnum source_alpha, GPU_BlendFuncEnum dest_alpha)
{
	if(image == NULL)
		return;
	
	image->blend_mode.source_color = source_color;
	image->blend_mode.dest_color = dest_color;
	image->blend_mode.source_alpha = source_alpha;
	image->blend_mode.dest_alpha = dest_alpha;
}

void GPU_SetBlendEquation(GPU_Image* image, GPU_BlendEqEnum color_equation, GPU_BlendEqEnum alpha_equation)
{
	if(image == NULL)
		return;
    
    image->blend_mode.color_equation = color_equation;
    image->blend_mode.alpha_equation = alpha_equation;
}

void GPU_SetBlendMode(GPU_Image* image, GPU_BlendPresetEnum preset)
{
    GPU_BlendMode b;
	if(image == NULL)
		return;
	
	b = GPU_GetBlendModeFromPreset(preset);
    GPU_SetBlendFunction(image, b.source_color, b.dest_color, b.source_alpha, b.dest_alpha);
    GPU_SetBlendEquation(image, b.color_equation, b.alpha_equation);
}

void GPU_SetShapeBlendFunction(GPU_BlendFuncEnum source_color, GPU_BlendFuncEnum dest_color, GPU_BlendFuncEnum source_alpha, GPU_BlendFuncEnum dest_alpha)
{
    GPU_Context* context;
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	context = _gpu_current_renderer->current_context_target->context;
	
	context->shapes_blend_mode.source_color = source_color;
	context->shapes_blend_mode.dest_color = dest_color;
	context->shapes_blend_mode.source_alpha = source_alpha;
	context->shapes_blend_mode.dest_alpha = dest_alpha;
}

void GPU_SetShapeBlendEquation(GPU_BlendEqEnum color_equation, GPU_BlendEqEnum alpha_equation)
{
    GPU_Context* context;
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
    
	context = _gpu_current_renderer->current_context_target->context;
    
    context->shapes_blend_mode.color_equation = color_equation;
    context->shapes_blend_mode.alpha_equation = alpha_equation;
}

void GPU_SetShapeBlendMode(GPU_BlendPresetEnum preset)
{
    GPU_BlendMode b;
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	b = GPU_GetBlendModeFromPreset(preset);
    GPU_SetShapeBlendFunction(b.source_color, b.dest_color, b.source_alpha, b.dest_alpha);
    GPU_SetShapeBlendEquation(b.color_equation, b.alpha_equation);
}

void GPU_SetImageFilter(GPU_Image* image, GPU_FilterEnum filter)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	if(image == NULL)
		return;
	
	_gpu_current_renderer->impl->SetImageFilter(_gpu_current_renderer, image, filter);
}

GPU_SnapEnum GPU_GetSnapMode(GPU_Image* image)
{
	if(image == NULL)
		return 0;
	
	return image->snap_mode;
}

void GPU_SetSnapMode(GPU_Image* image, GPU_SnapEnum mode)
{
	if(image == NULL)
		return;
	
	image->snap_mode = mode;
}

void GPU_SetWrapMode(GPU_Image* image, GPU_WrapEnum wrap_mode_x, GPU_WrapEnum wrap_mode_y)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	if(image == NULL)
		return;
	
	_gpu_current_renderer->impl->SetWrapMode(_gpu_current_renderer, image, wrap_mode_x, wrap_mode_y);
}


SDL_Color GPU_GetPixel(GPU_Target* target, Sint16 x, Sint16 y)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
	{
		SDL_Color c = {0,0,0,0};
		return c;
	}
	
	return _gpu_current_renderer->impl->GetPixel(_gpu_current_renderer, target, x, y);
}







void GPU_Clear(GPU_Target* target)
{
    if(!CHECK_RENDERER)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL renderer");
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL context");
	
	_gpu_current_renderer->impl->ClearRGBA(_gpu_current_renderer, target, 0, 0, 0, 0);
}

void GPU_ClearColor(GPU_Target* target, SDL_Color color)
{
    if(!CHECK_RENDERER)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL renderer");
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL context");
	
    _gpu_current_renderer->impl->ClearRGBA(_gpu_current_renderer, target, color.r, color.g, color.b, GET_ALPHA(color));
}

void GPU_ClearRGB(GPU_Target* target, Uint8 r, Uint8 g, Uint8 b)
{
    if(!CHECK_RENDERER)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL renderer");
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL context");
	
	_gpu_current_renderer->impl->ClearRGBA(_gpu_current_renderer, target, r, g, b, 255);
}

void GPU_ClearRGBA(GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    if(!CHECK_RENDERER)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL renderer");
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL context");
	
	_gpu_current_renderer->impl->ClearRGBA(_gpu_current_renderer, target, r, g, b, a);
}

void GPU_FlushBlitBuffer(void)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->FlushBlitBuffer(_gpu_current_renderer);
}

void GPU_Flip(GPU_Target* target)
{
    if(!CHECK_RENDERER)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL renderer");
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL context");
	
	_gpu_current_renderer->impl->Flip(_gpu_current_renderer, target);
}





// Shader API


Uint32 GPU_CompileShader_RW(GPU_ShaderEnum shader_type, SDL_RWops* shader_source)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return 0;
	
	return _gpu_current_renderer->impl->CompileShader_RW(_gpu_current_renderer, shader_type, shader_source);
}

Uint32 GPU_LoadShader(GPU_ShaderEnum shader_type, const char* filename)
{
	SDL_RWops* rwops;
	Uint32 result;

    if(filename == NULL)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_NULL_ARGUMENT, "filename");
        return 0;
    }
    rwops = SDL_RWFromFile(filename, "r");
    if(rwops == NULL)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_FILE_NOT_FOUND, "%s", filename);
        return 0;
    }
    result = GPU_CompileShader_RW(shader_type, rwops);
    SDL_RWclose(rwops);
    return result;
}

Uint32 GPU_CompileShader(GPU_ShaderEnum shader_type, const char* shader_source)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return 0;
	
	return _gpu_current_renderer->impl->CompileShader(_gpu_current_renderer, shader_type, shader_source);
}

Uint8 GPU_LinkShaderProgram(Uint32 program_object)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return 0;
	
	return _gpu_current_renderer->impl->LinkShaderProgram(_gpu_current_renderer, program_object);
}

Uint32 GPU_CreateShaderProgram(void)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return 0;
    
	return _gpu_current_renderer->impl->CreateShaderProgram(_gpu_current_renderer);
}

Uint32 GPU_LinkShaders(Uint32 shader_object1, Uint32 shader_object2)
{
    Uint32 p;
    
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return 0;
	
    if((_gpu_current_renderer->enabled_features & GPU_FEATURE_BASIC_SHADERS) != GPU_FEATURE_BASIC_SHADERS)
        return 0;
    
    p = _gpu_current_renderer->impl->CreateShaderProgram(_gpu_current_renderer);

	_gpu_current_renderer->impl->AttachShader(_gpu_current_renderer, p, shader_object1);
	_gpu_current_renderer->impl->AttachShader(_gpu_current_renderer, p, shader_object2);
	
	if(_gpu_current_renderer->impl->LinkShaderProgram(_gpu_current_renderer, p))
        return p;
    
    _gpu_current_renderer->impl->FreeShaderProgram(_gpu_current_renderer, p);
    return 0;
}

void GPU_FreeShader(Uint32 shader_object)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->FreeShader(_gpu_current_renderer, shader_object);
}

void GPU_FreeShaderProgram(Uint32 program_object)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->FreeShaderProgram(_gpu_current_renderer, program_object);
}

void GPU_AttachShader(Uint32 program_object, Uint32 shader_object)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->AttachShader(_gpu_current_renderer, program_object, shader_object);
}

void GPU_DetachShader(Uint32 program_object, Uint32 shader_object)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->DetachShader(_gpu_current_renderer, program_object, shader_object);
}

Uint8 GPU_IsDefaultShaderProgram(Uint32 program_object)
{
    GPU_Context* context;
    
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return 0;
    
    context = _gpu_current_renderer->current_context_target->context;
    return (program_object == context->default_textured_shader_program || program_object == context->default_untextured_shader_program);
}

void GPU_ActivateShaderProgram(Uint32 program_object, GPU_ShaderBlock* block)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->ActivateShaderProgram(_gpu_current_renderer, program_object, block);
}

void GPU_DeactivateShaderProgram(void)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->DeactivateShaderProgram(_gpu_current_renderer);
}

const char* GPU_GetShaderMessage(void)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return NULL;
	
	return _gpu_current_renderer->impl->GetShaderMessage(_gpu_current_renderer);
}

int GPU_GetAttributeLocation(Uint32 program_object, const char* attrib_name)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return 0;
	
	return _gpu_current_renderer->impl->GetAttributeLocation(_gpu_current_renderer, program_object, attrib_name);
}

GPU_AttributeFormat GPU_MakeAttributeFormat(int num_elems_per_vertex, GPU_TypeEnum type, Uint8 normalize, int stride_bytes, int offset_bytes)
{
	GPU_AttributeFormat f;
	f.is_per_sprite = 0;
	f.num_elems_per_value = num_elems_per_vertex;
	f.type = type;
	f.normalize = normalize;
	f.stride_bytes = stride_bytes;
	f.offset_bytes = offset_bytes;
    return f;
}

GPU_Attribute GPU_MakeAttribute(int location, void* values, GPU_AttributeFormat format)
{
	GPU_Attribute a;
	a.location = location;
	a.values = values;
	a.format = format;
    return a;
}

int GPU_GetUniformLocation(Uint32 program_object, const char* uniform_name)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return 0;
	
	return _gpu_current_renderer->impl->GetUniformLocation(_gpu_current_renderer, program_object, uniform_name);
}

GPU_ShaderBlock GPU_LoadShaderBlock(Uint32 program_object, const char* position_name, const char* texcoord_name, const char* color_name, const char* modelViewMatrix_name)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
    {
        GPU_ShaderBlock b;
        b.position_loc = -1;
        b.texcoord_loc = -1;
        b.color_loc = -1;
        b.modelViewProjection_loc = -1;
		return b;
    }
	
	return _gpu_current_renderer->impl->LoadShaderBlock(_gpu_current_renderer, program_object, position_name, texcoord_name, color_name, modelViewMatrix_name);
}

void GPU_SetShaderBlock(GPU_ShaderBlock block)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetShaderBlock(_gpu_current_renderer, block);
}

void GPU_SetShaderImage(GPU_Image* image, int location, int image_unit)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetShaderImage(_gpu_current_renderer, image, location, image_unit);
}

void GPU_GetUniformiv(Uint32 program_object, int location, int* values)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->GetUniformiv(_gpu_current_renderer, program_object, location, values);
}

void GPU_SetUniformi(int location, int value)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetUniformi(_gpu_current_renderer, location, value);
}

void GPU_SetUniformiv(int location, int num_elements_per_value, int num_values, int* values)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetUniformiv(_gpu_current_renderer, location, num_elements_per_value, num_values, values);
}


void GPU_GetUniformuiv(Uint32 program_object, int location, unsigned int* values)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->GetUniformuiv(_gpu_current_renderer, program_object, location, values);
}

void GPU_SetUniformui(int location, unsigned int value)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetUniformui(_gpu_current_renderer, location, value);
}

void GPU_SetUniformuiv(int location, int num_elements_per_value, int num_values, unsigned int* values)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetUniformuiv(_gpu_current_renderer, location, num_elements_per_value, num_values, values);
}


void GPU_GetUniformfv(Uint32 program_object, int location, float* values)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->GetUniformfv(_gpu_current_renderer, program_object, location, values);
}

void GPU_SetUniformf(int location, float value)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetUniformf(_gpu_current_renderer, location, value);
}

void GPU_SetUniformfv(int location, int num_elements_per_value, int num_values, float* values)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetUniformfv(_gpu_current_renderer, location, num_elements_per_value, num_values, values);
}

// Same as GPU_GetUniformfv()
void GPU_GetUniformMatrixfv(Uint32 program_object, int location, float* values)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->GetUniformfv(_gpu_current_renderer, program_object, location, values);
}

void GPU_SetUniformMatrixfv(int location, int num_matrices, int num_rows, int num_columns, Uint8 transpose, float* values)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetUniformMatrixfv(_gpu_current_renderer, location, num_matrices, num_rows, num_columns, transpose, values);
}


void GPU_SetAttributef(int location, float value)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetAttributef(_gpu_current_renderer, location, value);
}

void GPU_SetAttributei(int location, int value)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetAttributei(_gpu_current_renderer, location, value);
}

void GPU_SetAttributeui(int location, unsigned int value)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetAttributeui(_gpu_current_renderer, location, value);
}

void GPU_SetAttributefv(int location, int num_elements, float* value)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetAttributefv(_gpu_current_renderer, location, num_elements, value);
}

void GPU_SetAttributeiv(int location, int num_elements, int* value)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetAttributeiv(_gpu_current_renderer, location, num_elements, value);
}

void GPU_SetAttributeuiv(int location, int num_elements, unsigned int* value)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetAttributeuiv(_gpu_current_renderer, location, num_elements, value);
}

void GPU_SetAttributeSource(int num_values, GPU_Attribute source)
{
	if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
		return;
	
	_gpu_current_renderer->impl->SetAttributeSource(_gpu_current_renderer, num_values, source);
}




// gpu_strcasecmp()
// A portable strcasecmp() from UC Berkeley
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific written prior permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

/*
 * This array is designed for mapping upper and lower case letter
 * together for a case independent comparison.  The mappings are
 * based upon ascii character sequences.
 */
static const unsigned char caseless_charmap[] = {
	'\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
	'\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
	'\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
	'\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
	'\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
	'\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
	'\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
	'\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
	'\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
	'\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
	'\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
	'\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
	'\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
	'\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
	'\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
	'\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
	'\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
	'\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
	'\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
	'\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

int gpu_strcasecmp(const char* s1, const char* s2)
{
    unsigned char u1, u2;

    for (;;)
    {
        u1 = (unsigned char) *s1++;
        u2 = (unsigned char) *s2++;
        if (caseless_charmap[u1] != caseless_charmap[u2])
            return caseless_charmap[u1] - caseless_charmap[u2];
        if (u1 == '\0')
            return 0;
    }
    return 0;
}


#ifdef _MSC_VER
	#pragma warning(pop) 
#endif

