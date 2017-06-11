#include <starbase/game/logging.hpp>

#include <starbase/game/system/autodestruct_system.hpp>

namespace Starbase {

void AutoDestructSystem::Update(int step, Entity& ent, AutoDestruct& autoDestruct)
{
	const int dieStep = autoDestruct.initialStep + autoDestruct.ttl;
	if (step > dieStep) {
		m_entityManager.RemoveEntity(ent);
	}
}

} // namespace Starbase
