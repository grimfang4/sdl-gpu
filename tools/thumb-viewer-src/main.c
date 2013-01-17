#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>

#ifdef SDL_GPU_USE_SDL2
#define SDL_GetKeyState SDL_GetKeyboardState
#endif

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
	
	int numImages = 0;
	
	GPU_Image* images[argc-1];
	int i;
	for(i = 1; i < argc; i++)
	{
		images[numImages] = GPU_LoadImage(argv[i]);
		if(images[numImages] != NULL)
			numImages++;
	}
	
	Uint8* keystates = SDL_GetKeyState(NULL);
	GPU_Camera camera = GPU_GetDefaultCamera();
	
	float dt = 0.010f;
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
		{
			camera.y -= 200*dt;
		}
		else if(keystates[SDLK_DOWN])
		{
			camera.y += 200*dt;
		}
		if(keystates[SDLK_LEFT])
		{
			camera.x -= 200*dt;
		}
		else if(keystates[SDLK_RIGHT])
		{
			camera.x += 200*dt;
		}
		if(keystates[SDLK_MINUS])
		{
			camera.zoom -= 1.0f*dt;
		}
		else if(keystates[SDLK_EQUALS])
		{
			camera.zoom += 1.0f*dt;
		}
		
		GPU_ClearRGBA(screen, 255, 255, 255, 255);
		
		GPU_SetCamera(screen, &camera);
		
		float x = 100;
		float y = 100;
		for(i = 0; i < numImages; i++)
		{
			float x_scale = 150.0f/images[i]->w;
			float y_scale = 150.0f/images[i]->h;
			GPU_BlitScale(images[i], NULL, screen, x, y, x_scale, y_scale);
			
			x += 200;
			
			if((i+1)%4 == 0)
			{
				x = 100;
				y += 200;
			}
		}
		
		
		GPU_Flip();
		SDL_Delay(10);
	}
	
	for(i = 0; i < numImages; i++)
	{
		GPU_FreeImage(images[i]);
	}
	
	GPU_Quit();
	
	return 0;
}


