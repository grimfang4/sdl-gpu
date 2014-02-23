#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "compat.h"
#include "common.h"

int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	GPU_Image* image = GPU_LoadImage("data/test.bmp");
	//GPU_Image* image = GPU_LoadImage("data/big_test.png");
	if(image == NULL)
		return -1;
	
	// Copying the annoying way
	GPU_Image* image1 = GPU_CreateImage(image->w, image->h, GPU_FORMAT_RGBA);
	GPU_Target* image1_tgt = GPU_LoadTarget(image1);
	GPU_Blit(image, NULL, image1_tgt, image1_tgt->w/2, image1_tgt->h/2);
	GPU_FreeTarget(image1_tgt);
	
	// Copying the normal way
	GPU_Image* image2 = GPU_CopyImage(image);
	
	// Copying from a surface dump
	SDL_Surface* surface = GPU_CopySurfaceFromImage(image);
    //GPU_SaveSurface(surface, "save_surf1.bmp");
	GPU_Image* image3 = GPU_CopyImageFromSurface(surface);
	SDL_FreeSurface(surface);
	
	// A buffer for window capture
	GPU_Image* image4 = NULL;
	
	
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
				else if(event.key.keysym.sym == SDLK_SPACE)
                {
                    // Take a window capture
                    GPU_FreeImage(image4);
                    image4 = GPU_CopyImageFromTarget(screen);
                }
			}
		}
		
		if(keystates[KEY_UP])
		{
			camera.y -= 200*dt;
		}
		else if(keystates[KEY_DOWN])
		{
			camera.y += 200*dt;
		}
		if(keystates[KEY_LEFT])
		{
			camera.x -= 200*dt;
		}
		else if(keystates[KEY_RIGHT])
		{
			camera.x += 200*dt;
		}
		if(keystates[KEY_MINUS])
		{
			camera.zoom -= 1.0f*dt;
		}
		else if(keystates[KEY_EQUALS])
		{
			camera.zoom += 1.0f*dt;
		}
		
		
		GPU_ClearRGBA(screen, 100, 100, 100, 255);
		
		GPU_SetCamera(screen, &camera);
		
		GPU_Blit(image, NULL, screen, 128, 128);
		GPU_Blit(image1, NULL, screen, 128 + 256, 128);
		GPU_Blit(image2, NULL, screen, 128 + 512, 128);
		GPU_Blit(image3, NULL, screen, 128, 128 + 256);
		
		if(image4 != NULL)
            GPU_BlitScale(image4, NULL, screen, 3*screen->w/4, 3*screen->h/4, 0.25f, 0.25f);
		
		GPU_Flip(screen);
		
		frameCount++;
		if(frameCount%500 == 0)
			GPU_LogError("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	}
	
	GPU_LogError("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	GPU_FreeImage(image);
	GPU_FreeImage(image1);
	GPU_FreeImage(image2);
	GPU_FreeImage(image3);
	GPU_FreeImage(image4);
	GPU_Quit();
	
	return 0;
}


