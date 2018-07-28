#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "common.h"

#define IMAGE_FILE "data/happy_52x63.bmp"
#define SAVE_FILE "save.bmp"
#define SAVE_FILE2 "save2.png"


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
		
		GPU_Image* image;
		GPU_Image* image1;
		GPU_Image* image2;
        
        GPU_LogError("Loading image\n");
        image = GPU_LoadImage(IMAGE_FILE);
        if(image == NULL)
        {
            GPU_LogError("Failed to load image.\n");
            return -1;
        }
        
        GPU_LogError("Saving image\n");
        GPU_SaveImage(image, SAVE_FILE, GPU_FILE_AUTO);
        
        GPU_LogError("Reloading image\n");
        image1 = GPU_LoadImage(SAVE_FILE);
        if(image1 == NULL)
        {
            GPU_LogError("Failed to reload image.\n");
            return -1;
        }
        
        GPU_LogError("Saving image2\n");
        SDL_RWops* rwops = SDL_RWFromFile(SAVE_FILE2, "wb");
        GPU_SaveImage_RW(image, rwops, 1, GPU_FILE_PNG);
        
        GPU_LogError("Reloading image2\n");
        rwops = SDL_RWFromFile(SAVE_FILE2, "rb");
        image2 = GPU_LoadImage_RW(rwops, 1);
        if(image2 == NULL)
        {
            GPU_LogError("Failed to reload image2.\n");
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
            
            GPU_Blit(image, NULL, screen, screen->w/4, screen->h/2);
            GPU_Blit(image1, NULL, screen, 3*screen->w/4, screen->h/2);
            GPU_Blit(image2, NULL, screen, screen->w/4, 3*screen->h/4);
            
            GPU_Flip(screen);
            
            frameCount++;
            if(frameCount%500 == 0)
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        }
        
        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        
        GPU_FreeImage(image);
        GPU_FreeImage(image1);
	}
	
	GPU_Quit();
	
	return 0;
}


