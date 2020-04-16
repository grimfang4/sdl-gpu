#include "SDL_gpu.h"
#include "SDL_platform.h"
#include <string.h>

#ifndef _MSC_VER
	#include <strings.h>
#else
	#define __func__ __FUNCTION__
	// Disable warning: selection for inlining
	#pragma warning(disable: 4514 4711)
	// Disable warning: Spectre mitigation
	#pragma warning(disable: 5045)
#endif

#define GPU_MAX_ACTIVE_RENDERERS 20
#define GPU_MAX_REGISTERED_RENDERERS 10

void gpu_init_renderer_register(void);
void gpu_free_renderer_register(void);

typedef struct GPU_RendererRegistration
{
	GPU_RendererID id;
	GPU_Renderer* (*createFn)(GPU_RendererID request);
	void (*freeFn)(GPU_Renderer*);
} GPU_RendererRegistration;

static GPU_bool _gpu_renderer_register_is_initialized = GPU_FALSE;

static GPU_Renderer* _gpu_renderer_map[GPU_MAX_ACTIVE_RENDERERS];
static GPU_RendererRegistration _gpu_renderer_register[GPU_MAX_REGISTERED_RENDERERS];

static int _gpu_renderer_order_size = 0;
static GPU_RendererID _gpu_renderer_order[GPU_RENDERER_ORDER_MAX];






GPU_RendererEnum GPU_ReserveNextRendererEnum(void)
{
    static GPU_RendererEnum last_enum = GPU_RENDERER_CUSTOM_0;
    return last_enum++;
}

int GPU_GetNumActiveRenderers(void)
{
	int count;
	int i;

	gpu_init_renderer_register();

	count = 0;
	for(i = 0; i < GPU_MAX_ACTIVE_RENDERERS; i++)
	{
		if(_gpu_renderer_map[i] != NULL)
			count++;
	}
	return count;
}

void GPU_GetActiveRendererList(GPU_RendererID* renderers_array)
{
	int count;
	int i;

	gpu_init_renderer_register();

	count = 0;
	for(i = 0; i < GPU_MAX_ACTIVE_RENDERERS; i++)
	{
		if(_gpu_renderer_map[i] != NULL)
		{
			renderers_array[count] = _gpu_renderer_map[i]->id;
			count++;
		}
	}
}


int GPU_GetNumRegisteredRenderers(void)
{
	int count;
	int i;

	gpu_init_renderer_register();

	count = 0;
	for(i = 0; i < GPU_MAX_REGISTERED_RENDERERS; i++)
	{
		if(_gpu_renderer_register[i].id.renderer != GPU_RENDERER_UNKNOWN)
			count++;
	}
	return count;
}

void GPU_GetRegisteredRendererList(GPU_RendererID* renderers_array)
{
	int count;
	int i;

	gpu_init_renderer_register();

	count = 0;
	for(i = 0; i < GPU_MAX_REGISTERED_RENDERERS; i++)
	{
		if(_gpu_renderer_register[i].id.renderer != GPU_RENDERER_UNKNOWN)
		{
			renderers_array[count] = _gpu_renderer_register[i].id;
			count++;
		}
	}
}


GPU_RendererID GPU_GetRendererID(GPU_RendererEnum renderer)
{
	int i;

	gpu_init_renderer_register();

	for(i = 0; i < GPU_MAX_REGISTERED_RENDERERS; i++)
	{
		if(_gpu_renderer_register[i].id.renderer == renderer)
			return _gpu_renderer_register[i].id;
	}
	
	return GPU_MakeRendererID("Unknown", GPU_RENDERER_UNKNOWN, 0, 0);
}

GPU_Renderer* GPU_CreateRenderer_OpenGL_1_BASE(GPU_RendererID request);
void GPU_FreeRenderer_OpenGL_1_BASE(GPU_Renderer* renderer);
GPU_Renderer* GPU_CreateRenderer_OpenGL_1(GPU_RendererID request);
void GPU_FreeRenderer_OpenGL_1(GPU_Renderer* renderer);
GPU_Renderer* GPU_CreateRenderer_OpenGL_2(GPU_RendererID request);
void GPU_FreeRenderer_OpenGL_2(GPU_Renderer* renderer);
GPU_Renderer* GPU_CreateRenderer_OpenGL_3(GPU_RendererID request);
void GPU_FreeRenderer_OpenGL_3(GPU_Renderer* renderer);
GPU_Renderer* GPU_CreateRenderer_OpenGL_4(GPU_RendererID request);
void GPU_FreeRenderer_OpenGL_4(GPU_Renderer* renderer);
GPU_Renderer* GPU_CreateRenderer_GLES_1(GPU_RendererID request);
void GPU_FreeRenderer_GLES_1(GPU_Renderer* renderer);
GPU_Renderer* GPU_CreateRenderer_GLES_2(GPU_RendererID request);
void GPU_FreeRenderer_GLES_2(GPU_Renderer* renderer);
GPU_Renderer* GPU_CreateRenderer_GLES_3(GPU_RendererID request);
void GPU_FreeRenderer_GLES_3(GPU_Renderer* renderer);

