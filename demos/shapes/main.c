#include "SDL.h"
#include "SDL_gpu.h"
#include "SDL_gpuShapes.h"
#include <math.h>

int main(int argc, char* argv[])
{
	GPU_Target* screen = GPU_Init(800, 600, 0);
	if(screen == NULL)
		return -1;
	
	printf("Renderer: %s\n", GPU_GetRendererString());
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	int i;
	
	int numColors = 20;
	SDL_Color colors[numColors];
	for(i = 0; i < numColors; i++)
	{
		colors[i].r = rand()%256;
		colors[i].g = rand()%256;
		colors[i].b = rand()%256;
		colors[i].unused = rand()%256;
	}
	
	int numPixels = numColors;
	int px[numPixels];
	int py[numPixels];
	for(i = 0; i < numPixels; i++)
	{
		px[i] = rand()%screen->w;
		py[i] = rand()%screen->h;
	}
	
	int numLines = numColors;
	int lx1[numLines];
	int ly1[numLines];
	int lx2[numLines];
	int ly2[numLines];
	for(i = 0; i < numLines; i++)
	{
		lx1[i] = rand()%screen->w;
		ly1[i] = rand()%screen->h;
		lx2[i] = rand()%screen->w;
		ly2[i] = rand()%screen->h;
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
			}
		}
		
		GPU_Clear(screen);
		
		for(i = 0; i < numPixels; i++)
		{
			GPU_Pixel(screen, px[i], py[i], colors[i]);
		}
		
		for(i = 0; i < numLines; i++)
		{
			GPU_Line(screen, lx1[i], ly1[i], lx2[i], ly2[i], colors[i]);
		}
		
		GPU_Flip();
		
		frameCount++;
		if(frameCount%500 == 0)
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	GPU_Quit();
	
	return 0;
}


