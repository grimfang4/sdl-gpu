#include "SDL_gpu.h"


void GPU_Pixel(GPU_Target* target, Sint16 x, Sint16 y, SDL_Color color)
{
	if(target == NULL)
		return;
	
	// Bind the FBO
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, target->handle);
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	glBegin(GL_POINTS);
	glVertex3f(x, y, 0);
	glEnd();
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void GPU_Line(GPU_Target* target, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, SDL_Color color)
{
	if(target == NULL)
		return;
	
	// Bind the FBO
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, target->handle);
	
	glColor4ub(color.r, color.g, color.b, color.unused);
	
	glBegin(GL_LINES);
	glVertex3f(x1, y1, 0);
	glVertex3f(x2, y2, 0);
	glEnd();
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}