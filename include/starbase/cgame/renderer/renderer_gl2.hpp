#pragma once

#include <starbase/gl.hpp>
#include <starbase/game/fs/ifilesystem.hpp>

#include "irenderer.hpp"

namespace Starbase {

class RendererGL2 : public IRenderer {
private:
	IFilesystem& m_filesystem;

	GLuint m_vertexBuffer;
	GLuint m_indexesBuffer;

	GLuint m_vertexShader;
	GLuint m_fragmentShader;
	GLuint m_program;

	struct {
		GLint position;
	} m_attributes;

	GLuint MakeBuffer(GLenum target, const void* data, GLsizei size, GLenum usage);
	GLuint RendererGL2::MakeShader(GLenum type, const char* filename);
	GLuint MakeProgram(GLuint vertexShader, GLuint fragmentShader);

public:
	RendererGL2(IFilesystem& filesystem);

	virtual bool Init();

	virtual void Shutdown();

	virtual void DrawTest();

	~RendererGL2() {}
};

} // namespace Starbase