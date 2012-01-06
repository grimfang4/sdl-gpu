#include "SDL_gpu.h"
#include "SDL_opengl.h"

static GPU_Target* display = NULL;

GPU_Target* GPU_Init(Uint16 w, Uint16 h, Uint32 flags)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		return NULL;
	}
	
	if(flags & SDL_DOUBLEBUF)
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	flags &= ~SDL_DOUBLEBUF;
	
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	
	flags |= SDL_OPENGL;
	
	SDL_Surface* screen = SDL_SetVideoMode(w, h, 0, flags);
	
	if(screen == NULL)
		return NULL;
	
	glEnable( GL_TEXTURE_2D );
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	
	glViewport( 0, 0, w, h);
	
	glClear( GL_COLOR_BUFFER_BIT );
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	glOrtho(0.0f, w, h, 0.0f, -1.0f, 1.0f);
	
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	
	glEnable( GL_TEXTURE_2D );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	
	if(display == NULL)
		display = (GPU_Target*)malloc(sizeof(GPU_Target));

	display->handle = 0;
	display->w = screen->w;
	display->h = screen->h;
	
	return display;
}

void GPU_Quit(void)
{
	free(display);
	display = NULL;
	
	SDL_Quit();
}

const char* GPU_GetErrorString(void)
{
	return SDL_GetError();
}

const char* GPU_GetRendererString(void)
{
	return "OpenGL";
}

GPU_Image* GPU_LoadImage(const char* filename)
{
	
	GLuint texture;			// This is a handle to our texture object
	SDL_Surface *surface;	// This surface will tell us the details of the image
	GLenum texture_format;
	GLint  nOfColors;
	Uint16 w, h;
	
	if ( (surface = SDL_LoadBMP(filename)) ) { 
	
		// Check that the image's width is a power of 2
		if ( (surface->w & (surface->w - 1)) != 0 ) {
			printf("warning: image's width is not a power of 2\n");
		}
	
		// Also check if the height is a power of 2
		if ( (surface->h & (surface->h - 1)) != 0 ) {
			printf("warning: image's height is not a power of 2\n");
		}
		
		w = surface->w;
		h = surface->h;
	
		// get the number of channels in the SDL surface
		nOfColors = surface->format->BytesPerPixel;
		if (nOfColors == 4)     // contains an alpha channel
		{
				if (surface->format->Rmask == 0x000000ff)
						texture_format = GL_RGBA;
				else
						texture_format = GL_BGRA;
		} else if (nOfColors == 3)     // no alpha channel
		{
				if (surface->format->Rmask == 0x000000ff)
						texture_format = GL_RGB;
				else
						texture_format = GL_BGR;
		} else {
				printf("warning: the image is not truecolor..  this will probably break\n");
				// this error should not go unhandled
		}
	
		// Have OpenGL generate a texture object handle for us
		glGenTextures( 1, &texture );
	
		// Bind the texture object
		glBindTexture( GL_TEXTURE_2D, texture );
	
		// Set the texture's stretching properties
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
		// Edit the texture object's image data using the information SDL_Surface gives us
		glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
						texture_format, GL_UNSIGNED_BYTE, surface->pixels );
	} 
	else {
		printf("SDL could not load image: %s\n", SDL_GetError());
	}    
	
	// Free the SDL_Surface
	SDL_FreeSurface( surface );
	
	GPU_Image* result = (GPU_Image*)malloc(sizeof(GPU_Image));
	result->handle = texture;
	result->format = texture_format;
	result->w = w;
	result->h = h;
	
	return result;
}

void GPU_FreeImage(GPU_Image* image)
{
	if(image == NULL)
		return;
	
	glDeleteTextures( 1, &image->handle);
	free(image);
}

GPU_Target* GPU_GetDisplayTarget(void)
{
	return display;
}


GPU_Target* GPU_LoadTarget(GPU_Image* image)
{
	GLuint handle;
	// Create framebuffer object
	glGenFramebuffersEXT(1, &handle);
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, handle);
	
	// Attach the texture to it
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, image->handle, 0); // 502
	
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
		return NULL;
	
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	
	GPU_Target* result = (GPU_Target*)malloc(sizeof(GPU_Target));
	
	result->handle = handle;
	result->w = image->w;
	result->h = image->h;
	
	return result;
}



void GPU_FreeTarget(GPU_Target* target)
{
	if(target == NULL || target == display)
		return;
	
	glDeleteFramebuffers(1, &target->handle);
	
	free(target);
}



