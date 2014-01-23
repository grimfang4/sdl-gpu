#include "SDL.h"
#include "SDL_gpu.h"
#include "SDL_gpu_OpenGL_1.h"
#include "common.h"


void begin_3d(GPU_Target* screen)
{
    GPU_FlushBlitBuffer();
    
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
}

void end_3d(GPU_Target* screen)
{
    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    
    //GPU_SetCamera(screen, NULL);
}

void draw_spinning_triangle()
{
    float t = SDL_GetTicks()/1000.0f;
    glRotatef(100*t, 0, 0.707, 0.707);
    glRotatef(20*t, 0.707, 0.707, 0);

    GLfloat glverts[9];
    GLfloat glcolors[9];

    glverts[0] = 0;
    glverts[1] = 0.2f;
    glverts[2] = 0;
    glcolors[0] = 1.0f;
    glcolors[1] = 0.0f;
    glcolors[2] = 0.0f;
    
    glverts[3] = -0.2f;
    glverts[4] = -0.2f;
    glverts[5] = 0;
    glcolors[3] = 0.0f;
    glcolors[4] = 1.0f;
    glcolors[5] = 0.0f;
    
    glverts[6] = 0.2f;
    glverts[7] = -0.2f;
    glverts[8] = 0;
    glcolors[6] = 0.0f;
    glcolors[7] = 0.0f;
    glcolors[8] = 1.0f;
    
    glVertexPointer(3, GL_FLOAT, 0, glverts);
    glColorPointer(3, GL_FLOAT, 0, glcolors);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void draw_3d_stuff(GPU_Target* screen)
{
    begin_3d(screen);
    
    draw_spinning_triangle();
    
    end_3d(screen);
}

void draw_more_3d_stuff(GPU_Target* screen)
{
    begin_3d(screen);
    
    float t = SDL_GetTicks()/1000.0f;
    glRotatef(t*60, 0, 0, 1);
    glTranslatef(0.4f, 0.4f, 0);
    draw_spinning_triangle();
    
    end_3d(screen);
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
	
	
	float dt = 0.010f;
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	int maxSprites = 50;
	int numSprites = 1;
	
	float x[maxSprites];
	float y[maxSprites];
	float velx[maxSprites];
	float vely[maxSprites];
	int i;
	for(i = 0; i < maxSprites; i++)
	{
		x[i] = rand()%screen->w;
		y[i] = rand()%screen->h;
		velx[i] = 10 + rand()%screen->w/10;
		vely[i] = 10 + rand()%screen->h/10;
	}
	
	
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
				else if(event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_PLUS)
				{
					if(numSprites < maxSprites)
						numSprites++;
				}
				else if(event.key.keysym.sym == SDLK_MINUS)
				{
					if(numSprites > 0)
						numSprites--;
				}
			}
		}
		
		for(i = 0; i < numSprites; i++)
		{
			x[i] += velx[i]*dt;
			y[i] += vely[i]*dt;
			if(x[i] < 0)
			{
				x[i] = 0;
				velx[i] = -velx[i];
			}
			else if(x[i]> screen->w)
			{
				x[i] = screen->w;
				velx[i] = -velx[i];
			}
			
			if(y[i] < 0)
			{
				y[i] = 0;
				vely[i] = -vely[i];
			}
			else if(y[i]> screen->h)
			{
				y[i] = screen->h;
				vely[i] = -vely[i];
			}
		}
		
		GPU_Clear(screen);
		
		draw_3d_stuff(screen);
		
		for(i = 0; i < numSprites; i++)
		{
			GPU_Blit(image, NULL, screen, x[i], y[i]);
		}
		
		draw_more_3d_stuff(screen);
		
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


