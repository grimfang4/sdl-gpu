#include "SDL.h"
#include "SDL_gpu.h"
#include "SDL_gpuShapes.h"
#include <math.h>

int main(int argc, char* argv[])
{
	GPU_Target* screen = GPU_Init(800, 600, 0);
	if(screen == NULL)
		return -1;
	
	printf("Renderer: %s\n", GPU_GetRendererString());
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	int shapeType = 0;
	int numShapeTypes = 13;
	
	
	int i;
	
	int numColors = 20;
	SDL_Color colors[numColors];
	for(i = 0; i < numColors; i++)
	{
		colors[i].r = rand()%256;
		colors[i].g = rand()%256;
		colors[i].b = rand()%256;
		colors[i].unused = rand()%256;
	}
	
	int numPixels = numColors;
	int px[numPixels];
	int py[numPixels];
	for(i = 0; i < numPixels; i++)
	{
		px[i] = rand()%screen->w;
		py[i] = rand()%screen->h;
	}
	
	int numLines = numColors;
	int lx1[numLines];
	int ly1[numLines];
	int lx2[numLines];
	int ly2[numLines];
	for(i = 0; i < numLines; i++)
	{
		lx1[i] = rand()%screen->w;
		ly1[i] = rand()%screen->h;
		lx2[i] = rand()%screen->w;
		ly2[i] = rand()%screen->h;
	}
	
	int numTris = numColors;
	int tx1[numTris];
	int ty1[numTris];
	int tx2[numTris];
	int ty2[numTris];
	int tx3[numTris];
	int ty3[numTris];
	for(i = 0; i < numTris; i++)
	{
		tx1[i] = rand()%screen->w;
		ty1[i] = rand()%screen->h;
		tx2[i] = rand()%screen->w;
		ty2[i] = rand()%screen->h;
		tx3[i] = rand()%screen->w;
		ty3[i] = rand()%screen->h;
	}
	
	
	int numRects = numColors;
	int rx1[numRects];
	int ry1[numRects];
	int rx2[numRects];
	int ry2[numRects];
	float rr[numRects];
	for(i = 0; i < numRects; i++)
	{
		rx1[i] = rand()%screen->w;
		ry1[i] = rand()%screen->h;
		rx2[i] = rand()%screen->w;
		ry2[i] = rand()%screen->h;
		rr[i] = rand()%10 + 2;
	}
	
	int numArcs = numColors;
	int ax[numArcs];
	int ay[numArcs];
	float ar[numArcs];
	float aa1[numArcs];
	float aa2[numArcs];
	for(i = 0; i < numArcs; i++)
	{
		ax[i] = rand()%screen->w;
		ay[i] = rand()%screen->h;
		ar[i] = (rand()%screen->h)/10.0f;
		aa1[i] = rand()%360;
		aa2[i] = rand()%360;
	}
	
	int numCircles = numColors;
	int cx[numCircles];
	int cy[numCircles];
	float cr[numCircles];
	for(i = 0; i < numCircles; i++)
	{
		cx[i] = rand()%screen->w;
		cy[i] = rand()%screen->h;
		cr[i] = (rand()%screen->h)/10.0f;
	}
	
	int numPolys = numColors;
	int pn[numPolys];
	float* pv[numPolys];
	for(i = 0; i < numPolys; i++)
	{
		pn[i] = rand()%8;
		pv[i] = (float*)malloc(pn[i]*sizeof(float));
		int j;
		for(j = 0; j < pn[i]*2; j+=2)
		{
			pv[i][j] = rand()%screen->w;
			pv[i][j+1] = rand()%screen->h;
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
					shapeType++;
					if(shapeType >= numShapeTypes)
						shapeType = 0;
				}
			}
		}
		
		GPU_Clear(screen);
		
		switch(shapeType)
		{
			case 0:
				for(i = 0; i < numPixels; i++)
				{
					GPU_Pixel(screen, px[i], py[i], colors[i]);
				}
				break;
			case 1:
				for(i = 0; i < numLines; i++)
				{
					GPU_Line(screen, lx1[i], ly1[i], lx2[i], ly2[i], colors[i]);
				}
				break;
			case 2:
				for(i = 0; i < numTris; i++)
				{
					GPU_Tri(screen, tx1[i], ty1[i], tx2[i], ty2[i], tx3[i], ty3[i], colors[i]);
				}
				break;
			case 3:
				for(i = 0; i < numTris; i++)
				{
					GPU_TriFilled(screen, tx1[i], ty1[i], tx2[i], ty2[i], tx3[i], ty3[i], colors[i]);
				}
				break;
			case 4:
				for(i = 0; i < numRects; i++)
				{
					GPU_Rect(screen, rx1[i], ry1[i], rx2[i], ry2[i], colors[i]);
				}
				break;
			case 5:
				for(i = 0; i < numRects; i++)
				{
					GPU_RectFilled(screen, rx1[i], ry1[i], rx2[i], ry2[i], colors[i]);
				}
				break;
			case 6:
				for(i = 0; i < numArcs; i++)
				{
					GPU_Arc(screen, ax[i], ay[i], ar[i], aa1[i], aa2[i], colors[i]);
				}
				break;
			case 7:
				for(i = 0; i < numCircles; i++)
				{
					GPU_Circle(screen, cx[i], cy[i], cr[i], colors[i]);
				}
				break;
			case 8:
				for(i = 0; i < numCircles; i++)
				{
					GPU_CircleFilled(screen, cx[i], cy[i], cr[i], colors[i]);
				}
				break;
			case 9:
				for(i = 0; i < numRects; i++)
				{
					GPU_RectRound(screen, rx1[i], ry1[i], rx2[i], ry2[i], rr[i], colors[i]);
				}
				break;
			case 10:
				for(i = 0; i < numRects; i++)
				{
					GPU_RectRoundFilled(screen, rx1[i], ry1[i], rx2[i], ry2[i], rr[i], colors[i]);
				}
				break;
			case 11:
				for(i = 0; i < numPolys; i++)
				{
					GPU_Polygon(screen, pn[i], pv[i], colors[i]);
				}
				break;
			case 12:
				for(i = 0; i < numPolys; i++)
				{
					GPU_PolygonFilled(screen, pn[i], pv[i], colors[i]);
				}
				break;
		}
		
		GPU_Flip();
		
		frameCount++;
		if(frameCount%500 == 0)
			printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	}
	
	printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
	
	for(i = 0; i < numPolys; i++)
	{
		free(pv[i]);
	}
	
	GPU_Quit();
	
	return 0;
}


