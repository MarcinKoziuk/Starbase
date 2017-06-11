#pragma once

#include <functional>

#include "template/entity.hpp"
#include "component_list.hpp"

namespace Starbase {
    using Entity = TEntity<ComponentList>;
}
