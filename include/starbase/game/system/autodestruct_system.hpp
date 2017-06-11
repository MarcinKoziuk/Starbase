#pragma once

#include <starbase/game/entity/entity.hpp>
#include <starbase/game/entity/entitymanager.hpp>
#include <starbase/game/component/autodestruct.hpp>

namespace Starbase {

class AutoDestructSystem {
private:
	EntityManager& m_entityManager;

public:
	AutoDestructSystem(EntityManager& entityManager) : m_entityManager(entityManager) {}

	void Update(int step, Entity& ent, AutoDestruct& autoDestruct);
};

} // namespace Starbase
