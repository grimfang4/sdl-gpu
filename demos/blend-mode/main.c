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
	
	GPU_Image* bg_base = GPU_LoadImage("data/test4.bmp");
	if(bg_base == NULL)
		return -1;
	
	GPU_Image* bg = GPU_CreateImage(bg_base->w, bg_base->h, 4);
	GPU_Target* bg_target = GPU_LoadTarget(bg);
	if(bg == NULL || bg_target == NULL)
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
		
		GPU_ClearRGBA(screen, 150, 150, 150, 255);
		// Draw a face under everything
		GPU_BlitScale(image, NULL, screen, screen->w/2, screen->h/2, screen->w/(float)image->w, screen->h/(float)image->h);
		
		GPU_Clear(bg_target);
		
		GPU_Blit(bg_base, NULL, bg_target, bg->w/2, bg->h/2);
		
		GPU_SetBlendMode(GPU_BLEND_NORMAL);
		GPU_BlitScale(image, NULL, bg_target, x+50, y+50, 0.5f, 0.5f);
		
		GPU_SetBlendMode(GPU_BLEND_MULTIPLY);
		GPU_BlitScale(image, NULL, bg_target, x+250, y+50, 0.5f, 0.5f);
		
		GPU_SetBlendMode(GPU_BLEND_DARKEN);
		GPU_BlitScale(image, NULL, bg_target, x+450, y+50, 0.5f, 0.5f);
		
		GPU_SetBlendMode(GPU_BLEND_LIGHTEN);
		GPU_BlitScale(image, NULL, bg_target, x+650, y+50, 0.5f, 0.5f);
		
		GPU_SetBlendMode(GPU_BLEND_DIFFERENCE);
		GPU_BlitScale(image, NULL, bg_target, x+50, y+250, 0.5f, 0.5f);
		
		GPU_SetBlendMode(GPU_BLEND_PUNCHOUT);
		GPU_BlitScale(image, NULL, bg_target, x+250, y+250, 0.5f, 0.5f);
		
		GPU_SetBlendMode(GPU_BLEND_CUTOUT);
		GPU_BlitScale(image, NULL, bg_target, x+450, y+250, 0.5f, 0.5f);
		
		GPU_SetBlendMode(GPU_BLEND_NORMAL);
		
		// Put our result on the screen target
		GPU_BlitScale(bg, NULL, screen, screen->w/2, screen->h/2, screen->w/(float)bg->w, screen->h/(float)bg->h);
		
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


