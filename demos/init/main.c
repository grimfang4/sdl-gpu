#include "SDL.h"
#include "SDL_gpu.h"

void main_loop(GPU_Target* screen)
{
    Uint8 done;
    SDL_Event event;
    
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
        
        // Update logic here
        
        GPU_Clear(screen);
        
        // Draw stuff here
        
        GPU_Flip(screen);
    }
}

int main(int argc, char* argv[])
{
	GPU_Target* screen;
	
	screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	main_loop(screen);
	
	GPU_Quit();
	
	return 0;
}


