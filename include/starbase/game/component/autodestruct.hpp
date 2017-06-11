#pragma once

#include <glm/vec2.hpp>

namespace Starbase {

struct AutoDestruct {
	int initialStep;
	int ttl;

	AutoDestruct()
		: initialStep(0), ttl(0)
	{}

	AutoDestruct(int initialStep, int ttl)
		: initialStep(initialStep), ttl(ttl)
	{}
};

} // namespace Starbase