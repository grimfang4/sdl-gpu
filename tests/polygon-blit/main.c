#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "common.h"
#include <stdlib.h>


void set_vertex(float* vertex_array, unsigned int vertex_index, unsigned short* index_array, unsigned int index_array_index, float x, float y, float s, float t, float r, float g, float b, float a)
{
    unsigned int n = vertex_index*8;
    vertex_array[n++] = x;
    vertex_array[n++] = y;
    vertex_array[n++] = s;
    vertex_array[n++] = t;
    vertex_array[n++] = r;
    vertex_array[n++] = g;
    vertex_array[n++] = b;
    vertex_array[n++] = a;
    index_array[index_array_index++] = vertex_index;
}

void set_index(unsigned short* index_array, unsigned int index_array_index, unsigned short index)
{
    index_array[index_array_index] = index;
}

void polygon_blit(GPU_Image* image, GPU_Target* target, unsigned int num_vertices, float* vertices_xy, float tex_x, float tex_y, float scale_x, float scale_y)
{
    GPU_WrapEnum wrap_x, wrap_y;
    float* vertices;
    unsigned short* indices;
    unsigned short vcount;
    unsigned int icount;
    
    int numSegments;
    float r, g, b, a;
    unsigned int i;
    unsigned short last_index;
    
    if(num_vertices < 3)
        return;
    
    wrap_x = image->wrap_mode_x;
    wrap_y = image->wrap_mode_y;
    GPU_SetWrapMode(image, GPU_WRAP_REPEAT, GPU_WRAP_REPEAT);
    
    vertices = (float*)malloc(8*num_vertices*sizeof(float));
    indices = (unsigned short*)malloc((3 + (num_vertices-3)*3)*sizeof(unsigned short));
    vcount = 0;
    icount = 0;
    
    numSegments = 2*num_vertices;
    r = g = b = a = 255.0f;
    
    // Using a fan of triangles assumes that the polygon is convex
    i = 0;
    
    // First triangle
    set_vertex(vertices, vcount++, indices, icount++, vertices_xy[i], vertices_xy[i+1], (vertices_xy[i] - tex_x)*scale_x, (vertices_xy[i+1] - tex_y)*scale_y, r, g, b, a);
    i+=2;
    set_vertex(vertices, vcount++, indices, icount++, vertices_xy[i], vertices_xy[i+1], (vertices_xy[i] - tex_x)*scale_x, (vertices_xy[i+1] - tex_y)*scale_y, r, g, b, a);
    i+=2;
    set_vertex(vertices, vcount++, indices, icount++, vertices_xy[i], vertices_xy[i+1], (vertices_xy[i] - tex_x)*scale_x, (vertices_xy[i+1] - tex_y)*scale_y, r, g, b, a);
    i+=2;
    
    last_index = 2;
    
    while(i < numSegments)
    {
        set_index(indices, icount++, 0);  // Start from the first vertex
        set_index(indices, icount++, last_index);  // Double the last one
        set_vertex(vertices, vcount++, indices, icount++, vertices_xy[i], vertices_xy[i+1], (vertices_xy[i] - tex_x)*scale_x, (vertices_xy[i+1] - tex_y)*scale_y, r, g, b, a);
        i+=2;
        last_index++;
    }
    
    // Do the rendering
    GPU_TriangleBatch(image, target, vcount, vertices, icount, indices, GPU_NONE);
    
    // Restore wrap mode
    GPU_SetWrapMode(image, wrap_x, wrap_y);
    
    free(indices);
    free(vertices);
}

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
        
        GPU_Image* img;
        int i;
        #define NUM_POLYS 4
        int pn[NUM_POLYS];
        float* pv[NUM_POLYS];
        
        startTime = SDL_GetTicks();
        frameCount = 0;
        
        
        img = GPU_LoadImage("data/test4.bmp");
        
        for(i = 0; i < NUM_POLYS; i++)
        {
            int j;
            pn[i] = rand()%8;
            pv[i] = (float*)malloc(2*pn[i]*sizeof(float));
            for(j = 0; j < pn[i]*2; j+=2)
            {
                pv[i][j] = rand()%screen->w;
                pv[i][j+1] = rand()%screen->h;
            }
        }
        
        
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
                }
            }
            
            GPU_Clear(screen);
            
            for(i = 0; i < NUM_POLYS; i++)
            {
                polygon_blit(img, screen, pn[i], pv[i], pv[i][0], pv[i][1], 1.0f, 1.0f);
            }
            
            
            GPU_Flip(screen);
            
            frameCount++;
            if(frameCount%500 == 0)
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        }
        
        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        
        for(i = 0; i < NUM_POLYS; i++)
        {
            free(pv[i]);
        }
	}
	
	GPU_Quit();
	
	return 0;
}


