#ifndef _SDL_GPU_H__
#define _SDL_GPU_H__

#include "SDL.h"
#include "SDL_opengl.h"

typedef struct GPU_Image
{
	GLuint handle;
	GLenum format;
	Uint16 w, h;
} GPU_Image;

typedef struct GPU_Target
{
	// I think this shouldn't even hold a surface, just w, h, and a renderer-specific render target
	SDL_Surface* surface;
	Uint16 w, h;
} GPU_Target;

GPU_Target* GPU_Init(Uint16 w, Uint16 h, Uint32 flags);
void GPU_Quit(void);

const char* GPU_GetErrorString(void);
const char* GPU_GetRendererString(void);

GPU_Image* GPU_LoadImage(const char* filename);
void GPU_FreeImage(GPU_Image* image);

GPU_Target* GPU_LoadTarget(SDL_Surface* surface);
void GPU_FreeTarget(GPU_Target* target);

int GPU_Blit(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y);
int GPU_BlitRotate(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle);
int GPU_BlitScale(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float scaleX, float scaleY);
int GPU_BlitTransform(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle, float scaleX, float scaleY);

void GPU_Clear(void);
void GPU_Flip(void);





#endif

