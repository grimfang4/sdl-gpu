#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "common.h"
#include "compat.h"



void set_pixel(SDL_Surface* surface, int x, int y, Uint32 color)
{
	int bpp;
	Uint8* bits;

    if(surface == NULL || x < 0 || y < 0 || x >= surface->w || y >= surface->h)
        return;

    bpp = surface->format->BytesPerPixel;
    bits = ((Uint8 *)surface->pixels) + y*surface->pitch + x*bpp;

    /* Set the pixel */
    switch(bpp)
    {
        case 1:
            *((Uint8 *)(bits)) = (Uint8)color;
            break;
        case 2:
            *((Uint16 *)(bits)) = (Uint16)color;
            break;
        case 3: { /* Format/endian independent */
            Uint8 r,g,b;
            r = (color >> surface->format->Rshift) & 0xFF;
            g = (color >> surface->format->Gshift) & 0xFF;
            b = (color >> surface->format->Bshift) & 0xFF;
            *((bits)+surface->format->Rshift/8) = r;
            *((bits)+surface->format->Gshift/8) = g;
            *((bits)+surface->format->Bshift/8) = b;
            }
            break;
        case 4:
            *((Uint32 *)(bits)) = (Uint32)color;
            break;
    }
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

        Uint32 rmask, gmask, bmask, amask;
        SDL_Surface* surface;

        Uint32 red_pixel;
        Uint32 white_pixel;

        GPU_Rect image_rect = {0, 0, 400, 400};
        GPU_Rect surface_rect = {0, 0, 400, 400};

        SDL_Color border_color = {255, 255, 255, 255};
        SDL_Color image_rect_color = {255, 255, 0, 255};
        SDL_Color surface_rect_color = {0, 255, 0, 255};

        const Uint8* keystates = SDL_GetKeyState(NULL);
        SDL_Keymod kmod;

        int mx, my;
        Uint32 mouse_state;

        GPU_Image* image = GPU_CreateImage(400, 400, GPU_FORMAT_RGBA);
        if(image == NULL)
            return -1;

    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
    #else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
    #endif

        surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 400, 400, 32,
                                       rmask, gmask, bmask, amask);
        if(surface == NULL)
            return -2;

        red_pixel = SDL_MapRGBA(surface->format, 255, 0, 0, 255);
        white_pixel = SDL_MapRGBA(surface->format, 255, 255, 255, 255);

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
                    if(event.key.keysym.sym == SDLK_SPACE)
                    {
                        SDL_FillRect(surface, NULL, red_pixel);
                    }
                }
            }

            kmod = SDL_GetModState();
            if(keystates[KEY_w])
            {
                if(kmod & KMOD_SHIFT)
                    image_rect.h -= 1;
                else
                    image_rect.y -= 1;
            }
            if(keystates[KEY_s])
            {
                if(kmod & KMOD_SHIFT)
                    image_rect.h += 1;
                else
                    image_rect.y += 1;
            }
            if(keystates[KEY_a])
            {
                if(kmod & KMOD_SHIFT)
                    image_rect.w -= 1;
                else
                    image_rect.x -= 1;
            }
            if(keystates[KEY_d])
            {
                if(kmod & KMOD_SHIFT)
                    image_rect.w += 1;
                else
                    image_rect.x += 1;
            }

            if(keystates[KEY_UP])
            {
                if(kmod & KMOD_SHIFT)
                    surface_rect.h -= 1;
                else
                    surface_rect.y -= 1;
            }
            if(keystates[KEY_DOWN])
            {
                if(kmod & KMOD_SHIFT)
                    surface_rect.h += 1;
                else
                    surface_rect.y += 1;
            }
            if(keystates[KEY_LEFT])
            {
                if(kmod & KMOD_SHIFT)
                    surface_rect.w -= 1;
                else
                    surface_rect.x -= 1;
            }
            if(keystates[KEY_RIGHT])
            {
                if(kmod & KMOD_SHIFT)
                    surface_rect.w += 1;
                else
                    surface_rect.x += 1;
            }

            mouse_state = SDL_GetMouseState(&mx, &my);
            if(mouse_state & SDL_BUTTON_LMASK)
            {
                set_pixel(surface, mx + image->w/2 - screen->w/2, my + image->h/2 - screen->h/2, red_pixel);
            }
            if(mouse_state & SDL_BUTTON_RMASK)
            {
                set_pixel(surface, mx + image->w/2 - screen->w/2, my + image->h/2 - screen->h/2, white_pixel);
            }

            GPU_Clear(screen);

            GPU_UpdateImage(image, &image_rect, surface, &surface_rect);

            GPU_Blit(image, NULL, screen, screen->w/2, screen->h/2);
            GPU_Rectangle(screen, screen->w/2 - image->w/2, screen->h/2 - image->h/2,
                                  screen->w/2 + image->w/2, screen->h/2 + image->h/2, border_color);
            GPU_Rectangle(screen, image_rect.x + screen->w/2 - image->w/2, image_rect.y + screen->h/2 - image->h/2,
                                  image_rect.x + image_rect.w + screen->w/2 - image->w/2, image_rect.y + image_rect.h + screen->h/2 - image->h/2, image_rect_color);
            GPU_Rectangle(screen, surface_rect.x + screen->w/2 - image->w/2, surface_rect.y + screen->h/2 - image->h/2,
                                  surface_rect.x + surface_rect.w + screen->w/2 - image->w/2, surface_rect.y + surface_rect.h + screen->h/2 - image->h/2, surface_rect_color);

            GPU_Flip(screen);

            frameCount++;
            if(frameCount%500 == 0)
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        }

        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));

        GPU_FreeImage(image);
	}

	GPU_Quit();

	return 0;
}


