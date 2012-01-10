#include "SDL_gpu_Renderer.h"
#include <string.h>

#include "../OpenGL/SDL_gpu_OpenGL_internal.h"
#define MAX_ACTIVE_RENDERERS 10
#define MAX_REGISTERED_RENDERERS 1

// TODO: Add list of initialized renderers that need to be cleaned up at GPU_Quit().
// TODO: Add map<const char*, GPU_Renderer*> to hold all registered (potential) renderers.

typedef struct RendererRegistration
{
	char* id;
	GPU_Renderer* (*createFn)(void);
	void (*freeFn)(GPU_Renderer*);
} RendererRegistration;

static Uint8 initialized = 0;
// FIXME: This is a temporary holder in lieu of a map.
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

void GPU_GetActiveRendererList(const char** renderers_array)
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
		if(rendererRegister[i].id != NULL)
			count++;
	}
	return count;
}

void GPU_GetRegisteredRendererList(const char** renderers_array)
{
	GPU_InitRendererRegister();

	int count = 0;
	
	int i;
	for(i = 0; i < MAX_REGISTERED_RENDERERS; i++)
	{
		if(rendererRegister[i].id != NULL)
		{
			renderers_array[count] = rendererRegister[i].id;
			count++;
		}
	}
}


const char* GPU_GetDefaultRendererID(void)
{
	return "OpenGL";
}

void GPU_RegisterRenderers()
{
	int i = 0;
	
	if(i >= MAX_REGISTERED_RENDERERS)
		return;
	
	const char* id = "OpenGL";
	rendererRegister[i].id = (char*)malloc(strlen(id) + 1);
	strcpy(rendererRegister[i].id, "OpenGL");
	rendererRegister[i].createFn = &GPU_CreateRenderer_OpenGL;
	rendererRegister[i].freeFn = &GPU_FreeRenderer_OpenGL;
	i++;
	
	if(i >= MAX_REGISTERED_RENDERERS)
		return;
}


void GPU_InitRendererRegister(void)
{
	if(initialized)
		return;
	
	int i;
	for(i = 0; i < MAX_REGISTERED_RENDERERS; i++)
	{
		rendererRegister[i].id = NULL;
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

GPU_Renderer* GPU_CreateRenderer(const char* id)
{
	GPU_Renderer* result = NULL;
	int i;
	for(i = 0; i < MAX_REGISTERED_RENDERERS; i++)
	{
		if(rendererRegister[i].id == NULL)
			continue;
		
		if(strcmp(id, rendererRegister[i].id) == 0)
		{
			result = GPU_CreateRenderer_OpenGL();
			break;
		}
	}
	
	// TODO: Add an error here: If NULL, we don't understand the id.
	return result;
}



// Get a renderer from the map.
GPU_Renderer* GPU_GetRendererByID(const char* id)
{
	GPU_InitRendererRegister();
	
	if(id == NULL)
		return NULL;
	
	int i;
	for(i = 0; i < MAX_ACTIVE_RENDERERS; i++)
	{
		if(rendererMap[i] == NULL)
			continue;
		
		if(strcmp(id, rendererMap[i]->id) == 0)
		{
			return rendererMap[i];
		}
	}
	
	return NULL;
}

// Create a new renderer based on a registered id and store it in the map.
GPU_Renderer* GPU_AddRenderer(const char* id)
{
	int i;
	for(i = 0; i < MAX_ACTIVE_RENDERERS; i++)
	{
		if(rendererMap[i] == NULL)
		{
			// Create
			GPU_Renderer* renderer = GPU_CreateRenderer(id);
			// Add
			rendererMap[i] = renderer;
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
		if(rendererRegister[i].id == NULL)
			continue;
		
		if(strcmp(renderer->id, rendererRegister[i].id) == 0)
		{
			rendererRegister[i].freeFn(renderer);
			return;
		}
	}
}

// Remove a renderer from the map and free it.
void GPU_RemoveRenderer(const char* id)
{
	int i;
	for(i = 0; i < MAX_ACTIVE_RENDERERS; i++)
	{
		if(rendererMap[i] == NULL)
			continue;
		
		if(strcmp(id, rendererMap[i]->id) == 0)
		{
			GPU_FreeRenderer(rendererMap[i]);
			rendererMap[i] = NULL;
			return;
		}
	}
}