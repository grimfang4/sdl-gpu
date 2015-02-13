#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "compat.h"
#include "common.h"


int main(int argc, char* argv[])
{
	GPU_Target* screen;

	printRenderers();

	screen = GPU_Init(300, 200, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;

	printCurrentRenderer();

	{
		Uint32 startTime;
		long frameCount;
		Uint8 done;
		SDL_Event event;

		GPU_Image* image;
		GPU_Target* target;

        SDL_Color circleColor = {255, 0, 0, 128};
        SDL_Color circleColor2 = {0, 0, 255, 128};
        SDL_Color rect_color1 = {0, 255, 0, 128};

        const Uint8* keystates = SDL_GetKeyState(NULL);
        int x = 0;
        int y = 0;

        image = GPU_LoadImage("data/test.bmp");
        if(image == NULL)
            return -1;

        target = GPU_LoadTarget(image);
        if(target == NULL)
            return -1;

        GPU_CircleFilled(target, 70, 70, 20, circleColor);
        

        startTime = SDL_GetTicks();
        frameCount = 0;

        done = 0;
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
					else if (event.key.keysym.sym == SDLK_f)
						GPU_SetFullscreen(!GPU_GetFullscreen(), 0);
					else if (event.key.keysym.sym == SDLK_g)
						GPU_SetFullscreen(!GPU_GetFullscreen(), 1);
					else if (event.key.keysym.sym == SDLK_1)
						GPU_UnsetVirtualResolution(screen);
					else if (event.key.keysym.sym == SDLK_2)
						GPU_SetVirtualResolution(screen, 100, 100);
					else if (event.key.keysym.sym == SDLK_3)
						GPU_SetVirtualResolution(screen, 320, 240);
					else if (event.key.keysym.sym == SDLK_4)
						GPU_SetVirtualResolution(screen, 640, 480);
					else if (event.key.keysym.sym == SDLK_5)
						GPU_SetVirtualResolution(screen, 800, 600);
					else if (event.key.keysym.sym == SDLK_6)
						GPU_SetVirtualResolution(screen, 1024, 768);
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

            GPU_Blit(image, NULL, screen, image->w/2 + 50, image->h/2 + 50);

            GPU_CircleFilled(screen, 50 + 70, 50 + 70, 20, circleColor2);
            
            GPU_Rectangle(screen, 0, 0, screen->w, screen->h, rect_color1);

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
	}

	GPU_Quit();

	return 0;
}


