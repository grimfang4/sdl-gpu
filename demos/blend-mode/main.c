#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>

void printRenderers(void)
{
	const char* renderers[GPU_GetNumRegisteredRenderers()];
	GPU_GetRegisteredRendererList(renderers);
	
	printf("Available renderers:\n");
	int i;
	for(i = 0; i < GPU_GetNumRegisteredRenderers(); i++)
	{
		printf("%d) %s\n", i+1, renderers[i]);
	}
}

int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(NULL, 800, 600, 0);
	if(screen == NULL)
		return -1;
	
	printf("Using renderer: %s\n", GPU_GetCurrentRendererID());
	
	GPU_Image* image = GPU_LoadImage("data/test3.png");
	if(image == NULL)
		return -1;
	
	GPU_Image* bg = GPU_LoadImage("data/test4.bmp");
	if(bg == NULL)
		return -1;
	
	Uint8* keystates = SDL_GetKeyState(NULL);
	float x = 0, y = 0;
	
	
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
			}
		}
		
		if(keystates[SDLK_UP])
			y -= 1;
		else if(keystates[SDLK_DOWN])
			y += 1;
		if(keystates[SDLK_LEFT])
			x -= 1;
		else if(keystates[SDLK_RIGHT])
			x += 1;
		
		GPU_Clear(screen);
		
		GPU_BlitScale(bg, NULL, screen, screen->w/2, screen->h/2, screen->w/(float)bg->w, screen->h/(float)bg->h);
		
		GPU_SetBlendMode(GPU_BLEND_NORMAL);
		GPU_Blit(image, NULL, screen, x+100, y+150);
		
		GPU_SetBlendMode(GPU_BLEND_MULTIPLY);
		GPU_Blit(image, NULL, screen, x+350, y+150);
		
		GPU_SetBlendMode(GPU_BLEND_DARKEN);
		GPU_Blit(image, NULL, screen, x+600, y+150);
		
		GPU_SetBlendMode(GPU_BLEND_LIGHTEN);
		GPU_Blit(image, NULL, screen, x+100, y+400);
		
		GPU_SetBlendMode(GPU_BLEND_DIFFERENCE);
		GPU_Blit(image, NULL, screen, x+350, y+400);
		
		GPU_SetBlendMode(GPU_BLEND_NORMAL);
		
		GPU_Flip();
		
		frameCount++;
		if(frameCount%500 == 0)
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	GPU_FreeImage(image);
	GPU_Quit();
	
	return 0;
}