int GPU_Blit(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y)
{
	if(src == NULL || dest == NULL)
		return -1;
	
	
	// Bind the texture to which subsequent calls refer
	glBindTexture( GL_TEXTURE_2D, src->handle );
	
	// Bind the FBO
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, dest->handle);
	glPushAttrib(GL_VIEWPORT_BIT);
	//glViewport(0,0, dest->w, dest->h);
	
	float x1, y1, x2, y2;
	float dx1, dy1, dx2, dy2;
	dx1 = x;
	dy1 = y;
	if(srcrect == NULL)
	{
		x1 = 0;
		y1 = 0;
		x2 = 1;
		y2 = 1;
		dx2 = x + src->w;
		dy2 = y + src->h;
	}
	else
	{
		x1 = srcrect->x/(float)src->w;
		y1 = srcrect->y/(float)src->h;
		x2 = (srcrect->x + srcrect->w)/(float)src->w;
		y2 = (srcrect->y + srcrect->h)/(float)src->h;
		dx2 = x + srcrect->w;
		dy2 = y + srcrect->h;
	}
	
	if(dest != display)
	{
		dy1 = display->h - y;
		dy2 = display->h - y;
		
		if(srcrect == NULL)
			dy2 -= src->h;
		else
			dy2 -= srcrect->h;
	}
	
	glBegin( GL_QUADS );
		//Bottom-left vertex (corner)
		glTexCoord2f( x1, y1 );
		glVertex3f( dx1, dy1, 0.0f );
	
		//Bottom-right vertex (corner)
		glTexCoord2f( x2, y1 );
		glVertex3f( dx2, dy1, 0.f );
	
		//Top-right vertex (corner)
		glTexCoord2f( x2, y2 );
		glVertex3f( dx2, dy2, 0.f );
	
		//Top-left vertex (corner)
		glTexCoord2f( x1, y2 );
		glVertex3f( dx1, dy2, 0.f );
	glEnd();
	
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}


int GPU_BlitRotate(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle)
{
	if(src == NULL || dest == NULL)
		return -1;
	
	glPushMatrix();
	
	glTranslatef(x, y, 0);
	glRotatef(angle, 0, 0, 1);
	if(srcrect != NULL)
		glTranslatef(-srcrect->w/2, -srcrect->h/2, 0);
	else
		glTranslatef(-src->w/2, -src->h/2, 0);
	
	int result = GPU_Blit(src, srcrect, dest, 0, 0);
	
	glPopMatrix();
	
	return result;
}

int GPU_BlitScale(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float scaleX, float scaleY)
{
	if(src == NULL || dest == NULL)
		return -1;
	
	// Bind the texture to which subsequent calls refer
	glBindTexture( GL_TEXTURE_2D, src->handle );
	
	// Bind the FBO
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, dest->handle);
	glPushAttrib(GL_VIEWPORT_BIT);
	//glViewport(0,0,dest->w, dest->h);
	
	float x1, y1, x2, y2;
	float dx1, dy1, dx2, dy2;
	dx1 = x;
	dy1 = y;
	if(srcrect == NULL)
	{
		x1 = 0;
		y1 = 0;
		x2 = 1;
		y2 = 1;
		dx2 = x + src->w*scaleX;
		dy2 = y + src->h*scaleY;
	}
	else
	{
		x1 = srcrect->x/(float)src->w;
		y1 = srcrect->y/(float)src->h;
		x2 = (srcrect->x + srcrect->w)/(float)src->w;
		y2 = (srcrect->y + srcrect->h)/(float)src->h;
		dx2 = x + srcrect->w*scaleX;
		dy2 = y + srcrect->h*scaleY;
	}
	
	if(dest != display)
	{
		dy1 = display->h - y;
		dy2 = display->h - y;
		
		if(srcrect == NULL)
			dy2 -= src->h*scaleY;
		else
			dy2 -= srcrect->h*scaleY;
	}
	
	glBegin( GL_QUADS );
		//Bottom-left vertex (corner)
		glTexCoord2f( x1, y1 );
		glVertex3f( dx1, dy1, 0.0f );
	
		//Bottom-right vertex (corner)
		glTexCoord2f( x2, y1 );
		glVertex3f( dx2, dy1, 0.f );
	
		//Top-right vertex (corner)
		glTexCoord2f( x2, y2 );
		glVertex3f( dx2, dy2, 0.f );
	
		//Top-left vertex (corner)
		glTexCoord2f( x1, y2 );
		glVertex3f( dx1, dy2, 0.f );
	glEnd();
	
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

int GPU_BlitTransform(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle, float scaleX, float scaleY)
{
	if(src == NULL || dest == NULL)
		return -1;
	
	glPushMatrix();
	
	glRotatef(0, 0, 1, angle);
	
	if(srcrect != NULL)
		glTranslatef(-srcrect->w/2, -srcrect->h/2, 0);
	else
		glTranslatef(-src->w/2, -src->h/2, 0);
	
	int result = GPU_BlitScale(src, srcrect, dest, x, y, scaleX, scaleY);
	
	glPopMatrix();
	
	return result;
}



void GPU_SetBlending(Uint8 enable)
{
	if(enable)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
}


void GPU_SetColor(SDL_Color* color)
{
	if(color == NULL)
		glColor4ub(0, 0, 0, 255);
	else
		glColor4ub(color->r, color->g, color->b, color->unused);
}

void GPU_SetRGB(Uint8 r, Uint8 g, Uint8 b)
{
	glColor4ub(r, g, b, 255);
}

void GPU_SetRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	glColor4ub(r, g, b, a);
}


void GPU_Clear(GPU_Target* target)
{
	if(target == NULL)
		return;
	
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, target->handle);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0,0,target->w, target->h);
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void GPU_Flip(void)
{
	SDL_GL_SwapBuffers();
}

