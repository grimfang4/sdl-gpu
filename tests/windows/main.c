#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"

#ifdef SDL_GPU_USE_SDL1
// This demo doesn't work for SDL 1.2 because of the missing windowing features in that version.
int main(int argc, char* argv[])
{
    GPU_LogError("Sorry, this demo requires SDL 2.\n");
    return 0;
}

#else

typedef struct Sprite
{
    float x, y;
    float velx, vely;
} Sprite;

typedef struct Group
{
    Uint8 on;
    Uint32 windowID;
    Sprite sprite;
} Group;

#define screen_w 300
#define screen_h 300

#define sprite_w 100
#define sprite_h 100

Group create_first_group()
{
    Group g;
    Uint32 windowID = GPU_GetCurrentRenderer()->current_context_target->context->windowID;
    SDL_Log("New windowID: %u\n", windowID);
    
    g.windowID = windowID;
    
    g.sprite.x = rand()%screen_w;
    g.sprite.y = rand()%screen_h;
    g.sprite.velx = 10 + rand()%screen_w/10;
    g.sprite.vely = 10 + rand()%screen_h/10;
    
    g.on = 1;
    
    return g;
}

Group create_group()
{
    Group g;
    SDL_Window* window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, SDL_WINDOW_OPENGL);
    Uint32 windowID = SDL_GetWindowID(window);
    
    g.windowID = windowID;
    
    g.sprite.x = rand()%screen_w;
    g.sprite.y = rand()%screen_h;
    g.sprite.velx = 10 + rand()%screen_w/10;
    g.sprite.vely = 10 + rand()%screen_h/10;
    
    g.on = 1;
    
    return g;
}

void destroy_group(Group* groups, int i)
{
    SDL_DestroyWindow(SDL_GetWindowFromID(groups[i].windowID));
    groups[i].on = 0;
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
        
        int i = 0;
        
        float dt = 0.010f;
        
        #define MAX_GROUPS 30
        Group groups[MAX_GROUPS];
        int num_groups;
        
        GPU_Image* image;
        {
            SDL_Surface* surface = SDL_LoadBMP("data/test.bmp");
            
            image = GPU_CopyImageFromSurface(surface);
            
            SDL_FreeSurface(surface);
        }
		memset(groups, 0, sizeof(Group)*MAX_GROUPS);
        
        num_groups = 0;
        groups[num_groups] = create_first_group();
        num_groups++;
        
        startTime = SDL_GetTicks();
        frameCount = 0;
        
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
                    else if(event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_PLUS)
                    {
                        for(i = 0; i < MAX_GROUPS; i++)
                        {
                            if(!groups[i].on)
                            {
                                groups[i] = create_group();
                                num_groups++;
                                GPU_Log("Added window %u.  num_groups: %d\n", groups[i].windowID, num_groups);
                                break;
                            }
                        }
                    }
                    else if(event.key.keysym.sym == SDLK_MINUS)
                    {
                        if(num_groups > 0)
                        {
                            
                            for(i = MAX_GROUPS-1; i >= 0; i--)
                            {
                                if(groups[i].on)
                                {
                                    destroy_group(groups, i);
                                    
                                    num_groups--;
                                    GPU_Log("Removed window %u.  num_groups: %d\n", groups[i].windowID, num_groups);
                                    break;
                                }
                            }
                            
                            if(num_groups == 0)
                                done = 1;
                        }
                    }
                }
                else if(event.type == SDL_MOUSEBUTTONDOWN)
                {
                    GPU_Target* target = GPU_GetWindowTarget(event.button.windowID);
                    if(target == NULL)
                        GPU_Log("Clicked on window %u.  NULL target.\n", event.button.windowID);
                    else
                        GPU_Log("Clicked on window %u.  Target dims: %dx%d\n", event.button.windowID, target->w, target->h);
                }
                else if(event.type == SDL_WINDOWEVENT)
                {
                    if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                    {
                        Uint8 closed = 0;
                        for(i = 0; i < MAX_GROUPS; i++)
                        {
                            if(groups[i].on && groups[i].windowID == event.window.windowID)
                            {
                                destroy_group(groups, i);
                                
                                closed = 1;
                                num_groups--;
                                GPU_Log("num_groups: %d\n", num_groups);
                                break;
                            }
                        }
                        
                        // The last window was closed, then.
                        if(!closed || num_groups == 0)
                            done = 1;
                    }
                }
            }
            
            for(i = 0; i < MAX_GROUPS; i++)
            {
                if(!groups[i].on)
                    continue;
                
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
            
            for(i = 0; i < MAX_GROUPS; i++)
            {
                if(!groups[i].on)
                    continue;
                
                GPU_MakeCurrent(screen, groups[i].windowID);
                
                GPU_Clear(screen);
                
                GPU_Blit(image, NULL, screen, groups[i].sprite.x, groups[i].sprite.y);
                
                GPU_Flip(screen);
            }
            
            frameCount++;
            if(frameCount%500 == 0)
                GPU_Log("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        }
        
        GPU_Log("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        
        for(i = 0; i < MAX_GROUPS; i++)
        {
            if(!groups[i].on)
                continue;
            
            destroy_group(groups, i);
        }
        
	}
	
    GPU_Quit();
    
    return 0;
}

#endif
