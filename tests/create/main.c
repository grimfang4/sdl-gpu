#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "common.h"

#define IMAGE_FILE "data/test.bmp"

int main(int argc, char* argv[])
{
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
		Uint32 startTime;
		long frameCount;
		Uint8 done;
		SDL_Event event;
		SDL_Surface* surface;
		GPU_Image* image;
		GPU_Image* image1;
		GPU_Image* image2;
		GPU_Image* image3;
		
        GPU_LogError("Loading image\n");
        image = GPU_LoadImage(IMAGE_FILE);
        if(image == NULL)
        {
            GPU_LogError("Failed to load image.\n");
            return -1;
        }

        GPU_LogError("Loading image1\n");
        image1 = GPU_CreateImage(200, 200, GPU_FORMAT_RGBA);
        if(image1 == NULL)
        {
            GPU_LogError("Failed to create image.\n");
            return -1;
        }
        GPU_LoadTarget(image1);
        GPU_ClearRGBA(image1->target, 0, 0, 255, 255);
        GPU_FreeTarget(image1->target);

        GPU_LogError("Loading image2\n");
        image2 = GPU_CopyImage(image);
        if(image2 == NULL)
        {
            GPU_LogError("Failed to copy image.\n");
            return -1;
        }

        surface = SDL_LoadBMP(IMAGE_FILE);
        GPU_LogError("Loading image3\n");
        image3 = GPU_CopyImageFromSurface(surface);
        SDL_FreeSurface(surface);
        if(image == NULL)
        {
            GPU_LogError("Failed to copy image from surface.\n");
            return -1;
        }

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
                }
            }
            
            GPU_Clear(screen);
            
            GPU_Blit(image, NULL, screen, 150, 150);
            GPU_Blit(image1, NULL, screen, 300, 300);
            GPU_Blit(image2, NULL, screen, 450, 450);
            GPU_Blit(image3, NULL, screen, 600, 150);
            
            GPU_Flip(screen);
            
            frameCount++;
            if(frameCount%500 == 0)
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        }

        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));

        GPU_FreeImage(image);
        GPU_FreeImage(image1);
        GPU_FreeImage(image2);
        GPU_FreeImage(image3);
	}
	
	GPU_Quit();
	
	return 0;
}


