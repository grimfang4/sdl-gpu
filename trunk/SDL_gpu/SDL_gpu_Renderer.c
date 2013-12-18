#include "SDL_gpu.h"
#include <string.h>
#include <strings.h>
int strcasecmp(const char*, const char *);

#define MAX_ACTIVE_RENDERERS 20
#define MAX_REGISTERED_RENDERERS 2

// TODO: Add list of initialized renderers that need to be cleaned up at GPU_Quit().
// TODO: Add map<const char*, GPU_Renderer*> to hold all registered (potential) renderers.

typedef struct RendererRegistration
{
	GPU_RendererID id;
	GPU_Renderer* (*createFn)(GPU_RendererID request);
	void (*freeFn)(GPU_Renderer*);
} RendererRegistration;

static Uint8 initialized = 0;
// FIXME: This is a temporary holder in lieu of a map/vector.
static GPU_Renderer* rendererMap[MAX_ACTIVE_RENDERERS];
static RendererRegistration rendererRegister[MAX_REGISTERED_RENDERERS];


void GPU_InitRendererRegister(void);

int GPU_GetNumActiveRenderers(void)
{
	GPU_InitRendererRegister();

	int count = 0;
	int i;
	for(i = 0; i < MAX_ACTIVE_RENDERERS; i++)
	{
		if(rendererMap[i] != NULL)
			count++;
	}
	return count;
}

void GPU_GetActiveRendererList(GPU_RendererID* renderers_array)
{
	GPU_InitRendererRegister();

	int count = 0;
	
	int i;
	for(i = 0; i < MAX_ACTIVE_RENDERERS; i++)
	{
		if(rendererMap[i] != NULL)
		{
			renderers_array[count] = rendererMap[i]->id;
			count++;
		}
	}
}


int GPU_GetNumRegisteredRenderers(void)
{
	GPU_InitRendererRegister();

	int count = 0;
	int i;
	for(i = 0; i < MAX_REGISTERED_RENDERERS; i++)
	{
		if(rendererRegister[i].id.id != GPU_RENDERER_UNKNOWN)
			count++;
	}
	return count;
}

void GPU_GetRegisteredRendererList(GPU_RendererID* renderers_array)
{
	GPU_InitRendererRegister();

	int count = 0;
	
	int i;
	for(i = 0; i < MAX_REGISTERED_RENDERERS; i++)
	{
		if(rendererRegister[i].id.id != GPU_RENDERER_UNKNOWN)
		{
			renderers_array[count] = rendererRegister[i].id;
			count++;
		}
	}
}


GPU_RendererID GPU_GetRendererID(unsigned int index)
{
	if(index >= MAX_REGISTERED_RENDERERS)
		return GPU_MakeRendererID(GPU_RENDERER_UNKNOWN, 0, 0, -1);
	
	return rendererRegister[index].id;
}

GPU_Renderer* GPU_CreateRenderer_OpenGL_1(GPU_RendererID request);
void GPU_FreeRenderer_OpenGL_1(GPU_Renderer* renderer);
GPU_Renderer* GPU_CreateRenderer_OpenGL_2(GPU_RendererID request);
void GPU_FreeRenderer_OpenGL_2(GPU_Renderer* renderer);
GPU_Renderer* GPU_CreateRenderer_GLES_1(GPU_RendererID request);
void GPU_FreeRenderer_GLES_1(GPU_Renderer* renderer);

void GPU_RegisterRenderers()
{
	int i = 0;
	
	if(i >= MAX_REGISTERED_RENDERERS)
		return;
	
	#ifndef SDL_GPU_DISABLE_OPENGL
        #ifndef SDL_GPU_DISABLE_OPENGL_1
        rendererRegister[i].id = GPU_MakeRendererID(GPU_RENDERER_OPENGL_1, 1, 1, i);
        rendererRegister[i].createFn = &GPU_CreateRenderer_OpenGL_1;
        rendererRegister[i].freeFn = &GPU_FreeRenderer_OpenGL_1;
        
        i++;
        if(i >= MAX_REGISTERED_RENDERERS)
            return;
        #endif
	
        #ifndef SDL_GPU_DISABLE_OPENGL_2
        rendererRegister[i].id = GPU_MakeRendererID(GPU_RENDERER_OPENGL_2, 2, 0, i);
        rendererRegister[i].createFn = &GPU_CreateRenderer_OpenGL_2;
        rendererRegister[i].freeFn = &GPU_FreeRenderer_OpenGL_2;
        
        i++;
        if(i >= MAX_REGISTERED_RENDERERS)
            return;
        #endif
    #endif
	
	#ifndef SDL_GPU_DISABLE_GLES
        #ifndef SDL_GPU_DISABLE_GLES_1
        rendererRegister[i].id = GPU_MakeRendererID(GPU_RENDERER_GLES_1, 1, 1, i);
        rendererRegister[i].createFn = &GPU_CreateRenderer_GLES_1;
        rendererRegister[i].freeFn = &GPU_FreeRenderer_GLES_1;
        
        i++;
        if(i >= MAX_REGISTERED_RENDERERS)
            return;
        #endif
    #endif
	
}


