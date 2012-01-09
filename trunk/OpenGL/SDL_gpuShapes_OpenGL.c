#include "SDL_gpuShapes.h"
#include "SDL_gpu_OpenGL.h"
#include "SDL_opengl.h"


// FIXME: This should move into a file which users can use...
typedef struct ShapeRendererData_OpenGL
{
	GLuint handle;
	// What else?
} ShapeRendererData_OpenGL;




#define BEGIN \
	if(target == NULL) \
		return; \
	 \
	/* Bind the FBO */ \
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, ((TargetData_OpenGL*)target->data)->handle);

#define END \
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);


void Pixel(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, SDL_Color color)
{
	BEGIN;
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	glBegin(GL_POINTS);
	glVertex3f(x, y, 0);
	glEnd();
	
	END;
}

void Line(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color)
{
	BEGIN;
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	glBegin(GL_LINES);
	glVertex3f(x1, y1, 0);
	glVertex3f(x2, y2, 0);
	glEnd();
	
	END;
}


void Arc(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, float radius, float startAngle, float endAngle, SDL_Color color)
{
	BEGIN;
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	END;
}

void Circle(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, float radius, SDL_Color color)
{
	BEGIN;
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	END;
}

void CircleFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, float radius, SDL_Color color)
{
	BEGIN;
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	END;
}

void Tri(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Color color)
{
	BEGIN;
	
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
	BEGIN;
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	glBegin(GL_LINE_LOOP);
	glVertex3f(x1, y1, 0);
	glVertex3f(x1, y2, 0);
	glVertex3f(x2, y2, 0);
	glVertex3f(x2, y1, 0);
	glEnd();
	
	END;
}

void RectRoundFilled(GPU_ShapeRenderer* renderer, GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float radius, SDL_Color color)
{
	BEGIN;
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(x1, y1, 0);
	glVertex3f(x1, y2, 0);
	glVertex3f(x2, y1, 0);
	glVertex3f(x2, y2, 0);
	glEnd();
	
	END;
}

void Polygon(GPU_ShapeRenderer* renderer, GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
{
	BEGIN;
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	int i;
	glBegin(GL_LINE_LOOP);
	for(i = 0; i < 2*n; i+=2)
	{
		glVertex3f(vertices[i], vertices[i+1], 0);
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
		glVertex3f(vertices[i], vertices[i+1], 0);
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
