#include "SDL_gpu.h"
#include "SDL_platform.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include "stb_image.h"

void GPU_InitRendererRegister(void);

static GPU_Renderer* current_renderer = NULL;

GPU_RendererID GPU_GetCurrentRendererID(void)
{
	if(current_renderer == NULL)
		return GPU_MakeRendererID(GPU_RENDERER_UNKNOWN, 0, 0, -1);
	return current_renderer->id;
}

void GPU_SetCurrentRenderer(GPU_RendererID id)
{
	current_renderer = GPU_GetRendererByID(id);
	
	if(current_renderer != NULL && current_renderer->SetAsCurrent != NULL)
		current_renderer->SetAsCurrent(current_renderer);
}

GPU_Renderer* GPU_GetCurrentRenderer(void)
{
	return current_renderer;
}



void GPU_LogInfo(const char* format, ...)
{
#ifdef SDL_GPU_ENABLE_LOG
	va_list args;
	va_start(args, format);
	#ifdef __ANDROID__
		__android_log_vprint(ANDROID_LOG_INFO, "APPLICATION", format, args);
	#else
		vprintf(format, args);
	#endif
	va_end(args);
#endif
}

void GPU_LogWarning(const char* format, ...)
{
#ifdef SDL_GPU_ENABLE_LOG
	va_list args;
	va_start(args, format);
	#ifdef __ANDROID__
		__android_log_vprint(ANDROID_LOG_WARN, "APPLICATION", format, args);
	#else
		vprintf(format, args);
	#endif
	va_end(args);
#endif
}

void GPU_LogError(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	#ifdef __ANDROID__
		__android_log_vprint(ANDROID_LOG_ERROR, "APPLICATION", format, args);
	#else
		vprintf(format, args);
	#endif
	va_end(args);
}


static Uint32 isolate_renderer_flags(Uint32 flags)
{
    // TODO: Make video/window/renderer flags combinable and make this work right.
    return 0;
}




GPU_Target* GPU_Init(Uint16 w, Uint16 h, Uint32 flags)
{
    return GPU_InitRenderer(GPU_MakeRendererIDRequest(GPU_RENDERER_DEFAULT, 0, 0, isolate_renderer_flags(flags)), w, h, flags);
}

GPU_Target* GPU_InitRenderer(GPU_RendererID renderer_request, Uint16 w, Uint16 h, Uint32 flags)
{
	GPU_InitRendererRegister();
	
	if(GPU_GetNumActiveRenderers() == 0)
	{
	    Uint32 subsystems = SDL_WasInit(SDL_INIT_EVERYTHING);
	    if(!subsystems)
        {
            // Nothing has been set up, so init SDL and the video subsystem.
            if(SDL_Init(SDL_INIT_VIDEO) < 0)
            {
                GPU_LogError("GPU_Init() failed to initialize SDL.\n");
                return NULL;
            }
        }
        else if(!(subsystems & SDL_INIT_VIDEO))
        {
            // Something already set up SDL, so just init video.
            if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
            {
                GPU_LogError("GPU_Init() failed to initialize SDL video subsystem.\n");
                return NULL;
            }
        }
	}
	
	GPU_Renderer* renderer = GPU_AddRenderer(renderer_request);
	if(renderer == NULL || renderer->Init == NULL)
		return NULL;
    
	GPU_SetCurrentRenderer(renderer->id);
	
	return renderer->Init(renderer, renderer_request, w, h, flags);
}

Uint8 GPU_IsFeatureEnabled(GPU_FeatureEnum feature)
{
	if(current_renderer == NULL || current_renderer->IsFeatureEnabled == NULL)
		return 0;
	
	return current_renderer->IsFeatureEnabled(current_renderer, feature);
}

GPU_Target* GPU_CreateTargetFromWindow(Uint32 windowID)
{
	if(current_renderer == NULL || current_renderer->CreateTargetFromWindow == NULL)
		return NULL;
	
	return current_renderer->CreateTargetFromWindow(current_renderer, windowID, NULL);
}

void GPU_MakeCurrent(GPU_Target* target, Uint32 windowID)
{
	if(current_renderer == NULL || current_renderer->MakeCurrent == NULL)
		return;
	
	current_renderer->MakeCurrent(current_renderer, target, windowID);
}

