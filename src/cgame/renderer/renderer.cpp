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

#include <chipmunk/chipmunk_structs.h>

#ifndef NDEBUG
#define GLCALL(CALL)													\
	do {																\
		CALL;															\
		GLenum err;														\
		while ((err = glGetError()) != GL_NO_ERROR) {					\
			LOG(error) << "GL error " << err;							\
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

Renderer::Renderer(Display& display, IFilesystem& filesystem, ResourceLoader& rl)
	: m_filesystem(filesystem)
	, m_display(display)
	, m_resourceLoader(rl)
	, m_debugDraw(false)
	, m_zoom(5.f)
{}

GLuint Renderer::MakeVBO(GLenum target, const void* data, GLsizei size, GLenum usage)
{
	GLuint vbo;
	GLCALL(glGenBuffers(1, &vbo));
	GLCALL(glBindBuffer(target, vbo));
	GLCALL(glBufferData(target, size, data, usage));
	return vbo;
}

GLuint Renderer::MakeShader(GLenum type, const char* filename)
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
		ShowInfoLog(shader, glGetShaderiv, glGetShaderInfoLog);
		GLCALL(glDeleteShader(shader));
		return 0;
	}

	return shader;
}

GLuint Renderer::MakeProgram(GLuint vertexShader, GLuint fragmentShader)
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

bool Renderer::InitPathShader()
{
	m_pathShader.vertexShader = MakeShader(GL_VERTEX_SHADER, "shaders/simple.v.glsl");
	if (!m_pathShader.vertexShader)
		return false;

	m_pathShader.fragmentShader = MakeShader(GL_FRAGMENT_SHADER, "shaders/simple.f.glsl");
	if (!m_pathShader.fragmentShader)
		return false;

	m_pathShader.program = MakeProgram(m_pathShader.vertexShader, m_pathShader.fragmentShader);
	if (!m_pathShader.program)
		return false;

	GLCALL(m_pathShader.attributes.position = glGetAttribLocation(m_pathShader.program, "position"));

	GLCALL(m_pathShader.uniforms.color = glGetUniformLocation(m_pathShader.program, "color"));
	GLCALL(m_pathShader.uniforms.mvp = glGetUniformLocation(m_pathShader.program, "mvp"));

	return true;
}

bool Renderer::InitModelGL(ResourcePtr<Model> model, ModelGL* modelGl)
{
	for (const Model::Path& path : model->GetPaths()) {
		const GLfloat* positions = reinterpret_cast<const GLfloat*>(&path.points.front());
		const GLsizei numPositions = static_cast<GLsizei>(sizeof(GLfloat) * 2 * path.points.size());

		GLuint vbo = MakeVBO(GL_ARRAY_BUFFER, positions, numPositions, GL_STATIC_DRAW);
		modelGl->pathVBOs.push_back(vbo);
	}

	return true;
}

// for debug draw
bool Renderer::InitBodyGL(ResourcePtr<Body> bodyPtr, const Physics& physics, ModelGL* modelGl)
{
	for (const auto& polyShape : bodyPtr->GetPolygonShapes()) {
		const cpFloat* positions = reinterpret_cast<const cpFloat*>(&polyShape.front());
		const GLsizei numPositions = static_cast<GLsizei>(sizeof(cpFloat) * 2 * polyShape.size());

		GLuint vbo = MakeVBO(GL_ARRAY_BUFFER, positions, numPositions, GL_STATIC_DRAW);
		modelGl->pathVBOs.push_back(vbo);
	}

	return true;
}

bool Renderer::Init()
{
	return InitPathShader();
}

void Renderer::Shutdown()
{}

glm::mat4 Renderer::CalcMatrix(const Transform& trans)
{
	glm::mat4 model, view, projection;

	glm::vec2 windowSize = m_display.GetWindowSize();
	//float aspect = windowSize.x / (float)windowSize.y;

	projection = glm::ortho(-windowSize.x/m_zoom, windowSize.x/m_zoom, -windowSize.y/m_zoom, windowSize.y/m_zoom, -1.0f, 1.0f);;


	model = glm::translate(model, glm::vec3(trans.pos.x, trans.pos.y, 0));
	model = glm::rotate(model, trans.rot, glm::vec3(0.f, 0.f, 1.f));
	model = glm::scale(model, glm::vec3(trans.scale.x, trans.scale.y, 1.f));
	
	view = glm::mat4(1.f);

	return projection * view * model;
}

void Renderer::RenderableAdded(const Renderable& rend)
{
	const auto& model = rend.model;

	if (!m_modelGlData.count(model.Id())) {
		auto it = m_modelGlData.emplace(model.Id(), ModelGL());
		ModelGL& modelGl = it.first->second;

		bool ok = InitModelGL(model, &modelGl);
		if (SB_UNLIKELY(!ok)) {
			m_modelGlData.erase(it.first);
		}
	}
}

