#include "SDL.h"
#include "SDL_gpu.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>


// Loads a shader and prepends version/compatibility info before compiling it.
// Normally, you can just use GPU_LoadShader() for shader source files or GPU_CompileShader() for strings.
// However, some hardware (certain ATI/AMD cards) does not let you put non-#version preprocessing at the top of the file.
// Therefore, I need to prepend the version info here so I can support both GLSL and GLSLES with one shader file.
Uint32 load_shader(GPU_ShaderEnum shader_type, const char* filename)
{
    SDL_RWops* rwops;
    Uint32 shader;
    char* source;
    int header_size, file_size;
    const char* header = "";
    GPU_Renderer* renderer = GPU_GetCurrentRenderer();
    
    // Open file
    rwops = SDL_RWFromFile(filename, "rb");
    if(rwops == NULL)
    {
        GPU_PushErrorCode("load_shader", GPU_ERROR_FILE_NOT_FOUND, "Shader file \"%s\" not found", filename);
        return 0;
    }
    
    // Get file size
    file_size = SDL_RWseek(rwops, 0, SEEK_END);
    SDL_RWseek(rwops, 0, SEEK_SET);
    
    // Get size from header
    if(renderer->shader_language == GPU_LANGUAGE_GLSL)
    {
        if(renderer->max_shader_version >= 120)
            header = "#version 120\n";
        else
            header = "#version 110\n";  // Maybe this is good enough?
    }
    else if(renderer->shader_language == GPU_LANGUAGE_GLSLES)
        header = "#version 100\nprecision mediump int;\nprecision mediump float;\n";
    
    header_size = strlen(header);
    
    // Allocate source buffer
    source = (char*)malloc(sizeof(char)*(header_size + file_size + 1));
    
    // Prepend header
    strcpy(source, header);
    
    // Read in source code
    SDL_RWread(rwops, source + strlen(source), 1, file_size);
    source[header_size + file_size] = '\0';
    
    // Compile the shader
    shader = GPU_CompileShader(shader_type, source);
    
    // Clean up
    free(source);
    SDL_RWclose(rwops);
    
    return shader;
}

GPU_ShaderBlock load_shader_program(Uint32* p, const char* vertex_shader_file, const char* fragment_shader_file)
{
    Uint32 v, f;
    v = load_shader(GPU_VERTEX_SHADER, vertex_shader_file);
    
    if(!v)
        GPU_LogError("Failed to load vertex shader (%s): %s\n", vertex_shader_file, GPU_GetShaderMessage());
    
    f = load_shader(GPU_FRAGMENT_SHADER, fragment_shader_file);
    
    if(!f)
        GPU_LogError("Failed to load fragment shader (%s): %s\n", fragment_shader_file, GPU_GetShaderMessage());
    
    *p = GPU_LinkShaders(v, f);
    
    if(!*p)
    {
		GPU_ShaderBlock b = {-1, -1, -1, -1};
        GPU_LogError("Failed to link shader program (%s + %s): %s\n", vertex_shader_file, fragment_shader_file, GPU_GetShaderMessage());
        return b;
    }
    
	{
		GPU_ShaderBlock block = GPU_LoadShaderBlock(*p, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "gpu_ModelViewProjectionMatrix");
		GPU_ActivateShaderProgram(*p, &block);

		return block;
	}
}

void free_shader(Uint32 p)
{
    GPU_FreeShaderProgram(p);
}



// Not going to change dynamically in this demo, so can be done once.

void prepare_mask_shader(Uint32 shader, GPU_Image* image, GPU_Image* mask_image)
{
    if(image != NULL)
    {
        GPU_SetUniformf(GPU_GetUniformLocation(shader, "resolution_x"), image->w);
        GPU_SetUniformf(GPU_GetUniformLocation(shader, "resolution_y"), image->h);
    }
    if(mask_image != NULL)
    {
        GPU_SetUniformf(GPU_GetUniformLocation(shader, "mask_resolution_x"), mask_image->w);
        GPU_SetUniformf(GPU_GetUniformLocation(shader, "mask_resolution_y"), mask_image->h);
    }
    
    GPU_SetUniformf(GPU_GetUniformLocation(shader, "offset_x"), 0.0f);
    GPU_SetUniformf(GPU_GetUniformLocation(shader, "offset_y"), 0.0f);
    
    GPU_SetShaderImage(mask_image, GPU_GetUniformLocation(shader, "mask_tex"), 1);
}

void prepare_marching_ants_shader(Uint32 shader, GPU_Target* screen, GPU_Image* image)
{
    if(image != NULL)
    {
        GPU_SetUniformf(GPU_GetUniformLocation(shader, "resolution_x"), image->w);
        GPU_SetUniformf(GPU_GetUniformLocation(shader, "resolution_y"), image->h);
    }
    GPU_SetUniformf(GPU_GetUniformLocation(shader, "screen_w"), screen->w);
    GPU_SetUniformf(GPU_GetUniformLocation(shader, "screen_h"), screen->h);
    GPU_SetUniformf(GPU_GetUniformLocation(shader, "zoom"), 1.0f);
}



// Will change every frame

