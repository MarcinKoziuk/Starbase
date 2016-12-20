#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

#include <glm/vec3.hpp>

#include <starbase/gl.hpp>
#include <starbase/game/fs/ifilesystem.hpp>
#include <starbase/cgame/resource/model.hpp>

#include "irenderer.hpp"

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

struct PathBuffer {
	GLuint vertexBuffer;

	std::shared_ptr<const Resource::Model> model;
	int pathIndex;
};

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

	PathShader m_pathShader;
	std::unordered_map<std::uint32_t, std::vector<PathBuffer>> m_pathBuffers;

	GLuint MakeBuffer(GLenum target, const void* data, GLsizei size, GLenum usage);
	GLuint RendererGL2::MakeShader(GLenum type, const char* filename);
	GLuint MakeProgram(GLuint vertexShader, GLuint fragmentShader);

	bool InitPathShader();
	bool InitPathBuffer(PathBuffer& buf, std::shared_ptr<const Resource::Model>& model, int pathIndex);
	bool InitModel(std::uint32_t modelId, std::shared_ptr<const Resource::Model>& model);

public:
	RendererGL2(IFilesystem& filesystem);

	virtual bool Init();

	virtual void Shutdown();

	virtual void DrawTest();

	virtual void Draw(Entity& ent, Transform& trans, Renderable& rend, ResourceLoader& rl);

	~RendererGL2() {}
};

} // namespace Starbase