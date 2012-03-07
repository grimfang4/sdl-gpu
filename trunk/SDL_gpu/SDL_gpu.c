#include "SDL_gpu.h"
#include "SDL_gpu_Renderer.h"

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
}

GPU_Renderer* GPU_GetCurrentRenderer(void)
{
	return current_renderer;
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

GPU_Image* GPU_CreateImage(Uint16 w, Uint16 h, Uint8 bits_per_pixel)
{
	if(current_renderer == NULL || current_renderer->CreateImage == NULL)
		return NULL;
	
	return current_renderer->CreateImage(current_renderer, w, h, bits_per_pixel);
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



int GPU_Blit(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y)
{
	if(current_renderer == NULL || current_renderer->Blit == NULL)
		return -2;
	
	return current_renderer->Blit(current_renderer, src, srcrect, dest, x, y);
}


int GPU_BlitRotate(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle)
{
	if(current_renderer == NULL || current_renderer->BlitRotate == NULL)
		return -2;
	
	return current_renderer->BlitRotate(current_renderer, src, srcrect, dest, x, y, angle);
}

int GPU_BlitScale(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float scaleX, float scaleY)
{
	if(current_renderer == NULL || current_renderer->BlitScale == NULL)
		return -2;
	
	return current_renderer->BlitScale(current_renderer, src, srcrect, dest, x, y, scaleX, scaleY);
}

int GPU_BlitTransform(GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, Sint16 x, Sint16 y, float angle, float scaleX, float scaleY)
{
	if(current_renderer == NULL || current_renderer->BlitTransform == NULL)
		return -2;
	
	return current_renderer->BlitTransform(current_renderer, src, srcrect, dest, x, y, angle, scaleX, scaleY);
}



void GPU_SetClip(GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
	if(target == NULL)
		return;
	
	target->clip_rect.x = x;
	target->clip_rect.y = y;
	target->clip_rect.w = w;
	target->clip_rect.h = h;
}

void GPU_ResetClip(GPU_Target* target)
{
	if(target == NULL)
		return;
	
	target->clip_rect.x = 0;
	target->clip_rect.y = 0;
	target->clip_rect.w = target->w;
	target->clip_rect.h = target->h;
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

