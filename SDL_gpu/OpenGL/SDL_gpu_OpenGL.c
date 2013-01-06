// Hacks to fix compile errors due to polluted namespace
#ifdef _WIN32
#define _WINUSER_H
#define _WINGDI_H
#endif

#include "SDL_gpu_OpenGL_internal.h"
#include "SOIL.h"
#include <math.h>

static Uint8 checkExtension(const char* str)
{
	if(!glewIsExtensionSupported(str))
	{
		fprintf(stderr, "Error: %s is not supported.\n", str);
		return 0;
	}
	return 1;
}

static GPU_Target* Init(GPU_Renderer* renderer, Uint16 w, Uint16 h, Uint32 flags)
{
    #ifdef SDL_GPU_USE_SDL2
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    #else
	Uint8 useDoubleBuffering = flags & SDL_DOUBLEBUF;
	if(useDoubleBuffering)
	{
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		flags &= ~SDL_DOUBLEBUF;
	}
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	flags |= SDL_OPENGL;
	#endif
	
	
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        
    #ifdef SDL_GPU_USE_SDL2
        SDL_Window* window = SDL_CreateWindow("",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            w, h,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
        
        ((RendererData_OpenGL*)renderer->data)->window = window;
        if(window == NULL)
            return NULL;
        
        SDL_GetWindowSize(window, &renderer->window_w, &renderer->window_h);
        
        SDL_GLContext context = SDL_GL_CreateContext(window);
        ((RendererData_OpenGL*)renderer->data)->context = context;
	#else
        SDL_Surface* screen = SDL_SetVideoMode(w, h, 0, flags);
        
        if(screen == NULL)
            return NULL;
        
        renderer->window_w = screen->w;
        renderer->window_h = screen->h;
    #endif
    
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	
	checkExtension("GL_EXT_framebuffer_object");
	checkExtension("GL_ARB_framebuffer_object"); // glGenerateMipmap
	checkExtension("GL_EXT_framebuffer_blit");

	glEnable( GL_TEXTURE_2D );
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	
	glViewport( 0, 0, renderer->window_w, renderer->window_h);
	
	glClear( GL_COLOR_BUFFER_BIT );
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	glOrtho(0.0f, renderer->window_w, renderer->window_h, 0.0f, -1.0f, 1.0f);
	
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	
	glEnable( GL_TEXTURE_2D );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable(GL_BLEND);

	
	if(renderer->display == NULL)
		renderer->display = (GPU_Target*)malloc(sizeof(GPU_Target));
	
	renderer->display->data = (TargetData_OpenGL*)malloc(sizeof(TargetData_OpenGL));

	((TargetData_OpenGL*)renderer->display->data)->handle = 0;
	renderer->display->renderer = renderer;
	renderer->display->w = renderer->window_w;
	renderer->display->h = renderer->window_h;
	
	renderer->display->useClip = 0;
	renderer->display->clipRect.x = 0;
	renderer->display->clipRect.y = 0;
        renderer->display->clipRect.w = renderer->display->w;
        renderer->display->clipRect.h = renderer->display->h;

        return renderer->display;
}


static void SetAsCurrent(GPU_Renderer* renderer)
{
    #ifdef SDL_GPU_USE_SDL2
    SDL_GL_MakeCurrent(((RendererData_OpenGL*)renderer->data)->window, ((RendererData_OpenGL*)renderer->data)->context);
    #endif
}

// FIXME: Rename to SetWindowResolution
static int SetDisplayResolution(GPU_Renderer* renderer, Uint16 w, Uint16 h)
{
	if(renderer->display == NULL)
		return 0;
	
	#ifdef SDL_GPU_USE_SDL2
	SDL_SetWindowSize(((RendererData_OpenGL*)renderer->data)->window, w, h);
	SDL_GetWindowSize(((RendererData_OpenGL*)renderer->data)->window, &renderer->window_w, &renderer->window_h);
	#else
    SDL_Surface* surf = SDL_GetVideoSurface();
	Uint32 flags = surf->flags;
	
	
	SDL_Surface* screen = SDL_SetVideoMode(w, h, 0, flags);
	// There's a bug in SDL.  This is a workaround.  Let's resize again:
	screen = SDL_SetVideoMode(w, h, 0, flags);
	
	if(screen == NULL)
		return 0;
    
    renderer->window_w = screen->w;
    renderer->window_h = screen->h;
    #endif
    
	Uint16 virtualW = renderer->display->w;
	Uint16 virtualH = renderer->display->h;
    
	glEnable( GL_TEXTURE_2D );
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	
	glViewport( 0, 0, w, h);
	
	glClear( GL_COLOR_BUFFER_BIT );
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	glOrtho(0.0f, virtualW, virtualH, 0.0f, -1.0f, 1.0f);
	
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	
	glEnable( GL_TEXTURE_2D );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable(GL_BLEND);

	// Update display
	GPU_ClearClip(renderer->display);
	//renderer->display->clipRect.w = screen->w;
	//renderer->display->clipRect.h = screen->h;
	
	return 1;
}

static void SetVirtualResolution(GPU_Renderer* renderer, Uint16 w, Uint16 h)
{
	if(renderer->display == NULL)
		return;
	
	renderer->display->w = w;
	renderer->display->h = h;
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	glOrtho(0.0f, w, h, 0.0f, -1.0f, 1.0f);
	
	glMatrixMode( GL_MODELVIEW );
}

static void Quit(GPU_Renderer* renderer)
{
	free(renderer->display);
	renderer->display = NULL;
}



static int ToggleFullscreen(GPU_Renderer* renderer)
{
    #ifdef SDL_GPU_USE_SDL2
        Uint8 enable = !(SDL_GetWindowFlags(((RendererData_OpenGL*)renderer->data)->window) & SDL_WINDOW_FULLSCREEN);
        
        if(SDL_SetWindowFullscreen(((RendererData_OpenGL*)renderer->data)->window, enable) < 0)
            return 0;
            
        return 1;
    #else
        SDL_Surface* surf = SDL_GetVideoSurface();
        
        if(SDL_WM_ToggleFullScreen(surf))
            return 1;
        
        Uint16 w = surf->w;
        Uint16 h = surf->h;
        surf->flags ^= SDL_FULLSCREEN;
        return SetDisplayResolution(renderer, w, h);
	#endif
}



static GPU_Camera SetCamera(GPU_Renderer* renderer, GPU_Target* screen, GPU_Camera* cam)
{
	if(screen == NULL)
		return renderer->camera;
	
	GPU_Camera result = renderer->camera;
	
	if(cam == NULL)
	{
		renderer->camera.x = 0.0f;
		renderer->camera.y = 0.0f;
		renderer->camera.z = -10.0f;
		renderer->camera.angle = 0.0f;
		renderer->camera.zoom = 1.0f;
	}
	else
	{
		renderer->camera = *cam;
	}
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	// I want to use x, y, and z
	// The default z for objects is 0
	// The default z for the camera is -10. (should be neg)
	
	/*float fieldOfView = 60.0f;
	float fW = screen->w/2;
	float fH = screen->h/2;
	float aspect = fW/fH;
	float zNear = atan(fH)/((float)(fieldOfView / 360.0f * 3.14159f));
	float zFar = 255.0f;
	glFrustum( 0.0f + renderer->camera.x, 2*fW + renderer->camera.x, 2*fH + renderer->camera.y, 0.0f + renderer->camera.y, zNear, zFar );*/
	
	glFrustum(0.0f + renderer->camera.x, screen->w + renderer->camera.x, screen->h + renderer->camera.y, 0.0f + renderer->camera.y, 0.01f, 1.01f);
	
	//glMatrixMode( GL_MODELVIEW );
	//glLoadIdentity();
	
	
	float offsetX = screen->w/2.0f;
	float offsetY = screen->h/2.0f;
	glTranslatef(offsetX, offsetY, -0.01);
	glRotatef(renderer->camera.angle, 0, 0, 1);
	glTranslatef(-offsetX, -offsetY, 0);
	
	glTranslatef(renderer->camera.x + offsetX, renderer->camera.y + offsetY, 0);
	glScalef(renderer->camera.zoom, renderer->camera.zoom, 1.0f);
	glTranslatef(-renderer->camera.x - offsetX, -renderer->camera.y - offsetY, 0);
	
	return result;
}


static GPU_Image* CreateImage(GPU_Renderer* renderer, Uint16 w, Uint16 h, Uint8 channels)
{
	if(channels < 3 || channels > 4)
		return NULL;
	
	GLuint texture;
	GLint texture_format;
	
	unsigned char* pixels = (unsigned char*)malloc(w*h*channels);
	memset(pixels, 0, w*h*channels);
	
        int iw, ih;
        iw = w;
        ih = h;
        texture = SOIL_create_OGL_texture(pixels, &iw, &ih, channels, 0, 0);
        if(texture == 0)
        {
		free(pixels);
		return NULL;
	}
        w = iw;
        h = ih;

        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &texture_format);
        
        // Set the texture's stretching properties
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        
        GPU_Image* result = (GPU_Image*)malloc(sizeof(GPU_Image));
        ImageData_OpenGL* data = (ImageData_OpenGL*)malloc(sizeof(ImageData_OpenGL));
        result->data = data;
        result->renderer = renderer;
        result->channels = channels;
	data->handle = texture;
	data->format = texture_format;
	data->hasMipmaps = 0;
	
	result->w = w;
	result->h = h;
	
	free(pixels);
	
	return result;
}

