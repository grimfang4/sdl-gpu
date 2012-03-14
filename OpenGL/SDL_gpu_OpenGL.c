// Hacks to fix compile errors due to polluted namespace
#ifdef _WIN32
#define _WINUSER_H
#define _WINGDI_H
#endif

#include "SDL_gpu_OpenGL_internal.h"
#include "SOIL.h"
#include <math.h>

#ifdef _WIN32
#define GL_EXT_LOAD wglGetProcAddress
#define GL_STR_CAST LPCSTR
#else
#include "GL/glx.h"
#define GL_EXT_LOAD glXGetProcAddress
#define GL_STR_CAST const GLubyte*
#endif

PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT = NULL;

static GPU_Target* Init(GPU_Renderer* renderer, Uint16 w, Uint16 h, Uint32 flags)
{
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
    
	if(glGenFramebuffersEXT == NULL)
	{
		glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) GL_EXT_LOAD((GL_STR_CAST)"glGenFramebuffersEXT");
		glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) GL_EXT_LOAD((GL_STR_CAST)"glBindFramebufferEXT");
		glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) GL_EXT_LOAD((GL_STR_CAST)"glFramebufferTexture2DEXT");
		glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) GL_EXT_LOAD((GL_STR_CAST)"glCheckFramebufferStatusEXT");
		glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC) GL_EXT_LOAD((GL_STR_CAST)"glDeleteFramebuffersEXT");
	}
	
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
	
	glEnable(GL_BLEND);
    glTranslatef(0.375f, 0.375f, 0.0f);

	
	if(renderer->display == NULL)
		renderer->display = (GPU_Target*)malloc(sizeof(GPU_Target));
	
	renderer->display->data = (TargetData_OpenGL*)malloc(sizeof(TargetData_OpenGL));

	((TargetData_OpenGL*)renderer->display->data)->handle = 0;
	renderer->display->renderer = renderer;
	renderer->display->w = screen->w;
	renderer->display->h = screen->h;
	renderer->display->clip_rect.x = 0;
	renderer->display->clip_rect.y = 0;
	renderer->display->clip_rect.w = screen->w;
	renderer->display->clip_rect.h = screen->h;
	
	return renderer->display;
}

static void Quit(GPU_Renderer* renderer)
{
	free(renderer->display);
	renderer->display = NULL;
}

static GPU_Image* CreateImage(GPU_Renderer* renderer, Uint16 w, Uint16 h, Uint8 bits_per_pixel)
{
	GLuint texture;
	GLint texture_format;
	int channels = 4;
	switch(bits_per_pixel)
	{
		case 8: channels = 3;
		break;
		case 16: channels = 3;
		break;
		case 24: channels = 3;
		break;
	}
	
	unsigned char* pixels = (unsigned char*)malloc(w*h*channels);
	memset(pixels, 0, w*h*channels);
	
	texture = SOIL_create_OGL_texture(pixels, w, h, channels, 0, 0);
	if(texture == 0)
	{
		free(pixels);
		return NULL;
	}
	
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &texture_format);
	
	// Set the texture's stretching properties
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	GPU_Image* result = (GPU_Image*)malloc(sizeof(GPU_Image));
	ImageData_OpenGL* data = (ImageData_OpenGL*)malloc(sizeof(ImageData_OpenGL));
	result->data = data;
	result->renderer = renderer;
	data->handle = texture;
	data->format = texture_format;
	
	result->w = w;
	result->h = h;
	
	return result;
}

static GPU_Image* LoadImage(GPU_Renderer* renderer, const char* filename)
{
	
	GLuint texture;			// This is a handle to our texture object
	GLint texture_format;
	GLint w, h;
	
	
	texture = SOIL_load_OGL_texture(filename, SOIL_LOAD_AUTO, 0, 0);
	if(texture == 0)
		return NULL;
	
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &texture_format);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	
	// Set the texture's stretching properties
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	
	GPU_Image* result = (GPU_Image*)malloc(sizeof(GPU_Image));
	ImageData_OpenGL* data = (ImageData_OpenGL*)malloc(sizeof(ImageData_OpenGL));
	result->data = data;
	result->renderer = renderer;
	data->handle = texture;
	data->format = texture_format;
	
	result->w = w;
	result->h = h;
	
	return result;
}