int GPU_ToggleFullscreen(void)
{
	if(current_renderer == NULL || current_renderer->ToggleFullscreen == NULL)
		return 0;
	
	return current_renderer->ToggleFullscreen(current_renderer);
}

int GPU_SetWindowResolution(Uint16 w, Uint16 h)
{
	if(current_renderer == NULL || current_renderer->SetWindowResolution == NULL || w == 0 || h == 0)
		return 0;
	
	return current_renderer->SetWindowResolution(current_renderer, w, h);
}


void GPU_SetVirtualResolution(Uint16 w, Uint16 h)
{
	if(current_renderer == NULL || current_renderer->SetVirtualResolution == NULL || w == 0 || h == 0)
		return;
	
	current_renderer->SetVirtualResolution(current_renderer, w, h);
}

void GPU_CloseCurrentRenderer(void)
{
	if(current_renderer == NULL)
		return;
	
	if(current_renderer->Quit != NULL)
		current_renderer->Quit(current_renderer);
	GPU_RemoveRenderer(current_renderer->id);
	current_renderer = NULL;
	
	if(GPU_GetNumActiveRenderers() == 0)
		SDL_Quit();
}

void GPU_Quit(void)
{
	// FIXME: Remove all renderers
	if(current_renderer == NULL)
		return;
	
	if(current_renderer->Quit != NULL)
		current_renderer->Quit(current_renderer);
	GPU_RemoveRenderer(current_renderer->id);
	
	if(GPU_GetNumActiveRenderers() == 0)
		SDL_Quit();
}

void GPU_SetError(const char* fmt, ...)
{
	// FIXME: Parse varargs here
	SDL_SetError("%s", fmt);
}

const char* GPU_GetErrorString(void)
{
	return SDL_GetError();
}


void GPU_GetVirtualCoords(float* x, float* y, float displayX, float displayY)
{
	if(current_renderer == NULL || current_renderer->current_context_target == NULL || current_renderer->current_context_target->windowID == 0)
		return;
	
	if(x != NULL)
		*x = (displayX*current_renderer->current_context_target->w)/current_renderer->current_context_target->window_w;
	if(y != NULL)
		*y = (displayY*current_renderer->current_context_target->h)/current_renderer->current_context_target->window_h;
}

GPU_Rect GPU_MakeRect(float x, float y, float w, float h)
{
    GPU_Rect r = {x, y, w, h};
    return r;
}

GPU_RendererID GPU_MakeRendererIDRequest(GPU_RendererEnum id, int major_version, int minor_version, Uint32 flags)
{
    GPU_RendererID r = {id, major_version, minor_version, flags, -1};
    return r;
}

GPU_RendererID GPU_MakeRendererID(GPU_RendererEnum id, int major_version, int minor_version, int index)
{
    GPU_RendererID r = {id, major_version, minor_version, 0, index};
    return r;
}

GPU_Camera GPU_GetDefaultCamera(void)
{
	GPU_Camera cam = {0.0f, 0.0f, -10.0f, 0.0f, 1.0f};
	return cam;
}

GPU_Camera GPU_GetCamera(void)
{
	if(current_renderer == NULL || current_renderer->current_context_target == NULL)
		return GPU_GetDefaultCamera();
	return current_renderer->current_context_target->camera;
}

GPU_Camera GPU_SetCamera(GPU_Target* target, GPU_Camera* cam)
{
	if(current_renderer == NULL || current_renderer->SetCamera == NULL)
		return GPU_GetDefaultCamera();
	
	return current_renderer->SetCamera(current_renderer, target, cam);
}

GPU_Image* GPU_CreateImage(Uint16 w, Uint16 h, Uint8 channels)
{
	if(current_renderer == NULL || current_renderer->CreateImage == NULL)
		return NULL;
	
	return current_renderer->CreateImage(current_renderer, w, h, channels);
}

GPU_Image* GPU_LoadImage(const char* filename)
{
	if(current_renderer == NULL || current_renderer->LoadImage == NULL)
		return NULL;
	
	return current_renderer->LoadImage(current_renderer, filename);
}

