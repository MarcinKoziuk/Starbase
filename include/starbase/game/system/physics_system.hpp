#pragma once

#include <vector>
#include <unordered_map>

#include <starbase/game/id.hpp>
#include <starbase/game/entity/entity.hpp>
#include <starbase/game/component/physics.hpp>
#include <starbase/game/component/transform.hpp>

#include <starbase/game/chipmunk_safe.hpp>

namespace Starbase {

class PhysicsSystem {
private:
	std::unordered_map<id_t, std::shared_ptr<cpSpace>> m_spaces;

	void InitSpace(id_t spaceId);

	void InitBody(const Entity& ent, Transform& transf, Physics& phys);

	void ApplyGravity(cpSpace* space, float dt);
public:

	void PhysicsAdded(const Entity& ent, Transform& transf, Physics& physics);

	void PhysicsRemoved(const Entity& ent, Transform& transf, Physics& physics);

	void Simulate(float dt);

	void Update(Entity& ent, Transform& transf, Physics& physics);

	~PhysicsSystem();
};

} // namespace Starbase