void GPU_RegisterRenderer(GPU_RendererID id, GPU_Renderer* (*create_renderer)(GPU_RendererID request), void (*free_renderer)(GPU_Renderer* renderer))
{
    int i = GPU_GetNumRegisteredRenderers();
    
	if(i >= GPU_MAX_REGISTERED_RENDERERS)
		return;
    
    if(id.renderer == GPU_RENDERER_UNKNOWN)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_USER_ERROR, "Invalid renderer ID");
        return;
    }
    if(create_renderer == NULL)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_USER_ERROR, "NULL renderer create callback");
        return;
    }
    if(free_renderer == NULL)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_USER_ERROR, "NULL renderer free callback");
        return;
    }
    
    _gpu_renderer_register[i].id = id;
    _gpu_renderer_register[i].createFn = create_renderer;
    _gpu_renderer_register[i].freeFn = free_renderer;
}

void gpu_register_built_in_renderers(void)
{
	#ifndef SDL_GPU_DISABLE_OPENGL
        #ifndef SDL_GPU_DISABLE_OPENGL_1_BASE
        GPU_RegisterRenderer(GPU_MakeRendererID("OpenGL 1 BASE", GPU_RENDERER_OPENGL_1_BASE, 1, 1),
                             &GPU_CreateRenderer_OpenGL_1_BASE,
                             &GPU_FreeRenderer_OpenGL_1_BASE);
        #endif
        
        #ifndef SDL_GPU_DISABLE_OPENGL_1
        GPU_RegisterRenderer(GPU_MakeRendererID("OpenGL 1", GPU_RENDERER_OPENGL_1, 1, 1),
                             &GPU_CreateRenderer_OpenGL_1,
                             &GPU_FreeRenderer_OpenGL_1);
        #endif
	
        #ifndef SDL_GPU_DISABLE_OPENGL_2
            GPU_RegisterRenderer(GPU_MakeRendererID("OpenGL 2", GPU_RENDERER_OPENGL_2, 2, 0),
                                 &GPU_CreateRenderer_OpenGL_2,
                                 &GPU_FreeRenderer_OpenGL_2);
        #endif
	
        #ifndef SDL_GPU_DISABLE_OPENGL_3
            #ifdef __MACOSX__
            // Depending on OS X version, it might only support core GL 3.3 or 3.2
            GPU_RegisterRenderer(GPU_MakeRendererID("OpenGL 3", GPU_RENDERER_OPENGL_3, 3, 2),
                                 &GPU_CreateRenderer_OpenGL_3,
                                 &GPU_FreeRenderer_OpenGL_3);
            #else
            GPU_RegisterRenderer(GPU_MakeRendererID("OpenGL 3", GPU_RENDERER_OPENGL_3, 3, 0),
                                 &GPU_CreateRenderer_OpenGL_3,
                                 &GPU_FreeRenderer_OpenGL_3);
            #endif
        #endif
	
        #ifndef SDL_GPU_DISABLE_OPENGL_4
            #ifdef __MACOSX__
            GPU_RegisterRenderer(GPU_MakeRendererID("OpenGL 4", GPU_RENDERER_OPENGL_4, 4, 1),
                                 &GPU_CreateRenderer_OpenGL_4,
                                 &GPU_FreeRenderer_OpenGL_4);
            #else
            GPU_RegisterRenderer(GPU_MakeRendererID("OpenGL 4", GPU_RENDERER_OPENGL_4, 4, 0),
                                 &GPU_CreateRenderer_OpenGL_4,
                                 &GPU_FreeRenderer_OpenGL_4);
            #endif
        #endif
    #endif
	
	#ifndef SDL_GPU_DISABLE_GLES
        #ifndef SDL_GPU_DISABLE_GLES_1
        GPU_RegisterRenderer(GPU_MakeRendererID("OpenGLES 1", GPU_RENDERER_GLES_1, 1, 1),
                             &GPU_CreateRenderer_GLES_1,
                             &GPU_FreeRenderer_GLES_1);
        #endif
        #ifndef SDL_GPU_DISABLE_GLES_2
        GPU_RegisterRenderer(GPU_MakeRendererID("OpenGLES 2", GPU_RENDERER_GLES_2, 2, 0),
                             &GPU_CreateRenderer_GLES_2,
                             &GPU_FreeRenderer_GLES_2);
        #endif
        #ifndef SDL_GPU_DISABLE_GLES_3
        GPU_RegisterRenderer(GPU_MakeRendererID("OpenGLES 3", GPU_RENDERER_GLES_3, 3, 0),
                             &GPU_CreateRenderer_GLES_3,
                             &GPU_FreeRenderer_GLES_3);
        #endif
    #endif
	
}

