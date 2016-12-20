#pragma once

#if defined(_WIN32)
	#include <starbase/windows.hpp>
#endif

#if defined(_WIN32) \
    || defined(__linux) \
    || defined(__FreeBSD__) \
    || defined(__OpenBSD__) \
    || defined(__NetBSD__) \
    || defined(__DragonFly__)
	#define STARBASE_USING_GLEW 1
	#include <GL/glew.h>
#elif defined(__APPLE__)
    #include <OpenGL/gl.h>
#elif defined(EMSCRIPTEN)
    #include <GLES2/gl2.h>
#elif defined(__ANDROID__)
    #include <GLES2/gl2.h> 
#else
    #error Sorry, unsupported platform
#endif
