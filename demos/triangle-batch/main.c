#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"

int do_interleaved(GPU_Target* screen)
{
	GPU_LogError("do_interleaved()\n");
	GPU_Image* image = GPU_LoadImage("data/test3.png");
	if(image == NULL)
		return -1;
	
	int return_value = 0;
	
	float dt = 0.010f;
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	int max_vertices = 60000;
	int num_vertices = 303;
	
	int floats_per_vertex = 2 + 2 + 4;
	float* vertex_values = (float*)malloc(sizeof(float)*max_vertices*floats_per_vertex);
	float* velx = (float*)malloc(sizeof(float)*max_vertices/3);
	float* vely = (float*)malloc(sizeof(float)*max_vertices/3);
	int i;
    int val_n = 0;
	for(i = 0; i < max_vertices; i+=3)
	{
		float offset_x1 = rand()%(image->w/2);
		float offset_y1 = rand()%(image->h/2);
		float x1 = vertex_values[val_n++] = rand()%screen->w + offset_x1;
		float y1 = vertex_values[val_n++] = rand()%screen->h + offset_y1;
		vertex_values[val_n++] = offset_x1;
		vertex_values[val_n++] = offset_y1;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		
		float offset_x2 = 5 + rand()%(image->w/2);
		float offset_y2 = 5 + rand()%(image->h/2);
		vertex_values[val_n++] = x1 + offset_x2;
		vertex_values[val_n++] = y1 + offset_y2;
		vertex_values[val_n++] = offset_x1 + offset_x2;
		vertex_values[val_n++] = offset_y1 + offset_y2;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		
		float offset_x3 = -5 - rand()%(image->w/2);
		float offset_y3 = 5 + rand()%(image->h/2);
		vertex_values[val_n++] = x1 + offset_x3;
		vertex_values[val_n++] = y1 + offset_y3;
		vertex_values[val_n++] = offset_x1 + offset_x3;
		vertex_values[val_n++] = offset_y1 + offset_y3;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		if(i%3 == 0)  // Once per triangle
        {
            int n = i/3;
            velx[n] = 10 + rand()%screen->w/10;
            vely[n] = 10 + rand()%screen->h/10;
            if(rand()%2)
                velx[n] = -velx[n];
            if(rand()%2)
                vely[n] = -vely[n];
        }
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
				    num_vertices += 300;
					if(num_vertices > max_vertices)
						num_vertices = max_vertices;
                    GPU_LogError("Vertices: %d\n", num_vertices);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
				else if(event.key.keysym.sym == SDLK_MINUS)
				{
					if(num_vertices > 3)
						num_vertices -= 300;
					if(num_vertices < 3)
                        num_vertices = 3;
                    GPU_LogError("Vertices: %d\n", num_vertices);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
			}
		}
		
		// FIXME: Can cause squishing when a vertex hits a wall...
		for(i = 0; i < num_vertices; i++)
		{
		    int n = i/3;
		    val_n = floats_per_vertex*i;
			vertex_values[val_n] += velx[n]*dt;
			vertex_values[val_n+1] += vely[n]*dt;
			if(vertex_values[val_n] < 0)
			{
				vertex_values[val_n] = 0;
				velx[n] = -velx[n];
			}
			else if(vertex_values[val_n] > screen->w)
			{
				vertex_values[val_n] = screen->w;
				velx[n] = -velx[n];
			}
			
			if(vertex_values[val_n+1] < 0)
			{
				vertex_values[val_n+1] = 0;
				vely[n] = -vely[n];
			}
			else if(vertex_values[val_n+1] > screen->h)
			{
				vertex_values[val_n+1] = screen->h;
				vely[n] = -vely[n];
			}
		}
		
		GPU_Clear(screen);
		
        GPU_TriangleBatch(image, screen, num_vertices, vertex_values, 0, NULL, 0);
		
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
	
	free(vertex_values);
	free(velx);
	free(vely);
	
	GPU_FreeImage(image);
	
	return return_value;
}

