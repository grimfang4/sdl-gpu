#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "common.h"
#include <stdlib.h>


void log_surface_details(SDL_Surface* surface)
{
    if(surface == NULL)
    {
        GPU_Log("Surface: NULL\n");
        return;
    }
    GPU_Log("Surface: %dx%d, %d bpp, %x Rmask, %x Gmask, %x Bmask, %x Amask\n", surface->w, surface->h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
}

void log_image_details(GPU_Image* image)
{
    if(image == NULL)
    {
        GPU_Log("Image: NULL\n");
        return;
    }
    GPU_Log("Image: %dx%d, %d bytes_per_pixel, %d layer%s, 0x%x format\n", image->w, image->h, image->bytes_per_pixel, image->num_layers, (image->num_layers == 1? "" : "s"), image->format);
}

GPU_Image* copy_and_log(SDL_Surface* surface)
{
	GPU_Image* image;
    log_surface_details(surface);
    image = GPU_CopyImageFromSurface(surface);
    log_image_details(image);
    return image;
}

void update_luminance_data(GPU_Image* image)
{
    int bytes_per_row = image->bytes_per_pixel * image->w;
    unsigned int size = image->bytes_per_pixel * image->w * image->h;
    unsigned char* bytes = (unsigned char*)malloc(size);
    
    // A vertical grayscale gradient
    int i;
    for(i = 0; i < size; i++)
    {
        bytes[i] = i/bytes_per_row * 255 / image->h;
    }
    
    GPU_UpdateImageBytes(image, NULL, bytes, bytes_per_row);
    free(bytes);
}

void update_luminance_alpha_data(GPU_Image* image)
{
    int bytes_per_row = image->bytes_per_pixel * image->w;
    unsigned int size = image->bytes_per_pixel * image->w * image->h;
    unsigned char* bytes = (unsigned char*)malloc(size);
    
    // A vertical grayscale gradient superimposed with a horizontal alpha gradient
    int i;
    for(i = 0; i < size; i++)
    {
        if(i%2 == 0)
            bytes[i] = i/bytes_per_row * 255 / image->h;
        else
            bytes[i] = i%bytes_per_row * 255 / bytes_per_row;
    }
    
    GPU_UpdateImageBytes(image, NULL, bytes, bytes_per_row);
    free(bytes);
}

void update_bgr_data(GPU_Image* image)
{
    if(image == NULL)
        return;
    
    int bytes_per_row = image->bytes_per_pixel * image->w;
    unsigned int num_pixels = image->w * image->h;
    unsigned char* bytes = (unsigned char*)malloc(image->bytes_per_pixel * num_pixels);
    
    // Alternating columns of red and white
    int i;
    for(i = 0; i < num_pixels; i++)
    {
        if((i/4)%2 == 0)
        {
            bytes[i*image->bytes_per_pixel] = 0;
            bytes[i*image->bytes_per_pixel+1] = 0;
            bytes[i*image->bytes_per_pixel+2] = 255;
        }
        else
        {
            bytes[i*image->bytes_per_pixel] = 255;
            bytes[i*image->bytes_per_pixel+1] = 255;
            bytes[i*image->bytes_per_pixel+2] = 255;
        }
    }
    
    GPU_UpdateImageBytes(image, NULL, bytes, bytes_per_row);
    free(bytes);
}

void update_bgra_data(GPU_Image* image)
{
    if(image == NULL)
        return;
    
    int bytes_per_row = image->bytes_per_pixel * image->w;
    unsigned int num_pixels = image->w * image->h;
    unsigned char* bytes = (unsigned char*)malloc(image->bytes_per_pixel * num_pixels);
    
    // Alternating columns of blue and white
    int i;
    for(i = 0; i < num_pixels; i++)
    {
        if((i/4)%2 == 0)
        {
            bytes[i*image->bytes_per_pixel] = 255;
            bytes[i*image->bytes_per_pixel+1] = 0;
            bytes[i*image->bytes_per_pixel+2] = 0;
            bytes[i*image->bytes_per_pixel+3] = 255;
        }
        else
        {
            bytes[i*image->bytes_per_pixel] = 255;
            bytes[i*image->bytes_per_pixel+1] = 255;
            bytes[i*image->bytes_per_pixel+2] = 255;
            bytes[i*image->bytes_per_pixel+3] = 255;
        }
    }
    
    GPU_UpdateImageBytes(image, NULL, bytes, bytes_per_row);
    free(bytes);
}

void update_abgr_data(GPU_Image* image)
{
    if(image == NULL)
        return;
    
    int bytes_per_row = image->bytes_per_pixel * image->w;
    unsigned int num_pixels = image->w * image->h;
    unsigned char* bytes = (unsigned char*)malloc(image->bytes_per_pixel * num_pixels);
    
    // Alternating columns of green and white
    int i;
    for(i = 0; i < num_pixels; i++)
    {
        if((i/4)%2 == 0)
        {
            bytes[i*image->bytes_per_pixel] = 255;
            bytes[i*image->bytes_per_pixel+1] = 0;
            bytes[i*image->bytes_per_pixel+2] = 255;
            bytes[i*image->bytes_per_pixel+3] = 0;
        }
        else
        {
            bytes[i*image->bytes_per_pixel] = 255;
            bytes[i*image->bytes_per_pixel+1] = 255;
            bytes[i*image->bytes_per_pixel+2] = 255;
            bytes[i*image->bytes_per_pixel+3] = 255;
        }
    }
    
    GPU_UpdateImageBytes(image, NULL, bytes, bytes_per_row);
    free(bytes);
}

void blit_copy(GPU_Image* src, GPU_Image* dst)
{
    GPU_Target* target = GPU_GetTarget(dst);
    
    if(target != NULL)
        GPU_Blit(src, NULL, target, target->w/2, target->h/2);
    else
        GPU_LogError("Failed to load target.\n");
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
		
        SDL_Surface* surface;
        GPU_Image* image8;
        GPU_Image* image24;
        GPU_Image* image32;
        GPU_Image* image_png;
        GPU_Image* image_luminance;
        GPU_Image* image_luminance_alpha;
        GPU_Image* image_bgr;
        GPU_Image* image_bgra;
        GPU_Image* image_abgr;
        
        SDL_Color corner_color;
        
        surface = SDL_LoadBMP("data/test_8bit.bmp");
        image8 = copy_and_log(surface);
        SDL_FreeSurface(surface);
        if(image8 == NULL)
            return -1;
        
        surface = SDL_LoadBMP("data/test_24bit.bmp");
        image24 = copy_and_log(surface);
        SDL_FreeSurface(surface);
        if(image24 == NULL)
            return -2;
        
        surface = SDL_LoadBMP("data/test_32bit.bmp");
        image32 = copy_and_log(surface);
        SDL_FreeSurface(surface);
        if(image32 == NULL)
            return -3;
        
        surface = GPU_LoadSurface("data/test3.png");
        image_png = copy_and_log(surface);
        SDL_FreeSurface(surface);
        if(image_png == NULL)
            return -4;
        
        image_luminance = GPU_CreateImage(200, 200, GPU_FORMAT_LUMINANCE);
        if(image_luminance == NULL)
            return -5;
        
        //image_luminance = GPU_CopyImage(image_luminance);
        log_image_details(image_luminance);
        update_luminance_data(image_luminance);
        
        image_luminance_alpha = GPU_CreateImage(200, 200, GPU_FORMAT_LUMINANCE_ALPHA);
        if(image_luminance_alpha == NULL)
            return -6;
        
        //image_luminance_alpha = GPU_CopyImage(image_luminance_alpha);
        log_image_details(image_luminance_alpha);
        update_luminance_alpha_data(image_luminance_alpha);
        
        GPU_Log("BGR\n");
        image_bgr = GPU_CreateImage(200, 200, GPU_FORMAT_BGR);
        if(image_bgr == NULL)
            return -7;
        
        log_image_details(image_bgr);
        update_bgr_data(image_bgr);
        
        GPU_Log("BGRA\n");
        image_bgra = GPU_CreateImage(200, 200, GPU_FORMAT_BGRA);
        if(image_bgra == NULL)
            return -8;
        
        log_image_details(image_bgra);
        update_bgra_data(image_bgra);
        corner_color = GPU_GetPixel(GPU_GetTarget(image_bgra), 0, 0);
        GPU_Log("BGRA upper corner in RGBA: (%u, %u, %u, %u)\n", corner_color.r, corner_color.g, corner_color.b, corner_color.a);
        
        GPU_Log("ABGR\n");
        image_abgr = GPU_CreateImage(200, 200, GPU_FORMAT_ABGR);
        if(image_abgr == NULL)
            return -9;
        
        log_image_details(image_abgr);
        update_abgr_data(image_abgr);
        
        
        GPU_Log("  Ready\n");
        
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
                }
            }
            
            GPU_Clear(screen);
            
            //GPU_Blit(image8, NULL, screen, image8->w/2, image8->h/2);
            //GPU_Blit(image24, NULL, screen, image8->w + image24->w/2, image24->h/2);
            //GPU_Blit(image32, NULL, screen, image32->w/2, image8->h + image32->h/2);
            //GPU_Blit(image_png, NULL, screen, image8->w + image_png->w/2, image24->h + image_png->h/2);
            
            
            //GPU_Blit(image_luminance, NULL, screen, image8->w + image24->w + image_luminance->w/2, image_luminance->h/2);
            //GPU_Blit(image_luminance_alpha, NULL, screen, image8->w + image24->w + image_luminance_alpha->w/2, image_luminance->h + image_luminance_alpha->h/2);
            GPU_Blit(image_bgr, NULL, screen, image8->w/2, image8->h/2);
            GPU_Blit(image_bgra, NULL, screen, image8->w + image24->w/2, image24->h/2);
            GPU_Blit(image_abgr, NULL, screen, image32->w/2, image8->h + image32->h/2);
            
            GPU_Flip(screen);
            
            frameCount++;
            if(frameCount%500 == 0)
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        }
        
        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        
        GPU_FreeImage(image_luminance);
        GPU_FreeImage(image_png);
        GPU_FreeImage(image32);
        GPU_FreeImage(image24);
        GPU_FreeImage(image8);
	}
	
	GPU_Quit();
	
	return 0;
}


