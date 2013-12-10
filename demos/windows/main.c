#include "SDL.h"

typedef struct Sprite
{
    SDL_Texture* texture;
    float x, y;
    float velx, vely;
} Sprite;

typedef struct Group
{
    Uint32 windowID;
    SDL_Renderer* renderer;
    Sprite sprite;
} Group;

#define screen_w 300
#define screen_h 300

#define sprite_w 100
#define sprite_h 100

Group create_group()
{
    Group g;
    SDL_Window* window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, 0);
    g.windowID = SDL_GetWindowID(window);
    SDL_Log("New windowID: %u\n", g.windowID);
    
    g.renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    SDL_Surface* surface = SDL_LoadBMP("data/test.bmp");
    
    g.sprite.texture = SDL_CreateTextureFromSurface(g.renderer, surface);
    g.sprite.x = rand()%screen_w;
    g.sprite.y = rand()%screen_h;
    g.sprite.velx = 10 + rand()%screen_w/10;
    g.sprite.vely = 10 + rand()%screen_h/10;
    
    SDL_FreeSurface(surface);
    
    return g;
}

int main(int argc, char* argv[])
{
    int max_groups = 30;
    Group groups[max_groups];
    memset(groups, 0, sizeof(Group)*max_groups);
	
	int num_groups = 0;
	groups[num_groups] = create_group();
	num_groups++;
	
	int i = 0;
	
	float dt = 0.010f;
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
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
					if(num_groups < max_groups)
                    {
                        groups[num_groups] = create_group();
                        num_groups++;
                        SDL_Log("num_groups: %d\n", num_groups);
                    }
				}
				else if(event.key.keysym.sym == SDLK_MINUS)
				{
					if(num_groups > 0)
                    {
						
                        for(i = max_groups-1; i >= 0; i--)
                        {
                            if(groups[i].windowID != 0)
                            {
                                SDL_DestroyTexture(groups[i].sprite.texture);
                                SDL_DestroyRenderer(groups[i].renderer);
                                SDL_DestroyWindow(SDL_GetWindowFromID(groups[i].windowID));
                                groups[i].windowID = 0;
                                num_groups--;
                                SDL_Log("num_groups: %d\n", num_groups);
                                break;
                            }
                        }
                        
                        if(num_groups == 0)
                            done = 1;
                    }
				}
			}
			else if(event.type == SDL_WINDOWEVENT)
            {
                if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    Uint8 closed = 0;
                    for(i = 0; i < max_groups; i++)
                    {
                        if(groups[i].windowID != 0 && groups[i].windowID == event.window.windowID)
                        {
                            SDL_DestroyTexture(groups[i].sprite.texture);
                            SDL_DestroyRenderer(groups[i].renderer);
                            SDL_DestroyWindow(SDL_GetWindowFromID(groups[i].windowID));
                            groups[i].windowID = 0;
                            closed = 1;
                            num_groups--;
                            SDL_Log("num_groups: %d\n", num_groups);
                            break;
                        }
                    }
                    
                    // The main window was closed, then.
                    if(!closed || num_groups == 0)
                        done = 1;
                }
            }
		}
		
		for(i = 0; i < max_groups; i++)
		{
			groups[i].sprite.x += groups[i].sprite.velx*dt;
			groups[i].sprite.y += groups[i].sprite.vely*dt;
			if(groups[i].sprite.x < 0)
			{
				groups[i].sprite.x = 0;
				groups[i].sprite.velx = -groups[i].sprite.velx;
			}
			else if(groups[i].sprite.x > screen_w)
			{
				groups[i].sprite.x = screen_w;
				groups[i].sprite.velx = -groups[i].sprite.velx;
			}
			
			if(groups[i].sprite.y < 0)
			{
				groups[i].sprite.y = 0;
				groups[i].sprite.vely = -groups[i].sprite.vely;
			}
			else if(groups[i].sprite.y > screen_h)
			{
				groups[i].sprite.y = screen_h;
				groups[i].sprite.vely = -groups[i].sprite.vely;
			}
		}
		
		for(i = 0; i < max_groups; i++)
		{
		    if(groups[i].windowID == 0)
                continue;
		    
		    SDL_RenderClear(groups[i].renderer);
		    
		    SDL_Rect dstrect = {groups[i].sprite.x, groups[i].sprite.y, sprite_w, sprite_h};
		    SDL_RenderCopy(groups[i].renderer, groups[i].sprite.texture, NULL, &dstrect);
		    
		    SDL_RenderPresent(groups[i].renderer);
		}
		
		SDL_Delay(10);
	}
    
    for(i = 0; i < max_groups; i++)
    {
        if(groups[i].windowID == 0)
            continue;
        
        SDL_DestroyTexture(groups[i].sprite.texture);
        SDL_DestroyRenderer(groups[i].renderer);
        SDL_DestroyWindow(SDL_GetWindowFromID(groups[i].windowID));
    }
    
    
    SDL_Quit();
    
    return 0;
}
