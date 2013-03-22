// Hacks to fix compile errors due to polluted namespace
#ifdef _WIN32
#define _WINUSER_H
#define _WINGDI_H
#endif

#include "SDL_gpu_OpenGL_internal.h"
#include "SOIL.h"
#include <math.h>
#include <strings.h>


static Uint8 checkExtension(const char* str)
{
    if(!glewIsExtensionSupported(str))
    {
        GPU_LogError("OpenGL error: %s is not supported.\n", str);
        return 0;
    }
    return 1;
}

static Uint8 NPOT_enabled = 0;

static void initNPOT(void)
{
    NPOT_enabled = glewIsExtensionSupported("GL_ARB_texture_non_power_of_two");
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
    SDL_Window* window = ((RendererData_OpenGL*)renderer->data)->window;
    if(window == NULL)
    {
        window = SDL_CreateWindow("",
                                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                              w, h,
                                              SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

        ((RendererData_OpenGL*)renderer->data)->window = window;
        if(window == NULL)
        {
            GPU_LogError("Window creation failed.\n");
            return NULL;
        }
        
        SDL_GLContext context = SDL_GL_CreateContext(window);
        ((RendererData_OpenGL*)renderer->data)->context = context;
    }

    SDL_GetWindowSize(window, &renderer->window_w, &renderer->window_h);
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
        GPU_LogError("Failed to initialize: %s\n", glewGetErrorString(err));
    }

    checkExtension("GL_EXT_framebuffer_object");
    checkExtension("GL_ARB_framebuffer_object"); // glGenerateMipmap
    checkExtension("GL_EXT_framebuffer_blit");
    
    initNPOT();

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

    renderer->SetBlending(renderer, 1);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(renderer->display == NULL)
        renderer->display = (GPU_Target*)malloc(sizeof(GPU_Target));

    renderer->display->image = NULL;
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
    {
        GPU_LogError("GPU_CreateImage() could not create an image with %d color channels.  Try 3 or 4 instead.\n", channels);
        return NULL;
    }

    SOIL_Texture texture;

    unsigned char* pixels = (unsigned char*)malloc(w*h*channels);
    memset(pixels, 0, w*h*channels);

    texture = SOIL_create_OGL_texture(pixels, w, h, channels, 0, NPOT_enabled? 0 : SOIL_FLAG_POWER_OF_TWO);
    if(texture.texture == 0)
    {
        free(pixels);
        return NULL;
    }

    // Set the texture's stretching properties
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

    GPU_Image* result = (GPU_Image*)malloc(sizeof(GPU_Image));
    ImageData_OpenGL* data = (ImageData_OpenGL*)malloc(sizeof(ImageData_OpenGL));
    result->target = NULL;
    result->data = data;
    result->renderer = renderer;
    result->channels = channels;
    data->handle = texture.texture;
    data->format = texture.format;
    data->hasMipmaps = 0;

    result->w = w;//texture.data_width;
    result->h = h;//texture.data_height;
    data->tex_w = texture.width;
    data->tex_h = texture.height;

    free(pixels);

    return result;
}

static GPU_Image* LoadImage(GPU_Renderer* renderer, const char* filename)
{
    SDL_Surface* surface = GPU_LoadSurface(filename);
    if(surface == NULL)
    {
        GPU_LogError("Failed to load image \"%s\"\n", filename);
        return NULL;
    }
    
    GPU_Image* result = renderer->CopyImageFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return result;
}

