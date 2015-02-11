#include "common.h"
#include <string.h>

void printRenderers(void)
{
	SDL_version compiled;
	SDL_version linked;
	GPU_RendererID* renderers;
	int i;
	int order_size;
	GPU_RendererID order[GPU_RENDERER_ORDER_MAX];

    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);
    
    compiled = GPU_GetCompiledVersion();
    linked = GPU_GetLinkedVersion();
    if(compiled.major != linked.major || compiled.minor != linked.minor || compiled.patch != linked.patch)
        GPU_Log("SDL_gpu v%d.%d.%d (compiled with v%d.%d.%d)\n", linked.major, linked.minor, linked.patch, compiled.major, compiled.minor, compiled.patch);
    else
        GPU_Log("SDL_gpu v%d.%d.%d\n", linked.major, linked.minor, linked.patch);
    
	renderers = (GPU_RendererID*)malloc(sizeof(GPU_RendererID)*GPU_GetNumRegisteredRenderers());
	GPU_GetRegisteredRendererList(renderers);
	
	GPU_Log("\nAvailable renderers:\n");
	for(i = 0; i < GPU_GetNumRegisteredRenderers(); i++)
	{
		GPU_Log("* %s (%d.%d)\n", renderers[i].name, renderers[i].major_version, renderers[i].minor_version);
	}
	GPU_Log("Renderer init order:\n");
	
	GPU_GetRendererOrder(&order_size, order);
	for(i = 0; i < order_size; i++)
	{
		GPU_Log("%d) %s (%d.%d)\n", i+1, order[i].name, order[i].major_version, order[i].minor_version);
	}
	GPU_Log("\n");

	free(renderers);
}

void printCurrentRenderer(void)
{
    GPU_RendererID id = GPU_GetCurrentRenderer()->id;
    
	GPU_Log("Using renderer: %s (%d.%d)\n\n", id.name, id.major_version, id.minor_version);
}

GPU_Target* initialize_demo(int argc, char** argv, Uint16 w, Uint16 h)
{
    GPU_Target* screen;
	printRenderers();
	
	GPU_RendererEnum renderer = GPU_RENDERER_UNKNOWN;
	
	// Parse command line args
    {
        for(int i = 1; i < argc; i++)
        {
            char* s = argv[i];
            
            if(s[0] == '-')
            {
                if(strcmp(s, "-r") == 0 || strcmp(s, "--renderer") == 0)
                {
                    i++;
                    if(i >= argc)
                        break;
                    
                    s = argv[i];
                    if(SDL_strcasecmp(s, "BASE") == 0 || SDL_strcasecmp(s, "OpenGL_BASE") == 0 || SDL_strcasecmp(s, "OpenGL_1_BASE") == 0)
                        renderer = GPU_RENDERER_OPENGL_1_BASE;
                    else if(SDL_strcasecmp(s, "OpenGL_1") == 0)
                        renderer = GPU_RENDERER_OPENGL_1;
                    else if(SDL_strcasecmp(s, "OpenGL_2") == 0)
                        renderer = GPU_RENDERER_OPENGL_2;
                    else if(SDL_strcasecmp(s, "OpenGL_3") == 0)
                        renderer = GPU_RENDERER_OPENGL_3;
                    else if(SDL_strcasecmp(s, "OpenGL_4") == 0)
                        renderer = GPU_RENDERER_OPENGL_4;
                }
            }
        }
    }
	
    if(renderer == GPU_RENDERER_UNKNOWN)
        screen = GPU_Init(w, h, GPU_DEFAULT_INIT_FLAGS);
    else
        screen = GPU_InitRenderer(renderer, w, h, GPU_DEFAULT_INIT_FLAGS);
	
	if(screen == NULL)
		return NULL;
	
	printCurrentRenderer();
	return screen;
}
