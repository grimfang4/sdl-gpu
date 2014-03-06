#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"


GPU_ShaderBlock load_shaders(Uint32* v, Uint32* f, Uint32* p)
{
    GPU_Renderer* renderer = GPU_GetCurrentRenderer();
    const char* vertex_shader_file;
    const char* fragment_shader_file;
    if(renderer->shader_version < 130)
    {
        vertex_shader_file = "shader-attributes/test.vert";
        fragment_shader_file = "shader-attributes/test.frag";
    }
    else
    {
        vertex_shader_file = "shader-attributes/test3.vert";
        fragment_shader_file = "shader-attributes/test3.frag";
    }
    
    *v = GPU_LoadShader(GPU_VERTEX_SHADER, vertex_shader_file);
    
    if(!*v)
        GPU_LogError("Failed to load vertex shader: %s\n", GPU_GetShaderMessage());
    
    *f = GPU_LoadShader(GPU_FRAGMENT_SHADER, fragment_shader_file);
    
    if(!*f)
        GPU_LogError("Failed to load fragment shader: %s\n", GPU_GetShaderMessage());
    
    *p = GPU_LinkShaders(*v, *f);
    
    if(!*p)
    {
        GPU_LogError("Failed to link shader program: %s\n", GPU_GetShaderMessage());
		GPU_ShaderBlock b = {-1, -1, -1, -1};
        return b;
    }
    
    GPU_ShaderBlock block = GPU_LoadShaderBlock(*p, "gpu_Vertex", "gpu_TexCoord", NULL, "modelViewProjection");
    GPU_ActivateShaderProgram(*p, &block);
    
    return block;
}

void free_shaders(Uint32 v, Uint32 f, Uint32 p)
{
    GPU_FreeShader(v);
    GPU_FreeShader(f);
    GPU_FreeShaderProgram(p);
}

	
static const int grid_row_size = 10;
static const float grid_offset_x = 200;
static const float grid_offset_y = 30;
static const float grid_cell_w = 60;
static const float grid_cell_h = 60;

Uint8 use_color_expansion = 0;

void add_sprite(float* positions, float* colors, float* expanded_colors, float* src_rects, int* num_sprites, SDL_Color color, GPU_Rect src_rect)
{
    int i = *num_sprites;
    
	positions[2*i] = grid_offset_x + (i%grid_row_size)*grid_cell_w;
	positions[2*i+1] = grid_offset_y + (i/grid_row_size)*grid_cell_h;
	
	
    expanded_colors[4*i] = color.r/255.0f;
    expanded_colors[4*i+1] = color.g/255.0f;
    expanded_colors[4*i+2] = color.b/255.0f;
    #ifdef SDL_GPU_USE_SDL2
    expanded_colors[4*i+3] = color.a/255.0f;
    #else
    expanded_colors[4*i+3] = color.unused/255.0f;
    #endif
    
    int n;
    for(n = 0; n < 4; n++)
    {
        colors[4*(4*i+n)] = color.r/255.0f;
        colors[4*(4*i+n)+1] = color.g/255.0f;
        colors[4*(4*i+n)+2] = color.b/255.0f;
        #ifdef SDL_GPU_USE_SDL2
        colors[4*(4*i+n)+3] = color.a/255.0f;
        #else
        colors[4*(4*i+n)+3] = color.unused/255.0f;
        #endif
    }
	
	src_rects[4*i] = src_rect.x;
	src_rects[4*i+1] = src_rect.y;
	src_rects[4*i+2] = src_rect.w;
	src_rects[4*i+3] = src_rect.h;
	
	(*num_sprites)++;
}

int timeloc = -1;
GPU_Attribute color_attr;

void set_shader(Uint32 program, GPU_ShaderBlock* block)
{
    if(program == 0)
        program = GPU_GetContextTarget()->context->default_textured_shader_program;
    GPU_ActivateShaderProgram(program, block);
    
    timeloc = GPU_GetUniformLocation(program, "time");
    color_attr.location = GPU_GetAttributeLocation(program, "gpu_Color");
}