static GPU_Image* CopyImage(GPU_Renderer* renderer, GPU_Image* image)
{
	GLboolean old_blend;
	glGetBooleanv(GL_BLEND, &old_blend);
	if(old_blend)
		renderer->SetBlending(renderer, 0);
	
	// FIXME: GPU_Image needs a bits_per_pixel or whatever (bytes_per_pixel, channels, etc.) so the format can be copied
	GPU_Image* result = renderer->CreateImage(renderer, image->w, image->h, 32);
	
	GPU_Target* tgt = renderer->LoadTarget(renderer, result);
	renderer->Blit(renderer, image, NULL, tgt, tgt->w/2, tgt->h/2);
	renderer->FreeTarget(renderer, tgt);
	
	if(old_blend)
		renderer->SetBlending(renderer, 1);
	
	return result;
}

static GPU_Image* CopyImageFromSurface(GPU_Renderer* renderer, SDL_Surface* surface)
{
	if(surface == NULL)
		return NULL;
	
	// From gpwiki.org
	GLuint texture;			// This is a handle to our texture object
	GLenum texture_format;
	GLint  nOfColors;
	int w, h;
	
	// Check that the image's width is a power of 2
	/*if ( (surface->w & (surface->w - 1)) != 0 ) {
		printf("warning: image.bmp's width is not a power of 2\n");
	}

	// Also check if the height is a power of 2
	if ( (surface->h & (surface->h - 1)) != 0 ) {
		printf("warning: image.bmp's height is not a power of 2\n");
	}*/

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
			//printf("warning: the image is not truecolor..  this will probably break\n");
			return NULL;
	}

	// Have OpenGL generate a texture object handle for us
	glGenTextures( 1, &texture );

	// Bind the texture object
	glBindTexture( GL_TEXTURE_2D, texture );

	// Set the texture's stretching properties
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Edit the texture object's image data using the information SDL_Surface gives us
	glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
					texture_format, GL_UNSIGNED_BYTE, surface->pixels );
	
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	
	GPU_Image* result = (GPU_Image*)malloc(sizeof(GPU_Image));
	ImageData_OpenGL* data = (ImageData_OpenGL*)malloc(sizeof(ImageData_OpenGL));
	result->data = data;
	result->renderer = renderer;
	data->handle = texture;
	data->format = texture_format;
	
	result->w = w;
	result->h = h;
	
	return result;
}


static void FreeImage(GPU_Renderer* renderer, GPU_Image* image)
{
	if(image == NULL)
		return;
	
	glDeleteTextures( 1, &((ImageData_OpenGL*)image->data)->handle);
	free(image->data);
	free(image);
}

static GPU_Target* GetDisplayTarget(GPU_Renderer* renderer)
{
	return renderer->display;
}


static GPU_Target* LoadTarget(GPU_Renderer* renderer, GPU_Image* image)
{
	GLuint handle;
	// Create framebuffer object
	glGenFramebuffersEXT(1, &handle);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, handle);
	
	// Attach the texture to it
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, ((ImageData_OpenGL*)image->data)->handle, 0); // 502
	
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
		return NULL;
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	
	GPU_Target* result = (GPU_Target*)malloc(sizeof(GPU_Target));
	TargetData_OpenGL* data = (TargetData_OpenGL*)malloc(sizeof(TargetData_OpenGL));
	result->data = data;
	data->handle = handle;
	data->format = ((ImageData_OpenGL*)image->data)->format;
	result->renderer = renderer;
	result->w = image->w;
	result->h = image->h;
	
	result->clip_rect.x = 0;
	result->clip_rect.y = 0;
	result->clip_rect.w = image->w;
	result->clip_rect.h = image->h;
	
	return result;
}



static void FreeTarget(GPU_Renderer* renderer, GPU_Target* target)
{
	if(target == NULL || target == renderer->display)
		return;
	
	glDeleteFramebuffersEXT(1, &((TargetData_OpenGL*)target->data)->handle);
	
	free(target);
}