static GPU_Image* LoadImage(GPU_Renderer* renderer, const char* filename)
{
        
        GLuint texture;                 // This is a handle to our texture object
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
	
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

        
        GPU_Image* result = (GPU_Image*)malloc(sizeof(GPU_Image));
        ImageData_OpenGL* data = (ImageData_OpenGL*)malloc(sizeof(ImageData_OpenGL));
        result->data = data;
        result->renderer = renderer;
        
	int channels = 0;
	switch(texture_format)
	{
	case GL_LUMINANCE:
		channels = 1;
		break;
        case GL_LUMINANCE_ALPHA:
                channels = 2;
                break;
        case GL_BGR:
        case GL_RGB:
                channels = 3;
                break;
        case GL_BGRA:
        case GL_RGBA:
                channels = 4;
                break;
	}
        
        result->channels = channels;
        
        data->handle = texture;
        data->format = texture_format;
        data->hasMipmaps = 0;
        
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
	
	GPU_Image* result = renderer->CreateImage(renderer, image->w, image->h, image->channels);
	
	GPU_Target* tgt = renderer->LoadTarget(renderer, result);
	
	// Clear the clipRect
	SDL_Rect clip;
	Uint8 useClip = tgt->useClip;
	if(useClip)
	{
		clip = tgt->clipRect;
		GPU_ClearClip(tgt);
	}
	
    Uint16 w = renderer->display->w;
    Uint16 h = renderer->display->h;
    
    renderer->SetVirtualResolution(renderer, renderer->window_w, renderer->window_h);
	renderer->Blit(renderer, image, NULL, tgt, tgt->w/2, tgt->h/2);
    renderer->SetVirtualResolution(renderer, w, h);
	
	renderer->FreeTarget(renderer, tgt);
	
	if(useClip)
	{
		tgt->useClip = 1;
		tgt->clipRect = clip;
	}
	
	
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
        
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
        
        GPU_Image* result = (GPU_Image*)malloc(sizeof(GPU_Image));
        ImageData_OpenGL* data = (ImageData_OpenGL*)malloc(sizeof(ImageData_OpenGL));
        result->data = data;
        result->renderer = renderer;
        
	int channels = 0;
	switch(texture_format)
	{
	case GL_LUMINANCE:
		channels = 1;
		break;
        case GL_LUMINANCE_ALPHA:
                channels = 2;
                break;
        case GL_BGR:
        case GL_RGB:
                channels = 3;
                break;
        case GL_BGRA:
        case GL_RGBA:
                channels = 4;
                break;
	}
	
	result->channels = channels;
	
	data->handle = texture;
	data->format = texture_format;
	data->hasMipmaps = 0;
	
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
	if(renderer == NULL || image == NULL)
		return NULL;
	
	GLuint handle;
	// Create framebuffer object
	glGenFramebuffers(1, &handle);
        glBindFramebuffer(GL_FRAMEBUFFER, handle);
        
        // Attach the texture to it
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ((ImageData_OpenGL*)image->data)->handle, 0);
        
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE)
		return NULL;
	
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        GPU_Target* result = (GPU_Target*)malloc(sizeof(GPU_Target));
        TargetData_OpenGL* data = (TargetData_OpenGL*)malloc(sizeof(TargetData_OpenGL));
        result->data = data;
        data->handle = handle;
        data->format = ((ImageData_OpenGL*)image->data)->format;
        result->renderer = renderer;
        result->w = image->w;
        result->h = image->h;
	
	result->useClip = 0;
	result->clipRect.x = 0;
	result->clipRect.y = 0;
	result->clipRect.w = image->w;
	result->clipRect.h = image->h;
	
	return result;
}



