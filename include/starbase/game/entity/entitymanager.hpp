#pragma once

#include "template/entitymanager.hpp"
#include "component_list.hpp"

namespace Starbase {
	using EntityManager = TEntityManager<ComponentList>;
}