static int Blit(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y)
{
	if(src == NULL || dest == NULL)
		return -1;
	if(renderer != src->renderer || renderer != dest->renderer)
		return -2;
	
	
	// Bind the texture to which subsequent calls refer
	glBindTexture( GL_TEXTURE_2D, ((ImageData_OpenGL*)src->data)->handle );
	
	// Bind the FBO
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ((TargetData_OpenGL*)dest->data)->handle);
	
	Uint8 doClip = (dest->clip_rect.x > 0 || dest->clip_rect.y > 0 || dest->clip_rect.w < dest->w || dest->clip_rect.h < dest->h);
	if(doClip)
	{
		glEnable(GL_SCISSOR_TEST);
		int y = (renderer->display == dest? renderer->display->h - (dest->clip_rect.y + dest->clip_rect.h) : dest->clip_rect.y);
		glScissor(dest->clip_rect.x, y, dest->clip_rect.w, dest->clip_rect.h);
	}
	
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
		dx1 = x - src->w/2;
		dy1 = y - src->h/2;
		dx2 = x + src->w/2;
		dy2 = y + src->h/2;
	}
	else
	{
		x1 = srcrect->x/(float)src->w;
		y1 = srcrect->y/(float)src->h;
		x2 = (srcrect->x + srcrect->w)/(float)src->w;
		y2 = (srcrect->y + srcrect->h)/(float)src->h;
		dx1 = x - srcrect->w/2;
		dy1 = y - srcrect->h/2;
		dx2 = x + srcrect->w/2;
		dy2 = y + srcrect->h/2;
	}
	
	if(dest != renderer->display)
	{
		
		if(srcrect == NULL)
		{
			dy1 = y - src->h/2;
			dy2 = y + src->h/2;
		}
		else
		{
			dy1 = y - srcrect->h/2;
			dy2 = y + srcrect->h/2;
		}
		
		dy1 = renderer->display->h - dy1;
		dy2 = renderer->display->h - dy2;
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
	
	if(doClip)
	{
		glDisable(GL_SCISSOR_TEST);
	}
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	
	return 0;
}


static int BlitRotate(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle)
{
	if(src == NULL || dest == NULL)
		return -1;
	if(renderer != src->renderer || renderer != dest->renderer)
		return -2;
	
	glPushMatrix();
	
	glTranslatef(x, (dest == renderer->display? y : renderer->display->h-y), 0);
	glRotatef(angle, 0, 0, 1);
	glTranslatef(0, (dest == renderer->display? 0 : -renderer->display->h), 0);
	
	int result = GPU_Blit(src, srcrect, dest, 0, 0);
	
	glPopMatrix();
	
	return result;
}

static int BlitScale(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float scaleX, float scaleY)
{
	if(src == NULL || dest == NULL)
		return -1;
	if(renderer != src->renderer || renderer != dest->renderer)
		return -2;
	
	glPushMatrix();
	
	glTranslatef(x, (dest == renderer->display? y : renderer->display->h*(1-scaleY) - y), 0);
	glScalef(scaleX, scaleY, 1.0f);
	
	int result = GPU_Blit(src, srcrect, dest, 0, 0);
	
	glPopMatrix();
	
	return result;
}

static int BlitTransform(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle, float scaleX, float scaleY)
{
	if(src == NULL || dest == NULL)
		return -1;
	if(renderer != src->renderer || renderer != dest->renderer)
		return -2;
	
	glPushMatrix();
	
	glTranslatef(x, (dest == renderer->display? y : renderer->display->h - y), 0);
	glRotatef(angle, 0, 0, 1);
	glScalef(scaleX, scaleY, 1.0f);
	glTranslatef(0, (dest == renderer->display? 0 : -renderer->display->h), 0);
	
	int result = GPU_Blit(src, srcrect, dest, 0, 0);
	
	glPopMatrix();
	
	return result;
}



static void SetBlending(GPU_Renderer* renderer, Uint8 enable)
{
	if(enable)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
}


static void SetRGBA(GPU_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	glColor4ub(r, g, b, a);
}


static void ReplaceRGB(GPU_Renderer* renderer, GPU_Image* image, Uint8 from_r, Uint8 from_g, Uint8 from_b, Uint8 to_r, Uint8 to_g, Uint8 to_b)
{
	if(image == NULL)
		return;
	if(renderer != image->renderer)
		return;
	
	glBindTexture( GL_TEXTURE_2D, ((ImageData_OpenGL*)image->data)->handle );

	GLint textureWidth, textureHeight;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);

	// FIXME: Does not take into account GL_PACK_ALIGNMENT
	GLubyte *buffer = (GLubyte *)malloc(textureWidth*textureHeight*4);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	int x,y,i;
	for(y = 0; y < textureHeight; y++)
	{
		for(x = 0; x < textureWidth; x++)
		{
			i = ((y*textureWidth) + x)*4;
			if(buffer[i] == from_r && buffer[i+1] == from_g && buffer[i+2] == from_b)
			{
				buffer[i] = to_r;
				buffer[i+1] = to_g;
				buffer[i+2] = to_b;
			}
		}
	}
	
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
 

	free(buffer);
}