void update_color_shader(float r, float g, float b, float a, int color_loc)
{
    float fcolor[4] = {r, g, b, a};
    GPU_SetUniformfv(color_loc, 4, 1, fcolor);
}

void update_vertex_color_shader(float t, float vertex_colors[24])
{
    for(int i = 0; i < 6; ++i)
    {
        vertex_colors[i] = 0.5f + 0.5f*(1 + sin(t + (i)))/2;
        vertex_colors[i+1] = 0.5f + 0.5f*(1 + sin(t + (i+1)))/2;
        vertex_colors[i+2] = 0.5f + 0.5f*(1 + sin(t + (i+2)))/2;
        vertex_colors[i+3] = 1.0f;
    }
}

void update_marching_ants_shader(float t, int time_loc)
{
    GPU_SetUniformf(time_loc, t);
}




void main_loop(GPU_Target* screen)
{
    GPU_Image* image = GPU_LoadImage("data/test3.png");
    
    Uint32 color_shader;
    GPU_ShaderBlock color_block = load_shader_program(&color_shader, "data/shaders/common.vert", "data/shaders/color.frag");
    int color_loc = GPU_GetUniformLocation(color_shader, "myColor");
    
    // Disabled for now because it seems to be a bit buggy
    // Having problems with this...  It seems to take 1 color for each vertex, even shared ones (indices, so not 16 floats but 24).
    // It also seems not to be working properly anyhow.  Some vertices are still dark.
    /*Uint32 vertex_color_shader;
    GPU_ShaderBlock vertex_color_block = load_shader_program(&vertex_color_shader, "data/shaders/common.vert", "data/shaders/common.frag");
    vertex_color_block.color_loc = -1;  // Disable so it doesn't conflict with custom attribute values
    int vertex_color_loc = GPU_GetAttributeLocation(vertex_color_shader, "gpu_Color");
    GPU_AttributeFormat vertex_color_attribute_format = GPU_MakeAttributeFormat(4, GPU_TYPE_FLOAT, 0, 4*sizeof(float), 0);
    float vertex_colors[24];
    GPU_Attribute vertex_color_attribute = GPU_MakeAttribute(vertex_color_loc, vertex_colors, vertex_color_attribute_format);*/
    
    Uint32 mask_shader;
    GPU_ShaderBlock mask_block = load_shader_program(&mask_shader, "data/shaders/common.vert", "data/shaders/alpha_mask.frag");
    GPU_Image* mask_image = GPU_LoadImage("data/test2.png");
    prepare_mask_shader(mask_shader, image, mask_image);
    
    Uint32 marching_ants_shader;
    GPU_ShaderBlock marching_ants_block = load_shader_program(&marching_ants_shader, "data/shaders/common.vert", "data/shaders/marching_ants.frag");
    prepare_marching_ants_shader(marching_ants_shader, screen, image);
    int marching_ants_time_loc = GPU_GetUniformLocation(marching_ants_shader, "time");
    
    // TODO: Add a real vertex shader
    
    
    float t;
    
    Uint8 done;
    SDL_Event event;
    
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
        
        t = SDL_GetTicks()/1000.0f;
        
        GPU_Clear(screen);
        
        // Color mod
        GPU_ActivateShaderProgram(color_shader, &color_block);
        update_color_shader((1+sin(t))/2, (1+sin(t+1))/2, (1+sin(t+2))/2, 1.0f, color_loc);
        GPU_Blit(image, NULL, screen, screen->w/4, screen->h/4);
        
        // Per-vertex colors
        // Disabled for now because it seems to be a bit buggy
        /*GPU_ActivateShaderProgram(vertex_color_shader, &vertex_color_block);
        update_vertex_color_shader(t, vertex_colors);
        GPU_SetAttributeSource(6, vertex_color_attribute);
        GPU_Blit(image, NULL, screen, 3*screen->w/4, screen->h/4);
        GPU_SetAttributeSource(0, vertex_color_attribute);*/
        
        // Alpha mask
        GPU_ActivateShaderProgram(mask_shader, &mask_block);
        GPU_Blit(image, NULL, screen, screen->w/4, 3*screen->h/4);
        
        // Marching ants
        GPU_ActivateShaderProgram(0, NULL);
        GPU_Blit(image, NULL, screen, 3*screen->w/4, 3*screen->h/4);
        GPU_ActivateShaderProgram(marching_ants_shader, &marching_ants_block);
        update_marching_ants_shader(t, marching_ants_time_loc);
        GPU_Blit(image, NULL, screen, 3*screen->w/4, 3*screen->h/4);
        
        GPU_ActivateShaderProgram(0, NULL);
        
        GPU_Flip(screen);
    }
    
    GPU_FreeImage(mask_image);
    GPU_FreeImage(image);
    
    free_shader(color_shader);
    free_shader(mask_shader);
    free_shader(marching_ants_shader);
}

int main(int argc, char* argv[])
{
	GPU_Target* screen;
	
	screen = GPU_Init(800, 600, GPU_DEFAULT_INIT_FLAGS);
	if(screen == NULL)
		return -1;
	
	main_loop(screen);
	
	GPU_Quit();
	
	return 0;
}


