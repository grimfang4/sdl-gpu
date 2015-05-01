
# Required BLURRR_SDK_ROOT to be passed in or env
IF(NOT BLURRR_ROOT)
	IF(NOT DEFINED ENV{BLURRR_ROOT}) 
		MESSAGE(AUTHOR_WARNING "BLURRR_SDK_ROOT not passed in") 
	ELSE()
		SET(BLURRR_ROOT $ENV{BLURRR_ROOT})
	ENDIF()
ENDIF()

# Probably want to use this form
IF(NOT BLURRR_SDK_PATH)
	IF(DEFINED ENV{BLURRR_SDK_PATH})
		SET(BLURRR_SDK_PATH $ENV{BLURRR_SDK_PATH})
	ENDIF()
MESSAGE("ENV BLURRR_SDK_PATH $ENV{BLURRR_SDK_PATH}")
	
ENDIF()	
MESSAGE("BLURRR_SDK_PATH ${BLURRR_SDK_PATH}")

# CMake doesn't know how to handle multiple architectures.
# So we must manually fake it by running CMake multiple times, each time specifying the architecture.
IF(NOT ANDROID_ABI)
	MESSAGE(FATAL_ERROR "ANDROID_ABI not passed in") 
ENDIF()
IF(NOT BLURRR_CMAKE_ANDROID_REAL_BINARY_DIR)
	MESSAGE(FATAL_ERROR "BLURRR_CMAKE_ANDROID_REAL_BINARY_DIR not passed in") 
ENDIF()


# Allow for the possibility that the user defined CMAKE_LIBRARY_PATH, etc. and the normal CMake stuff will work without the initial cache.
IF(BLURRR_ROOT OR BLURRR_SDK_PATH)
	IF(NOT BLURRR_SDK_PATH)
		# Convenience variables
		SET(BLURRR_SDK_PATH "${BLURRR_ROOT}/Libraries/Android/SDK/C" CACHE INTERNAL "Blurrr SDK path. (Read-only)")
	ENDIF()

	# Note: The layout is a little different than the other platforms.
	# This is because I'm trying to conform to the official NDK_MODULE_PATH external module system.
	# The Android system has a hard limitation in that it requires environmental variables
	# which Android Studio refuses to support,
	# and the other hard limitation in that it can't handle any modules in paths with spaces.
	# This is one reason I'm not directly using it.
	# But for advanced use cases that need to link to my modules that don't use this 
	# build system, it is really handy to comply with this official mechanism.

	# Because we are generating CMake multiple times in their own little subdirectories,
	# we need an anchor back to the real build directory root so the build system can interact with 
	# shared/common stuff like resources.
	SET(BLURRR_CMAKE_ANDROID_REAL_BINARY_DIR "${BLURRR_CMAKE_ANDROID_REAL_BINARY_DIR}" CACHE INTERNAL "Real CMake binary directory. (Read-only)")


	#	SET(BLURRR_INCLUDE_PATH "${BLURRR_SDK_PATH}/include" CACHE STRING "Blurrr SDK include path. (Read-only)")
	#	SET(BLURRR_LIBRARY_PATH "${BLURRR_SDK_PATH}/libs/${BLURRR_ANDROID_ARCH}" CACHE STRING "Blurrr SDK library path. (Read-only)")
	
#	SET(ALMIXER_INCLUDE_DIR "${BLURRR_SDK_PATH}/ALmixer/include" CACHE STRING "ALmixer include directory")
#	SET(ALMIXER_LIBRARY "${BLURRR_SDK_PATH}/ALmixer/libs/${ANDROID_ABI}/libALmixer.so" CACHE STRING "ALmixer library")

#	SET(OPENAL_INCLUDE_DIR "${BLURRR_SDK_PATH}/openal-soft/jni/OpenAL/include/AL" CACHE STRING "OpenAL include directory")
#	SET(OPENAL_LIBRARY "${BLURRR_SDK_PATH}/openal-soft/libs/${ANDROID_ABI}/libopenal.so" CACHE STRING "OpenAL library")

#	SET(CHIPMUNK_INCLUDE_DIR "${BLURRR_SDK_PATH}/chipmunk/include/chipmunk" CACHE STRING "Chipmunk include directory")
#	SET(CHIPMUNK_LIBRARY "${BLURRR_SDK_PATH}/chipmunk/libs/${ANDROID_ABI}/libchipmunk.so" CACHE STRING "Chipmunk library")

	SET(SDL_INCLUDE_DIR "${BLURRR_SDK_PATH}/SDL2/include" CACHE STRING "SDL include directory")
	SET(SDL_LIBRARY "${BLURRR_SDK_PATH}/SDL2/libs/${ANDROID_ABI}/libSDL2.so" CACHE STRING "SDL library")
	SET(SDL_LIBRARY_MAIN "${BLURRR_SDK_PATH}/SDL2/libs/${ANDROID_ABI}/libSDL2main.so" CACHE STRING "SDLmain library")
	
	SET(SDL2_INCLUDE_DIR "${BLURRR_SDK_PATH}/SDL2/include" CACHE STRING "SDL include directory")
	SET(SDL2_LIBRARY "${BLURRR_SDK_PATH}/SDL2/libs/${ANDROID_ABI}/libSDL2.so" CACHE STRING "SDL library")
	SET(SDL2_LIBRARY_MAIN "${BLURRR_SDK_PATH}/SDL2/libs/${ANDROID_ABI}/libSDL2main.so" CACHE STRING "SDLmain library")



#	SET(LUA_INCLUDE_DIR "${BLURRR_SDK_PATH}/lua/include" CACHE STRING "Lua include directory")
#	SET(LUA_LIBRARY "${BLURRR_SDK_PATH}/lua/libs/${ANDROID_ABI}/liblua.so" CACHE STRING "Lua library")



ENDIF(BLURRR_ROOT OR BLURRR_SDK_PATH)
