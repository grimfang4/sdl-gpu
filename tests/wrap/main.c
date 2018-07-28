#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"
#include "demo-font.h"


int main(int argc, char* argv[])
{
	GPU_Target* screen;

	printRenderers();
	
	screen = GPU_Init(1024, 700, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	{
		Uint32 startTime;
		long frameCount;
		Uint8 done;
		SDL_Event event;
		
        float x, y;
        SDL_Surface* font_surface;
        DemoFont* font;
        GPU_Rect rect1, rect2;
        
        GPU_Image* image = GPU_LoadImage("data/happy_52x63.bmp");
        if(image == NULL)
            return -1;
        
        
        font_surface = GPU_LoadSurface("data/comic14.png");
        font = FONT_Alloc(font_surface);
        GPU_SetRGB(font->image, 255, 0, 0);
        SDL_FreeSurface(font_surface);
        
        
        rect1 = GPU_MakeRect(0.0f, 0.0f, image->w*3, image->h*3);
        rect2 = GPU_MakeRect(-image->w*2, -image->h*2, image->w*3, image->h*3);
        
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
            
            
            x = 0;
            y = 0;
            FONT_Draw(font, screen, x + 10, y + 10, "NONE");
            x += 40;
            y += 20;
            GPU_SetWrapMode(image, GPU_WRAP_NONE, GPU_WRAP_NONE);
            GPU_Blit(image, &rect1, screen, x + rect1.w/2, y + rect1.h/2);
            x += image->w*4;
            GPU_Blit(image, &rect2, screen, x + rect2.w/2, y + rect2.h/2);
            
            x = 0;
            y += (rect1.h + 10);
            FONT_Draw(font, screen, x + 10, y + 10, "REPEAT");
            x += 40;
            y += 20;
            GPU_SetWrapMode(image, GPU_WRAP_REPEAT, GPU_WRAP_REPEAT);
            GPU_Blit(image, &rect1, screen, x + rect1.w/2, y + rect1.h/2);
            x += image->w*4;
            GPU_Blit(image, &rect2, screen, x + rect2.w/2, y + rect2.h/2);
            
            x = 0;
            y += (rect1.h + 10);
            FONT_Draw(font, screen, x + 10, y + 10, "MIRRORED");
            x += 40;
            y += 20;
            GPU_SetWrapMode(image, GPU_WRAP_MIRRORED, GPU_WRAP_MIRRORED);
            GPU_Blit(image, &rect1, screen, x + rect1.w/2, y + rect1.h/2);
            x += image->w*4;
            GPU_Blit(image, &rect2, screen, x + rect2.w/2, y + rect2.h/2);
            
            x = 500;
            y = 0;
            FONT_Draw(font, screen, x + 10, y + 10, "REPEAT/MIRRORED");
            x += 40;
            y += 20;
            GPU_SetWrapMode(image, GPU_WRAP_REPEAT, GPU_WRAP_MIRRORED);
            GPU_Blit(image, &rect1, screen, x + rect1.w/2, y + rect1.h/2);
            x += image->w*4;
            GPU_Blit(image, &rect2, screen, x + rect2.w/2, y + rect2.h/2);
            
            x = 500;
            y += (rect1.h + 10);
            FONT_Draw(font, screen, x + 10, y + 10, "NONE/REPEAT");
            x += 40;
            y += 20;
            GPU_SetWrapMode(image, GPU_WRAP_NONE, GPU_WRAP_REPEAT);
            GPU_Blit(image, &rect1, screen, x + rect1.w/2, y + rect1.h/2);
            x += image->w*4;
            GPU_Blit(image, &rect2, screen, x + rect2.w/2, y + rect2.h/2);
            
            x = 500;
            y += (rect1.h + 10);
            FONT_Draw(font, screen, x + 10, y + 10, "NONE/MIRRORED");
            x += 40;
            y += 20;
            GPU_SetWrapMode(image, GPU_WRAP_NONE, GPU_WRAP_MIRRORED);
            GPU_Blit(image, &rect1, screen, x + rect1.w/2, y + rect1.h/2);
            x += image->w*4;
            GPU_Blit(image, &rect2, screen, x + rect2.w/2, y + rect2.h/2);
            
            GPU_Flip(screen);
            
            frameCount++;
            if(frameCount%500 == 0)
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        }
        
        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        
        FONT_Free(font);
        GPU_FreeImage(image);
	}
	
	GPU_Quit();
	
	return 0;
}


