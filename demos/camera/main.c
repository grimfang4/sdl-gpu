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
	
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	
	GPU_Image* img = GPU_LoadImage("data/test3.png");
	
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
				else if(event.key.keysym.sym == SDLK_r)
				{
					camera.x = 0.0f;
					camera.y = 0.0f;
					camera.z = -10.0f;
					camera.zoom = 1.0f;
					camera.angle = 0.0f;
				}
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
		if(keystates[SDLK_z])
		{
			camera.z -= 200*dt;
		}
		else if(keystates[SDLK_x])
		{
			camera.z += 200*dt;
		}
		if(keystates[SDLK_MINUS])
		{
			camera.zoom -= 1.0f*dt;
		}
		else if(keystates[SDLK_EQUALS])
		{
			camera.zoom += 1.0f*dt;
		}
		if(keystates[SDLK_COMMA])
		{
			camera.angle -= 100*dt;
		}
		else if(keystates[SDLK_PERIOD])
		{
			camera.angle += 100*dt;
		}
		
		GPU_ClearRGBA(screen, 255, 255, 255, 255);
		
		GPU_SetCamera(screen, &camera);
		
		GPU_Blit(img, NULL, screen, 50, 50);
		GPU_Blit(img, NULL, screen, 320, 50);
		GPU_Blit(img, NULL, screen, 50, 500);
		
		
		GPU_Flip();
		
		frameCount++;
		if(frameCount%500 == 0)
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	GPU_FreeImage(img);
	
	GPU_Quit();
	
	return 0;
}


