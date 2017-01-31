#pragma once

#include <starbase/game/entity/entity.hpp>
#include <starbase/game/component/physics.hpp>
#include <starbase/game/component/shipcontrols.hpp>


namespace Starbase {

class ShipControlsSystem {
public:

	void Update(Entity& ent, Physics& physics, ShipControls& shipControls);
};

} // namespace Starbase
