
#define STB_IMAGE_WRITE_IMPLEMENTATION

// stbi_write_png_to_mem() returns a malloc'ed array,
// so let's do the same thing as done with stb_image.c so it is more compatible with SDL's allocator.
#include "SDL.h"
#include <stdlib.h>
#undef malloc
#undef free
#define malloc SDL_malloc
#define free SDL_free


#include "stb_image_write.h"
