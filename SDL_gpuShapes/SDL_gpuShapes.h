#ifndef _SDL_GPUSHAPES_H__
#define _SDL_GPUSHAPES_H__

#include "SDL_gpu.h"



struct GPU_ShapeRenderer;

typedef struct GPU_ShapeRenderer
{
	GPU_Renderer* renderer;
	
	void (*Pixel)(struct GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, Sint16 x, Sint16 y, SDL_Color color);

	void (*Line)(struct GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color);

	void (*Arc)(struct GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, Sint16 x, Sint16 y, float radius, float startAngle, float endAngle, SDL_Color color);

	void (*Circle)(struct GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, Sint16 x, Sint16 y, float radius, SDL_Color color);

	void (*CircleFilled)(struct GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, Sint16 x, Sint16 y, float radius, SDL_Color color);

	void (*Tri)(struct GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Color color);

	void (*TriFilled)(struct GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Color color);

	void (*Rect)(struct GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color);

	void (*RectFilled)(struct GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color);

	void (*RectRound)(struct GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float radius, SDL_Color color);

	void (*RectRoundFilled)(struct GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float radius, SDL_Color color);

	void (*Polygon)(struct GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color);

	void (*PolygonFilled)(struct GPU_ShapeRenderer* shapeRenderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color);
	
	void* data;
	
} GPU_ShapeRenderer;

// Call this after setting a GPU_Renderer (e.g after GPU_Init())
void GPU_LoadShapeRenderer(void);

void GPU_Pixel(GPU_Target* target, Sint16 x, Sint16 y, SDL_Color color);

void GPU_Line(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color);

void GPU_Arc(GPU_Target* target, Sint16 x, Sint16 y, float radius, float startAngle, float endAngle, SDL_Color color);

void GPU_Circle(GPU_Target* target, Sint16 x, Sint16 y, float radius, SDL_Color color);

void GPU_CircleFilled(GPU_Target* target, Sint16 x, Sint16 y, float radius, SDL_Color color);

void GPU_Tri(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Color color);

void GPU_TriFilled(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Color color);

void GPU_Rect(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color);

void GPU_RectFilled(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color);

void GPU_RectRound(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float radius, SDL_Color color);

void GPU_RectRoundFilled(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float radius, SDL_Color color);

void GPU_Polygon(GPU_Target* target, Uint16 n, float* vertices, SDL_Color color);

void GPU_PolygonFilled(GPU_Target* target, Uint16 n, float* vertices, SDL_Color color);




#endif

