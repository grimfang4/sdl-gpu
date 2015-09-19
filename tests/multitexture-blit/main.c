#include "SDL.h"
#include "SDL_gpu.h"
#include "common.h"

#define VERT \
"#version 130\n" \
"in vec2 RPG_Vertex;\n" \
"in vec2 RPG_TexCoord;\n" \
"in vec2 RPG_TexCoord2;\n" \
"uniform mat4 RPG_ModelViewProjectionMatrix;\n" \
"out vec2 texCoord;\n" \
"out vec2 texCoord2;\n" \
"void main(void)\n" \
"{\n" \
"    texCoord = vec2(RPG_TexCoord);\n" \
"    texCoord2 = vec2(RPG_TexCoord2);\n" \
"    gl_Position = RPG_ModelViewProjectionMatrix * vec4(RPG_Vertex, 0.0, 1.0);\n" \
"}"

#define FRAG \
"#version 110\n" \
"uniform sampler2D texAlpha;\n" \
"uniform sampler2D texRGB;\n" \
"varying vec2 texCoord;\n" \
"varying vec2 texCoord2;\n" \
"void main()\n" \
"{\n" \
"    float alpha = texture2D(texAlpha, texCoord).a;\n" \
"    vec3 rgb = texture2D(texRGB, texCoord2).rgb;\n" \
"    gl_FragColor = vec4(rgb, alpha);\n" \
"}"

int main(int argc, char* argv[])
{
	GPU_Target* screen;
	
	screen = initialize_demo(argc, argv, 800, 600);
	if(screen == NULL)
		return 1;
	
	{
		Uint32 startTime;
		long frameCount;
		Uint8 done;
		SDL_Event event;
        
        
        float dt = 0.010f;
        
        #define MAX_SPRITES 50
        int numSprites = 1;
        
        float x[MAX_SPRITES];
        float y[MAX_SPRITES];
        float velx[MAX_SPRITES];
        float vely[MAX_SPRITES];
        int i;
        
        GPU_Image* image = GPU_LoadImage("data/test3.png");
        if(image == NULL)
            return 2;
        GPU_Image* image2 = GPU_LoadImage("data/test4.bmp");
        if(image2 == NULL)
            return 3;
        
        GPU_Image* images[2];
        images[0] = image;
        images[1] = image2;
        
        GPU_Rect rects[2];
        rects[0] = GPU_MakeRect(0, 0, image->w, image->h);
        rects[1] = GPU_MakeRect(0, 0, image2->w, image2->h);
        
        Uint32 vert = GPU_CompileShader(GPU_VERTEX_SHADER, VERT);
        Uint32 frag = GPU_CompileShader(GPU_FRAGMENT_SHADER, FRAG);
        
        Uint32 prog = GPU_LinkShaders(vert, frag);
        GPU_ShaderBlock block = GPU_LoadShaderBlock(prog, "RPG_Vertex", "RPG_TexCoord", NULL, "RPG_ModelViewProjectionMatrix");
        GPU_ActivateShaderProgram(prog, &block);
        
        char* samplers[2] = {"texAlpha", "texRGB"};
        char* texcoords[2] = {"RPG_TexCoord", "RPG_TexCoord2"};
        GPU_MultitextureBlock block2 = GPU_LoadMultitextureBlock(2, samplers, texcoords);
        GPU_SetMultitextureBlock(&block2);
        
        GPU_SetSnapMode(image, GPU_SNAP_NONE);
        
        for(i = 0; i < MAX_SPRITES; i++)
        {
            x[i] = rand()%screen->w;
            y[i] = rand()%screen->h;
            velx[i] = 10 + rand()%screen->w/10;
            vely[i] = 10 + rand()%screen->h/10;
        }
        
        
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
                        if(numSprites < MAX_SPRITES)
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
                GPU_MultitextureBlit(images, rects, screen, x[i], y[i]);
            }
            
            GPU_Flip(screen);
            
            frameCount++;
            if(frameCount%500 == 0)
                printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        }
        
        printf("Average FPS: %.2f\n", 1000.0f*frameCount/(SDL_GetTicks() - startTime));
        
        GPU_FreeImage(image);
	}
	
	GPU_Quit();
	
	return 0;
}