Uint8 GPU_SaveImage(GPU_Image* image, const char* filename)
{
	if(current_renderer == NULL || current_renderer->SaveImage == NULL)
		return 0;
	
	return current_renderer->SaveImage(current_renderer, image, filename);
}

GPU_Image* GPU_CopyImage(GPU_Image* image)
{
	if(current_renderer == NULL || current_renderer->CopyImage == NULL)
		return NULL;
	
	return current_renderer->CopyImage(current_renderer, image);
}

void GPU_UpdateImage(GPU_Image* image, const GPU_Rect* rect, SDL_Surface* surface)
{
	if(current_renderer == NULL || current_renderer->UpdateImage == NULL)
		return;
	
	current_renderer->UpdateImage(current_renderer, image, rect, surface);
}

SDL_Surface* GPU_LoadSurface(const char* filename)
{
	int width, height, channels;
	Uint32 Rmask, Gmask, Bmask, Amask = 0;
	
	if(filename == NULL)
        return NULL;
	
	#ifdef __ANDROID__
	unsigned char* data;
	if(strlen(filename) > 0 && filename[0] != '/')
	{
        // Must use SDL_RWops to access the assets directory automatically
        SDL_RWops* rwops = SDL_RWFromFile(filename, "r");
        if(rwops == NULL)
            return NULL;
        int data_bytes = SDL_RWseek(rwops, 0, SEEK_END);
        SDL_RWseek(rwops, 0, SEEK_SET);
        unsigned char* c_data = (unsigned char*)malloc(data_bytes);
        SDL_RWread(rwops, c_data, 1, data_bytes);
        data = stbi_load_from_memory(c_data, data_bytes, &width, &height, &channels, 0);
        free(c_data);
        SDL_FreeRW(rwops);
	}
	else
    {
        // Absolute filename
        data = stbi_load(filename, &width, &height, &channels, 0);
    }
	#else
	unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
	#endif
	
	if(data == NULL)
	{
		GPU_LogError("GPU_LoadSurface() failed: %s\n", stbi_failure_reason());
		return NULL;
	}
	if(channels < 3 || channels > 4)
	{
		GPU_LogError("GPU_LoadSurface() failed to load an unsupported pixel format.\n");
		stbi_image_free(data);
		return NULL;
	}
	
	if(channels == 3)
	{
	    // These are reversed from what SDL_image uses...  That is bad. :(  Needs testing.
	    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
		Rmask = 0xff0000;
		Gmask = 0x00ff00;
		Bmask = 0x0000ff;
		#else
		Rmask = 0x0000ff;
		Gmask = 0x00ff00;
		Bmask = 0xff0000;
		#endif
	}
	else
	{
		Rmask = 0x000000ff;
		Gmask = 0x0000ff00;
		Bmask = 0x00ff0000;
		Amask = 0xff000000;
	}
	
	SDL_Surface* result = SDL_CreateRGBSurfaceFrom(data, width, height, channels*8, width*channels, Rmask, Gmask, Bmask, Amask);
	
	return result;
}

#include "stb_image.h"
#include "stb_image_write.h"

// From http://stackoverflow.com/questions/5309471/getting-file-extension-in-c
static const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename)
        return "";
    return dot + 1;
}

Uint8 GPU_SaveSurface(SDL_Surface* surface, const char* filename)
{
    const char* extension;
    Uint8 result;
    unsigned char* data;

    if(surface == NULL || filename == NULL ||
            surface->w < 1 || surface->h < 1)
    {
        return 0;
    }

    extension = get_filename_ext(filename);

    data = surface->pixels;

    if(strcasecmp(extension, "png") == 0)
        result = stbi_write_png(filename, surface->w, surface->h, surface->format->BytesPerPixel, (const unsigned char *const)data, 0);
    else if(strcasecmp(extension, "bmp") == 0)
        result = stbi_write_bmp(filename, surface->w, surface->h, surface->format->BytesPerPixel, (void*)data);
    else if(strcasecmp(extension, "tga") == 0)
        result = stbi_write_tga(filename, surface->w, surface->h, surface->format->BytesPerPixel, (void*)data);
    //else if(strcasecmp(extension, "dds") == 0)
    //    result = stbi_write_dds(filename, surface->w, surface->h, surface->format->BytesPerPixel, (const unsigned char *const)data);
    else
    {
        GPU_LogError("GPU_SaveSurface() failed: Unsupported format (%s).\n", extension);
        result = 0;
    }

    return result;
}

