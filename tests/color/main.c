#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "common.h"

void makeColorTransparent(GPU_Image* image, SDL_Color color)
{
    SDL_Surface* surface = GPU_CopySurfaceFromImage(image);
    Uint8* pixels = surface->pixels;
    
    int x,y,i;
    for(y = 0; y < surface->h; y++)
    {
        for(x = 0; x < surface->w; x++)
        {
            i = y*surface->pitch + x*surface->format->BytesPerPixel;
            if(pixels[i] == color.r && pixels[i+1] == color.g && pixels[i+2] == color.b)
                pixels[i+3] = 0;
        }
    }
    
    GPU_UpdateImage(image, NULL, surface, NULL);
    SDL_FreeSurface(surface);
}

void replaceColor(GPU_Image* image, SDL_Color from, SDL_Color to)
{
    SDL_Surface* surface = GPU_CopySurfaceFromImage(image);
    Uint8* pixels = surface->pixels;
    
    int x,y,i;
    for(y = 0; y < surface->h; y++)
    {
        for(x = 0; x < surface->w; x++)
        {
            i = y*surface->pitch + x*surface->format->BytesPerPixel;
            if(pixels[i] == from.r && pixels[i+1] == from.g && pixels[i+2] == from.b)
            {
                pixels[i] = to.r;
                pixels[i+1] = to.g;
                pixels[i+2] = to.b;
            }
        }
    }
    
    GPU_UpdateImage(image, NULL, surface, NULL);
    SDL_FreeSurface(surface);
}




#define MIN(a,b,c) (a<b && a<c? a : (b<c? b : c))
#define MAX(a,b,c) (a>b && a>c? a : (b>c? b : c))

// From Wikipedia?
static void rgb_to_hsv(int red, int green, int blue, int* hue, int* sat, int* val)
{
    float r = red/255.0f;
    float g = green/255.0f;
    float b = blue/255.0f;

    float h, s, v;

    float min, max, delta;
    min = MIN( r, g, b );
    max = MAX( r, g, b );

    v = max;				// v
    delta = max - min;
    if( max != 0 && min != max)
    {
        s = delta / max;		// s

        if( r == max )
            h = ( g - b ) / delta;		// between yellow & magenta
        else if( g == max )
            h = 2 + ( b - r ) / delta;	// between cyan & yellow
        else
            h = 4 + ( r - g ) / delta;	// between magenta & cyan
        h *= 60;				// degrees
        if( h < 0 )
            h += 360;
    }
    else {
        // r = g = b = 0		// s = 0, v is undefined
        s = 0;
        h = 0;// really undefined: -1
    }

    *hue = h * 256.0f/360.0f;
    *sat = s * 255;
    *val = v * 255;
}

// From Wikipedia?
static void hsv_to_rgb(int hue, int sat, int val, int* r, int* g, int* b)
{
    float h = hue/255.0f;
    float s = sat/255.0f;
    float v = val/255.0f;

    int H = floor(h*5.999f);
    float chroma = v*s;
    float x = chroma * (1 - fabs(fmod(h*5.999f, 2) - 1));

    unsigned char R = 0, G = 0, B = 0;
	unsigned char m;

    switch(H)
    {
    case 0:
        R = 255*chroma;
        G = 255*x;
        break;
    case 1:
        R = 255*x;
        G = 255*chroma;
        break;
    case 2:
        G = 255*chroma;
        B = 255*x;
        break;
    case 3:
        G = 255*x;
        B = 255*chroma;
        break;
    case 4:
        R = 255*x;
        B = 255*chroma;
        break;
    case 5:
        R = 255*chroma;
        B = 255*x;
        break;
    }

    m = 255*(v - chroma);

    *r = R+m;
    *g = G+m;
    *b = B+m;
}

static int clamp(int value, int low, int high)
{
    if(value <= low)
        return low;
    if(value >= high)
        return high;
    return value;
}