static unsigned char* getRawImageData(GPU_Image* image)
{
    unsigned char* data = (unsigned char*)malloc(image->w * image->h * image->channels);
    
    glBindTexture(GL_TEXTURE_2D, ((ImageData_OpenGL*)image->data)->handle);
    glGetTexImage(GL_TEXTURE_2D, 0, ((ImageData_OpenGL*)image->data)->format, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return data;
}

static Uint8 SaveImage(GPU_Renderer* renderer, GPU_Image* image, const char* filename)
{
    int image_type;
    const char* extension;
    Uint8 result;
    unsigned char* data;
    
    if(filename == NULL)
        return 0;
    
    if(strlen(filename) < 5)
    {
        GPU_LogError("GPU_SaveImage() failed: Unsupported format.\n");
        return 0;
    }
    
    extension = filename + strlen(filename)-1 - 3;
    
    /* Doesn't support length 4 extensions yet */
    if(extension[0] != '.')
    {
        GPU_LogError("GPU_SaveImage() failed: Unsupported format.\n");
        return 0;
    }
    
    extension++;
    if(strcasecmp(extension, "png") == 0)
        image_type = SOIL_SAVE_TYPE_PNG;
    else if(strcasecmp(extension, "bmp") == 0)
        image_type = SOIL_SAVE_TYPE_BMP;
    else if(strcasecmp(extension, "tga") == 0)
        image_type = SOIL_SAVE_TYPE_TGA;
    else if(strcasecmp(extension, "dds") == 0)
        image_type = SOIL_SAVE_TYPE_DDS;
    else
    {
        GPU_LogError("GPU_SaveImage() failed: Unsupported format (%s).\n", extension);
        return 0;
    }
    
    data = getRawImageData(image);
    
	result = SOIL_save_image(filename, image_type, image->w, image->h, image->channels, data);
	
	free(data);
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


// Returns 0 if a direct conversion is safe.  Returns 1 if a copy is needed.  Returns -1 on error.
static int compareFormats(GLenum glFormat, SDL_Surface* surface, GLenum* surfaceFormatResult)
{
    SDL_PixelFormat* format = surface->format;
    switch(glFormat)
    {
        case GL_RGB:
        case GL_BGR:
            //GPU_LogError("Wanted 3 channels, got %d\n", format->BytesPerPixel);
            if(format->BytesPerPixel != 3)
				return 1;
			
			if(format->Rmask == 0x0000FF && 
				format->Gmask == 0x00FF00 && 
				format->Bmask == 0xFF0000)
			{
				if(surfaceFormatResult != NULL)
					*surfaceFormatResult = GL_RGB;
				return 0;
			}
            if(format->Rmask == 0xFF0000 && 
				format->Gmask == 0x00FF00 && 
				format->Bmask == 0x0000FF)
			{
				if(surfaceFormatResult != NULL)
					*surfaceFormatResult = GL_BGR;
				return 0;
			}
            //GPU_LogError("Masks don't match\n");
			return 1;
			
        case GL_RGBA:
        case GL_BGRA:
            //GPU_LogError("Wanted 4 channels, got %d\n", format->BytesPerPixel);
            if(format->BytesPerPixel != 4)
				return 1;
			
			if(format->Rmask == 0x000000FF && 
				format->Gmask == 0x0000FF00 && 
				format->Bmask == 0x00FF0000)
			{
				if(surfaceFormatResult != NULL)
					*surfaceFormatResult = GL_RGBA;
				return 0;
			}
			if(format->Rmask == 0x0000FF && 
				format->Gmask == 0x00FF00 && 
				format->Bmask == 0xFF0000)
			{
				if(surfaceFormatResult != NULL)
					*surfaceFormatResult = GL_RGBA;
				return 0;
			}
			if(format->Rmask == 0xFF000000 && 
				format->Gmask == 0x00FF0000 && 
				format->Bmask == 0x0000FF00)
			{
				if(surfaceFormatResult != NULL)
					*surfaceFormatResult = GL_BGRA;
				return 0;
			}
			if(format->Rmask == 0xFF0000 && 
				format->Gmask == 0x00FF00 && 
				format->Bmask == 0x0000FF)
			{
				if(surfaceFormatResult != NULL)
					*surfaceFormatResult = GL_BGRA;
				return 0;
			}
            //GPU_LogError("Masks don't match: %X, %X, %X\n", format->Rmask, format->Gmask, format->Bmask);
			return 1;
        default:
            GPU_LogError("GPU_UpdateImage() was passed an image with an invalid format.\n");
            return -1;
    }
}

// From SDL_AllocFormat()
static SDL_PixelFormat* AllocFormat(GLenum glFormat)
{
    // Yes, I need to do the whole thing myself... :(
    int channels;
    Uint32 Rmask, Gmask, Bmask, Amask, mask;
    
    switch(glFormat)
    {
        case GL_RGB:
            channels = 3;
            Rmask = 0x0000FF;
            Gmask = 0x00FF00;
            Bmask = 0xFF0000;
            break;
        case GL_BGR:
            channels = 3;
            Rmask = 0xFF0000;
            Gmask = 0x00FF00;
            Bmask = 0x0000FF;
            break;
        case GL_RGBA:
            channels = 4;
            Rmask = 0x000000FF;
            Gmask = 0x0000FF00;
            Bmask = 0x00FF0000;
            Amask = 0xFF000000;
            break;
        case GL_BGRA:
            channels = 4;
            Rmask = 0xFF000000;
            Gmask = 0x00FF0000;
            Bmask = 0x0000FF00;
            Amask = 0x000000FF;
            break;
        default:
            return NULL;
    }
    
    SDL_PixelFormat* result = (SDL_PixelFormat*)malloc(sizeof(SDL_PixelFormat));
    memset(result, 0, sizeof(SDL_PixelFormat));
    
	result->BitsPerPixel = 8*channels;
	result->BytesPerPixel = channels;
	
    result->Rmask = Rmask;
    result->Rshift = 0;
    result->Rloss = 8;
    if (Rmask) {
        for (mask = Rmask; !(mask & 0x01); mask >>= 1)
            ++result->Rshift;
        for (; (mask & 0x01); mask >>= 1)
            --result->Rloss;
    }

    result->Gmask = Gmask;
    result->Gshift = 0;
    result->Gloss = 8;
    if (Gmask) {
        for (mask = Gmask; !(mask & 0x01); mask >>= 1)
            ++result->Gshift;
        for (; (mask & 0x01); mask >>= 1)
            --result->Gloss;
    }

    result->Bmask = Bmask;
    result->Bshift = 0;
    result->Bloss = 8;
    if (Bmask) {
        for (mask = Bmask; !(mask & 0x01); mask >>= 1)
            ++result->Bshift;
        for (; (mask & 0x01); mask >>= 1)
            --result->Bloss;
    }

    result->Amask = Amask;
    result->Ashift = 0;
    result->Aloss = 8;
    if (Amask) {
        for (mask = Amask; !(mask & 0x01); mask >>= 1)
            ++result->Ashift;
        for (; (mask & 0x01); mask >>= 1)
            --result->Aloss;
    }

    return result;
}

static Uint8 hasColorkey(SDL_Surface* surface)
{
    #ifdef SDL_GPU_USE_SDL2
    return (SDL_GetColorKey(surface, NULL) == 0);
    #else
    return (surface->flags & SDL_SRCCOLORKEY);
    #endif
}

static void FreeFormat(SDL_PixelFormat* format)
{
    free(format);
}

// Returns NULL on failure.  Returns the original surface if no copy is needed.  Returns a new surface converted to the right format otherwise.
static SDL_Surface* copySurfaceIfNeeded(GLenum glFormat, SDL_Surface* surface, GLenum* surfaceFormatResult)
{
	// If format doesn't match, we need to do a copy
    int format_compare = compareFormats(glFormat, surface, surfaceFormatResult);
	
	// There's a problem
    if(format_compare < 0)
		return NULL;
    
    // Copy it
    if(format_compare > 0)
    {
		// Convert to the right format
        SDL_PixelFormat* dst_fmt = AllocFormat(glFormat);
        surface = SDL_ConvertSurface(surface, dst_fmt, 0);
        FreeFormat(dst_fmt);
		if(surfaceFormatResult != NULL && surface != NULL)
			*surfaceFormatResult = glFormat;
    }
    
    // No copy needed
    return surface;
}

// From SDL_UpdateTexture()
static int UpdateImage(GPU_Renderer* renderer, GPU_Image* image,
                 const SDL_Rect* rect, SDL_Surface* surface)
{
    ImageData_OpenGL* data = (ImageData_OpenGL*)image->data;
    if(renderer == NULL || image == NULL || surface == NULL)
        return 0;
    
    SDL_Surface* newSurface = copySurfaceIfNeeded(data->format, surface, NULL);
    if(newSurface == NULL)
	{
        GPU_LogError("GPU_UpdateImage() failed to convert surface to proper pixel format.\n");
		return 0;
	}
		
	
    SDL_Rect updateRect;
    if(rect != NULL)
        updateRect = *rect;
    else
    {
        updateRect.x = 0;
        updateRect.y = 0;
        updateRect.w = newSurface->w;
        updateRect.h = newSurface->h;
    }
        
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, data->handle);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //glPixelStorei(GL_UNPACK_ROW_LENGTH,
    //                          (newSurface->pitch / newSurface->format->BytesPerPixel));
    glTexSubImage2D(GL_TEXTURE_2D, 0, updateRect.x, updateRect.y, updateRect.w,
                                updateRect.h, data->format, GL_UNSIGNED_BYTE,
                                newSurface->pixels);
    
	// Delete temporary surface
    if(surface != newSurface)
        SDL_FreeSurface(newSurface);
    return 1;
}

// From SDL_UpdateTexture()
static int InitImageWithSurface(GPU_Renderer* renderer, GPU_Image* image, SDL_Surface* surface)
{
    ImageData_OpenGL* data = (ImageData_OpenGL*)image->data;
    if(renderer == NULL || image == NULL || surface == NULL)
        return 0;
    
    GLenum internal_format = data->format;
    GLenum original_format = internal_format;
	
    SDL_Surface* newSurface = copySurfaceIfNeeded(internal_format, surface, &original_format);
    if(newSurface == NULL)
	{
        GPU_LogError("GPU_InitImageWithSurface() failed to convert surface to proper pixel format.\n");
		return 0;
	}
    
    //GPU_LogError("Original Pitch: %d, Bpp: %d\n", surface->pitch, surface->format->BytesPerPixel);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, data->handle);
    int alignment = 1;
    if(newSurface->format->BytesPerPixel == 4)
        alignment = 4;
    
    // FIXME: Support POT textures here
    //GPU_LogError("New Pitch: %d, Bpp: %d, Alignment: %d\n", newSurface->pitch, newSurface->format->BytesPerPixel, alignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    //glPixelStorei(GL_UNPACK_ROW_LENGTH,
    //                          (newSurface->pitch / newSurface->format->BytesPerPixel));
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, newSurface->w,
                                newSurface->h, 0, original_format, GL_UNSIGNED_BYTE,
                                newSurface->pixels);
    
	// Delete temporary surface
    if(surface != newSurface)
        SDL_FreeSurface(newSurface);
    return 1;
}


