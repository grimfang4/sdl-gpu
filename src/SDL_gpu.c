#include "SDL_gpu.h"
#include "SDL_gpu_RendererImpl.h"
#include "SDL_platform.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include <stdlib.h>
#include <string.h>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef _MSC_VER
	#define __func__ __FUNCTION__
	#pragma warning(push)
	// Visual Studio wants to complain about while(0)
	#pragma warning(disable: 4127)

	// Disable warning: selection for inlining
	#pragma warning(disable: 4514 4711)
	// Disable warning: Spectre mitigation
	#pragma warning(disable: 5045)
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
void gpu_free_renderer_register(void);
GPU_Renderer* gpu_create_and_add_renderer(GPU_RendererID id);

int gpu_default_print(GPU_LogLevelEnum log_level, const char* format, va_list args);

/*! A mapping of windowID to a GPU_Target to facilitate GPU_GetWindowTarget(). */
typedef struct GPU_WindowMapping
{
    Uint32 windowID;
    GPU_Target* target;
} GPU_WindowMapping;

static GPU_Renderer* _gpu_current_renderer = NULL;

static GPU_DebugLevelEnum _gpu_debug_level = GPU_DEBUG_LEVEL_0;

#define GPU_DEFAULT_MAX_NUM_ERRORS 20
#define GPU_ERROR_FUNCTION_STRING_MAX 128
#define GPU_ERROR_DETAILS_STRING_MAX 512
static GPU_ErrorObject* _gpu_error_code_queue = NULL;
static unsigned int _gpu_num_error_codes = 0;
static unsigned int _gpu_error_code_queue_size = GPU_DEFAULT_MAX_NUM_ERRORS;
static GPU_ErrorObject _gpu_error_code_result;

#define GPU_INITIAL_WINDOW_MAPPINGS_SIZE 10
static GPU_WindowMapping* _gpu_window_mappings = NULL;
static int _gpu_window_mappings_size = 0;
static int _gpu_num_window_mappings = 0;

static Uint32 _gpu_init_windowID = 0;

static GPU_InitFlagEnum _gpu_preinit_flags = GPU_DEFAULT_INIT_FLAGS;
static GPU_InitFlagEnum _gpu_required_features = 0;

static GPU_bool _gpu_initialized_SDL_core = GPU_FALSE;
static GPU_bool _gpu_initialized_SDL = GPU_FALSE;

static int (*_gpu_print)(GPU_LogLevelEnum log_level, const char* format, va_list args) = &gpu_default_print;


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

void GPU_SetCoordinateMode(GPU_bool use_math_coords)
{
    if(_gpu_current_renderer == NULL)
        return;

    _gpu_current_renderer->coordinate_mode = use_math_coords;
}

