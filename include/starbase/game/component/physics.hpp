#pragma once

#include <memory>

#include <glm/vec2.hpp>

#include <starbase/game/id.hpp>
#include <starbase/game/resource/body.hpp>
#include <starbase/game/chipmunk_safe.hpp>

namespace Starbase {

struct Physics {
	id_t spaceId;
	ResourcePtr<Body> body;

	struct {
		entity_id entity;
	} cpUserData;

	struct {
		std::shared_ptr<cpSpace> space;
		cpBodyUniquePtr body;
		std::vector<cpShapeUniquePtr> shapes;
	} cp;

	Physics()
	{}

	Physics(id_t spaceId, const ResourcePtr<Body>& body)
		: spaceId(spaceId)
		, body(body)
	{}
};

} // namespace Starbase