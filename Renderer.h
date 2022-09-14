#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>


// asmaloney on Cherno's error handling in OpenGL,
// GLCallR uses a lambda, capturing the return using "auto", and then returning after the check
#define DEBUG 1
// wrappers for opengl error checking
#define ASSERT(x) if (!(x)) __debugbreak(); // MSVC compiler intrinsic

#ifdef DEBUG
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#define GLCallR(x)[&](){\
    GLClearError();\
    auto retVal = x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))\
    return retVal;\
    }()
#else
#define GLCallR(x) x
#define GLCall(x) x
#endif


void GLClearError();

bool GLLogCall(const char* function, const char* file, int line);