GPU_Image* GPU_CopyImageFromSurface(SDL_Surface* surface)
{
	if(current_renderer == NULL || current_renderer->CopyImageFromSurface == NULL)
		return NULL;
	
	return current_renderer->CopyImageFromSurface(current_renderer, surface);
}

GPU_Image* GPU_CopyImageFromTarget(GPU_Target* target)
{
	if(current_renderer == NULL || current_renderer->CopyImageFromTarget == NULL)
		return NULL;
	
	return current_renderer->CopyImageFromTarget(current_renderer, target);
}

SDL_Surface* GPU_CopySurfaceFromTarget(GPU_Target* target)
{
	if(current_renderer == NULL || current_renderer->CopySurfaceFromTarget == NULL)
		return NULL;
	
	return current_renderer->CopySurfaceFromTarget(current_renderer, target);
}

SDL_Surface* GPU_CopySurfaceFromImage(GPU_Image* image)
{
	if(current_renderer == NULL || current_renderer->CopySurfaceFromImage == NULL)
		return NULL;
	
	return current_renderer->CopySurfaceFromImage(current_renderer, image);
}

void GPU_FreeImage(GPU_Image* image)
{
	if(current_renderer == NULL || current_renderer->FreeImage == NULL)
		return;
	
	current_renderer->FreeImage(current_renderer, image);
}


void GPU_SubSurfaceCopy(SDL_Surface* src, GPU_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y)
{
	if(current_renderer == NULL || current_renderer->SubSurfaceCopy == NULL)
        return;
	
	current_renderer->SubSurfaceCopy(current_renderer, src, srcrect, dest, x, y);
}

GPU_Target* GPU_GetCurrentTarget(void)
{
	if(current_renderer == NULL)
		return NULL;
	
	return current_renderer->current_context_target;
}


GPU_Target* GPU_LoadTarget(GPU_Image* image)
{
	if(current_renderer == NULL || current_renderer->LoadTarget == NULL)
		return NULL;
	
	return current_renderer->LoadTarget(current_renderer, image);
}



void GPU_FreeTarget(GPU_Target* target)
{
	if(current_renderer == NULL || current_renderer->FreeTarget == NULL)
		return;
	
	current_renderer->FreeTarget(current_renderer, target);
}



int GPU_Blit(GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y)
{
	if(current_renderer == NULL || current_renderer->Blit == NULL)
		return -2;
	
	return current_renderer->Blit(current_renderer, src, srcrect, dest, x, y);
}


int GPU_BlitRotate(GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float angle)
{
	if(current_renderer == NULL || current_renderer->BlitRotate == NULL)
		return -2;
	
	return current_renderer->BlitRotate(current_renderer, src, srcrect, dest, x, y, angle);
}

int GPU_BlitScale(GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float scaleX, float scaleY)
{
	if(current_renderer == NULL || current_renderer->BlitScale == NULL)
		return -2;
	
	return current_renderer->BlitScale(current_renderer, src, srcrect, dest, x, y, scaleX, scaleY);
}

int GPU_BlitTransform(GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float angle, float scaleX, float scaleY)
{
	if(current_renderer == NULL || current_renderer->BlitTransform == NULL)
		return -2;
	
	return current_renderer->BlitTransform(current_renderer, src, srcrect, dest, x, y, angle, scaleX, scaleY);
}

int GPU_BlitTransformX(GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float pivot_x, float pivot_y, float angle, float scaleX, float scaleY)
{
	if(current_renderer == NULL || current_renderer->BlitTransformX == NULL)
		return -2;
	
	return current_renderer->BlitTransformX(current_renderer, src, srcrect, dest, x, y, pivot_x, pivot_y, angle, scaleX, scaleY);
}

