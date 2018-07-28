#ifndef _DEMO_FONT_H__
#define _DEMO_FONT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "SDL_gpu.h"

typedef struct DemoFont
{
    GPU_Image* image;
    
    char* buffer;
    int charPos[256];
    Uint16 charWidth[256];
} DemoFont;

DemoFont* FONT_Alloc(SDL_Surface* source_surface);
void FONT_Free(DemoFont* font);

void FONT_Draw(DemoFont* font, GPU_Target* target, float x, float y, const char* formatted_text, ...);

#ifdef __cplusplus
}
#endif

#endif
