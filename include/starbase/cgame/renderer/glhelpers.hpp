#pragma once

#include <starbase/gl.hpp>
#include <starbase/game/logging.hpp>
#include <starbase/game/fs/ifilesystem.hpp>

#ifndef NDEBUG
#define GLCALL(CALL)												\
	do {															\
		CALL;														\
		GLenum err;													\
		while ((err = glGetError()) != GL_NO_ERROR) {				\
			LOG(error) << "GL error " << err;						\
			for (;;)												\
				;													\
		}															\
	} while (0)
#else
#define GLCALL(CALL) CALL
#endif

namespace Starbase {

GLint GetUniformLocation(GLuint program, const char* name);
GLint GetAttribLocation(GLuint program, const char* name);
GLuint MakeVBO(GLenum target, const void* data, GLsizei size, GLenum usage);
GLuint MakeShader(GLenum type, const char* filename, IFilesystem& fs);
GLuint MakeProgram(GLuint vertexShader, GLuint fragmentShader);
GLuint MakeProgram(const char* vsFilename, const char* fsFilename, IFilesystem& fs);

} // namespace Starbase
