#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "compat.h"
#include "common.h"
#include "demo-font.h"


int main(int argc, char* argv[])
{
	GPU_Target* screen;

	printRenderers();

	screen = GPU_Init(1000, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;

	printCurrentRenderer();

	{
		SDL_Surface* font_surface;
		DemoFont* font;
		
		Uint32 startTime;
		long frameCount;
		const Uint8* keystates;
		int x;
		int y;
		Uint8 done;
		SDL_Event event;

		font_surface = GPU_LoadSurface("data/comic14.png");
		font = FONT_Alloc(font_surface);
		GPU_SetRGB(font->image, 255, 0, 0);
		SDL_FreeSurface(font_surface);

		startTime = SDL_GetTicks();
		frameCount = 0;

		keystates = SDL_GetKeyState(NULL);

		x = 0;
		y = 0;

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
					else if (event.key.keysym.sym == SDLK_f)
						GPU_SetFullscreen(!GPU_GetFullscreen(), 0);
					else if (event.key.keysym.sym == SDLK_g)
						GPU_SetFullscreen(!GPU_GetFullscreen(), 1);
				}
			}

			if (keystates[KEY_UP])
				y -= 1;
			else if (keystates[KEY_DOWN])
				y += 1;
			if (keystates[KEY_LEFT])
				x -= 1;
			else if (keystates[KEY_RIGHT])
				x += 1;

			GPU_Clear(screen);
            
            SDL_Color white = {255, 255, 255, 255};
            GPU_RectangleRound(screen, 20, 20, 50, 50, 5, white);
            
            GPU_SetLineThickness(4);
            GPU_RectangleRound(screen, 100, 20, 150, 50, 5, white);
            GPU_SetLineThickness(1);
            
            GPU_SetLineThickness(7);
            GPU_RectangleRound(screen, 200, 20, 250, 50, 5, white);
            GPU_SetLineThickness(1);
            
            GPU_RectangleRound(screen, 20 + x/7.0f, 100 + y/13.0f, 90 + x*1.101f, 300 + y*1.005f, 5, white);
            
			GPU_Flip(screen);

			frameCount++;
			if (frameCount % 500 == 0)
			{
				printf("Average FPS: %.2f\n", 1000.0f*frameCount / (SDL_GetTicks() - startTime));
				printf("x,y: (%d, %d)\n", x, y);
			}
		}

		printf("Average FPS: %.2f\n", 1000.0f*frameCount / (SDL_GetTicks() - startTime));

		FONT_Free(font);
	}

	GPU_Quit();

	return 0;
}


