#pragma once

#include <starbase/game/entity/entitymanager.hpp>
#include <starbase/game/resource/resourceloader.hpp>
#include <starbase/game/component/physics.hpp>
#include <starbase/game/component/shipcontrols.hpp>

namespace Starbase {

class ShipControlsSystem {
public:

	EntityManager& m_em;
	ResourceLoader& m_resourceLoader;

	ShipControlsSystem(EntityManager& em, ResourceLoader& resourceLoader) : m_em(em), m_resourceLoader(resourceLoader) {}

	void SpawnBullet(int step, const id_t spaceId, const glm::vec2& pos, const glm::vec2& vel);

	void Update(int step, Entity& ent, const Transform& transf, Physics& physics, ShipControls& shipControls);
};

} // namespace Starbase
