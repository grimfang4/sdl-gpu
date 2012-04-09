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
	
	GPU_Target* screen = GPU_Init(NULL, 800, 800, 0);
	if(screen == NULL)
		return -1;
	
	printf("Using renderer: %s\n", GPU_GetCurrentRendererID());
	
	GPU_Image* image = GPU_LoadImage("data/test.bmp");
	if(image == NULL)
		return -1;
	
	GPU_Image* image2 = GPU_CopyImage(image);
	GPU_GenerateMipmaps(image2);
	GPU_SetImageFilter(image2, GPU_LINEAR);
	
	GPU_Image* image3 = GPU_CopyImage(image);
	GPU_GenerateMipmaps(image3);
	GPU_SetImageFilter(image3, GPU_LINEAR_MIPMAP);
	
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
		
		GPU_Clear(screen);
		
		float x = 0;
		float y = image->h/2;
		GPU_Blit(image, NULL, screen, x + image->w/2, y);
		x += image->w + 10;
		GPU_BlitScale(image, NULL, screen, x + 0.5f*image->w/2, y, 0.5f, 0.5f);
		x += 0.5f*image->w + 10;
		GPU_BlitScale(image, NULL, screen, x + 0.25f*image->w/2, y, 0.25f, 0.25f);
		x += 0.25f*image->w + 10;
		GPU_BlitScale(image, NULL, screen, x + 0.125f*image->w/2, y, 0.125f, 0.125f);
		x += 0.125f*image->w + 10;
		GPU_BlitScale(image, NULL, screen, x + 0.0625f*image->w/2, y, 0.0625f, 0.0625f);
		x += 0.0625f*image->w + 10;
		GPU_BlitScale(image, NULL, screen, x + 0.03125f*image->w/2, y, 0.03125f, 0.03125f);
		
		x = 0;
		y = image->h + 10 + image->h/2;
		GPU_Blit(image2, NULL, screen, x + image->w/2, y);
		x += image->w + 10;
		GPU_BlitScale(image2, NULL, screen, x + 0.5f*image->w/2, y, 0.5f, 0.5f);
		x += 0.5f*image->w + 10;
		GPU_BlitScale(image2, NULL, screen, x + 0.25f*image->w/2, y, 0.25f, 0.25f);
		x += 0.25f*image->w + 10;
		GPU_BlitScale(image2, NULL, screen, x + 0.125f*image->w/2, y, 0.125f, 0.125f);
		x += 0.125f*image->w + 10;
		GPU_BlitScale(image2, NULL, screen, x + 0.0625f*image->w/2, y, 0.0625f, 0.0625f);
		x += 0.0625f*image->w + 10;
		GPU_BlitScale(image2, NULL, screen, x + 0.03125f*image->w/2, y, 0.03125f, 0.03125f);
		
		x = 0;
		y = 2*(image->h + 10) + image->h/2;
		GPU_Blit(image3, NULL, screen, x + image->w/2, y);
		x += image->w + 10;
		GPU_BlitScale(image3, NULL, screen, x + 0.5f*image->w/2, y, 0.5f, 0.5f);
		x += 0.5f*image->w + 10;
		GPU_BlitScale(image3, NULL, screen, x + 0.25f*image->w/2, y, 0.25f, 0.25f);
		x += 0.25f*image->w + 10;
		GPU_BlitScale(image3, NULL, screen, x + 0.125f*image->w/2, y, 0.125f, 0.125f);
		x += 0.125f*image->w + 10;
		GPU_BlitScale(image3, NULL, screen, x + 0.0625f*image->w/2, y, 0.0625f, 0.0625f);
		x += 0.0625f*image->w + 10;
		GPU_BlitScale(image3, NULL, screen, x + 0.03125f*image->w/2, y, 0.03125f, 0.03125f);
		
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


