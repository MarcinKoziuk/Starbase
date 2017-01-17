#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <starbase/gl.hpp>
#include <starbase/game/fwd.hpp>
#include <starbase/game/fs/ifilesystem.hpp>
#include <starbase/cgame/fwd.hpp>

namespace Starbase {

struct PathShader {
	GLuint vertexShader, fragmentShader;
	GLuint program;

	struct {
		GLint color;
		GLint mvp;
	} uniforms;

	struct {
		GLint position;
	} attributes;
};

struct ModelGL {
	std::shared_ptr<const Resource::Model> modelRef;
	std::vector<GLuint> pathVBOs;
};

class Renderer {
private:
	IFilesystem& m_filesystem;
	Display& m_display;
	ResourceLoader& m_resourceLoader;

	PathShader m_pathShader;

	std::unordered_map<id_t, ModelGL> m_modelGlData;

	GLuint MakeVBO(GLenum target, const void* data, GLsizei size, GLenum usage);
	GLuint MakeShader(GLenum type, const char* filename);
	GLuint MakeProgram(GLuint vertexShader, GLuint fragmentShader);

	bool InitPathShader();
	bool InitModelGL(id_t, ModelGL* modelGl);
	glm::mat4 CalcMatrix(const Transform& trans);

public:
	Renderer(Display& display, IFilesystem& filesystem, ResourceLoader& rl);

	~Renderer() {}

	bool Init();

	void Shutdown();

	void RenderableAdded(const Renderable& rend);

	void RenderableRemoved(const Renderable& rend);

	void BeginDraw();

	void Draw(const Entity& ent, const Transform& trans, const Renderable& rend);

	void EndDraw();
};

} // namespace Starbase