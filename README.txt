SDL_gpu, a library for making 2D OpenGL/Direct3D as simple as 2D SDL.
by Jonathan Dearborn

SDL_gpu is licensed under the terms of the MIT License.
See LICENSE.txt for details of the usage license granted to you for this code.


=============
LATEST SOURCE
=============

SDL_gpu is hosted on Google Code (http://code.google.com/p/sdl-gpu).  You can check out the latest version of the source code with Subversion:
svn checkout http://sdl-gpu.googlecode.com/svn/trunk/ sdl-gpu


============
DEPENDENCIES
============

SDL 1.2 (www.libsdl.org)
A rendering backend (OpenGL only, at the moment)


========
BUILDING
========

SDL_gpu uses CMake (www.cmake.org) to coordinate the library compile process.  CMake is available as a GUI program or on the command line.

For Linux systems, run CMake in the base directory:
cmake -G "Unix Makefiles"
make
sudo make install

For Linux systems, changing the default installation directory can be done like so:
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/usr


=========
INCLUDING
=========

Add the include for SDL_gpu.h to your sources.  Link to SDL_gpu (libSDL_gpu.a).

The include for the shapes library is SDL_gpuShapes.h and its lib is SDL_gpuShapes (libSDL_gpuShapes.a).


=============
DOCUMENTATION
=============

See the documentation directory in the source distribution.