int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	printCurrentRenderer();
	
	GPU_Image* image = GPU_LoadImage("data/happy_50x50.bmp");
	if(image == NULL)
		return -1;
    
	
	int maxSprites = 100;
	int numSprites = 0;
    
    
	float positions[2*maxSprites];
	float colors[4*4*maxSprites];
	float expanded_colors[4*maxSprites];
	float src_rects[4*maxSprites];
    
    color_attr.format = GPU_MakeAttributeFormat(4, GPU_TYPE_FLOAT, 0, 4*sizeof(float), 0);
    color_attr.format.is_per_sprite = 0;
    color_attr.values = colors;
    
	
	Uint32 v, f, p;
	GPU_ShaderBlock block = load_shaders(&v, &f, &p);
	
	Uint8 shader_index = 1;
	set_shader(p, &block);
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	int i;
	SDL_Color color = {255, 255, 255, 255};
	GPU_Rect src_rect = {0, 0, image->w, image->h};
	
	add_sprite(positions, colors, expanded_colors, src_rects, &numSprites, color, src_rect);
	
	int mx, my;
	Uint32 mouse_state;
	
	Uint8 done = 0;
	SDL_Event event;
	while(!done)
	{
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT)
				done = 1;
			else if(event.type == SDL_MOUSEBUTTONDOWN)
            {
                if(event.button.x <= 150 && event.button.y <= 150)
                {
                    if(event.button.button == SDL_BUTTON_LEFT)
                    {
                        float dx = event.button.x/3 - src_rect.x;
                        float dy = event.button.y/3 - src_rect.y;
                        src_rect.x = event.button.x/3;
                        src_rect.y = event.button.y/3;
                        src_rect.w -= dx;
                        src_rect.h -= dy;
                    }
                    else if(event.button.button == SDL_BUTTON_RIGHT)
                    {
                        src_rect.w = event.button.x/3 - src_rect.x;
                        src_rect.h = event.button.y/3 - src_rect.y;
                    }
                }
            }
			else if(event.type == SDL_KEYDOWN)
			{
				if(event.key.keysym.sym == SDLK_ESCAPE)
					done = 1;
				else if(event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_PLUS)
				{
					if(numSprites < maxSprites)
                        add_sprite(positions, colors, expanded_colors, src_rects, &numSprites, color, src_rect);
				}
				else if(event.key.keysym.sym == SDLK_MINUS)
				{
					if(numSprites > 0)
						numSprites--;
				}
				else if(event.key.keysym.sym == SDLK_SPACE)
				{
				    shader_index++;
				    shader_index %= 2;
				    if(shader_index == 0)
                        set_shader(0, NULL);
                    else if(shader_index == 1)
                        set_shader(p, &block);
				}
				else if(event.key.keysym.sym == SDLK_RETURN)
				{
				    use_color_expansion = !use_color_expansion;
				    if(use_color_expansion)
                    {
                        GPU_LogError("Using attribute expansion.\n");
                        color_attr.format.is_per_sprite = 1;
                        color_attr.values = expanded_colors;
                    }
                    else
                    {
                        GPU_LogError("Using per-vertex attributes.\n");
                        color_attr.format.is_per_sprite = 0;
                        color_attr.values = colors;
                    }
				}
			}
		}
		
		mouse_state = SDL_GetMouseState(&mx, &my);
		if(mouse_state & (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
        {
            if(mx <= 150 && my <= 150)
            {
                if(mouse_state & SDL_BUTTON_LMASK)
                {
                    float dx = mx/3 - src_rect.x;
                    float dy = my/3 - src_rect.y;
                    src_rect.x = mx/3;
                    src_rect.y = my/3;
                    src_rect.w -= dx;
                    src_rect.h -= dy;
                }
                else if(mouse_state & SDL_BUTTON_RMASK)
                {
                    src_rect.w = mx/3 - src_rect.x;
                    src_rect.h = my/3 - src_rect.y;
                }
            }
        }
		
        GPU_SetUniformf(timeloc, SDL_GetTicks()/1000.0f);
        
		GPU_Clear(screen);
		
        if(use_color_expansion)
            GPU_SetAttributeSource(numSprites, color_attr);
        else
            GPU_SetAttributeSource(4*numSprites, color_attr);
        
		for(i = 0; i < numSprites; i++)
		{
		    GPU_Rect r = {src_rects[4*i], src_rects[4*i+1], src_rects[4*i+2], src_rects[4*i+3]};
		    GPU_Blit(image, &r, screen, positions[2*i], positions[2*i+1]);
		}
		//GPU_BlitBatchSeparate(image, screen, numSprites, positions, src_rects, expanded_colors, 0);
		
		set_shader(0, NULL);
		
		GPU_BlitScale(image, NULL, screen, 75, 75, 3.0f, 3.0f);
		SDL_Color red = {255, 0, 0, 255};
		SDL_Color green = {0, 255, 0, 255};
		SDL_Color blue = {0, 0, 255, 255};
		GPU_Rectangle(screen, 3*src_rect.x, 3*src_rect.y, 3*(src_rect.x + src_rect.w), 3*(src_rect.y + src_rect.h), red);
		GPU_CircleFilled(screen, 3*src_rect.x, 3*src_rect.y, 4, blue);
		GPU_CircleFilled(screen, 3*(src_rect.x + src_rect.w), 3*(src_rect.y + src_rect.h), 4, green);
		
		if(shader_index == 1)
            set_shader(p, &block);
		
		
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


