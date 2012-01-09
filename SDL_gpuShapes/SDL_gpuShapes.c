#include "SDL_gpuShapes.h"
#include <string.h>

#include "../OpenGL/SDL_gpuShapes_OpenGL.h"


static GPU_ShapeRenderer* shapeRenderer = NULL;

static void (*freeShapeRendererFn)(GPU_ShapeRenderer* renderer) = NULL;

void GPU_LoadShapeRenderer(void)
{
	// Free the old one
	if(shapeRenderer != NULL)
	{
		freeShapeRendererFn(shapeRenderer);
		shapeRenderer = NULL;
		freeShapeRendererFn = NULL;
	}
	
	const char* rendererID = GPU_GetCurrentRendererID();
	GPU_Renderer* renderer = GPU_GetRendererByID(rendererID);
	if(renderer == NULL)
		return;
	
	if(strcmp(rendererID, "OpenGL"))
	{
		GPU_ShapeRenderer* sr = GPU_CreateShapeRenderer_OpenGL();
		if(sr == NULL)
			return;
		sr->renderer = renderer;
		freeShapeRendererFn = &GPU_FreeShapeRenderer_OpenGL;
	}
	
}



void GPU_Pixel(GPU_Target* target, Sint16 x, Sint16 y, SDL_Color color)
{
	if(shapeRenderer == NULL || shapeRenderer->Pixel == NULL)
		return;
	
	shapeRenderer->Pixel(shapeRenderer, target, x, y, color);
}

void GPU_Line(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color)
{
	if(shapeRenderer == NULL || shapeRenderer->Line == NULL)
		return;
	
	shapeRenderer->Line(shapeRenderer, target, x1, y1, x2, y2, color);
}


void GPU_Arc(GPU_Target* target, Sint16 x, Sint16 y, float radius, float startAngle, float endAngle, SDL_Color color)
{
	if(shapeRenderer == NULL || shapeRenderer->Arc == NULL)
		return;
	
	shapeRenderer->Arc(shapeRenderer, target, x, y, radius, startAngle, endAngle, color);
}

void GPU_Circle(GPU_Target* target, Sint16 x, Sint16 y, float radius, SDL_Color color)
{
	if(shapeRenderer == NULL || shapeRenderer->Circle == NULL)
		return;
	
	shapeRenderer->Circle(shapeRenderer, target, x, y, radius, color);
}

void GPU_CircleFilled(GPU_Target* target, Sint16 x, Sint16 y, float radius, SDL_Color color)
{
	if(shapeRenderer == NULL || shapeRenderer->CircleFilled == NULL)
		return;
	
	shapeRenderer->CircleFilled(shapeRenderer, target, x, y, radius, color);
}

void GPU_Tri(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Color color)
{
	if(shapeRenderer == NULL || shapeRenderer->Tri == NULL)
		return;
	
	shapeRenderer->Tri(shapeRenderer, target, x1, y1, x2, y2, x3, y3, color);
}

void GPU_TriFilled(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Color color)
{
	if(shapeRenderer == NULL || shapeRenderer->TriFilled == NULL)
		return;
	
	shapeRenderer->TriFilled(shapeRenderer, target, x1, y1, x2, y2, x3, y3, color);
}

void GPU_Rect(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color)
{
	if(shapeRenderer == NULL || shapeRenderer->Rect == NULL)
		return;
	
	shapeRenderer->Rect(shapeRenderer, target, x1, y1, x2, y2, color);
}

void GPU_RectFilled(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color)
{
	if(shapeRenderer == NULL || shapeRenderer->RectFilled == NULL)
		return;
	
	shapeRenderer->RectFilled(shapeRenderer, target, x1, y1, x2, y2, color);
}

void GPU_RectRound(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float radius, SDL_Color color)
{
	if(shapeRenderer == NULL || shapeRenderer->RectRound == NULL)
		return;
	
	shapeRenderer->RectRound(shapeRenderer, target, x1, y1, x2, y2, radius, color);
}

void GPU_RectRoundFilled(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float radius, SDL_Color color)
{
	if(shapeRenderer == NULL || shapeRenderer->RectRoundFilled == NULL)
		return;
	
	shapeRenderer->RectRoundFilled(shapeRenderer, target, x1, y1, x2, y2, radius, color);
}

void GPU_Polygon(GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
	if(shapeRenderer == NULL || shapeRenderer->Polygon == NULL)
		return;
	
	shapeRenderer->Polygon(shapeRenderer, target, n, vertices, color);
}

void GPU_PolygonFilled(GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
	if(shapeRenderer == NULL || shapeRenderer->PolygonFilled == NULL)
		return;
	
	shapeRenderer->PolygonFilled(shapeRenderer, target, n, vertices, color);
}