static void FreeTarget(GPU_Renderer* renderer, GPU_Target* target)
{
        if(target == NULL || target == renderer->display)
                return;
        
        glDeleteFramebuffers(1, &((TargetData_OpenGL*)target->data)->handle);
        
        free(target->data);
        free(target);
}



static int Blit(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y)
{
	if(src == NULL || dest == NULL)
		return -1;
	if(renderer != src->renderer || renderer != dest->renderer)
		return -2;
        
        
        // Bind the texture to which subsequent calls refer
        glBindTexture( GL_TEXTURE_2D, ((ImageData_OpenGL*)src->data)->handle );
        
        // Bind the FBO
        glBindFramebuffer(GL_FRAMEBUFFER, ((TargetData_OpenGL*)dest->data)->handle);
        
        // Rendering to FBO clips outside of the viewport.  This makes textures larger than the screen viewport to fail drawing completely.
        // However, when the viewport is adjusted to fit the texture, it scales the projected region up to the viewport.
	// This scaling can be fixed by scaling the texture coords.
	// At this point though, textures with smaller dimensions are clipped...
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	
	Uint8 viewScaleX = (dest->w > vp[2]);
	Uint8 viewScaleY = (dest->h > vp[3]);
	float wRatio, hRatio;
	if(viewScaleX || viewScaleY)
	{
		unsigned int w = vp[2];
		unsigned int h = vp[3];
		int destW, destH;
		if(renderer->display == dest)
		{
			GPU_GetDisplayResolution(&destW, &destH);
		}
		else
		{
			destW = dest->w;
			destH = dest->h;
		}
		
		if(viewScaleX)
		{
			wRatio = destW/(float)w;
			w = destW;
		}
		if(viewScaleY)
		{
			hRatio = destH/(float)h;
			h = destH;
		}
		glViewport( 0, 0, w, h);
	}
	
	
	if(dest->useClip)
	{
		glEnable(GL_SCISSOR_TEST);
		int y = (renderer->display == dest? renderer->display->h - (dest->clipRect.y + dest->clipRect.h) : dest->clipRect.y);
		float xFactor = ((float)renderer->window_w)/renderer->display->w;
		float yFactor = ((float)renderer->window_h)/renderer->display->h;
		glScissor(dest->clipRect.x * xFactor, y * yFactor, dest->clipRect.w * xFactor, dest->clipRect.h * yFactor);
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
	
	// Fix the viewport scaling
	if(viewScaleX)
	{
		x1 *= wRatio;
		x2 *= wRatio;
	}
	if(viewScaleY)
	{
		y1 *= hRatio;
		y2 *= hRatio;
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
		glVertex3f( dx2, dy1, 0.0f );
	
		//Top-right vertex (corner)
		glTexCoord2f( x2, y2 );
		glVertex3f( dx2, dy2, 0.0f );
	
		//Top-left vertex (corner)
                glTexCoord2f( x1, y2 );
                glVertex3f( dx1, dy2, 0.0f );
        glEnd();
        
        if(dest->useClip)
        {
		glDisable(GL_SCISSOR_TEST);
	}
	
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	/// restore viewport
	glViewport(vp[0], vp[1], vp[2], vp[3]);
	return 0;
}


static int BlitRotate(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float angle)
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

static int BlitScale(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float scaleX, float scaleY)
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

static int BlitTransform(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float angle, float scaleX, float scaleY)
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

static int BlitTransformX(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float pivot_x, float pivot_y, float angle, float scaleX, float scaleY)
{
	if(src == NULL || dest == NULL)
		return -1;
	if(renderer != src->renderer || renderer != dest->renderer)
		return -2;
	
	glPushMatrix();
	
	if(dest == renderer->display)
	{
		glTranslatef(x + pivot_x, y + pivot_y, 0);
		glRotatef(angle, 0, 0, 1);
		glTranslatef(-pivot_x, -pivot_y, 0);
		glScalef(scaleX, scaleY, 1.0f);
	}
	else
	{
		glTranslatef(x + pivot_x, renderer->display->h - y - pivot_y, 0);
		glRotatef(-angle, 0, 0, 1);
		glTranslatef(-pivot_x, pivot_y, 0);
		glScalef(scaleX, scaleY, 1.0f);
		glTranslatef(0, -renderer->display->h, 0);
	}
	
	int result = GPU_Blit(src, srcrect, dest, 0, 0);
	
	glPopMatrix();
	
	return result;
}

static int BlitTransformMatrix(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float* matrix3x3)
{
	if(src == NULL || dest == NULL)
		return -1;
	if(renderer != src->renderer || renderer != dest->renderer)
		return -2;
	
	glPushMatrix();
	
	if(dest == renderer->display)
	{
		// column-major 3x3 to column-major 4x4 (and scooting the translations to the homogeneous column)
		float matrix[16] = {matrix3x3[0],matrix3x3[1],matrix3x3[2],0,matrix3x3[3],matrix3x3[4],matrix3x3[5],0,0,0,matrix3x3[8],0,matrix3x3[6],matrix3x3[7],0,1};
		glTranslatef(x, y, 0);
		glMultMatrixf(matrix);
	}
	else
	{
		// column-major 3x3 to column-major 4x4 (and scooting the translations to the homogeneous column)
		float matrix[16] = {matrix3x3[0],matrix3x3[1],matrix3x3[2],0,matrix3x3[3],matrix3x3[4],matrix3x3[5],0,0,0,matrix3x3[8],0,matrix3x3[6],matrix3x3[7],0,1};
		glTranslatef(x, renderer->display->h - y, 0);
		glScalef(1, -1, 1);
		glMultMatrixf(matrix);
		glScalef(1, -1, 1);
		glTranslatef(0, -renderer->display->h, 0);
	}
	
	int result = GPU_Blit(src, srcrect, dest, 0, 0);
	
	glPopMatrix();
	
	return result;
}

static float SetZ(GPU_Renderer* renderer, float z)
{
        if(renderer == NULL)
                return 0.0f;
        
        float oldZ = ((RendererData_OpenGL*)(renderer->data))->z;
        ((RendererData_OpenGL*)(renderer->data))->z = z;
        
        return oldZ;
}

static float GetZ(GPU_Renderer* renderer)
{
        if(renderer == NULL)
                return 0.0f;
        return ((RendererData_OpenGL*)(renderer->data))->z;
}

static void GenerateMipmaps(GPU_Renderer* renderer, GPU_Image* image)
{
        if(image == NULL)
                return;
        
        glBindTexture( GL_TEXTURE_2D, ((ImageData_OpenGL*)image->data)->handle );
        glGenerateMipmap(GL_TEXTURE_2D);
        ((ImageData_OpenGL*)image->data)->hasMipmaps = 1;
        
        GLint filter;
        glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &filter);
	if(filter == GL_LINEAR)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
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
	glColor4f(r/255.01f, g/255.01f, b/255.01f, a/255.01f);
}


static void ReplaceRGB(GPU_Renderer* renderer, GPU_Image* image, Uint8 from_r, Uint8 from_g, Uint8 from_b, Uint8 to_r, Uint8 to_g, Uint8 to_b)
{
	if(image == NULL || image->channels < 3)
		return;
        if(renderer != image->renderer)
                return;
        
        glBindTexture( GL_TEXTURE_2D, ((ImageData_OpenGL*)image->data)->handle );

        GLint textureWidth, textureHeight;

        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);
        
        GLenum texture_format = ((ImageData_OpenGL*)image->data)->format;

        // FIXME: Does not take into account GL_PACK_ALIGNMENT
        GLubyte *buffer = (GLubyte *)malloc(textureWidth*textureHeight*image->channels);

        glGetTexImage(GL_TEXTURE_2D, 0, texture_format, GL_UNSIGNED_BYTE, buffer);

        int x,y,i;
        for(y = 0; y < textureHeight; y++)
	{
		for(x = 0; x < textureWidth; x++)
		{
			i = ((y*textureWidth) + x)*image->channels;
			if(buffer[i] == from_r && buffer[i+1] == from_g && buffer[i+2] == from_b)
			{
				buffer[i] = to_r;
				buffer[i+1] = to_g;
				buffer[i+2] = to_b;
			}
		}
	}
	
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, texture_format, GL_UNSIGNED_BYTE, buffer);
 

	free(buffer);
}



