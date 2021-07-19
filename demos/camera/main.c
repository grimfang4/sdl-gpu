#include <SDL.h>
#include "SDL_gpu.h"

#define MOVE_VEL 800
#define ZOOM_VEL 2
#define ROTATE_VEL 45

void main_loop(GPU_Target* screen)
{
    Uint8 done;
    SDL_Event event;
    Uint32 start_time, end_time;
    float dt;
    GPU_Camera camera;
    GPU_Image *image1, *image2;

    image1 = GPU_LoadImage("data/test3.png");
    image2 = GPU_LoadImage("data/test2.png");

    camera = GPU_GetDefaultCamera();

    start_time = SDL_GetTicks();
    dt = 0.0f;

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
                else if(event.key.keysym.sym == SDLK_c)
                    camera.use_centered_origin = !camera.use_centered_origin;
                else if(event.key.keysym.sym == SDLK_w)
                    camera.y -= MOVE_VEL*dt;
                else if(event.key.keysym.sym == SDLK_s)
                    camera.y += MOVE_VEL*dt;
                else if(event.key.keysym.sym == SDLK_a)
                    camera.x -= MOVE_VEL*dt;
                else if(event.key.keysym.sym == SDLK_d)
                    camera.x += MOVE_VEL*dt;
                else if(event.key.keysym.sym == SDLK_d)
                    camera.x += MOVE_VEL*dt;
                else if(event.key.keysym.sym == SDLK_r)
                    camera.zoom_x = camera.zoom_y += ZOOM_VEL*dt;
                else if(event.key.keysym.sym == SDLK_f)
                    camera.zoom_x = camera.zoom_y -= ZOOM_VEL*dt;
                else if(event.key.keysym.sym == SDLK_q)
                    camera.angle -= ROTATE_VEL*dt;
                else if(event.key.keysym.sym == SDLK_e)
                    camera.angle += ROTATE_VEL*dt;
            }
        }

        GPU_SetCamera(screen, &camera);

        GPU_Clear(screen);

        for (int i = -10; i < 10; i++) {
            GPU_BlitScale(image2, NULL, screen, i*128, 0, 0.5f, 0.5f);
            GPU_BlitScale(image2, NULL, screen, 0, i*128, 0.5f, 0.5f);
            GPU_BlitScale(image1, NULL, screen, i*100, i*100, 0.5f, 0.5f);
            GPU_BlitScale(image1, NULL, screen, i*100, -i*100, 0.5f, 0.5f);
        }

        GPU_Flip(screen);

        SDL_Delay(10);
        end_time = SDL_GetTicks();
        dt = (end_time - start_time)/1000.0f;
        start_time = end_time;
    }
}

int main(int argc, char* argv[])
{
    GPU_Target* screen;

    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_1);

    screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
    if(screen == NULL)
        return -1;

    GPU_EnableCamera(screen, true);

    main_loop(screen);

    GPU_Quit();

    return 0;
}


