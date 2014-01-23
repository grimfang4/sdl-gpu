#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "compat.h"
#include "common.h"


int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	GPU_Image* image = GPU_LoadImage("data/test.bmp");
	if(image == NULL)
		return -1;
	
	GPU_Target* target = GPU_LoadTarget(image);
	if(target == NULL)
		return -1;
	
	
	SDL_Color circleColor = {255, 0, 0, 128};
	SDL_Color circleColor2 = {0, 0, 255, 128};
	
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	Uint8* keystates = SDL_GetKeyState(NULL);
	int x = 0;
	int y = 0;
	
	Uint8 switched = 1;
	GPU_SetVirtualResolution(screen, 640, 480);
	GPU_SetVirtualResolution(target, 640, 480);
	
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
				if(event.key.keysym.sym == SDLK_SPACE)
				{
					if(switched)
                    {
						GPU_SetVirtualResolution(screen, 800, 600);
						GPU_SetVirtualResolution(target, 800, 600);
                    }
					else
                    {
						GPU_SetVirtualResolution(screen, 640, 480);
						GPU_SetVirtualResolution(target, 640, 480);
                    }
					switched = !switched;
				}
			}
		}
		
		if(keystates[KEY_UP])
			y -= 1;
		else if(keystates[KEY_DOWN])
			y += 1;
		if(keystates[KEY_LEFT])
			x -= 1;
		else if(keystates[KEY_RIGHT])
			x += 1;
		
		GPU_Clear(screen);
		
		GPU_CircleFilled(target, 70, 70, 20, circleColor);
	
		GPU_Blit(image, NULL, screen, image->w/2 + 50, image->h/2 + 50);
		
		GPU_CircleFilled(screen, 50 + 70, 50 + 70, 20, circleColor2);
		
		GPU_Flip(screen);
		
		frameCount++;
		if(frameCount%500 == 0)
		{
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
			printf("x,y: (%d, %d)\n", x, y);
		}
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	GPU_FreeImage(image);
	GPU_Quit();
	
	return 0;
}


