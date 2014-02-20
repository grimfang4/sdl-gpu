#include "SDL_gpu.h"

void printRenderers(void)
{
    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);
    
    SDL_version compiled = GPU_GetCompiledVersion();
    SDL_version linked = GPU_GetLinkedVersion();
    if(compiled.major != linked.major || compiled.minor != linked.minor || compiled.patch != linked.patch)
        GPU_Log("SDL_gpu v%d.%d.%d (compiled with v%d.%d.%d)\n", linked.major, linked.minor, linked.patch, compiled.major, compiled.minor, compiled.patch);
    else
        GPU_Log("SDL_gpu v%d.%d.%d\n", linked.major, linked.minor, linked.patch);
    
	GPU_RendererID renderers[GPU_GetNumRegisteredRenderers()];
	GPU_GetRegisteredRendererList(renderers);
	
	GPU_Log("\nAvailable renderers:\n");
	int i;
	for(i = 0; i < GPU_GetNumRegisteredRenderers(); i++)
	{
		GPU_Log("* %s (%d.%d)\n", GPU_GetRendererEnumString(renderers[i].id), renderers[i].major_version, renderers[i].minor_version);
	}
	GPU_Log("Renderer init order:\n");
	
	int order_size;
	GPU_RendererID order[GPU_RENDERER_ORDER_MAX];
	GPU_GetRendererOrder(&order_size, order);
	for(i = 0; i < order_size; i++)
	{
		GPU_Log("%d) %s (%d.%d)\n", i+1, GPU_GetRendererEnumString(order[i].id), order[i].major_version, order[i].minor_version);
	}
	GPU_Log("\n");
}

void printCurrentRenderer(void)
{
    GPU_RendererID id = GPU_GetCurrentRenderer()->id;
    const char* renderer_string = GPU_GetRendererEnumString(id.id);
    
	GPU_Log("Using renderer: %s (%d.%d)\n\n", renderer_string, id.major_version, id.minor_version);
}
