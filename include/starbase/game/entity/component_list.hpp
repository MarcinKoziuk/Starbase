#pragma once

#include "template/component_list.hpp"

#include <starbase/game/component/transform.hpp>
#include <starbase/game/component/physics.hpp>
#include <starbase/game/component/shipcontrols.hpp>

#ifdef STARBASE_CLIENT

#include <starbase/cgame/component/renderable.hpp>

namespace Starbase {
	using ComponentList = TComponentList<Transform, Physics, ShipControls, Renderable>;
}

#else

namespace Starbase {
	using ComponentList = TComponentList<Transform, Physics, ShipControls>;
}

#endif /* STARBASE_SERVER */