void Renderer::RenderableRemoved(const Renderable& rend)
{
	m_modelGlData.erase(rend.model.Id());
}

void Renderer::BeginDraw()
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::DebugDraw(const Entity& ent, const Transform& trans, const Physics& physics)
{
	ModelGL modelGl;
	InitBodyGL(physics.body, physics, &modelGl);

	glm::mat4 mvp = CalcMatrix(trans);

	const Body& body = *physics.body;
	std::size_t numShapes = body.GetPolygonShapes().size();

	for (std::size_t i = 0; i < numShapes; i++) {
		GLuint vbo = modelGl.pathVBOs[i];
		const std::vector<glm::tvec2<cpFloat>>& polyShape = body.GetPolygonShapes()[i];

		GLCALL(glUniform3f(m_pathShader.uniforms.color, 0.0, 1.0, 0.0));
		GLCALL(glUniformMatrix4fv(m_pathShader.uniforms.mvp, 1, GL_FALSE, glm::value_ptr(mvp)));

		GLCALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
		GLCALL(glVertexAttribPointer(
			m_pathShader.attributes.position,
			2,
			GL_DOUBLE,
			GL_FALSE,
			sizeof(GLdouble) * 2,
			nullptr
		));

		GLCALL(glEnableVertexAttribArray(m_pathShader.attributes.position));

		GLCALL(glDrawArrays(
			GL_LINE_LOOP,
			0,
			(GLsizei)polyShape.size()
		));

		GLCALL(glDisableVertexAttribArray(m_pathShader.attributes.position));
	}

	GLCALL(glUniform3f(m_pathShader.uniforms.color, 0.0, 1.0, 0.0));

	// center!
	const float vals[] = {
		-1, 1,
		-1, -1,
		1, -1,
		1, 1 };

	GLuint vbo = MakeVBO(GL_ARRAY_BUFFER, &vals, sizeof(vals)*sizeof(float), GL_STATIC_DRAW);

	GLCALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GLCALL(glVertexAttribPointer(
		m_pathShader.attributes.position,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(GLfloat) * 2,
		nullptr
	));

	GLCALL(glEnableVertexAttribArray(m_pathShader.attributes.position));

	GLCALL(glDrawArrays(
		GL_LINE_LOOP,
		0,
		4
	));

	GLCALL(glDisableVertexAttribArray(m_pathShader.attributes.position));

	glDeleteBuffers(1, &vbo);
}

void Renderer::Draw(const Entity& ent, const Transform& trans, const Renderable& rend, const Physics* maybePhysics)
{
	const id_t modelId = rend.model.Id();

	if (SB_UNLIKELY(!m_modelGlData.count(modelId))) {
		LOG(error) << "Need to implicitly initialize renderable; this may cause a memory leak!";
		RenderableAdded(rend);
	}

	const ModelGL& modelGl = m_modelGlData.at(modelId);
	const Model& model = *rend.model.Get();

	GLCALL(glUseProgram(m_pathShader.program));

	std::size_t numPaths = model.GetPaths().size();
	assert(modelGl.pathVBOs.size() == numPaths);

	for (std::size_t i = 0; i < numPaths; i++) {
		GLuint vbo = modelGl.pathVBOs[i];
		const Model::Path& path = model.GetPaths()[i];
		const Model::Style& style = path.style;

		glm::mat4 mvp = CalcMatrix(trans);

		GLCALL(glUniform3f(m_pathShader.uniforms.color, style.color.x, style.color.y, style.color.z));
		GLCALL(glUniformMatrix4fv(m_pathShader.uniforms.mvp, 1, GL_FALSE, glm::value_ptr(mvp)));

		GLCALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
		GLCALL(glVertexAttribPointer(
			m_pathShader.attributes.position,
			2,
			GL_FLOAT,
			GL_FALSE,
			sizeof(GLfloat) * 2,
			nullptr
		));

		GLCALL(glEnableVertexAttribArray(m_pathShader.attributes.position));

		GLenum mode = path.closed ? GL_LINE_LOOP : GL_LINE_STRIP;
		GLCALL(glDrawArrays(
			mode,
			0,
			(GLsizei)path.points.size()
		));

		GLCALL(glDisableVertexAttribArray(m_pathShader.attributes.position));
	}

	if (m_debugDraw && maybePhysics != nullptr) {
		DebugDraw(ent, trans, *maybePhysics);
	}
}

void Renderer::EndDraw()
{}

} // namespace Starbase
