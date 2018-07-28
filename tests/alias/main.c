#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "common.h"


int main(int argc, char* argv[])
{
	GPU_Target* screen;

	printRenderers();
	
	screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();

	{
		GPU_Image* image;
		GPU_Target* alias_target;
		GPU_Image* alias_image;
		Uint32 startTime;
		long frameCount;

		Uint8 done;
		SDL_Event event;

		image = GPU_LoadImage("data/test.bmp");
		if (image == NULL)
			return -1;

		alias_target = GPU_CreateAliasTarget(screen);

		GPU_SetViewport(screen, GPU_MakeRect(50, 30, 400, 300));

		GPU_SetViewport(alias_target, GPU_MakeRect(400, 30, 400, 300));
		GPU_SetTargetRGBA(alias_target, 255, 100, 100, 200);


		alias_image = GPU_CreateAliasImage(image);

		GPU_SetImageFilter(alias_image, GPU_FILTER_NEAREST);
		GPU_SetRGBA(alias_image, 100, 255, 100, 200);


		startTime = SDL_GetTicks();
		frameCount = 0;

		done = 0;
		while (!done)
		{
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT)
					done = 1;
				else if (event.type == SDL_KEYDOWN)
				{
					if (event.key.keysym.sym == SDLK_ESCAPE)
						done = 1;
				}
			}

			GPU_Clear(screen);

			GPU_Blit(image, NULL, screen, image->w / 2, image->h / 2);
			GPU_Blit(alias_image, NULL, screen, image->w + alias_image->w / 2, alias_image->h / 2);

			GPU_Blit(image, NULL, alias_target, image->w / 2, image->h / 2);
			GPU_Blit(alias_image, NULL, alias_target, image->w + alias_image->w / 2, alias_image->h / 2);

			GPU_Flip(screen);

			frameCount++;
			if (frameCount % 500 == 0)
				printf("Average FPS: %.2f\n", 1000.0f*frameCount / (SDL_GetTicks() - startTime));
		}

		printf("Average FPS: %.2f\n", 1000.0f*frameCount / (SDL_GetTicks() - startTime));

		GPU_FreeImage(alias_image);
		GPU_FreeImage(image);
		GPU_FreeTarget(alias_target);
	}

	GPU_Quit();
	
	return 0;
}


