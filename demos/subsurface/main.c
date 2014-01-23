#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"


int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
    
    printCurrentRenderer();
	
	SDL_Surface* surf = SDL_LoadBMP("data/test.bmp");
	GPU_Image* image = GPU_CreateImage(100, 100, 4);
	GPU_Target* tgt = GPU_LoadTarget(image);
	GPU_Image* image2 = GPU_LoadImage("data/test.bmp");
	if(image == NULL || surf == NULL || tgt == NULL || image2 == NULL)
		return -1;
	
	GPU_Rect rect = {71, 64, 96, 52};
	GPU_SubSurfaceCopy(surf, &rect, tgt, 0, 0);
	GPU_FreeTarget(tgt);
	tgt = NULL;
	SDL_FreeSurface(surf);
	surf = NULL;
	
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
		
		GPU_Blit(image, NULL, screen, screen->w/2, screen->h/2);
        GPU_Blit(image2, &rect, screen, rect.w/2, rect.h/2);
		
		GPU_Flip(screen);
		
		frameCount++;
		if(frameCount%500 == 0)
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	GPU_FreeImage(image);
	GPU_Quit();
	
	return 0;
}


