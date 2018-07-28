#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"
#include <math.h>


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
		float x, y;
		float velx, vely;
		float dt;
		Uint32 startTime;
		long frameCount;
		Uint8 done;
		SDL_Event event;

		image = GPU_LoadImage("data/test3.png");
		if (image == NULL)
			return -1;

		x = 0;
		y = 0;
		velx = 50.0f;
		vely = 70.0f;

		dt = 0.010f;

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

			x += velx*dt;
			y += vely*dt;
			if (x < 0)
			{
				x = 0;
				velx = -velx;
			}
			else if (x + image->w > screen->w)
			{
				x = screen->w - image->w;
				velx = -velx;
			}

			if (y < 0)
			{
				y = 0;
				vely = -vely;
			}
			else if (y + image->h > screen->h)
			{
				y = screen->h - image->h;
				vely = -vely;
			}

			GPU_Clear(screen);

			GPU_Blit(image, NULL, screen, 50, 50);
			GPU_Blit(image, NULL, screen, 150, 50);
			GPU_Blit(image, NULL, screen, 350, 250);

			GPU_SetBlending(image, 1);
			GPU_SetRGBA(image, 255, 100, 255, 127.5f + 127.5f*sin(SDL_GetTicks() / 1000.0f));

			GPU_Blit(image, NULL, screen, x, y);

			GPU_SetRGBA(image, 255, 255, 255, 255);
			GPU_SetBlending(image, 0);

			GPU_Flip(screen);

			frameCount++;
			if (frameCount % 500 == 0)
				printf("Average FPS: %.2f\n", 1000.0f*frameCount / (SDL_GetTicks() - startTime));
		}

		printf("Average FPS: %.2f\n", 1000.0f*frameCount / (SDL_GetTicks() - startTime));

		GPU_FreeImage(image);
	}

	GPU_Quit();
	
	return 0;
}


