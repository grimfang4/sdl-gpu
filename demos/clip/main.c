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
	
	
	float dt = 0.010f;
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	int maxSprites = 50;
	int numSprites = 3;
	
	float x[maxSprites];
	float y[maxSprites];
	float velx[maxSprites];
	float vely[maxSprites];
	int i;
	for(i = 0; i < maxSprites; i++)
	{
		x[i] = rand()%screen->w;
		y[i] = rand()%screen->h;
		velx[i] = 10 + rand()%screen->w/10;
		vely[i] = 10 + rand()%screen->h/10;
	}
	
	SDL_Color lineColor = {255, 0, 0, 255};
	
	GPU_SetClip(screen, 40, 40, 600, 300);
	
	
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
				else if(event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_PLUS)
				{
					if(numSprites < maxSprites)
						numSprites++;
				}
				else if(event.key.keysym.sym == SDLK_MINUS)
				{
					if(numSprites > 0)
						numSprites--;
				}
			}
		}
		
		for(i = 0; i < numSprites; i++)
		{
			x[i] += velx[i]*dt;
			y[i] += vely[i]*dt;
			if(x[i] < 0)
			{
				x[i] = 0;
				velx[i] = -velx[i];
			}
			else if(x[i] + image->w > screen->w)
			{
				x[i] = screen->w - image->w;
				velx[i] = -velx[i];
			}
			
			if(y[i] < 0)
			{
				y[i] = 0;
				vely[i] = -vely[i];
			}
			else if(y[i] + image->h > screen->h)
			{
				y[i] = screen->h - image->h;
				vely[i] = -vely[i];
			}
		}
		
		GPU_Clear(screen);
		
		for(i = 0; i < numSprites; i++)
		{
			GPU_Blit(image, NULL, screen, x[i], y[i]);
		}
		
		GPU_Line(screen, 0, 0, screen->w, screen->h, lineColor);
		GPU_Line(screen, 0, screen->h, screen->w, 0, lineColor);
		
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


