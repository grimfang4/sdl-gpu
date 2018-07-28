#ifndef _SDL_GPU_DEMO_COMMON_H__
#define _SDL_GPU_DEMO_COMMON_H__

#include "SDL_gpu.h"

#ifdef SDL_GPU_USE_SDL2

#define SDL_GetKeyState SDL_GetKeyboardState
#define KEY_UP SDL_SCANCODE_UP
#define KEY_DOWN SDL_SCANCODE_DOWN
#define KEY_LEFT SDL_SCANCODE_LEFT
#define KEY_RIGHT SDL_SCANCODE_RIGHT
#define KEY_w SDL_SCANCODE_W
#define KEY_a SDL_SCANCODE_A
#define KEY_s SDL_SCANCODE_S
#define KEY_d SDL_SCANCODE_D
#define KEY_z SDL_SCANCODE_Z
#define KEY_x SDL_SCANCODE_X
#define KEY_MINUS SDL_SCANCODE_MINUS
#define KEY_EQUALS SDL_SCANCODE_EQUALS
#define KEY_COMMA SDL_SCANCODE_COMMA
#define KEY_PERIOD SDL_SCANCODE_PERIOD

#else

#define SDL_Keymod SDLMod
#define KEY_UP SDLK_UP
#define KEY_DOWN SDLK_DOWN
#define KEY_LEFT SDLK_LEFT
#define KEY_RIGHT SDLK_RIGHT
#define KEY_w SDLK_w
#define KEY_a SDLK_a
#define KEY_s SDLK_s
#define KEY_d SDLK_d
#define KEY_z SDLK_z
#define KEY_x SDLK_x
#define KEY_MINUS SDLK_MINUS
#define KEY_EQUALS SDLK_EQUALS
#define KEY_COMMA SDLK_COMMA
#define KEY_PERIOD SDLK_PERIOD

#endif


#ifdef SDL_GPU_USE_SDL2
    #define GET_ALPHA(sdl_color) (sdl_color.a)
#else
    #define GET_ALPHA(sdl_color) (sdl_color.unused)
#endif

#endif
