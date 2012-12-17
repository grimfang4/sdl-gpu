#include "SDL.h"
#include "stb_image.h"

SDL_Surface* loadImage(const char* filename)
{
	int w, h, bytesperpixel;
    unsigned char *data = stbi_load(filename, &w, &h, &bytesperpixel, 0);
	if(data == NULL)
	{
		printf("Failed to load image \"%s\": NULL from stbi_load()\n", filename);
		return NULL;
	}
	
	
//     N=#comp     components
//       1           grey
//       2           grey, alpha
//       3           red, green, blue
//       4           red, green, blue, alpha
	
	Uint32 Rmask, Gmask, Bmask, Amask = 0;
	switch(bytesperpixel)
	{
		case 1:
			Rmask = 0xff;
			Gmask = 0xff;
			Bmask = 0xff;
			break;
		case 2:
			#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			Rmask = 0xff00;
			Gmask = 0xff00;
			Bmask = 0xff00;
			Amask = 0x00ff;
			#else
			Rmask = 0x00ff;
			Gmask = 0x00ff;
			Bmask = 0x00ff;
			Amask = 0xff00;
			#endif
			break;
		case 3:
			#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			Rmask = 0xFF0000;
			Gmask = 0x00FF00;
			Bmask = 0x0000FF;
			#else
			Rmask = 0x0000FF;
			Gmask = 0x00FF00;
			Bmask = 0xFF0000;
			#endif
			break;
		case 4:
			#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			Rmask = 0xFF000000;
			Gmask = 0x00FF0000;
			Bmask = 0x0000FF00;
			Amask = 0x000000FF;
			#else
			Rmask = 0x000000FF;
			Gmask = 0x0000FF00;
			Bmask = 0x00FF0000;
			Amask = 0xFF000000;
			#endif
			break;
		default:
			stbi_image_free(data);
			printf("Failed to load image \"%s\": Unsupported bit depth of %d\n", filename, bytesperpixel*8);
			return NULL;
	}
	
    // ... w = width, h = height, n = # 8-bit components per pixel ...
    // No padding for pitch, so it's w*bytesperpixel
	SDL_Surface* result = SDL_CreateRGBSurfaceFrom(data, w, h, bytesperpixel*8, w*bytesperpixel,
                        Rmask, Gmask, Bmask, Amask);
	
	if(result == NULL)
	{
		printf("Failed to load image \"%s\": SDL_CreateRGBSurfaceFrom() failed.\n", filename);
		return NULL;
	}
	
	return result;
}

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	
	SDL_Surface* screen = SDL_SetVideoMode(800, 600, 0, SDL_SWSURFACE);
	
	int numImages = 0;
	
	SDL_Surface* images[argc-1];
	
	int i;
	for(i = 1; i < argc; i++)
	{
		images[numImages] = loadImage(argv[i]);
		if(images[numImages] != NULL)
			numImages++;
	}
	
	printf("Number of images: %d\n", numImages);
	
	float x = 0;
	float y = 0;
	
	Uint8* keystates = SDL_GetKeyState(NULL);
	
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
			}
		}
		
		
		if(keystates[SDLK_UP])
		{
			y -= 1000*dt;
		}
		else if(keystates[SDLK_DOWN])
		{
			y += 1000*dt;
		}
		if(keystates[SDLK_LEFT])
		{
			x -= 1000*dt;
		}
		else if(keystates[SDLK_RIGHT])
		{
			x += 1000*dt;
		}
		
		SDL_FillRect(screen, NULL, 0x000000);
		
		float xx = 10;
		float yy = 10;
		float maxY = 0;
		for(i = 0; i < numImages; i++)
		{
			SDL_Rect destrect = {-x + xx, -y + yy, 1,1};
			SDL_BlitSurface(images[i], NULL, screen, &destrect);
			
			xx += images[i]->w + 10;
			if(maxY < images[i]->h)
				maxY = images[i]->h;
			
			if((i+1)%4 == 0)
			{
				xx = 10;
				yy += maxY + 10;
				maxY = 0;
			}
		}
		
		
		SDL_Flip(screen);
		SDL_Delay(10);
	}
	
	for(i = 0; i < numImages; i++)
	{
		SDL_FreeSurface(images[i]);
	}
	
	SDL_Quit();
	
	return 0;
}


