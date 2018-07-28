#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "compat.h"
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
		int mx, my;
		GPU_Image* image;
		GPU_Target* target;
		SDL_Color c;
		
        image = GPU_LoadImage("data/test.bmp");
        if(image == NULL)
            return -1;
        
        target = GPU_LoadTarget(image);
        
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
            
            SDL_GetMouseState(&mx, &my);
            c = GPU_GetPixel(target, mx - 50, my - 50);
            
            GPU_ClearRGBA(screen, c.r, c.g, c.b, GET_ALPHA(c));
            
            GPU_Blit(image, NULL, screen, image->w/2 + 50, image->h/2 + 50);
            
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


