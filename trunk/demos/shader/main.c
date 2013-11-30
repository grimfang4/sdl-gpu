#include "SDL.h"
#include "SDL_gpu.h"
#include "SDL_gpu_OpenGL.h"

void printRenderers(void)
{
	const char* renderers[GPU_GetNumRegisteredRenderers()];
	GPU_GetRegisteredRendererList(renderers);
	
	printf("Available renderers:\n");
	int i;
	for(i = 0; i < GPU_GetNumRegisteredRenderers(); i++)
	{
		printf("%d) %s\n", i+1, renderers[i]);
	}
}

static int read_string_rw(SDL_RWops* rwops, char* result)
{
   if(rwops == NULL)
        return 0;
    
    size_t size = 100;
    long total = 0;
    long len = 0;
    while((len = SDL_RWread(rwops, &result[total], 1, size)) > 0)
    {
        total += len;
    }
    
    SDL_RWclose(rwops);
    
    result[total] = '\0';
    
    return total;
}

static char shader_message[256];

#define GPU_VERTEX_SHADER 0
#define GPU_FRAGMENT_SHADER 1
#define GPU_PIXEL_SHADER 1
#define GPU_GEOMETRY_SHADER 2

Uint32 GPU_CompileShader_RW(int shader_type, SDL_RWops* shader_source)
{
    // Create the proper new shader object
    GLuint shader_object = 0;
    switch(shader_type)
    {
    case GPU_VERTEX_SHADER:
        shader_object = glCreateShader(GL_VERTEX_SHADER);
        break;
    case GPU_FRAGMENT_SHADER:
        shader_object = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    case GPU_GEOMETRY_SHADER:
        shader_object = glCreateShader(GL_GEOMETRY_SHADER);
        break;
    }
    
    if(shader_object == 0)
    {
        GPU_LogError("Failed to create new shader object.\n");
        snprintf(shader_message, 256, "Failed to create new shader object.\n");
        return 0;
    }
    
    // Read in the shader source code
    char* source_string = (char*)malloc(1000);
    int result = read_string_rw(shader_source, source_string);
    if(!result)
    {
        GPU_LogError("Failed to read shader source.\n");
        snprintf(shader_message, 256, "Failed to read shader source.\n");
        free(source_string);
        glDeleteShader(shader_object);
        return 0;
    }
   
    const char* ss = source_string;
	glShaderSource(shader_object, 1, &ss, NULL);
    free(source_string);
    
    // Compile the shader source
    GLint compiled;
	
	glCompileShader(shader_object);
	
    glGetShaderiv(shader_object, GL_COMPILE_STATUS, &compiled);
    if(!compiled)
    {
        GPU_LogError("Failed to compile shader source.\n");
        glGetShaderInfoLog(shader_object, 256, NULL, shader_message);
        glDeleteShader(shader_object);
        return 0;
    }
    
    return shader_object;
}

Uint32 GPU_LoadShader(int shader_type, const char* filename)
{
    SDL_RWops* rwops = SDL_RWFromFile(filename, "r");
    Uint32 result = GPU_CompileShader_RW(shader_type, rwops);
    SDL_RWclose(rwops);
    return result;
}

Uint32 GPU_CompileShader(int shader_type, const char* shader_source)
{
    SDL_RWops* rwops = SDL_RWFromConstMem(shader_source, strlen(shader_source)+1);
    Uint32 result = GPU_CompileShader_RW(shader_type, rwops);
    SDL_RWclose(rwops);
    return result;
}

Uint32 GPU_LinkShaderProgram(Uint32 program_object)
{
	glLinkProgram(program_object);
	
	int linked;
	glGetProgramiv(program_object, GL_LINK_STATUS, &linked);
	
	if(!linked)
    {
        GPU_LogError("Failed to link shader program.\n");
        glGetProgramInfoLog(program_object, 256, NULL, shader_message);
        glDeleteProgram(program_object);
        return 0;
    }
    
	return program_object;
}

Uint32 GPU_LinkShaders(Uint32 shader_object1, Uint32 shader_object2)
{
    GLuint p = glCreateProgram();

	glAttachShader(p, shader_object1);
	glAttachShader(p, shader_object2);
	
	return GPU_LinkShaderProgram(p);
}

Uint32 GPU_LinkShaders3(Uint32 shader_object1, Uint32 shader_object2, Uint32 shader_object3)
{
    GLuint p = glCreateProgram();

	glAttachShader(p, shader_object1);
	glAttachShader(p, shader_object2);
	glAttachShader(p, shader_object3);
	
	return GPU_LinkShaderProgram(p);
}

void GPU_FreeShader(Uint32 shader_object)
{
    glDeleteShader(shader_object);
}

void GPU_FreeShaderProgram(Uint32 program_object)
{
    glDeleteProgram(program_object);
}

void GPU_AttachShader(Uint32 program_object, Uint32 shader_object)
{
    glAttachShader(program_object, shader_object);
}

void GPU_DetachShader(Uint32 program_object, Uint32 shader_object)
{
    glDetachShader(program_object, shader_object);
}

void GPU_ActivateShaderProgram(Uint32 program_object)
{
    glUseProgram(program_object);
}

const char* GPU_GetShaderMessage(void)
{
    return shader_message;
}

int GPU_GetUniformLocation(Uint32 program_object, const char* uniform_name)
{
    return glGetUniformLocation(program_object, uniform_name);
}



