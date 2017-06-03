#include <array>
#include <limits>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <starbase/game/logging.hpp>
#include <starbase/game/entity/entity.hpp>
#include <starbase/game/component/transform.hpp>
#include <starbase/game/component/physics.hpp>
#include <starbase/game/resource/resourceloader.hpp>
#include <starbase/cgame/display.hpp>
#include <starbase/cgame/component/renderable.hpp>
#include <starbase/cgame/renderer/renderer.hpp>
#include <starbase/cgame/renderer/glhelpers.hpp>

#include <chipmunk/chipmunk_structs.h>

#include <SDL2/SDL.h>//for SDL_GetTicks()



namespace Starbase {

Renderer::Renderer(Display& display, IFilesystem& filesystem, ResourceLoader& rl)
	: m_filesystem(filesystem)
	, m_display(display)
	, m_resourceLoader(rl)
	, m_entityRenderer(m_filesystem, m_renderParams)
{
	m_renderParams.windowSize = m_display.GetWindowSize();
	m_renderParams.debug = false;
	m_renderParams.wireframe = false;
	m_renderParams.zoom = 5.f;
}

bool Renderer::InitFramebuffer(Framebuffer& fb)
{
	const glm::tvec2<int> windowSize = m_renderParams.windowSize;

	GLCALL(glActiveTexture(GL_TEXTURE0));
	GLCALL(glGenTextures(1, &fb.texture));

	GLCALL(glBindTexture(GL_TEXTURE_2D, fb.texture));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowSize.x, windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));

	/* Depth buffer */
	GLCALL(glGenRenderbuffers(1, &fb.rboDepth));
	GLCALL(glBindRenderbuffer(GL_RENDERBUFFER, fb.rboDepth));
	
	GLCALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, windowSize.x, windowSize.y));
	GLCALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));

	/* Framebuffer to link everything together */
	GLCALL(glGenFramebuffers(1, &fb.fbo));
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo));
	GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.texture, 0));
	GLCALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fb.rboDepth));
	GLenum status;
	if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
		LOG(error) << "glCheckFramebufferStatus: error " << status;
		return false;
	}
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	static const GLfloat vertices[] = {
		-1, -1,
		1, -1,
		-1,  1,
		1,  1,
	};
	glGenBuffers(1, &fb.vboVertices);
	glBindBuffer(GL_ARRAY_BUFFER, fb.vboVertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	fb.program = MakeProgram("shaders/postproc.v.glsl", "shaders/postproc.f.glsl", m_filesystem);
	if (!fb.program)
		return false;

	fb.attributes.texCoord = GetAttribLocation(fb.program, "texCoord");

	fb.uniforms.fboTexture = GetUniformLocation(fb.program, "fboTexture");
	fb.uniforms.time = GetUniformLocation(fb.program, "time");
	fb.uniforms.resolution = GetUniformLocation(fb.program, "resolution");
	fb.uniforms.blurDirection = GetUniformLocation(fb.program, "blurDirection");
	fb.uniforms.blurRadius = GetUniformLocation(fb.program, "blurRadius");

	return true;
}

void Renderer::RescaleFramebuffer(Framebuffer& fb)
{
	const glm::tvec2<int> windowSize = m_renderParams.windowSize;

	glBindTexture(GL_TEXTURE_2D, fb.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowSize.x, windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, fb.rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, windowSize.x, windowSize.y);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Renderer::DrawFramebuffer(Framebuffer& fb, GLuint destVBO, int step)
{
	glBindFramebuffer(GL_FRAMEBUFFER, destVBO);

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClear(GL_COLOR_BUFFER_BIT);

	float ticksf = (float)SDL_GetTicks();
	glm::vec2 resf = glm::vec2(m_renderParams.windowSize);

	glUseProgram(fb.program);
	glBindTexture(GL_TEXTURE_2D, fb.texture);

	glUniform1i(fb.uniforms.fboTexture, 0);
	glUniform2f(fb.uniforms.resolution, resf.x, resf.y);
	glUniform1f(fb.uniforms.time, ticksf);
	if (step == 1)
		glUniform2f(fb.uniforms.blurDirection, 1.f, 0.f);
	else
		glUniform2f(fb.uniforms.blurDirection, 0.f, 1.f);
	glUniform1f(fb.uniforms.blurRadius, 2.0f);

	glEnableVertexAttribArray(fb.attributes.texCoord);

	glBindBuffer(GL_ARRAY_BUFFER, fb.vboVertices);
	glVertexAttribPointer(
		fb.attributes.texCoord,  // attribute
		2,                  // number of elements per vertex, here (x,y)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
	);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(fb.attributes.texCoord);
}

static void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    LOG(error) << message;
}

bool Renderer::Init()
{
	if (!m_entityRenderer.Init())
		return false;

	if (!InitFramebuffer(m_fbA))
		return false;

	if (!InitFramebuffer(m_fbB))
		return false;

	//glDebugMessageCallback​(&DebugCallback, nullptr​);

	return true;
}

void Renderer::Shutdown()
{}

void Renderer::RenderableAdded(const Renderable& rend)
{
	m_entityRenderer.RenderableAdded(rend);
}

void Renderer::RenderableRemoved(const Renderable& rend)
{
	m_entityRenderer.RenderableRemoved(rend);
}

void Renderer::PhysicsAdded(const Physics& phys)
{
	m_entityRenderer.PhysicsAdded(phys);
}

void Renderer::PhysicsRemoved(const Physics& phys)
{
	m_entityRenderer.PhysicsRemoved(phys);
}

void Renderer::BeginDraw()
{
	glm::tvec2<int> curWindowSize = m_display.GetWindowSize();
	if (curWindowSize != m_renderParams.windowSize) {
		m_renderParams.windowSize = curWindowSize;
		RescaleFramebuffer(m_fbA);
		RescaleFramebuffer(m_fbB);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbA.fbo);

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::EndDraw()
{
	DrawFramebuffer(m_fbA, m_fbB.fbo, 1);
	DrawFramebuffer(m_fbB, 0, 2);

	/*glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClear(GL_COLOR_BUFFER_BIT);

	float ticksf = (float)SDL_GetTicks();

	glUseProgram(m_fbA.program);
	glBindTexture(GL_TEXTURE_2D, m_fbA.texture);
	glUniform1i(m_fbA.uniforms.fboTexture, 0);
	glUniform1f(m_fbA.uniforms.time, ticksf);
	glEnableVertexAttribArray(m_fbA.attributes.texCoord);

	glBindBuffer(GL_ARRAY_BUFFER, m_fbA.vboVertices);
	glVertexAttribPointer(
		m_fbA.attributes.texCoord,  // attribute
		2,                  // number of elements per vertex, here (x,y)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
	);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(m_fbA.attributes.texCoord);*/
}

void Renderer::Draw(const Entity& ent, const Transform& trans, const Renderable& rend, const Physics* maybePhysics)
{
	m_entityRenderer.Draw(ent, trans, rend, maybePhysics);
}

} // namespace Starbase
