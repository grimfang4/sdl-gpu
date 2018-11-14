#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "compat.h"
#include "common.h"
#include <stdlib.h>


int main(int argc, char* argv[])
{
	GPU_Target* screen;

	printRenderers();

	screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;

	printCurrentRenderer();

	{
		float x, y;

		int numImages = 0;

		const Uint8* keystates = SDL_GetKeyState(NULL);
		GPU_Camera camera = GPU_GetDefaultCamera();

		float dt = 0.010f;
		Uint8 done = 0;
		SDL_Event event;

		GPU_Image** images = (GPU_Image**)malloc(sizeof(GPU_Image*)*(argc - 1));
		int i;
		for (i = 1; i < argc; i++)
		{
			images[numImages] = GPU_LoadImage(argv[i]);
			if (images[numImages] != NULL)
				numImages++;
		}

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

			if (keystates[KEY_UP])
			{
				camera.y -= 200 * dt;
			}
			else if (keystates[KEY_DOWN])
			{
				camera.y += 200 * dt;
			}
			if (keystates[KEY_LEFT])
			{
				camera.x -= 200 * dt;
			}
			else if (keystates[KEY_RIGHT])
			{
				camera.x += 200 * dt;
			}
            if(keystates[KEY_MINUS])
            {
                camera.zoom_x -= 1.0f*dt;
                camera.zoom_y -= 1.0f*dt;
            }
            else if(keystates[KEY_EQUALS])
            {
                camera.zoom_x += 1.0f*dt;
                camera.zoom_y += 1.0f*dt;
            }

			GPU_ClearRGBA(screen, 255, 255, 255, 255);

			GPU_SetCamera(screen, &camera);

			x = 100;
			y = 100;
			for (i = 0; i < numImages; i++)
			{
				float x_scale = 150.0f / images[i]->w;
				float y_scale = 150.0f / images[i]->h;
				GPU_BlitScale(images[i], NULL, screen, x, y, x_scale, y_scale);

				x += 200;

				if ((i + 1) % 4 == 0)
				{
					x = 100;
					y += 200;
				}
			}


			GPU_Flip(screen);
			SDL_Delay(10);
		}

		for (i = 0; i < numImages; i++)
		{
			GPU_FreeImage(images[i]);
		}
		free(images);
	}

	GPU_Quit();

	return 0;
}


