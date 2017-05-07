#include <starbase/cgame/renderer/glhelpers.hpp>
#include <starbase/cgame/renderer/shader.hpp>

namespace Starbase {

optional<Shader> Shader::Create
    ( IFilesystem& fs
	, const char* vertexShaderPath
	, const char* fragmentShaderPath
	, const char* const uniformNames[]
	, const char* const attributeNames[])
{
	Shader ret;

	const GLuint vertexShader = MakeShader(GL_VERTEX_SHADER, vertexShaderPath, fs);
	if (!vertexShader)
		return nullopt;

	const GLuint fragmentShader = MakeShader(GL_FRAGMENT_SHADER, fragmentShaderPath, fs);
	if (!fragmentShader)
		return nullopt;

	const GLuint program = MakeProgram(vertexShader, fragmentShader);
	if (!program)
		return nullopt;

	ret.program = program;
	for (const char* const* p = uniformNames; *p != nullptr; p++) {
		const char* const name = *p;
		GLCALL(ret.uniforms[ID(name)] = glGetUniformLocation(program, name));
	}
	
	for (const char* const* p = attributeNames; *p != nullptr; p++) {
		const char* const name = *p;
		GLCALL(ret.attributes[ID(name)] = glGetAttribLocation(program, name));
	}

	return ret;
}

} // namespace Starbase
