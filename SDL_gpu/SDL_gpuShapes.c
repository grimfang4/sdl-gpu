#include "SDL_gpu.h"
#include <string.h>

#define CHECK_RENDERER(ret) \
GPU_Renderer* renderer = GPU_GetCurrentRenderer(); \
if(renderer == NULL) \
    return ret;


float GPU_SetThickness(float thickness)
{
	CHECK_RENDERER(1.0f);
	if(renderer->SetThickness == NULL)
		return 1.0f;
	
	return renderer->SetThickness(renderer, thickness);
}

float GPU_GetThickness(void)
{
	CHECK_RENDERER(1.0f);
	if(renderer->GetThickness == NULL)
		return 1.0f;
	
	return renderer->GetThickness(renderer);
}

void GPU_Pixel(GPU_Target* target, float x, float y, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->Pixel == NULL)
		return;
	
	renderer->Pixel(renderer, target, x, y, color);
}

void GPU_Line(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->Line == NULL)
		return;
	
	renderer->Line(renderer, target, x1, y1, x2, y2, color);
}


void GPU_Arc(GPU_Target* target, float x, float y, float radius, float startAngle, float endAngle, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->Arc == NULL)
		return;
	
	renderer->Arc(renderer, target, x, y, radius, startAngle, endAngle, color);
}


void GPU_ArcFilled(GPU_Target* target, float x, float y, float radius, float startAngle, float endAngle, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->ArcFilled == NULL)
		return;
	
	renderer->ArcFilled(renderer, target, x, y, radius, startAngle, endAngle, color);
}

void GPU_Circle(GPU_Target* target, float x, float y, float radius, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->Circle == NULL)
		return;
	
	renderer->Circle(renderer, target, x, y, radius, color);
}

void GPU_CircleFilled(GPU_Target* target, float x, float y, float radius, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->CircleFilled == NULL)
		return;
	
	renderer->CircleFilled(renderer, target, x, y, radius, color);
}

void GPU_Tri(GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->Tri == NULL)
		return;
	
	renderer->Tri(renderer, target, x1, y1, x2, y2, x3, y3, color);
}

void GPU_TriFilled(GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->TriFilled == NULL)
		return;
	
	renderer->TriFilled(renderer, target, x1, y1, x2, y2, x3, y3, color);
}

void GPU_Rectangle(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->Rectangle == NULL)
		return;
	
	renderer->Rectangle(renderer, target, x1, y1, x2, y2, color);
}

void GPU_RectangleFilled(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->RectangleFilled == NULL)
		return;
	
	renderer->RectangleFilled(renderer, target, x1, y1, x2, y2, color);
}

void GPU_RectangleRound(GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->RectangleRound == NULL)
		return;
	
	renderer->RectangleRound(renderer, target, x1, y1, x2, y2, radius, color);
}

void GPU_RectangleRoundFilled(GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->RectangleRoundFilled == NULL)
		return;
	
	renderer->RectangleRoundFilled(renderer, target, x1, y1, x2, y2, radius, color);
}

void GPU_Polygon(GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->Polygon == NULL)
		return;
	
	renderer->Polygon(renderer, target, n, vertices, color);
}

void GPU_PolygonFilled(GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->PolygonFilled == NULL)
		return;
	
	renderer->PolygonFilled(renderer, target, n, vertices, color);
}


void GPU_PolygonBlit(GPU_Image* src, GPU_Rect* srcrect, GPU_Target* target, Uint16 n, float* vertices, float textureX, float textureY, float angle, float scaleX, float scaleY)
{
	CHECK_RENDERER();
	if(renderer->PolygonBlit == NULL)
		return;
	
	renderer->PolygonBlit(renderer, src, srcrect, target, n, vertices, textureX, textureY, angle, scaleX, scaleY);
}
