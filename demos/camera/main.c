#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>

void printRenderers(void)
{
	const char* renderers[GPU_GetNumRegisteredRenderers()];
	GPU_GetRegisteredRendererList(renderers);
	
	printf("Available renderers:\n");
	int i;
	for(i = 0; i < GPU_GetNumRegisteredRenderers(); i++)
	{
		printf("%d) %s\n", i+1, renderers[i]);
	}
}

#include "../SDL_gpu/OpenGL/glew.h"
#include "../SDL_gpu/OpenGL/SDL_gpu_OpenGL.h"



typedef struct Camera
{
	float x, y, z;
	float angle;
	float zoom;
} Camera;

// Camera zoom needs to have a clear specification
// I think it would make the most sense for zoom to be 1.0 at no zoom, 2.0 at 2x zoom, etc.
// If it represented a z-coordinate, it must be easy to calculate screen coordinates from it.  0.0 would be no zoom, camera lens angle would have to be determined somehow...

Camera GPU_SetCamera(GPU_Target* screen, Camera cam)
{
	static Camera oldCam = {0.0f, 0.0f, -10.0f, 0.0f, 1.0f};  // Move this into the renderer data so GPU_GetCamera() can use it.
	
	if(screen == NULL)
		return oldCam;
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	// I want to use x, y, and z
	// The default z for objects is 0
	// The default z for the camera should not be 0. (should be neg)
	
	/*float fieldOfView = 60.0f;
	float fW = screen->w/2;
	float fH = screen->h/2;
	float aspect = fW/fH;
	float zNear = atan(fH)/((float)(fieldOfView / 360.0f * 3.14159f));
	float zFar = 255.0f;
	glFrustum( 0.0f + cam.x, 2*fW + cam.x, 2*fH + cam.y, 0.0f + cam.y, zNear, zFar );*/
	
	glFrustum(0.0f + cam.x, screen->w + cam.x, screen->h + cam.y, 0.0f + cam.y, 0.01f, 1.01f);
	
	//glMatrixMode( GL_MODELVIEW );
	//glLoadIdentity();
	
	
	float offsetX = screen->w/2.0f;
	float offsetY = screen->h/2.0f;
	glTranslatef(offsetX, offsetY, -0.01);
	glRotatef(cam.angle, 0, 0, 1);
	glTranslatef(-offsetX, -offsetY, 0);
	
	glTranslatef(cam.x + offsetX, cam.y + offsetY, 0);
	glScalef(cam.zoom, cam.zoom, 1.0f);
	glTranslatef(-cam.x - offsetX, -cam.y - offsetY, 0);
	
	Camera result = oldCam;
	oldCam = cam;
	return result;
}

int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(NULL, 800, 600, 0);
	if(screen == NULL)
		return -1;
	
	printf("Using renderer: %s\n", GPU_GetCurrentRendererID());
	
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	
	GPU_Image* img = GPU_LoadImage("data/test3.png");
	
	Uint8* keystates = SDL_GetKeyState(NULL);
	
	Camera camera;
	camera.x = 0;
	camera.y = 0;
	camera.z = -10;
	camera.zoom = 1;
	camera.angle = 0.0f;
	
	float dt = 0.010f;
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
				else if(event.key.keysym.sym == SDLK_r)
				{
					camera.x = 0.0f;
					camera.y = 0.0f;
					camera.z = -10.0f;
					camera.zoom = 1.0f;
					camera.angle = 0.0f;
				}
			}
		}
		
		if(keystates[SDLK_UP])
		{
			camera.y -= 200*dt;
		}
		else if(keystates[SDLK_DOWN])
		{
			camera.y += 200*dt;
		}
		if(keystates[SDLK_LEFT])
		{
			camera.x -= 200*dt;
		}
		else if(keystates[SDLK_RIGHT])
		{
			camera.x += 200*dt;
		}
		if(keystates[SDLK_z])
		{
			camera.z -= 200*dt;
		}
		else if(keystates[SDLK_x])
		{
			camera.z += 200*dt;
		}
		if(keystates[SDLK_MINUS])
		{
			camera.zoom -= 1.0f*dt;
		}
		else if(keystates[SDLK_EQUALS])
		{
			camera.zoom += 1.0f*dt;
		}
		if(keystates[SDLK_COMMA])
		{
			camera.angle -= 100*dt;
		}
		else if(keystates[SDLK_PERIOD])
		{
			camera.angle += 100*dt;
		}
		
		GPU_ClearRGBA(screen, 255, 255, 255, 255);
		
		GPU_SetCamera(screen, camera);
		
		GPU_Blit(img, NULL, screen, 50, 50);
		GPU_Blit(img, NULL, screen, 320, 50);
		GPU_Blit(img, NULL, screen, 50, 500);
		
		
		GPU_Flip();
		
		frameCount++;
		if(frameCount%500 == 0)
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	GPU_FreeImage(img);
	
	GPU_Quit();
	
	return 0;
}