static GPU_Image* CreateUninitializedImage(GPU_Renderer* renderer, Uint16 w, Uint16 h, Uint8 channels)
{
    if(channels < 3 || channels > 4)
    {
        GPU_LogError("GPU_CreateUninitializedImage() could not create an image with %d color channels.  Try 3 or 4 instead.\n", channels);
        return NULL;
    }
	
	GLuint handle;
	GLenum format;
	if(channels == 3)
		format = GL_RGB;
	else
		format = GL_RGBA;
	
	glGenTextures( 1, &handle );
    if(handle == 0)
    {
        GPU_LogError("GPU_CreateUninitializedImage() failed to generate a texture handle.\n");
        return NULL;
    }
    
    glBindTexture( GL_TEXTURE_2D, handle );

    // Set the texture's stretching properties
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

    GPU_Image* result = (GPU_Image*)malloc(sizeof(GPU_Image));
    ImageData_OpenGL* data = (ImageData_OpenGL*)malloc(sizeof(ImageData_OpenGL));
    result->target = NULL;
    result->data = data;
    result->renderer = renderer;
    result->channels = channels;
    data->handle = handle;
    data->format = format;
    data->hasMipmaps = 0;

    result->w = w;
    result->h = h;
    // POT textures will change this later
    data->tex_w = w;
    data->tex_h = h;
	
    glBindTexture( GL_TEXTURE_2D, 0 );

    return result;
}

