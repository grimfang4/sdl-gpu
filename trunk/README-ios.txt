
Building SDL2_gpu for iOS
-------------------------

Building for iOS takes a little extra effort.  As of writing, CMake does not include explicit support for the iOS toolchain, so we have to use a toolchain file to specify the correct programs for CMake to use.

SDL_gpu includes a version of the ios-cmake toolchain file.

The FindSDL2.cmake script also does not handle locating the appropriate binaries for iOS (the default SDL framework does not include iOS).

You will need to build the SDL2 library for iOS first.


Here's an example command line invokation of CMake for SDL_gpu.  You may need to adjust the paths yourself.

mkdir ios-build
cd ios-build
cmake -DCMAKE_TOOLCHAIN_FILE=../scripts/ios-cmake/toolchain/iOS.cmake -DCMAKE_IOS_SDK_ROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS7.1.sdk -DSDL2_LIBRARY=../../SDL2/lib/ios/libSDL2.a -DSDL2_INCLUDE_DIR=../../SDL2/include/SDL2 .. -G "Xcode"

The settings in CMakeLists.txt will automatically pick the OpenGL ES renderers for you.  It will also disable building of the demos and tools.

If you do not get a message saying that the iOS default settings are being used, then the path to ios-cmake may be incorrect.  You will have to delete the CMake cache files before trying again.