GPU_bool GPU_GetCoordinateMode(void)
{
    if(_gpu_current_renderer == NULL)
        return GPU_FALSE;

    return _gpu_current_renderer->coordinate_mode;
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


static GPU_bool gpu_init_SDL(void)
{
    if(!_gpu_initialized_SDL)
    {
        if(!_gpu_initialized_SDL_core && !SDL_WasInit(SDL_INIT_EVERYTHING))
        {
            // Nothing has been set up, so init SDL and the video subsystem.
            if(SDL_Init(SDL_INIT_VIDEO) < 0)
            {
                GPU_PushErrorCode("GPU_Init", GPU_ERROR_BACKEND_ERROR, "Failed to initialize SDL video subsystem");
                return GPU_FALSE;
            }
            _gpu_initialized_SDL_core = GPU_TRUE;
        }

        // SDL is definitely ready now, but we're going to init the video subsystem to be sure that SDL_gpu keeps it available until GPU_Quit().
        if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
        {
            GPU_PushErrorCode("GPU_Init", GPU_ERROR_BACKEND_ERROR, "Failed to initialize SDL video subsystem");
            return GPU_FALSE;
        }
        _gpu_initialized_SDL = GPU_TRUE;
    }
    return GPU_TRUE;
}

void GPU_SetInitWindow(Uint32 windowID)
{
    _gpu_init_windowID = windowID;
}

Uint32 GPU_GetInitWindow(void)
{
    return _gpu_init_windowID;
}

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

static void gpu_init_error_queue(void)
{
    if(_gpu_error_code_queue == NULL)
    {
        unsigned int i;
        _gpu_error_code_queue = (GPU_ErrorObject*)SDL_malloc(sizeof(GPU_ErrorObject)*_gpu_error_code_queue_size);

        for(i = 0; i < _gpu_error_code_queue_size; i++)
        {
            _gpu_error_code_queue[i].function = (char*)SDL_malloc(GPU_ERROR_FUNCTION_STRING_MAX+1);
            _gpu_error_code_queue[i].error = GPU_ERROR_NONE;
            _gpu_error_code_queue[i].details = (char*)SDL_malloc(GPU_ERROR_DETAILS_STRING_MAX+1);
        }
        _gpu_num_error_codes = 0;

        _gpu_error_code_result.function = (char*)SDL_malloc(GPU_ERROR_FUNCTION_STRING_MAX+1);
        _gpu_error_code_result.error = GPU_ERROR_NONE;
        _gpu_error_code_result.details = (char*)SDL_malloc(GPU_ERROR_DETAILS_STRING_MAX+1);
    }
}

static void gpu_init_window_mappings(void)
{
    if(_gpu_window_mappings == NULL)
    {
        _gpu_window_mappings_size = GPU_INITIAL_WINDOW_MAPPINGS_SIZE;
        _gpu_window_mappings = (GPU_WindowMapping*)SDL_malloc(_gpu_window_mappings_size * sizeof(GPU_WindowMapping));
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
        new_array = (GPU_WindowMapping*)SDL_malloc(_gpu_window_mappings_size * sizeof(GPU_WindowMapping));
        memcpy(new_array, _gpu_window_mappings, _gpu_num_window_mappings * sizeof(GPU_WindowMapping));
        SDL_free(_gpu_window_mappings);
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
    for(i = 0; i < _gpu_num_window_mappings; ++i)
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
    for(i = 0; i < _gpu_num_window_mappings; ++i)
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

    gpu_init_error_queue();

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

    GPU_PushErrorCode("GPU_Init", GPU_ERROR_BACKEND_ERROR, "No renderer out of %d was able to initialize properly", renderer_order_size);
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

    gpu_init_error_queue();
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
        GPU_PushErrorCode("GPU_InitRendererByID", GPU_ERROR_BACKEND_ERROR, "Renderer %s failed to initialize properly", renderer->id.name);
        // Init failed, destroy the renderer...
        // Erase the window mappings
        _gpu_num_window_mappings = 0;
        GPU_CloseCurrentRenderer();
    }
    else
        GPU_SetInitWindow(0);
    return screen;
}

GPU_bool GPU_IsFeatureEnabled(GPU_FeatureEnum feature)
{
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return GPU_FALSE;

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

GPU_bool GPU_SetFullscreen(GPU_bool enable_fullscreen, GPU_bool use_desktop_resolution)
{
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return GPU_FALSE;

    return _gpu_current_renderer->impl->SetFullscreen(_gpu_current_renderer, enable_fullscreen, use_desktop_resolution);
}

GPU_bool GPU_GetFullscreen(void)
{
#ifdef SDL_GPU_USE_SDL2
    GPU_Target* target = GPU_GetContextTarget();
    if(target == NULL)
        return GPU_FALSE;
    return (SDL_GetWindowFlags(SDL_GetWindowFromID(target->context->windowID))
                   & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
#else
    SDL_Surface* surf = SDL_GetVideoSurface();
    if(surf == NULL)
        return GPU_FALSE;
    return (surf->flags & SDL_FULLSCREEN) != 0;
#endif
}

GPU_Target* GPU_GetActiveTarget(void)
{
    GPU_Target* context_target = GPU_GetContextTarget();
    if(context_target == NULL)
        return NULL;

    return context_target->context->active_target;
}

GPU_bool GPU_SetActiveTarget(GPU_Target* target)
{
    if(_gpu_current_renderer == NULL)
        return GPU_FALSE;

    return _gpu_current_renderer->impl->SetActiveTarget(_gpu_current_renderer, target);
}

GPU_bool GPU_AddDepthBuffer(GPU_Target* target)
{
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL || target == NULL)
        return GPU_FALSE;

    return _gpu_current_renderer->impl->AddDepthBuffer(_gpu_current_renderer, target);
}

void GPU_SetDepthTest(GPU_Target* target, GPU_bool enable)
{
    if(target != NULL)
        target->use_depth_test = enable;
}

void GPU_SetDepthWrite(GPU_Target* target, GPU_bool enable)
{
    if(target != NULL)
        target->use_depth_write = enable;
}

void GPU_SetDepthFunction(GPU_Target* target, GPU_ComparisonEnum compare_operation)
{
    if(target != NULL)
        target->depth_function = compare_operation;
}

GPU_bool GPU_SetWindowResolution(Uint16 w, Uint16 h)
{
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL || w == 0 || h == 0)
        return GPU_FALSE;

    return _gpu_current_renderer->impl->SetWindowResolution(_gpu_current_renderer, w, h);
}


void GPU_GetVirtualResolution(GPU_Target* target, Uint16* w, Uint16* h)
{
    // No checking here for NULL w or h...  Should we?
	if (target == NULL)
	{
		*w = 0;
		*h = 0;
	}
	else {
		*w = target->w;
		*h = target->h;
	}
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

    GPU_FlushBlitBuffer();  // TODO: Perhaps move SetImageVirtualResolution into the renderer so we can check to see if this image is bound first.
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

    GPU_FlushBlitBuffer();  // TODO: Perhaps move SetImageVirtualResolution into the renderer so we can check to see if this image is bound first.
    image->w = image->base_w;
    image->h = image->base_h;
    image->using_virtual_resolution = 0;
}

void gpu_free_error_queue(void)
{
    unsigned int i;
    // Free the error queue
    for(i = 0; i < _gpu_error_code_queue_size; i++)
    {
        SDL_free(_gpu_error_code_queue[i].function);
        _gpu_error_code_queue[i].function = NULL;
        SDL_free(_gpu_error_code_queue[i].details);
        _gpu_error_code_queue[i].details = NULL;
    }
    SDL_free(_gpu_error_code_queue);
    _gpu_error_code_queue = NULL;
    _gpu_num_error_codes = 0;

    SDL_free(_gpu_error_code_result.function);
    _gpu_error_code_result.function = NULL;
    SDL_free(_gpu_error_code_result.details);
    _gpu_error_code_result.details = NULL;
}

// Deletes all existing errors
void GPU_SetErrorQueueMax(unsigned int max)
{
    gpu_free_error_queue();

    // Reallocate with new size
    _gpu_error_code_queue_size = max;
    gpu_init_error_queue();
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
    if(_gpu_num_error_codes > 0 && GPU_GetDebugLevel() >= GPU_DEBUG_LEVEL_1)
        GPU_LogError("GPU_Quit: %d uncleared error%s.\n", _gpu_num_error_codes, (_gpu_num_error_codes > 1? "s" : ""));

    gpu_free_error_queue();

    if(_gpu_current_renderer == NULL)
        return;

    _gpu_current_renderer->impl->Quit(_gpu_current_renderer);
    GPU_FreeRenderer(_gpu_current_renderer);
    // FIXME: Free all renderers
    _gpu_current_renderer = NULL;

    _gpu_init_windowID = 0;

    // Free window mappings
    SDL_free(_gpu_window_mappings);
    _gpu_window_mappings = NULL;
    _gpu_window_mappings_size = 0;
    _gpu_num_window_mappings = 0;

    gpu_free_renderer_register();

    if(_gpu_initialized_SDL)
    {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        _gpu_initialized_SDL = 0;

        if(_gpu_initialized_SDL_core)
        {
            SDL_Quit();
            _gpu_initialized_SDL_core = 0;
        }
    }
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
    gpu_init_error_queue();

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

    if(_gpu_num_error_codes < _gpu_error_code_queue_size)
    {
        if(function == NULL)
            _gpu_error_code_queue[_gpu_num_error_codes].function[0] = '\0';
        else
        {
            strncpy(_gpu_error_code_queue[_gpu_num_error_codes].function, function, GPU_ERROR_FUNCTION_STRING_MAX);
            _gpu_error_code_queue[_gpu_num_error_codes].function[GPU_ERROR_FUNCTION_STRING_MAX] = '\0';
        }
        _gpu_error_code_queue[_gpu_num_error_codes].error = error;
        if(details == NULL)
            _gpu_error_code_queue[_gpu_num_error_codes].details[0] = '\0';
        else
        {
            va_list lst;
            va_start(lst, details);
            vsnprintf(_gpu_error_code_queue[_gpu_num_error_codes].details, GPU_ERROR_DETAILS_STRING_MAX, details, lst);
            va_end(lst);
        }
        _gpu_num_error_codes++;
    }
}

GPU_ErrorObject GPU_PopErrorCode(void)
{
    unsigned int i;
    GPU_ErrorObject result = {NULL, NULL, GPU_ERROR_NONE};

    gpu_init_error_queue();

    if(_gpu_num_error_codes <= 0)
        return result;

    // Pop the oldest
    strcpy(_gpu_error_code_result.function, _gpu_error_code_queue[0].function);
    _gpu_error_code_result.error = _gpu_error_code_queue[0].error;
    strcpy(_gpu_error_code_result.details, _gpu_error_code_queue[0].details);

    // We'll be returning that one
    result = _gpu_error_code_result;

    // Move the rest down
    _gpu_num_error_codes--;
    for(i = 0; i < _gpu_num_error_codes; i++)
    {
        strcpy(_gpu_error_code_queue[i].function, _gpu_error_code_queue[i+1].function);
        _gpu_error_code_queue[i].error = _gpu_error_code_queue[i+1].error;
        strcpy(_gpu_error_code_queue[i].details, _gpu_error_code_queue[i+1].details);
    }
    return result;
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
    if(target == NULL || _gpu_current_renderer == NULL)
        return;

    // Scale from raw window/image coords to the virtual scale
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
        // What is the backing for this target?!
        if(x != NULL)
            *x = displayX;
        if(y != NULL)
            *y = displayY;
    }
    
    // Invert coordinates to math coords
    if(_gpu_current_renderer->coordinate_mode)
        *y = target->h - *y;
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

void GPU_UnsetViewport(GPU_Target* target)
{
    if(target != NULL)
        target->viewport = GPU_MakeRect(0, 0, target->w, target->h);
}

GPU_Camera GPU_GetDefaultCamera(void)
{
    GPU_Camera cam = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -100.0f, 100.0f, true};
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
	// TODO: Remove from renderer and flush here
    return _gpu_current_renderer->impl->SetCamera(_gpu_current_renderer, target, cam);
}