int GPU_BlitTransformMatrix(GPU_Image* src, GPU_Rect* srcrect, GPU_Target* dest, float x, float y, float* matrix3x3)
{
	if(current_renderer == NULL || current_renderer->BlitTransformMatrix == NULL || matrix3x3 == NULL)
		return -2;
	
	return current_renderer->BlitTransformMatrix(current_renderer, src, srcrect, dest, x, y, matrix3x3);
}


float GPU_SetZ(float z)
{
	if(current_renderer == NULL || current_renderer->SetZ == NULL)
		return 0.0f;
	
	return current_renderer->SetZ(current_renderer, z);
}

float GPU_GetZ(void)
{
	if(current_renderer == NULL || current_renderer->GetZ == NULL)
		return 0.0f;
	
	return current_renderer->GetZ(current_renderer);
}

void GPU_GenerateMipmaps(GPU_Image* image)
{
	if(current_renderer == NULL || current_renderer->GenerateMipmaps == NULL)
		return;
	
	current_renderer->GenerateMipmaps(current_renderer, image);
}




GPU_Rect GPU_SetClipRect(GPU_Target* target, GPU_Rect rect)
{
	if(target == NULL || current_renderer == NULL || current_renderer->SetClip == NULL)
	{
		GPU_Rect r = {0,0,0,0};
		return r;
	}
	
	return current_renderer->SetClip(current_renderer, target, rect.x, rect.y, rect.w, rect.h);
}

GPU_Rect GPU_SetClip(GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
	if(target == NULL || current_renderer == NULL || current_renderer->SetClip == NULL)
	{
		GPU_Rect r = {0,0,0,0};
		return r;
	}
	
	return current_renderer->SetClip(current_renderer, target, x, y, w, h);
}

void GPU_ClearClip(GPU_Target* target)
{
	if(target == NULL || current_renderer == NULL || current_renderer->ClearClip == NULL)
        return;
	
	current_renderer->ClearClip(current_renderer, target);
}


Uint8 GPU_GetBlending(void)
{
	if(current_renderer == NULL || current_renderer->GetBlending == NULL)
		return 0;
	
	return current_renderer->GetBlending(current_renderer);
}


void GPU_SetBlending(Uint8 enable)
{
	if(current_renderer == NULL || current_renderer->SetBlending == NULL)
		return;
	
	current_renderer->SetBlending(current_renderer, enable);
}


void GPU_SetColor(SDL_Color* color)
{
	if(current_renderer == NULL || current_renderer->SetRGBA == NULL)
		return;
	
	if(color == NULL)
		current_renderer->SetRGBA(current_renderer, 255, 255, 255, 255);
	else
    #ifdef SDL_GPU_USE_SDL2
		current_renderer->SetRGBA(current_renderer, color->r, color->g, color->b, color->a);
    #else
		current_renderer->SetRGBA(current_renderer, color->r, color->g, color->b, color->unused);
    #endif
}

void GPU_SetRGB(Uint8 r, Uint8 g, Uint8 b)
{
	if(current_renderer == NULL || current_renderer->SetRGBA == NULL)
		return;
	
	current_renderer->SetRGBA(current_renderer, r, g, b, 255);
}

void GPU_SetRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(current_renderer == NULL || current_renderer->SetRGBA == NULL)
		return;
	
	current_renderer->SetRGBA(current_renderer, r, g, b, a);
}



SDL_Color GPU_GetPixel(GPU_Target* target, Sint16 x, Sint16 y)
{
	if(current_renderer == NULL || current_renderer->GetPixel == NULL)
	{
		SDL_Color c = {0,0,0,0};
		return c;
	}
	
	return current_renderer->GetPixel(current_renderer, target, x, y);
}

void GPU_SetImageFilter(GPU_Image* image, GPU_FilterEnum filter)
{
	if(current_renderer == NULL || current_renderer->SetImageFilter == NULL)
		return;
	
	current_renderer->SetImageFilter(current_renderer, image, filter);
}

void GPU_SetBlendMode(GPU_BlendEnum mode)
{
	if(current_renderer == NULL || current_renderer->SetBlendMode == NULL)
		return;
	
	current_renderer->SetBlendMode(current_renderer, mode);
}







