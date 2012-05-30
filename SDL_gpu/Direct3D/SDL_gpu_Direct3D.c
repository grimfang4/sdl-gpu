#include "SDL_gpu_Direct3D_internal.h"

static GPU_Target* Init(GPU_Renderer* renderer, Uint16 w, Uint16 h, Uint32 flags)
{
	return NULL;
}

static void Quit(GPU_Renderer* renderer)
{
	
}

static GPU_Image* LoadImage(GPU_Renderer* renderer, const char* filename)
{
	return NULL;
}

static void FreeImage(GPU_Renderer* renderer, GPU_Image* image)
{
	
}

static GPU_Target* GetDisplayTarget(GPU_Renderer* renderer)
{
	return NULL;
}


static GPU_Target* LoadTarget(GPU_Renderer* renderer, GPU_Image* image)
{
	return NULL;
}



static void FreeTarget(GPU_Renderer* renderer, GPU_Target* target)
{
	
}



static int Blit(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y)
{
	return -2;
}


static int BlitRotate(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float angle)
{
	return -2;
}

static int BlitScale(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float scaleX, float scaleY)
{
	return -2;
}

static int BlitTransform(GPU_Renderer* renderer, GPU_Image* src, SDL_Rect* srcrect, GPU_Target* dest, float x, float y, float angle, float scaleX, float scaleY)
{
	return -2;
}



static void SetBlending(GPU_Renderer* renderer, Uint8 enable)
{
	
}


static void SetRGBA(GPU_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	
}



static void MakeRGBTransparent(GPU_Renderer* renderer, GPU_Image* image, Uint8 r, Uint8 g, Uint8 b)
{
	
}










static void Clear(GPU_Renderer* renderer, GPU_Target* target)
{
	
}

static void Flip(GPU_Renderer* renderer)
{
	
}





GPU_Renderer* GPU_CreateRenderer_Direct3D(void)
{
	GPU_Renderer* renderer = (GPU_Renderer*)malloc(sizeof(GPU_Renderer));
	if(renderer == NULL)
		return NULL;
	
	memset(renderer, 0, sizeof(GPU_Renderer));
	
	renderer->id = "Direct3D";
	renderer->display = NULL;
	
	renderer->data = (RendererData_Direct3D*)malloc(sizeof(RendererData_Direct3D));
	
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

void GPU_FreeRenderer_Direct3D(GPU_Renderer* renderer)
{
	if(renderer == NULL)
		return;
	
	free(renderer->data);
	free(renderer);
}