void GPU_EnableCamera(GPU_Target* target, GPU_bool use_camera)
{
	if (target == NULL)
		return;
	// TODO: Flush here
	target->use_camera = use_camera;
}

GPU_bool GPU_IsCameraEnabled(GPU_Target* target)
{
	if (target == NULL)
		return GPU_FALSE;
	return target->use_camera;
}

GPU_Image* GPU_CreateImage(Uint16 w, Uint16 h, GPU_FormatEnum format)
{
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return NULL;

    return _gpu_current_renderer->impl->CreateImage(_gpu_current_renderer, w, h, format);
}

GPU_Image* GPU_CreateImageUsingTexture(GPU_TextureHandle handle, GPU_bool take_ownership)
{
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return NULL;

    return _gpu_current_renderer->impl->CreateImageUsingTexture(_gpu_current_renderer, handle, take_ownership);
}

GPU_Image* GPU_LoadImage(const char* filename)
{
    return GPU_LoadImage_RW(SDL_RWFromFile(filename, "r"), 1);
}

GPU_Image* GPU_LoadImage_RW(SDL_RWops* rwops, GPU_bool free_rwops)
{
	GPU_Image* result;
	SDL_Surface* surface;
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return NULL;
        
    surface = GPU_LoadSurface_RW(rwops, free_rwops);
    if(surface == NULL)
    {
        GPU_PushErrorCode("GPU_LoadImage_RW", GPU_ERROR_DATA_ERROR, "Failed to load image data.");
        return NULL;
    }

    result = _gpu_current_renderer->impl->CopyImageFromSurface(_gpu_current_renderer, surface);
    SDL_FreeSurface(surface);

    return result;
}

