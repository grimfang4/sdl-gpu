#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"


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
        
        GPU_Image* screen2_image = GPU_CreateImage(800, 600, GPU_FORMAT_RGBA);
        GPU_Target* screen2 = GPU_LoadTarget(screen2_image);
        
        GPU_Target* target = screen;
        float x = 400.0f;
        float y = 300.0f;
        
		Uint8 usingVirtual = 0;
		SDL_Color red = { 255, 0, 0, 255 };
        
        GPU_Image* image = GPU_LoadImage("data/test.bmp");
        if(image == NULL)
            return -1;
        
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
                    else if(event.key.keysym.sym == SDLK_SPACE)
                    {
                        if(target == screen)
                            target = screen2;
                        else
                            target = screen;
                    }
                    else if(event.key.keysym.sym == SDLK_RETURN)
                    {
                        usingVirtual = !usingVirtual;
                        
                        if(usingVirtual)
                        {
                            GPU_SetVirtualResolution(screen, 640, 480);
                            GPU_SetVirtualResolution(screen2, 640, 480);
                        }
                        else
                        {
                            GPU_SetVirtualResolution(screen, 800, 600);
                            GPU_SetVirtualResolution(screen2, 800, 600);
                        }
                        
                    }
                }
            }
            
            GPU_Clear(screen);
            GPU_Clear(screen2);
            
            GPU_BlitScale(image, NULL, target, x, y, 0.1f, 0.1f);
            
            
            {
				SDL_Rect area = {target->w/2 - 150, target->h/2 - 150, 300, 300};
                float scale_x = 0.25f;
                float scale_y = 0.5f;
                GPU_Rect r = {0.2*image->w, 0.4*image->w, 0.3*image->w, 0.6*image->h};

				GPU_Rectangle(target, area.x, area.y, area.x + area.w, area.y + area.h, red);

                GPU_BlitTransformX(image, NULL, target, area.x, area.y, 0, 0, SDL_GetTicks()/10.0f, scale_x, scale_y);
                GPU_BlitTransformX(image, NULL, target, area.x + area.w, area.y, image->w, 0, SDL_GetTicks()/10.0f, scale_x, scale_y);
                
                GPU_BlitTransformX(image, &r, target, area.x, area.y + area.h, 0, r.h, SDL_GetTicks()/10.0f, scale_x, scale_y);
                
                GPU_BlitTransform(image, &r, target, area.x - 150, area.y + area.h, SDL_GetTicks()/10.0f, scale_x, scale_y);
                GPU_CircleFilled(target, area.x - 150, area.y + area.h, 10, red);
                
                r = GPU_MakeRect(-0.2*image->w, 0, image->w, image->h);
                GPU_BlitTransformX(image, &r, target, area.x + area.w, area.y + area.h, r.w, r.h, SDL_GetTicks()/10.0f, scale_x, scale_y);
                
                GPU_BlitTransform(image, &r, target, area.x + area.w + 150, area.y + area.h, SDL_GetTicks()/10.0f, scale_x, scale_y);
                GPU_CircleFilled(target, area.x + area.w + 150, area.y + area.h, 10, red);
            }
            
            if(target == screen2)
                GPU_CircleFilled(screen2, 0, 0, 100, red);
            
            GPU_Blit(screen2_image, NULL, screen, 400, 300);
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


