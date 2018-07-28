#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"


int main(int argc, char* argv[])
{
	GPU_Target* screen;
	
	screen = initialize_demo(argc, argv, 800, 600);
	if(screen == NULL)
		return 1;
	
	{
		Uint32 startTime;
		long frameCount;
		Uint8 done;
		SDL_Event event;
        
        
        float dt = 0.010f;
        
        #define MAX_SPRITES 50
        int numSprites = 1;
        
        float x[MAX_SPRITES];
        float y[MAX_SPRITES];
        float velx[MAX_SPRITES];
        float vely[MAX_SPRITES];
        int i;
        
        GPU_Image* image = GPU_LoadImage("data/test.bmp");
        if(image == NULL)
            return 2;
        
        GPU_SetSnapMode(image, GPU_SNAP_NONE);
        
        for(i = 0; i < MAX_SPRITES; i++)
        {
            x[i] = rand()%screen->w;
            y[i] = rand()%screen->h;
            velx[i] = 10 + rand()%screen->w/10;
            vely[i] = 10 + rand()%screen->h/10;
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
                    else if(event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_PLUS)
                    {
                        if(numSprites < MAX_SPRITES)
                            numSprites++;
                    }
                    else if(event.key.keysym.sym == SDLK_MINUS)
                    {
                        if(numSprites > 0)
                            numSprites--;
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
            if(frameCount%500 == 0)
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        }
        
        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        
        GPU_FreeImage(image);
	}
	
	GPU_Quit();
	
	return 0;
}