GPU_Image* GPU_CreateAliasImage(GPU_Image* image)
{
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return NULL;

    return _gpu_current_renderer->impl->CreateAliasImage(_gpu_current_renderer, image);
}

GPU_bool GPU_SaveImage(GPU_Image* image, const char* filename, GPU_FileFormatEnum format)
{
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return GPU_FALSE;

    return _gpu_current_renderer->impl->SaveImage(_gpu_current_renderer, image, filename, format);
}

GPU_bool GPU_SaveImage_RW(GPU_Image* image, SDL_RWops* rwops, GPU_bool free_rwops, GPU_FileFormatEnum format)
{
    GPU_bool result;
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return GPU_FALSE;

    SDL_Surface* surface = GPU_CopySurfaceFromImage(image);
    result = GPU_SaveSurface_RW(surface, rwops, free_rwops, format);
    SDL_FreeSurface(surface);
    return result;
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

GPU_bool GPU_ReplaceImage(GPU_Image* image, SDL_Surface* surface, const GPU_Rect* surface_rect)
{
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return GPU_FALSE;

    return _gpu_current_renderer->impl->ReplaceImage(_gpu_current_renderer, image, surface, surface_rect);
}

static SDL_Surface* gpu_copy_raw_surface_data(unsigned char* data, int width, int height, int channels)
{
    int i;
    Uint32 Rmask, Gmask, Bmask, Amask = 0;
    SDL_Surface* result;
    
    if(data == NULL)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_DATA_ERROR, "Got NULL data");
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
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
#else
        Rmask = 0x000000ff;
        Gmask = 0x0000ff00;
        Bmask = 0x00ff0000;
        Amask = 0xff000000;
#endif
        break;
    default:
        Rmask = Gmask = Bmask = 0;
        GPU_PushErrorCode(__func__, GPU_ERROR_DATA_ERROR, "Invalid number of channels: %d", channels);
        return NULL;
        break;
    }

    result = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, channels*8, Rmask, Gmask, Bmask, Amask);
    //result = SDL_CreateRGBSurfaceFrom(data, width, height, channels * 8, width * channels, Rmask, Gmask, Bmask, Amask);
    if(result == NULL)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_DATA_ERROR, "Failed to create new %dx%d surface", width, height);
        return NULL;
    }

    // Copy row-by-row in case the pitch doesn't match
    for(i = 0; i < height; ++i)
    {
        memcpy((Uint8*)result->pixels + i*result->pitch, data + channels*width*i, channels*width);
    }
    
    if(result != NULL && result->format->palette != NULL)
    {
        // SDL_CreateRGBSurface has no idea what palette to use, so it uses a blank one.
        // We'll at least create a grayscale one, but it's not ideal...
        // Better would be to get the palette from stbi, but stbi doesn't do that!
        SDL_Color colors[256];

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

SDL_Surface* GPU_LoadSurface_RW(SDL_RWops* rwops, GPU_bool free_rwops)
{
    int width, height, channels;
    unsigned char* data;
    SDL_Surface* result;
    
    int data_bytes;
    unsigned char* c_data;

    if(rwops == NULL)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_NULL_ARGUMENT, "rwops");
        return NULL;
    }

    // Get count of bytes
    SDL_RWseek(rwops, 0, SEEK_SET);
    data_bytes = (int)SDL_RWseek(rwops, 0, SEEK_END);
    SDL_RWseek(rwops, 0, SEEK_SET);
    
    // Read in the rwops data
    c_data = (unsigned char*)SDL_malloc(data_bytes);
    SDL_RWread(rwops, c_data, 1, data_bytes);
    
    // Load image
    data = stbi_load_from_memory(c_data, data_bytes, &width, &height, &channels, 0);
    
    // Clean up temp data
    SDL_free(c_data);
    if(free_rwops)
        SDL_RWclose(rwops);

    if(data == NULL)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_DATA_ERROR, "Failed to load from rwops: %s", stbi_failure_reason());
        return NULL;
    }

    // Copy into a surface
    result = gpu_copy_raw_surface_data(data, width, height, channels);

    stbi_image_free(data);

    return result;
}

