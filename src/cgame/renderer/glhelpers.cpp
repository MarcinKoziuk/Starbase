#include <starbase/game/logging.hpp>
#include <starbase/cgame/renderer/glhelpers.hpp>

namespace Starbase {

static void ShowInfoLog(
	GLuint object,
	PFNGLGETSHADERIVPROC glGet__iv,
	PFNGLGETSHADERINFOLOGPROC glGet__InfoLog
)
{
	GLint len;
	std::string log;
	glGet__iv(object, GL_INFO_LOG_LENGTH, &len);
	log.resize(len);
	glGet__InfoLog(object, len, NULL, &log[0]);
	LOG(error) << log;
}

GLint GetUniformLocation(GLuint program, const char* name)
{
	GLint location;
	GLCALL(location = glGetUniformLocation(program, name));
	if (location == -1) {
		LOG(error) << "glGetUniformLocation for " << name << " returned -1";
	}

	return location;
}

GLint GetAttribLocation(GLuint program, const char* name)
{
	GLint location;
	GLCALL(location = glGetAttribLocation(program, name));
	if (location == -1) {
		LOG(error) << "glGetAttribLocation for " << name << " returned -1";
	}

	return location;
}

GLuint MakeVBO(GLenum target, const void* data, GLsizei size, GLenum usage)
{
	GLuint vbo;
	GLCALL(glGenBuffers(1, &vbo));
	GLCALL(glBindBuffer(target, vbo));
	GLCALL(glBufferData(target, size, data, usage));
	return vbo;
}

GLuint MakeShader(GLenum type, const char* filename, IFilesystem& fs)
{
	std::string str;

	if (!fs.ReadString(filename, &str))
		return 0;

	GLchar* source = (GLchar*)&str[0];
	GLint length = (GLint)str.length();

	GLuint shader;
	GLCALL(shader = glCreateShader(type));
	GLCALL(glShaderSource(shader, 1, (const GLchar**)&source, &length));
	GLCALL(glCompileShader(shader));

	GLint shaderOk;
	GLCALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderOk));

	if (!shaderOk) {
		LOG(error) << "Failed to compile shader " << filename;
		ShowInfoLog(shader, glGetShaderiv, glGetShaderInfoLog);
		GLCALL(glDeleteShader(shader));
		return 0;
	}

	return shader;
}

GLuint MakeProgram(GLuint vertexShader, GLuint fragmentShader)
{
	GLuint program;
	GLCALL(program = glCreateProgram());
	GLCALL(glAttachShader(program, vertexShader));
	GLCALL(glAttachShader(program, fragmentShader));
	GLCALL(glLinkProgram(program));

	GLint linkOk;
	GLCALL(glGetProgramiv(program, GL_LINK_STATUS, &linkOk));
	if (!linkOk) {
		LOG(error) << "Failed to link shader program";
		GLCALL(glDeleteProgram(program));
		return 0;
	}

	GLint validateOk;
	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &validateOk);
	if (!validateOk) {
		LOG(error) << "Failed to validate shader program";
		GLCALL(glDeleteProgram(program));
		return 0;
	}

	return program;
}

GLuint MakeProgram(const char* vsFilename, const char* fsFilename, IFilesystem& fs)
{
	GLuint vertexShader = MakeShader(GL_VERTEX_SHADER, vsFilename, fs);
	if (!vertexShader)
		return 0;

	GLuint fragmentShader = MakeShader(GL_FRAGMENT_SHADER, fsFilename, fs);
	if (!fragmentShader)
		return 0;

	GLuint program = MakeProgram(vertexShader, fragmentShader);
	if (!program)
		return 0;

	return program;
}

} // namespace Starbase
