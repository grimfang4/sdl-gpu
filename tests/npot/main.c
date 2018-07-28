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
		Uint32 startTime;
		long frameCount;
		Uint8 done;
		SDL_Event event;
        
        GPU_Image* image = GPU_LoadImage("data/npot1.png");
        if(image == NULL)
            return -1;
        
        
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
            
            GPU_ClearRGBA(screen, 200, 200, 255, 255);
            
            GPU_Blit(image, NULL, screen, image->w/2, image->h/2);
            GPU_BlitTransform(image, NULL, screen, screen->w/2, screen->h/2, 360*sin(SDL_GetTicks()/2000.0f), 2.5*sin(SDL_GetTicks()/1000.0f), 2.5*sin(SDL_GetTicks()/1200.0f));
            
            GPU_Flip(screen);
            
            frameCount++;
            if(frameCount%500 == 0)
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        }
        
        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        
        GPU_FreeImage(image);
	}
	
	GPU_Quit();
	
	return 0;
}


