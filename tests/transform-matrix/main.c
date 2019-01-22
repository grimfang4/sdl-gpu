#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"
#include <math.h>

// Column-major indexing
#define INDEX(row,col) ((col)*4 + (row))


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
		
		GPU_Camera camera = GPU_GetDefaultCamera();
        
        GPU_Image* image = GPU_LoadImage("data/test.bmp");
        if(image == NULL)
            return -1;

		float x = screen->w/2;
		float y = screen->h/2;
		float z = 0;

		Uint8 use_camera = 0;
		Uint8 use_perspective = 1;
		Uint8 rotate_stuff = 0;
        
		GPU_EnableCamera(screen, use_camera);
        
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
                    if(event.key.keysym.sym == SDLK_RETURN)
                        rotate_stuff = !rotate_stuff;
                    if(event.key.keysym.sym == SDLK_LEFT)
                        x -= 30;
                    if(event.key.keysym.sym == SDLK_RIGHT)
                        x += 30;
                    if(event.key.keysym.sym == SDLK_UP)
                        y -= 30;
                    if(event.key.keysym.sym == SDLK_DOWN)
                        y += 30;
                    if(event.key.keysym.sym == SDLK_a)
						camera.angle -= 30;
					if (event.key.keysym.sym == SDLK_d)
						camera.angle += 30;
					if (event.key.keysym.sym == SDLK_x)
					{
						z -= 0.5f;
						GPU_LogError("z: %.1f\n", z);
					}
					if (event.key.keysym.sym == SDLK_z)
					{
						z += 0.5f;
						GPU_LogError("z: %.1f\n", z);
					}
					if (event.key.keysym.sym == SDLK_p)
					{
						use_perspective = 1;
						use_camera = 0;
						GPU_EnableCamera(screen, use_camera);
					}
					if (event.key.keysym.sym == SDLK_o)
					{
						use_perspective = 0;
						use_camera = 0;
						GPU_EnableCamera(screen, use_camera);
					}
					if (event.key.keysym.sym == SDLK_c)
					{
						use_camera = 1;
						GPU_EnableCamera(screen, use_camera);
						GPU_ResetProjection(screen);
					}
					
                }
            }
            
            GPU_Clear(screen);
            
            GPU_MatrixMode(screen, GPU_PROJECTION);

			if (!use_camera)
			{
                GPU_LoadIdentity();
                // Apply projection matrix

				if (use_perspective)
				{
					GPU_Perspective(90, screen->w / (float)screen->h, 0.1, 2000);
				}
				else
                {
					GPU_Ortho(0, 800, 600, 0, -1000, 1000);
                }
			}
            
            
            GPU_MatrixMode(screen, GPU_VIEW);
            GPU_LoadIdentity();
            
			if (!use_camera)
			{
                // Apply view matrix
                
                if(use_perspective)
                {
                    GPU_LookAt(screen->w/2, screen->h/2, 300,  // eye
                                             screen->w/2, screen->h/2, 0,  // target
                                             0, 1, 0);  // up
                }
                else
                {
                    GPU_LookAt(0, 0, 0.1,  // eye
                                             0, 0, 0,  // target
                                             0, 1, 0);  // up
                }
			}
            
            
			
            // Apply model matrix
            GPU_MatrixMode(screen, GPU_MODEL);
            GPU_LoadIdentity();
			
			GPU_Translate(x, y, z);

            // Rotate
            if(rotate_stuff)
            {
                float a = SDL_GetTicks() / 10.0f;
                GPU_Rotate(a, 0.57, 0.57, 0.57);
            }

            GPU_SetCamera(screen, &camera);
            
            GPU_Blit(image, NULL, screen, 0, 0);
            
            GPU_SetLineThickness(10.0f);
            GPU_Rectangle(screen, -image->w/2, -image->w/2, image->w/2, image->w/2, GPU_MakeColor(0, 0, 255, 255));
            GPU_Circle(screen, 0, 0, image->w/2, GPU_MakeColor(255, 0, 0, 255));
            
            GPU_RectangleFilled(screen, -image->w/8, -image->w/8, image->w/8, image->w/8, GPU_MakeColor(0, 255, 0, 255));
            GPU_SetLineThickness(1.0f);
            
            GPU_BlitScale(image, NULL, screen, 200, 200, 0.5f, 0.5f);
            
            float scale = 200;
            GPU_MatrixMode(screen, GPU_MODEL);
            GPU_PushMatrix();
            GPU_SetLineThickness(4.0f);
            GPU_Translate(40, 40, 0.0f);
            GPU_Rotate(90, 0.0f, 0.0f, 1.0f);
            //GPU_Translate(-screen->w/2, -screen->h/2, 0.0f);
            GPU_Translate(-40, -40, 0.0f);
            
            
            GPU_Line(screen, 0, 0, scale, 0, GPU_MakeColor(255, 0, 0, 255));
            GPU_CircleFilled(screen, 0, 0, scale/16, GPU_MakeColor(255, 0, 0, 255));
            GPU_Circle(screen, 0, 0, scale, GPU_MakeColor(255, 0, 0, 255));
            GPU_Circle(screen, 0, 0, scale*4, GPU_MakeColor(0, 255, 0, 255));
            GPU_SetLineThickness(1.0f);
            
            GPU_PopMatrix();
            
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


