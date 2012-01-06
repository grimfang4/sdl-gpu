#ifndef _SDL_GPUSHAPES_H__
#define _SDL_GPUSHAPES_H__

#include "SDL_gpu.h"

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

