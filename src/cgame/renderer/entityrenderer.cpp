#include <array>
#include <limits>
#include <cmath>
#include <cassert>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <starbase/game/logging.hpp>
#include <starbase/game/entity/entity.hpp>
#include <starbase/game/component/transform.hpp>
#include <starbase/game/component/physics.hpp>
#include <starbase/game/component/shipcontrols.hpp>

#include <starbase/cgame/component/renderable.hpp>
#include <starbase/cgame/renderer/glhelpers.hpp>
#include <starbase/cgame/renderer/entityrenderer.hpp>

namespace Starbase {

using PathShader = EntityRenderer::PathShader;
using LineShader = EntityRenderer::LineShader;
using PathGL = EntityRenderer::PathGL;
using ModelGL = EntityRenderer::ModelGL;
using BodyGL = EntityRenderer::BodyGL;

bool EntityRenderer::PathShader::Init(PathShader& dest, IFilesystem& fs)
{
	dest.program = MakeProgram("shaders/path.v.glsl", "shaders/path.f.glsl", fs);
	if (!dest.program)
		return false;

	dest.attributes.position = GetAttribLocation(dest.program, "position");
	dest.attributes.cornerVect = GetAttribLocation(dest.program, "cornerVect");

	dest.uniforms.scale = GetUniformLocation(dest.program, "scale");
	dest.uniforms.thickness = GetUniformLocation(dest.program, "thickness");
	dest.uniforms.color = GetUniformLocation(dest.program, "color");
	dest.uniforms.mvp = GetUniformLocation(dest.program, "mvp");
	dest.uniforms.zoom = GetUniformLocation(dest.program, "zoom");

	return true;
}

bool EntityRenderer::LineShader::Init(LineShader& dest, IFilesystem& fs)
{
	dest.program = MakeProgram("shaders/line.v.glsl", "shaders/line.f.glsl", fs);
	if (!dest.program)
		return false;

	dest.attributes.position = GetAttribLocation(dest.program, "position");
	dest.uniforms.mvp = GetUniformLocation(dest.program, "mvp");

	return true;
}

EntityRenderer::ModelGL::ModelGL(const Model& model)
	: refcount(1L)
{
	for (const Model::Path& path : model.GetPaths()) {
		paths.emplace_back(path);
	}
}

static glm::vec2 MakeCornerVect(const float ang, const glm::vec2& p)
{
	const float x = p.x * std::cos(ang) - p.y * std::sin(ang);
	const float y = p.x * std::sin(ang) + p.y * std::cos(ang);
	return glm::vec2(x, y);
}

static std::array<glm::vec2, 4> MakeCornerVects(const glm::vec2& p1, const glm::vec2& p2)
{
	std::array<glm::vec2, 4> cornerVecs;

	const float ang = std::atan2(p2.y - p1.y, p2.x - p1.x);
	cornerVecs[0] = MakeCornerVect(ang, glm::vec2(0, 1));
	cornerVecs[1] = MakeCornerVect(ang, glm::vec2(0, -1));
	cornerVecs[2] = MakeCornerVect(ang, glm::vec2(0, 1));
	cornerVecs[3] = MakeCornerVect(ang, glm::vec2(0, -1));

	return cornerVecs;
}

void EntityRenderer::PathGL::AddRect(const glm::vec2& p1, const glm::vec2& p2)
{
	static const GLushort indexes[] = {
		0, 1, 2,
		1, 2, 3
	};

	std::array<glm::vec2, 4> cornerVecs = MakeCornerVects(p1, p2);
	std::size_t indexOffset = vertices.size();

	assert((indexOffset + 4) < std::numeric_limits<GLushort>::max());

	vertices.push_back(p1);
	vertices.push_back(p1);
	vertices.push_back(p2);
	vertices.push_back(p2);

	for (const glm::vec2 cornerVec : cornerVecs) {
		cornerVects.push_back(cornerVec);
	}

	for (const GLushort i : indexes) {
		indices.push_back(static_cast<GLushort>(indexOffset) + i);
	}
}

void EntityRenderer::PathGL::AddRects(const Model::Path& path)
{
	for (std::size_t i = 0; i < (path.points.size() - 1); i++) {
		const glm::vec2& p1 = path.points[i];
		const glm::vec2& p2 = path.points[i + 1];

		AddRect(p1, p2);
	}

	if (path.closed && !path.points.empty()) {
		AddRect(path.points.back(), path.points.front());
	}
}

EntityRenderer::PathGL::PathGL(const Model::Path& path)
{
	AddRects(path);

	const GLfloat* verticesPtr = &vertices.front().x;
	const GLsizei numVertices = static_cast<GLsizei>(sizeof(glm::vec2) * vertices.size());
	verticesVBO = MakeVBO(GL_ARRAY_BUFFER, verticesPtr, numVertices, GL_STATIC_DRAW);

	const GLushort* indicesPtr = &indices.front();
	const GLsizei numIndexes = static_cast<GLsizei>(sizeof(GLushort) * indices.size());
	indicesVBO = MakeVBO(GL_ELEMENT_ARRAY_BUFFER, indicesPtr, numIndexes, GL_STATIC_DRAW);

	const GLfloat* cornerVectsPtr = &cornerVects.front().x;
	const GLsizei numCornerVecs = static_cast<GLsizei>(sizeof(glm::vec2) * cornerVects.size());
	cornerVectsVBO = MakeVBO(GL_ARRAY_BUFFER, cornerVectsPtr, numCornerVecs, GL_STATIC_DRAW);
}

EntityRenderer::BodyGL::BodyGL(const Body& body)
{
	for (const auto& polyShape : body.GetPolygonShapes()) {
		const cpFloat* positions = reinterpret_cast<const cpFloat*>(&polyShape.front());
		const GLsizei numPositions = static_cast<GLsizei>(sizeof(cpFloat) * 2 * polyShape.size());

		GLuint vbo = MakeVBO(GL_ARRAY_BUFFER, positions, numPositions, GL_STATIC_DRAW);
		shapeVBOs.push_back(vbo);
	}

	const static float diameter = 0.2f;
	const static float vals[] = {
		-diameter, diameter,
		-diameter, -diameter,
		diameter, -diameter,
		diameter, diameter
	};
	centerVBO = MakeVBO(GL_ARRAY_BUFFER, &vals, sizeof(vals) * sizeof(float), GL_STATIC_DRAW);
}

EntityRenderer::EntityRenderer(IFilesystem& fs, const RenderParams& renderParams, EventManager& eventManager)
	: m_filesystem(fs)
	, m_renderParams(renderParams)
	, m_eventManager(eventManager)
{
	eventManager.Connect<Physics, EventManager::component_added>([this](Entity& ent, Physics& phys) {
		PhysicsAdded(phys);
	});
	eventManager.Connect<Physics, EventManager::component_removed>([this](Entity& ent, Physics& phys) {
		PhysicsRemoved(phys);
	});
	eventManager.Connect<Renderable, EventManager::component_added>([this](Entity& ent, Renderable& rend) {
		RenderableAdded(rend);
	});
	eventManager.Connect<Renderable, EventManager::component_removed>([this](Entity& ent, Renderable& rend) {
		RenderableRemoved(rend);
	});
}

bool EntityRenderer::Init()
{
	if (!PathShader::Init(m_pathShader, m_filesystem))
		return false;

	if (!LineShader::Init(m_lineShader, m_filesystem))
		return false;

	return true;
}

void EntityRenderer::RenderableAdded(const Renderable& rend)
{
	const ResourcePtr<Model>& model = rend.model;

	if (!m_modelsGL.count(model.Id())) {
		m_modelsGL.emplace(model.Id(), ModelGL(*model));
	} else {
		m_modelsGL.at(model.Id()).refcount++;
	}
}

void EntityRenderer::RenderableRemoved(const Renderable& rend)
{
	ModelGL& modelGL = m_modelsGL.at(rend.model.Id());
	modelGL.refcount--;

	if (modelGL.refcount < 0) {
		m_modelsGL.erase(rend.model.Id());
	}
}

void EntityRenderer::PhysicsAdded(const Physics& phys)
{
	const ResourcePtr<Body>& body = phys.body;

	if (!m_bodiesGL.count(body.Id())) {
		m_bodiesGL.emplace(body.Id(), BodyGL(*body));
	} else {
		m_bodiesGL.at(body.Id()).refcount++;
	}
}

void EntityRenderer::PhysicsRemoved(const Physics& phys)
{
	BodyGL& bodyGL = m_bodiesGL.at(phys.body.Id());
	bodyGL.refcount--;

	if (bodyGL.refcount < 0) {
		m_bodiesGL.erase(phys.body.Id());
	}
}

static glm::mat4 CalcMatrix(double alpha, const Transform& trans, const RenderParams& renderParams)
{
	glm::mat4 model, view, projection;

	const float w = static_cast<float>(renderParams.windowSize.x);
	const float h = static_cast<float>(renderParams.windowSize.y);
	const float zoom = renderParams.zoom;

	projection = glm::ortho(-w / zoom, w / zoom, -h / zoom, h / zoom, -1.0f, 1.0f);;

	const glm::vec2 renderPos(
		trans.pos.x * float(alpha) + trans.prevPos.x * float(1.0 - alpha),
		trans.pos.y * float(alpha) + trans.prevPos.y * float(1.0 - alpha)
	);

	model = glm::translate(model, glm::vec3(renderPos, 0));
	model = glm::translate(model, glm::vec3(-renderParams.offset, 0));
	model = glm::rotate(model, trans.rot, glm::vec3(0.f, 0.f, 1.f));
	model = glm::scale(model, glm::vec3(trans.scale.x, trans.scale.y, 1.f));

	view = glm::mat4(1.f);

	return projection * view * model;
}

void EntityRenderer::Draw(double alpha, const EntityRenderer::ComponentGroup& cg)
{
	NormalDraw(alpha, cg);

	if (cg.phys != nullptr && m_renderParams.debug) {
		DebugDraw(alpha, cg.ent, cg.trans, *cg.phys);
	}
}

static bool IsPathVisible(const EntityRenderer::ComponentGroup& cg, const Model::Path& path)
{
	if (path.group == ID("_thrust")) {
		return cg.contr != nullptr && cg.contr->actionFlags.thrustForward;
	}
	return true;
}

void EntityRenderer::NormalDraw(double alpha, const EntityRenderer::ComponentGroup& cg)
{
	const id_t modelId = cg.rend.model.Id();
	const ModelGL& modelGL = m_modelsGL.at(modelId);
	const Model& model = *cg.rend.model.Get();

	const std::size_t numPaths = model.GetPaths().size();
	assert(modelGL.paths.size() == numPaths);

	GLCALL(glUseProgram(m_pathShader.program));

	if (m_renderParams.wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	for (std::size_t i = 0; i < numPaths; i++) {
		const Model::Path& path = model.GetPaths()[i];
		const Model::Style& style = path.style;
		const PathGL& pathGL = modelGL.paths[i];

		if (!IsPathVisible(cg, path))
			continue;

		glm::mat4 mvp = CalcMatrix(alpha, cg.trans, m_renderParams);

		GLCALL(glUniformMatrix4fv(m_pathShader.uniforms.mvp, 1, GL_FALSE, glm::value_ptr(mvp)));

		GLCALL(glUniform2f(m_pathShader.uniforms.scale, cg.trans.scale.x, cg.trans.scale.y));
		GLCALL(glUniform1f(m_pathShader.uniforms.thickness, style.thickness));
		GLCALL(glUniform1f(m_pathShader.uniforms.zoom, m_renderParams.zoom));

		GLCALL(glUniform3f(m_pathShader.uniforms.color, style.color.x, style.color.y, style.color.z));

		GLCALL(glBindBuffer(GL_ARRAY_BUFFER, pathGL.verticesVBO));
		GLCALL(glVertexAttribPointer(
			m_pathShader.attributes.position,
			2,
			GL_FLOAT,
			GL_FALSE,
			sizeof(GLfloat) * 2,
			nullptr
		));
		GLCALL(glEnableVertexAttribArray(m_pathShader.attributes.position));

		GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pathGL.indicesVBO));

		GLCALL(glBindBuffer(GL_ARRAY_BUFFER, pathGL.cornerVectsVBO));
		GLCALL(glVertexAttribPointer(
			m_pathShader.attributes.cornerVect,
			2,
			GL_FLOAT,
			GL_FALSE,
			sizeof(GLfloat) * 2,
			nullptr
		));
		GLCALL(glEnableVertexAttribArray(m_pathShader.attributes.cornerVect));

		GLCALL(glDrawElements(
			GL_TRIANGLE_STRIP,
			(GLsizei)pathGL.indices.size(),
			GL_UNSIGNED_SHORT,
			nullptr
		));

		GLCALL(glDisableVertexAttribArray(m_pathShader.attributes.position));
		GLCALL(glDisableVertexAttribArray(m_pathShader.attributes.cornerVect));
	}
}

void EntityRenderer::DebugDraw(double alpha, const Entity& ent, const Transform& trans, const Physics& physics)
{
	const id_t bodyId = physics.body.Id();
	const BodyGL& bodyGL = m_bodiesGL.at(bodyId);
	const Body& body = *physics.body.Get();

	glm::mat4 mvp = CalcMatrix(alpha, trans, m_renderParams);

	const std::size_t numShapes = body.GetPolygonShapes().size();
	for (std::size_t i = 0; i < numShapes; i++) {
		GLuint vbo = bodyGL.shapeVBOs[i];
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

	GLCALL(glBindBuffer(GL_ARRAY_BUFFER, bodyGL.centerVBO));
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

}

} // namespace Starbase
