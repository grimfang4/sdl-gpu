#include "SDL_gpu_Direct3D_internal.h"
#include "SDL_gpuShapes_Direct3D_internal.h"




static void Pixel(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, SDL_Color color)
{
	
}

static void Line(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color)
{
	
}


static void Arc(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, float radius, float startAngle, float endAngle, SDL_Color color)
{
	
}


static void ArcFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, float radius, float startAngle, float endAngle, SDL_Color color)
{
	
}

static void Circle(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, float radius, SDL_Color color)
{
	
}

static void CircleFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, float radius, SDL_Color color)
{
	
}

static void Tri(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Color color)
{
	
}

static void TriFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Color color)
{
	
}

static void Rect(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color)
{
	
}

static void RectFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color)
{
	
}

static void RectRound(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float radius, SDL_Color color)
{
	
}

static void RectRoundFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float radius, SDL_Color color)
{
	
}

static void Polygon(GPU_ShapeRenderer* renderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
	
}

static void PolygonFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
	
}










GPU_ShapeRenderer* GPU_CreateShapeRenderer_Direct3D(void)
{
	GPU_ShapeRenderer* renderer = (GPU_ShapeRenderer*)malloc(sizeof(GPU_ShapeRenderer));
	
	renderer->data = (ShapeRendererData_Direct3D*)malloc(sizeof(ShapeRendererData_Direct3D));
	
	renderer->Pixel = &Pixel;
	renderer->Line = &Line;
	renderer->Arc = &Arc;
	renderer->Circle = &Circle;
	renderer->CircleFilled = &CircleFilled;
	renderer->Tri = &Tri;
	renderer->TriFilled = &TriFilled;
	renderer->Rect = &Rect;
	renderer->RectFilled = &RectFilled;
	renderer->RectRound = &RectRound;
	renderer->RectRoundFilled = &RectRoundFilled;
	renderer->Polygon = &Polygon;
	renderer->PolygonFilled = &PolygonFilled;
	
	return renderer;
}

void GPU_FreeShapeRenderer_Direct3D(GPU_ShapeRenderer* renderer)
{
	if(renderer == NULL)
		return;
	
	free(renderer->data);
	free(renderer);
}
