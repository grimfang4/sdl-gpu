#include "SDL.h"
#include "SDL_gpu.h"

int main(int argc, char* argv[])
{
	GPU_Target* screen = GPU_Init(800, 600, 0);
	if(screen == NULL)
		return -1;
	
	printf("Renderer: %s\n", GPU_GetRendererString());
	
	GPU_Image* image = GPU_LoadImage("data/test.bmp");
	if(image == NULL)
		return -1;
	
	float x = 0, y = 0;
	float velx = 50.0f, vely = 70.0f;
	
	float dt = 0.010f;
	
	float frameTimeAvg = 1.0f;
	
	Uint8 done = 0;
	SDL_Event event;
	while(!done)
	{
		Uint32 frameStart = SDL_GetTicks();
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
		
		x += velx*dt;
		y += vely*dt;
		if(x < 0)
		{
			x = 0;
			velx = -velx;
		}
		else if(x + image->w > screen->w)
		{
			x = screen->w - image->w;
			velx = -velx;
		}
		
		if(y < 0)
		{
			y = 0;
			vely = -vely;
		}
		else if(y + image->h > screen->h)
		{
			y = screen->h - image->h;
			vely = -vely;
		}
		
		GPU_Clear(screen);
		
		GPU_Blit(image, NULL, screen, x, y);
		
		GPU_Flip();
		
		frameTimeAvg = (frameTimeAvg + (SDL_GetTicks() - frameStart)/1000.0f)/2;
	}
	
	printf("Average FPS: %.2f\n", 1/frameTimeAvg);
	
	GPU_FreeImage(image);
	GPU_Quit();
}