void shiftHSV(GPU_Image* image, int hue, int saturation, int value)
{
    SDL_Surface* surface = GPU_CopySurfaceFromImage(image);
    Uint8* pixels = surface->pixels;
    
    int x,y,i;
	int r, g, b, h, s, v;
    for(y = 0; y < surface->h; y++)
    {
        for(x = 0; x < surface->w; x++)
        {
            i = y*surface->pitch + x*surface->format->BytesPerPixel;
            
            if(surface->format->BytesPerPixel == 4 && pixels[i+3] == 0)
                continue;

            r = pixels[i];
            g = pixels[i+1];
            b = pixels[i+2];
            rgb_to_hsv(r, g, b, &h, &s, &v);
            h += hue;
            s += saturation;
            v += value;
            // Wrap hue
            while(h < 0)
                h += 256;
            while(h > 255)
                h -= 256;

            // Clamp
            s = clamp(s, 0, 255);
            v = clamp(v, 0, 255);

            hsv_to_rgb(h, s, v, &r, &g, &b);

            pixels[i] = r;
            pixels[i+1] = g;
            pixels[i+2] = b;
        }
    }
    
    GPU_UpdateImage(image, NULL, surface, NULL);
    SDL_FreeSurface(surface);
}

void shiftHSVExcept(GPU_Image* image, int hue, int saturation, int value, int notHue, int notSat, int notVal, int range)
{
    SDL_Surface* surface = GPU_CopySurfaceFromImage(image);
    Uint8* pixels = surface->pixels;
    
    int x,y,i;
	int r, g, b, h, s, v;
    for(y = 0; y < surface->h; y++)
    {
        for(x = 0; x < surface->w; x++)
        {
            i = y*surface->pitch + x*surface->format->BytesPerPixel;
            
            if(surface->format->BytesPerPixel == 4 && pixels[i+3] == 0)
                continue;

            r = pixels[i];
            g = pixels[i+1];
            b = pixels[i+2];
            rgb_to_hsv(r, g, b, &h, &s, &v);
            h += hue;
            s += saturation;
            v += value;
            // Wrap hue
            while(h < 0)
                h += 256;
            while(h > 255)
                h -= 256;

            // Clamp
            s = clamp(s, 0, 255);
            v = clamp(v, 0, 255);

            hsv_to_rgb(h, s, v, &r, &g, &b);
                        
            if(notHue - range <= h && notHue + range >= h
                    && notSat - range <= s && notSat + range >= s
                    && notVal - range <= v && notVal + range >= v)
                    continue;

            pixels[i] = r;
            pixels[i+1] = g;
            pixels[i+2] = b;
        }
    }
    
    GPU_UpdateImage(image, NULL, surface, NULL);
    SDL_FreeSurface(surface);
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
        GPU_Image* image;
        GPU_Image* image1;
        GPU_Image* image1a;
        SDL_Color yellow = {246, 255, 0};
        SDL_Color red = {200, 0, 0};
        GPU_Image* image2;
        GPU_Image* image3;
        GPU_Image* image4;
        
        image = GPU_LoadImage("data/test3.png");
        if(image == NULL)
            return -1;
        
        image1 = GPU_CopyImage(image);
        
        makeColorTransparent(image1, yellow);
        
        image1a = GPU_CopyImage(image);
        
        replaceColor(image1a, yellow, red);
        
        
        
        
        image2 = GPU_CopyImage(image);
        
        shiftHSV(image2, 100, 0, 0);
        
        image3 = GPU_CopyImage(image);
        
        shiftHSV(image3, 0, -100, 0);
        
        image4 = GPU_CopyImage(image);
        
        shiftHSV(image4, 0, 0, 100);
        
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
            
            GPU_Blit(image, NULL, screen, 150, 150);
            GPU_Blit(image1, NULL, screen, 300, 150);
            GPU_Blit(image1a, NULL, screen, 450, 150);
            GPU_Blit(image2, NULL, screen, 150, 300);
            GPU_Blit(image3, NULL, screen, 300, 300);
            GPU_Blit(image4, NULL, screen, 450, 300);
            
            GPU_Flip(screen);
            
            frameCount++;
            if(frameCount%500 == 0)
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        }
        
        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        
        GPU_FreeImage(image);
        GPU_FreeImage(image1);
        GPU_FreeImage(image1a);
        GPU_FreeImage(image2);
        GPU_FreeImage(image3);
        GPU_FreeImage(image4);
	}
	
	GPU_Quit();
	
	return 0;
}


