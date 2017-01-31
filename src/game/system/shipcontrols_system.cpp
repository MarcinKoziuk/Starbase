#include <starbase/game/logging.hpp>

#include <starbase/game/system/shipcontrols_system.hpp>
#include <starbase/game/chipmunk_safe.hpp>

namespace Starbase {

static inline void
cpBodyApplyTorque(cpBody *body, cpFloat torque)
{
	cpVect g = cpBodyGetPosition(body);
	cpVect c = cpBodyGetCenterOfGravity(body);
	cpBodyApplyImpulseAtLocalPoint(body, cpv(0.0, torque), cpv(1.0 + c.x, c.y));
	cpBodyApplyImpulseAtLocalPoint(body, cpv(0.0, -torque), cpv(-1.0 + c.x, c.y));
}

void ShipControlsSystem::Update(Entity& ent, Physics& phys, ShipControls& scontrols)
{
	cpBody* body = phys.cp.body.get();

	cpFloat ang = cpBodyGetAngularVelocity(body);
	const cpFloat maxAng = 15.0;

	cpBodyApplyTorque(body, -ang * 4);

	if (scontrols.actionFlags.rotateLeft && ang < maxAng) {
		cpBodyApplyTorque(body, 20.0);
	}
	if (scontrols.actionFlags.rotateRight && ang > -maxAng) {
		cpBodyApplyTorque(body, -20.0);
	}
	if (scontrols.actionFlags.thrustForward) {
		cpBodyApplyForceAtLocalPoint(body, cpv(0, -140), cpv(0, 0));
	}
}

} // namespace Starbase
