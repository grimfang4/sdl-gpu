#include "SDL.h"
#include "SDL_gpu.h"

int main(int argc, char* argv[])
{
    GPU_Target* screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
    
    SDL_Event event;
    Uint8 done = 0;
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
        
        GPU_Image* image = GPU_LoadImage("data/test.bmp");
        GPU_Image* image2 = GPU_CopyImage(image);
        GPU_Target* target = GPU_LoadTarget(image2);
        
        GPU_Blit(image, NULL, screen, screen->w/4, screen->h/2);
        
        GPU_Blit(image2, NULL, screen, 3*screen->w/4, screen->h/2);
        
        GPU_FreeTarget(target);
        GPU_FreeImage(image2);
        GPU_FreeImage(image);
        
        GPU_Flip(screen);
        SDL_Delay(100);
    }
    
    GPU_Quit();
    
    return 0;
}


