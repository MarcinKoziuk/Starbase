#pragma once

#include <vector>

#include <glm/vec2.hpp>

namespace Starbase {

std::vector<glm::vec2> Casteljau(const std::vector<glm::vec2>& curve);

} // namespace Starbase
