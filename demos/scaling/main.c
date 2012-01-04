#include "SDL.h"
#include "SDL_gpu.h"

int main(int argc, char* argv[])
{
	GPU_Target* screen = GPU_Init(800, 600, 0);
	if(screen == NULL)
		return -1;
	
	printf("Renderer: %s\n", GPU_GetRendererString());
	
	GPU_Image* image = GPU_LoadImage("scaling/test.bmp");
	if(image == NULL)
		return -1;
	
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
		
		SDL_Rect srcrect = {0, 0, 100, 100};
		SDL_Rect destrect = {50, 50, 150, 150};
		GPU_Blit(image, &srcrect, screen, &destrect);
		
		GPU_Flip();
		SDL_Delay(1);
	}
	
	GPU_FreeImage(image);
	GPU_Quit();
}


