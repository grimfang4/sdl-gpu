#include "SDL_gpu.h"
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif


typedef struct Ship
{
    GPU_Image* image;

    // Motion
	float pos_x, pos_y;
	float vel_x, vel_y;
	float angle;

	// Properties of the ship
	float thrust;
	float drag_coefficient;
} Ship;

Ship* create_ship(const char* image_file)
{
    Ship* ship = malloc(sizeof(Ship));
    if(ship == NULL)
        return NULL;

	ship->image = GPU_LoadImage(image_file);
	ship->pos_x = 0.0f;
	ship->pos_y = 0.0f;
	ship->vel_x = 0.0f;
	ship->vel_y = 0.0f;
	ship->angle = 0.0f;
	ship->thrust = 500.0f;
	ship->drag_coefficient = 0.00005f;

	return ship;
}

void free_ship(Ship* ship)
{
    if(ship == NULL)
        return;

    GPU_FreeImage(ship->image);
    free(ship);
}

void apply_thrust(Ship* ship, float dt)
{
    ship->vel_x += ship->thrust * cosf(ship->angle) * dt;
    ship->vel_y += ship->thrust * sinf(ship->angle) * dt;
}

void apply_drag(Ship* ship, float dt)
{
    float vel_angle = atan2f(ship->vel_y, ship->vel_x);
    float vel = sqrtf(ship->vel_x*ship->vel_x + ship->vel_y*ship->vel_y);
    vel -= ship->drag_coefficient * vel * vel;
    if(vel < 0)
        vel = 0;

    ship->vel_x = vel * cosf(vel_angle);
    ship->vel_y = vel * sinf(vel_angle);
}

void update_ship(Ship* ship, GPU_Rect play_area, float dt)
{
    ship->pos_x += ship->vel_x * dt;
    ship->pos_y += ship->vel_y * dt;

    if(ship->pos_x < play_area.x)
    {
        ship->pos_x = play_area.x;
        ship->vel_x = -ship->vel_x;
    }
    else if(ship->pos_x >= play_area.x + play_area.w)
    {
        ship->pos_x = play_area.x + play_area.w;
        ship->vel_x = -ship->vel_x;
    }

    if(ship->pos_y < play_area.y)
    {
        ship->pos_y = play_area.y;
        ship->vel_y = -ship->vel_y;
    }
    else if(ship->pos_y >= play_area.y + play_area.h)
    {
        ship->pos_y = play_area.y + play_area.h;
        ship->vel_y = -ship->vel_y;
    }
}

void draw_ship(Ship* ship, GPU_Target* screen)
{
    GPU_BlitRotate(ship->image, NULL, screen, ship->pos_x, ship->pos_y, ship->angle * 180 / M_PI);
}



int main(int argc, char* argv[])
{
	GPU_Target* screen;
    
    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);
    
	screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;


	{
        // Loop variables
		Uint32 start_time;
		Uint32 end_time;
        float dt;
		Uint8 done;


        // Interaction variables

        Uint32 mouse_state;
        int mouse_x, mouse_y;
		SDL_Event event;

        // Game variables

        GPU_Rect play_area = {0, 0, screen->w, screen->h};

        Ship* player_ship = create_ship("data/tutorial-space/ship.png");

        if(player_ship == NULL || player_ship->image == NULL)
        {
            free_ship(player_ship);
            return -2;
        }


        player_ship->pos_x = play_area.x + play_area.w/2;
        player_ship->pos_y = play_area.y + play_area.h/2;


        start_time = SDL_GetTicks();
        dt = 0.0f;

        done = 0;


        // Game loop

        while(!done)
        {
            // Check events

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

            // Update

            mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);

            apply_drag(player_ship, dt);

            player_ship->angle = atan2f(mouse_y - player_ship->pos_y, mouse_x - player_ship->pos_x);
            if(mouse_state & SDL_BUTTON_LMASK)
                apply_thrust(player_ship, dt);

            update_ship(player_ship, play_area, dt);


            // Draw

            GPU_Clear(screen);

            draw_ship(player_ship, screen);

            GPU_Flip(screen);


            // Timing

            SDL_Delay(10);
            end_time = SDL_GetTicks();
            dt = (end_time - start_time)/1000.0f;
            start_time = end_time;
        }


        // Clean up

        free_ship(player_ship);
	}

	GPU_Quit();

	return 0;
}


