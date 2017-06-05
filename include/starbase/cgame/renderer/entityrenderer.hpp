#pragma once

#include <vector>
#include <unordered_map>

#include <starbase/gl.hpp>

#include <starbase/game/fwd.hpp>
#include <starbase/cgame/fwd.hpp>
#include <starbase/cgame/renderer/renderparams.hpp>
#include <starbase/cgame/renderer/camera.hpp>

namespace Starbase {

class EntityRenderer {
public:
	struct PathShader {
		GLuint program;

		struct {
			GLint mvp;
			GLint resolution;
			GLint scale;
			GLint thickness;
			GLint zoom;
			GLint color;
		} uniforms;

		struct {
			GLint position;
			GLint cornerVect;
		} attributes;

		static bool Init(PathShader& dest, IFilesystem& fs);
	};

	struct LineShader {
		GLuint program;

		struct {
			GLint mvp;
		} uniforms;

		struct {
			GLint position;
		} attributes;

		static bool Init(LineShader& dest, IFilesystem& fs);
	};

	struct PathGL {
		GLuint indicesVBO;
		GLuint verticesVBO;
		GLuint cornerVectsVBO;
		std::vector<GLushort> indices;
		std::vector<glm::vec2> vertices;
		std::vector<glm::vec2> cornerVects;

		void AddRect(const glm::vec2& p1, const glm::vec2& p2);
		void AddRects(const Model::Path& path);
		PathGL(const Model::Path& path);
	};

	struct ModelGL {
		std::vector<PathGL> paths;
		std::size_t refcount;

		ModelGL(const Model& model);
	};

	struct BodyGL {
		std::vector<GLuint> shapeVBOs;
		GLuint centerVBO;
		std::size_t refcount;

		BodyGL(const Body& body);
	};

	struct ComponentGroup {
		const Entity& ent;
		const Transform& trans;
		const Renderable& rend;
		const Physics* phys;
		const ShipControls* contr;

		ComponentGroup(const Entity& ent, const Transform& trans, const Renderable& rend, const Physics* phys, const ShipControls* contr)
			: ent(ent), trans(trans), rend(rend), phys(phys), contr(contr) {}
	};

private:
	IFilesystem& m_filesystem;
	const RenderParams& m_renderParams;

	LineShader m_lineShader;
	PathShader m_pathShader;

	std::unordered_map<id_t, ModelGL> m_modelsGL;
	std::unordered_map<id_t, BodyGL> m_bodiesGL;

private:
	void NormalDraw(const ComponentGroup& cg);

	void DebugDraw(const Entity& ent, const Transform& trans, const Physics& physics);

public:
	EntityRenderer(IFilesystem& fs, const RenderParams& renderParams);

	bool Init();

	void RenderableAdded(const Renderable& rend);

	void RenderableRemoved(const Renderable& rend);

	void PhysicsAdded(const Physics& phys);

	void PhysicsRemoved(const Physics& phys);

	void Draw(const ComponentGroup& cg);
};

} // namespace Starbase