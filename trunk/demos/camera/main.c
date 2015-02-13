#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "compat.h"
#include "common.h"

#define PI 3.14159265359


void getScreenToWorld(float screenX, float screenY, float* worldX, float* worldY)
{
	GPU_Target* screen = GPU_GetContextTarget();
	GPU_Camera camera = GPU_GetCamera(screen);
	if(screen == NULL)
		return;

	if(worldX)
	{
		//if(camera.angle == 0.0f)
			*worldX = (screenX - screen->w/2) / camera.zoom + camera.x + screen->w/2;
		//else
			//*worldX = (screenX - screen->w/2) / camera.zoom * cos(-camera.angle*PI/180) - (screenY - screen->h/2) / camera.zoom * sin(-camera.angle*PI/180) + camera.x + screen->w/2;
	}
	if(worldY)
	{
		//if(camera.angle == 0.0f)
			*worldY = (screenY - screen->h/2) / camera.zoom + camera.y + screen->h/2;
		//else
			//*worldY = (screenX - screen->w/2) / camera.zoom * sin(-camera.angle*PI/180) + (screenY - screen->h/2) / camera.zoom * cos(-camera.angle*PI/180) + camera.y + screen->h/2;
	}
}

void getWorldToScreen(float worldX, float worldY, float* screenX, float* screenY)
{
	GPU_Target* screen = GPU_GetContextTarget();
	GPU_Camera camera = GPU_GetCamera(screen);
	if(screen == NULL)
		return;

	if(screenX)
	{
		//if(camera.angle == 0.0f)
			*screenX = (worldX - camera.x - screen->w/2)*camera.zoom + screen->w/2;
		//else
			//*screenX = (worldX - camera.x - screen->w/2)*camera.zoom * cos(-camera.angle*PI/180) + screen->w/2;
	}
	if(screenY)
	{
		//if(camera.angle == 0.0f)
			*screenY = (worldY - camera.y - screen->h/2)*camera.zoom + screen->h/2;
		//else
			//*screenY = (worldY - camera.y - screen->h/2)*camera.zoom * sin(-camera.angle*PI/180) + screen->h/2;
	}
}

void printScreenToWorld(float screenX, float screenY)
{
	float worldX, worldY;
	getScreenToWorld(screenX, screenY, &worldX, &worldY);

	printf("ScreenToWorld: (%.1f, %.1f) -> (%.1f, %.1f)\n", screenX, screenY, worldX, worldY);
}

void printWorldToScreen(float worldX, float worldY)
{
	float screenX, screenY;
	getWorldToScreen(worldX, worldY, &screenX, &screenY);

	printf("WorldToScreen: (%.1f, %.1f) -> (%.1f, %.1f)\n", worldX, worldY, screenX, screenY);
}


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
                }
                else if(event.type == SDL_MOUSEBUTTONDOWN)
                {
                    int mx, my;
                    float x, y;
                    SDL_GetMouseState(&mx, &my);
                    GPU_GetVirtualCoords(screen, &x, &y, mx, my);

                    printf("Angle: %.1f\n", camera.angle);
                    printScreenToWorld(x, y);
                    printWorldToScreen(50, 50);
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
                camera.zoom -= 1.0f*dt;
            }
            else if(keystates[KEY_EQUALS])
            {
                camera.zoom += 1.0f*dt;
            }
            if(keystates[KEY_COMMA])
            {
                camera.angle -= 100*dt;
            }
            else if(keystates[KEY_PERIOD])
            {
                camera.angle += 100*dt;
            }

            GPU_ClearRGBA(screen, 255, 255, 255, 255);
            GPU_SetCamera(screen, NULL);

            GPU_ClearRGBA(target, 255, 255, 255, 255);

            GPU_SetCamera(target, &camera);

            GPU_Rectangle(target, 0, 0, 800, 600, black);
            GPU_Blit(img, NULL, target, 50, 50);
            GPU_Blit(img, NULL, target, 320, 50);
            GPU_Blit(img, NULL, target, 50, 500);

            if(target != screen)
            {
                GPU_Blit(buffer, NULL, screen, buffer->w/2, buffer->h/2);
                GPU_CircleFilled(screen, 0, 0, 20, red);
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


