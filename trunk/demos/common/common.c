#include "SDL_gpu.h"

void printRenderers(void)
{
	GPU_RendererID renderers[GPU_GetNumRegisteredRenderers()];
	GPU_GetRegisteredRendererList(renderers);
	
	printf("Available renderers:\n");
	int i;
	for(i = 0; i < GPU_GetNumRegisteredRenderers(); i++)
	{
		printf("%d) %s (%d.%d)\n", i+1, GPU_GetRendererEnumString(renderers[i].id), renderers[i].major_version, renderers[i].minor_version);
	}
}

void printCurrentRenderer(void)
{
    GPU_RendererID id = GPU_GetCurrentRenderer()->id;
    const char* renderer_string = GPU_GetRendererEnumString(id.id);
    
	printf("Using renderer: %s (%d.%d)\n", renderer_string, id.major_version, id.minor_version);
}
