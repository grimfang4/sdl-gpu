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
		return 1;

	printCurrentRenderer();

	{
		Uint32 startTime;
		long frameCount;
		Uint8 done;
		SDL_Event event;

        Uint32 rmask, gmask, bmask, amask;
        int num_surfaces = 4;
        SDL_Surface* surfaces[4];
        int i;
        int current_surface = 0;

        SDL_Color red = {255, 0, 0, 255};
        SDL_Color white = {255, 255, 255, 255};

        GPU_Rect surface_rect = {50, 70, 100, 100};

        SDL_Color border_color = {255, 255, 255, 255};
        SDL_Color surface_rect_color = {0, 255, 0, 255};

        const Uint8* keystates = SDL_GetKeyState(NULL);
        SDL_Keymod kmod;

        int mx, my;
        Uint32 mouse_state;

        GPU_Image* image;
        GPU_Image* selection_image;

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
        
        // Create surfaces which will replace the main image
        surfaces[0] = GPU_LoadSurface("data/test.bmp");
        if(surfaces[0] == NULL)
            return 2;
        
        for(i = 1; i < num_surfaces; ++i)
        {
            surfaces[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 50 * (i+1), 40 * (i+1), 32,
                                       rmask, gmask, bmask, amask);
            if(surfaces[i] == NULL)
                return 3;
            
            SDL_FillRect(surfaces[i], NULL, SDL_MapRGBA(surfaces[i]->format, rand()%256, rand()%256, rand()%256, 255));
        }
        
        image = GPU_CopyImageFromSurface(surfaces[0]);
        if(image == NULL)
            return 4;
        
        selection_image = GPU_CopyImageFromSurface(surfaces[0]);
        if(selection_image == NULL)
            return 5;
        
        // Fade out the selection image
        GPU_SetColor(selection_image, GPU_MakeColor(255, 255, 255, 100));
        
        if(!GPU_LoadTarget(image))
            return 6;

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
                    if(event.key.keysym.sym == SDLK_r)
                    {
                        GPU_Clear(image->target);
                    }
                    if(event.key.keysym.sym == SDLK_SPACE)
                    {
                        // Replace images with the next surface
                        current_surface++;
                        if(current_surface >= num_surfaces)
                            current_surface = 0;
                        
                        GPU_ReplaceImage(image, surfaces[current_surface], NULL);
                        GPU_ReplaceImage(selection_image, surfaces[current_surface], NULL);
                    }
                    if(event.key.keysym.sym == SDLK_RETURN)
                    {
                        // Replace the main image with a section of the current source surface
                        GPU_ReplaceImage(image, surfaces[current_surface], &surface_rect);
                    }
                }
            }

            kmod = SDL_GetModState();
            
            // Adjust source rectangle
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
            
            // Scribble some pixels on the current main image
            mouse_state = SDL_GetMouseState(&mx, &my);
            if(mouse_state & SDL_BUTTON_LMASK)
            {
                GPU_Pixel(image->target, mx + image->w/2 - screen->w/2, my + image->h/2 - screen->h/2, red);
            }
            if(mouse_state & SDL_BUTTON_RMASK)
            {
                GPU_Pixel(image->target, mx + image->w/2 - screen->w/2, my + image->h/2 - screen->h/2, white);
            }

            GPU_Clear(screen);
            
            // Draw selection image
            GPU_Blit(selection_image, NULL, screen, selection_image->w/2, selection_image->h/2);
            GPU_Rectangle(screen, 0, 0, selection_image->w, selection_image->h, border_color);
            
            // Draw source region
            GPU_Rectangle(screen, surface_rect.x, surface_rect.y,
                                  surface_rect.x + surface_rect.w, surface_rect.y + surface_rect.h, surface_rect_color);
            
            // Draw replaceable main image
            GPU_Blit(image, NULL, screen, screen->w/2, screen->h/2);
            GPU_Rectangle(screen, screen->w/2 - image->w/2, screen->h/2 - image->h/2,
                                  screen->w/2 + image->w/2, screen->h/2 + image->h/2, border_color);
            
            GPU_Flip(screen);

            frameCount++;
            if(frameCount%500 == 0)
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        }

        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));

        GPU_FreeImage(selection_image);
        GPU_FreeImage(image);
        
        for(i = 0; i < num_surfaces; ++i)
        {
            SDL_FreeSurface(surfaces[i]);
        }
	}

	GPU_Quit();

	return 0;
}


