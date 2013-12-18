#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "common.h"

int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(800, 600, 0);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	GPU_Image* image = GPU_LoadImage("data/test3.png");
	if(image == NULL)
		return -1;
	
	GPU_Image* image1 = GPU_CopyImage(image);
	
	SDL_Color yellow = {246, 255, 0};
	GPU_MakeColorTransparent(image1, yellow);
	
	GPU_Image* image1a = GPU_CopyImage(image);
	
	SDL_Color red = {200, 0, 0};
	GPU_ReplaceColor(image1a, yellow, red);
	
	
	
	
	GPU_Image* image2 = GPU_CopyImage(image);
	
	GPU_ShiftHSV(image2, 100, 0, 0);
	
	GPU_Image* image3 = GPU_CopyImage(image);
	
	GPU_ShiftHSV(image3, 0, -100, 0);
	
	GPU_Image* image4 = GPU_CopyImage(image);
	
	GPU_ShiftHSV(image4, 0, 0, 100);
	
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
		
		GPU_Blit(image, NULL, screen, 150, 150);
		GPU_Blit(image1, NULL, screen, 300, 150);
		GPU_Blit(image1a, NULL, screen, 450, 150);
		GPU_Blit(image2, NULL, screen, 150, 300);
		GPU_Blit(image3, NULL, screen, 300, 300);
		GPU_Blit(image4, NULL, screen, 450, 300);
		
		GPU_Flip(screen);
		
		frameCount++;
		if(frameCount%500 == 0)
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	GPU_FreeImage(image);
	GPU_FreeImage(image1);
	GPU_FreeImage(image1a);
	GPU_FreeImage(image2);
	GPU_FreeImage(image3);
	GPU_FreeImage(image4);
	GPU_Quit();
	
	return 0;
}


