#pragma once

#include <string>
#include <memory>
#include <vector>

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

private:
	cpFloat m_mass;
	std::vector<CircleShape> m_circleShapes;
	std::vector<std::vector<glm::tvec2<cpFloat>>> m_polygonShapes;

	static std::shared_ptr<const Body> placeholder;

	void AddShape(const NSVGshape* shape, const glm::mat4& transform);

public:
	Body();

    virtual ~Body() {}

    virtual std::size_t CalculateSize() const;

    virtual const char* GetResourceName() const
    { return "body"; }

	cpFloat GetMass() const
	{ return m_mass; }

	const std::vector<CircleShape>& GetCircleShapes() const
	{ return m_circleShapes; }

	const std::vector<std::vector<glm::tvec2<cpFloat>>>& GetPolygonShapes() const
	{ return m_polygonShapes; }

    static std::shared_ptr<const Body> Placeholder();

    static std::shared_ptr<const Body> Create(id_t id, IFilesystem& filesystem);

	typedef ResourcePtr<Body> Ptr;
};

} // namespace Starbase
