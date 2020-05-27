#pragma once

#include <vector>

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

namespace Starbase {

std::vector<glm::vec2> CasteljauRaw(const std::vector<glm::vec2>& curve);

std::vector<glm::vec2> Casteljau(const std::vector<glm::vec2>& curve, const glm::mat4& transform, bool closed);

} // namespace Starbase