static void MakeRGBTransparent(GPU_Renderer* renderer, GPU_Image* image, Uint8 r, Uint8 g, Uint8 b)
{
	if(image == NULL || image->channels < 4)
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
	if(image == NULL || image->channels < 3)
		return;
        if(renderer != image->renderer)
                return;
        
        glBindTexture( GL_TEXTURE_2D, ((ImageData_OpenGL*)image->data)->handle );

        GLint textureWidth, textureHeight;

        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);

        // FIXME: Does not take into account GL_PACK_ALIGNMENT
        GLubyte *buffer = (GLubyte *)malloc(textureWidth*textureHeight*image->channels);
        
        GLenum texture_format = ((ImageData_OpenGL*)image->data)->format;

        glGetTexImage(GL_TEXTURE_2D, 0, texture_format, GL_UNSIGNED_BYTE, buffer);

        int x,y,i;
        for(y = 0; y < textureHeight; y++)
	{
		for(x = 0; x < textureWidth; x++)
		{
			i = ((y*textureWidth) + x)*image->channels;
			
			if(image->channels == 4 && buffer[i+3] == 0)
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
	
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, texture_format, GL_UNSIGNED_BYTE, buffer);
 

	free(buffer);
}


static void ShiftHSVExcept(GPU_Renderer* renderer, GPU_Image* image, int hue, int saturation, int value, int notHue, int notSat, int notVal, int range)
{
	if(image == NULL || image->channels < 3)
		return;
        if(renderer != image->renderer)
                return;
        
        glBindTexture( GL_TEXTURE_2D, ((ImageData_OpenGL*)image->data)->handle );

        GLint textureWidth, textureHeight;

        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);

        // FIXME: Does not take into account GL_PACK_ALIGNMENT
        GLubyte *buffer = (GLubyte *)malloc(textureWidth*textureHeight*image->channels);
        
        GLenum texture_format = ((ImageData_OpenGL*)image->data)->format;

        glGetTexImage(GL_TEXTURE_2D, 0, texture_format, GL_UNSIGNED_BYTE, buffer);

        int x,y,i;
        for(y = 0; y < textureHeight; y++)
	{
		for(x = 0; x < textureWidth; x++)
		{
			i = ((y*textureWidth) + x)*image->channels;
			
			if(image->channels == 4 && buffer[i+3] == 0)
				continue;
			
			int r = buffer[i];
			int g = buffer[i+1];
			int b = buffer[i+2];
			
			int h, s, v;
			rgb_to_hsv(r, g, b, &h, &s, &v);
			
			if(notHue - range <= h && notHue + range >= h
				&& notSat - range <= s && notSat + range >= s
				&& notVal - range <= v && notVal + range >= v)
				continue;
			
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
	
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, texture_format, GL_UNSIGNED_BYTE, buffer);
 

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
        glBindFramebuffer(GL_FRAMEBUFFER, ((TargetData_OpenGL*)target->data)->handle);
        
        unsigned char pixels[4];
        glReadPixels(x, y, 1, 1, ((TargetData_OpenGL*)target->data)->format, GL_UNSIGNED_BYTE, pixels);
        
        result.r = pixels[0];
        result.g = pixels[1];
	result.b = pixels[2];
	result.unused = pixels[3];
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	return result;
}

