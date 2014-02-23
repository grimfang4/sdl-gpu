#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "compat.h"
#include "common.h"

#ifndef M_PI
  #define M_PI 3.14159
#endif


// Post-multiply
void scale(float* matrix3x3, float scale_x, float scale_y)
{
	matrix3x3[0] *= scale_x;
	matrix3x3[1] *= scale_x;
	matrix3x3[2] *= scale_x;
	matrix3x3[3] *= scale_y;
	matrix3x3[4] *= scale_y;
	matrix3x3[5] *= scale_y;
}

void translate(float* matrix3x3, float dx, float dy)
{
	matrix3x3[6] += matrix3x3[0]*dx + matrix3x3[3]*dy;
	matrix3x3[7] += matrix3x3[1]*dx + matrix3x3[4]*dy;
	matrix3x3[8] += matrix3x3[2]*dx + matrix3x3[5]*dy;
}

void rotate(float* matrix3x3, float radians)
{
	float cosT = cos(radians);
	float sinT = sin(radians);
	float a = matrix3x3[0];
	float b = matrix3x3[3];
	float d = matrix3x3[1];
	float e = matrix3x3[4];
	float g = matrix3x3[2];
	float h = matrix3x3[5];
	matrix3x3[0] = a*cosT+b*sinT;
	matrix3x3[1] = d*cosT+e*sinT;
	matrix3x3[2] = g*cosT+h*sinT;
	matrix3x3[3] = -a*sinT+b*cosT;
	matrix3x3[4] = -d*sinT+e*cosT;
	matrix3x3[5] = -g*sinT+h*cosT;
}

int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	GPU_Image* image = GPU_LoadImage("data/test.bmp");
	if(image == NULL)
		return -1;
	
	GPU_Image* screen2_image = GPU_CreateImage(800, 600, GPU_FORMAT_RGBA);
	GPU_Target* screen2 = GPU_LoadTarget(screen2_image);
	
	float angle = 0.0f;
	float dx = 0.0f;
	float dy = 0.0f;
	float scale_x = 1.0f;
	float scale_y = 1.0f;
	
	float dt = 0.010f;
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	GPU_Target* target = screen;
	float x = 400.0f;
	float y = 300.0f;
	SDL_Color red = {255, 0, 0, 255};
	Uint8* keystates = SDL_GetKeyState(NULL);
	
	Uint8 done = 0;
	SDL_Event event;
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
			}
		}
		
		if(keystates[KEY_UP])
			dy -= 100*dt;
		else if(keystates[KEY_DOWN])
			dy += 100*dt;
		if(keystates[KEY_LEFT])
			dx -= 100*dt;
		else if(keystates[KEY_RIGHT])
			dx += 100*dt;
		if(keystates[KEY_COMMA])
			angle -= 100*dt;
		else if(keystates[KEY_PERIOD])
			angle += 100*dt;
		if(keystates[KEY_a])
			scale_x -= 1*dt;
		else if(keystates[KEY_s])
			scale_x += 1*dt;
		if(keystates[KEY_z])
			scale_y -= 1*dt;
		else if(keystates[KEY_x])
			scale_y += 1*dt;
		
		GPU_Clear(screen);
		GPU_Clear(screen2);
		
		GPU_BlitScale(image, NULL, target, x, y, 0.1f, 0.1f);
		
		
		float matrix[9] = {1,0,0,0,1,0,0,0,1};
		translate(matrix, dx, dy);
		scale(matrix, scale_x, scale_y);
		rotate(matrix, angle*M_PI/180);
		GPU_BlitTransformMatrix(image, NULL, target, x, y, matrix);
		
		
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
	GPU_Quit();
	
	return 0;
}