// From SDL_CreateTextureFromSurface
static GPU_Image* CopyImageFromSurface(GPU_Renderer* renderer, SDL_Surface* surface)
{
    const SDL_PixelFormat *fmt;
    Uint8 needAlpha;
    GPU_Image* image;
    int channels;

    if(renderer == NULL)
        return NULL;

    if (!surface) {
        GPU_LogError("GPU_CopyImageFromSurface() passed NULL surface.\n");
        return NULL;
    }

    /* See what the best texture format is */
    fmt = surface->format;
    if (fmt->Amask || hasColorkey(surface)) {
        needAlpha = 1;
    } else {
        needAlpha = 0;
    }
    
    // Get appropriate storage format
    // TODO: More options would be nice...
    if(needAlpha)
    {
        channels = 4;
    }
    else
    {
        channels = 3;
    }

    image = CreateUninitializedImage(renderer, surface->w, surface->h, channels);
    if (!image) {
        return NULL;
    }

    if (SDL_MUSTLOCK(surface)) {
        SDL_LockSurface(surface);
        InitImageWithSurface(renderer, image, surface);
        SDL_UnlockSurface(surface);
    } else {
        InitImageWithSurface(renderer, image, surface);
    }

    return image;
}




static void FreeImage(GPU_Renderer* renderer, GPU_Image* image)
{
    if(image == NULL)
        return;
    
    // Delete the attached target first
    if(image->target != NULL)
        renderer->FreeTarget(renderer, image->target);
    
    glDeleteTextures( 1, &((ImageData_OpenGL*)image->data)->handle);
    free(image->data);
    free(image);
}