static void MakeRGBTransparent(GPU_Renderer* renderer, GPU_Image* image, Uint8 r, Uint8 g, Uint8 b)
{
	if(image == NULL)
		return;
	if(renderer != image->renderer)
		return;
	
	glBindTexture( GL_TEXTURE_2D, ((ImageData_OpenGL*)image->data)->handle );

	GLint textureWidth, textureHeight;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);

	// FIXME: Does not take into account GL_PACK_ALIGNMENT
	GLubyte *buffer = (GLubyte *)malloc(textureWidth*textureHeight*4);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	int x,y,i;
	for(y = 0; y < textureHeight; y++)
	{
		for(x = 0; x < textureWidth; x++)
		{
			i = ((y*textureWidth) + x)*4;
			if(buffer[i] == r && buffer[i+1] == g && buffer[i+2] == b)
				buffer[i+3] = 0;
		}
	}
	
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
 

	free(buffer);
}

#define MIN(a,b,c) (a<b && a<c? a : (b<c? b : c))
#define MAX(a,b,c) (a>b && a>c? a : (b>c? b : c))


static void rgb_to_hsv(int red, int green, int blue, int* hue, int* sat, int* val)
{
	float r = red/255.0f;
	float g = green/255.0f;
	float b = blue/255.0f;
	
	float h, s, v;
	
	float min, max, delta;
	min = MIN( r, g, b );
	max = MAX( r, g, b );
	
	v = max;				// v
	delta = max - min;
	if( max != 0 && min != max)
	{
		s = delta / max;		// s
		
		if( r == max )
			h = ( g - b ) / delta;		// between yellow & magenta
		else if( g == max )
			h = 2 + ( b - r ) / delta;	// between cyan & yellow
		else
			h = 4 + ( r - g ) / delta;	// between magenta & cyan
		h *= 60;				// degrees
		if( h < 0 )
			h += 360;
	}
	else {
		// r = g = b = 0		// s = 0, v is undefined
		s = 0;
		h = 0;// really undefined: -1
	}
	
	*hue = h * 256.0f/360.0f;
	*sat = s * 255;
	*val = v * 255;
}

static void hsv_to_rgb(int hue, int sat, int val, int* r, int* g, int* b)
{
	float h = hue/255.0f;
    float s = sat/255.0f;
    float v = val/255.0f;
    
    int H = floor(h*5.999f);
    float chroma = v*s;
    float x = chroma * (1 - fabs(fmod(h*5.999f, 2) - 1));
    
    unsigned char R = 0, G = 0, B = 0;
    switch(H)
    {
        case 0:
            R = 255*chroma;
            G = 255*x;
        break;
        case 1:
            R = 255*x;
            G = 255*chroma;
        break;
        case 2:
            G = 255*chroma;
            B = 255*x;
        break;
        case 3:
            G = 255*x;
            B = 255*chroma;
        break;
        case 4:
            R = 255*x;
            B = 255*chroma;
        break;
        case 5:
            R = 255*chroma;
            B = 255*x;
        break;
    }
    
    unsigned char m = 255*(v - chroma);
    
	*r = R+m;
	*g = G+m;
	*b = B+m;
}

static int clamp(int value, int low, int high)
{
	if(value <= low)
		return low;
	if(value >= high)
		return high;
	return value;
}


static void ShiftHSV(GPU_Renderer* renderer, GPU_Image* image, int hue, int saturation, int value)
{
	if(image == NULL)
		return;
	if(renderer != image->renderer)
		return;
	
	glBindTexture( GL_TEXTURE_2D, ((ImageData_OpenGL*)image->data)->handle );

	GLint textureWidth, textureHeight;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);

	// FIXME: Does not take into account GL_PACK_ALIGNMENT
	GLubyte *buffer = (GLubyte *)malloc(textureWidth*textureHeight*4);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	int x,y,i;
	for(y = 0; y < textureHeight; y++)
	{
		for(x = 0; x < textureWidth; x++)
		{
			i = ((y*textureWidth) + x)*4;
			
			if(buffer[i+3] == 0)
				continue;
			
			int r = buffer[i];
			int g = buffer[i+1];
			int b = buffer[i+2];
			int h, s, v;
			rgb_to_hsv(r, g, b, &h, &s, &v);
			h += hue;
			s += saturation;
			v += value;
			// Wrap hue
			while(h < 0)
				h += 256;
			while(h > 255)
				h -= 256;
			
			// Clamp
			s = clamp(s, 0, 255);
			v = clamp(v, 0, 255);
			
			hsv_to_rgb(h, s, v, &r, &g, &b);
			
			buffer[i] = r;
			buffer[i+1] = g;
			buffer[i+2] = b;
		}
	}
	
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
 

	free(buffer);
}


