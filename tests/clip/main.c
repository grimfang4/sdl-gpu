#include "SDL.h"
#include "SDL_gpu.h"
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
		float dt;
		Uint32 startTime;
		long frameCount;
		int maxSprites = 50;
		int numSprites;
		float x[50];
		float y[50];
		float velx[50];
		float vely[50];
		int i;
		Uint8 done;
		SDL_Event event;
		SDL_Color lineColor = { 255, 0, 0, 255 };

		image = GPU_LoadImage("data/test.bmp");
		if (image == NULL)
			return -1;


		dt = 0.010f;

		startTime = SDL_GetTicks();
		frameCount = 0;

		numSprites = 3;

		for (i = 0; i < maxSprites; i++)
		{
			x[i] = rand() % screen->w;
			y[i] = rand() % screen->h;
			velx[i] = 10 + rand() % screen->w / 10;
			vely[i] = 10 + rand() % screen->h / 10;
		}


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
					else if (event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_PLUS)
					{
						if (numSprites < maxSprites)
							numSprites++;
					}
					else if (event.key.keysym.sym == SDLK_MINUS)
					{
						if (numSprites > 0)
							numSprites--;
					}
					else if (event.key.keysym.sym == SDLK_SPACE)
					{
					    if(screen->using_virtual_resolution)
                            GPU_UnsetVirtualResolution(screen);
                        else
                            GPU_SetVirtualResolution(screen, 400, 400);
					}
				}
			}

			for (i = 0; i < numSprites; i++)
			{
				x[i] += velx[i] * dt;
				y[i] += vely[i] * dt;
				if (x[i] < 0)
				{
					x[i] = 0;
					velx[i] = -velx[i];
				}
				else if (x[i] + image->w > 800)
				{
					x[i] = 800 - image->w;
					velx[i] = -velx[i];
				}

				if (y[i] < 0)
				{
					y[i] = 0;
					vely[i] = -vely[i];
				}
				else if (y[i] + image->h > 600)
				{
					y[i] = 600 - image->h;
					vely[i] = -vely[i];
				}
			}

			GPU_Clear(screen);

            GPU_SetClip(screen, 40, 40, 600, 300);
			for (i = 0; i < numSprites; i++)
			{
				GPU_Blit(image, NULL, screen, x[i], y[i]);
			}

			GPU_Line(screen, 0, 0, screen->w, screen->h, lineColor);
			GPU_Line(screen, 0, screen->h, screen->w, 0, lineColor);
			
            GPU_UnsetClip(screen);
            GPU_Rectangle(screen, 40, 40, 40 + 600, 40 + 300, lineColor);

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