static void SubSurfaceCopy(GPU_Renderer* renderer, SDL_Surface* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y)
{
    if(renderer == NULL || src == NULL || dest == NULL || dest->image == NULL)
        return;

    if(renderer != dest->renderer)
        return;

    SDL_Rect r;
    if(srcrect != NULL)
        r = *srcrect;
    else
    {
        r.x = 0;
        r.y = 0;
        r.w = src->w;
        r.h = src->h;
    }

    glBindTexture( GL_TEXTURE_2D, ((ImageData_OpenGL*)dest->image->data)->handle );

    //GLenum texture_format = GL_RGBA;//((ImageData_OpenGL*)image->data)->format;

    SDL_Surface* temp = SDL_CreateRGBSurface(SDL_SWSURFACE, r.w, r.h, src->format->BitsPerPixel, src->format->Rmask, src->format->Gmask, src->format->Bmask, src->format->Amask);

    if(temp == NULL)
    {
        GPU_LogError("GPU_SubSurfaceCopy(): Failed to create new %dx%d RGB surface.\n", r.w, r.h);
        return;
    }

    // Copy data to new surface
    #ifdef SDL_GPU_USE_SDL2
    SDL_BlendMode blendmode;
    SDL_GetSurfaceBlendMode(src, &blendmode);
    SDL_SetSurfaceBlendMode(src, SDL_BLENDMODE_NONE);
    #else
    Uint32 srcAlpha = src->flags & SDL_SRCALPHA;
    SDL_SetAlpha(src, 0, src->format->alpha);
    #endif

    SDL_BlitSurface(src, &r, temp, NULL);

    #ifdef SDL_GPU_USE_SDL2
    SDL_SetSurfaceBlendMode(src, blendmode);
    #else
    SDL_SetAlpha(src, srcAlpha, src->format->alpha);
    #endif

    // Make surface into an image
    GPU_Image* image = GPU_CopyImageFromSurface(temp);
    if(image == NULL)
    {
        GPU_LogError("GPU_SubSurfaceCopy(): Failed to create new image texture.\n");
        return;
    }

    // Copy image to dest
    Uint8 blending = GPU_GetBlending();
    GPU_SetBlending(0);
    GPU_Blit(image, NULL, dest, x + r.w/2, y + r.h/2);
    GPU_SetBlending(blending);

    // Using glTextSubImage might be more efficient
    //glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, r.w, r.h, texture_format, GL_UNSIGNED_BYTE, buffer);

    GPU_FreeImage(image);

    SDL_FreeSurface(temp);
}

