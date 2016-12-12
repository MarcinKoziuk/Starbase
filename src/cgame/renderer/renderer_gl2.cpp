#include <starbase/game/logging.hpp>

#include <starbase/cgame/renderer/renderer_gl2.hpp>

#ifndef NDEBUG
#define GLCALL(CALL)													\
	do {																\
		CALL;															\
		GLenum err;														\
		while ((err = glGetError()) != GL_NO_ERROR) {					\
			LOG(error) << "GL error " << err;				\
		}																\
	} while (0)
#else
#define GLCALL(CALL) CALL
#endif

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

RendererGL2::RendererGL2(IFilesystem& filesystem)
	: m_filesystem(filesystem)
{}

GLuint RendererGL2::MakeBuffer(GLenum target, const void* data, GLsizei size, GLenum usage)
{
	GLuint buffer;
	GLCALL(glGenBuffers(1, &buffer));
	GLCALL(glBindBuffer(target, buffer));
	GLCALL(glBufferData(target, size, data, usage));
	return buffer;
}

GLuint RendererGL2::MakeShader(GLenum type, const char* filename)
{
	std::string str;

	if (!m_filesystem.ReadString(filename, &str))
		return 0;

	GLchar* source = (GLchar*) &str[0];
	GLint length = (GLint) str.length();

	GLuint shader;
	GLCALL(shader = glCreateShader(type));
	GLCALL(glShaderSource(shader, 1, (const GLchar**)&source, &length));
	GLCALL(glCompileShader(shader));

	GLint shaderOk;
	GLCALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderOk));

	if (!shaderOk) {
		LOG(error) << "Failed to compile shader " << filename;
		GLCALL(glDeleteShader(shader));
		return 0;
	}

	return shader;
}

GLuint RendererGL2::MakeProgram(GLuint vertexShader, GLuint fragmentShader)
{
	GLuint program;
	GLCALL(program = glCreateProgram());
	GLCALL(glAttachShader(program, vertexShader));
	GLCALL(glAttachShader(program, fragmentShader));
	GLCALL(glLinkProgram(program));

	GLint programOk;
	GLCALL(glGetProgramiv(program, GL_LINK_STATUS, &programOk));
	if (!programOk) {
		LOG(error) << "Failed to link shader program";
		GLCALL(glDeleteProgram(program));
		return 0;
	}

	return program;
}

bool RendererGL2::Init()
{
	static const GLfloat vertices[] = {
		-0.4f, -0.4f,
		0.4f, -0.4f,
		-0.4f,  0.4f,
		0.4f,  0.4f
	};
	static const GLushort indexes[] = { 0, 1, 2, 3 };

	m_vertexBuffer = MakeBuffer(GL_ARRAY_BUFFER, vertices, sizeof(vertices), GL_STREAM_DRAW);
	m_indexesBuffer = MakeBuffer(GL_ARRAY_BUFFER, indexes, sizeof(indexes), GL_STREAM_DRAW);

	m_vertexShader = MakeShader(GL_VERTEX_SHADER, "shaders/simple.v.glsl");
	if (!m_vertexShader)
		return false;

	m_fragmentShader = MakeShader(GL_FRAGMENT_SHADER, "shaders/simple.f.glsl");
	if (!m_fragmentShader)
		return false;

	m_program = MakeProgram(m_vertexShader, m_fragmentShader);
	if (!m_program)
		return false;

	GLCALL(m_attributes.position = glGetAttribLocation(m_program, "position"));

	return true;
}

void RendererGL2::Shutdown()
{}

void RendererGL2::DrawTest()
{
	GLCALL(glClearColor(1.f, 0.f, 0.f, 1.f));
	GLCALL(glClear(GL_COLOR_BUFFER_BIT));

	GLCALL(glUseProgram(m_program));
	
	GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer));
	GLCALL(glVertexAttribPointer(
		m_attributes.position,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(GLfloat) * 2,
		0
	));
	GLCALL(glEnableVertexAttribArray(m_attributes.position));

	GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexesBuffer));
	GLCALL(glDrawElements(
		GL_TRIANGLE_STRIP,
		4,
		GL_UNSIGNED_SHORT,
		0
	));

	GLCALL(glDisableVertexAttribArray(m_attributes.position));
}

} // namespace Starbase
