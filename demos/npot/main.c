#include "SDL.h"
#include "SDL_gpu.h"
#include "OpenGL/glew.h"
#include "OpenGL/SDL_gpu_OpenGL.h"
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
	
	GPU_Image* image = GPU_LoadImage("data/npot1.png");
	GPU_Image* image2 = GPU_LoadImage("data/npot1.bmp");
	if(image == NULL || image2 == NULL)
		return -1;
	
	printf("w, h: %d, %d\n", image->w, image->h);
	printf("tex_w, tex_h: %d, %d\n", ((ImageData_OpenGL*)image->data)->tex_width, ((ImageData_OpenGL*)image->data)->tex_height);
	
	int mode = 0;
	
	Uint8* keystates = SDL_GetKeyState(NULL);
	GPU_Camera camera = GPU_GetDefaultCamera();
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
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
				if(event.key.keysym.sym == SDLK_SPACE)
				{
					mode++;
					if(mode > 1)
						mode = 0;
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
		if(keystates[SDLK_MINUS])
		{
			camera.zoom -= 1.0f*dt;
		}
		else if(keystates[SDLK_EQUALS])
		{
			camera.zoom += 1.0f*dt;
		}
		
		
		GPU_ClearRGBA(screen, 100, 100, 100, 255);
		
		GPU_SetCamera(screen, &camera);
		
		if(mode == 0)
			GPU_Blit(image, NULL, screen, screen->w/2, screen->h/2);
		else if(mode == 1)
			GPU_Blit(image2, NULL, screen, screen->w/2, screen->h/2);
		
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


