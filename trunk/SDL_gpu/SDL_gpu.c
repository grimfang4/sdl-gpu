#include "SDL_gpu.h"
#include "SDL_gpu_Renderer.h"

#ifdef ANDROID
#include <android/log.h>
#endif

static GPU_Renderer* current_renderer = NULL;

const char* GPU_GetCurrentRendererID(void)
{
	if(current_renderer == NULL)
		return NULL;
	return current_renderer->id;
}

void GPU_SetCurrentRenderer(const char* id)
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
	#ifdef ANDROID
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
	#ifdef ANDROID
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
	#ifdef ANDROID
		__android_log_vprint(ANDROID_LOG_ERROR, "APPLICATION", format, args);
	#else
		vprintf(format, args);
	#endif
	va_end(args);
}


GPU_Target* GPU_Init(const char* renderer_id, Uint16 w, Uint16 h, Uint32 flags)
{
	GPU_InitRendererRegister();
	
	if(renderer_id == NULL)
	{
		renderer_id = GPU_GetDefaultRendererID();
		if(renderer_id == NULL)
			return NULL;
	}
	
	
	if(GPU_GetNumActiveRenderers() == 0)
	{
		if(SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			return NULL;
		}
	}
	
	
	GPU_Renderer* renderer = GPU_GetRendererByID(renderer_id);
	if(renderer == NULL)
		renderer = GPU_AddRenderer(renderer_id);
	if(renderer == NULL || renderer->Init == NULL)
		return NULL;
	
	GPU_SetCurrentRenderer(renderer->id);
	
	return renderer->Init(renderer, w, h, flags);
}

int GPU_ToggleFullscreen(void)
{
	if(current_renderer == NULL || current_renderer->ToggleFullscreen == NULL)
		return 0;
	
	return current_renderer->ToggleFullscreen(current_renderer);
}

// TODO: Add error code return value
void GPU_GetDisplayResolution(int* w, int* h)
{
    if(current_renderer == NULL)
		return;
    
	if(w)
		*w = current_renderer->window_w;
	if(h)
		*h = current_renderer->window_h;
}

int GPU_SetDisplayResolution(Uint16 w, Uint16 h)
{
	if(current_renderer == NULL || current_renderer->SetDisplayResolution == NULL || w == 0 || h == 0)
		return 0;
	
	return current_renderer->SetDisplayResolution(current_renderer, w, h);
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
	
	if(current_renderer->Quit == NULL)
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
	
	if(current_renderer->Quit == NULL)
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
	if(current_renderer == NULL || current_renderer->display == NULL)
		return;
	
	if(x != NULL)
		*x = (displayX*current_renderer->display->w)/current_renderer->window_w;
	if(y != NULL)
		*y = (displayY*current_renderer->display->h)/current_renderer->window_h;
}

GPU_Camera GPU_GetDefaultCamera(void)
{
	GPU_Camera cam = {0.0f, 0.0f, -10.0f, 0.0f, 1.0f};
	return cam;
}

// One camera for the whole renderer, not per-target since the use-case would be rare.
GPU_Camera GPU_GetCamera(void)
{
	if(current_renderer == NULL)
		return GPU_GetDefaultCamera();
	return current_renderer->camera;
}