void GPU_GetUniformiv(Uint32 program_object, int location, int* values)
{
    glGetUniformiv(program_object, location, values);
}

void GPU_SetUniformi(int location, int value)
{
    glUniform1i(location, value);
}

void GPU_SetUniformiv(int location, int num_elements_per_value, int num_values, int* values)
{
    switch(num_elements_per_value)
    {
        case 1:
        glUniform1iv(location, num_values, values);
        break;
        case 2:
        glUniform2iv(location, num_values, values);
        break;
        case 3:
        glUniform3iv(location, num_values, values);
        break;
        case 4:
        glUniform4iv(location, num_values, values);
        break;
    }
}


void GPU_GetUniformuiv(Uint32 program_object, int location, unsigned int* values)
{
    glGetUniformuiv(program_object, location, values);
}

void GPU_SetUniformui(int location, unsigned int value)
{
    glUniform1ui(location, value);
}

void GPU_SetUniformuiv(int location, int num_elements_per_value, int num_values, unsigned int* values)
{
    switch(num_elements_per_value)
    {
        case 1:
        glUniform1uiv(location, num_values, values);
        break;
        case 2:
        glUniform2uiv(location, num_values, values);
        break;
        case 3:
        glUniform3uiv(location, num_values, values);
        break;
        case 4:
        glUniform4uiv(location, num_values, values);
        break;
    }
}


void GPU_GetUniformfv(Uint32 program_object, int location, float* values)
{
    glGetUniformfv(program_object, location, values);
}

void GPU_SetUniformf(int location, float value)
{
    glUniform1f(location, value);
}

void GPU_SetUniformfv(int location, int num_elements_per_value, int num_values, float* values)
{
    switch(num_elements_per_value)
    {
        case 1:
        glUniform1fv(location, num_values, values);
        break;
        case 2:
        glUniform2fv(location, num_values, values);
        break;
        case 3:
        glUniform3fv(location, num_values, values);
        break;
        case 4:
        glUniform4fv(location, num_values, values);
        break;
    }
}


void load_shaders(Uint32* v, Uint32* f, Uint32* p)
{
    *v = GPU_LoadShader(GPU_VERTEX_SHADER, "shader/test.vert");
    
    if(!*v)
    {
        GPU_LogError("Failed to load vertex shader: %s\n", GPU_GetShaderMessage());
    }
    
    *f = GPU_LoadShader(GPU_FRAGMENT_SHADER, "shader/test.frag");
    
    if(!*f)
    {
        GPU_LogError("Failed to load fragment shader: %s\n", GPU_GetShaderMessage());
    }
    
    *p = GPU_LinkShaders(*v, *f);
    
    if(!*p)
    {
        GPU_LogError("Failed to link shader program: %s\n", GPU_GetShaderMessage());
        return;
    }
    
    GPU_ActivateShaderProgram(*p);
}

void free_shaders(Uint32 v, Uint32 f, Uint32 p)
{
    GPU_FreeShader(v);
    GPU_FreeShader(f);
    GPU_FreeShaderProgram(p);
}

int main(int argc, char* argv[])
{
	printRenderers();
	
	GPU_Target* screen = GPU_Init(NULL, 800, 600, 0);
	if(screen == NULL)
		return -1;
	
	printf("Using renderer: %s\n", GPU_GetCurrentRendererID());
	
	GPU_Image* image = GPU_LoadImage("data/test.bmp");
	if(image == NULL)
		return -1;
	
	Uint32 v, f, p;
	load_shaders(&v, &f, &p);
	int uloc = GPU_GetUniformLocation(p, "tex");
	GPU_SetUniformi(uloc, 0);
	
	float dt = 0.010f;
	
	Uint32 startTime = SDL_GetTicks();
	long frameCount = 0;
	
	int maxSprites = 50;
	int numSprites = 1;
	
	float x[maxSprites];
	float y[maxSprites];
	float velx[maxSprites];
	float vely[maxSprites];
	int i;
	for(i = 0; i < maxSprites; i++)
	{
		x[i] = rand()%screen->w;
		y[i] = rand()%screen->h;
		velx[i] = 10 + rand()%screen->w/10;
		vely[i] = 10 + rand()%screen->h/10;
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
				else if(event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_PLUS)
				{
					if(numSprites < maxSprites)
						numSprites++;
				}
				else if(event.key.keysym.sym == SDLK_MINUS)
				{
					if(numSprites > 0)
						numSprites--;
				}
			}
		}
		
		for(i = 0; i < numSprites; i++)
		{
			x[i] += velx[i]*dt;
			y[i] += vely[i]*dt;
			if(x[i] < 0)
			{
				x[i] = 0;
				velx[i] = -velx[i];
			}
			else if(x[i]> screen->w)
			{
				x[i] = screen->w;
				velx[i] = -velx[i];
			}
			
			if(y[i] < 0)
			{
				y[i] = 0;
				vely[i] = -vely[i];
			}
			else if(y[i]> screen->h)
			{
				y[i] = screen->h;
				vely[i] = -vely[i];
			}
		}
		
		GPU_Clear(screen);
		
		for(i = 0; i < numSprites; i++)
		{
			GPU_Blit(image, NULL, screen, x[i], y[i]);
		}
		
		GPU_Flip();
		
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


