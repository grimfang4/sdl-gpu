#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "common.h"

static inline const char* bool_string(Uint8 value)
{
    return (value? "true" : "false");
}

int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
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
	
	GPU_LogError("Supports GPU_FEATURE_VERTEX_SHADER: %s\n", bool_string(GPU_IsFeatureEnabled(GPU_FEATURE_VERTEX_SHADER)));
	GPU_LogError("Supports GPU_FEATURE_FRAGMENT_SHADER: %s\n", bool_string(GPU_IsFeatureEnabled(GPU_FEATURE_FRAGMENT_SHADER)));
	GPU_LogError("Supports GPU_FEATURE_GEOMETRY_SHADER: %s\n", bool_string(GPU_IsFeatureEnabled(GPU_FEATURE_GEOMETRY_SHADER)));
	
	GPU_LogError("Supports GPU_FEATURE_WRAP_REPEAT_MIRRORED: %s\n", bool_string(GPU_IsFeatureEnabled(GPU_FEATURE_WRAP_REPEAT_MIRRORED)));
	
	GPU_Quit();
	
	return 0;
}


