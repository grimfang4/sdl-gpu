
#define STB_IMAGE_WRITE_IMPLEMENTATION

// NOTE: stbi_write_png_to_mem() returns a malloc'ed array,
// This might be bad for making sure the allocator and deallocator match
// and for crossing DLL boundaries with incompatible C runtimes.


#include "stb_image_write.h"