void GPU_Clear(GPU_Target* target)
{
	if(current_renderer == NULL || current_renderer->Clear == NULL)
		return;
	
	current_renderer->Clear(current_renderer, target);
}

void GPU_ClearRGBA(GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if(current_renderer == NULL || current_renderer->ClearRGBA == NULL)
		return;
	
	current_renderer->ClearRGBA(current_renderer, target, r, g, b, a);
}

void GPU_FlushBlitBuffer(void)
{
	if(current_renderer == NULL || current_renderer->FlushBlitBuffer == NULL)
		return;
	
	current_renderer->FlushBlitBuffer(current_renderer);
}

void GPU_Flip(GPU_Target* target)
{
	if(current_renderer == NULL || current_renderer->Flip == NULL)
		return;
	
	current_renderer->Flip(current_renderer, target);
}





// Shader API


Uint32 GPU_CompileShader_RW(int shader_type, SDL_RWops* shader_source)
{
	if(current_renderer == NULL || current_renderer->CompileShader_RW == NULL)
		return 0;
	
	return current_renderer->CompileShader_RW(current_renderer, shader_type, shader_source);
}

Uint32 GPU_LoadShader(int shader_type, const char* filename)
{
    SDL_RWops* rwops = SDL_RWFromFile(filename, "r");
    Uint32 result = GPU_CompileShader_RW(shader_type, rwops);
    SDL_RWclose(rwops);
    return result;
}

Uint32 GPU_CompileShader(int shader_type, const char* shader_source)
{
	if(current_renderer == NULL || current_renderer->CompileShader == NULL)
		return 0;
	
	return current_renderer->CompileShader(current_renderer, shader_type, shader_source);
}

Uint32 GPU_LinkShaderProgram(Uint32 program_object)
{
	if(current_renderer == NULL || current_renderer->LinkShaderProgram == NULL)
		return 0;
	
	return current_renderer->LinkShaderProgram(current_renderer, program_object);
}

Uint32 GPU_LinkShaders(Uint32 shader_object1, Uint32 shader_object2)
{
	if(current_renderer == NULL || current_renderer->LinkShaders == NULL)
		return 0;
	
	return current_renderer->LinkShaders(current_renderer, shader_object1, shader_object2);
}

void GPU_FreeShader(Uint32 shader_object)
{
	if(current_renderer == NULL || current_renderer->FreeShader == NULL)
		return;
	
	current_renderer->FreeShader(current_renderer, shader_object);
}

void GPU_FreeShaderProgram(Uint32 program_object)
{
	if(current_renderer == NULL || current_renderer->FreeShaderProgram == NULL)
		return;
	
	current_renderer->FreeShaderProgram(current_renderer, program_object);
}

void GPU_AttachShader(Uint32 program_object, Uint32 shader_object)
{
	if(current_renderer == NULL || current_renderer->AttachShader == NULL)
		return;
	
	current_renderer->AttachShader(current_renderer, program_object, shader_object);
}

void GPU_DetachShader(Uint32 program_object, Uint32 shader_object)
{
	if(current_renderer == NULL || current_renderer->DetachShader == NULL)
		return;
	
	current_renderer->DetachShader(current_renderer, program_object, shader_object);
}

void GPU_ActivateShaderProgram(Uint32 program_object)
{
	if(current_renderer == NULL || current_renderer->ActivateShaderProgram == NULL)
		return;
	
	current_renderer->ActivateShaderProgram(current_renderer, program_object);
}

void GPU_DeactivateShaderProgram(void)
{
	if(current_renderer == NULL || current_renderer->DeactivateShaderProgram == NULL)
		return;
	
	current_renderer->DeactivateShaderProgram(current_renderer);
}

const char* GPU_GetShaderMessage(void)
{
	if(current_renderer == NULL || current_renderer->GetShaderMessage == NULL)
		return NULL;
	
	return current_renderer->GetShaderMessage(current_renderer);
}

int GPU_GetAttribLocation(Uint32 program_object, const char* attrib_name)
{
	if(current_renderer == NULL || current_renderer->GetAttribLocation == NULL)
		return 0;
	
	return current_renderer->GetAttribLocation(current_renderer, program_object, attrib_name);
}

