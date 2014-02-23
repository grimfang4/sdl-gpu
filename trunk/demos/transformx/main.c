#include "SDL.h"
#include "SDL_gpu.h"
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
	
	GPU_Image* screen2_image = GPU_CreateImage(800, 600, GPU_FORMAT_RGBA);
	GPU_Target* screen2 = GPU_LoadTarget(screen2_image);
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	GPU_Target* target = screen;
	float x = 400.0f;
	float y = 300.0f;
	
	Uint8 usingVirtual = 0;
	
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
				else if(event.key.keysym.sym == SDLK_SPACE)
				{
					if(target == screen)
						target = screen2;
					else
						target = screen;
				}
				else if(event.key.keysym.sym == SDLK_RETURN)
				{
				    usingVirtual = !usingVirtual;
				    
				    if(usingVirtual)
                    {
                        GPU_SetVirtualResolution(screen, 640, 480);
                        GPU_SetVirtualResolution(screen2, 640, 480);
                    }
                    else
                    {
                        GPU_SetVirtualResolution(screen, 800, 600);
                        GPU_SetVirtualResolution(screen2, 800, 600);
                    }
                    
				}
			}
		}
		
		GPU_Clear(screen);
		GPU_Clear(screen2);
		
		GPU_BlitScale(image, NULL, target, x, y, 0.1f, 0.1f);
		
		SDL_Rect area = {target->w/2 - 150, target->h/2 - 150, 300, 300};
		SDL_Color red = {255, 0, 0, 255};
		GPU_Rectangle(target, area.x, area.y, area.x + area.w, area.y + area.h, red);
		
		float scale_x = 0.25f;
		float scale_y = 0.5f;
		GPU_BlitTransformX(image, NULL, target, area.x, area.y, 0, 0, SDL_GetTicks()/10.0f, scale_x, scale_y);
		GPU_BlitTransformX(image, NULL, target, area.x + area.w, area.y, image->w, 0, SDL_GetTicks()/10.0f, scale_x, scale_y);
		GPU_BlitTransformX(image, NULL, target, area.x, area.y + area.h, 0, image->h, SDL_GetTicks()/10.0f, scale_x, scale_y);
		GPU_BlitTransformX(image, NULL, target, area.x + area.w, area.y + area.h, image->w, image->h, SDL_GetTicks()/10.0f, scale_x, scale_y);
		
		
		if(target == screen2)
			GPU_CircleFilled(screen2, 0, 0, 100, red);
		
		GPU_Blit(screen2_image, NULL, screen, 400, 300);
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


