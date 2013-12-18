#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"


void load_shaders(Uint32* v, Uint32* f, Uint32* p)
{
    *v = GPU_LoadShader(GPU_VERTEX_SHADER, "shader/test.vert");
    
    if(!*v)
        GPU_LogError("Failed to load vertex shader: %s\n", GPU_GetShaderMessage());
    
    *f = GPU_LoadShader(GPU_FRAGMENT_SHADER, "shader/test.frag");
    
    if(!*f)
        GPU_LogError("Failed to load fragment shader: %s\n", GPU_GetShaderMessage());
    
    *p = GPU_LinkShaders(*v, *f);
    
    if(!*p)
    {
        GPU_LogError("Failed to link shader program: %s\n", GPU_GetShaderMessage());
        return;
    }
    
    GPU_ActivateShaderProgram(*p);
}

void free_shaders(Uint32 v, Uint32 f, Uint32 p)
{
    GPU_FreeShader(v);
    GPU_FreeShader(f);
    GPU_FreeShaderProgram(p);
}

int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(800, 600, 0);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	GPU_Image* image = GPU_LoadImage("data/test.bmp");
	if(image == NULL)
		return -1;
	
	Uint32 v, f, p;
	load_shaders(&v, &f, &p);
	int uloc = GPU_GetUniformLocation(p, "tex");
	GPU_SetUniformi(uloc, 0);
	int timeloc = GPU_GetUniformLocation(p, "time");
	
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
				else if(event.key.keysym.sym == SDLK_SPACE)
				{
				    if(screen->current_shader_program == screen->default_textured_shader_program
				       || screen->current_shader_program == screen->default_untextured_shader_program)
                        GPU_ActivateShaderProgram(p);
                    else
                        GPU_ActivateShaderProgram(0);
                    
                    
                    uloc = GPU_GetUniformLocation(p, "tex");
                    GPU_SetUniformi(uloc, 0);
                    timeloc = GPU_GetUniformLocation(p, "time");
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
		
        GPU_SetUniformf(timeloc, SDL_GetTicks()/1000.0f);
        
		GPU_Clear(screen);
		
		for(i = 0; i < numSprites; i++)
		{
			GPU_Blit(image, NULL, screen, x[i], y[i]);
		}
		
		GPU_Flip(screen);
		
		frameCount++;
		if(frameCount%500 == 0)
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	GPU_FreeImage(image);
	
	free_shaders(v, f, p);
	
	GPU_Quit();
	
	return 0;
}