void gpu_init_renderer_register(void)
{
	int i;

	if(_gpu_renderer_register_is_initialized)
		return;
	
	for(i = 0; i < GPU_MAX_REGISTERED_RENDERERS; i++)
	{
		_gpu_renderer_register[i].id.name = "Unknown";
		_gpu_renderer_register[i].id.renderer = GPU_RENDERER_UNKNOWN;
		_gpu_renderer_register[i].createFn = NULL;
		_gpu_renderer_register[i].freeFn = NULL;
	}
	for(i = 0; i < GPU_MAX_ACTIVE_RENDERERS; i++)
	{
		_gpu_renderer_map[i] = NULL;
	}
	
	GPU_GetDefaultRendererOrder(&_gpu_renderer_order_size, _gpu_renderer_order);
	
	_gpu_renderer_register_is_initialized = 1;
	
	gpu_register_built_in_renderers();
}

void gpu_free_renderer_register(void)
{
	int i;

	for(i = 0; i < GPU_MAX_REGISTERED_RENDERERS; i++)
	{
		_gpu_renderer_register[i].id.name = "Unknown";
		_gpu_renderer_register[i].id.renderer = GPU_RENDERER_UNKNOWN;
		_gpu_renderer_register[i].createFn = NULL;
		_gpu_renderer_register[i].freeFn = NULL;
	}
	for(i = 0; i < GPU_MAX_ACTIVE_RENDERERS; i++)
	{
		_gpu_renderer_map[i] = NULL;
	}
	
	_gpu_renderer_register_is_initialized = 0;
	_gpu_renderer_order_size = 0;
}


void GPU_GetRendererOrder(int* order_size, GPU_RendererID* order)
{
    if(order_size != NULL)
        *order_size = _gpu_renderer_order_size;
    
    if(order != NULL && _gpu_renderer_order_size > 0)
        memcpy(order, _gpu_renderer_order, _gpu_renderer_order_size*sizeof(GPU_RendererID));
}

void GPU_SetRendererOrder(int order_size, GPU_RendererID* order)
{
    if(order == NULL)
    {
        // Restore the default order
        int count = 0;
        GPU_RendererID default_order[GPU_RENDERER_ORDER_MAX];
        GPU_GetDefaultRendererOrder(&count, default_order);
        GPU_SetRendererOrder(count, default_order);  // Call us again with the default order
        return;
    }
    
    if(order_size <= 0)
        return;
    
    if(order_size > GPU_RENDERER_ORDER_MAX)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_USER_ERROR, "Given order_size (%d) is greater than GPU_RENDERER_ORDER_MAX (%d)", order_size, GPU_RENDERER_ORDER_MAX);
        order_size = GPU_RENDERER_ORDER_MAX;
    }
    
    memcpy(_gpu_renderer_order, order, order_size*sizeof(GPU_RendererID));
    _gpu_renderer_order_size = order_size;
}



void GPU_GetDefaultRendererOrder(int* order_size, GPU_RendererID* order)
{
    int count = 0;
    GPU_RendererID default_order[GPU_RENDERER_ORDER_MAX];
    
    #ifndef SDL_GPU_DISABLE_GLES
        #ifndef SDL_GPU_DISABLE_GLES_3
            default_order[count++] = GPU_MakeRendererID("OpenGLES 3", GPU_RENDERER_GLES_3, 3, 0);
        #endif
        #ifndef SDL_GPU_DISABLE_GLES_2
            default_order[count++] = GPU_MakeRendererID("OpenGLES 2", GPU_RENDERER_GLES_2, 2, 0);
        #endif
        #ifndef SDL_GPU_DISABLE_GLES_1
            default_order[count++] = GPU_MakeRendererID("OpenGLES 1", GPU_RENDERER_GLES_1, 1, 1);
        #endif
    #endif
    
    #ifndef SDL_GPU_DISABLE_OPENGL
        #ifdef __MACOSX__
        
            // My understanding of OS X OpenGL support:
            // OS X 10.9: GL 2.1, 3.3, 4.1
            // OS X 10.7: GL 2.1, 3.2
            // OS X 10.6: GL 1.4, 2.1
            #ifndef SDL_GPU_DISABLE_OPENGL_4
            default_order[count++] = GPU_MakeRendererID("OpenGL 4", GPU_RENDERER_OPENGL_4, 4, 1);
            #endif
            #ifndef SDL_GPU_DISABLE_OPENGL_3
            default_order[count++] = GPU_MakeRendererID("OpenGL 3", GPU_RENDERER_OPENGL_3, 3, 2);
            #endif
        
        #else
        
            #ifndef SDL_GPU_DISABLE_OPENGL_4
            default_order[count++] = GPU_MakeRendererID("OpenGL 4", GPU_RENDERER_OPENGL_4, 4, 0);
            #endif
            #ifndef SDL_GPU_DISABLE_OPENGL_3
            default_order[count++] = GPU_MakeRendererID("OpenGL 3", GPU_RENDERER_OPENGL_3, 3, 0);
            #endif
            
        #endif
        
        #ifndef SDL_GPU_DISABLE_OPENGL_2
        default_order[count++] = GPU_MakeRendererID("OpenGL 2", GPU_RENDERER_OPENGL_2, 2, 0);
        #endif
        #ifndef SDL_GPU_DISABLE_OPENGL_1
        default_order[count++] = GPU_MakeRendererID("OpenGL 1", GPU_RENDERER_OPENGL_1, 1, 1);
        #endif
        
    #endif
    
    if(order_size != NULL)
        *order_size = count;
    
    if(order != NULL && count > 0)
        memcpy(order, default_order, count*sizeof(GPU_RendererID));
}


