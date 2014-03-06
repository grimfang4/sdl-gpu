#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "common.h"
#include "compat.h"

#define MIN(a, b) ((a) < (b)? (a) : (b))

int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	GPU_Image* image = GPU_LoadImage("data/pixel_perfect.png");
	if(image == NULL)
		return -1;
	
	int mode = 0;
	int num_modes = 7;
	float x = 0.0f;
	float y = 0.0f;
	SDL_Color color = {0, 255, 0, 255};
	
	Uint8* keystates = SDL_GetKeyState(NULL);
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
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
				else if(event.key.keysym.sym == SDLK_RETURN)
                {
					mode++;
					if(mode >= num_modes)
                        mode = 0;
                }
				else if(event.key.keysym.sym == SDLK_r)
                {
					x = y = 0.0f;
                }
				else if(event.key.keysym.sym == SDLK_f)
                {
                    if(image->filter_mode == GPU_FILTER_NEAREST)
                    {
                        GPU_SetImageFilter(image, GPU_FILTER_LINEAR);
                        GPU_LogError("GPU_FILTER_LINEAR\n");
                    }
                    else
                    {
                        GPU_SetImageFilter(image, GPU_FILTER_NEAREST);
                        GPU_LogError("GPU_FILTER_NEAREST\n");
                    }
                }
				else if(event.key.keysym.sym == SDLK_UP)
					y -= 1.0f;
				else if(event.key.keysym.sym == SDLK_DOWN)
					y += 1.0f;
				else if(event.key.keysym.sym == SDLK_LEFT)
					x -= 1.0f;
				else if(event.key.keysym.sym == SDLK_RIGHT)
					x += 1.0f;
			}
			else if(event.type == SDL_KEYUP)
			{
                GPU_LogError("x, y: (%.2f, %.2f)\n", x, y);
			}
		}
		
        if(keystates[KEY_w])
            y -= 0.1f;
        else if(keystates[KEY_s])
            y += 0.1f;
        if(keystates[KEY_a])
            x -= 0.1f;
        else if(keystates[KEY_d])
            x += 0.1f;
		
		GPU_Clear(screen);
		
		if(mode == 0)
        {
            // Blitting
            GPU_Blit(image, NULL, screen, x + image->w/2.0f, y + image->h/2.0f);
        }
        else if(mode == 1)
        {
            // Vertical line
            GPU_Line(screen, x, y + 1, x, y + screen->h - 1, color);
        }
        else if(mode == 2)
        {
            // Horizontal line
            GPU_Line(screen, x + 1, y, x + screen->w - 1, y, color);
        }
        else if(mode == 3)
        {
            // Bounding rect
            GPU_Rectangle(screen, x, y, x + screen->w - 1, y + screen->h - 1, color);
        }
        else if(mode == 4)
        {
            // Filled bounding rect
            GPU_RectangleFilled(screen, x, y, x + screen->w - 1, y + screen->h - 1, color);
        }
        else if(mode == 5)
        {
            // Pixels
            int i;
            for(i = 0; i < 100; i++)
                GPU_Pixel(screen, x + 2*(i%10), y + 2*(i/10), color);
        }
        else if(mode == 6)
        {
            // Circle
            GPU_Circle(screen, x + screen->w/2.0f, y + screen->h/2.0f, MIN(screen->w, screen->h)/2.0f - 1.0f, color);
        }
		
		GPU_Flip(screen);
		
		frameCount++;
		if(frameCount%500 == 0)
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	GPU_FreeImage(image);
	GPU_Quit();
	
	return 0;
}