void GPU_InitRendererRegister(void)
{
	if(initialized)
		return;
	
	int i;
	for(i = 0; i < MAX_REGISTERED_RENDERERS; i++)
	{
		rendererRegister[i].id.id = GPU_RENDERER_UNKNOWN;
		rendererRegister[i].id.index = i;
		rendererRegister[i].createFn = NULL;
		rendererRegister[i].freeFn = NULL;
	}
	for(i = 0; i < MAX_ACTIVE_RENDERERS; i++)
	{
		rendererMap[i] = NULL;
	}
	
	initialized = 1;
	
	GPU_RegisterRenderers();
}





GPU_RendererID GPU_GetDefaultRendererID(void)
{
    
    #ifdef ANDROID
        #ifdef SDL_GPU_PREFER_GLES_2
        return GPU_MakeRendererIDRequest(GPU_RENDERER_OPENGLES_2, 2, 0, 0);
        #else
        return GPU_MakeRendererIDRequest(GPU_RENDERER_OPENGLES_1, 1, 1, 0);
        #endif
    #else
    return GPU_MakeRendererIDRequest(GPU_RENDERER_OPENGL_1, 1, 1, 0);
    #endif
}

const char* GPU_GetRendererEnumString(GPU_RendererEnum id)
{
    if(id == GPU_RENDERER_DEFAULT)
        id = GPU_GetDefaultRendererID().id;
    
    if(id == GPU_RENDERER_OPENGL_1)
        return "OpenGL 1.x";
    if(id == GPU_RENDERER_OPENGL_2)
        return "OpenGL 2.x";
    if(id == GPU_RENDERER_OPENGL_3)
        return "OpenGL 3.x";
    if(id == GPU_RENDERER_OPENGL_4)
        return "OpenGL 4.x";
    if(id == GPU_RENDERER_GLES_1)
        return "OpenGLES 1.x";
    if(id == GPU_RENDERER_GLES_2)
        return "OpenGLES 2.x";
    if(id == GPU_RENDERER_GLES_3)
        return "OpenGLES 3.x";
    if(id == GPU_RENDERER_D3D9)
        return "Direct3D 9";
    
    return "Unknown";
}


GPU_Renderer* GPU_CreateRenderer(GPU_RendererID id)
{
	GPU_Renderer* result = NULL;
	int i;
	for(i = 0; i < MAX_REGISTERED_RENDERERS; i++)
	{
		if(rendererRegister[i].id.id == GPU_RENDERER_UNKNOWN)
			continue;
		
		if(id.id == rendererRegister[i].id.id)
		{
			if(rendererRegister[i].createFn != NULL)
            {
				result = rendererRegister[i].createFn(id);
            }
			break;
		}
	}
	
	if(result == NULL)
		GPU_LogError("Could not create renderer: \"%s %d.%d\" was not found in the renderer registry.\n", GPU_GetRendererEnumString(id.id), id.major_version, id.minor_version);
	return result;
}


GPU_Renderer* GPU_GetRenderer(unsigned int index)
{
	if(index >= MAX_ACTIVE_RENDERERS)
		return NULL;
	
	return rendererMap[index];
}

// Get a renderer from the map.
GPU_Renderer* GPU_GetRendererByID(GPU_RendererID id)
{
	GPU_InitRendererRegister();
	
	if(id.index < 0)
		return NULL;
    
    return GPU_GetRenderer(id.index);
}

// Create a new renderer based on a registered id and store it in the map.
GPU_Renderer* GPU_AddRenderer(GPU_RendererID id)
{
    if(id.id == GPU_RENDERER_DEFAULT)
        id = GPU_GetDefaultRendererID();
    
	int i;
	for(i = 0; i < MAX_ACTIVE_RENDERERS; i++)
	{
		if(rendererMap[i] == NULL)
		{
			// Create
			GPU_Renderer* renderer = GPU_CreateRenderer(id);
			if(renderer == NULL)
            {
                // TODO: Add more info here
                GPU_LogError("Failed to create new renderer.\n");
                return NULL;
            }
            
			// Add
			rendererMap[i] = renderer;
			renderer->id.index = i;
			// Return
			return renderer;
		}
	}
	
	return NULL;
}

void GPU_FreeRenderer(GPU_Renderer* renderer)
{
	int i;
	for(i = 0; i < MAX_REGISTERED_RENDERERS; i++)
	{
		if(rendererRegister[i].id.id == GPU_RENDERER_UNKNOWN)
			continue;
		
		if(renderer->id.id == rendererRegister[i].id.id)
		{
			rendererRegister[i].freeFn(renderer);
			return;
		}
	}
}

// Remove a renderer from the map and free it.
void GPU_RemoveRenderer(GPU_RendererID id)
{
	int i;
	for(i = 0; i < MAX_ACTIVE_RENDERERS; i++)
	{
		if(rendererMap[i] == NULL)
			continue;
		
		if(i == id.index && id.id == rendererMap[i]->id.id)
		{
			GPU_FreeRenderer(rendererMap[i]);
			rendererMap[i] = NULL;
			return;
		}
	}
}
