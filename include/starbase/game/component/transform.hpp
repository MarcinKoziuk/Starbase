#pragma once

#include <glm/vec2.hpp>

namespace Starbase {

struct Transform {
    glm::vec2 pos;
    float rot;
	glm::vec2 scale;
	glm::vec2 vel;

	Transform()
		: scale(1.f, 1.f)
		, rot(0.f)
	{}

	Transform(glm::vec2 pos, float rot, glm::vec2 scale, glm::vec2 vel)
		: pos(pos)
		, scale(scale)
		, rot(rot)
		, vel(vel)
	{}

	Transform(glm::vec2 pos, float rot, glm::vec2 scale)
		: Transform(pos, rot, scale, glm::vec2{})
	{}

	Transform(glm::vec2 pos, float rot)
		: Transform(pos, rot, glm::vec2(1.f, 1.f))
	{}

	Transform(glm::vec2 pos)
		: Transform(pos, 0.f)
	{}
};

} // namespace Starbase