SDL_Surface* GPU_LoadSurface(const char* filename)
{
    return GPU_LoadSurface_RW(SDL_RWFromFile(filename, "r"), 1);
}

// From http://stackoverflow.com/questions/5309471/getting-file-extension-in-c
static const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename)
        return "";
    return dot + 1;
}

GPU_bool GPU_SaveSurface(SDL_Surface* surface, const char* filename, GPU_FileFormatEnum format)
{
    GPU_bool result;
    unsigned char* data;

    if(surface == NULL || filename == NULL ||
            surface->w < 1 || surface->h < 1)
    {
        return GPU_FALSE;
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
            return GPU_FALSE;
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
        result = GPU_FALSE;
        break;
    }

    return result;
}

static void write_func(void *context, void *data, int size)
{
    SDL_RWwrite((SDL_RWops*)context, data, 1, size);
}

GPU_bool GPU_SaveSurface_RW(SDL_Surface* surface, SDL_RWops* rwops, GPU_bool free_rwops, GPU_FileFormatEnum format)
{
    GPU_bool result;
    unsigned char* data;

    if(surface == NULL || rwops == NULL ||
            surface->w < 1 || surface->h < 1)
    {
        return GPU_FALSE;
    }

    data = surface->pixels;

    if(format == GPU_FILE_AUTO)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_DATA_ERROR, "Invalid output file format (GPU_FILE_AUTO)");
        return GPU_FALSE;
    }

    // FIXME: The limitations here are not communicated clearly.  BMP and TGA won't support arbitrary row length/pitch.
    switch(format)
    {
    case GPU_FILE_PNG:
        result = (stbi_write_png_to_func(write_func, rwops, surface->w, surface->h, surface->format->BytesPerPixel, (const unsigned char *const)data, surface->pitch) > 0);
        break;
    case GPU_FILE_BMP:
        result = (stbi_write_bmp_to_func(write_func, rwops, surface->w, surface->h, surface->format->BytesPerPixel, (const unsigned char *const)data) > 0);
        break;
    case GPU_FILE_TGA:
        result = (stbi_write_tga_to_func(write_func, rwops, surface->w, surface->h, surface->format->BytesPerPixel, (const unsigned char *const)data) > 0);
        break;
    default:
        GPU_PushErrorCode(__func__, GPU_ERROR_DATA_ERROR, "Unsupported output file format");
        result = GPU_FALSE;
        break;
    }

    if(result && free_rwops)
        SDL_RWclose(rwops);
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
	GPU_Target* result = GPU_GetTarget(image);
	
	if(result != NULL)
        result->refcount++;
    
    return result;
}


