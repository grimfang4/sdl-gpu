#ifndef _SDL_GPU_H__
#define _SDL_GPU_H__

#include "SDL.h"


#ifdef __cplusplus
extern "C" {
#endif



struct GPU_Renderer;

typedef struct GPU_Image
{
	struct GPU_Renderer* renderer;
	void* data;
	Uint16 w, h;
	int channels;
} GPU_Image;


typedef struct GPU_Target
{
	struct GPU_Renderer* renderer;
	void* data;
	Uint16 w, h;
	SDL_Rect clip_rect;
} GPU_Target;

typedef unsigned int GPU_FilterEnum;
static const GPU_FilterEnum GPU_NEAREST = 0;
static const GPU_FilterEnum GPU_LINEAR = 1;

typedef struct GPU_Renderer
{
	char* id;
	
	GPU_Target* display;
	
	GPU_Target* (*Init)(struct GPU_Renderer* renderer, Uint16 w, Uint16 h, Uint32 flags);
	void (*Quit)(struct GPU_Renderer* renderer);

	GPU_Image* (*CreateImage)(struct GPU_Renderer* renderer, Uint16 w, Uint16 h, Uint8 channels);
	GPU_Image* (*LoadImage)(struct GPU_Renderer* renderer, const char* filename);
	GPU_Image* (*CopyImage)(struct GPU_Renderer* renderer, GPU_Image* image);
	GPU_Image* (*CopyImageFromSurface)(struct GPU_Renderer* renderer, SDL_Surface* surface);
	void (*FreeImage)(struct GPU_Renderer* renderer, GPU_Image* image);

	GPU_Target* (*GetDisplayTarget)(struct GPU_Renderer* renderer);
	GPU_Target* (*LoadTarget)(struct GPU_Renderer* renderer, GPU_Image* image);
	void (*FreeTarget)(struct GPU_Renderer* renderer, GPU_Target* target);

	int (*Blit)(struct GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y);
	int (*BlitRotate)(struct GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle);
	int (*BlitScale)(struct GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float scaleX, float scaleY);
	int (*BlitTransform)(struct GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle, float scaleX, float scaleY);
	
	float (*SetZ)(struct GPU_Renderer* renderer, float z);
	float (*GetZ)(struct GPU_Renderer* renderer);
	
	void (*GenerateMipmaps)(struct GPU_Renderer* renderer, GPU_Image* image);

	void (*SetBlending)(struct GPU_Renderer* renderer, Uint8 enable);
	void (*SetRGBA)(struct GPU_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	void (*ReplaceRGB)(struct GPU_Renderer* renderer, GPU_Image* image, Uint8 from_r, Uint8 from_g, Uint8 from_b, Uint8 to_r, Uint8 to_g, Uint8 to_b);
	
	void (*MakeRGBTransparent)(struct GPU_Renderer* renderer, GPU_Image* image, Uint8 r, Uint8 g, Uint8 b);
	
	void (*ShiftHSV)(struct GPU_Renderer* renderer, GPU_Image* image, int hue, int saturation, int value);
	
	void (*ShiftHSVExcept)(struct GPU_Renderer* renderer, GPU_Image* image, int hue, int saturation, int value, int notHue, int notSat, int notVal, int range);
	
	SDL_Color (*GetPixel)(struct GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y);
	
	void (*SetImageFilter)(struct GPU_Renderer* renderer, GPU_Image* image, GPU_FilterEnum filter);

	void (*Clear)(struct GPU_Renderer* renderer, GPU_Target* target);
	void (*ClearRGBA)(struct GPU_Renderer* renderer, GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
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
GPU_Renderer* GPU_GetCurrentRenderer(void);



// Defined by renderer

GPU_Image* GPU_CreateImage(Uint16 w, Uint16 h, Uint8 channels);
GPU_Image* GPU_LoadImage(const char* filename);
GPU_Image* GPU_CopyImage(GPU_Image* image);
GPU_Image* GPU_CopyImageFromSurface(SDL_Surface* surface);
void GPU_FreeImage(GPU_Image* image);

GPU_Target* GPU_GetDisplayTarget(void);
GPU_Target* GPU_LoadTarget(GPU_Image* image);
void GPU_FreeTarget(GPU_Target* target);

int GPU_Blit(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y);
int GPU_BlitRotate(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle);
int GPU_BlitScale(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float scaleX, float scaleY);
int GPU_BlitTransform(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle, float scaleX, float scaleY);

float GPU_SetZ(float z);
float GPU_GetZ(void);

void GPU_GenerateMipmaps(GPU_Image* image);

void GPU_SetClip(GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h);
void GPU_ResetClip(GPU_Target* target);

void GPU_SetBlending(Uint8 enable);
void GPU_SetColor(SDL_Color* color);
void GPU_SetRGB(Uint8 r, Uint8 g, Uint8 b);
void GPU_SetRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

void GPU_ReplaceColor(GPU_Image* image, SDL_Color from, SDL_Color to);
void GPU_MakeColorTransparent(GPU_Image* image, SDL_Color color);
void GPU_ShiftHSV(GPU_Image* image, int hue, int saturation, int value);
void GPU_ShiftHSVExcept(GPU_Image* image, int hue, int saturation, int value, int notHue, int notSat, int notVal, int range);
SDL_Color GPU_GetPixel(GPU_Target* target, Sint16 x, Sint16 y);
void GPU_SetImageFilter(GPU_Image* image, GPU_FilterEnum filter);

void GPU_Clear(GPU_Target* target);
void GPU_ClearRGBA(GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void GPU_Flip(void);

#ifdef __cplusplus
}
#endif



#endif

