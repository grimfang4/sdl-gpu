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
	
	GPU_Image* image = GPU_LoadImage("data/test.bmp");
	//GPU_Image* image = GPU_LoadImage("data/big_test.png");
	if(image == NULL)
		return -1;
	
	GPU_Image* image1 = GPU_CreateImage(image->w, image->h, 4);
	GPU_Target* image1_tgt = GPU_LoadTarget(image1);
	GPU_Blit(image, NULL, image1_tgt, image1_tgt->w/2, image1_tgt->h/2);
	GPU_FreeTarget(image1_tgt);
	
	GPU_Image* image2 = GPU_CopyImage(image);
	
	
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
		
		GPU_Blit(image, NULL, screen, screen->w/2, image->h/2);
		GPU_Blit(image1, NULL, screen, screen->w/2 - image1->w/2, screen->h/2 + image1->h/2);
		//GPU_Blit(image1, NULL, screen, image1->w/2, image1->h/2);
		GPU_Blit(image2, NULL, screen, screen->w/2 + image2->w/2, screen->h/2 + image2->h/2);
		
		GPU_Flip();
		
		frameCount++;
		if(frameCount%500 == 0)
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	GPU_FreeImage(image);
	GPU_FreeImage(image1);
	GPU_FreeImage(image2);
	GPU_Quit();
	
	return 0;
}


