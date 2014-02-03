#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "compat.h"
#include "common.h"


int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	GPU_Image* image = GPU_LoadImage("data/test.bmp");
	if(image == NULL)
		return -1;
	
	GPU_LoadTarget(image);
	
	
	SDL_Color circleColor = {255, 0, 0, 128};
	SDL_Color circleColor2 = {0, 0, 255, 128};
	
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	Uint8* keystates = SDL_GetKeyState(NULL);
	float x = 50;
	float y = 50;
	
	// Intermediate image buffers
	int max_images = 20;
	GPU_Image* images[max_images];
	
	int using_images = 1;
	GPU_FilterEnum filter_mode = GPU_NEAREST;
	
	Uint8 switched = 1;
	GPU_SetVirtualResolution(screen, 640, 480);
	GPU_SetVirtualResolution(image->target, 640, 480);
	GPU_SetImageFilter(image, filter_mode);
	int i;
	for(i = 0; i < max_images; i++)
    {
        images[i] = GPU_CreateImage(640, 480, 3);
        GPU_LoadTarget(images[i]);
        GPU_SetVirtualResolution(images[i]->target, 640, 480);
        GPU_SetImageFilter(images[i], filter_mode);
    }
	
	Uint8 done = 0;
	SDL_Event event;
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
				if(event.key.keysym.sym == SDLK_SPACE)
				{
					if(switched)
                    {
						GPU_SetVirtualResolution(screen, 800, 600);
                        GPU_SetVirtualResolution(image->target, 800, 600);
                        for(i = 0; i < max_images; i++)
                        {
                            GPU_SetVirtualResolution(images[i]->target, 800, 600);
                        }
                    }
					else
                    {
						GPU_SetVirtualResolution(screen, 640, 480);
                        GPU_SetVirtualResolution(image->target, 640, 480);
                        for(i = 0; i < max_images; i++)
                        {
                            GPU_SetVirtualResolution(images[i]->target, 640, 480);
                        }
                    }
					switched = !switched;
				}
				if(event.key.keysym.sym == SDLK_f)
                {
                    if(filter_mode == GPU_NEAREST)
                        filter_mode = GPU_LINEAR;
                    else
                        filter_mode = GPU_NEAREST;
                    
                    
                    GPU_SetImageFilter(image, filter_mode);
                    for(i = 0; i < max_images; i++)
                    {
                        GPU_SetImageFilter(images[i], filter_mode);
                    }
                }
				if(event.key.keysym.sym == SDLK_EQUALS)
                {
                    if(using_images < max_images)
                        using_images++;
                    GPU_LogError("using_images: %d\n", using_images);
                }
				if(event.key.keysym.sym == SDLK_MINUS)
                {
                    if(using_images > 0)
                        using_images--;
                    GPU_LogError("using_images: %d\n", using_images);
                }
			}
		}
		
		if(keystates[KEY_UP])
			y -= 0.1f;
		else if(keystates[KEY_DOWN])
			y += 0.1f;
		if(keystates[KEY_LEFT])
			x -= 0.1f;
		else if(keystates[KEY_RIGHT])
			x += 0.1f;
		
		GPU_Clear(screen);
		
		GPU_Image* top_image = image;
		if(using_images > 0)
        {
            GPU_Clear(images[0]->target);
            GPU_Blit(image, NULL, images[0]->target, image->w/2, image->h/2);
            GPU_CircleFilled(images[0]->target, 70, 70, 20, circleColor);
            
            for(i = 0; i < using_images-1; i++)
            {
                GPU_Clear(images[i+1]->target);
                GPU_Blit(images[i], NULL, images[i+1]->target, images[i]->w/2, images[i]->h/2);
            }
            
            top_image = images[using_images-1];
        }
        
        GPU_Blit(top_image, NULL, screen, top_image->w/2 + x, top_image->h/2 + y);
        
        if(using_images == 0)
            GPU_CircleFilled(screen, x + 70, y + 70, 20, circleColor);
		
		GPU_CircleFilled(screen, x + 70, y + 70, 20, circleColor2);
		
		GPU_Flip(screen);
		
		frameCount++;
		if(frameCount%500 == 0)
		{
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
		}
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	GPU_FreeImage(image);
	GPU_Quit();
	
	return 0;
}