static SDL_Color GetPixel(GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y)
{
	SDL_Color result = {0,0,0,0};
	if(target == NULL)
		return result;
	if(renderer != target->renderer)
		return result;
	if(x < 0 || y < 0 || x >= target->w || y >= target->h)
		return result;
	
	// Bind the FBO
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ((TargetData_OpenGL*)target->data)->handle);
	
	unsigned char pixels[4];
	glReadPixels(x, y, 1, 1, ((TargetData_OpenGL*)target->data)->format, GL_UNSIGNED_BYTE, pixels);
	
	result.r = pixels[0];
	result.g = pixels[1];
	result.b = pixels[2];
	result.unused = pixels[3];
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	
	return result;
}

static void SetImageFilter(GPU_Renderer* renderer, GPU_Image* image, GPU_FilterEnum filter)
{
	if(image == NULL)
		return;
	if(renderer != image->renderer)
		return;
	
	glBindTexture( GL_TEXTURE_2D, ((ImageData_OpenGL*)image->data)->handle );
	
	if(filter == GPU_NEAREST)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else if(filter == GPU_LINEAR)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}









static void Clear(GPU_Renderer* renderer, GPU_Target* target)
{
	if(target == NULL)
		return;
	if(renderer != target->renderer)
		return;
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ((TargetData_OpenGL*)target->data)->handle);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0,0,target->w, target->h);
	
	Uint8 doClip = (target->clip_rect.x > 0 || target->clip_rect.y > 0 || target->clip_rect.w < target->w || target->clip_rect.h < target->h);
	if(doClip)
	{
		glEnable(GL_SCISSOR_TEST);
		int y = (renderer->display == target? renderer->display->h - (target->clip_rect.y + target->clip_rect.h) : target->clip_rect.y);
		glScissor(target->clip_rect.x, y, target->clip_rect.w, target->clip_rect.h);
	}
	
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);
	glPopAttrib();
	
	if(doClip)
	{
		glDisable(GL_SCISSOR_TEST);
	}
	
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}


static void ClearRGBA(GPU_Renderer* renderer, GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(target == NULL)
		return;
	if(renderer != target->renderer)
		return;
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, ((TargetData_OpenGL*)target->data)->handle);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0,0,target->w, target->h);
	
	Uint8 doClip = (target->clip_rect.x > 0 || target->clip_rect.y > 0 || target->clip_rect.w < target->w || target->clip_rect.h < target->h);
	if(doClip)
	{
		glEnable(GL_SCISSOR_TEST);
		int y = (renderer->display == target? renderer->display->h - (target->clip_rect.y + target->clip_rect.h) : target->clip_rect.y);
		glScissor(target->clip_rect.x, y, target->clip_rect.w, target->clip_rect.h);
	}
	
	glClearColor(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	if(doClip)
	{
		glDisable(GL_SCISSOR_TEST);
	}
	
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

static void Flip(GPU_Renderer* renderer)
{
	SDL_GL_SwapBuffers();
}





GPU_Renderer* GPU_CreateRenderer_OpenGL(void)
{
	GPU_Renderer* renderer = (GPU_Renderer*)malloc(sizeof(GPU_Renderer));
	if(renderer == NULL)
		return NULL;
	
	memset(renderer, 0, sizeof(GPU_Renderer));
	
	renderer->id = "OpenGL";
	renderer->display = NULL;
	
	renderer->data = (RendererData_OpenGL*)malloc(sizeof(RendererData_OpenGL));
	
	renderer->Init = &Init;
	renderer->Quit = &Quit;
	renderer->CreateImage = &CreateImage;
	renderer->LoadImage = &LoadImage;
	renderer->CopyImage = &CopyImage;
	renderer->CopyImageFromSurface = &CopyImageFromSurface;
	renderer->FreeImage = &FreeImage;
	
	renderer->GetDisplayTarget = &GetDisplayTarget;
	renderer->LoadTarget = &LoadTarget;
	renderer->FreeTarget = &FreeTarget;
	
	renderer->Blit = &Blit;
	renderer->BlitRotate = &BlitRotate;
	renderer->BlitScale = &BlitScale;
	renderer->BlitTransform = &BlitTransform;
	
	renderer->SetBlending = &SetBlending;
	renderer->SetRGBA = &SetRGBA;
	
	renderer->ReplaceRGB = &ReplaceRGB;
	renderer->MakeRGBTransparent = &MakeRGBTransparent;
	renderer->ShiftHSV = &ShiftHSV;
	renderer->GetPixel = &GetPixel;
	renderer->SetImageFilter = &SetImageFilter;
	
	renderer->Clear = &Clear;
	renderer->ClearRGBA = &ClearRGBA;
	renderer->Flip = &Flip;
	
	return renderer;
}

void GPU_FreeRenderer_OpenGL(GPU_Renderer* renderer)
{
	if(renderer == NULL)
		return;
	
	free(renderer->data);
	free(renderer);
}
