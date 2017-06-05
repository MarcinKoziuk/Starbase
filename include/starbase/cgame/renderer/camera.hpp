#pragma once

#include <glm/vec2.hpp>

namespace Starbase {

class Camera {
public:
	Camera();

	Camera(const glm::vec2& bounds);

	void Follow(const glm::vec2& entityPos);

public:
	glm::vec2 m_bounds;
	glm::vec2 m_pos;
};

} // namespace Starbase
