#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "compat.h"
#include "common.h"

int main(int argc, char* argv[])
{
	GPU_Target* screen;

	printRenderers();
	
	screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	{
		Uint32 startTime;
		long frameCount;
		Uint8 done;
		SDL_Event event;
		const Uint8* keystates;
        GPU_Camera camera;
		float dt;
		SDL_Surface* surface;
		GPU_Image* image;
		GPU_Image* image1;
		GPU_Image* image2;
		GPU_Image* image3;
		GPU_Image* image4;
		
        image = GPU_LoadImage("data/test.bmp");
        //image = GPU_LoadImage("data/big_test.png");
        if(image == NULL)
            return -1;

        // Copying the annoying way
        image1 = GPU_CreateImage(image->w, image->h, GPU_FORMAT_RGBA);
        GPU_LoadTarget(image1);
        GPU_Blit(image, NULL, image1->target, image1->target->w/2, image1->target->h/2);
        GPU_FreeTarget(image1->target);

        // Copying the normal way
        image2 = GPU_CopyImage(image);

        // Copying from a surface dump
        surface = GPU_CopySurfaceFromImage(image);
        //GPU_SaveSurface(surface, "save_surf1.bmp", GPU_FILE_AUTO);
        image3 = GPU_CopyImageFromSurface(surface);
        SDL_FreeSurface(surface);

        // A buffer for window capture
        image4 = NULL;


        keystates = SDL_GetKeyState(NULL);
        camera = GPU_GetDefaultCamera();

        startTime = SDL_GetTicks();
        frameCount = 0;

        dt = 0.010f;
        done = 0;
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
                camera.zoom_x -= 1.0f*dt;
                camera.zoom_y -= 1.0f*dt;
            }
            else if(keystates[KEY_EQUALS])
            {
                camera.zoom_x += 1.0f*dt;
                camera.zoom_y += 1.0f*dt;
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
	}
	
	GPU_Quit();
	
	return 0;
}


