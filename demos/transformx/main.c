#include "SDL.h"
#include "SDL_gpu.h"

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
	if(image == NULL)
		return -1;
	
	GPU_Image* screen2_image = GPU_CreateImage(800, 600, 4);
	GPU_Target* screen2 = GPU_LoadTarget(screen2_image);
	
	float dt = 0.010f;
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	GPU_Target* target = screen;
	float x = 400.0f;
	float y = 300.0f;
	SDL_Color red = {255, 0, 0, 255};
	
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
			}
		}
		
		GPU_Clear(screen);
		GPU_Clear(screen2);
		
		GPU_BlitScale(image, NULL, target, x, y, 0.1f, 0.1f);
		GPU_BlitTransformX(image, NULL, target, x, y, 100, 100, SDL_GetTicks()/10.0f, 0.25f, 0.25f);
		
		
		if(target == screen2)
			GPU_CircleFilled(screen2, 0, 0, 100, red);
		
		GPU_Blit(screen2_image, NULL, screen, 400, 300);
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


