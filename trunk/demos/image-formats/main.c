#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "common.h"


void log_surface_details(SDL_Surface* surface)
{
    if(surface == NULL)
    {
        GPU_Log("Surface: NULL\n");
        return;
    }
    GPU_Log("Surface: %dx%d, %d bpp, %x Rmask, %x Gmask, %x Bmask, %x Amask\n", surface->w, surface->h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
}

void log_image_details(GPU_Image* image)
{
    if(image == NULL)
    {
        GPU_Log("Image: NULL\n");
        return;
    }
    GPU_Log("Image: %dx%d, %d channels, 0x%x format\n", image->w, image->h, image->channels, image->format);
}

GPU_Image* copy_and_log(SDL_Surface* surface)
{
    log_surface_details(surface);
    GPU_Image* image = GPU_CopyImageFromSurface(surface);
    log_image_details(image);
    return image;
}

int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	SDL_Surface* surface;
	
	surface = SDL_LoadBMP("data/test_8bit.bmp");
	GPU_Image* image8 = copy_and_log(surface);
	SDL_FreeSurface(surface);
	if(image8 == NULL)
		return -1;
    
	surface = SDL_LoadBMP("data/test_24bit.bmp");
	GPU_Image* image24 = copy_and_log(surface);
	SDL_FreeSurface(surface);
	if(image24 == NULL)
		return -2;
    
	surface = SDL_LoadBMP("data/test_32bit.bmp");
	GPU_Image* image32 = copy_and_log(surface);
	SDL_FreeSurface(surface);
	if(image32 == NULL)
		return -3;
    
	surface = GPU_LoadSurface("data/test3.png");
	GPU_Image* image_png = copy_and_log(surface);
	SDL_FreeSurface(surface);
	if(image_png == NULL)
		return -4;
    
	
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	Uint8 done = 0;
	SDL_Event event;
	while(!done)
	{
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT)
				done = 1;
			else if(event.type == SDL_KEYDOWN)
			{
				if(event.key.keysym.sym == SDLK_ESCAPE)
					done = 1;
			}
		}
		
		GPU_Clear(screen);
		
		GPU_Blit(image8, NULL, screen, image8->w/2, image8->h/2);
		GPU_Blit(image24, NULL, screen, image8->w + image24->w/2, image24->h/2);
		GPU_Blit(image32, NULL, screen, image32->w/2, image8->h + image32->h/2);
		GPU_Blit(image_png, NULL, screen, image8->w + image_png->w/2, image24->h + image_png->h/2);
		
		GPU_Flip(screen);
		
		frameCount++;
		if(frameCount%500 == 0)
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	GPU_FreeImage(image_png);
	GPU_FreeImage(image32);
	GPU_FreeImage(image24);
	GPU_FreeImage(image8);
	GPU_Quit();
	
	return 0;
}


