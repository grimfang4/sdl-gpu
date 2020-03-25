#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "common.h"
#include "compat.h"

#define MIN(a, b) ((a) < (b)? (a) : (b))

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
		GPU_Image* image2;
        int mode;
        int num_modes;
        float x, y;
        SDL_Color color = {0, 255, 0, 255};
        float dt;
        GPU_Camera camera;
        const Uint8* keystates;

        image = GPU_LoadImage("data/pixel_perfect.png");
        if(image == NULL)
            return -1;

        /*GPU_Image* gen = GPU_CreateImage(257, 257, GPU_FORMAT_RGB);
        GPU_LoadTarget(gen);
        for(int i = 0; i < gen->w/2; i++)
        {
            SDL_Color color = (i%2 == 0? GPU_MakeColor(255, 255, 255, 255) : GPU_MakeColor(0, 0, 255, 255));
            GPU_Rectangle(gen->target, i, i, gen->w-i-1, gen->h-1-i, color);
        }
        GPU_SaveImage(gen, "data/pixel_perfect_odd.png", GPU_FILE_AUTO);*/

        image2 = GPU_LoadImage("data/pixel_perfect_odd.png");
        if(image2 == NULL)
            return -2;

        mode = 0;
        num_modes = 7;
        x = 0.0f;
        y = 0.0f;

        dt = 0.010f;
        camera = GPU_GetDefaultCamera();

        keystates = SDL_GetKeyState(NULL);

        startTime = SDL_GetTicks();
        frameCount = 0;

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
                    else if(event.key.keysym.sym == SDLK_RETURN)
                    {
                        mode++;
                        if(mode >= num_modes)
                            mode = 0;
                    }
                    else if(event.key.keysym.sym == SDLK_r)
                    {
                        x = y = 0.0f;

                        camera = GPU_GetDefaultCamera();
                    }
                    else if(event.key.keysym.sym == SDLK_f)
                    {
                        if(image->filter_mode == GPU_FILTER_NEAREST)
                        {
                            GPU_SetImageFilter(image, GPU_FILTER_LINEAR);
                            GPU_SetImageFilter(image2, GPU_FILTER_LINEAR);
                            GPU_LogError("GPU_FILTER_LINEAR\n");
                        }
                        else
                        {
                            GPU_SetImageFilter(image, GPU_FILTER_NEAREST);
                            GPU_SetImageFilter(image2, GPU_FILTER_NEAREST);
                            GPU_LogError("GPU_FILTER_NEAREST\n");
                        }
                    }
                    else if(event.key.keysym.sym == SDLK_p)
                    {
                        GPU_SnapEnum mode = GPU_GetSnapMode(image);
                        if(mode == GPU_SNAP_POSITION_AND_DIMENSIONS)
                        {
                            GPU_SetSnapMode(image, GPU_SNAP_NONE);
                            GPU_SetSnapMode(image2, GPU_SNAP_NONE);
                            GPU_LogError("Pixel snap off\n");
                        }
                        else if(mode == GPU_SNAP_NONE)
                        {
                            GPU_SetSnapMode(image, GPU_SNAP_POSITION);
                            GPU_SetSnapMode(image2, GPU_SNAP_POSITION);
                            GPU_LogError("Pixel snap POSITION\n");
                        }
                        else if(mode == GPU_SNAP_POSITION)
                        {
                            GPU_SetSnapMode(image, GPU_SNAP_DIMENSIONS);
                            GPU_SetSnapMode(image2, GPU_SNAP_DIMENSIONS);
                            GPU_LogError("Pixel snap DIMENSIONS\n");
                        }
                        else
                        {
                            GPU_SetSnapMode(image, GPU_SNAP_POSITION_AND_DIMENSIONS);
                            GPU_SetSnapMode(image2, GPU_SNAP_POSITION_AND_DIMENSIONS);
                            GPU_LogError("Pixel snap POSITION_AND_DIMENSIONS\n");
                        }
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

            GPU_SetCamera(screen, &camera);

            GPU_Clear(screen);

            if(mode == 0)
            {
                // Blitting
                GPU_Blit(image, NULL, screen, x + image->w/2, y + image->h/2);
                GPU_Blit(image2, NULL, screen, x + image2->w/2, y + image->h + image2->h/2);

				float scale = 2.0f;
				GPU_BlitScale(image, NULL, screen, x + image->w + scale * image->w / 2, y + scale * image->h / 2, 2, 2);
				GPU_BlitScale(image2, NULL, screen, x + image2->w + scale * image2->w / 2, y + scale * image->h + scale * image2->h / 2, 2, 2);
            }
            else if(mode == 1)
            {
                // Vertical line
                GPU_Line(screen, x, y + 1, x, y + screen->h - 1, color);
            }
            else if(mode == 2)
            {
                // Horizontal line
                GPU_Line(screen, x + 1, y, x + screen->w - 1, y, color);
            }
            else if(mode == 3)
            {
                // Bounding rect
                GPU_Rectangle(screen, x, y, x + screen->w - 1, y + screen->h - 1, color);
            }
            else if(mode == 4)
            {
                // Filled bounding rect
                GPU_RectangleFilled(screen, x, y, x + screen->w - 1, y + screen->h - 1, color);
            }
            else if(mode == 5)
            {
                // Pixels
                int i;
                for(i = 0; i < 100; i++)
                    GPU_Pixel(screen, x + 2*(i%10), y + 2*(i/10), color);
            }
            else if(mode == 6)
            {
                // Circle
                GPU_Circle(screen, x + screen->w/2.0f, y + screen->h/2.0f, MIN(screen->w, screen->h)/2.0f - 1.0f, color);
            }

            GPU_Flip(screen);

            frameCount++;
            if(frameCount%500 == 0)
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        }

        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));

        GPU_FreeImage(image2);
        GPU_FreeImage(image);
	}

	GPU_Quit();

	return 0;
}


