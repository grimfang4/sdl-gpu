#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"

int do_interleaved(GPU_Target* screen)
{
	GPU_LogError("do_interleaved()\n");
	GPU_Image* image = GPU_LoadImage("data/small_test.png");
	if(image == NULL)
		return -1;
	
	int return_value = 0;
	
	float dt = 0.010f;
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	int maxSprites = 50000;
	int numSprites = 101;
	
	int floats_per_sprite = 2 + 4 + 4;
	float* sprite_values = (float*)malloc(sizeof(float)*maxSprites*floats_per_sprite);
	float* velx = (float*)malloc(sizeof(float)*maxSprites);
	float* vely = (float*)malloc(sizeof(float)*maxSprites);
	int i;
    int val_n = 0;
	for(i = 0; i < maxSprites; i++)
	{
		sprite_values[val_n++] = rand()%screen->w;
		sprite_values[val_n++] = rand()%screen->h;
		sprite_values[val_n++] = 0;
		sprite_values[val_n++] = 0;
		sprite_values[val_n++] = image->w;
		sprite_values[val_n++] = image->h;
		sprite_values[val_n++] = rand()%256;
		sprite_values[val_n++] = rand()%256;
		sprite_values[val_n++] = rand()%256;
		sprite_values[val_n++] = rand()%256;
		velx[i] = 10 + rand()%screen->w/10;
		vely[i] = 10 + rand()%screen->h/10;
		if(rand()%2)
            velx[i] = -velx[i];
		if(rand()%2)
            vely[i] = -vely[i];
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
				else if(event.key.keysym.sym == SDLK_SPACE)
                {
					done = 1;
					return_value = 2;
                }
				else if(event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_PLUS)
				{
					if(numSprites < maxSprites)
						numSprites += 100;
                    GPU_LogError("Sprites: %d\n", numSprites);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
				else if(event.key.keysym.sym == SDLK_MINUS)
				{
					if(numSprites > 1)
						numSprites -= 100;
					if(numSprites < 1)
                        numSprites = 1;
                    GPU_LogError("Sprites: %d\n", numSprites);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
			}
		}
		
		for(i = 0; i < numSprites; i++)
		{
		    val_n = floats_per_sprite*i;
			sprite_values[val_n] += velx[i]*dt;
			sprite_values[val_n+1] += vely[i]*dt;
			if(sprite_values[val_n] < 0)
			{
				sprite_values[val_n] = 0;
				velx[i] = -velx[i];
			}
			else if(sprite_values[val_n] > screen->w)
			{
				sprite_values[val_n] = screen->w;
				velx[i] = -velx[i];
			}
			
			if(sprite_values[val_n+1] < 0)
			{
				sprite_values[val_n+1] = 0;
				vely[i] = -vely[i];
			}
			else if(sprite_values[val_n+1] > screen->h)
			{
				sprite_values[val_n+1] = screen->h;
				vely[i] = -vely[i];
			}
		}
		
		GPU_Clear(screen);
		
        GPU_BlitBatch(image, screen, numSprites, sprite_values, 0);
		
		GPU_Flip(screen);
		
		frameCount++;
		if(SDL_GetTicks() - startTime > 5000)
        {
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
			frameCount = 0;
			startTime = SDL_GetTicks();
        }
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	free(sprite_values);
	free(velx);
	free(vely);
	
	GPU_FreeImage(image);
	
	return return_value;
}

int do_separate(GPU_Target* screen)
{
	GPU_LogError("do_separate()\n");
	GPU_Image* image = GPU_LoadImage("data/small_test.png");
	if(image == NULL)
		return -1;
	
	int return_value = 0;
	
	float dt = 0.010f;
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	int maxSprites = 50000;
	int numSprites = 101;
	
	float* positions = (float*)malloc(sizeof(float)*maxSprites*2);
	float* colors = (float*)malloc(sizeof(float)*maxSprites*4);
	float* velx = (float*)malloc(sizeof(float)*maxSprites);
	float* vely = (float*)malloc(sizeof(float)*maxSprites);
	int i;
    int val_n = 0;
	for(i = 0; i < maxSprites; i++)
	{
		positions[val_n++] = rand()%screen->w;
		positions[val_n++] = rand()%screen->h;
		colors[i*4] = rand()%256;
		colors[i*4+1] = rand()%256;
		colors[i*4+2] = rand()%256;
		colors[i*4+3] = rand()%256;
		velx[i] = 10 + rand()%screen->w/10;
		vely[i] = 10 + rand()%screen->h/10;
		if(rand()%2)
            velx[i] = -velx[i];
		if(rand()%2)
            vely[i] = -vely[i];
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
				else if(event.key.keysym.sym == SDLK_SPACE)
                {
					done = 1;
					return_value = 3;
                }
				else if(event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_PLUS)
				{
					if(numSprites < maxSprites)
						numSprites += 100;
                    GPU_LogError("Sprites: %d\n", numSprites);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
				else if(event.key.keysym.sym == SDLK_MINUS)
				{
					if(numSprites > 1)
						numSprites -= 100;
					if(numSprites < 1)
                        numSprites = 1;
                    GPU_LogError("Sprites: %d\n", numSprites);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
			}
		}
		
		for(i = 0; i < numSprites; i++)
		{
		    val_n = 2*i;
			positions[val_n] += velx[i]*dt;
			positions[val_n+1] += vely[i]*dt;
			if(positions[val_n] < 0)
			{
				positions[val_n] = 0;
				velx[i] = -velx[i];
			}
			else if(positions[val_n] > screen->w)
			{
				positions[val_n] = screen->w;
				velx[i] = -velx[i];
			}
			
			if(positions[val_n+1] < 0)
			{
				positions[val_n+1] = 0;
				vely[i] = -vely[i];
			}
			else if(positions[val_n+1] > screen->h)
			{
				positions[val_n+1] = screen->h;
				vely[i] = -vely[i];
			}
		}
		
		GPU_Clear(screen);
		
        GPU_BlitBatchSeparate(image, screen, numSprites, positions, NULL, colors, 0);
		
		GPU_Flip(screen);
		
		frameCount++;
		if(SDL_GetTicks() - startTime > 5000)
        {
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
			frameCount = 0;
			startTime = SDL_GetTicks();
        }
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	free(positions);
	free(colors);
	free(velx);
	free(vely);
	
	GPU_FreeImage(image);
	
	return return_value;
}


int do_attributes(GPU_Target* screen)
{
	GPU_LogError("do_attributes()\n");
	GPU_Image* image = GPU_LoadImage("data/small_test.png");
	if(image == NULL)
		return -1;
	
	int return_value = 0;
	
	float dt = 0.010f;
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	int maxSprites = 50000;
	int numSprites = 101;
	
	// 2 pos floats per vertex, 2 texcoords, 4 color components
	int floats_per_vertex = 2 + 2 + 4;
	int floats_per_sprite = floats_per_vertex*4;
	float* sprite_values = (float*)malloc(sizeof(float)*maxSprites*floats_per_sprite);
	
	// Load attributes for the textured shader
	Uint32 program_object = 0;
	GPU_ActivateShaderProgram(program_object, NULL);
	// Disable the default shader's attributes (not a typical thing to do...)
	GPU_ShaderBlock block = {-1,-1,-1,GPU_GetUniformLocation(program_object, "gpu_ModelViewProjectionMatrix")};
	GPU_ActivateShaderProgram(program_object, &block);
	
	GPU_Attribute attributes[3] = {
	    GPU_MakeAttribute(GPU_GetAttributeLocation(program_object, "gpu_Vertex"), sprite_values, 
                                                    GPU_MakeAttributeFormat(2, GPU_TYPE_FLOAT, 0, floats_per_vertex*sizeof(float), 0)),
        GPU_MakeAttribute(GPU_GetAttributeLocation(program_object, "gpu_TexCoord"), sprite_values, 
                                                    GPU_MakeAttributeFormat(2, GPU_TYPE_FLOAT, 0, floats_per_vertex*sizeof(float), 2*sizeof(float))),
        GPU_MakeAttribute(GPU_GetAttributeLocation(program_object, "gpu_Color"), sprite_values, 
                                                    GPU_MakeAttributeFormat(4, GPU_TYPE_FLOAT, 0, floats_per_vertex*sizeof(float), 4*sizeof(float)))
    };
	
	float* velx = (float*)malloc(sizeof(float)*maxSprites);
	float* vely = (float*)malloc(sizeof(float)*maxSprites);
	int i;
    int val_n = 0;
	for(i = 0; i < maxSprites; i++)
	{
	    float x = rand()%screen->w;
		float y = rand()%screen->h;
		sprite_values[val_n++] = x - image->w/2;
		sprite_values[val_n++] = y - image->h/2;
		
		sprite_values[val_n++] = 0;
		sprite_values[val_n++] = 0;
		
		sprite_values[val_n++] = rand()%101/100.0f;
		sprite_values[val_n++] = rand()%101/100.0f;
		sprite_values[val_n++] = rand()%101/100.0f;
		sprite_values[val_n++] = rand()%101/100.0f;
		
		sprite_values[val_n++] = x + image->w/2;
		sprite_values[val_n++] = y - image->h/2;
		
		sprite_values[val_n++] = 1;
		sprite_values[val_n++] = 0;
		
		sprite_values[val_n++] = rand()%101/100.0f;
		sprite_values[val_n++] = rand()%101/100.0f;
		sprite_values[val_n++] = rand()%101/100.0f;
		sprite_values[val_n++] = rand()%101/100.0f;
		
		sprite_values[val_n++] = x + image->w/2;
		sprite_values[val_n++] = y + image->h/2;
		
		sprite_values[val_n++] = 1;
		sprite_values[val_n++] = 1;
		
		sprite_values[val_n++] = rand()%101/100.0f;
		sprite_values[val_n++] = rand()%101/100.0f;
		sprite_values[val_n++] = rand()%101/100.0f;
		sprite_values[val_n++] = rand()%101/100.0f;
		
		sprite_values[val_n++] = x - image->w/2;
		sprite_values[val_n++] = y + image->h/2;
		
		sprite_values[val_n++] = 0;
		sprite_values[val_n++] = 1;
		
		sprite_values[val_n++] = rand()%101/100.0f;
		sprite_values[val_n++] = rand()%101/100.0f;
		sprite_values[val_n++] = rand()%101/100.0f;
		sprite_values[val_n++] = rand()%101/100.0f;
		
		velx[i] = 10 + rand()%screen->w/10;
		vely[i] = 10 + rand()%screen->h/10;
		if(rand()%2)
            velx[i] = -velx[i];
		if(rand()%2)
            vely[i] = -vely[i];
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
				else if(event.key.keysym.sym == SDLK_SPACE)
                {
					done = 1;
					return_value = 1;
                }
				else if(event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_PLUS)
				{
					if(numSprites < maxSprites)
						numSprites += 100;
                    GPU_LogError("Sprites: %d\n", numSprites);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
				else if(event.key.keysym.sym == SDLK_MINUS)
				{
					if(numSprites > 1)
						numSprites -= 100;
					if(numSprites < 1)
                        numSprites = 1;
                    GPU_LogError("Sprites: %d\n", numSprites);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
			}
		}
		
		GPU_Clear(screen);
		
		for(i = 0; i < numSprites; i++)
		{
		    val_n = floats_per_sprite*i;
		    float x = sprite_values[val_n] + image->w/2;
		    float y = sprite_values[val_n+1] + image->h/2;
		    
			x += velx[i]*dt;
			y += vely[i]*dt;
			if(x < 0)
			{
				x = 0;
				velx[i] = -velx[i];
			}
			else if(x > screen->w)
			{
				x = screen->w;
				velx[i] = -velx[i];
			}
			
			if(y < 0)
			{
				y = 0;
				vely[i] = -vely[i];
			}
			else if(y > screen->h)
			{
				y = screen->h;
				vely[i] = -vely[i];
			}
			
            sprite_values[val_n] = x - image->w/2;
            sprite_values[val_n+1] = y - image->h/2;
            val_n += floats_per_vertex;
            sprite_values[val_n] = x + image->w/2;
            sprite_values[val_n+1] = y - image->h/2;
            val_n += floats_per_vertex;
            sprite_values[val_n] = x + image->w/2;
            sprite_values[val_n+1] = y + image->h/2;
            val_n += floats_per_vertex;
            sprite_values[val_n] = x - image->w/2;
            sprite_values[val_n+1] = y + image->h/2;
		}
		
		//float color[4] = {0.5f, 0, 0, 1.0f};
		//GPU_SetAttributefv(attributes[2].location, 4, color);
		GPU_SetAttributeSource(numSprites*4, attributes[0]);
		GPU_SetAttributeSource(numSprites*4, attributes[1]);
		GPU_SetAttributeSource(numSprites*4, attributes[2]);
        GPU_BlitBatch(image, screen, numSprites, NULL, 0);
		
		GPU_Flip(screen);
		
		frameCount++;
		if(SDL_GetTicks() - startTime > 5000)
        {
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
			frameCount = 0;
			startTime = SDL_GetTicks();
        }
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	free(sprite_values);
	free(velx);
	free(vely);
	
	GPU_FreeImage(image);
	
	// Reset the default shader's block
	GPU_ActivateShaderProgram(screen->context->default_textured_shader_program, NULL);
	
	return return_value;
}

int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	int i = 1;
	while(i > 0)
    {
        if(i == 1)
            i = do_interleaved(screen);
        else if(i == 2)
            i = do_separate(screen);
        else if(i == 3)
            i = do_attributes(screen);
        else
            i = 0;
    }
	
	GPU_Quit();
	
	return i;
}


