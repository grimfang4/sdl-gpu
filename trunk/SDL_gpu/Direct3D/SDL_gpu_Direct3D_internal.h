#ifndef _SDL_GPU_DIRECT3D_INTERNAL_H__
#define _SDL_GPU_DIRECT3D_INTERNAL_H__

#include "SDL_gpu_Direct3D.h"


GPU_Renderer* GPU_CreateRenderer_Direct3D(void);
void GPU_FreeRenderer_Direct3D(GPU_Renderer* renderer);

GPU_ShapeRenderer* GPU_CreateShapeRenderer_Direct3D(void);
void GPU_FreeShapeRenderer_Direct3D(GPU_ShapeRenderer* renderer);



#endif
