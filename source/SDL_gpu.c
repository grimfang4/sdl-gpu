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
	
	display = GPU_LoadTarget(screen);
	
	return display;
}

void GPU_Quit(void)
{
	// GPU_FreeTarget(display);  // Don't free the display surface.  SDL_Quit() does it.
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


GPU_Target* GPU_LoadTarget(SDL_Surface* surface)
{
	GPU_Target* result = (GPU_Target*)malloc(sizeof(GPU_Target));
	result->surface = surface;
	
	return result;
}


void GPU_FreeTarget(GPU_Target* target)
{
	if(target == NULL)
		return;
	
	SDL_FreeSurface(target->surface);
	
	free(target);
}



int GPU_Blit(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, SDL_Rect* destrect)
{
	if(src == NULL || dest == NULL)
		return -1;
	
	
	// Bind the texture to which subsequent calls refer
	glBindTexture( GL_TEXTURE_2D, src->handle );
	
	float x1, y1, x2, y2;
	if(srcrect == NULL)
	{
		x1 = 0;
		y1 = 0;
		x2 = 1;
		y2 = 1;
	}
	else
	{
		x1 = srcrect->x/(float)src->w;
		y1 = srcrect->y/(float)src->h;
		x2 = (srcrect->x + srcrect->w)/(float)src->w;
		y2 = (srcrect->y + srcrect->h)/(float)src->h;
	}
	float dx1, dy1, dx2, dy2;
	if(destrect == NULL)
	{
		dx1 = 0;
		dy1 = 0;
		dx2 = x2;
		dy2 = y2;
	}
	else
	{
		dx1 = destrect->x;
		dy1 = destrect->y;
		dx2 = destrect->x + destrect->w;
		dy2 = destrect->y + destrect->h;
	}
	
	glBegin( GL_QUADS );
		//Bottom-left vertex (corner)
		glTexCoord2f( x1, y1 );
		glVertex3f( destrect->x, destrect->y, 0.0f );
	
		//Bottom-right vertex (corner)
		glTexCoord2f( x2, y1 );
		glVertex3f( destrect->x + destrect->w, destrect->y, 0.f );
	
		//Top-right vertex (corner)
		glTexCoord2f( x2, y2 );
		glVertex3f( destrect->x + destrect->w, destrect->y + destrect->h, 0.f );
	
		//Top-left vertex (corner)
		glTexCoord2f( x1, y2 );
		glVertex3f( destrect->x, destrect->y + destrect->h, 0.f );
	glEnd();
}

void GPU_Flip(void)
{
	SDL_GL_SwapBuffers();
}