static GPU_Target* GetDisplayTarget(GPU_Renderer* renderer)
{
    return renderer->display;
}


static GPU_Target* LoadTarget(GPU_Renderer* renderer, GPU_Image* image)
{
    if(renderer == NULL || image == NULL)
        return NULL;
    
    if(image->target != NULL)
        return image->target;
    
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
    result->image = image;
    result->w = image->w;
    result->h = image->h;

    result->useClip = 0;
    result->clipRect.x = 0;
    result->clipRect.y = 0;
    result->clipRect.w = image->w;
    result->clipRect.h = image->h;
    
    image->target = result;
    return result;
}



static void FreeTarget(GPU_Renderer* renderer, GPU_Target* target)
{
    if(target == NULL || target == renderer->display)
        return;

    glDeleteFramebuffers(1, &((TargetData_OpenGL*)target->data)->handle);
    
    if(target->image != NULL)
        target->image->target = NULL;  // Remove reference to this object
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

    // Modify the viewport and projection matrix if rendering to a texture
    GLint vp[4];
    if(renderer->display != dest)
    {
        glGetIntegerv(GL_VIEWPORT, vp);
        
        unsigned int w = dest->w;
        unsigned int h = dest->h;
        
        glViewport( 0, 0, w, h);
        
        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
        glLoadIdentity();
        
        glOrtho(0.0f, w, 0.0f, h, -1.0f, 1.0f); // Special inverted orthographic projection because tex coords are inverted already.
        
        glMatrixMode( GL_MODELVIEW );
    }


    if(dest->useClip)
    {
        glEnable(GL_SCISSOR_TEST);
        int y = (renderer->display == dest? renderer->display->h - (dest->clipRect.y + dest->clipRect.h) : dest->clipRect.y);
        float xFactor = ((float)renderer->window_w)/renderer->display->w;
        float yFactor = ((float)renderer->window_h)/renderer->display->h;
        glScissor(dest->clipRect.x * xFactor, y * yFactor, dest->clipRect.w * xFactor, dest->clipRect.h * yFactor);
    }

    Uint16 tex_w = ((ImageData_OpenGL*)src->data)->tex_w;
    Uint16 tex_h = ((ImageData_OpenGL*)src->data)->tex_h;

    float x1, y1, x2, y2;
    float dx1, dy1, dx2, dy2;
    if(srcrect == NULL)
    {
        // Scale tex coords according to actual texture dims
        x1 = 0;
        y1 = 0;
        x2 = ((float)src->w)/tex_w;
        y2 = ((float)src->h)/tex_h;
        // Center the image on the given coords
        dx1 = x - src->w/2;
        dy1 = y - src->h/2;
        dx2 = x + src->w/2;
        dy2 = y + src->h/2;
    }
    else
    {
        // Scale srcrect tex coords according to actual texture dims
        x1 = srcrect->x/(float)tex_w;
        y1 = srcrect->y/(float)tex_h;
        x2 = (srcrect->x + srcrect->w)/(float)tex_w;
        y2 = (srcrect->y + srcrect->h)/(float)tex_h;
        // Center the image on the given coords
        dx1 = x - srcrect->w/2;
        dy1 = y - srcrect->h/2;
        dx2 = x + srcrect->w/2;
        dy2 = y + srcrect->h/2;
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

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // restore viewport and projection
    if(renderer->display != dest)
    {
        glViewport(vp[0], vp[1], vp[2], vp[3]);
        
        glMatrixMode( GL_PROJECTION );
        glPopMatrix();
        glMatrixMode( GL_MODELVIEW );
    }
    return 0;
}


static int BlitRotate(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float angle)
{
    if(src == NULL || dest == NULL)
        return -1;
    if(renderer != src->renderer || renderer != dest->renderer)
        return -2;

    glPushMatrix();

    glTranslatef(x, y, 0);
    glRotatef(angle, 0, 0, 1);

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

    glTranslatef(x, y, 0);
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

    glTranslatef(x, y, 0);
    glRotatef(angle, 0, 0, 1);
    glScalef(scaleX, scaleY, 1.0f);

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

    // Shift away from the center (these are relative to the image corner)
    pivot_x -= src->w/2;
    pivot_y -= src->h/2;

    // Scale the pivot point so it moves the src image the right amount according to the viewport scale
    //pivot_x *= ?;
    //pivot_y *= ?;

    glTranslatef(x, y, 0);
    glRotatef(angle, 0, 0, 1);
    glScalef(scaleX, scaleY, 1.0f);
    glTranslatef(-pivot_x, -pivot_y, 0);

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

    // column-major 3x3 to column-major 4x4 (and scooting the translations to the homogeneous column)
    float matrix[16] = {matrix3x3[0], matrix3x3[1], matrix3x3[2], 0,
                        matrix3x3[3], matrix3x3[4], matrix3x3[5], 0,
                        0,            0,            matrix3x3[8], 0,
                        matrix3x3[6], matrix3x3[7], 0,            1};
    glTranslatef(x, y, 0);
    glMultMatrixf(matrix);

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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
}

static Uint8 GetBlending(GPU_Renderer* renderer)
{
    return ((RendererData_OpenGL*)renderer->data)->blending;
}


static void SetBlending(GPU_Renderer* renderer, Uint8 enable)
{
    if(enable)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);

    ((RendererData_OpenGL*)renderer->data)->blending = enable;
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

    textureWidth = ((ImageData_OpenGL*)image->data)->tex_w;
    textureHeight = ((ImageData_OpenGL*)image->data)->tex_h;

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

    textureWidth = ((ImageData_OpenGL*)image->data)->tex_w;
    textureHeight = ((ImageData_OpenGL*)image->data)->tex_h;

    GLenum texture_format = ((ImageData_OpenGL*)image->data)->format;

    // FIXME: Does not take into account GL_PACK_ALIGNMENT
    GLubyte *buffer = (GLubyte *)malloc(textureWidth*textureHeight*4);

    glGetTexImage(GL_TEXTURE_2D, 0, texture_format, GL_UNSIGNED_BYTE, buffer);

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

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, texture_format, GL_UNSIGNED_BYTE, buffer);

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

    textureWidth = ((ImageData_OpenGL*)image->data)->tex_w;
    textureHeight = ((ImageData_OpenGL*)image->data)->tex_h;

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

    textureWidth = ((ImageData_OpenGL*)image->data)->tex_w;
    textureHeight = ((ImageData_OpenGL*)image->data)->tex_h;

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
            minFilter = GL_LINEAR_MIPMAP_NEAREST;
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
    renderer->SaveImage = &SaveImage;
    renderer->CopyImage = &CopyImage;
    renderer->CopyImageFromSurface = &CopyImageFromSurface;
    renderer->SubSurfaceCopy = &SubSurfaceCopy;
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

    renderer->GetBlending = &GetBlending;
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
