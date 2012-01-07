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
	GLuint handle;
	Uint16 w, h;
} GPU_Target;

GPU_Target* GPU_Init(Uint16 w, Uint16 h, Uint32 flags);
void GPU_Quit(void);

const char* GPU_GetErrorString(void);
const char* GPU_GetRendererString(void);

GPU_Image* GPU_LoadImage(const char* filename);
void GPU_FreeImage(GPU_Image* image);

GPU_Target* GPU_GetDisplayTarget(void);
GPU_Target* GPU_LoadTarget(GPU_Image* image);
void GPU_FreeTarget(GPU_Target* target);

int GPU_Blit(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y);
int GPU_BlitRotate(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle);
int GPU_BlitScale(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float scaleX, float scaleY);
int GPU_BlitTransform(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle, float scaleX, float scaleY);

void GPU_SetBlending(Uint8 enable);
void GPU_SetColor(SDL_Color* color);
void GPU_SetRGB(Uint8 r, Uint8 g, Uint8 b);
void GPU_SetRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

void GPU_MakeColorTransparent(GPU_Image* image, SDL_Color color);

void GPU_Clear(GPU_Target* target);
void GPU_Flip(void);





#endif

