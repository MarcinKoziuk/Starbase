#pragma once

namespace Starbase {

struct Entity;
struct Transform;
struct Renderable;
class ResourceLoader;

class IRenderer {
public:
	virtual bool Init() = 0;

	virtual void Shutdown() = 0;

	virtual void Draw(Entity& entity, Transform& trans, Renderable& rend, ResourceLoader& rl) = 0;

	virtual void DrawTest() = 0;

	~IRenderer() {}
};

} // namespace Starbase