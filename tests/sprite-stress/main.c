#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"
#include <stdlib.h>


int main(int argc, char* argv[])
{
	GPU_Target* screen;

	printRenderers();
	GPU_SetPreInitFlags(GPU_INIT_DISABLE_VSYNC);
	screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	{
		Uint32 startTime;
		long frameCount;
		Uint8 done;
		SDL_Event event;
		
        float dt = 0.010f;
        
        int maxSprites = 50000;
        int numSprites = 101;
        
        float* x = (float*)malloc(sizeof(float)*maxSprites);
        float* y = (float*)malloc(sizeof(float)*maxSprites);
        float* velx = (float*)malloc(sizeof(float)*maxSprites);
        float* vely = (float*)malloc(sizeof(float)*maxSprites);
        int i;
        
        GPU_Image* image = GPU_LoadImage("data/small_test.png");
        if(image == NULL)
            return -1;
        
        GPU_SetSnapMode(image, GPU_SNAP_NONE);
        
        startTime = SDL_GetTicks();
        frameCount = 0;
        for(i = 0; i < maxSprites; i++)
        {
            x[i] = rand()%screen->w;
            y[i] = rand()%screen->h;
            velx[i] = 10 + rand()%screen->w/10;
            vely[i] = 10 + rand()%screen->h/10;
            if(rand()%2)
                velx[i] = -velx[i];
            if(rand()%2)
                vely[i] = -vely[i];
        }
        
        
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
                    else if(event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_PLUS)
                    {
                        if(numSprites < maxSprites)
                            numSprites += 100;
                        GPU_LogError("Sprites: %d\n", numSprites);
                        frameCount = 0;
                        startTime = SDL_GetTicks();
                    }
                    else if(event.key.keysym.sym == SDLK_MINUS)
                    {
                        if(numSprites > 1)
                            numSprites -= 100;
                        if(numSprites < 1)
                            numSprites = 1;
                        GPU_LogError("Sprites: %d\n", numSprites);
                        frameCount = 0;
                        startTime = SDL_GetTicks();
                    }
                }
            }
            
            for(i = 0; i < numSprites; i++)
            {
                x[i] += velx[i]*dt;
                y[i] += vely[i]*dt;
                if(x[i] < 0)
                {
                    x[i] = 0;
                    velx[i] = -velx[i];
                }
                else if(x[i]> screen->w)
                {
                    x[i] = screen->w;
                    velx[i] = -velx[i];
                }
                
                if(y[i] < 0)
                {
                    y[i] = 0;
                    vely[i] = -vely[i];
                }
                else if(y[i]> screen->h)
                {
                    y[i] = screen->h;
                    vely[i] = -vely[i];
                }
            }
            
            GPU_Clear(screen);
            
            for(i = 0; i < numSprites; i++)
            {
                GPU_Blit(image, NULL, screen, x[i], y[i]);
            }
            
            GPU_Flip(screen);
            
            frameCount++;
            if(SDL_GetTicks() - startTime > 5000)
            {
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
                frameCount = 0;
                startTime = SDL_GetTicks();
            }
        }
        
        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        
        free(x);
        free(y);
        free(velx);
        free(vely);
        
        GPU_FreeImage(image);
	}
	
	GPU_Quit();
	
	return 0;
}


