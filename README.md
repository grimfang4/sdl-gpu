SDL_gpu, a library for making hardware-accelerated 2D graphics easy.
by Jonathan Dearborn

[![Build Status](https://travis-ci.org/grimfang4/sdl-gpu.svg?branch=master)](https://travis-ci.org/grimfang4/sdl-gpu)

SDL_gpu is licensed under the terms of the MIT License.
See LICENSE.txt for details of the usage license granted to you for this code.

FEATURES
========

* High performance (it automatically collects and submits batches instead of separate draw commands for each sprite and redundant state changes)
* Shader API
* Arbitrary geometry rendering (triangles)
* Can be integrated with explicit OpenGL calls (mixed 2D and 3D)
* Full blend state control
* Built-in primitive shapes (points, lines, tris, rects, ellipses, polygons, even arcs)
* Uses a style familiar to SDL 1.2 users
* Compatible with either SDL 1.2 or 2.0
* Loads BMP, TGA, and PNG files via stb-image
* Rotates and scales about the center of images, making reasoning about the resulting corner coordinates more obvious (adjustable via anchor settings)


HELP OUT
========

SDL_gpu is free and open source!  You can help either by contributing a pull request, filling out a bug report, sending an email, or give me a chance to put more time into it by donating:

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=3WJCLJ3P4BV9A)

Anything you can do to help is really appreciated!


LATEST SOURCE
=============

SDL_gpu is hosted on Github (https://github.com/grimfang4/sdl-gpu).  You can check out the latest version of the source code with Git:
```
git clone https://github.com/grimfang4/sdl-gpu.git sdl-gpu
```


DEPENDENCIES
============

```
SDL 1.2 or SDL 2.0 (www.libsdl.org)
A rendering backend
	Currently implemented:
		OpenGL 1.1, 2.0, 3.0, 4.0
		OpenGL ES 1.1, 2.0, 3.0
```


BUILDING
========

You can see automated build status at the project page on Travis CI:

[![Build Status](https://travis-ci.org/grimfang4/sdl-gpu.svg?branch=master)](https://travis-ci.org/grimfang4/sdl-gpu)

Automated builds will soon be available on the Github.io page.

SDL_gpu uses CMake (www.cmake.org) to coordinate the library build process.  CMake is available as a GUI program or on the command line.

For Linux/UNIX systems, run CMake in the base directory:
```
cmake -G "Unix Makefiles"
make
sudo make install
```

For Linux/UNIX systems, changing the default installation directory can be done like so:
```
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/usr
```

For Windows systems, you can use cmake-gui and select appropriate options in there (warning: cmake-gui is messy!).


INCLUDING / LINKING
===================

Add the include for SDL_gpu.h to your sources.  Link to SDL2_gpu (libSDL2_gpu.a) or SDL_gpu (if you use the old SDL 1.2).


FULL DOCUMENTATION
==================

Documentation is automatically generated with Doxygen (http://sourceforge.net/projects/doxygen).

Pre-generated documentation can be found on the Github.io page:
https://grimfang4.github.io/sdl-gpu/


CONVERSION
==========

SDL_gpu can be used to replace the SDL_Render subsystem of SDL2.  SDL_gpu uses GPU_Target to represent a render target (a render destination, e.g. the screen) instead of an SDL_Renderer object.  SDL_gpu also uses GPU_Image as a texture container (a render source) instead of SDL_Texture.

Here is a list of most of the comparable functions:

```
SDL_CreateWindow() : Either use GPU_SetInitWindow() or replace with GPU_Init()
SDL_CreateRenderer() : GPU_Init()
SDL_LoadBMP() : GPU_LoadImage() or GPU_LoadSurface()
SDL_CreateTextureFromSurface() : GPU_CopyImageFromSurface()
SDL_SetRenderDrawColor() : Pass color into rendering function (e.g. GPU_ClearRGBA(), GPU_Line())
SDL_RenderClear() : GPU_Clear(), GPU_ClearRGBA()
SDL_QueryTexture() : image->w, image->h
SDL_RenderCopy() : GPU_Blit() or GPU_BlitRect()
SDL_RenderPresent() : GPU_Flip()
SDL_DestroyTexture() : GPU_FreeImage()
SDL_DestroyRenderer() : GPU_FreeTarget() (but don't free the screen target yourself)

SDL_RenderDrawPoint() : GPU_Pixel()
SDL_RenderDrawLine() : GPU_Line()
SDL_RenderDrawRect() : GPU_Rectangle()
SDL_RenderFillRect() : GPU_RectangleFilled()
SDL_RenderCopyEx() : GPU_BlitRotate() or GPU_BlitScale() or GPU_BlitTransform()
SDL_SetRenderDrawBlendMode() : GPU_SetShapeBlendMode()
SDL_SetTextureBlendMode() : GPU_SetBlendMode()
SDL_SetTextureColorMod() : GPU_SetRGBA() or GPU_SetColor()
SDL_SetTextureAlphaMod() : GPU_SetRGBA() or GPU_SetColor()
SDL_UpdateTexture() : GPU_UpdateImage() or GPU_UpdateImageBytes()
SDL_RenderSetClipRect() : GPU_SetClip() or GPU_SetClipRect()
SDL_RenderReadPixels() : GPU_CopySurfaceFromTarget() or GPU_CopySurfaceFromImage()
SDL_RenderSetViewport() : GPU_SetViewport()
SDL_SetRenderTarget() : GPU_LoadTarget()
```

Some SDL functions use a rectangular region passed as an SDL_Rect.  SDL_gpu uses floating point coordinates for subpixel precision, so you may have to use GPU_Rect for some SDL_gpu functions.

