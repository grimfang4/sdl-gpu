#ifndef _COMMON_H__
#define _COMMON_H__

#include "SDL_gpu.h"
#include <stdlib.h>
#include <string.h>

void printRenderers(void);
void printCurrentRenderer(void);
GPU_Target* initialize_demo(int argc, char** argv, Uint16 w, Uint16 h);
Uint32 load_shader(GPU_ShaderEnum shader_type, const char* filename);

#endif
