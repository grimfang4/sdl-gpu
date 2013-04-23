#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>

void printRenderers(void)
{
	const char* renderers[GPU_GetNumRegisteredRenderers()];
	GPU_GetRegisteredRendererList(renderers);
	
	printf("Available renderers:\n");
	int i;
	for(i = 0; i < GPU_GetNumRegisteredRenderers(); i++)
	{
		printf("%d) %s\n", i+1, renderers[i]);
	}
}

static inline const char* bool_string(Uint8 value)
{
    return (value? "true" : "false");
}

int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(NULL, 800, 600, 0);
	if(screen == NULL)
		return -1;
	
	GPU_LogError("Using renderer: %s\n", GPU_GetCurrentRendererID());
	
	GPU_LogError("Supports GPU_FEATURE_ALL_BASE: %s\n", bool_string(GPU_IsFeatureEnabled(GPU_FEATURE_ALL_BASE)));
	GPU_LogError("Supports GPU_FEATURE_ALL_BLEND_MODES: %s\n", bool_string(GPU_IsFeatureEnabled(GPU_FEATURE_ALL_BLEND_MODES)));
	GPU_LogError("Supports GPU_FEATURE_ALL_GL_FORMATS: %s\n", bool_string(GPU_IsFeatureEnabled(GPU_FEATURE_ALL_GL_FORMATS)));
	
	GPU_LogError("Supports GPU_FEATURE_NON_POWER_OF_TWO: %s\n", bool_string(GPU_IsFeatureEnabled(GPU_FEATURE_NON_POWER_OF_TWO)));
	GPU_LogError("Supports GPU_FEATURE_RENDER_TARGETS: %s\n", bool_string(GPU_IsFeatureEnabled(GPU_FEATURE_RENDER_TARGETS)));
	GPU_LogError("Supports GPU_FEATURE_BLEND_EQUATIONS: %s\n", bool_string(GPU_IsFeatureEnabled(GPU_FEATURE_BLEND_EQUATIONS)));
	GPU_LogError("Supports GPU_FEATURE_BLEND_FUNC_SEPARATE: %s\n", bool_string(GPU_IsFeatureEnabled(GPU_FEATURE_BLEND_FUNC_SEPARATE)));
	GPU_LogError("Supports GPU_FEATURE_GL_BGR: %s\n", bool_string(GPU_IsFeatureEnabled(GPU_FEATURE_GL_BGR)));
	GPU_LogError("Supports GPU_FEATURE_GL_BGRA: %s\n", bool_string(GPU_IsFeatureEnabled(GPU_FEATURE_GL_BGRA)));
	GPU_LogError("Supports GPU_FEATURE_GL_ABGR: %s\n", bool_string(GPU_IsFeatureEnabled(GPU_FEATURE_GL_ABGR)));
	
	GPU_Quit();
	
	return 0;
}


