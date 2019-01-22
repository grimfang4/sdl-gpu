#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "compat.h"
#include "common.h"

#define PI 3.14159265359


int main(int argc, char* argv[])
{
	GPU_Target* screen;

	GPU_Log("Running \"%s\"\n", argv[0]);
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
        SDL_Color red = {255, 0, 0, 255};
        SDL_Color black = {0, 0, 0, 255};
        GPU_Image* img;
        GPU_Image* buffer;
        GPU_Target* target;
        const Uint8* keystates;
        GPU_Camera camera;
        float dt;

        startTime = SDL_GetTicks();
        frameCount = 0;



        img = GPU_LoadImage("data/test3.png");
        buffer = GPU_CreateImage(800, 600, GPU_FORMAT_RGBA);
        GPU_LoadTarget(buffer);

        target = screen;

        keystates = SDL_GetKeyState(NULL);

        camera = GPU_GetDefaultCamera();

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
                    else if(event.key.keysym.sym == SDLK_r)
                    {
                        camera = GPU_GetDefaultCamera();
                    }
                    else if(event.key.keysym.sym == SDLK_RETURN)
                    {
                        if(target == screen)
                            target = buffer->target;
                        else
                            target = screen;
                    }
					else if (event.key.keysym.sym == SDLK_SPACE)
					{
					    if(screen->using_virtual_resolution)
                            GPU_UnsetVirtualResolution(screen);
                        else
                            GPU_SetVirtualResolution(screen, 400, 400);
					}
                    else if(event.key.keysym.sym == SDLK_w)
                    {
                        camera.y -= 100;
                    }
                    else if(event.key.keysym.sym == SDLK_s)
                    {
                        camera.y += 100;
                    }
                    else if(event.key.keysym.sym == SDLK_a)
                    {
                        camera.x -= 100;
                    }
                    else if(event.key.keysym.sym == SDLK_d)
                    {
                        camera.x += 100;
                    }
                    else if(event.key.keysym.sym == SDLK_o)
                    {
                        GPU_MatrixMode(target, GPU_PROJECTION);
                        GPU_LoadIdentity();
                        
                        camera.x = 0;
                        camera.y = 0;
                        camera.z = 0.0f;
                        
                        if(target->image == NULL)
                            GPU_Ortho(0, target->w, target->h, 0, target->camera.z_near, target->camera.z_far);
                        else
                            GPU_Ortho(0, target->w, 0, target->h, target->camera.z_near, target->camera.z_far);

                        GPU_MatrixMode(target, GPU_MODEL);
                    }
                    else if(event.key.keysym.sym == SDLK_p)
                    {
                        GPU_MatrixMode(target, GPU_PROJECTION);
                        GPU_LoadIdentity();
                        
                        camera.x = target->w/2;
                        camera.y = target->h/2;
                        camera.z = 1000.0f;
                        
                        if(target->image == NULL)
                            GPU_Frustum(-400, 400, 300, -300, 1000.0f, 10000.0f);
                        else
                            GPU_Frustum(-400, 400, -300, 300, 1000.0f, 10000.0f);
                        
                        GPU_MatrixMode(target, GPU_MODEL);
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
            if(keystates[KEY_z])
            {
                camera.z -= 200*dt;
            }
            else if(keystates[KEY_x])
            {
                camera.z += 200*dt;
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
            if(keystates[KEY_COMMA])
            {
                camera.angle -= 100*dt;
            }
            else if(keystates[KEY_PERIOD])
            {
                camera.angle += 100*dt;
            }
            
            GPU_ClearRGBA(target, 0, 0, 0, 255);
            
            // No camera view
            GPU_SetCamera(target, NULL);
            
            GPU_Circle(target, 0, 0, 25, GPU_MakeColor(255, 255, 255, 255));
            GPU_Circle(target, target->w, 0, 25, GPU_MakeColor(255, 0, 0, 255));
            GPU_Circle(target, target->w, target->h, 25, GPU_MakeColor(0, 255, 0, 255));
            GPU_Circle(target, 0, target->h, 25, GPU_MakeColor(0, 0, 255, 255));
            
            GPU_Rectangle(target, target->w/2 - 50, target->h/2 - 50, target->w/2 + 50, target->h/2 + 50, GPU_MakeColor(255, 255, 255, 255));

            // Camera's view
            GPU_SetCamera(target, &camera);
            
            GPU_Line(target, 0, target->h/2, target->w, target->h/2, GPU_MakeColor(255, 255, 255, 255));
            
            GPU_CircleFilled(target, 0, 0, 15, GPU_MakeColor(255, 255, 255, 255));
            GPU_CircleFilled(target, target->w, 0, 15, GPU_MakeColor(255, 0, 0, 255));
            GPU_CircleFilled(target, target->w, target->h, 15, GPU_MakeColor(0, 255, 0, 255));
            GPU_CircleFilled(target, 0, target->h, 15, GPU_MakeColor(0, 0, 255, 255));
            
            GPU_RectangleFilled(target, target->w/2 - 20, target->h/2 - 20, target->w/2 + 20, target->h/2 + 20, GPU_MakeColor(255, 0, 0, 255));
            
            if(target != screen)
            {
                GPU_ResetProjection(screen);
                GPU_SetCamera(screen, NULL);
                
                GPU_ClearRGBA(screen, 0, 0, 0, 255);
                GPU_BlitRect(buffer, NULL, screen, NULL);
            }
            
            GPU_Flip(screen);

            frameCount++;
            if(frameCount%500 == 0)
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        }

        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));

        GPU_FreeImage(img);
	}

	GPU_Quit();

	return 0;
}


