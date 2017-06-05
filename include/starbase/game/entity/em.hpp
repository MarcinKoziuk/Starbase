#include <starbase/game/entity/entitymanager.hpp>

#include <starbase/game/component/transform.hpp>
#include <starbase/game/component/physics.hpp>
#include <starbase/game/component/shipcontrols.hpp>

#include <starbase/cgame/component/renderable.hpp>

namespace Starbase {

using CGameParams = EParams<Transform, Physics, ShipControls, Renderable>;
using EntityManager = TEntityManager<CGameParams>;

}
