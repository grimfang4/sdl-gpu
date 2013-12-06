#include "SDL_gpu.h"

void printRenderers(void)
{
	GPU_RendererID renderers[GPU_GetNumRegisteredRenderers()];
	GPU_GetRegisteredRendererList(renderers);
	
	printf("Available renderers:\n");
	int i;
	for(i = 0; i < GPU_GetNumRegisteredRenderers(); i++)
	{
		printf("%d) %s\n", i+1, GPU_GetRendererEnumString(renderers[i].id));
	}
}

void printCurrentRenderer(void)
{
    GPU_RendererID id = GPU_GetCurrentRendererID();
    const char* renderer_string = GPU_GetRendererEnumString(id.id);
    
	printf("Using renderer: %s %d.%d\n", renderer_string, id.major_version, id.minor_version);
}