GPU_Renderer* GPU_CreateRenderer(GPU_RendererID id)
{
	GPU_Renderer* result = NULL;
	int i;
	for(i = 0; i < GPU_MAX_REGISTERED_RENDERERS; i++)
	{
		if(_gpu_renderer_register[i].id.renderer == GPU_RENDERER_UNKNOWN)
			continue;
		
		if(id.renderer == _gpu_renderer_register[i].id.renderer)
		{
			if(_gpu_renderer_register[i].createFn != NULL)
            {
                // Use the registered name
                id.name = _gpu_renderer_register[i].id.name;
				result = _gpu_renderer_register[i].createFn(id);
            }
			break;
		}
	}
	
	if(result == NULL)
    {
        GPU_PushErrorCode(__func__, GPU_ERROR_DATA_ERROR, "Renderer was not found in the renderer registry.");
    }
	return result;
}

// Get a renderer from the map.
GPU_Renderer* GPU_GetRenderer(GPU_RendererID id)
{
	int i;
	gpu_init_renderer_register();
	
	// Invalid enum?
	if(id.renderer == GPU_RENDERER_UNKNOWN)
        return NULL;
	
	for(i = 0; i < GPU_MAX_ACTIVE_RENDERERS; i++)
	{
		if(_gpu_renderer_map[i] == NULL)
			continue;
		
		if(id.renderer == _gpu_renderer_map[i]->id.renderer)
			return _gpu_renderer_map[i];
	}
    
    return NULL;
}

// Create a new renderer based on a registered id and store it in the map.
GPU_Renderer* gpu_create_and_add_renderer(GPU_RendererID id)
{
	int i;
	for(i = 0; i < GPU_MAX_ACTIVE_RENDERERS; i++)
	{
		if(_gpu_renderer_map[i] == NULL)
		{
			// Create
			GPU_Renderer* renderer = GPU_CreateRenderer(id);
			if(renderer == NULL)
            {
                GPU_PushErrorCode(__func__, GPU_ERROR_BACKEND_ERROR, "Failed to create new renderer.");
                return NULL;
            }
            
			// Add
			_gpu_renderer_map[i] = renderer;
			// Return
			return renderer;
		}
	}
	
    GPU_PushErrorCode(__func__, GPU_ERROR_BACKEND_ERROR, "Couldn't create a new renderer.  Too many active renderers for GPU_MAX_ACTIVE_RENDERERS (%d).", GPU_MAX_ACTIVE_RENDERERS);
	return NULL;
}

// Free renderer memory according to how the registry instructs
void gpu_free_renderer_memory(GPU_Renderer* renderer)
{
	int i;
	if(renderer == NULL)
        return;
	
	for(i = 0; i < GPU_MAX_REGISTERED_RENDERERS; i++)
	{
		if(_gpu_renderer_register[i].id.renderer == GPU_RENDERER_UNKNOWN)
			continue;
		
		if(renderer->id.renderer == _gpu_renderer_register[i].id.renderer)
		{
			_gpu_renderer_register[i].freeFn(renderer);
			return;
		}
	}
}

// Remove a renderer from the active map and free it.
void GPU_FreeRenderer(GPU_Renderer* renderer)
{
	int i;
	GPU_Renderer* current_renderer;
	
	if(renderer == NULL)
        return;
	
    current_renderer = GPU_GetCurrentRenderer();
    if(current_renderer == renderer)
        GPU_SetCurrentRenderer(GPU_MakeRendererID("Unknown", GPU_RENDERER_UNKNOWN, 0, 0));
        
	for(i = 0; i < GPU_MAX_ACTIVE_RENDERERS; i++)
	{
		if(renderer == _gpu_renderer_map[i])
		{
			gpu_free_renderer_memory(renderer);
			_gpu_renderer_map[i] = NULL;
			return;
		}
	}
}
