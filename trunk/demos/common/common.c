#include "SDL_gpu.h"

void printRenderers(void)
{
    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);
    
	GPU_RendererID renderers[GPU_GetNumRegisteredRenderers()];
	GPU_GetRegisteredRendererList(renderers);
	
	printf("Available renderers:\n");
	int i;
	for(i = 0; i < GPU_GetNumRegisteredRenderers(); i++)
	{
		printf("* %s (%d.%d)\n", GPU_GetRendererEnumString(renderers[i].id), renderers[i].major_version, renderers[i].minor_version);
	}
	printf("Renderer order:\n");
	
	int order_size;
	GPU_RendererID order[GPU_RENDERER_ORDER_MAX];
	GPU_GetRendererOrder(&order_size, order);
	for(i = 0; i < order_size; i++)
	{
		printf("%d) %s (%d.%d)\n", i+1, GPU_GetRendererEnumString(order[i].id), order[i].major_version, order[i].minor_version);
	}
}

void printCurrentRenderer(void)
{
    GPU_RendererID id = GPU_GetCurrentRenderer()->id;
    const char* renderer_string = GPU_GetRendererEnumString(id.id);
    
	printf("Using renderer: %s (%d.%d)\n", renderer_string, id.major_version, id.minor_version);
}
