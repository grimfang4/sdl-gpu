#include "demo-font.h"

static Uint32 _GetPixel(SDL_Surface *Surface, int x, int y)
{
    Uint8* bits;
    Uint32 bpp;

    if(x < 0 || x >= Surface->w)
        return 0;  // Best I could do for errors

    bpp = Surface->format->BytesPerPixel;
    bits = ((Uint8*)Surface->pixels) + y*Surface->pitch + x*bpp;

    Uint8 r, g, b;
    switch (bpp)
    {
        case 1:
            return *((Uint8*)Surface->pixels + y * Surface->pitch + x);
            break;
        case 2:
            return *((Uint16*)Surface->pixels + y * Surface->pitch/2 + x);
            break;
        case 3:
            // Endian-correct, but slower
            r = *((bits)+Surface->format->Rshift/8);
            g = *((bits)+Surface->format->Gshift/8);
            b = *((bits)+Surface->format->Bshift/8);
            return SDL_MapRGB(Surface->format, r, g, b);
            break;
        case 4:
            return *((Uint32*)Surface->pixels + y * Surface->pitch/4 + x);
            break;
    }

    return 0;  // Best I could do for errors
}

DemoFont* FONT_Alloc(SDL_Surface* source_surface)
{
    if(source_surface == NULL)
    {
        printf("ERROR: DemoFont given a NULL surface\n");
        return NULL;
    }
    
    DemoFont* font = (DemoFont*)malloc(sizeof(DemoFont));
    if(font == NULL)
        return NULL;
    
    font->buffer = (char*)malloc(sizeof(char)*1024);
    
    int x = 1, i = 0;
    
    memset(font->charPos, 0, sizeof(int)*256);
    memset(font->charWidth, 0, sizeof(Uint16)*256);

    SDL_LockSurface(source_surface);

    Uint32 pixel = SDL_MapRGB(source_surface->format, 255, 0, 255); // pink pixel
    
    // Get the character positions and widths
    while(x < source_surface->w)
    {
        if(_GetPixel(source_surface, x, 0) != pixel)
        {
            font->charPos[i] = x;
            font->charWidth[i] = x;
            while(x < source_surface->w && _GetPixel(source_surface, x, 0) != pixel)
                x++;
            font->charWidth[i] = x - font->charWidth[i];
            i++;
        }

        x++;
    }


    pixel = _GetPixel(source_surface, 0, source_surface->h - 1);
    
    Uint8 uses_alpha;
    #ifdef SDL_GPU_USE_SDL2
    SDL_BlendMode blendMode;
    SDL_GetSurfaceBlendMode(source_surface, &blendMode);
    uses_alpha = (blendMode != SDL_BLENDMODE_NONE);
    #else
    uses_alpha = ((source_surface->flags & SDL_SRCALPHA) == SDL_SRCALPHA);
    #endif

    if(!uses_alpha)
    {
        pixel = _GetPixel(source_surface, 0, source_surface->h - 1);
        SDL_UnlockSurface(source_surface);
        #ifdef SDL_GPU_USE_SDL2
        SDL_SetColorKey(source_surface, SDL_TRUE, pixel);
        #else
        SDL_SetColorKey(source_surface, SDL_SRCCOLORKEY, pixel);
        #endif
    }
    else
        SDL_UnlockSurface(source_surface);
    
    font->image = GPU_CopyImageFromSurface(source_surface);
    if(font->image == NULL)
    {
        FONT_Free(font);
        return NULL;
    }
    
    return font;
}

void FONT_Free(DemoFont* font)
{
    GPU_FreeImage(font->image);
    free(font->buffer);
    free(font);
}


void FONT_Draw(DemoFont* font, GPU_Target* target, float x, float y, const char* formatted_text, ...)
{
    if(font == NULL || formatted_text == NULL || font->image == NULL)
        return;

    va_list lst;
    va_start(lst, formatted_text);
    vsnprintf(font->buffer, 1024, formatted_text, lst);
    va_end(lst);
    
    
    char* text = font->buffer;
    const char* c = text;
    unsigned char num;
    GPU_Rect srcRect;
    
    srcRect.y = 0;
    srcRect.h = font->image->h;
    
    int letterSpacing = 1;
    
    for(; *c != '\0'; c++)
    {
        if (*c == ' ')
        {
            x += font->charWidth[0] + letterSpacing;
            continue;
        }
        unsigned char ctest = (unsigned char)(*c);
        // Skip bad characters
        if(ctest < 33 || (ctest > 126 && ctest < 161))
            continue;
        
        num = ctest - 33;  // Get array index
        if(num > 126) // shift the extended characters down to the correct index
            num -= 34;
        
        srcRect.x = font->charPos[num];
        srcRect.w = font->charWidth[num];
        GPU_Blit(font->image, &srcRect, target, x, y);
        
        x += srcRect.w + letterSpacing;
    }
}
