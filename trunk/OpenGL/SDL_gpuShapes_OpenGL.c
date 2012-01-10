#include "SDL_gpuShapes.h"
#include "SDL_gpu_OpenGL.h"
#include "SDL_opengl.h"
#include <math.h>

#ifndef DEGPERRAD 
#define DEGPERRAD 57.2957795f
#endif

#ifndef RADPERDEG 
#define RADPERDEG 0.0174532925f
#endif

void Circle(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, float radius, SDL_Color color);

// FIXME: This should move into a file which users can use...
typedef struct ShapeRendererData_OpenGL
{
	GLuint handle;
	// What else?
} ShapeRendererData_OpenGL;




#define BEGIN \
	if(target == NULL) \
		return; \
	if(renderer->renderer != target->renderer) \
		return; \
	 \
	/* Bind the FBO */ \
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ((TargetData_OpenGL*)target->data)->handle); \
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT); \
	Uint8 doClip = (target->clip_rect.x > 0 || target->clip_rect.y > 0 || target->clip_rect.w < target->w || target->clip_rect.h < target->h); \
	if(doClip) \
	{ \
		glEnable(GL_SCISSOR_TEST); \
		int y = (renderer->renderer->display == target? renderer->renderer->display->h - (target->clip_rect.y + target->clip_rect.h) : target->clip_rect.y); \
		glScissor(target->clip_rect.x, y, target->clip_rect.w, target->clip_rect.h); \
	}

#define END \
	if(doClip) \
	{ \
		glDisable(GL_SCISSOR_TEST); \
	} \
	glPopAttrib(); \
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	
#define INVERT_Y(y) \
	if(renderer->renderer->display != target) \
		(y) = renderer->renderer->display->h - (y);

void Pixel(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, SDL_Color color)
{
	BEGIN;
	
	INVERT_Y(y);
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	glBegin(GL_POINTS);
	glVertex3f(x, y, 0);
	glEnd();
	
	END;
}

void Line(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color)
{
	BEGIN;
	
	INVERT_Y(y1);
	INVERT_Y(y2);
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	glBegin(GL_LINES);
	glVertex3f(x1, y1, 0);
	glVertex3f(x2, y2, 0);
	glEnd();
	
	END;
}


void Arc(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, float radius, float startAngle, float endAngle, SDL_Color color)
{
    float originalSA = startAngle;

    if(startAngle > endAngle)
    {
        float swapa = endAngle;
        endAngle = startAngle;
        startAngle = swapa;
    }
    if(startAngle == endAngle)
        return;
    
    // Big angle
    if(endAngle - startAngle >= 360)
    {
        Circle(renderer, target, x, y, radius, color);
        return;
    }
    
    // Shift together
    while(startAngle < 0 && endAngle < 0)
    {
        startAngle += 360;
        endAngle += 360;
    }
    while(startAngle > 360 && endAngle > 360)
    {
        startAngle -= 360;
        endAngle -= 360;
    }
    
    // Check if the angle to be drawn crosses 0
    Uint8 crossesZero = (startAngle < 0 && endAngle > 0) || (startAngle < 360 && endAngle > 360);
    
    // Push all values to 0 <= angle < 360
    while(startAngle >= 360)
        startAngle -= 360;
    while(endAngle >= 360)
        endAngle -= 360;
    while(startAngle < 0)
        startAngle += 360;
    while(endAngle < 0)
        endAngle += 360;
    
    if(endAngle == 0)
        endAngle = 360;
    else if(crossesZero)
    {
        Arc(renderer, target, x, y, radius, originalSA, 359.9f, color);
        startAngle = 0;
    }
	
	
	BEGIN;
	
	INVERT_Y(y);
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	float t = startAngle;
	float dt = (1 - (endAngle - startAngle)/360) * 5;  // A segment every 5 degrees of a full circle
	float dx, dy;
	glBegin(GL_LINES);
	dx = radius*cos(t*RADPERDEG);
	dy = radius*sin(t*RADPERDEG);
	while(t < endAngle)
	{
		glVertex3f(x+dx, y+dy, 0);
		t += dt;
		dx = radius*cos(t*RADPERDEG);
		dy = radius*sin(t*RADPERDEG);
		glVertex3f(x+dx, y+dy, 0);
	}
	glEnd();
	
	END;
}


void ArcFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, float radius, float startAngle, float endAngle, SDL_Color color)
{
    float originalSA = startAngle;

    if(startAngle > endAngle)
    {
        float swapa = endAngle;
        endAngle = startAngle;
        startAngle = swapa;
    }
    if(startAngle == endAngle)
        return;
    
    // Big angle
    if(endAngle - startAngle >= 360)
    {
        Circle(renderer, target, x, y, radius, color);
        return;
    }
    
    // Shift together
    while(startAngle < 0 && endAngle < 0)
    {
        startAngle += 360;
        endAngle += 360;
    }
    while(startAngle > 360 && endAngle > 360)
    {
        startAngle -= 360;
        endAngle -= 360;
    }
    
    // Check if the angle to be drawn crosses 0
    Uint8 crossesZero = (startAngle < 0 && endAngle > 0) || (startAngle < 360 && endAngle > 360);
    
    // Push all values to 0 <= angle < 360
    while(startAngle >= 360)
        startAngle -= 360;
    while(endAngle >= 360)
        endAngle -= 360;
    while(startAngle < 0)
        startAngle += 360;
    while(endAngle < 0)
        endAngle += 360;
    
    if(endAngle == 0)
        endAngle = 360;
    else if(crossesZero)
    {
        ArcFilled(renderer, target, x, y, radius, originalSA, 359.9f, color);
        startAngle = 0;
    }
	
	
	BEGIN;
	
	INVERT_Y(y);
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	float t = startAngle;
	float dt = (1 - (endAngle - startAngle)/360) * 5;  // A segment every 5 degrees of a full circle
	float dx, dy;
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(x, y, 0);
	dx = radius*cos(t*RADPERDEG);
	dy = radius*sin(t*RADPERDEG);
	while(t < endAngle)
	{
		glVertex3f(x+dx, y+dy, 0);
		t += dt;
		dx = radius*cos(t*RADPERDEG);
		dy = radius*sin(t*RADPERDEG);
		glVertex3f(x+dx, y+dy, 0);
	}
	glEnd();
	
	END;
}

void Circle(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, float radius, SDL_Color color)
{
	BEGIN;
	
	INVERT_Y(y);
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	float t = 0;
	float dt = 5;  // A segment every 5 degrees of a full circle
	float dx, dy;
	glBegin(GL_LINES);
	dx = radius*cos(t*RADPERDEG);
	dy = radius*sin(t*RADPERDEG);
	while(t < 360)
	{
		glVertex3f(x+dx, y+dy, 0);
		t += dt;
		dx = radius*cos(t*RADPERDEG);
		dy = radius*sin(t*RADPERDEG);
		glVertex3f(x+dx, y+dy, 0);
	}
	glEnd();
	
	END;
}

void CircleFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, float radius, SDL_Color color)
{
	BEGIN;
	
	INVERT_Y(y);
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	float t = 0;
	float dt = 5;  // A segment every 5 degrees of a full circle
	float dx, dy;
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(x, y, 0);
	dx = radius*cos(t*RADPERDEG);
	dy = radius*sin(t*RADPERDEG);
	while(t < 360)
	{
		glVertex3f(x+dx, y+dy, 0);
		t += dt;
		dx = radius*cos(t*RADPERDEG);
		dy = radius*sin(t*RADPERDEG);
		glVertex3f(x+dx, y+dy, 0);
	}
	glEnd();
	
	END;
}

void Tri(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Color color)
{
	BEGIN;
	
	INVERT_Y(y1);
	INVERT_Y(y2);
	INVERT_Y(y3);
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	glBegin(GL_LINE_LOOP);
	glVertex3f(x1, y1, 0);
	glVertex3f(x2, y2, 0);
	glVertex3f(x3, y3, 0);
	glEnd();
	
	END;
}

void TriFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Color color)
{
	BEGIN;
	
	INVERT_Y(y1);
	INVERT_Y(y2);
	INVERT_Y(y3);
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(x1, y1, 0);
	glVertex3f(x2, y2, 0);
	glVertex3f(x3, y3, 0);
	glEnd();
	
	END;
}

void Rect(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color)
{
	BEGIN;
	
	INVERT_Y(y1);
	INVERT_Y(y2);
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	glBegin(GL_LINE_LOOP);
	glVertex3f(x1, y1, 0);
	glVertex3f(x1, y2, 0);
	glVertex3f(x2, y2, 0);
	glVertex3f(x2, y1, 0);
	glEnd();
	
	END;
}

void RectFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color)
{
	BEGIN;
	
	INVERT_Y(y1);
	INVERT_Y(y2);
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(x1, y1, 0);
	glVertex3f(x1, y2, 0);
	glVertex3f(x2, y1, 0);
	glVertex3f(x2, y2, 0);
	glEnd();
	
	END;
}

void RectRound(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float radius, SDL_Color color)
{	
	Sint16 minX = (x1 < x2? x1 : x2) + (Sint16)(radius);
    Sint16 maxX = (x1 > x2? x1 : x2) - (Sint16)(radius);
    Sint16 minY = (y1 < y2? y1 : y2) + (Sint16)(radius);
    Sint16 maxY = (y1 > y2? y1 : y2) - (Sint16)(radius);
    Line(renderer, target, minX,y1,maxX,y1,color);
    Line(renderer, target, minX,y2,maxX,y2,color);
    Line(renderer, target, x1,minY,x1,maxY,color);
    Line(renderer, target, x2,minY,x2,maxY,color);
	
	Arc(renderer, target, minX, minY, radius, 180, 270, color);
	Arc(renderer, target, maxX, minY, radius, 270, 360, color);
	Arc(renderer, target, maxX, maxY, radius, 0, 90, color);
	Arc(renderer, target, minX, maxY, radius, 90, 180, color);
}

void RectRoundFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float radius, SDL_Color color)
{
	Sint16 minX = (x1 < x2? x1 : x2) + (Sint16)(radius);
    Sint16 maxX = (x1 > x2? x1 : x2) - (Sint16)(radius);
    Sint16 minY = (y1 < y2? y1 : y2) + (Sint16)(radius);
    Sint16 maxY = (y1 > y2? y1 : y2) - (Sint16)(radius);
	
    // Center
    SDL_Rect area;
    area.x=minX;
    area.y=minY;
    area.w=maxX-minX;
    area.h=maxY-minY;
    RectFilled(renderer, target,area.x, area.y, area.x+area.w, area.y+area.h, color);
    // Top
    area.x= minX;
    area.y= y1;
    area.w= maxX-minX;
    area.h= (Sint16)(radius);
    RectFilled(renderer, target,area.x, area.y, area.x+area.w, area.y+area.h, color);
    // Bottom
    area.y= y2-(Sint16)(radius);
    RectFilled(renderer, target,area.x, area.y, area.x+area.w, area.y+area.h, color);
    // Left
    area.x= x1;
    area.y= minY-1;
    area.w= (Sint16)(radius);
    area.h= maxY-minY+1;
    RectFilled(renderer, target,area.x, area.y, area.x+area.w, area.y+area.h, color);
    // Right
    area.x= x2-(Sint16)(radius);
    RectFilled(renderer, target,area.x, area.y, area.x+area.w, area.y+area.h, color);
	
	ArcFilled(renderer, target, minX, minY, radius, 180, 270, color);
	ArcFilled(renderer, target, maxX, minY, radius, 270, 360, color);
	ArcFilled(renderer, target, maxX, maxY, radius, 0, 90, color);
	ArcFilled(renderer, target, minX, maxY, radius, 90, 180, color);
}

void Polygon(GPU_ShapeRenderer* renderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
	BEGIN;
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	int i;
	glBegin(GL_LINE_LOOP);
	for(i = 0; i < 2*n; i+=2)
	{
		float y = vertices[i+1];
		
		INVERT_Y(y);
		
		glVertex3f(vertices[i], y, 0);
	}
	glEnd();
	
	END;
}

void PolygonFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
	BEGIN;
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	int i;
	glBegin(GL_POLYGON);
	for(i = 0; i < 2*n; i+=2)
	{
		float y = vertices[i+1];
		
		INVERT_Y(y);
		
		glVertex3f(vertices[i], y, 0);
	}
	glEnd();
	
	END;
}










GPU_ShapeRenderer* GPU_CreateShapeRenderer_OpenGL(void)
{
	GPU_ShapeRenderer* renderer = (GPU_ShapeRenderer*)malloc(sizeof(GPU_ShapeRenderer));
	
	renderer->data = (ShapeRendererData_OpenGL*)malloc(sizeof(ShapeRendererData_OpenGL));
	
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

void GPU_FreeShapeRenderer_OpenGL(GPU_ShapeRenderer* renderer)
{
	if(renderer == NULL)
		return;
	
	free(renderer->data);
	free(renderer);
}