int GPU_GetUniformLocation(Uint32 program_object, const char* uniform_name)
{
	if(current_renderer == NULL || current_renderer->GetUniformLocation == NULL)
		return 0;
	
	return current_renderer->GetUniformLocation(current_renderer, program_object, uniform_name);
}

GPU_ShaderBlock GPU_LoadShaderBlock(Uint32 program_object, const char* position_name, const char* texcoord_name, const char* color_name, const char* modelViewMatrix_name)
{
	if(current_renderer == NULL || current_renderer->LoadShaderBlock == NULL)
    {
        GPU_ShaderBlock b;
        b.position_loc = -1;
        b.texcoord_loc = -1;
        b.color_loc = -1;
        b.modelViewProjection_loc = -1;
		return b;
    }
	
	return current_renderer->LoadShaderBlock(current_renderer, program_object, position_name, texcoord_name, color_name, modelViewMatrix_name);
}

void GPU_SetShaderBlock(GPU_ShaderBlock block)
{
	if(current_renderer == NULL || current_renderer->SetShaderBlock == NULL)
		return;
	
	current_renderer->SetShaderBlock(current_renderer, block);
}

void GPU_GetUniformiv(Uint32 program_object, int location, int* values)
{
	if(current_renderer == NULL || current_renderer->GetUniformiv == NULL)
		return;
	
	current_renderer->GetUniformiv(current_renderer, program_object, location, values);
}

void GPU_SetUniformi(int location, int value)
{
	if(current_renderer == NULL || current_renderer->SetUniformi == NULL)
		return;
	
	current_renderer->SetUniformi(current_renderer, location, value);
}

void GPU_SetUniformiv(int location, int num_elements_per_value, int num_values, int* values)
{
	if(current_renderer == NULL || current_renderer->SetUniformiv == NULL)
		return;
	
	current_renderer->SetUniformiv(current_renderer, location, num_elements_per_value, num_values, values);
}


void GPU_GetUniformuiv(Uint32 program_object, int location, unsigned int* values)
{
	if(current_renderer == NULL || current_renderer->GetUniformuiv == NULL)
		return;
	
	current_renderer->GetUniformuiv(current_renderer, program_object, location, values);
}

void GPU_SetUniformui(int location, unsigned int value)
{
	if(current_renderer == NULL || current_renderer->SetUniformui == NULL)
		return;
	
	current_renderer->SetUniformui(current_renderer, location, value);
}

void GPU_SetUniformuiv(int location, int num_elements_per_value, int num_values, unsigned int* values)
{
	if(current_renderer == NULL || current_renderer->SetUniformuiv == NULL)
		return;
	
	current_renderer->SetUniformuiv(current_renderer, location, num_elements_per_value, num_values, values);
}


void GPU_GetUniformfv(Uint32 program_object, int location, float* values)
{
	if(current_renderer == NULL || current_renderer->GetUniformfv == NULL)
		return;
	
	current_renderer->GetUniformfv(current_renderer, program_object, location, values);
}

void GPU_SetUniformf(int location, float value)
{
	if(current_renderer == NULL || current_renderer->SetUniformf == NULL)
		return;
	
	current_renderer->SetUniformf(current_renderer, location, value);
}

void GPU_SetUniformfv(int location, int num_elements_per_value, int num_values, float* values)
{
	if(current_renderer == NULL || current_renderer->SetUniformfv == NULL)
		return;
	
	current_renderer->SetUniformfv(current_renderer, location, num_elements_per_value, num_values, values);
}

// Same as GPU_GetUniformfv()
void GPU_GetUniformMatrixfv(Uint32 program_object, int location, float* values)
{
	if(current_renderer == NULL || current_renderer->GetUniformfv == NULL)
		return;
	
	current_renderer->GetUniformfv(current_renderer, program_object, location, values);
}

void GPU_SetUniformMatrixfv(int location, int num_matrices, int num_rows, int num_columns, Uint8 transpose, float* values)
{
	if(current_renderer == NULL || current_renderer->SetUniformMatrixfv == NULL)
		return;
	
	current_renderer->SetUniformMatrixfv(current_renderer, location, num_matrices, num_rows, num_columns, transpose, values);
}