GPU_Target* GPU_GetTarget(GPU_Image* image)
{
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return NULL;

    return _gpu_current_renderer->impl->GetTarget(_gpu_current_renderer, image);
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


void GPU_BlitRotate(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees)
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

    _gpu_current_renderer->impl->BlitRotate(_gpu_current_renderer, image, src_rect, target, x, y, degrees);
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

void GPU_BlitTransform(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees, float scaleX, float scaleY)
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

    _gpu_current_renderer->impl->BlitTransform(_gpu_current_renderer, image, src_rect, target, x, y, degrees, scaleX, scaleY);
}

void GPU_BlitTransformX(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float pivot_x, float pivot_y, float degrees, float scaleX, float scaleY)
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

    _gpu_current_renderer->impl->BlitTransformX(_gpu_current_renderer, image, src_rect, target, x, y, pivot_x, pivot_y, degrees, scaleX, scaleY);
}

void GPU_BlitRect(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, GPU_Rect* dest_rect)
{
    float w = 0.0f;
    float h = 0.0f;
    
    if(image == NULL)
        return;
    
    if(src_rect == NULL)
    {
        w = image->w;
        h = image->h;
    }
    else
    {
        w = src_rect->w;
        h = src_rect->h;
    }
    
    GPU_BlitRectX(image, src_rect, target, dest_rect, 0.0f, w*0.5f, h*0.5f, GPU_FLIP_NONE);
}

void GPU_BlitRectX(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, GPU_Rect* dest_rect, float degrees, float pivot_x, float pivot_y, GPU_FlipEnum flip_direction)
{
    float w, h;
    float dx, dy;
    float dw, dh;
    float scale_x, scale_y;
    
    if(image == NULL || target == NULL)
        return;
    
    if(src_rect == NULL)
    {
        w = image->w;
        h = image->h;
    }
    else
    {
        w = src_rect->w;
        h = src_rect->h;
    }
    
    if(dest_rect == NULL)
    {
        dx = 0.0f;
        dy = 0.0f;
        dw = target->w;
        dh = target->h;
    }
    else
    {
        dx = dest_rect->x;
        dy = dest_rect->y;
        dw = dest_rect->w;
        dh = dest_rect->h;
    }
    
    scale_x = dw / w;
    scale_y = dh / h;
    
    if(flip_direction & GPU_FLIP_HORIZONTAL)
    {
        scale_x = -scale_x;
        dx += dw;
        pivot_x = w - pivot_x;
    }
    if(flip_direction & GPU_FLIP_VERTICAL)
    {
        scale_y = -scale_y;
        dy += dh;
        pivot_y = h - pivot_y;
    }
    
    GPU_BlitTransformX(image, src_rect, target, dx + pivot_x * scale_x, dy + pivot_y * scale_y, pivot_x, pivot_y, degrees, scale_x, scale_y);
}

void GPU_TriangleBatch(GPU_Image* image, GPU_Target* target, unsigned short num_vertices, float* values, unsigned int num_indices, unsigned short* indices, GPU_BatchFlagEnum flags)
{
    GPU_PrimitiveBatchV(image, target, GPU_TRIANGLES, num_vertices, (void*)values, num_indices, indices, flags);
}