static void SetImageFilter(GPU_Renderer* renderer, GPU_Image* image, GPU_FilterEnum filter)
{
	if(image == NULL)
		return;
        if(renderer != image->renderer)
                return;
        
        glBindTexture( GL_TEXTURE_2D, ((ImageData_OpenGL*)image->data)->handle );
        
        GLenum minFilter = GL_NEAREST;
        GLenum magFilter = GL_NEAREST;
        
        if(filter == GPU_LINEAR)
        {
                if(((ImageData_OpenGL*)image->data)->hasMipmaps)
                        minFilter = GL_NEAREST_MIPMAP_LINEAR;
                else
                        minFilter = GL_LINEAR;
        
		magFilter = GL_LINEAR;
        }
        else if(filter == GPU_LINEAR_MIPMAP)
        {
                if(((ImageData_OpenGL*)image->data)->hasMipmaps)
                        minFilter = GL_LINEAR_MIPMAP_LINEAR;
                else
                        minFilter = GL_LINEAR;
        
		magFilter = GL_LINEAR;
	}
	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}


static void SetBlendMode(GPU_Renderer* renderer, GPU_BlendEnum mode)
{
        if(mode == GPU_BLEND_NORMAL)
        {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glBlendEquation(GL_FUNC_ADD);
	}
	else if(mode == GPU_BLEND_MULTIPLY)
	{
		glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
	}
	else if(mode == GPU_BLEND_ADD)
	{
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_FUNC_ADD);
	}
	else if(mode == GPU_BLEND_SUBTRACT)
	{
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_FUNC_SUBTRACT);
	}
	else if(mode == GPU_BLEND_ADD_COLOR)
	{
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
	}
	else if(mode == GPU_BLEND_SUBTRACT_COLOR)
	{
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
		glBlendEquation(GL_FUNC_SUBTRACT);
	}
	else if(mode == GPU_BLEND_DARKEN)
	{
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_MIN);
	}
	else if(mode == GPU_BLEND_LIGHTEN)
	{
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_MAX);
	}
	else if(mode == GPU_BLEND_DIFFERENCE)
	{
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
		glBlendEquation(GL_FUNC_SUBTRACT);
	}
	else if(mode == GPU_BLEND_PUNCHOUT)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
	}
	else if(mode == GPU_BLEND_CUTOUT)
        {
                glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
                glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
        }
}







