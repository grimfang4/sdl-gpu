#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "common.h"

#define IMAGE_FILE "data/test.bmp"

int main_loop()
{
    int result = -1;
	GPU_Target* screen;

	printRenderers();
	
	screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
    {
        GPU_LogError("Failed to init SDL_gpu.\n");
		return -1;
    }
	
	printCurrentRenderer();
	
	{
		Uint8 done;
		SDL_Event event;
		GPU_Image* image;
		
        GPU_LogError("Loading image\n");
        image = GPU_LoadImage(IMAGE_FILE);
        if(image == NULL)
        {
            GPU_LogError("Failed to load image.\n");
            return -1;
        }

        done = 0;
        while(!done)
        {
            while(SDL_PollEvent(&event))
            {
                if(event.type == SDL_QUIT)
                {
                    done = 1;
                    result = -1;
                }
                else if(event.type == SDL_KEYDOWN)
                {
                    if(event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        done = 1;
                        result = -1;
                    }
                    else if(event.key.keysym.sym == SDLK_SPACE)
                    {
                        done = 1;
                        result = 1;
                    }
                }
            }
            
            GPU_Clear(screen);
            
            GPU_Blit(image, NULL, screen, 150, 150);
            
            GPU_Flip(screen);
        }

        GPU_FreeImage(image);
	}
	
	GPU_Quit();
	
	return result;
}


int main(int argc, char* argv[])
{
    while(main_loop() > 0)
    {
        GPU_Log("\nReinitializing.\n\n");
    }
    return 0;
}


