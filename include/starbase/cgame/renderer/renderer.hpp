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
#include <starbase/game/entity/eventmanager.hpp>

#include <starbase/cgame/fwd.hpp>
#include <starbase/cgame/renderer/entityrenderer.hpp>
#include <starbase/cgame/renderer/framebuffer.hpp>
#include <starbase/cgame/renderer/camera.hpp>

namespace Starbase {

struct Framebuffer {
	GLuint fbo;
	GLuint program;

	int msaa;
	GLenum textureType;
	GLuint texture;
	GLuint rboDepth;
	GLuint vboVertices;

	struct {
		GLint fboTexture;
		GLint time;
		GLint resolution;

		GLint blurDirection;
		GLint blurRadius;
	} uniforms;

	struct {
		GLint texCoord;
	} attributes;
};

class Renderer {
private:
	IFilesystem& m_filesystem;
	Display& m_display;
	ResourceLoader& m_resourceLoader;
	EntityRenderer m_entityRenderer;

	Framebuffer m_fbMulti;
	Framebuffer m_fbBlurHoriz;
	Framebuffer m_fbBlurVert;
	
	enum DrawStep { BLUR_HORIZ, BLUR_VERT };

	bool InitFramebuffer(Framebuffer& fb, int msaa);
	void RescaleFramebuffer(Framebuffer& fb);
	void DrawFramebuffer(Framebuffer& fb, GLuint destFBO, int step);

public:
	typedef EntityRenderer::ComponentGroup ComponentGroup;

	RenderParams m_renderParams;

	Renderer(Display&, IFilesystem&, ResourceLoader&, EventManager&);

	~Renderer() {}

	bool Init();

	void Shutdown();

	void BeginDraw();

	void Draw(const ComponentGroup& cg);

	void EndDraw();
};

} // namespace Starbase