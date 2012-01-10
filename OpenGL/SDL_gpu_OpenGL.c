#include "SDL_gpu_OpenGL.h"
#include "SDL_opengl.h"
#include "SOIL.h"

#ifdef WINDOWS
#include "windows.h"
#define GL_EXT_LOAD wglGetProcAddress
#else
#include "GL/glx.h"
#define GL_EXT_LOAD glXGetProcAddress
#endif

PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT = NULL;

static GPU_Target* Init(GPU_Renderer* renderer, Uint16 w, Uint16 h, Uint32 flags)
{
	if(glGenFramebuffersEXT == NULL)
	{
		glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) GL_EXT_LOAD((const GLubyte*)"glGenFramebuffersEXT");
		glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) GL_EXT_LOAD((const GLubyte*)"glBindFramebufferEXT");
		glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) GL_EXT_LOAD((const GLubyte*)"glFramebufferTexture2DEXT");
		glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) GL_EXT_LOAD((const GLubyte*)"glCheckFramebufferStatusEXT");
		glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC) GL_EXT_LOAD((const GLubyte*)"glDeleteFramebuffersEXT");
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

	
	if(renderer->display == NULL)
		renderer->display = (GPU_Target*)malloc(sizeof(GPU_Target));
	
	renderer->display->data = (TargetData_OpenGL*)malloc(sizeof(TargetData_OpenGL));

	((TargetData_OpenGL*)renderer->display->data)->handle = 0;
	renderer->display->w = screen->w;
	renderer->display->h = screen->h;
	renderer->display->clip_rect.x = 0;
	renderer->display->clip_rect.y = 0;
	renderer->display->clip_rect.w = screen->w;
	renderer->display->clip_rect.h = screen->h;
	
	return renderer->display;
}

void Quit(GPU_Renderer* renderer)
{
	free(renderer->display);
	renderer->display = NULL;
}

GPU_Image* LoadImage(GPU_Renderer* renderer, const char* filename)
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
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	
	GPU_Image* result = (GPU_Image*)malloc(sizeof(GPU_Image));
	ImageData_OpenGL* data = (ImageData_OpenGL*)malloc(sizeof(ImageData_OpenGL));
	result->data = data;
	data->handle = texture;
	data->format = texture_format;
	
	result->w = w;
	result->h = h;
	
	return result;
}

void FreeImage(GPU_Renderer* renderer, GPU_Image* image)
{
	if(image == NULL)
		return;
	
	glDeleteTextures( 1, &((ImageData_OpenGL*)image->data)->handle);
	free(image->data);
	free(image);
}

GPU_Target* GetDisplayTarget(GPU_Renderer* renderer)
{
	return renderer->display;
}


GPU_Target* LoadTarget(GPU_Renderer* renderer, GPU_Image* image)
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
	result->w = image->w;
	result->h = image->h;
	
	result->clip_rect.x = 0;
	result->clip_rect.y = 0;
	result->clip_rect.w = image->w;
	result->clip_rect.h = image->h;
	
	return result;
}



void FreeTarget(GPU_Renderer* renderer, GPU_Target* target)
{
	if(target == NULL || target == renderer->display)
		return;
	
	glDeleteFramebuffersEXT(1, &((TargetData_OpenGL*)target->data)->handle);
	
	free(target);
}



int Blit(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y)
{
	if(src == NULL || dest == NULL)
		return -1;
	
	
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
	
	if(dest != renderer->display)
	{
		dy1 = renderer->display->h - y;
		dy2 = renderer->display->h - y;
		
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
	
	if(doClip)
	{
		glDisable(GL_SCISSOR_TEST);
	}
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	
	return 0;
}


int BlitRotate(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle)
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

int BlitScale(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float scaleX, float scaleY)
{
	if(src == NULL || dest == NULL)
		return -1;
	
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
	
	if(dest != renderer->display)
	{
		dy1 = renderer->display->h - y;
		dy2 = renderer->display->h - y;
		
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
	
	if(doClip)
	{
		glDisable(GL_SCISSOR_TEST);
	}
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	
	return 0;
}

int BlitTransform(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle, float scaleX, float scaleY)
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



void SetBlending(GPU_Renderer* renderer, Uint8 enable)
{
	if(enable)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
}


void SetRGBA(GPU_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	glColor4ub(r, g, b, a);
}



void MakeRGBTransparent(GPU_Renderer* renderer, GPU_Image* image, Uint8 r, Uint8 g, Uint8 b)
{
	if(image == NULL)
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










void Clear(GPU_Renderer* renderer, GPU_Target* target)
{
	if(target == NULL)
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
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	if(doClip)
	{
		glDisable(GL_SCISSOR_TEST);
	}
	
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void Flip(GPU_Renderer* renderer)
{
	SDL_GL_SwapBuffers();
}





GPU_Renderer* GPU_CreateRenderer_OpenGL(void)
{
	GPU_Renderer* renderer = (GPU_Renderer*)malloc(sizeof(GPU_Renderer));
	
	renderer->id = "OpenGL";
	renderer->display = NULL;
	
	renderer->data = (RendererData_OpenGL*)malloc(sizeof(RendererData_OpenGL));
	
	renderer->Init = &Init;
	renderer->Quit = &Quit;
	renderer->LoadImage = &LoadImage;
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
	
	renderer->MakeRGBTransparent = &MakeRGBTransparent;
	
	renderer->Clear = &Clear;
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
