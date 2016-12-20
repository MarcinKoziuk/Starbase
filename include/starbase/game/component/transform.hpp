#pragma once

#include <glm/vec2.hpp>

namespace Starbase {

struct Transform {
    glm::vec2 pos;
    glm::vec2 scale;
    float rot;
};

} // namespace Starbase