static void Clear(GPU_Renderer* renderer, GPU_Target* target)
{
	if(target == NULL)
		return;
        if(renderer != target->renderer)
                return;
        
        glBindFramebuffer(GL_FRAMEBUFFER, ((TargetData_OpenGL*)target->data)->handle);
        glPushAttrib(GL_VIEWPORT_BIT);
        glViewport(0,0,target->w, target->h);
        
        if(target->useClip)
	{
		glEnable(GL_SCISSOR_TEST);
		int y = (renderer->display == target? renderer->display->h - (target->clipRect.y + target->clipRect.h) : target->clipRect.y);
		float xFactor = ((float)renderer->window_w)/renderer->display->w;
		float yFactor = ((float)renderer->window_h)/renderer->display->h;
                glScissor(target->clipRect.x * xFactor, y * yFactor, target->clipRect.w * xFactor, target->clipRect.h * yFactor);
        }
        
        glPushAttrib(GL_COLOR_BUFFER_BIT);
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
        glPopAttrib();
        
        if(target->useClip)
        {
                glDisable(GL_SCISSOR_TEST);
        }
        
        glPopAttrib();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


static void ClearRGBA(GPU_Renderer* renderer, GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(target == NULL)
		return;
        if(renderer != target->renderer)
                return;
        
        glBindFramebuffer(GL_FRAMEBUFFER, ((TargetData_OpenGL*)target->data)->handle);
        glPushAttrib(GL_VIEWPORT_BIT);
        glViewport(0,0,target->w, target->h);
        
        if(target->useClip)
	{
		glEnable(GL_SCISSOR_TEST);
		int y = (renderer->display == target? renderer->display->h - (target->clipRect.y + target->clipRect.h) : target->clipRect.y);
		float xFactor = ((float)renderer->window_w)/renderer->display->w;
		float yFactor = ((float)renderer->window_h)/renderer->display->h;
		glScissor(target->clipRect.x * xFactor, y * yFactor, target->clipRect.w * xFactor, target->clipRect.h * yFactor);
	}
	
	glClearColor(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	if(target->useClip)
	{
                glDisable(GL_SCISSOR_TEST);
        }

        glPopAttrib();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void Flip(GPU_Renderer* renderer)
{
    #ifdef SDL_GPU_USE_SDL2
    SDL_GL_SwapWindow(((RendererData_OpenGL*)renderer->data)->window);
    #else
        SDL_GL_SwapBuffers();
        #endif
}





GPU_Renderer* GPU_CreateRenderer_OpenGL(void)
{
        GPU_Renderer* renderer = (GPU_Renderer*)malloc(sizeof(GPU_Renderer));
        if(renderer == NULL)
		return NULL;
        
        memset(renderer, 0, sizeof(GPU_Renderer));
        
        renderer->id = "OpenGL";
        renderer->display = NULL;
        renderer->camera = GPU_GetDefaultCamera();
        
        renderer->data = (RendererData_OpenGL*)malloc(sizeof(RendererData_OpenGL));
        memset(renderer->data, 0, sizeof(RendererData_OpenGL));
        
        renderer->Init = &Init;
        renderer->SetAsCurrent = &SetAsCurrent;
	renderer->SetDisplayResolution = &SetDisplayResolution;
	renderer->SetVirtualResolution = &SetVirtualResolution;
	renderer->Quit = &Quit;
	
	renderer->ToggleFullscreen = &ToggleFullscreen;
	renderer->SetCamera = &SetCamera;
	
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
	renderer->BlitTransformX = &BlitTransformX;
	renderer->BlitTransformMatrix = &BlitTransformMatrix;
	
	renderer->SetZ = &SetZ;
	renderer->GetZ = &GetZ;
	renderer->GenerateMipmaps = &GenerateMipmaps;
	
	renderer->SetBlending = &SetBlending;
	renderer->SetRGBA = &SetRGBA;
	
	renderer->ReplaceRGB = &ReplaceRGB;
	renderer->MakeRGBTransparent = &MakeRGBTransparent;
	renderer->ShiftHSV = &ShiftHSV;
	renderer->ShiftHSVExcept = &ShiftHSVExcept;
	renderer->GetPixel = &GetPixel;
	renderer->SetImageFilter = &SetImageFilter;
	renderer->SetBlendMode = &SetBlendMode;
	
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
