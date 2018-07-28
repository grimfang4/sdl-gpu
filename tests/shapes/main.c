#include "SDL.h"
#include "SDL_gpu.h"
#include <math.h>
#include "compat.h"
#include "common.h"
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159f
#endif

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
		
        int shapeType;
        int numShapeTypes;
        int i;
        #define NUM_COLORS 20
        SDL_Color colors[NUM_COLORS];
        
        #define NUM_PIXELS NUM_COLORS
        int px[NUM_PIXELS];
        int py[NUM_PIXELS];
        
        #define NUM_LINES NUM_COLORS
        int lx1[NUM_LINES];
        int ly1[NUM_LINES];
        int lx2[NUM_LINES];
        int ly2[NUM_LINES];
        
        #define NUM_TRIS NUM_COLORS
        int tx1[NUM_TRIS];
        int ty1[NUM_TRIS];
        int tx2[NUM_TRIS];
        int ty2[NUM_TRIS];
        int tx3[NUM_TRIS];
        int ty3[NUM_TRIS];
        
        #define NUM_RECTS NUM_COLORS
        int rx1[NUM_RECTS];
        int ry1[NUM_RECTS];
        int rx2[NUM_RECTS];
        int ry2[NUM_RECTS];
        float rr[NUM_RECTS];
        
        #define NUM_ARCS NUM_COLORS
        int ax[NUM_ARCS];
        int ay[NUM_ARCS];
        float ar[NUM_ARCS];
        float ar2[NUM_ARCS];
        float aa1[NUM_ARCS];
        float aa2[NUM_ARCS];
        
        #define NUM_POLYS NUM_COLORS
        int pn[NUM_POLYS];
        float* pv[NUM_POLYS];
        
        Uint8 blend;
        float thickness;
        
        startTime = SDL_GetTicks();
        frameCount = 0;
        
        shapeType = 0;
        numShapeTypes = 18;
        
        for(i = 0; i < NUM_COLORS; i++)
        {
            colors[i].r = rand()%256;
            colors[i].g = rand()%256;
            colors[i].b = rand()%256;
            GET_ALPHA(colors[i]) = rand()%256;
        }
        
        
        
        for(i = 0; i < NUM_PIXELS; i++)
        {
            px[i] = rand()%screen->w;
            py[i] = rand()%screen->h;
        }
        
        for(i = 0; i < NUM_LINES; i++)
        {
            lx1[i] = rand()%screen->w;
            ly1[i] = rand()%screen->h;
            lx2[i] = rand()%screen->w;
            ly2[i] = rand()%screen->h;
        }
        
        for(i = 0; i < NUM_TRIS; i++)
        {
            tx1[i] = rand()%screen->w;
            ty1[i] = rand()%screen->h;
            tx2[i] = rand()%screen->w;
            ty2[i] = rand()%screen->h;
            tx3[i] = rand()%screen->w;
            ty3[i] = rand()%screen->h;
        }
        
        
        for(i = 0; i < NUM_RECTS; i++)
        {
            rx1[i] = rand()%screen->w;
            ry1[i] = rand()%screen->h;
            rx2[i] = rand()%screen->w;
            ry2[i] = rand()%screen->h;
            rr[i] = rand()%10 + 2;
        }
        
        for(i = 0; i < NUM_ARCS; i++)
        {
            ax[i] = rand()%screen->w;
            ay[i] = rand()%screen->h;
            ar[i] = (rand()%screen->h)/10.0f;
            ar2[i] = ((rand()%101)/100.0f)*ar[i];
            aa1[i] = rand()%360;
            aa2[i] = rand()%360;
        }
        
        for(i = 0; i < NUM_POLYS; i++)
        {
            float cx = rand()%screen->w;
            float cy = rand()%screen->h;
            float radius = 20 + rand()%(screen->w/8);
            
            int j;
            
            pn[i] = rand()%8 + 3;
            pv[i] = (float*)malloc(2*pn[i]*sizeof(float));
            
            for(j = 0; j < pn[i]*2; j+=2)
            {
                pv[i][j] = cx + radius*cos(2*M_PI*(((float)j)/(pn[i]*2))) + rand()%((int)radius/2);
                pv[i][j+1] = cy + radius*sin(2*M_PI*(((float)j)/(pn[i]*2))) + rand()%((int)radius/2);
            }
        }
        
        blend = 0;
        thickness = 1.0f;
        
        GPU_SetShapeBlending(blend);
        
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
                        shapeType++;
                        if(shapeType >= numShapeTypes)
                            shapeType = 0;
                    }
                    else if(event.key.keysym.sym == SDLK_BACKSPACE)
                    {
                        shapeType--;
                        if(shapeType < 0)
                            shapeType = numShapeTypes-1;
                    }
                    else if(event.key.keysym.sym == SDLK_b)
                    {
                        blend = !blend;
                        GPU_SetShapeBlending(blend);
                    }
                    else if(event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_EQUALS)
                    {
                        thickness += 0.25f;
                        GPU_LogError("thickness: %.2f\n", thickness);
                        GPU_SetLineThickness(thickness);
                    }
                    else if(event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_MINUS)
                    {
                        if(thickness > 0.25f)
                            thickness -= 0.25f;
                        GPU_LogError("thickness: %.2f\n", thickness);
                        GPU_SetLineThickness(thickness);
                    }
                }
                else if(event.type == SDL_MOUSEBUTTONDOWN)
                {
                    if(event.button.button == SDL_BUTTON_LEFT)
                    {
                        shapeType++;
                        if(shapeType >= numShapeTypes)
                            shapeType = 0;
                    }
                    else if(event.button.button == SDL_BUTTON_RIGHT)
                    {
                        shapeType--;
                        if(shapeType < 0)
                            shapeType = numShapeTypes-1;
                    }
                }
            }
            
            GPU_Clear(screen);
            
            switch(shapeType)
            {
                case 0:
                    for(i = 0; i < NUM_PIXELS; i++)
                    {
                        GPU_Pixel(screen, px[i], py[i], colors[i]);
                    }
                    break;
                case 1:
                    for(i = 0; i < NUM_LINES; i++)
                    {
                        GPU_Line(screen, lx1[i], ly1[i], lx2[i], ly2[i], colors[i]);
                    }
                    break;
                case 2:
                    for(i = 0; i < NUM_TRIS; i++)
                    {
                        GPU_Tri(screen, tx1[i], ty1[i], tx2[i], ty2[i], tx3[i], ty3[i], colors[i]);
                    }
                    break;
                case 3:
                    for(i = 0; i < NUM_TRIS; i++)
                    {
                        GPU_TriFilled(screen, tx1[i], ty1[i], tx2[i], ty2[i], tx3[i], ty3[i], colors[i]);
                    }
                    break;
                case 4:
                    for(i = 0; i < NUM_RECTS; i++)
                    {
                        GPU_Rectangle(screen, rx1[i], ry1[i], rx2[i], ry2[i], colors[i]);
                    }
                    break;
                case 5:
                    for(i = 0; i < NUM_RECTS; i++)
                    {
                        GPU_RectangleFilled(screen, rx1[i], ry1[i], rx2[i], ry2[i], colors[i]);
                    }
                    break;
                case 6:
                    for(i = 0; i < NUM_RECTS; i++)
                    {
                        GPU_RectangleRound(screen, rx1[i], ry1[i], rx2[i], ry2[i], rr[i], colors[i]);
                    }
                    break;
                case 7:
                    for(i = 0; i < NUM_RECTS; i++)
                    {
                        GPU_RectangleRoundFilled(screen, rx1[i], ry1[i], rx2[i], ry2[i], rr[i], colors[i]);
                    }
                    break;
                case 8:
                    for(i = 0; i < NUM_ARCS; i++)
                    {
                        GPU_Arc(screen, ax[i], ay[i], ar[i], aa1[i], aa2[i], colors[i]);
                    }
                    break;
                case 9:
                    for(i = 0; i < NUM_ARCS; i++)
                    {
                        GPU_ArcFilled(screen, ax[i], ay[i], ar[i], aa1[i], aa2[i], colors[i]);
                    }
                    break;
                case 10:
                    for(i = 0; i < NUM_ARCS; i++)
                    {
                        GPU_Circle(screen, ax[i], ay[i], ar[i], colors[i]);
                    }
                    break;
                case 11:
                    for(i = 0; i < NUM_ARCS; i++)
                    {
                        GPU_CircleFilled(screen, ax[i], ay[i], ar[i], colors[i]);
                    }
                    break;
                case 12:
                    for(i = 0; i < NUM_ARCS; i++)
                    {
                        GPU_Ellipse(screen, ax[i], ay[i], ar[i], ar2[i], aa1[i], colors[i]);
                    }
                    break;
                case 13:
                    for(i = 0; i < NUM_ARCS; i++)
                    {
                        GPU_EllipseFilled(screen, ax[i], ay[i], ar[i], ar2[i], aa1[i], colors[i]);
                    }
                    break;
                case 14:
                    for(i = 0; i < NUM_ARCS; i++)
                    {
                        GPU_Sector(screen, ax[i], ay[i], ar[i], ar2[i], aa1[i], aa2[i], colors[i]);
                    }
                    break;
                case 15:
                    for(i = 0; i < NUM_ARCS; i++)
                    {
                        GPU_SectorFilled(screen, ax[i], ay[i], ar[i], ar2[i], aa1[i], aa2[i], colors[i]);
                    }
                    break;
                case 16:
                    for(i = 0; i < NUM_POLYS; i++)
                    {
                        GPU_Polygon(screen, pn[i], pv[i], colors[i]);
                    }
                    break;
                case 17:
                    for(i = 0; i < NUM_POLYS; i++)
                    {
                        GPU_PolygonFilled(screen, pn[i], pv[i], colors[i]);
                    }
                    break;
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


