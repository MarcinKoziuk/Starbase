#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <starbase/game/id.hpp>
#include <starbase/game/fs/ifilesystem.hpp>
#include <starbase/game/resource/iresource.hpp>
#include <starbase/game/resource/resource_ptr.hpp>

#include <chipmunk/chipmunk_types.h>

struct NSVGshape;

namespace Starbase {

class Body : public IResource {
public:
	struct CircleShape {
		glm::tvec2<cpFloat> pos;
		cpFloat radius;

		CircleShape(const glm::tvec2<cpFloat>& pos, cpFloat radius)
			: pos(pos), radius(radius) {}
	};

	struct Hardpoint {
		enum Size { TINY, SMALL, MEDIUM, LARGE, XLARGE };

		Size size;
		struct {
			bool weapons : 1;
			bool sensors : 1;
			bool utility : 1;
		} supports;
		glm::vec2 pos;

		Hardpoint() : size(TINY), supports{false, false, false} {}
	};

private:
	cpFloat m_mass;
	cpFloat m_friction;
	std::vector<CircleShape> m_circleShapes;
	std::vector<std::vector<glm::tvec2<cpFloat>>> m_polygonShapes;
	std::vector<Hardpoint> m_hardpoints;

	static std::shared_ptr<const Body> placeholder;

	void AddShape(const NSVGshape* shape, const glm::mat4& transform);
	void AddHardpoint(const NSVGshape* shape, const glm::mat4& transform);

public:
	Body();

    virtual ~Body() {}

    virtual std::size_t CalculateSize() const;

    virtual const char* GetResourceName() const
    { return "body"; }

	cpFloat GetMass() const
	{ return m_mass; }

	cpFloat GetFriction() const
	{ return m_friction; }

	const std::vector<CircleShape>& GetCircleShapes() const
	{ return m_circleShapes; }

	const std::vector<std::vector<glm::tvec2<cpFloat>>>& GetPolygonShapes() const
	{ return m_polygonShapes; }

	const std::vector<Hardpoint>& GetHardpoints() const
	{ return m_hardpoints; }

    static std::shared_ptr<const Body> Placeholder();

    static std::shared_ptr<const Body> Create(id_t id, IFilesystem& filesystem);

	typedef ResourcePtr<Body> Ptr;
};

} // namespace Starbase
