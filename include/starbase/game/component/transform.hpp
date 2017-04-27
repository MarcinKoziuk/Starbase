#pragma once

#include <glm/vec2.hpp>

namespace Starbase {

struct Transform {
    glm::vec2 pos;
    glm::vec2 scale;
    float rot;
	glm::vec2 vel; // has nothing to do with transform ;P, todo

	Transform()
		: scale(1.f, 1.f)
		, rot(0.f)
	{}
};

} // namespace Starbase