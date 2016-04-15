#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"
#include "math.h"


int main(int argc, char* argv[])
{
	GPU_Target* screen;
	
	screen = initialize_demo(argc, argv, 800, 600);
	if(screen == NULL)
		return 1;
	
	{
		Uint32 startTime;
		long frameCount;
		Uint8 done;
		SDL_Event event;
        float dt = 0.010f;
		
		const Uint8* keystates = SDL_GetKeyboardState(NULL);
		
		Uint8 mode = 0;
		
		float anchor_x = 0.5f;
		float anchor_y = 0.5f;
		
		float angle = 0.0f;
		float scale = 1.0f;
		
        GPU_Image* image = GPU_LoadImage("data/test.bmp");
        if(image == NULL)
            return 2;
        
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
                    if(event.key.keysym.sym == SDLK_1)
                    {
                        anchor_x = 0.5f;
                        anchor_y = 0.5f;
                    }
                    if(event.key.keysym.sym == SDLK_2)
                    {
                        anchor_x = 0.0f;
                        anchor_y = 0.0f;
                    }
                    if(event.key.keysym.sym == SDLK_3)
                    {
                        anchor_x = 1.0f;
                        anchor_y = 1.0f;
                    }
                    if(event.key.keysym.sym == SDLK_4)
                    {
                        anchor_x = 1.0f;
                        anchor_y = 0.5f;
                    }
                    if(event.key.keysym.sym == SDLK_SPACE)
						mode = !mode;
                    if(event.key.keysym.sym == SDLK_BACKSPACE)
						GPU_SetCoordinateMode(!GPU_GetCoordinateMode());
                }
            }
            
            
            if(keystates[SDL_SCANCODE_UP])
                anchor_y -= (GPU_GetCoordinateMode()? -dt : dt);
            if(keystates[SDL_SCANCODE_DOWN])
                anchor_y += (GPU_GetCoordinateMode()? -dt : dt);
            if(keystates[SDL_SCANCODE_LEFT])
                anchor_x -= dt;
            if(keystates[SDL_SCANCODE_RIGHT])
                anchor_x += dt;
            
            GPU_SetAnchor(image, anchor_x, anchor_y);
            
            GPU_Clear(screen);
            
            if(!mode)
            {
                GPU_Blit(image, NULL, screen, 0, 0);
                GPU_Blit(image, NULL, screen, screen->w, 0);
                GPU_Blit(image, NULL, screen, 0, screen->h);
                GPU_Blit(image, NULL, screen, screen->w, screen->h);
                GPU_Blit(image, NULL, screen, screen->w/2, screen->h/2);
            }
            else
            {
                angle = fmodf(SDL_GetTicks()/100.0f, 360.0f);
                scale = 1 + 0.5f*sin(SDL_GetTicks()/1000.0f);
                
                GPU_BlitTransform(image, NULL, screen, 0, 0, angle, scale, scale);
                GPU_BlitTransform(image, NULL, screen, screen->w, 0, angle, scale, scale);
                GPU_BlitTransform(image, NULL, screen, 0, screen->h, angle, scale, scale);
                GPU_BlitTransform(image, NULL, screen, screen->w, screen->h, angle, scale, scale);
                GPU_BlitTransform(image, NULL, screen, screen->w/2, screen->h/2, angle, scale, scale);
            }
            
            GPU_Rectangle(screen, screen->w/2 - image->w/2, screen->h/2 - image->h/2, screen->w/2 + image->w/2, screen->h/2 + image->h/2, GPU_MakeColor(0, 255, 255, 255));
            
            GPU_CircleFilled(screen, screen->w/2 - image->w/2 + anchor_x * image->w, screen->h/2 - image->h/2 + anchor_y * image->h, 5, GPU_MakeColor(255, 0, 0, 255));
            
            GPU_Circle(screen, 0, 0, 5, GPU_MakeColor(255, 128, 0, 255));
            GPU_Circle(screen, screen->w, 0, 5, GPU_MakeColor(255, 128, 0, 255));
            GPU_Circle(screen, 0, screen->h, 5, GPU_MakeColor(255, 128, 0, 255));
            GPU_Circle(screen, screen->w, screen->h, 5, GPU_MakeColor(255, 128, 0, 255));
            GPU_Circle(screen, screen->w/2, screen->h/2, 5, GPU_MakeColor(255, 128, 0, 255));
            
            
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


