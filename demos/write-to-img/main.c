#include <SDL_gpu.h>
#include <stdlib.h>
#include <math.h>

static GPU_Target * screen;

int main(int argc, char * argv[])
{
    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);
    screen =  GPU_Init(1024, 800, GPU_DEFAULT_INIT_FLAGS);
    if(!screen)
    {
        printf("initialation failed\n");
        return -1;
    }
    
    char done = 0;
    SDL_Event event;
    //work begins here
    GPU_Image * src0, *src1, *dst0;
    
    src0 = GPU_LoadImage("data/test.bmp");
    src1 = GPU_LoadImage("data/test.png");
    dst0 = GPU_CreateImage(800,400,GPU_FORMAT_RGBA);
    if(!(src0 && src1 && dst0))
    {
        printf("an image initialization failed\n");
        GPU_FreeImage(src0);
        GPU_FreeImage(src1);
        GPU_FreeImage(dst0);
        GPU_Quit();
        return -1;
    }
    //copy them both side by side
    //uses sdl's coordinate system
    SDL_Rect d0,d1;
    d0.w = d1.w = d0.h = d1.h =400;
    d0.y = d1.y = 0;
    d0.x = 0;
    d1.x = 400;
    SDL_Surface * sdl_src0, *sdl_src1,* sdl_dst0;
    
    sdl_src0 = GPU_CopySurfaceFromImage(src0);
    sdl_src1 = GPU_CopySurfaceFromImage(src1);
    sdl_dst0 = GPU_CopySurfaceFromImage(dst0);
    //using sdl2 api
    SDL_BlitScaled(sdl_src0,0,sdl_dst0,&d0);
    SDL_BlitScaled(sdl_src1,0,sdl_dst0, &d1);
    
    GPU_FreeImage(dst0);
    dst0 = GPU_CopyImageFromSurface(sdl_dst0);
    //now delete all sdl surfaces
    SDL_FreeSurface(sdl_dst0);
    SDL_FreeSurface(sdl_src0);
    SDL_FreeSurface(sdl_src1);
    
    //get rid of source images
    GPU_FreeImage(src0);
    GPU_FreeImage(src1);
    
    //save it to file 
    GPU_SaveImage(dst0, "example.png", GPU_FILE_PNG);
    
    //just display it once done and wait for the app to quit
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
            else if(event.type == SDL_WINDOWEVENT)
            {
                switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                        GPU_SetWindowResolution(event.window.data1, event.window.data2);
                        break;
                    default:
                        break;
                }
            }
        }
        GPU_Clear(screen);
        GPU_Blit(dst0, 0, screen, screen->w/2, screen->h/2);
        GPU_Flip(screen);
        SDL_Delay(5);
        
    }
    GPU_FreeImage(dst0);
    GPU_Quit();
    
    return 0;
}