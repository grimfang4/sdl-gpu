#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"
#include <stdlib.h>

void fill_vertex_values(float* vertex_values, float* velx, float* vely, unsigned int max_vertices, GPU_Target* screen, GPU_Image* image)
{
    unsigned int i;
    int val_n = 0;
    Uint16 w, h;
    Uint16 tex_w, tex_h;
    
    if(image != NULL)
    {
        w = image->w;
        h = image->h;
        
        tex_w = image->texture_w;
        tex_h = image->texture_h;
    }
    else
        w = h = tex_w = tex_h = 80;
    
    for(i = 0; i < max_vertices; i+=3)
    {
	    float offset_x1, offset_x2, offset_x3;
	    float offset_y1, offset_y2, offset_y3;
	    float x1, y1;
	    
		offset_x1 = rand()%(w/2);
		offset_y1 = rand()%(h/2);
		x1 = vertex_values[val_n++] = rand()%screen->w + offset_x1;
		y1 = vertex_values[val_n++] = rand()%screen->h + offset_y1;
        if(image != NULL)
        {
            vertex_values[val_n++] = offset_x1/tex_w;
            vertex_values[val_n++] = offset_y1/tex_h;
        }
		vertex_values[val_n++] = (rand()%256)/255.0f;
		vertex_values[val_n++] = (rand()%256)/255.0f;
		vertex_values[val_n++] = (rand()%256)/255.0f;
		vertex_values[val_n++] = (rand()%256)/255.0f;
		
		offset_x2 = 5 + rand()%(w/2);
		offset_y2 = 5 + rand()%(h/2);
		vertex_values[val_n++] = x1 + offset_x2;
		vertex_values[val_n++] = y1 + offset_y2;
        if(image != NULL)
        {
            vertex_values[val_n++] = (offset_x1 + offset_x2)/tex_w;
            vertex_values[val_n++] = (offset_y1 + offset_y2)/tex_h;
        }
		vertex_values[val_n++] = (rand()%256)/255.0f;
		vertex_values[val_n++] = (rand()%256)/255.0f;
		vertex_values[val_n++] = (rand()%256)/255.0f;
		vertex_values[val_n++] = (rand()%256)/255.0f;
		
		offset_x3 = -5 - rand()%(w/2);
		offset_y3 = 5 + rand()%(h/2);
		vertex_values[val_n++] = x1 + offset_x3;
		vertex_values[val_n++] = y1 + offset_y3;
        if(image != NULL)
        {
            vertex_values[val_n++] = (offset_x1 + offset_x3)/tex_w;
            vertex_values[val_n++] = (offset_y1 + offset_y3)/tex_h;
        }
		vertex_values[val_n++] = (rand()%256)/255.0f;
		vertex_values[val_n++] = (rand()%256)/255.0f;
		vertex_values[val_n++] = (rand()%256)/255.0f;
		vertex_values[val_n++] = (rand()%256)/255.0f;
        
        // Once per triangle
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
}

void fill_vertex_values_uncolored(float* vertex_values, float* velx, float* vely, unsigned int max_vertices, GPU_Target* screen, GPU_Image* image)
{
    unsigned int i;
    int val_n = 0;
    Uint16 w, h;
    Uint16 tex_w, tex_h;
    
    if(image != NULL)
    {
        w = image->w;
        h = image->h;
        
        tex_w = image->texture_w;
        tex_h = image->texture_h;
    }
    else
        w = h = tex_w = tex_h = 80;
    
    for(i = 0; i < max_vertices; i+=3)
    {
	    float offset_x1, offset_x2, offset_x3;
	    float offset_y1, offset_y2, offset_y3;
	    float x1, y1;
	    
		offset_x1 = rand()%(w/2);
		offset_y1 = rand()%(h/2);
		x1 = vertex_values[val_n++] = rand()%screen->w + offset_x1;
		y1 = vertex_values[val_n++] = rand()%screen->h + offset_y1;
        if(image != NULL)
        {
            vertex_values[val_n++] = offset_x1/tex_w;
            vertex_values[val_n++] = offset_y1/tex_h;
        }
		
		offset_x2 = 5 + rand()%(w/2);
		offset_y2 = 5 + rand()%(h/2);
		vertex_values[val_n++] = x1 + offset_x2;
		vertex_values[val_n++] = y1 + offset_y2;
        if(image != NULL)
        {
            vertex_values[val_n++] = (offset_x1 + offset_x2)/tex_w;
            vertex_values[val_n++] = (offset_y1 + offset_y2)/tex_h;
        }
		
		offset_x3 = -5 - rand()%(w/2);
		offset_y3 = 5 + rand()%(h/2);
		vertex_values[val_n++] = x1 + offset_x3;
		vertex_values[val_n++] = y1 + offset_y3;
        if(image != NULL)
        {
            vertex_values[val_n++] = (offset_x1 + offset_x3)/tex_w;
            vertex_values[val_n++] = (offset_y1 + offset_y3)/tex_h;
        }
        
        // Once per triangle
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
}

int do_interleaved(GPU_Target* screen)
{
    Uint32 startTime;
    long frameCount;
    Uint8 done;
    SDL_Event event;
	
	int return_value = 0;
	
	float dt = 0.010f;
	
	int max_vertices = 60000;
	int num_vertices = 303;
	
	int floats_per_vertex = 2 + 2 + 4;
	float* vertex_values = (float*)malloc(sizeof(float)*max_vertices*floats_per_vertex);
	float* velx = (float*)malloc(sizeof(float)*max_vertices/3);
	float* vely = (float*)malloc(sizeof(float)*max_vertices/3);
	int i;
    
	GPU_Image* image = GPU_LoadImage("data/test3.png");
	GPU_LogError("do_interleaved()\n");
	if(image == NULL)
		return -1;
	
	startTime = SDL_GetTicks();
	frameCount = 0;
	
	fill_vertex_values(vertex_values, velx, vely, max_vertices, screen, image);
	
	
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
		    int val_n = floats_per_vertex*i;
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
		
        GPU_TriangleBatch(image, screen, num_vertices, vertex_values, 0, NULL, GPU_BATCH_XY_ST_RGBA);
		
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
    Uint32 startTime;
    long frameCount;
    Uint8 done;
    SDL_Event event;
	
	int return_value = 0;
	
	float dt = 0.010f;
	
	int max_vertices = 60000;
	int num_vertices = 303;
	
	int floats_per_vertex = 2 + 2 + 4;
	float* vertex_values = (float*)malloc(sizeof(float)*max_vertices*floats_per_vertex);
	float* velx = (float*)malloc(sizeof(float)*max_vertices/3);
	float* vely = (float*)malloc(sizeof(float)*max_vertices/3);
	unsigned short* indices = (unsigned short*)malloc(sizeof(unsigned short)*max_vertices);
	int i;
    
	GPU_Image* image = GPU_LoadImage("data/test3.png");
	GPU_LogError("do_indexed()\n");
	if(image == NULL)
		return -1;
	
	startTime = SDL_GetTicks();
	frameCount = 0;
	
	fill_vertex_values(vertex_values, velx, vely, max_vertices, screen, image);
	// Setup indices
	for(i = 0; i < max_vertices; i++)
        indices[i] = i;
	
	
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
		    int val_n = floats_per_vertex*i;
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
		
        GPU_TriangleBatch(image, screen, num_vertices, vertex_values, num_vertices, indices, GPU_BATCH_XY_ST_RGBA);
        
        {
            // Quad
            int n_v = 4;
            float v[] = {0, 0, 0, 0,
                       100, 0, 1, 0,
                       0, 100, 0, 1,
                       100, 100, 1, 1};
            
            int n_i = 6;
            unsigned short i[] = {0, 1, 2, 1, 3, 2};
            GPU_TriangleBatch(image, screen, n_v, v, n_i, i, GPU_BATCH_XY_ST);
        }
        
        {
            // Star
            int n_v = 10;
            float v[] = {0, -300,    0.5, 0,  // top
                         100, -110,  0.667, 0.317,  // top to right
                         300, -70,   1,   0.383,  // right
                         160, 90,    0.767,   0.65,  // right to bot
                         190, 300,   0.817, 1,  // bot right
                         0, 210,     0.5, 0.75,  // bot right to bot left
                         -190, 300,  0.183, 1,  // bot left
                         -160, 90,   0.233, 0.65,  // bot left to left
                         -300, -70,  0, 0.383,  // left
                         -100, -110, 0.333, 0.317};  // left to top
            
            int n_i = 15+9;
            unsigned short i[] = {0, 1, 9,
                                  1, 2, 3,
                                  3, 4, 5,
                                  5, 6, 7,
                                  7, 8, 9,
                                  9, 1, 7,
                                  1, 3, 7,
                                  3, 5, 7};
            
            GPU_MatrixMode(screen, GPU_MODEL);
            GPU_PushMatrix();
            GPU_Translate(300, 300, 0);
            GPU_TriangleBatch(image, screen, n_v, v, n_i, i, GPU_BATCH_XY_ST);
            GPU_PopMatrix();
        }
		
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
    Uint32 startTime;
    long frameCount;
    Uint8 done;
    SDL_Event event;
	
	int return_value = 0;
	
	float dt = 0.010f;
	
	int max_vertices = 60000;
	int num_vertices = 303;
	
	float* velx = (float*)malloc(sizeof(float)*max_vertices/3);
	float* vely = (float*)malloc(sizeof(float)*max_vertices/3);
	int i;
	
	// 2 pos floats per vertex, 2 texcoords, 4 color components
	int floats_per_vertex = 2 + 2 + 4;
	float* vertex_values = (float*)malloc(sizeof(float)*max_vertices*floats_per_vertex);
	
	Uint32 program_object;
	GPU_Attribute attributes[3];
    
	GPU_Image* image = GPU_LoadImage("data/small_test.png");
	
	GPU_LogError("do_attributes()\n");
	if(image == NULL)
		return -1;
	
	startTime = SDL_GetTicks();
	frameCount = 0;
	
	fill_vertex_values(vertex_values, velx, vely, max_vertices, screen, image);
	
	// Load attributes for the textured shader
	program_object = 0;
	GPU_ActivateShaderProgram(program_object, NULL);
	// Disable the default shader's attributes (not a typical thing to do...)
	{
        GPU_ShaderBlock block = {-1,-1,-1,GPU_GetUniformLocation(program_object, "gpu_ModelViewProjectionMatrix")};
        GPU_ActivateShaderProgram(program_object, &block);
	}
	
	attributes[0] = GPU_MakeAttribute(GPU_GetAttributeLocation(program_object, "gpu_Vertex"), vertex_values,
			GPU_MakeAttributeFormat(2, GPU_TYPE_FLOAT, 0, floats_per_vertex*sizeof(float), 0));
	attributes[1] = GPU_MakeAttribute(GPU_GetAttributeLocation(program_object, "gpu_TexCoord"), vertex_values,
			GPU_MakeAttributeFormat(2, GPU_TYPE_FLOAT, 0, floats_per_vertex*sizeof(float), 2 * sizeof(float)));
	attributes[2] = GPU_MakeAttribute(GPU_GetAttributeLocation(program_object, "gpu_Color"), vertex_values,
			GPU_MakeAttributeFormat(4, GPU_TYPE_FLOAT, 0, floats_per_vertex*sizeof(float), 4 * sizeof(float)));
        
        
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
                    done = 1;
                    return_value = 4;
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
            int val_n = floats_per_vertex*i;
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
        GPU_TriangleBatch(image, screen, num_vertices, NULL, 0, NULL, GPU_NONE);
            
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

int do_untextured(GPU_Target* screen)
{
    Uint32 startTime;
    long frameCount;
    Uint8 done;
    SDL_Event event;
	
	int return_value = 0;
	
	float dt = 0.010f;
	
	int max_vertices = 60000;
	int num_vertices = 303;
	
	float* velx = (float*)malloc(sizeof(float)*max_vertices/3);
	float* vely = (float*)malloc(sizeof(float)*max_vertices/3);
	int i;
    int val_n = 0;
	
	// 2 pos floats per vertex, 4 color components
	int floats_per_vertex = 2 + 4;
	float* vertex_values = (float*)malloc(sizeof(float)*max_vertices*floats_per_vertex);
	
	GPU_LogError("do_untextured()\n");
	
	startTime = SDL_GetTicks();
	frameCount = 0;
	
	fill_vertex_values(vertex_values, velx, vely, max_vertices, screen, NULL);
	
        
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
                    done = 1;
                    return_value = 5;
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
        
        GPU_TriangleBatch(NULL, screen, num_vertices, vertex_values, 0, NULL, GPU_BATCH_XY | GPU_BATCH_RGBA);
        
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
	
	// Reset the default shader's block
	GPU_ActivateShaderProgram(screen->context->default_textured_shader_program, NULL);
	
	return return_value;
}

int do_uncolored(GPU_Target* screen)
{
    Uint32 startTime;
    long frameCount;
    Uint8 done;
    SDL_Event event;
	
	int return_value = 0;
	
	float dt = 0.010f;
	
	int max_vertices = 60000;
	int num_vertices = 303;
	
	int floats_per_vertex = 2 + 2;  // xy, st
	float* vertex_values = (float*)malloc(sizeof(float)*max_vertices*floats_per_vertex);
	float* velx = (float*)malloc(sizeof(float)*max_vertices/3);
	float* vely = (float*)malloc(sizeof(float)*max_vertices/3);
	unsigned short* indices = (unsigned short*)malloc(sizeof(unsigned short)*max_vertices);
	int i;
    
	GPU_Image* image = GPU_LoadImage("data/test3.png");
	GPU_LogError("do_uncolored()\n");
	if(image == NULL)
		return -1;
	
	startTime = SDL_GetTicks();
	frameCount = 0;
	
	fill_vertex_values_uncolored(vertex_values, velx, vely, max_vertices, screen, image);
	// Setup indices
	for(i = 0; i < max_vertices; i++)
        indices[i] = i;
	
	
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
		
		// FIXME: Can cause squishing when a vertex hits a wall...
		for(i = 0; i < num_vertices; i++)
		{
		    int n = i/3;
		    int val_n = floats_per_vertex*i;
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
		
        GPU_TriangleBatch(image, screen, num_vertices, vertex_values, num_vertices, indices, GPU_BATCH_XY_ST);
		
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

int main(int argc, char* argv[])
{
	GPU_Target* screen;
	int i = 1;

	printRenderers();
	
	screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	while(i > 0)
    {
        if(i == 1)
            i = do_interleaved(screen);
        else if(i == 2)
            i = do_indexed(screen);
        else if(i == 3)
            i = do_attributes(screen);
        else if(i == 4)
            i = do_untextured(screen);
        else if(i == 5)
            i = do_uncolored(screen);
        else
            i = 0;
    }
	
	GPU_Quit();
	
	return i;
}


