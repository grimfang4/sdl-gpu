#ifndef _COMMON_H__
#define _COMMON_H__

#include "SDL_gpu.h"

void printRenderers(void);
void printCurrentRenderer(void);
GPU_Target* initialize_demo(int argc, char** argv, Uint16 w, Uint16 h);

#endif
