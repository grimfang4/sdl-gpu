#include "SDL_gpu.h"

#define BEGIN \
	if(target == NULL) \
		return; \
	 \
	/* Bind the FBO */ \
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, target->handle);

#define END \
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);


void GPU_Pixel(GPU_Target* target, Sint16 x, Sint16 y, SDL_Color color)
{
	BEGIN;
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	glBegin(GL_POINTS);
	glVertex3f(x, y, 0);
	glEnd();
	
	END;
}

void GPU_Line(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color)
{
	BEGIN;
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	glBegin(GL_LINES);
	glVertex3f(x1, y1, 0);
	glVertex3f(x2, y2, 0);
	glEnd();
	
	END;
}


void GPU_Arc(GPU_Target* target, Sint16 x, Sint16 y, float radius, float startAngle, float endAngle, SDL_Color color)
{
	BEGIN;
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	END;
}

void GPU_Circle(GPU_Target* target, Sint16 x, Sint16 y, float radius, SDL_Color color)
{
	BEGIN;
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	END;
}

void GPU_CircleFilled(GPU_Target* target, Sint16 x, Sint16 y, float radius, SDL_Color color)
{
	BEGIN;
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	END;
}

void GPU_Tri(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Color color)
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

void GPU_TriFilled(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, SDL_Color color)
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

void GPU_Rect(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color)
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

void GPU_RectFilled(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color)
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

void GPU_RectRound(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float radius, SDL_Color color)
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

void GPU_RectRoundFilled(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, float radius, SDL_Color color)
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

void GPU_Polygon(GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
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

void GPU_PolygonFilled(GPU_Target* target, Uint16 n, float* vertices, SDL_Color color)
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