GPU_Camera GPU_SetCamera(GPU_Target* screen, GPU_Camera* cam)
{
	if(current_renderer == NULL || current_renderer->SetCamera == NULL)
		return GPU_GetDefaultCamera();
	
	return current_renderer->SetCamera(current_renderer, screen, cam);
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

GPU_Image* GPU_CopyImage(GPU_Image* image)
{
	if(current_renderer == NULL || current_renderer->CopyImage == NULL)
		return NULL;
	
	return current_renderer->CopyImage(current_renderer, image);
}

GPU_Image* GPU_CopyImageFromSurface(SDL_Surface* surface)
{
	if(current_renderer == NULL || current_renderer->CopyImageFromSurface == NULL)
		return NULL;
	
	return current_renderer->CopyImageFromSurface(current_renderer, surface);
}

void GPU_FreeImage(GPU_Image* image)
{
	if(current_renderer == NULL || current_renderer->FreeImage == NULL)
		return;
	
	current_renderer->FreeImage(current_renderer, image);
}

GPU_Target* GPU_GetDisplayTarget(void)
{
	if(current_renderer == NULL || current_renderer->GetDisplayTarget == NULL)
		return NULL;
	
	return current_renderer->GetDisplayTarget(current_renderer);
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



int GPU_Blit(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y)
{
	if(current_renderer == NULL || current_renderer->Blit == NULL)
		return -2;
	
	return current_renderer->Blit(current_renderer, src, srcrect, dest, x, y);
}


int GPU_BlitRotate(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float angle)
{
	if(current_renderer == NULL || current_renderer->BlitRotate == NULL)
		return -2;
	
	return current_renderer->BlitRotate(current_renderer, src, srcrect, dest, x, y, angle);
}

int GPU_BlitScale(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float scaleX, float scaleY)
{
	if(current_renderer == NULL || current_renderer->BlitScale == NULL)
		return -2;
	
	return current_renderer->BlitScale(current_renderer, src, srcrect, dest, x, y, scaleX, scaleY);
}

int GPU_BlitTransform(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float angle, float scaleX, float scaleY)
{
	if(current_renderer == NULL || current_renderer->BlitTransform == NULL)
		return -2;
	
	return current_renderer->BlitTransform(current_renderer, src, srcrect, dest, x, y, angle, scaleX, scaleY);
}

int GPU_BlitTransformX(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float pivot_x, float pivot_y, float angle, float scaleX, float scaleY)
{
	if(current_renderer == NULL || current_renderer->BlitTransformX == NULL)
		return -2;
	
	return current_renderer->BlitTransformX(current_renderer, src, srcrect, dest, x, y, pivot_x, pivot_y, angle, scaleX, scaleY);
}

int GPU_BlitTransformMatrix(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float* matrix3x3)
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




SDL_Rect GPU_SetClipRect(GPU_Target* target, SDL_Rect rect)
{
	if(target == NULL)
	{
		SDL_Rect r = {0,0,0,0};
		return r;
	}
	
	target->useClip = 1;
	
	SDL_Rect r = target->clipRect;
	
	target->clipRect = rect;
	
	return r;
}

SDL_Rect GPU_SetClip(GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
	if(target == NULL)
	{
		SDL_Rect r = {0,0,0,0};
		return r;
	}
	
	target->useClip = 1;
	
	SDL_Rect r = target->clipRect;
	
	target->clipRect.x = x;
	target->clipRect.y = y;
	target->clipRect.w = w;
	target->clipRect.h = h;
	
	return r;
}

void GPU_ClearClip(GPU_Target* target)
{
	if(target == NULL)
		return;
	
	target->useClip = 0;
	target->clipRect.x = 0;
	target->clipRect.y = 0;
	target->clipRect.w = target->w;
	target->clipRect.h = target->h;
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
		current_renderer->SetRGBA(current_renderer, 0, 0, 0, 255);
	else
		current_renderer->SetRGBA(current_renderer, color->r, color->g, color->b, color->unused);
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




void GPU_ReplaceColor(GPU_Image* image, SDL_Color from, SDL_Color to)
{
	if(current_renderer == NULL || current_renderer->ReplaceRGB == NULL)
		return;
	
	current_renderer->ReplaceRGB(current_renderer, image, from.r, from.g, from.b, to.r, to.g, to.b);
}


void GPU_MakeColorTransparent(GPU_Image* image, SDL_Color color)
{
	if(current_renderer == NULL || current_renderer->MakeRGBTransparent == NULL)
		return;
	
	current_renderer->MakeRGBTransparent(current_renderer, image, color.r, color.g, color.b);
}

void GPU_ShiftHSV(GPU_Image* image, int hue, int saturation, int value)
{
	if(current_renderer == NULL || current_renderer->ShiftHSV == NULL)
		return;
	
	current_renderer->ShiftHSV(current_renderer, image, hue, saturation, value);
}

void GPU_ShiftHSVExcept(GPU_Image* image, int hue, int saturation, int value, int notHue, int notSat, int notVal, int range)
{
	if(current_renderer == NULL || current_renderer->ShiftHSVExcept == NULL)
		return;
	
	current_renderer->ShiftHSVExcept(current_renderer, image, hue, saturation, value, notHue, notSat, notVal, range);
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

void GPU_Flip(void)
{
	if(current_renderer == NULL || current_renderer->Flip == NULL)
		return;
	
	current_renderer->Flip(current_renderer);
}

