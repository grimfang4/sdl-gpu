#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "compat.h"
#include "common.h"
#include <stdlib.h>


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

        GPU_Image* image;
        SDL_Color circleColor = {255, 0, 0, 128};
        SDL_Color circleColor2 = {0, 0, 255, 128};
        const Uint8* keystates;
        float x, y;
        int max_images;
        GPU_Image** images;
        int using_images;
        GPU_FilterEnum filter_mode;
        Uint8 show_original;
        Uint8 switched;
        int i;
		GPU_Image* top_image;

        image = GPU_LoadImage("data/test.bmp");
        if(image == NULL)
            return -1;

        GPU_LoadTarget(image);

        startTime = SDL_GetTicks();
        frameCount = 0;

        keystates = SDL_GetKeyState(NULL);
        x = 0;
        y = 0;

        // Intermediate image buffers
        max_images = 20;
        images = (GPU_Image**)malloc(sizeof(GPU_Image*)*max_images);

        using_images = 1;
        filter_mode = GPU_FILTER_NEAREST;
        show_original = 0;

        switched = 1;
        //GPU_SetVirtualResolution(screen, 640, 480);
        //GPU_SetVirtualResolution(image->target, 640, 480);
        GPU_SetImageFilter(image, filter_mode);

        for(i = 0; i < max_images; i++)
        {
            images[i] = GPU_CreateImage(640, 480, GPU_FORMAT_RGBA);
            GPU_LoadTarget(images[i]);
            //GPU_SetVirtualResolution(images[i]->target, 640, 480);
            GPU_SetImageFilter(images[i], filter_mode);
        }


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
                    if(event.key.keysym.sym == SDLK_SPACE)
                    {
                        if(switched)
                        {
                            GPU_SetVirtualResolution(screen, 800, 600);
                            GPU_SetVirtualResolution(image->target, 800, 600);
                            for(i = 0; i < max_images; i++)
                            {
                                GPU_SetVirtualResolution(images[i]->target, 800, 600);
                            }
                        }
                        else
                        {
                            GPU_SetVirtualResolution(screen, 640, 480);
                            GPU_SetVirtualResolution(image->target, 640, 480);
                            for(i = 0; i < max_images; i++)
                            {
                                GPU_SetVirtualResolution(images[i]->target, 640, 480);
                            }
                        }
                        switched = !switched;
                    }
                    else if(event.key.keysym.sym == SDLK_v)
                    {
                        GPU_UnsetVirtualResolution(screen);
                        GPU_UnsetVirtualResolution(image->target);
                        for(i = 0; i < max_images; i++)
                        {
                            GPU_UnsetVirtualResolution(images[i]->target);
                        }
                    }
                    else if(event.key.keysym.sym == SDLK_RETURN)
                        show_original = !show_original;
                    else if(event.key.keysym.sym == SDLK_f)
                    {
                        if(filter_mode == GPU_FILTER_NEAREST)
                        {
                            filter_mode = GPU_FILTER_LINEAR;
                            GPU_LogError("GPU_FILTER_LINEAR\n");
                        }
                        else
                        {
                            filter_mode = GPU_FILTER_NEAREST;
                            GPU_LogError("GPU_FILTER_NEAREST\n");
                        }


                        GPU_SetImageFilter(image, filter_mode);
                        for(i = 0; i < max_images; i++)
                        {
                            GPU_SetImageFilter(images[i], filter_mode);
                        }
                    }
                    else if(event.key.keysym.sym == SDLK_EQUALS)
                    {
                        if(using_images < max_images)
                            using_images++;
                        GPU_LogError("using_images: %d\n", using_images);
                    }
                    else if(event.key.keysym.sym == SDLK_MINUS)
                    {
                        if(using_images > 0)
                            using_images--;
                        GPU_LogError("using_images: %d\n", using_images);
                    }
                    else if(event.key.keysym.sym == SDLK_r)
                    {
                        x = y = 0.0f;
                    }
                    else if(event.key.keysym.sym == SDLK_UP)
                        y -= 1.0f;
                    else if(event.key.keysym.sym == SDLK_DOWN)
                        y += 1.0f;
                    else if(event.key.keysym.sym == SDLK_LEFT)
                        x -= 1.0f;
                    else if(event.key.keysym.sym == SDLK_RIGHT)
                        x += 1.0f;
                }
                else if(event.type == SDL_KEYUP)
                {
                    if(event.key.keysym.sym == SDLK_w
                       || event.key.keysym.sym == SDLK_s
                       || event.key.keysym.sym == SDLK_a
                       || event.key.keysym.sym == SDLK_d
                       || event.key.keysym.sym == SDLK_UP
                       || event.key.keysym.sym == SDLK_DOWN
                       || event.key.keysym.sym == SDLK_LEFT
                       || event.key.keysym.sym == SDLK_RIGHT)
                        GPU_LogError("x, y: (%.2f, %.2f)\n", x, y);
                }
            }

            if(keystates[KEY_w])
                y -= 0.1f;
            else if(keystates[KEY_s])
                y += 0.1f;
            if(keystates[KEY_a])
                x -= 0.1f;
            else if(keystates[KEY_d])
                x += 0.1f;

            GPU_Clear(screen);

            top_image = image;
            if(using_images > 0)
            {
                GPU_Clear(images[0]->target);
                GPU_Blit(image, NULL, images[0]->target, image->w/2, image->h/2);
                GPU_CircleFilled(images[0]->target, 70, 70, 20, circleColor);

                for(i = 0; i < using_images-1; i++)
                {
                    GPU_Clear(images[i+1]->target);
                    GPU_Blit(images[i], NULL, images[i+1]->target, images[i]->w/2, images[i]->h/2);
                }

                top_image = images[using_images-1];
            }

            GPU_Blit(top_image, NULL, screen, top_image->w/2 + x, top_image->h/2 + y);

            if(using_images == 0)
                GPU_CircleFilled(screen, x + 70, y + 70, 20, circleColor);

            GPU_CircleFilled(screen, x + 70, y + 70, 20, circleColor2);

            if(show_original)
                GPU_Blit(image, NULL, screen, image->w/2 + x, image->h/2 + y);

            GPU_Flip(screen);

            frameCount++;
            if(frameCount%500 == 0)
            {
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
            }
        }

        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));

        GPU_FreeImage(image);
        free(images);
	}

	GPU_Quit();

	return 0;
}


