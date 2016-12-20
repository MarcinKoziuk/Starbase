#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL2/SDL.h> //temp

#include <starbase/game/logging.hpp>
#include <starbase/game/entity/entity.hpp>
#include <starbase/game/component/transform.hpp>
#include <starbase/game/resource/resourceloader.hpp>
#include <starbase/cgame/component/renderable.hpp>
#include <starbase/cgame/renderer/renderer_gl2.hpp>

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
		ShowInfoLog(shader, glGetShaderiv, glGetShaderInfoLog);
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

bool RendererGL2::InitPathShader()
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

bool RendererGL2::InitPathBuffer(PathBuffer& buf, std::shared_ptr<const Resource::Model>& model, int pathIndex)
{
	const Resource::Model::Path& path = model->GetPaths()[pathIndex];
	const GLfloat* positions = reinterpret_cast<const GLfloat*>(&path.positions.front());
	const GLsizei positionsSize = sizeof(GLfloat) * 2 * path.positions.size();

	buf.vertexBuffer = MakeBuffer(GL_ARRAY_BUFFER, positions, positionsSize, GL_DYNAMIC_DRAW);
	buf.pathIndex = pathIndex;
	buf.model = model;

	return true;
}

bool RendererGL2::InitModel(std::uint32_t modelId, std::shared_ptr<const Resource::Model>& model)
{
	std::size_t numPaths = model->GetPaths().size();
	std::vector<PathBuffer>& pathBuffers = m_pathBuffers.emplace(modelId, std::vector<PathBuffer>()).first->second;
	for (int i = 0; i < numPaths; i++) {
		const Resource::Model::Path& path = model->GetPaths()[i];
		pathBuffers.emplace_back();
		PathBuffer& buf = pathBuffers.back();

		InitPathBuffer(buf, model, i);
	}

	return true;
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

	InitPathShader();

	return true;
}

void RendererGL2::Shutdown()
{}

static glm::mat4 CalcMatrix(Transform& trans)
{
	glm::mat4 model, view, projection;

	float aspect = 1280.f / 800.f;
	//projection = glm::ortho(-aspect,  -1.0f, aspect, 1.0f, -1.0f, 1.0f);
	projection = glm::ortho(-640.f, 640.f, -400.f, 400.f, -1.0f, 1.0f);;

	model - glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(trans.pos.x, trans.pos.y, 0));
	model = glm::rotate(model, trans.rot, glm::vec3(0.f, 0.f, 1.f)); // glm::radians(SDL_GetTicks() / 100.f)
	model = glm::scale(model, glm::vec3(trans.scale.x, trans.scale.y, 1.f));
	
	view = glm::mat4(1.f);

	return projection * view * model;
}

void RendererGL2::Draw(Entity& ent, Transform& trans, Renderable& rend, ResourceLoader& rl)
{

	if (SB_UNLIKELY(!m_pathBuffers.count(rend.modelId))) {
		InitModel(rend.modelId, rl.Get<Resource::Model>(rend.modelId));
	}

	GLCALL(glUseProgram(m_pathShader.program));
	for (PathBuffer& buf : m_pathBuffers[rend.modelId]) {
		const Resource::Model::Path& path = buf.model->GetPaths()[buf.pathIndex];

		glm::mat4 mvp = CalcMatrix(trans);

		GLCALL(glUniform3f(m_pathShader.uniforms.color, path.color.x, path.color.y, path.color.z));
		GLCALL(glUniformMatrix4fv(m_pathShader.uniforms.mvp, 1, GL_FALSE, glm::value_ptr(mvp)));

		GLCALL(glBindBuffer(GL_ARRAY_BUFFER, buf.vertexBuffer));
		GLCALL(glVertexAttribPointer(
			m_pathShader.attributes.position,
			2,
			GL_FLOAT,
			GL_FALSE,
			sizeof(GLfloat) * 2,
			false
		));

		GLCALL(glEnableVertexAttribArray(m_pathShader.attributes.position));

		GLenum mode = path.closed ? GL_LINE_LOOP : GL_LINE_STRIP;
		GLCALL(glDrawArrays(
			mode,
			0,
			path.positions.size()
		));

		GLCALL(glDisableVertexAttribArray(m_pathShader.attributes.position));
	}
}

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