int do_indexed(GPU_Target* screen)
{
	GPU_LogError("do_indexed()\n");
	GPU_Image* image = GPU_LoadImage("data/test3.png");
	if(image == NULL)
		return -1;
	
	int return_value = 0;
	
	float dt = 0.010f;
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	int max_vertices = 60000;
	int num_vertices = 303;
	
	int floats_per_vertex = 2 + 2 + 4;
	float* vertex_values = (float*)malloc(sizeof(float)*max_vertices*floats_per_vertex);
	float* velx = (float*)malloc(sizeof(float)*max_vertices/3);
	float* vely = (float*)malloc(sizeof(float)*max_vertices/3);
	unsigned short* indices = (unsigned short*)malloc(sizeof(unsigned short)*max_vertices);
	int i;
    int val_n = 0;
	for(i = 0; i < max_vertices; i+=3)
	{
		float offset_x1 = rand()%(image->w/2);
		float offset_y1 = rand()%(image->h/2);
		float x1 = vertex_values[val_n++] = rand()%screen->w + offset_x1;
		float y1 = vertex_values[val_n++] = rand()%screen->h + offset_y1;
		vertex_values[val_n++] = offset_x1;
		vertex_values[val_n++] = offset_y1;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		indices[i] = i;
		
		float offset_x2 = 5 + rand()%(image->w/2);
		float offset_y2 = 5 + rand()%(image->h/2);
		vertex_values[val_n++] = x1 + offset_x2;
		vertex_values[val_n++] = y1 + offset_y2;
		vertex_values[val_n++] = offset_x1 + offset_x2;
		vertex_values[val_n++] = offset_y1 + offset_y2;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		indices[i+1] = i+1;
		
		float offset_x3 = -5 - rand()%(image->w/2);
		float offset_y3 = 5 + rand()%(image->h/2);
		vertex_values[val_n++] = x1 + offset_x3;
		vertex_values[val_n++] = y1 + offset_y3;
		vertex_values[val_n++] = offset_x1 + offset_x3;
		vertex_values[val_n++] = offset_y1 + offset_y3;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		vertex_values[val_n++] = rand()%256;
		indices[i+2] = i+2;
		
		if(i%3 == 0)  // Once per triangle
        {
            int n = i/3;
            velx[n] = 10 + rand()%screen->w/10;
            vely[n] = 10 + rand()%screen->h/10;
            if(rand()%2)
                velx[n] = -velx[n];
            if(rand()%2)
                vely[n] = -vely[n];
        }
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
				    num_vertices += 300;
					if(num_vertices > max_vertices)
						num_vertices = max_vertices;
                    GPU_LogError("Vertices: %d\n", num_vertices);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
				else if(event.key.keysym.sym == SDLK_MINUS)
				{
					if(num_vertices > 3)
						num_vertices -= 300;
					if(num_vertices < 3)
                        num_vertices = 3;
                    GPU_LogError("Vertices: %d\n", num_vertices);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
			}
		}
		
		// FIXME: Can cause squishing when a vertex hits a wall...
		for(i = 0; i < num_vertices; i++)
		{
		    int n = i/3;
		    val_n = floats_per_vertex*i;
			vertex_values[val_n] += velx[n]*dt;
			vertex_values[val_n+1] += vely[n]*dt;
			if(vertex_values[val_n] < 0)
			{
				vertex_values[val_n] = 0;
				velx[n] = -velx[n];
			}
			else if(vertex_values[val_n] > screen->w)
			{
				vertex_values[val_n] = screen->w;
				velx[n] = -velx[n];
			}
			
			if(vertex_values[val_n+1] < 0)
			{
				vertex_values[val_n+1] = 0;
				vely[n] = -vely[n];
			}
			else if(vertex_values[val_n+1] > screen->h)
			{
				vertex_values[val_n+1] = screen->h;
				vely[n] = -vely[n];
			}
		}
		
		GPU_Clear(screen);
		
        GPU_TriangleBatch(image, screen, num_vertices, vertex_values, num_vertices, indices, 0);
		
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
	
	free(vertex_values);
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
	
	int max_vertices = 60000;
	int num_vertices = 303;
	
	// 2 pos floats per vertex, 2 texcoords, 4 color components
	int floats_per_vertex = 2 + 2 + 4;
	float* vertex_values = (float*)malloc(sizeof(float)*max_vertices*floats_per_vertex);
	
	// Load attributes for the textured shader
	Uint32 program_object = 0;
	GPU_ActivateShaderProgram(program_object, NULL);
	// Disable the default shader's attributes (not a typical thing to do...)
	GPU_ShaderBlock block = {-1,-1,-1,GPU_GetUniformLocation(program_object, "gpu_ModelViewProjectionMatrix")};
	GPU_ActivateShaderProgram(program_object, &block);
	
	GPU_Attribute attributes[3] = {
	    GPU_MakeAttribute(GPU_GetAttributeLocation(program_object, "gpu_Vertex"), vertex_values, 
                                                    GPU_MakeAttributeFormat(2, GPU_TYPE_FLOAT, 0, floats_per_vertex*sizeof(float), 0)),
        GPU_MakeAttribute(GPU_GetAttributeLocation(program_object, "gpu_TexCoord"), vertex_values, 
                                                    GPU_MakeAttributeFormat(2, GPU_TYPE_FLOAT, 0, floats_per_vertex*sizeof(float), 2*sizeof(float))),
        GPU_MakeAttribute(GPU_GetAttributeLocation(program_object, "gpu_Color"), vertex_values, 
                                                    GPU_MakeAttributeFormat(4, GPU_TYPE_FLOAT, 0, floats_per_vertex*sizeof(float), 4*sizeof(float)))
    };
	
	float* velx = (float*)malloc(sizeof(float)*max_vertices/3);
	float* vely = (float*)malloc(sizeof(float)*max_vertices/3);
	int i;
    int val_n = 0;
	for(i = 0; i < max_vertices; i+=3)
	{
		float offset_x1 = rand()%(image->w/2);
		float offset_y1 = rand()%(image->h/2);
		float x1 = vertex_values[val_n++] = rand()%screen->w + offset_x1;
		float y1 = vertex_values[val_n++] = rand()%screen->h + offset_y1;
		vertex_values[val_n++] = offset_x1/image->texture_w;
		vertex_values[val_n++] = offset_y1/image->texture_h;
		vertex_values[val_n++] = rand()%101/100.0f;
		vertex_values[val_n++] = rand()%101/100.0f;
		vertex_values[val_n++] = rand()%101/100.0f;
		vertex_values[val_n++] = rand()%101/100.0f;
		
		float offset_x2 = 5 + rand()%(image->w/2);
		float offset_y2 = 5 + rand()%(image->h/2);
		vertex_values[val_n++] = x1 + offset_x2;
		vertex_values[val_n++] = y1 + offset_y2;
		vertex_values[val_n++] = (offset_x1 + offset_x2)/image->texture_w;
		vertex_values[val_n++] = (offset_y1 + offset_y2)/image->texture_h;
		vertex_values[val_n++] = rand()%101/100.0f;
		vertex_values[val_n++] = rand()%101/100.0f;
		vertex_values[val_n++] = rand()%101/100.0f;
		vertex_values[val_n++] = rand()%101/100.0f;
		
		float offset_x3 = -5 - rand()%(image->w/2);
		float offset_y3 = 5 + rand()%(image->h/2);
		vertex_values[val_n++] = x1 + offset_x3;
		vertex_values[val_n++] = y1 + offset_y3;
		vertex_values[val_n++] = (offset_x1 + offset_x3)/image->texture_w;
		vertex_values[val_n++] = (offset_y1 + offset_y3)/image->texture_h;
		vertex_values[val_n++] = rand()%101/100.0f;
		vertex_values[val_n++] = rand()%101/100.0f;
		vertex_values[val_n++] = rand()%101/100.0f;
		vertex_values[val_n++] = rand()%101/100.0f;
		
		if(i%3 == 0)  // Once per triangle
        {
            int n = i/3;
            velx[n] = 10 + rand()%screen->w/10;
            vely[n] = 10 + rand()%screen->h/10;
            if(rand()%2)
                velx[n] = -velx[n];
            if(rand()%2)
                vely[n] = -vely[n];
        }
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
				    num_vertices += 300;
					if(num_vertices > max_vertices)
						num_vertices = max_vertices;
                    GPU_LogError("Vertices: %d\n", num_vertices);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
				else if(event.key.keysym.sym == SDLK_MINUS)
				{
					if(num_vertices > 3)
						num_vertices -= 300;
					if(num_vertices < 3)
                        num_vertices = 3;
                    GPU_LogError("Vertices: %d\n", num_vertices);
                    frameCount = 0;
                    startTime = SDL_GetTicks();
				}
			}
		}
		
		GPU_Clear(screen);
		
		// FIXME: Can cause squishing when a vertex hits a wall...
		for(i = 0; i < num_vertices; i++)
		{
		    int n = i/3;
		    val_n = floats_per_vertex*i;
			vertex_values[val_n] += velx[n]*dt;
			vertex_values[val_n+1] += vely[n]*dt;
			if(vertex_values[val_n] < 0)
			{
				vertex_values[val_n] = 0;
				velx[n] = -velx[n];
			}
			else if(vertex_values[val_n] > screen->w)
			{
				vertex_values[val_n] = screen->w;
				velx[n] = -velx[n];
			}
			
			if(vertex_values[val_n+1] < 0)
			{
				vertex_values[val_n+1] = 0;
				vely[n] = -vely[n];
			}
			else if(vertex_values[val_n+1] > screen->h)
			{
				vertex_values[val_n+1] = screen->h;
				vely[n] = -vely[n];
			}
		}
		
		GPU_SetAttributeSource(num_vertices, attributes[0]);
		GPU_SetAttributeSource(num_vertices, attributes[1]);
		GPU_SetAttributeSource(num_vertices, attributes[2]);
        GPU_TriangleBatch(image, screen, num_vertices, NULL, 0, NULL, 0);
		
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
	
	free(vertex_values);
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
            i = do_indexed(screen);
        else if(i == 3)
            i = do_attributes(screen);
        else
            i = 0;
    }
	
	GPU_Quit();
	
	return i;
}


