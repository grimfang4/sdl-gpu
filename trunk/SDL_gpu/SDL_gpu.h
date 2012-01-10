#ifndef _SDL_GPU_H__
#define _SDL_GPU_H__

#include "SDL.h"




typedef struct GPU_Image
{
	void* data;
	Uint16 w, h;
} GPU_Image;


typedef struct GPU_Target
{
	void* data;
	Uint16 w, h;
	SDL_Rect clip_rect;
} GPU_Target;


struct GPU_Renderer;

typedef struct GPU_Renderer
{
	char* id;
	
	GPU_Target* display;
	
	GPU_Target* (*Init)(struct GPU_Renderer* renderer, Uint16 w, Uint16 h, Uint32 flags);
	void (*Quit)(struct GPU_Renderer* renderer);

	GPU_Image* (*LoadImage)(struct GPU_Renderer* renderer, const char* filename);
	void (*FreeImage)(struct GPU_Renderer* renderer, GPU_Image* image);

	GPU_Target* (*GetDisplayTarget)(struct GPU_Renderer* renderer);
	GPU_Target* (*LoadTarget)(struct GPU_Renderer* renderer, GPU_Image* image);
	void (*FreeTarget)(struct GPU_Renderer* renderer, GPU_Target* target);

	int (*Blit)(struct GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y);
	int (*BlitRotate)(struct GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle);
	int (*BlitScale)(struct GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float scaleX, float scaleY);
	int (*BlitTransform)(struct GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle, float scaleX, float scaleY);

	void (*SetBlending)(struct GPU_Renderer* renderer, Uint8 enable);
	void (*SetRGBA)(struct GPU_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	void (*MakeRGBTransparent)(struct GPU_Renderer* renderer, GPU_Image* image, Uint8 r, Uint8 g, Uint8 b);

	void (*Clear)(struct GPU_Renderer* renderer, GPU_Target* target);
	void (*Flip)(struct GPU_Renderer* renderer);
	
	void* data;
	
} GPU_Renderer;


// Setup calls
GPU_Target* GPU_Init(const char* renderer_id, Uint16 w, Uint16 h, Uint32 flags);
void GPU_CloseCurrentRenderer(void);
void GPU_Quit(void);

void GPU_SetError(const char* fmt, ...);
const char* GPU_GetErrorString(void);



// Renderer controls
const char* GPU_GetCurrentRendererID(void);
const char* GPU_GetDefaultRendererID(void);

int GPU_GetNumActiveRenderers(void);
void GPU_GetActiveRendererList(const char** renderers_array);

int GPU_GetNumRegisteredRenderers(void);
void GPU_GetRegisteredRendererList(const char** renderers_array);

GPU_Renderer* GPU_AddRenderer(const char* id);
void GPU_RemoveRenderer(const char* id);

GPU_Renderer* GPU_GetRendererByID(const char* id);
void GPU_SetCurrentRenderer(const char* id);



// Defined by renderer
GPU_Image* GPU_LoadImage(const char* filename);
void GPU_FreeImage(GPU_Image* image);

GPU_Target* GPU_GetDisplayTarget(void);
GPU_Target* GPU_LoadTarget(GPU_Image* image);
void GPU_FreeTarget(GPU_Target* target);

int GPU_Blit(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y);
int GPU_BlitRotate(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle);
int GPU_BlitScale(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float scaleX, float scaleY);
int GPU_BlitTransform(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle, float scaleX, float scaleY);

void GPU_SetClip(GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h);
void GPU_ResetClip(GPU_Target* target);

void GPU_SetBlending(Uint8 enable);
void GPU_SetColor(SDL_Color* color);
void GPU_SetRGB(Uint8 r, Uint8 g, Uint8 b);
void GPU_SetRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

void GPU_MakeColorTransparent(GPU_Image* image, SDL_Color color);

void GPU_Clear(GPU_Target* target);
void GPU_Flip(void);





#endif

