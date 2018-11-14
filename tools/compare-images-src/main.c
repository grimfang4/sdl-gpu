#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "compat.h"
#include "common.h"
#include <stdlib.h>

#define PI 3.14159265359


void getScreenToWorld(float screenX, float screenY, float* worldX, float* worldY)
{
	GPU_Target* screen = GPU_GetContextTarget();
	GPU_Camera camera = GPU_GetCamera(screen);
	if(screen == NULL)
		return;

	if(worldX)
	{
		//if(camera.angle == 0.0f)
			*worldX = (screenX - screen->w/2) / camera.zoom_x + camera.x + screen->w/2;
		//else
			//*worldX = (screenX - screen->w/2) / camera.zoom_x * cos(-camera.angle*PI/180) - (screenY - screen->h/2) / camera.zoom * sin(-camera.angle*PI/180) + camera.x + screen->w/2;
	}
	if(worldY)
	{
		//if(camera.angle == 0.0f)
			*worldY = (screenY - screen->h/2) / camera.zoom_y + camera.y + screen->h/2;
		//else
			//*worldY = (screenX - screen->w/2) / camera.zoom_y * sin(-camera.angle*PI/180) + (screenY - screen->h/2) / camera.zoom * cos(-camera.angle*PI/180) + camera.y + screen->h/2;
	}
}

void getWorldToScreen(float worldX, float worldY, float* screenX, float* screenY)
{
	GPU_Target* screen = GPU_GetContextTarget();
	GPU_Camera camera = GPU_GetCamera(screen);
	if(screen == NULL)
		return;

	if(screenX)
	{
		//if(camera.angle == 0.0f)
			*screenX = (worldX - camera.x - screen->w/2)*camera.zoom_x + screen->w/2;
		//else
			//*screenX = (worldX - camera.x - screen->w/2)*camera.zoom_x * cos(-camera.angle*PI/180) + screen->w/2;
	}
	if(screenY)
	{
		//if(camera.angle == 0.0f)
			*screenY = (worldY - camera.y - screen->h/2)*camera.zoom_y + screen->h/2;
		//else
			//*screenY = (worldY - camera.y - screen->h/2)*camera.zoom_y * sin(-camera.angle*PI/180) + screen->h/2;
	}
}

void printScreenToWorld(float screenX, float screenY)
{
	float worldX, worldY;
	getScreenToWorld(screenX, screenY, &worldX, &worldY);

	printf("ScreenToWorld: (%.1f, %.1f) -> (%.1f, %.1f)\n", screenX, screenY, worldX, worldY);
}

void printWorldToScreen(float worldX, float worldY)
{
	float screenX, screenY;
	getWorldToScreen(worldX, worldY, &screenX, &screenY);

	printf("WorldToScreen: (%.1f, %.1f) -> (%.1f, %.1f)\n", worldX, worldY, screenX, screenY);
}


int main(int argc, char* argv[])
{
	GPU_Target* screen;

	printRenderers();

	screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;

	printCurrentRenderer();

	{
		int numImages;
		GPU_Image** images;
		int i;
		const Uint8* keystates;
		GPU_Camera camera;
		float dt;
		Uint8 done;
		SDL_Event event;
		float x, y;

		images = (GPU_Image**)malloc(sizeof(GPU_Image*)*(argc - 1));

		numImages = 0;

		for (i = 1; i < argc; i++)
		{
			images[numImages] = GPU_LoadImage(argv[i]);
			if (images[numImages] != NULL)
				numImages++;
		}

		keystates = SDL_GetKeyState(NULL);
		camera = GPU_GetDefaultCamera();

		dt = 0.010f;
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
					else if (event.key.keysym.sym == SDLK_r)
					{
					    camera = GPU_GetDefaultCamera();
					}
				}
				else if (event.type == SDL_MOUSEBUTTONDOWN)
				{
					float x, y;
					GPU_GetVirtualCoords(screen, &x, &y, event.button.x, event.button.y);

					printScreenToWorld(x, y);
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

			x = 0;
			y = 0;
			for (i = 0; i < numImages; i++)
			{
				x += images[i]->w / 2.0f;
				y += images[i]->h / 2.0f;
				GPU_Blit(images[i], NULL, screen, x, y);
				x += images[i]->w / 2.0f;
				y += images[i]->h / 2.0f;
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


