#pragma once

#include <glm/vec2.hpp>

namespace Starbase {

struct RenderParams {
	glm::tvec2<int> windowSize;
	glm::vec2 offset;
	float zoom;
	bool debug;
	bool wireframe;
};

} // namespace Starbase