void GPU_TriangleBatchX(GPU_Image* image, GPU_Target* target, unsigned short num_vertices, void* values, unsigned int num_indices, unsigned short* indices, GPU_BatchFlagEnum flags)
{
    GPU_PrimitiveBatchV(image, target, GPU_TRIANGLES, num_vertices, values, num_indices, indices, flags);
}

void GPU_PrimitiveBatch(GPU_Image* image, GPU_Target* target, GPU_PrimitiveEnum primitive_type, unsigned short num_vertices, float* values, unsigned int num_indices, unsigned short* indices, GPU_BatchFlagEnum flags)
{
    GPU_PrimitiveBatchV(image, target, primitive_type, num_vertices, (void*)values, num_indices, indices, flags);
}

void GPU_PrimitiveBatchV(GPU_Image* image, GPU_Target* target, GPU_PrimitiveEnum primitive_type, unsigned short num_vertices, void* values, unsigned int num_indices, unsigned short* indices, GPU_BatchFlagEnum flags)
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


    _gpu_current_renderer->impl->PrimitiveBatchV(_gpu_current_renderer, image, target, primitive_type, num_vertices, values, num_indices, indices, flags);
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

/* Adapted from SDL_IntersectRect() */
GPU_bool GPU_IntersectRect(GPU_Rect A, GPU_Rect B, GPU_Rect* result)
{
    GPU_bool has_horiz_intersection = GPU_FALSE;
    float Amin, Amax, Bmin, Bmax;
    GPU_Rect intersection;

    // Special case for empty rects
    if (A.w <= 0.0f || A.h <= 0.0f || B.w <= 0.0f || B.h <= 0.0f)
        return GPU_FALSE;

    // Horizontal intersection
    Amin = A.x;
    Amax = Amin + A.w;
    Bmin = B.x;
    Bmax = Bmin + B.w;
    if (Bmin > Amin)
        Amin = Bmin;
    if (Bmax < Amax)
        Amax = Bmax;
    
    intersection.x = Amin;
    intersection.w = Amax - Amin;
    
    has_horiz_intersection = (Amax > Amin);

    // Vertical intersection
    Amin = A.y;
    Amax = Amin + A.h;
    Bmin = B.y;
    Bmax = Bmin + B.h;
    if (Bmin > Amin)
        Amin = Bmin;
    if (Bmax < Amax)
        Amax = Bmax;
    
    intersection.y = Amin;
    intersection.h = Amax - Amin;
    
    if(has_horiz_intersection && Amax > Amin)
    {
        if(result != NULL)
            *result = intersection;
        return GPU_TRUE;
    }
    else
        return GPU_FALSE;
}


GPU_bool GPU_IntersectClipRect(GPU_Target* target, GPU_Rect B, GPU_Rect* result)
{
    if(target == NULL)
        return GPU_FALSE;
    
    if(!target->use_clip_rect)
    {
        GPU_Rect A = {0, 0, target->w, target->h};
        return GPU_IntersectRect(A, B, result);
    }
    
    return GPU_IntersectRect(target->clip_rect, B, result);
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

    target->use_color = GPU_FALSE;
    target->color = c;
}

GPU_bool GPU_GetBlending(GPU_Image* image)
{
    if(image == NULL)
        return GPU_FALSE;

    return image->use_blending;
}


void GPU_SetBlending(GPU_Image* image, GPU_bool enable)
{
    if(image == NULL)
        return;

    image->use_blending = enable;
}

