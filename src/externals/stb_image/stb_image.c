
#define STB_IMAGE_IMPLEMENTATION

// Make stb-image use SDL's allocator so we can let SDL handle the memory that stb allocates.
// To do this, we'll redefine malloc and free.
#include "SDL.h"
#include <stdlib.h>
#undef malloc
#undef free
#define malloc SDL_malloc
#define free SDL_free

#include "stb_image.h"
