#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "common.h"

#define IMAGE_FILE "data/test3.png"

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
		GPU_Image* target_image = GPU_CreateImage(screen->w, screen->h, GPU_FORMAT_RGBA);
		GPU_Target* target = GPU_LoadTarget(target_image);
		int mode = 0;
		
        image = GPU_LoadImage(IMAGE_FILE);
        if(image == NULL)
        {
            GPU_LogError("Failed to load image.\n");
            return -1;
        }
        
        GPU_SetAnchor(target_image, 0, 0);
        
        startTime = SDL_GetTicks();
        frameCount = 0;
        
        GPU_SetDepthTest(screen, 1);
        if(!GPU_AddDepthBuffer(target))
            return -2;

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
                    if(event.key.keysym.sym == SDLK_SPACE)
                        mode = !mode;
                }
            }
            
            GPU_ClearRGB(screen, 200, 200, 200);
            
            GPU_MatrixMode(screen, GPU_MODEL);
            if(!mode)
            {
                // Draw to screen directly
                
                // Images drawn left to right, but layered alternating.  Positive z values are on top.
                GPU_Blit(image, NULL, screen, 150, 300);
                
                GPU_PushMatrix();
                GPU_Translate(0, 0, 5);
                GPU_Blit(image, NULL, screen, 300, 300);
                
                GPU_Translate(0, 0, -10);
                GPU_Blit(image, NULL, screen, 450, 300);
                
                GPU_Translate(0, 0, 10);
                GPU_Blit(image, NULL, screen, 600, 300);
                GPU_PopMatrix();
            }
            else
            {
                // Draw to temp buffer
                GPU_Clear(target);
                
                // Images drawn left to right, but layered alternating.  Positive z values are on top.
                GPU_Blit(image, NULL, target, 150, 300);
                
                GPU_PushMatrix();
                GPU_Translate(0, 0, 5);
                GPU_Blit(image, NULL, target, 300, 300);
                
                GPU_Translate(0, 0, -10);
                GPU_Blit(image, NULL, target, 450, 300);
                
                GPU_Translate(0, 0, 10);
                GPU_Blit(image, NULL, target, 600, 300);
                GPU_PopMatrix();
                
                GPU_Blit(target_image, NULL, screen, 0, 0);
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