void GPU_SetShapeBlending(GPU_bool enable)
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
    case GPU_BLEND_NORMAL_FACTOR_ALPHA:
    {
        GPU_BlendMode b = {GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_FUNC_ONE_MINUS_DST_ALPHA, GPU_FUNC_ONE, GPU_EQ_ADD, GPU_EQ_ADD};
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


void GPU_SetDefaultAnchor(float anchor_x, float anchor_y)
{
    if(_gpu_current_renderer == NULL)
        return;
    
    _gpu_current_renderer->default_image_anchor_x = anchor_x;
    _gpu_current_renderer->default_image_anchor_y = anchor_y;
}

void GPU_GetDefaultAnchor(float* anchor_x, float* anchor_y)
{
    if(_gpu_current_renderer == NULL)
        return;
    
    if(anchor_x != NULL)
        *anchor_x = _gpu_current_renderer->default_image_anchor_x;
    
    if(anchor_y != NULL)
        *anchor_y = _gpu_current_renderer->default_image_anchor_y;
}

void GPU_SetAnchor(GPU_Image* image, float anchor_x, float anchor_y)
{
    if(image == NULL)
        return;

    image->anchor_x = anchor_x;
    image->anchor_y = anchor_y;
}

void GPU_GetAnchor(GPU_Image* image, float* anchor_x, float* anchor_y)
{
    if(image == NULL)
        return;
    
    if(anchor_x != NULL)
        *anchor_x = image->anchor_x;
    
    if(anchor_y != NULL)
        *anchor_y = image->anchor_y;
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

GPU_TextureHandle GPU_GetTextureHandle(GPU_Image* image)
{
    if(image == NULL || image->renderer == NULL)
        return 0;
    return image->renderer->impl->GetTextureHandle(image->renderer, image);
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
    
    if(target != NULL && target->context == NULL)
    {
        _gpu_current_renderer->impl->FlushBlitBuffer(_gpu_current_renderer);
        return;
    }
    
    MAKE_CURRENT_IF_NONE(target);
    if(!CHECK_CONTEXT)
        RETURN_ERROR(GPU_ERROR_USER_ERROR, "NULL context");

    _gpu_current_renderer->impl->Flip(_gpu_current_renderer, target);
}





// Shader API


Uint32 GPU_CompileShader_RW(GPU_ShaderEnum shader_type, SDL_RWops* shader_source, GPU_bool free_rwops)
{
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
    {
        if(free_rwops)
            SDL_RWclose(shader_source);
        return GPU_FALSE;
    }

    return _gpu_current_renderer->impl->CompileShader_RW(_gpu_current_renderer, shader_type, shader_source, free_rwops);
}

Uint32 GPU_LoadShader(GPU_ShaderEnum shader_type, const char* filename)
{
    SDL_RWops* rwops;

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
    
    return GPU_CompileShader_RW(shader_type, rwops, 1);
}

Uint32 GPU_CompileShader(GPU_ShaderEnum shader_type, const char* shader_source)
{
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return 0;

    return _gpu_current_renderer->impl->CompileShader(_gpu_current_renderer, shader_type, shader_source);
}

GPU_bool GPU_LinkShaderProgram(Uint32 program_object)
{
    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return GPU_FALSE;

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
    Uint32 shaders[2];
    shaders[0] = shader_object1;
    shaders[1] = shader_object2;
    return GPU_LinkManyShaders(shaders, 2);
}

Uint32 GPU_LinkManyShaders(Uint32 *shader_objects, int count)
{
    Uint32 p;
    int i;

    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return 0;

    if((_gpu_current_renderer->enabled_features & GPU_FEATURE_BASIC_SHADERS) != GPU_FEATURE_BASIC_SHADERS)
        return 0;

    p = _gpu_current_renderer->impl->CreateShaderProgram(_gpu_current_renderer);

    for (i = 0; i < count; i++)
        _gpu_current_renderer->impl->AttachShader(_gpu_current_renderer, p, shader_objects[i]);

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

GPU_bool GPU_IsDefaultShaderProgram(Uint32 program_object)
{
    GPU_Context* context;

    if(_gpu_current_renderer == NULL || _gpu_current_renderer->current_context_target == NULL)
        return GPU_FALSE;

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

GPU_AttributeFormat GPU_MakeAttributeFormat(int num_elems_per_vertex, GPU_TypeEnum type, GPU_bool normalize, int stride_bytes, int offset_bytes)
{
    GPU_AttributeFormat f;
    f.is_per_sprite = GPU_FALSE;
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

    _gpu_current_renderer->current_context_target->context->current_shader_block = block;
}

GPU_ShaderBlock GPU_GetShaderBlock(void)
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

    return _gpu_current_renderer->current_context_target->context->current_shader_block;
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

void GPU_SetUniformMatrixfv(int location, int num_matrices, int num_rows, int num_columns, GPU_bool transpose, float* values)
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
static const unsigned char caseless_charmap[] =
{
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

    do
    {
        u1 = (unsigned char) *s1++;
        u2 = (unsigned char) *s2++;
        if (caseless_charmap[u1] != caseless_charmap[u2])
            return caseless_charmap[u1] - caseless_charmap[u2];
	} while (u1 != '\0');

    return 0;
}


#ifdef _MSC_VER
    #pragma warning(pop) 
#endif

